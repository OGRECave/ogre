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
#ifndef _OgreParallaxCorrectedCubemap_H_
#define _OgreParallaxCorrectedCubemap_H_

#include "OgreHlmsPbsPrerequisites.h"
#include "Cubemaps/OgreCubemapProbe.h"
#include "OgreIdString.h"
#include "OgreId.h"
#include "OgreFrameListener.h"
#include "Compositor/OgreCompositorWorkspaceListener.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
#define OGRE_MAX_CUBE_PROBES 4u
    typedef vector<CubemapProbe*>::type CubemapProbeVec;

    /**
    */
    class _OgreHlmsPbsExport ParallaxCorrectedCubemap : public IdObject,
                                                        public CompositorWorkspaceListener,
                                                        public FrameListener
    {
        CubemapProbeVec mProbes;
        CubemapProbe    *mCollectedProbes[OGRE_MAX_CUBE_PROBES];
        uint32          mNumCollectedProbes;
        Real            mProbeNDFs[OGRE_MAX_CUBE_PROBES];
        Real            mProbeBlendFactors[OGRE_MAX_CUBE_PROBES];

        public: bool                    mPaused;
        /// This variable should be updated every frame and often represents the camera position,
        /// but it can also be used set to other things like the player's character position.
        public: Vector3                 mTrackedPosition;
        public: uint32                  mMask; /// @see CubemapProbe::mMask
    private:
        GpuProgramParametersSharedPtr   mBlendCubemapParamsVs[OGRE_MAX_CUBE_PROBES];
        GpuProgramParametersSharedPtr   mBlendCubemapParams[OGRE_MAX_CUBE_PROBES];
        TextureUnitState                *mBlendCubemapTUs[OGRE_MAX_CUBE_PROBES];
        GpuProgramParametersSharedPtr   mCopyCubemapParams[6];
        TextureUnitState                *mCopyCubemapTUs[6];
        CubemapProbe                    mBlankProbe;
        CubemapProbe                    mFinalProbe;
        Camera                          *mBlendProxyCamera;
        CompositorWorkspace             *mBlendWorkspace;
        CompositorWorkspace             *mCopyWorkspace;
        TexturePtr                      mBlendCubemap;
        HlmsSamplerblock const          *mSamplerblockPoint;
        HlmsSamplerblock const          *mSamplerblockTrilinear;
        float                           mCurrentMip;
        uint32                          mProxyVisibilityMask;
        uint8                           mReservedRqId;
        MeshPtr                         mProxyMesh;
        Item                            *mProxyItems[OGRE_MAX_CUBE_PROBES];
        SceneNode                       *mProxyNodes[OGRE_MAX_CUBE_PROBES];

        Root                            *mRoot;
        SceneManager                    *mSceneManager;
        CompositorWorkspaceDef const    *mDefaultWorkspaceDef;

        struct TempRtt
        {
            TexturePtr  texture;
            uint32      refCount;
        };

        /// GPUs have the chance to optimize textures (i.e. layouts, compression) if
        /// drivers know it's not a RenderTexture or not an UAV (in other words it's
        /// read only).
        /// For dynamic textures (which we render every frame) we render directly to
        /// the Cubemap. However for static textures, we render to a temporary RTT
        /// and then copy the results to the actual optimized cubemap.
        /// Only Vulkan & D3D12 support changing the resource layout. But since
        /// we need to support other APIs, we perform a copy instead.
        /// The temp. RTTs can be shared by all probes if they match the same
        /// resolution and format.
        typedef vector<TempRtt>::type TempRttVec;
        TempRttVec  mTmpRtt;

        void createProxyGeometry(void);
        void destroyProxyGeometry(void);
        void createCubemapBlendWorkspaceDefinition(void);
        void createCubemapBlendWorkspace(void);
        void destroyCompositorData(void);

        void calculateBlendFactors(void);
        void setFinalProbeTo( size_t probeIdx );

        void updateSceneGraph(void);
        void updateRender(void);

    public:
        ParallaxCorrectedCubemap( IdType id, Root *root, SceneManager *sceneManager,
                                  const CompositorWorkspaceDef *probeWorkspaceDef,
                                  uint8 reservedRqId, uint32 proxyVisibilityMask );
        ~ParallaxCorrectedCubemap();

        /// Adds a cubemap probe.
        CubemapProbe* createProbe(void);
        void destroyProbe( CubemapProbe *probe );
        void destroyAllProbes(void);

        const CubemapProbeVec& getProbes(void) const        { return mProbes; }

        /** Enables/disables this ParallaxCorrectedCubemap system.
            It will (de)allocate some resources, thus it may cause stalls.
            If you need to temporarily pause the system (or toggle at high frequency)
            use mPaused instead (it's a public variable).
        @param bEnabled
            True to enable. False to disable. When false, the rest of the arguments are ignored.
        @param maxWidth
            This system allows probes to be of different resolution. The final merge must have
            a particular resolution though. This setting defines that value.
            If there's a probe that has a bigger resolution than this, then you'll be wasting
            memory and power on that probe.
            In other words, no probe should have a higher res than this setting.
        @param maxHeight
            See maxHeight.
        @param pixelFormat
            PixelFormat of the final blended/merged cubemap.
        */
        void setEnabled( bool bEnabled, uint32 maxWidth, uint32 maxHeight, PixelFormat pixelFormat );
        bool getEnabled(void) const;

        /// By default the probes will be constructed when the user enters its vecinity.
        /// This can cause noticeable stalls. Use this function to regenerate them all
        /// at once (i.e. at loading time)
        void updateAllDirtyProbes(void);

        size_t getConstBufferSize(void) const;
        void fillConstBufferData( const Matrix4 &viewMatrix,
                                  float * RESTRICT_ALIAS passBufferPtr ) const;

        /// See mTmpRtt. Finds an RTT that is compatible to copy to baseParams.
        /// Creates one if none found.
        TexturePtr findTmpRtt( const TexturePtr &baseParams );
        void releaseTmpRtt( const TexturePtr &tmpRtt );

        SceneManager* getSceneManager(void) const;
        const CompositorWorkspaceDef* getDefaultWorkspaceDef(void) const;

        TexturePtr _tempGetBlendCubemap(void) const     { return mBlendCubemap; }

        //Statistics
        uint32 getNumCollectedProbes(void) const        { return mNumCollectedProbes; }

        //CompositorWorkspaceListener overloads
        virtual void passPreExecute( CompositorPass *pass );
        virtual void allWorkspacesBeginUpdate(void);

        //FrameListener overloads
        virtual bool frameStarted( const FrameEvent& evt );
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
