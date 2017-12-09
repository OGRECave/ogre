
#include "Terra/TerraShadowMapper.h"

#include "OgreTextureManager.h"
#include "OgreHardwarePixelBuffer.h"

#include "OgreSceneManager.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorChannel.h"
#include "OgreRenderTexture.h"

#include "OgreHlmsManager.h"
#include "OgreHlmsCompute.h"
#include "OgreHlmsComputeJob.h"
#include "Vao/OgreConstBufferPacked.h"
#include "OgreRoot.h"

#include "OgreLwString.h"

namespace Ogre
{
    ShadowMapper::ShadowMapper( SceneManager *sceneManager, CompositorManager2 *compositorManager ) :
        m_shadowStarts( 0 ),
        m_shadowPerGroupData( 0 ),
        m_shadowWorkspace( 0 ),
        m_shadowJob( 0 ),
        m_jobParamDelta( 0 ),
        m_jobParamXYStep( 0 ),
        m_jobParamIsStep( 0 ),
        m_jobParamHeightDelta( 0 ),
        m_sceneManager( sceneManager ),
        m_compositorManager( compositorManager )
    {
    }
    //-----------------------------------------------------------------------------------
    ShadowMapper::~ShadowMapper()
    {
        destroyShadowMap();
    }
    //-----------------------------------------------------------------------------------
    void ShadowMapper::createShadowMap( IdType id, TexturePtr &heightMapTex )
    {
        destroyShadowMap();

        m_heightMapTex = heightMapTex;

        VaoManager *vaoManager = m_sceneManager->getDestinationRenderSystem()->getVaoManager();

        if( !m_shadowStarts )
        {
            m_shadowStarts = vaoManager->createConstBuffer( 4096u * 16u,
                                                            BT_DYNAMIC_PERSISTENT, 0, false );
        }
        if( !m_shadowPerGroupData )
        {
            m_shadowPerGroupData = vaoManager->createConstBuffer( 4096u * 16u,
                                                                  BT_DYNAMIC_PERSISTENT, 0, false );
        }

        HlmsManager *hlmsManager = Root::getSingleton().getHlmsManager();
        HlmsCompute *hlmsCompute = hlmsManager->getComputeHlms();
        m_shadowJob = hlmsCompute->findComputeJob( "Terra/ShadowGenerator" );

        //TODO: Mipmaps
        m_shadowMapTex = TextureManager::getSingleton().createManual(
                    "ShadowMap" + StringConverter::toString( id ),
                    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                    TEX_TYPE_2D, m_heightMapTex->getWidth(), m_heightMapTex->getHeight(), 0,
                    PF_A2B10G10R10, TU_RENDERTARGET | TU_UAV );

        CompositorChannelVec finalTarget( 1, CompositorChannel() );
        finalTarget[0].target = m_shadowMapTex->getBuffer(0)->getRenderTarget();
        finalTarget[0].textures.push_back( m_shadowMapTex );
        m_shadowWorkspace = m_compositorManager->addWorkspace( m_sceneManager, finalTarget, 0,
                                                               "Terra/ShadowGeneratorWorkspace", false );

        ShaderParams &shaderParams = m_shadowJob->getShaderParams( "default" );
        m_jobParamDelta = shaderParams.findParameter( "delta" );
        m_jobParamXYStep = shaderParams.findParameter( "xyStep" );
        m_jobParamIsStep = shaderParams.findParameter( "isSteep" );
        m_jobParamHeightDelta = shaderParams.findParameter( "heightDelta" );

        setGaussianFilterParams( 8, 0.5f );
    }
    //-----------------------------------------------------------------------------------
    void ShadowMapper::destroyShadowMap(void)
    {
        m_heightMapTex.setNull();

        VaoManager *vaoManager = m_sceneManager->getDestinationRenderSystem()->getVaoManager();

        if( m_shadowStarts )
        {
            if( m_shadowStarts->getMappingState() != MS_UNMAPPED )
                m_shadowStarts->unmap( UO_UNMAP_ALL );
            vaoManager->destroyConstBuffer( m_shadowStarts );
            m_shadowStarts = 0;
        }

        if( m_shadowPerGroupData )
        {
            if( m_shadowPerGroupData->getMappingState() != MS_UNMAPPED )
                m_shadowPerGroupData->unmap( UO_UNMAP_ALL );
            vaoManager->destroyConstBuffer( m_shadowPerGroupData );
            m_shadowPerGroupData = 0;
        }

        if( m_shadowWorkspace )
        {
            m_compositorManager->removeWorkspace( m_shadowWorkspace );
            m_shadowWorkspace = 0;
        }

        if( !m_shadowMapTex.isNull() )
        {
            ResourcePtr resPtr = m_shadowMapTex;
            TextureManager::getSingleton().remove( resPtr );
            m_shadowMapTex.setNull();
        }
    }
    //-----------------------------------------------------------------------------------
    inline size_t ShadowMapper::getStartsPtrCount( int32 *starts, int32 *startsBase )
    {
        const size_t offset = starts - startsBase;
        if( (offset & 0x11) == 0 )
            return offset >> 2u;
        else
            return ( (offset - 2u) >> 2u ) + 4096u;
    }
    //-----------------------------------------------------------------------------------
    inline int32 ShadowMapper::getXStepsNeededToReachY( uint32 y, float fStep )
    {
        return static_cast<int32>( ceilf( Ogre::max( ( (y << 1u) - 1u ) * fStep, 0.0f ) ) );
    }
    //-----------------------------------------------------------------------------------
    inline float ShadowMapper::getErrorAfterXsteps( uint32 xIterationsToSkip, float dx, float dy )
    {
        //Round accumulatedError to next multiple of dx, then subtract accumulatedError.
        //That's the error at position (x; y). *MUST* be done in double precision, otherwise
        //we get artifacts with certain light angles.
        const double accumulatedError = dx * 0.5 + dy * (double)(xIterationsToSkip);
        const double newErrorAtX = ceil(accumulatedError / dx) * dx - accumulatedError;
        return static_cast<float>( newErrorAtX );
    }
    //-----------------------------------------------------------------------------------
    void ShadowMapper::updateShadowMap( const Vector3 &lightDir, const Vector2 &xzDimensions,
                                        float heightScale )
    {
        struct PerGroupData
        {
            int32 iterations;
            float deltaErrorStart;
            float padding0;
            float padding1;
        };

        Vector2 lightDir2d( Vector2(lightDir.x, lightDir.z).normalisedCopy() );
        float heightDelta = lightDir.y;

        if( lightDir2d.squaredLength() < 1e-6f )
        {
            //lightDir = Vector3::UNIT_Y. Fix NaNs.
            lightDir2d.x = 1.0f;
            lightDir2d.y = 0.0f;
        }

        assert( m_shadowStarts->getNumElements() >= (m_heightMapTex->getHeight() << 4u) );

        int32 *startsBase = reinterpret_cast<int32*>(
                    m_shadowStarts->map( 0, m_shadowStarts->getNumElements() ) );
        PerGroupData *perGroupData = reinterpret_cast<PerGroupData*>(
                    m_shadowPerGroupData->map( 0, m_shadowPerGroupData->getNumElements() ) );

        uint32 width    = m_heightMapTex->getWidth();
        uint32 height   = m_heightMapTex->getHeight();

        //Bresenham's line algorithm.
        float x0 = 0;
        float y0 = 0;
        float x1 = static_cast<float>( width - 1u );
        float y1 = static_cast<float>( height - 1u );

        uint32 heightOrWidth;
        uint32 widthOrHeight;

        if( fabsf( lightDir2d.x ) > fabsf( lightDir2d.y ) )
        {
            y1 *= fabsf( lightDir2d.y ) / fabsf( lightDir2d.x );
            heightOrWidth = height;
            widthOrHeight = width;

            heightDelta *= 1.0f / fabsf( lightDir.x );
        }
        else
        {
            x1 *= fabsf( lightDir2d.x ) / fabsf( lightDir2d.y );
            heightOrWidth = width;
            widthOrHeight = height;

            heightDelta *= 1.0f / fabsf( lightDir.z );
        }

        if( lightDir2d.x < 0 )
            std::swap( x0, x1 );
        if( lightDir2d.y < 0 )
            std::swap( y0, y1 );

        const bool steep = fabsf(y1 - y0) > fabsf(x1 - x0);
        if( steep )
        {
            std::swap( x0, y0 );
            std::swap( x1, y1 );
        }

        m_jobParamIsStep->setManualValue( (int32)steep );

        float dx;
        float dy;
        {
            float _x0 = x0;
            float _y0 = y0;
            float _x1 = x1;
            float _y1 = y1;
            if( _x0 > _x1 )
            {
                std::swap(_x0, _x1);
                std::swap(_y0, _y1);
            }
            dx = _x1 - _x0 + 1.0f;
            dy = fabs(_y1 - _y0);
            if( fabsf( lightDir2d.x ) > fabsf( lightDir2d.y ) )
                dy += 1.0f * fabsf( lightDir2d.y ) / fabsf( lightDir2d.x );
            else
                dy += 1.0f * fabsf( lightDir2d.x ) / fabsf( lightDir2d.y );
            m_jobParamDelta->setManualValue( Vector2( dx, dy ) );
        }

        const int32 xyStep[2] =
        {
            (x0 < x1) ? 1 : -1,
            (y0 < y1) ? 1 : -1
        };
        m_jobParamXYStep->setManualValue( xyStep, 2u );

        heightDelta = ( -heightDelta * (xzDimensions.x / width) ) / heightScale;
        //Avoid sending +/- inf (which causes NaNs inside the shader).
        //Values greater than 1.0 (or less than -1.0) are pointless anyway.
        heightDelta = Ogre::max( -1.0f, Ogre::min( 1.0f, heightDelta ) );
        m_jobParamHeightDelta->setManualValue( heightDelta );

        //y0 is not needed anymore, and we need it to be either 0 or heightOrWidth for the
        //algorithm to work correctly (depending on the sign of xyStep[1]). So do this now.
        if( y0 >= y1 )
            y0 = heightOrWidth;

        int32 *starts = startsBase;

        const float fStep = (dx * 0.5f) / dy;
        //TODO numExtraIterations correct? -1? +1?
        uint32 numExtraIterations = static_cast<uint32>(
                    Ogre::min( ceilf( dy ), ceilf( ((heightOrWidth - 1u) / fStep - 1u) * 0.5f ) ) );

        const uint32 threadsPerGroup = m_shadowJob->getThreadsPerGroupX();
        const uint32 firstThreadGroups = alignToNextMultiple( heightOrWidth,
                                                              threadsPerGroup ) / threadsPerGroup;
        const uint32 lastThreadGroups = alignToNextMultiple( numExtraIterations,
                                                             threadsPerGroup ) / threadsPerGroup;
        const uint32 totalThreadGroups = firstThreadGroups + lastThreadGroups;

        const int32 idy = static_cast<int32>( floorf( dy ) );

        //"First" series of threadgroups
        for( uint32 h=0; h<firstThreadGroups; ++h )
        {
            const uint32 startY = h * threadsPerGroup;

            for( uint32 i=0; i<threadsPerGroup; ++i )
            {
                *starts++ = static_cast<int32>( x0 );
                *starts++ = static_cast<int32>( y0 ) + static_cast<int32>( startY + i ) * xyStep[1];
                ++starts;
                ++starts;

                if( starts - startsBase >= (4096u << 2u) )
                    starts -= (4096u << 2u) - 2u;
            }

            perGroupData->iterations = widthOrHeight - std::max<int32>( 0, idy - (heightOrWidth - startY) );
            perGroupData->deltaErrorStart = 0;
            perGroupData->padding0 = 0;
            perGroupData->padding1 = 0;
            ++perGroupData;
        }

        //"Last" series of threadgroups
        for( uint32 h=0; h<lastThreadGroups; ++h )
        {
            const int32 xN = getXStepsNeededToReachY( threadsPerGroup * h + 1u, fStep );

            for( uint32 i=0; i<threadsPerGroup; ++i )
            {
                *starts++ = static_cast<int32>( x0 ) + xN * xyStep[0];
                *starts++ = static_cast<int32>( y0 ) -  static_cast<int32>( i ) * xyStep[1];
                ++starts;
                ++starts;

                if( starts - startsBase >= (4096u << 2u) )
                    starts -= (4096u << 2u) - 2u;
            }

            perGroupData->iterations = widthOrHeight - xN;
            perGroupData->deltaErrorStart = getErrorAfterXsteps( xN, dx, dy ) - dx * 0.5f;
            ++perGroupData;
        }

        m_shadowPerGroupData->unmap( UO_KEEP_PERSISTENT );
        m_shadowStarts->unmap( UO_KEEP_PERSISTENT );

        //Re-Set them every frame (they may have changed if we have multiple Terra instances)
        m_shadowJob->setConstBuffer( 0, m_shadowStarts );
        m_shadowJob->setConstBuffer( 1, m_shadowPerGroupData );
        m_shadowJob->setTexture( 0, m_heightMapTex );

        m_shadowJob->setNumThreadGroups( totalThreadGroups, 1u, 1u );

        ShaderParams &shaderParams = m_shadowJob->getShaderParams( "default" );
        shaderParams.setDirty();

        m_shadowWorkspace->_update();
    }
    //-----------------------------------------------------------------------------------
    void ShadowMapper::fillUavDataForCompositorChannel( CompositorChannel &outChannel,
                                                        ResourceLayoutMap &outInitialLayouts,
                                                        ResourceAccessMap &outInitialUavAccess ) const
    {
        outChannel.target = m_shadowMapTex->getBuffer(0)->getRenderTarget();
        outChannel.textures.push_back( m_shadowMapTex );
        outInitialLayouts.insert( m_shadowWorkspace->getResourcesLayout().begin(),
                                  m_shadowWorkspace->getResourcesLayout().end() );
        outInitialUavAccess.insert( m_shadowWorkspace->getUavsAccess().begin(),
                                    m_shadowWorkspace->getUavsAccess().end() );
    }
    //-----------------------------------------------------------------------------------
    void ShadowMapper::setGaussianFilterParams( uint8 kernelRadius, float gaussianDeviationFactor )
    {
        HlmsManager *hlmsManager = Root::getSingleton().getHlmsManager();
        HlmsCompute *hlmsCompute = hlmsManager->getComputeHlms();

        HlmsComputeJob *job = 0;
        job = hlmsCompute->findComputeJob( "Terra/GaussianBlurH" );
        setGaussianFilterParams( job, kernelRadius, gaussianDeviationFactor );
        job = hlmsCompute->findComputeJob( "Terra/GaussianBlurV" );
        setGaussianFilterParams( job, kernelRadius, gaussianDeviationFactor );
    }
    //-----------------------------------------------------------------------------------
    void ShadowMapper::setGaussianFilterParams( HlmsComputeJob *job, uint8 kernelRadius,
                                                float gaussianDeviationFactor )
    {
        assert( !(kernelRadius & 0x01) && "kernelRadius must be even!" );

        if( job->getProperty( "kernel_radius" ) != kernelRadius )
            job->setProperty( "kernel_radius", kernelRadius );
        ShaderParams &shaderParams = job->getShaderParams( "default" );

        std::vector<float> weights( kernelRadius + 1u );

        const float fKernelRadius = kernelRadius;
        const float gaussianDeviation = fKernelRadius * gaussianDeviationFactor;

        //It's 2.0f if using the approximate filter (sampling between two pixels to
        //get the bilinear interpolated result and cut the number of samples in half)
        const float stepSize = 1.0f;

        //Calculate the weights
        float fWeightSum = 0;
        for( uint32 i=0; i<kernelRadius + 1u; ++i )
        {
            const float _X = i - fKernelRadius + ( 1.0f - 1.0f / stepSize );
            float fWeight = 1.0f / sqrt ( 2.0f * Math::PI * gaussianDeviation * gaussianDeviation );
            fWeight *= exp( - ( _X * _X ) / ( 2.0f * gaussianDeviation * gaussianDeviation ) );

            fWeightSum += fWeight;
			weights[i] = fWeight;
        }

        fWeightSum = fWeightSum * 2.0f - weights[kernelRadius];

        //Normalize the weights
        for( uint32 i=0; i<kernelRadius + 1u; ++i )
            weights[i] /= fWeightSum;

        //Remove shader constants from previous calls (needed in case we've reduced the radius size)
        ShaderParams::ParamVec::iterator itor = shaderParams.mParams.begin();
        ShaderParams::ParamVec::iterator end  = shaderParams.mParams.end();

        while( itor != end )
        {
            String::size_type pos = itor->name.find( "c_weights[" );

            if( pos != String::npos )
            {
                itor = shaderParams.mParams.erase( itor );
                end  = shaderParams.mParams.end();
            }
            else
            {
                ++itor;
            }
        }

        //Set the shader constants, 16 at a time (since that's the limit of what ManualParam can hold)
        char tmp[32];
        LwString weightsString( LwString::FromEmptyPointer( tmp, sizeof(tmp) ) );
        const uint32 floatsPerParam = sizeof( ShaderParams::ManualParam().dataBytes ) / sizeof(float);
        for( uint32 i=0; i<kernelRadius + 1u; i += floatsPerParam )
        {
            weightsString.clear();
            weightsString.a( "c_weights[", i, "]" );

            ShaderParams::Param p;
            p.isAutomatic   = false;
            p.isDirty       = true;
            p.name = weightsString.c_str();
            shaderParams.mParams.push_back( p );
            ShaderParams::Param *param = &shaderParams.mParams.back();

            param->setManualValue( &weights[i], std::min<uint32>( floatsPerParam, weights.size() - i ) );
        }

        shaderParams.setDirty();
    }
}
