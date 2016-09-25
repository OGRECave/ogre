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

namespace Ogre
{
    class ParallaxCorrectedCubemap;
    class CompositorWorkspaceDef;

    class CubemapProbe : public UtilityAlloc
    {
        friend class ParallaxCorrectedCubemap;

        Vector3 mProbePos;
        Aabb    mArea;
        Matrix3 mAabbOrientation;
        Vector3 mAabbFalloff;

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
                               PixelFormat pf=PF_A8B8G8R8, bool isStatic=true, uint8 fsaa=1 );

        /** Initializes the workspace so we can actually render to the cubemap.
            You must call setTextureParams first.
        @param workspaceDefOverride
            Pass a null IdString() to use the default workspace definition passed to
            ParallaxCorrectedCubemap.
            This value allows you to override it with a different workspace definition.
        */
        void initWorkspace( IdString workspaceDefOverride=IdString() );

        /** Sets cubemap probe's parameters.
        @param probePos
            The location where the cubemap camera will be positioned for rendering.
        @param area
            The region that will be affected by this probe. When user's camera is inside this area,
            this probe becomes the main active one. Note AABB is actually an OBB (Oriented Bounding
            Box). See aabbOrientation parameter.
            The OBB should closely match the shape of the environment around it. The better it fits,
            the more accurate the reflections.
        @param aabbOrientation
            The orientation of the AABB that makes it an OBB. Skewing and shearing is supported.
        @param aabbFalloff
            A value in range [0; 1]. It indicates a % of the OBB's size that will have smooth
            interpolation with other probes. W/ falloff = 1.0; stepping outside the OBB's results
            in a lighting "pop". The smaller the value, the smoother the transition, but at the cost
            of quality & precision while inside the OBB (as results get mixed up with other probes').
            The falloff is per axis.
        */
        void set( const Vector3 &probePos, const Aabb &area,
                  const Matrix3 &aabbOrientation, const Vector3 &aabbFalloff );

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

        const Vector3& getProbePos(void) const              { return mProbePos; }
        const Aabb& getArea(void) const                     { return mArea; }
        const Matrix3& getAabbOrientation(void) const       { return mAabbOrientation; }
        const Vector3& getAabbFalloff(void) const           { return mAabbFalloff; }
    };
}

#include "OgreHeaderSuffix.h"

#endif
