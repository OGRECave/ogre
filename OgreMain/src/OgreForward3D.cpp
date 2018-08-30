/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#include "OgreStableHeaders.h"

#include "OgreForward3D.h"
#include "OgreSceneManager.h"
#include "OgreRenderTarget.h"
#include "OgreViewport.h"
#include "OgreCamera.h"

#include "Compositor/OgreCompositorShadowNode.h"

#include "Vao/OgreVaoManager.h"
#include "Vao/OgreTexBufferPacked.h"

#include "OgreHlms.h"

#include "OgreProfiler.h"

namespace Ogre
{
    Forward3D::Forward3D( uint32 width, uint32 height,
                          uint32 numSlices, uint32 lightsPerCell,
                          float minDistance, float maxDistance,
                          SceneManager *sceneManager ) :
        ForwardPlusBase( sceneManager, false ),
        mWidth( width ),
        mHeight( height ),
        mNumSlices( numSlices ),
        /*mWidth( 1 ),
        mHeight( 1 ),
        mNumSlices( 2 ),*/
        mLightsPerCell( lightsPerCell + 3u ),
        mTableSize( mWidth * mHeight * mLightsPerCell ),
        mMinDistance( minDistance ),
        mMaxDistance( maxDistance ),
        mInvMaxDistance( 1.0f / mMaxDistance )
    {
        assert( numSlices > 1 && "Must use at least 2 slices for Forward3D!" );

        uint32 sliceWidth   = mWidth;
        uint32 sliceHeight  = mHeight;
        mResolutionAtSlice.reserve( mNumSlices );

        for( uint32 i=0; i<mNumSlices; ++i )
        {
            mResolutionAtSlice.push_back( Resolution( sliceWidth, sliceHeight,
                                                      getDepthAtSlice( i + 1 ) ) );
            sliceWidth   *= 2;
            sliceHeight  *= 2;
        }

        mResolutionAtSlice.back().zEnd = std::numeric_limits<Real>::max();

        const size_t p = -((1 - (1 << (mNumSlices << 1))) / 3);
        mLightCountInCell.resize( p * mWidth * mHeight, LightCount() );
    }
    //-----------------------------------------------------------------------------------
    Forward3D::~Forward3D()
    {
    }
    //-----------------------------------------------------------------------------------
    inline Real Forward3D::getDepthAtSlice( uint32 uSlice ) const
    {
        /*Real normalizedDepth = uSlice / static_cast<Real>( mNumSlices-1 );

        normalizedDepth = 1.0f - normalizedDepth;
        normalizedDepth = Math::Sqrt( normalizedDepth );
        normalizedDepth = Math::Sqrt( normalizedDepth );
        normalizedDepth = Math::Sqrt( normalizedDepth );

        normalizedDepth = 1.0f - normalizedDepth;*/
        Real normalizedDepth = uSlice / static_cast<Real>( mNumSlices-1 );
        return -((normalizedDepth * mMaxDistance) - mMinDistance);
    }
    //-----------------------------------------------------------------------------------
    inline uint32 Forward3D::getSliceAtDepth( Real depth ) const
    {
        //normalizedDepth is in range [0; 1]
        //We use the formula f(x) = 1 - (1 - (x + min)) ^ 8
        //to have a non-linear distribution of the slices across depth.
        /*Real normalizedDepth = 1.0f - Math::saturate( (-depth + mMinDistance) * mInvMaxDistance );
        normalizedDepth = (normalizedDepth * normalizedDepth) * (normalizedDepth * normalizedDepth);
        normalizedDepth = normalizedDepth * normalizedDepth;
        normalizedDepth = 1.0f - normalizedDepth;*/
        Real normalizedDepth = Math::saturate( (-depth + mMinDistance) * mInvMaxDistance );

        return static_cast<uint32>( floorf( normalizedDepth * (mNumSlices-1) ) );
    }
    //-----------------------------------------------------------------------------------
    inline void Forward3D::projectionSpaceToGridSpace( const Vector2 &projSpace, uint32 slice,
                                                       uint32 &outX, uint32 &outY ) const
    {
        const Resolution &res = mResolutionAtSlice[slice];
        float fx = Math::saturate( projSpace.x ) * res.width;
        float fy = Math::saturate( projSpace.y ) * res.height;
        outX = static_cast<uint32>( Ogre::min( floorf( fx ), res.width - 1 ) );
        outY = static_cast<uint32>( Ogre::min( floorf( fy ), res.height - 1 ) );
    }
    //-----------------------------------------------------------------------------------
    inline bool OrderLightByDistanceToCamera3D( const Light *left, const Light *right )
    {
        if( left->getType() != right->getType() )
            return left->getType() < right->getType();
        return left->getCachedDistanceToCameraAsReal() < right->getCachedDistanceToCameraAsReal();
    }

    void Forward3D::collectLights( Camera *camera )
    {
        CachedGrid *cachedGrid = 0;
        if( getCachedGridFor( camera, &cachedGrid ) )
            return; //Up to date.

        OgreProfile( "Forward3D Light Collect" );

        //Cull the lights against the camera. Get non-directional, non-shadow-casting lights
        //(lights set to cast shadows but currently not casting shadows are also included)
        if( mSceneManager->getCurrentShadowNode() )
        {
            //const LightListInfo &globalLightList = mSceneManager->getGlobalLightList();
            const CompositorShadowNode *shadowNode = mSceneManager->getCurrentShadowNode();

            //Exclude shadow casting lights
            const LightClosestArray &shadowCastingLights = shadowNode->getShadowCastingLights();
            LightClosestArray::const_iterator itor = shadowCastingLights.begin();
            LightClosestArray::const_iterator end  = shadowCastingLights.end();

            while( itor != end )
            {
                if( itor->light )
                    itor->light->setVisible( false );
                ++itor;
            }

            mSceneManager->cullLights( camera, Light::LT_POINT,
                                       Light::MAX_FORWARD_PLUS_LIGHTS, mCurrentLightList );

            //Restore shadow casting lights
            itor = shadowCastingLights.begin();
            end  = shadowCastingLights.end();

            while( itor != end )
            {
                if( itor->light )
                    itor->light->setVisible( true );
                ++itor;
            }
        }
        else
        {
            mSceneManager->cullLights( camera, Light::LT_POINT,
                                       Light::MAX_FORWARD_PLUS_LIGHTS, mCurrentLightList );
        }

        const size_t numLights = mCurrentLightList.size();

        //Sort by distance to camera
        std::sort( mCurrentLightList.begin(), mCurrentLightList.end(), OrderLightByDistanceToCamera3D );

        //Allocate the buffers if not already.
        CachedGridBuffer &gridBuffers = cachedGrid->gridBuffers[cachedGrid->currentBufIdx];
        if( !gridBuffers.gridBuffer )
        {
            const size_t p = -((1 - (1 << (mNumSlices << 1))) / 3);
            gridBuffers.gridBuffer = mVaoManager->createTexBuffer( PF_R16_UINT,
                                                                   p * mTableSize * sizeof(uint16),
                                                                   BT_DYNAMIC_PERSISTENT, 0, false );
        }

        const size_t bufferBytesNeeded = calculateBytesNeeded( std::max<size_t>( numLights, 96u ), 0u );

        if( !gridBuffers.globalLightListBuffer ||
            gridBuffers.globalLightListBuffer->getNumElements() < bufferBytesNeeded )
        {
            if( gridBuffers.globalLightListBuffer )
            {
                if( gridBuffers.globalLightListBuffer->getMappingState() != MS_UNMAPPED )
                    gridBuffers.globalLightListBuffer->unmap( UO_UNMAP_ALL );
                mVaoManager->destroyTexBuffer( gridBuffers.globalLightListBuffer );
            }

            gridBuffers.globalLightListBuffer = mVaoManager->createTexBuffer(
                                                                    PF_FLOAT32_RGBA,
                                                                    bufferBytesNeeded,
                                                                    BT_DYNAMIC_PERSISTENT, 0, false );
        }

        //Fill the first buffer with the light. The other buffer contains indexes into this list.
        fillGlobalLightListBuffer( camera, gridBuffers.globalLightListBuffer );

        //Fill the indexes buffer
        uint16 * RESTRICT_ALIAS gridBuffer = reinterpret_cast<uint16 * RESTRICT_ALIAS>(
                    gridBuffers.gridBuffer->map( 0, gridBuffers.gridBuffer->getNumElements() ) );

        memset( mLightCountInCell.begin(), 0, mLightCountInCell.size() * sizeof(LightCount) );

        Matrix4 viewMat = camera->getViewMatrix();
        Matrix4 projMatrix = camera->getProjectionMatrix();

        Real nearPlane  = camera->getNearClipDistance();
        Real farPlane   = camera->getFarClipDistance();

        if( farPlane == 0 )
            farPlane = std::numeric_limits<Real>::max();

        assert( mNumSlices < 256 );

        Real projSpaceSliceEnd[256];
        for( uint32 i=0; i<mNumSlices-1; ++i )
        {
            Vector4 r = projMatrix * Vector4( 0, 0, Math::Clamp( mResolutionAtSlice[i].zEnd,
                                                                 -farPlane, -nearPlane ), 1.0f );
            projSpaceSliceEnd[i] = r.z / r.w;
        }

        projSpaceSliceEnd[mNumSlices-1] = 1.0f;

        LightArray::const_iterator itLight = mCurrentLightList.begin();

        for( size_t i=0; i<numLights; ++i )
        {
            //Aabb lightAabb = (*itLight)->getWorldAabb();
            //lightAabb.transformAffine( viewMat );
            Aabb lightAabb = (*itLight)->getLocalAabb();
            lightAabb.transformAffine( viewMat * (*itLight)->_getParentNodeFullTransform() );

            //Lower left origin
            Vector3 vMin3 = lightAabb.getMinimum();
            //Upper right
            Vector3 vMax3 = lightAabb.getMaximum();

            //Light space is backwards, in range [-farDistance; -nearDistance]
            std::swap( vMin3.z, vMax3.z );

            vMin3.z = Math::Clamp( vMin3.z, -farPlane, -nearPlane );
            vMax3.z = Math::Clamp( vMax3.z, -farPlane, -nearPlane );

            // bottomLeft[0] = bottom left corner of front face of the AABB.
            // bottomLeft[1] = bottom left corner of back face of the AABB.
            // topRight[0] = top right corner of front face of the AABB.
            // topRight[1] = top right corner of back face of the AABB.
            // All of it in projection space in range [0; 1]
            Vector3 bottomLeft[2], topRight[2];
            {
                Vector4 vStart4[2], vEnd4[2];
                vStart4[0]  = Vector4( vMin3 );
                vStart4[1]  = vStart4[0];
                vEnd4[0]    = Vector4( vMax3 );
                vEnd4[1]    = vEnd4[0];

                vEnd4[0].z  = vMin3.z;
                vStart4[1].z= vMax3.z;

                for( int j=0; j<2; ++j )
                {
                    vStart4[j]  = projMatrix * vStart4[j];
                    vEnd4[j]    = projMatrix * vEnd4[j];

                    const Real invStartW = 1.0f / vStart4[j].w;
                    const Real invEndW = 1.0f / vEnd4[j].w;
                    bottomLeft[j].x = (vStart4[j].x * invStartW) * 0.5f + 0.5f;
                    bottomLeft[j].y = (vStart4[j].y * invStartW) * 0.5f + 0.5f;
                    bottomLeft[j].z = (vStart4[j].z * invStartW);
                    topRight[j].x   = (vEnd4[j].x * invEndW) * 0.5f + 0.5f;
                    topRight[j].y   = (vEnd4[j].y * invEndW) * 0.5f + 0.5f;
                    topRight[j].z   = (vEnd4[j].z * invEndW);
                }
            }

            const Real lightSpaceMinDepth = bottomLeft[0].z;
            const Real lightSpaceMaxDepth = bottomLeft[1].z;

            uint32 minSlice = getSliceAtDepth( vMin3.z );
            uint32 maxSlice = getSliceAtDepth( vMax3.z );

            const Real invLightSpaceDepthDist = 1.0f / (lightSpaceMaxDepth - lightSpaceMinDepth);

            //We will interpolate between the front and back faces of the AABB by view space
            //depth at both the beginning of the current slice and the end of it.
            //The 2D rectangle that encloses both slices defines the area occupied in the
            //
            //Since the end of the current slice is the beginning of the next one, we just
            //copy the data from interpBL[1] onto interpBL[0] at the end of each iteration
            //and only calculate interpBL[1] in every iteration (performance optimization)
            Vector3 interpBL[2], interpTR[2];
            interpBL[0] = bottomLeft[0];
            interpTR[0] = topRight[0];

            assert( (minSlice > maxSlice) || (minSlice < mNumSlices && maxSlice < mNumSlices) );

            //Derive the offset analytically.
            // Normally offset is =
            //     = w * h * mLightsPerCell * 2 + w * 2 * h * 2 * mLightsPerCell * 2 +
            //       w * 4 * h * 4 * mLightsPerCell * 2 + ...
            //
            //This is a **geometric series** of 4^n where n is minSlice+1.
            //  The formula is:
            //    = [(1 - 4^n) / (1 - 4)] * (w * h * mLightsPerCell * 2)
            //    = [(1 - 4^n) / (1 - 4)] * mTableSize
            //    = [(1 - (1 << (n * 2))) / (-3)] * mTableSize
            const size_t p = -((1 - (1 << (minSlice << 1))) / 3);
            size_t offset           = p * mTableSize;
            size_t offsetLightCount = p * mWidth * mHeight;

            const Light::LightTypes lightType = (*itLight)->getType();

            for( uint32 slice=minSlice; slice<=maxSlice; ++slice )
            {
                //The end of this slice may go past beyond the back face of the AABB.
                //Clamp to avoid overestimating the rectangle's area
                const Real depthAtSlice = Ogre::min( lightSpaceMaxDepth, projSpaceSliceEnd[slice] );

                //Interpolate the back face
                float fW = (depthAtSlice - lightSpaceMinDepth) * invLightSpaceDepthDist;
                interpBL[1] = Math::lerp( bottomLeft[0], bottomLeft[1], fW );
                interpTR[1] = Math::lerp( topRight[0], topRight[1], fW );

                //Find the rectangle that encloses both the front and back faces.
                const Vector2 finalBL( Ogre::min( interpBL[0].x, interpBL[1].x ),
                                       Ogre::min( interpBL[0].y, interpBL[1].y ) );
                const Vector2 finalTR( Ogre::max( interpTR[0].x, interpTR[1].x ),
                                       Ogre::max( interpTR[0].y, interpTR[1].y ) );

                uint32 startX, startY, endX, endY;
                projectionSpaceToGridSpace( finalBL, slice, startX, startY );
                projectionSpaceToGridSpace( finalTR, slice, endX, endY );

                const Resolution &sliceRes = mResolutionAtSlice[slice];
                for( uint32 y=startY; y<=endY; ++y )
                {
                    for( uint32 x=startX; x<=endX; ++x )
                    {
                        FastArray<LightCount>::iterator numLightsInCell =
                                mLightCountInCell.begin() + offsetLightCount + (y * sliceRes.width) + x;

                        //assert( numLightsInCell < mLightCountInCell.end() );

                        //mLightsPerCell - 3 because three slots is reserved
                        //for the number of lights in cell per type
                        if( numLightsInCell->lightCount[0] < mLightsPerCell - 3u )
                        {
                            uint16 * RESTRICT_ALIAS cellElem =
                                    gridBuffer + offset + (y * sliceRes.width + x) * mLightsPerCell +
                                    (numLightsInCell->lightCount[0] + 3u);
                            *cellElem = i * 6;
                            ++numLightsInCell->lightCount[0];
                            ++numLightsInCell->lightCount[lightType];
                        }
                    }
                }

                //The old back face is the new front face.
                interpBL[0] = interpBL[1];
                interpTR[0] = interpTR[1];
                offset           += sliceRes.width * sliceRes.height * mLightsPerCell;
                offsetLightCount += sliceRes.width * sliceRes.height;
            }

            ++itLight;
        }

        {
            //Now write all the light counts
            FastArray<LightCount>::const_iterator itor = mLightCountInCell.begin();
            FastArray<LightCount>::const_iterator end  = mLightCountInCell.end();

            const size_t cellSize = mLightsPerCell;
            size_t gridIdx = 0;

            while( itor != end )
            {
                uint32 accumLight = itor->lightCount[1];
                gridBuffer[gridIdx+0u] = static_cast<uint16>( accumLight );
                accumLight += itor->lightCount[2];
                gridBuffer[gridIdx+1u] = static_cast<uint16>( accumLight );
                accumLight += itor->lightCount[3];
                gridBuffer[gridIdx+2u] = static_cast<uint16>( accumLight );
                gridIdx += cellSize;
                ++itor;
            }
        }

        gridBuffers.gridBuffer->unmap( UO_KEEP_PERSISTENT );

        deleteOldGridBuffers();
    }
    //-----------------------------------------------------------------------------------
    size_t Forward3D::getConstBufferSize(void) const
    {
        // (2 + mNumSlices) vars * 4 (vec4) * 4 bytes = 12
        return (2 + mNumSlices) * 4 * 4;
    }
    //-----------------------------------------------------------------------------------
    void Forward3D::fillConstBufferData( Viewport *viewport, RenderTarget* renderTarget,
                                         IdString shaderSyntax,
                                         float * RESTRICT_ALIAS passBufferPtr ) const
    {
        //vec4 f3dData;
        *passBufferPtr++ = mMinDistance;
        *passBufferPtr++ = mInvMaxDistance;
        *passBufferPtr++ = static_cast<float>( mNumSlices - 1 );
        *reinterpret_cast<uint32*RESTRICT_ALIAS>(passBufferPtr) = mTableSize;
        ++passBufferPtr;

        const float fLightsPerCell = static_cast<float>( mLightsPerCell );

        const float viewportWidth = static_cast<float>( viewport->getActualWidth() );
        const float viewportHeight = static_cast<float>( viewport->getActualHeight() );
        const float viewportWidthOffset = static_cast<float>( viewport->getActualLeft() );
        float viewportHeightOffset = static_cast<float>( viewport->getActualTop() );

        //The way ogre represents viewports is top = 0 bottom = 1. As a result if 'texture flipping'
        //is required then all is ok. However if it is not required then viewport offsets are
        //actually represented from the bottom up.
        //As a result we need convert our viewport height offsets to work bottom up instead of top down;
        //This is compounded by OpenGL standard being different to DirectX and Metal
        if( !renderTarget->requiresTextureFlipping() && shaderSyntax == "glsl" )
        {
            viewportHeightOffset =
                    static_cast<float>( (1.0 - (viewport->getTop() + viewport->getHeight())) *
                                        renderTarget->getHeight() );
        }

        //vec4 f3dGridHWW[mNumSlices];
        for( uint32 i=0; i<mNumSlices; ++i )
        {
            *passBufferPtr++ = static_cast<float>( mResolutionAtSlice[i].width ) / viewportWidth;
            *passBufferPtr++ = static_cast<float>( mResolutionAtSlice[i].height ) / viewportHeight;
            *passBufferPtr++ = static_cast<float>( mResolutionAtSlice[i].width * mLightsPerCell );
            *passBufferPtr++ = i < 1u ? fLightsPerCell : viewportHeight;
        }

        //vec4 f3dViewportOffset
        *passBufferPtr++ = viewportWidthOffset;
        *passBufferPtr++ = viewportHeightOffset;
        *passBufferPtr++ = 0;
        *passBufferPtr++ = 0;
    }
    //-----------------------------------------------------------------------------------
    void Forward3D::setHlmsPassProperties( Hlms *hlms )
    {
        ForwardPlusBase::setHlmsPassProperties( hlms );

        hlms->_setProperty( HlmsBaseProp::ForwardPlus, HlmsBaseProp::Forward3D.mHash );
        hlms->_setProperty( HlmsBaseProp::Forward3DNumSlices, mNumSlices );
    }
}
