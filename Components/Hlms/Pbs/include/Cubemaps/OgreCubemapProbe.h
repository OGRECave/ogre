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
#ifndef _OgreCubemapProbe_H_
#define _OgreCubemapProbe_H_

#include "OgreHlmsPbsPrerequisites.h"
#include "OgreVector3.h"
#include "Math/Simple/OgreAabb.h"
#include "OgreIdString.h"
#include "OgrePixelFormat.h"
#include "OgreHeaderPrefix.h"

//It's slightly more accurate if we render the cubemaps and generate the cubemaps, then blend.
//But Ogre doesn't yet support RTT to mipmaps, so we generate the mipmaps after blending.
#define GENERATE_MIPMAPS_ON_BLEND 1

namespace Ogre
{
    class ParallaxCorrectedCubemap;
    class CompositorWorkspaceDef;

    class _OgreHlmsPbsExport CubemapProbe : public UtilityAlloc
    {
        friend class ParallaxCorrectedCubemap;

        /// When the camera enters this area, the probe is collected for blending.
        /// Also its center is where the camera will be positioned.
        Aabb    mArea;
        /// Value between [0; 1] per axis. At 1, the inner region matches mArea (the outer region)
        Vector3 mAreaInnerRegion;
        /// Orientationt. These are not AABBs, but rather OBB (oriented bounding boxes).
        Matrix3 mOrientation;
        /// The general shape this probe is supposed to represent.
        Aabb    mProbeShape;

        TexturePtr  mTexture;
        uint8       mFsaa;

        IdString            mWorkspaceDefName;
        CompositorWorkspace *mWorkspace;
        Camera              *mCamera;

        ParallaxCorrectedCubemap *mCreator;

        /// False if it should be updated every frame. True if only updated when dirty
        bool    mStatic;

    public:
        /// True if we must re-render to update the texture's contents. False when we don't.
        bool    mDirty;

    protected:
        void destroyWorkspace(void);
        void destroyTexture(void);

    public:
        CubemapProbe( ParallaxCorrectedCubemap *creator );
        ~CubemapProbe();

        /**
        @param width
        @param height
        @param pf
        @param isStatic
            Set to False if it should be updated every frame. True if only updated when dirty
        @param fsaa
        */
        void setTextureParams( uint32 width, uint32 height,
                               PixelFormat pf=PF_A8B8G8R8, bool isStatic=true, uint8 fsaa=0 );

        /** Initializes the workspace so we can actually render to the cubemap.
            You must call setTextureParams first.
        @param workspaceDefOverride
            Pass a null IdString() to use the default workspace definition passed to
            ParallaxCorrectedCubemap.
            This value allows you to override it with a different workspace definition.
        */
        void initWorkspace( float cameraNear = 0.5f, float cameraFar = 500.0f,
                            IdString workspaceDefOverride=IdString() );

        /** Sets cubemap probe's parameters.
        @param area
            When the camera enters this area, the probe is collected for blending.
            Also its center is where the camera will be positioned.
        @param areaInnerRegion
            A value in range [0; 1]. It indicates a % of the OBB's size that will have smooth
            interpolation with other probes. When region = 1.0; stepping outside the OBB's results
            in a lighting "pop". The smaller the value, the smoother the transition, but at the cost
            of quality & precision while inside the OBB (as results get mixed up with other probes').
            The value is per axis.
        @param orientation
            The orientation of the AABB that makes it an OBB. Skewing and shearing is supported.
            This orientation is applied to both probeShape AND area.
        @param probeShape
            Note AABB is actually an OBB (Oriented Bounding Box). See orientation parameter.
            The OBB should closely match the shape of the environment around it. The better it fits,
            the more accurate the reflections.
        */
        void set( const Aabb &area, const Vector3 &areaInnerRegion,
                  const Matrix3 &orientation, const Aabb &probeShape );

        /** Set to False if it should be updated every frame. True if only updated when dirty
        @remarks
            This call is not cheap.
        */
        void setStatic( bool isStatic );
        bool getStatic(void) const          { return mStatic; }

        Aabb getAreaLS(void) const          { return Aabb( Vector3::ZERO, mArea.mHalfSize ); }

        /** Gets the Normalized Distance Function.
        @param posLS
            Position, in local space (relative to this probe)
        @return
            Interpretation:
                <=0 means we're inside the inner range, or in its border.
                Range (0; 1) we're between inner and outer range.
                >=1 means we're outside the object.
        */
        Real getNDF( const Vector3 &posLS ) const;

        void _prepareForRendering(void);
        void _updateRender(void);

        const Aabb& getArea(void) const                     { return mArea; }
        const Vector3& getAreaInnerRegion(void) const       { return mAreaInnerRegion; }
        const Matrix3& getOrientation(void) const           { return mOrientation; }
        const Aabb& getProbeShape(void) const               { return mProbeShape; }
    };
}

#include "OgreHeaderSuffix.h"

#endif
