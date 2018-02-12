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
#include "OgreTexture.h"
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

        /// Where to position the camera while constructing the probe.
        Vector3 mProbeCameraPos;
        /// When the camera enters this area, the probe is collected for blending.
        Aabb    mArea;
        /// Value between [0; 1] per axis. At 1, the inner region matches mArea (the outer region)
        Vector3 mAreaInnerRegion;
        /// Orientationt. These are not AABBs, but rather OBB (oriented bounding boxes).
        Matrix3 mOrientation;
        Matrix3 mInvOrientation;
        /// The general shape this probe is supposed to represent.
        Aabb    mProbeShape;

        TexturePtr  mTexture;
        uint8       mFsaa;

        IdString            mWorkspaceDefName;
        CompositorWorkspace *mClearWorkspace;
        CompositorWorkspace *mWorkspace;
        Camera              *mCamera;

        ParallaxCorrectedCubemap *mCreator;

        ConstBufferPacked   *mConstBufferForManualProbes;
        uint32              mNumDatablockUsers;

        /// False if it should be updated every frame. True if only updated when dirty
        bool    mStatic;

    public:
        /// While disabled, this probe won't be updated (even if dirty) and won't be considered
        /// for blending (i.e. won't be used at all).
        bool    mEnabled;
        /// True if we must re-render to update the texture's contents. False when we don't.
        bool    mDirty;
        /// Number of iterations. The more iterations, the more light bounces and
        /// light reflections we can capture (i.e. mirror of mirrors), but it will
        /// take longer to rebuild the probe.
        /// Default value is 32.
        /// For non-static probes, you should set this value to 1 for performance.
        uint16  mNumIterations;

        /// Mask to group probes. This probe will only be updated (even if dirty) and
        /// blended if mMask & system->mMask is non-zero.
        /// Useful for example for probes to be used during the day while other probes to
        /// be used during the night; thus you only want one group to be active at the same
        /// time.
        /// Or if you have per room probes, but during a panoramic shot where many rooms
        /// are in sight, and you want a more "global" probe.
        /// Defaults to 0xffffffff
        uint32  mMask;

        void destroyWorkspace(void);
    protected:
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
        @param useManual
            Set to true if you plan on using thie probe for manually rendering, so we keep
            mipmaps at the probe level. User is responsible for supplying a workspace
            definition that will generate mipmaps though!
        */
        void setTextureParams( uint32 width, uint32 height, bool useManual=false,
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
        bool isInitialized(void) const;

        /** Sets cubemap probe's parameters.
        @param cameraPos
            Specify where the camera will be positioned.
        @param area
            When the camera enters this area, the probe is collected for blending.
        @param areaInnerRegion
            A value in range [0; 1]. It indicates a % of the OBB's size that will have smooth
            interpolation with other probes. When region = 1.0; stepping outside the OBB's results
            in a lighting "pop". The smaller the value, the smoother the transition, but at the cost
            of quality & precision while inside the OBB (as results get mixed up with other probes').
            The value is per axis.
        @param orientation
            The orientation of the AABB that makes it an OBB.
            This orientation is applied to both probeShape AND area.
            Skewing and shearing is not tested. May or may not work.
        @param probeShape
            Note AABB is actually an OBB (Oriented Bounding Box). See orientation parameter.
            The OBB should closely match the shape of the environment around it. The better it fits,
            the more accurate the reflections.
        */
        void set( const Vector3 &cameraPos,
                  const Aabb &area, const Vector3 &areaInnerRegion,
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
        void _clearCubemap(void);
        void _updateRender(void);

        const Vector3& getProbeCameraPos(void) const        { return mProbeCameraPos; }
        const Aabb& getArea(void) const                     { return mArea; }
        const Vector3& getAreaInnerRegion(void) const       { return mAreaInnerRegion; }
        const Matrix3& getOrientation(void) const           { return mOrientation; }
        const Aabb& getProbeShape(void) const               { return mProbeShape; }

        TexturePtr getInternalTexture(void) const           { return mTexture; }
        void _addReference(void);
        void _removeReference(void);

        ConstBufferPacked* getConstBufferForManualProbes(void)  { return mConstBufferForManualProbes; }
    };
}

#include "OgreHeaderSuffix.h"

#endif
