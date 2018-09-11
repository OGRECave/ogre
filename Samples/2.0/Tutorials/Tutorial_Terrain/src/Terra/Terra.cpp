
#include "Terra/Terra.h"
#include "Terra/TerraShadowMapper.h"

#include "OgreImage.h"
#include "OgreTextureManager.h"
#include "OgreHardwarePixelBuffer.h"

#include "OgreCamera.h"
#include "OgreSceneManager.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorChannel.h"
#include "OgreMaterialManager.h"
#include "OgreTechnique.h"
#include "OgreRenderTexture.h"

namespace Ogre
{
    Terra::Terra( IdType id, ObjectMemoryManager *objectMemoryManager,
                  SceneManager *sceneManager, uint8 renderQueueId,
                  CompositorManager2 *compositorManager, Camera *camera ) :
        MovableObject( id, objectMemoryManager, sceneManager, renderQueueId ),
        m_width( 0u ),
        m_depth( 0u ),
        m_depthWidthRatio( 1.0f ),
        m_skirtSize( 10.0f ),
        m_invWidth( 1.0f ),
        m_invDepth( 1.0f ),
        m_xzDimensions( Vector2::UNIT_SCALE ),
        m_xzInvDimensions( Vector2::UNIT_SCALE ),
        m_xzRelativeSize( Vector2::UNIT_SCALE ),
        m_height( 1.0f ),
        m_terrainOrigin( Vector3::ZERO ),
        m_basePixelDimension( 256u ),
        m_currentCell( 0u ),
        m_prevLightDir( Vector3::ZERO ),
        m_shadowMapper( 0 ),
        m_compositorManager( compositorManager ),
        m_camera( camera )
    {
    }
    //-----------------------------------------------------------------------------------
    Terra::~Terra()
    {
        if( m_shadowMapper )
        {
            m_shadowMapper->destroyShadowMap();
            delete m_shadowMapper;
            m_shadowMapper = 0;
        }
        destroyNormalTexture();
        destroyHeightmapTexture();
        m_terrainCells.clear();
    }
    //-----------------------------------------------------------------------------------
    void Terra::destroyHeightmapTexture(void)
    {
        if( !m_heightMapTex.isNull() )
        {
            ResourcePtr resPtr = m_heightMapTex;
            TextureManager::getSingleton().remove( resPtr );
            m_heightMapTex.setNull();
        }
    }
    //-----------------------------------------------------------------------------------
    void Terra::createHeightmapTexture( const Ogre::Image &image, const String &imageName )
    {
        destroyHeightmapTexture();

        if( image.getBPP() != 8 && image.getBPP() != 16 && image.getFormat() != PF_FLOAT32_R )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "Texture " + imageName + "must be 8 bpp, 16 bpp, or 32-bit Float",
                         "Terra::createHeightmapTexture" );
        }

        //const uint8 numMipmaps = image.getNumMipmaps();
        const uint8 numMipmaps = 0u;

        m_heightMapTex = TextureManager::getSingleton().createManual(
                    "HeightMapTex" + StringConverter::toString( getId() ),
                    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                    TEX_TYPE_2D, (uint)image.getWidth(), (uint)image.getHeight(),
                    numMipmaps, image.getFormat(), TU_STATIC_WRITE_ONLY );

        for( uint8 mip=0; mip<=numMipmaps; ++mip )
        {
            v1::HardwarePixelBufferSharedPtr pixelBufferBuf = m_heightMapTex->getBuffer(0, mip);
            const PixelBox &currImage = pixelBufferBuf->lock( Box( 0, 0,
                                                                   pixelBufferBuf->getWidth(),
                                                                   pixelBufferBuf->getHeight() ),
                                                              v1::HardwareBuffer::HBL_DISCARD );
            PixelUtil::bulkPixelConversion( image.getPixelBox(0, mip), currImage );
            pixelBufferBuf->unlock();
        }
    }
    //-----------------------------------------------------------------------------------
    void Terra::createHeightmap( Image &image, const String &imageName )
    {
        m_width = image.getWidth();
        m_depth = image.getHeight();
        m_depthWidthRatio = m_depth / (float)(m_width);
        m_invWidth = 1.0f / m_width;
        m_invDepth = 1.0f / m_depth;

        if( PixelUtil::getComponentCount( image.getFormat() ) != 1 )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "Only grayscale images supported! " + imageName,
                         "Terra::createHeightmap" );
        }

        //image.generateMipmaps( false, Image::FILTER_NEAREST );

        createHeightmapTexture( image, imageName );

        m_heightMap.resize( m_width * m_depth );

        const float maxValue = powf( 2.0f, (float)image.getBPP() ) - 1.0f;
        const float invMaxValue = 1.0f / maxValue;

        if( image.getBPP() == 8 )
        {
            const uint8 * RESTRICT_ALIAS data = reinterpret_cast<uint8*RESTRICT_ALIAS>(image.getData());
            for( uint32 y=0; y<m_depth; ++y )
            {
                for( uint32 x=0; x<m_width; ++x )
                    m_heightMap[y * m_width + x] = (data[y * m_width + x] * invMaxValue) * m_height;
            }
        }
        else if( image.getBPP() == 16 )
        {
            const uint16 * RESTRICT_ALIAS data = reinterpret_cast<uint16*RESTRICT_ALIAS>(
                                                                                        image.getData());
            for( uint32 y=0; y<m_depth; ++y )
            {
                for( uint32 x=0; x<m_width; ++x )
                    m_heightMap[y * m_width + x] = (data[y * m_width + x] * invMaxValue) * m_height;
            }
        }
        else if( image.getFormat() == PF_FLOAT32_R )
        {
            const float * RESTRICT_ALIAS data = reinterpret_cast<float*RESTRICT_ALIAS>(image.getData());
            for( uint32 y=0; y<m_depth; ++y )
            {
                for( uint32 x=0; x<m_width; ++x )
                    m_heightMap[y * m_width + x] = data[y * m_width + x] * m_height;
            }
        }

        m_xzRelativeSize = m_xzDimensions / Vector2( static_cast<Real>(m_width),
                                                     static_cast<Real>(m_depth) );

        createNormalTexture();

        m_prevLightDir = Vector3::ZERO;

        delete m_shadowMapper;
        m_shadowMapper = new ShadowMapper( mManager, m_compositorManager );
        m_shadowMapper->createShadowMap( getId(), m_heightMapTex );

        calculateOptimumSkirtSize();
    }
    //-----------------------------------------------------------------------------------
    void Terra::createNormalTexture(void)
    {
        destroyNormalTexture();

        m_normalMapTex = TextureManager::getSingleton().createManual(
                    "NormalMapTex_" + StringConverter::toString( getId() ),
                    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                    TEX_TYPE_2D, m_heightMapTex->getWidth(), m_heightMapTex->getHeight(),
                    PixelUtil::getMaxMipmapCount( m_heightMapTex->getWidth(),
                                                  m_heightMapTex->getHeight() ),
                    PF_A2B10G10R10, TU_RENDERTARGET|TU_AUTOMIPMAP );

        MaterialPtr normalMapperMat = MaterialManager::getSingleton().load(
                    "Terra/GpuNormalMapper",
                    ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME ).
                staticCast<Material>();
        Pass *pass = normalMapperMat->getTechnique(0)->getPass(0);
        TextureUnitState *texUnit = pass->getTextureUnitState(0);
        texUnit->setTexture( m_heightMapTex );

        //Normalize vScale for better precision in the shader math
        const Vector3 vScale = Vector3( m_xzRelativeSize.x, m_height, m_xzRelativeSize.y ).normalisedCopy();

        GpuProgramParametersSharedPtr psParams = pass->getFragmentProgramParameters();
        psParams->setNamedConstant( "heightMapResolution", Vector4( static_cast<Real>( m_width ),
                                                                    static_cast<Real>( m_depth ),
                                                                    1, 1 ) );
        psParams->setNamedConstant( "vScale", vScale );

        CompositorChannelVec finalTargetChannels( 1, CompositorChannel() );
        finalTargetChannels[0].target = m_normalMapTex->getBuffer()->getRenderTarget();
        finalTargetChannels[0].textures.push_back( m_normalMapTex );

        Camera *dummyCamera = mManager->createCamera( "TerraDummyCamera" );

        CompositorWorkspace *workspace =
                m_compositorManager->addWorkspace( mManager, finalTargetChannels, dummyCamera,
                                                   "Terra/GpuNormalMapperWorkspace", false );
        workspace->_beginUpdate( true );
        workspace->_update();
        workspace->_endUpdate( true );

        m_compositorManager->removeWorkspace( workspace );
        mManager->destroyCamera( dummyCamera );
    }
    //-----------------------------------------------------------------------------------
    void Terra::destroyNormalTexture(void)
    {
        if( !m_normalMapTex.isNull() )
        {
            ResourcePtr resPtr = m_normalMapTex;
            TextureManager::getSingleton().remove( resPtr );
            m_normalMapTex.setNull();
        }
    }
    //-----------------------------------------------------------------------------------
    void Terra::calculateOptimumSkirtSize(void)
    {
        m_skirtSize = std::numeric_limits<float>::max();

        const uint32 basePixelDimension = m_basePixelDimension;
        const uint32 vertPixelDimension = static_cast<uint32>(m_basePixelDimension * m_depthWidthRatio);

        for( size_t y=vertPixelDimension-1u; y<m_depth-1u; y += vertPixelDimension )
        {
            const size_t ny = y + 1u;

            bool allEqualInLine = true;
            float minHeight = m_heightMap[y * m_width];
            for( size_t x=0; x<m_width; ++x )
            {
                const float minValue = Ogre::min( m_heightMap[y * m_width + x],
                                                  m_heightMap[ny * m_width + x] );
                minHeight = Ogre::min( minValue, minHeight );
                allEqualInLine &= m_heightMap[y * m_width + x] == m_heightMap[ny * m_width + x];
            }

            if( !allEqualInLine )
                m_skirtSize = Ogre::min( minHeight, m_skirtSize );
        }

        for( size_t x=basePixelDimension-1u; x<m_width-1u; x += basePixelDimension )
        {
            const size_t nx = x + 1u;

            bool allEqualInLine = true;
            float minHeight = m_heightMap[x];
            for( size_t y=0; y<m_depth; ++y )
            {
                const float minValue = Ogre::min( m_heightMap[y * m_width + x],
                                                  m_heightMap[y * m_width + nx] );
                minHeight = Ogre::min( minValue, minHeight );
                allEqualInLine &= m_heightMap[y * m_width + x] == m_heightMap[y * m_width + nx];
            }

            if( !allEqualInLine )
                m_skirtSize = Ogre::min( minHeight, m_skirtSize );
        }

        m_skirtSize /= m_height;
    }
    //-----------------------------------------------------------------------------------
    inline GridPoint Terra::worldToGrid( const Vector3 &vPos ) const
    {
        GridPoint retVal;
        const float fWidth = static_cast<float>( m_width );
        const float fDepth = static_cast<float>( m_depth );

        const float fX = floorf( ((vPos.x - m_terrainOrigin.x) * m_xzInvDimensions.x) * fWidth );
        const float fZ = floorf( ((vPos.z - m_terrainOrigin.z) * m_xzInvDimensions.y) * fDepth );
        retVal.x = fX >= 0.0f ? static_cast<uint32>( fX ) : 0xffffffff;
        retVal.z = fZ >= 0.0f ? static_cast<uint32>( fZ ) : 0xffffffff;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    inline Vector2 Terra::gridToWorld( const GridPoint &gPos ) const
    {
        Vector2 retVal;
        const float fWidth = static_cast<float>( m_width );
        const float fDepth = static_cast<float>( m_depth );

        retVal.x = (gPos.x / fWidth) * m_xzDimensions.x + m_terrainOrigin.x;
        retVal.y = (gPos.z / fDepth) * m_xzDimensions.y + m_terrainOrigin.z;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    bool Terra::isVisible( const GridPoint &gPos, const GridPoint &gSize ) const
    {
        if( gPos.x >= static_cast<int32>( m_width ) ||
            gPos.z >= static_cast<int32>( m_depth ) ||
            gPos.x + gSize.x <= 0 ||
            gPos.z + gSize.z <= 0 )
        {
            //Outside terrain bounds.
            return false;
        }

//        return true;

        const Vector2 cellPos = gridToWorld( gPos );
        const Vector2 cellSize( (gSize.x + 1u) * m_xzRelativeSize.x,
                                (gSize.z + 1u) * m_xzRelativeSize.y );

        const Vector3 vHalfSize = Vector3( cellSize.x, m_height, cellSize.y ) * 0.5f;
        const Vector3 vCenter = Vector3( cellPos.x, m_terrainOrigin.y, cellPos.y ) + vHalfSize;

        for( int i=0; i<6; ++i )
        {
            //Skip far plane if view frustum is infinite
            if( i == FRUSTUM_PLANE_FAR && m_camera->getFarClipDistance() == 0 )
                continue;

            Plane::Side side = m_camera->getFrustumPlane(i).getSide( vCenter, vHalfSize );

            //We only need one negative match to know the obj is outside the frustum
            if( side == Plane::NEGATIVE_SIDE )
                return false;
        }

        return true;
    }
    //-----------------------------------------------------------------------------------
    void Terra::addRenderable( const GridPoint &gridPos, const GridPoint &cellSize, uint32 lodLevel )
    {
        TerrainCell *cell = &m_terrainCells[m_currentCell++];
        cell->setOrigin( gridPos, cellSize.x, cellSize.z, lodLevel );
        m_collectedCells[0].push_back( cell );
    }
    //-----------------------------------------------------------------------------------
    void Terra::optimizeCellsAndAdd(void)
    {
        //Keep iterating until m_collectedCells[0] stops shrinking
        size_t numCollectedCells = std::numeric_limits<size_t>::max();
        while( numCollectedCells != m_collectedCells[0].size() )
        {
            numCollectedCells = m_collectedCells[0].size();

            if( m_collectedCells[0].size() > 1 )
            {
                m_collectedCells[1].clear();

                std::vector<TerrainCell*>::const_iterator itor = m_collectedCells[0].begin();
                std::vector<TerrainCell*>::const_iterator end  = m_collectedCells[0].end();

                while( end - itor >= 2u )
                {
                    TerrainCell *currCell = *itor;
                    TerrainCell *nextCell = *(itor+1);

                    m_collectedCells[1].push_back( currCell );
                    if( currCell->merge( nextCell ) )
                        itor += 2;
                    else
                        ++itor;
                }

                while( itor != end )
                    m_collectedCells[1].push_back( *itor++ );

                m_collectedCells[1].swap( m_collectedCells[0] );
            }
        }

        std::vector<TerrainCell*>::const_iterator itor = m_collectedCells[0].begin();
        std::vector<TerrainCell*>::const_iterator end  = m_collectedCells[0].end();
        while( itor != end )
            mRenderables.push_back( *itor++ );

        m_collectedCells[0].clear();
    }
    //-----------------------------------------------------------------------------------
    void Terra::update( const Vector3 &lightDir, float lightEpsilon )
    {
        const float lightCosAngleChange = Math::Clamp(
                    (float)m_prevLightDir.dotProduct( lightDir.normalisedCopy() ), -1.0f, 1.0f );
        if( lightCosAngleChange <= (1.0f - lightEpsilon) )
        {
            m_shadowMapper->updateShadowMap( lightDir, m_xzDimensions, m_height );
            m_prevLightDir = lightDir.normalisedCopy();
        }
        //m_shadowMapper->updateShadowMap( Vector3::UNIT_X, m_xzDimensions, m_height );
        //m_shadowMapper->updateShadowMap( Vector3(2048,0,1024), m_xzDimensions, m_height );
        //m_shadowMapper->updateShadowMap( Vector3(1,0,0.1), m_xzDimensions, m_height );
        //m_shadowMapper->updateShadowMap( Vector3::UNIT_Y, m_xzDimensions, m_height ); //Check! Does NAN

        mRenderables.clear();
        m_currentCell = 0;

        Vector3 camPos = m_camera->getDerivedPosition();

        const uint32 basePixelDimension = m_basePixelDimension;
        const uint32 vertPixelDimension = static_cast<uint32>(m_basePixelDimension * m_depthWidthRatio);

        GridPoint cellSize;
        cellSize.x = basePixelDimension;
        cellSize.z = vertPixelDimension;

        //Quantize the camera position to basePixelDimension steps
        GridPoint camCenter = worldToGrid( camPos );
        camCenter.x = (camCenter.x / basePixelDimension) * basePixelDimension;
        camCenter.z = (camCenter.z / vertPixelDimension) * vertPixelDimension;

        uint32 currentLod = 0;

//        camCenter.x = 64;
//        camCenter.z = 64;

        //LOD 0: Add full 4x4 grid
        for( int32 z=-2; z<2; ++z )
        {
            for( int32 x=-2; x<2; ++x )
            {
                GridPoint pos = camCenter;
                pos.x += x * cellSize.x;
                pos.z += z * cellSize.z;

                if( isVisible( pos, cellSize ) )
                    addRenderable( pos, cellSize, currentLod );
            }
        }

        optimizeCellsAndAdd();

        m_currentCell = 16u; //The first 16 cells don't use skirts.

        const uint32 maxRes = std::max( m_width, m_depth );
        //TODO: When we're too far (outside the terrain), just display a 4x4 grid or something like that.

        size_t numObjectsAdded = std::numeric_limits<size_t>::max();
        //LOD n: Add 4x4 grid, ignore 2x2 center (which
        //is the same as saying the borders of the grid)
        while( numObjectsAdded != m_currentCell ||
               (mRenderables.empty() && (1u << currentLod) <= maxRes) )
        {
            numObjectsAdded = m_currentCell;

            cellSize.x <<= 1u;
            cellSize.z <<= 1u;
            ++currentLod;

            //Row 0
            {
                const int32 z = 1;
                for( int32 x=-2; x<2; ++x )
                {
                    GridPoint pos = camCenter;
                    pos.x += x * cellSize.x;
                    pos.z += z * cellSize.z;

                    if( isVisible( pos, cellSize ) )
                        addRenderable( pos, cellSize, currentLod );
                }
            }
            //Row 3
            {
                const int32 z = -2;
                for( int32 x=-2; x<2; ++x )
                {
                    GridPoint pos = camCenter;
                    pos.x += x * cellSize.x;
                    pos.z += z * cellSize.z;

                    if( isVisible( pos, cellSize ) )
                        addRenderable( pos, cellSize, currentLod );
                }
            }
            //Cells [0, 1] & [0, 2];
            {
                const int32 x = -2;
                for( int32 z=-1; z<1; ++z )
                {
                    GridPoint pos = camCenter;
                    pos.x += x * cellSize.x;
                    pos.z += z * cellSize.z;

                    if( isVisible( pos, cellSize ) )
                        addRenderable( pos, cellSize, currentLod );
                }
            }
            //Cells [3, 1] & [3, 2];
            {
                const int32 x = 1;
                for( int32 z=-1; z<1; ++z )
                {
                    GridPoint pos = camCenter;
                    pos.x += x * cellSize.x;
                    pos.z += z * cellSize.z;

                    if( isVisible( pos, cellSize ) )
                        addRenderable( pos, cellSize, currentLod );
                }
            }

            optimizeCellsAndAdd();
        }
    }
    //-----------------------------------------------------------------------------------
    void Terra::load( const String &texName, const Vector3 center, const Vector3 &dimensions )
    {
        Ogre::Image image;
        image.load( texName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );

        load( image, center, dimensions, texName );
    }
    //-----------------------------------------------------------------------------------
    void Terra::load( Image &image, const Vector3 center, const Vector3 &dimensions, const String &imageName )
    {
        m_terrainOrigin = center - dimensions * 0.5f;
        m_xzDimensions = Vector2( dimensions.x, dimensions.z );
        m_xzInvDimensions = 1.0f / m_xzDimensions;
        m_height = dimensions.y;
        m_basePixelDimension = 64u;
        createHeightmap( image, imageName );

        {
            //Find out how many TerrainCells we need. I think this might be
            //solved analitically with a power series. But my math is rusty.
            const uint32 basePixelDimension = m_basePixelDimension;
            const uint32 vertPixelDimension = static_cast<uint32>( m_basePixelDimension *
                                                                   m_depthWidthRatio );
            const uint32 maxPixelDimension = std::max( basePixelDimension, vertPixelDimension );
            const uint32 maxRes = std::max( m_width, m_depth );

            uint32 numCells = 16u; //4x4
            uint32 accumDim = 0u;
            uint32 iteration = 1u;
            while( accumDim < maxRes )
            {
                numCells += 12u; //4x4 - 2x2
                accumDim += maxPixelDimension * (1u << iteration);
                ++iteration;
            }

            numCells += 12u;
            accumDim += maxPixelDimension * (1u << iteration);
            ++iteration;

            m_terrainCells.clear();
            m_terrainCells.resize( numCells, TerrainCell( this ) );
        }

        VaoManager *vaoManager = mManager->getDestinationRenderSystem()->getVaoManager();
        std::vector<TerrainCell>::iterator itor = m_terrainCells.begin();
        std::vector<TerrainCell>::iterator end  = m_terrainCells.end();

        const std::vector<TerrainCell>::iterator begin = itor;

        while( itor != end )
        {
            itor->initialize( vaoManager, (itor - begin) >= 16u );
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    bool Terra::getHeightAt( Vector3 &vPos ) const
    {
        bool retVal = false;
        GridPoint pos2D = worldToGrid( vPos );

        if( pos2D.x < m_width-1 && pos2D.z < m_depth-1 )
        {
            const Vector2 vPos2D = gridToWorld( pos2D );

            const float dx = (vPos.x - vPos2D.x) * m_width * m_xzInvDimensions.x;
            const float dz = (vPos.z - vPos2D.y) * m_depth * m_xzInvDimensions.y;

            float a, b, c;
            const float h00 = m_heightMap[ pos2D.z * m_width + pos2D.x ];
            const float h11 = m_heightMap[ (pos2D.z+1) * m_width + pos2D.x + 1 ];

            c = h00;
            if( dx < dz )
            {
                //Plane eq: y = ax + bz + c
                //x=0 z=0 -> c		= h00
                //x=0 z=1 -> b + c	= h01 -> b = h01 - c
                //x=1 z=1 -> a + b + c  = h11 -> a = h11 - b - c
                const float h01 = m_heightMap[ (pos2D.z+1) * m_width + pos2D.x ];

                b = h01 - c;
                a = h11 - b - c;
            }
            else
            {
                //Plane eq: y = ax + bz + c
                //x=0 z=0 -> c		= h00
                //x=1 z=0 -> a + c	= h10 -> a = h10 - c
                //x=1 z=1 -> a + b + c  = h11 -> b = h11 - a - c
                const float h10 = m_heightMap[ pos2D.z * m_width + pos2D.x + 1 ];

                a = h10 - c;
                b = h11 - a - c;
            }

            vPos.y = a * dx + b * dz + c + m_terrainOrigin.y;
            retVal = true;
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void Terra::setDatablock( HlmsDatablock *datablock )
    {
        std::vector<TerrainCell>::iterator itor = m_terrainCells.begin();
        std::vector<TerrainCell>::iterator end  = m_terrainCells.end();

        while( itor != end )
        {
            itor->setDatablock( datablock );
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    Ogre::TexturePtr Terra::_getShadowMapTex(void) const
    {
        return m_shadowMapper->getShadowMapTex();
    }
    //-----------------------------------------------------------------------------------
    const String& Terra::getMovableType(void) const
    {
        static const String movType = "Terra";
        return movType;
    }
}
