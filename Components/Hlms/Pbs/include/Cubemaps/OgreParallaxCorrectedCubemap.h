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
#include "Compositor/OgreCompositorWorkspaceListener.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
#define OGRE_MAX_CUBE_PROBES 4u
    /**
    */
    class _OgreHlmsPbsExport ParallaxCorrectedCubemap : public IdObject,
                                                        public CompositorWorkspaceListener
    {
        typedef vector<CubemapProbe*>::type CubemapProbeVec;
        CubemapProbeVec mProbes;
        CubemapProbe    *mCollectedProbes[OGRE_MAX_CUBE_PROBES];
        uint32          mNumCollectedProbes;
        Real            mProbeNDFs[OGRE_MAX_CUBE_PROBES];
        Real            mProbeBlendFactors[OGRE_MAX_CUBE_PROBES];

        GpuProgramParametersSharedPtr   mBlendCubemapParams[6];
        TextureUnitState                *mBlendCubemapTUs[6*OGRE_MAX_CUBE_PROBES];
        CubemapProbe                    mBlankProbe;
        Camera                          *mBlendDummyCamera;
        CompositorWorkspace             *mBlendWorkspace;
        TexturePtr                      mBlendCubemap;
        HlmsSamplerblock const          *mSamplerblockPoint;
        HlmsSamplerblock const          *mSamplerblockTrilinear;
        float                           mCurrentMip;

        SceneManager                    *mSceneManager;
        CompositorWorkspaceDef const    *mProbeWorkspaceDef;

        void createCubemapBlendWorkspaceDefinition(void);
        void createCubemapBlendWorkspace(void);
        void destroyCompositorData(void);

        void calculateBlendFactors(void);

    public:
        ParallaxCorrectedCubemap( IdType id, SceneManager *sceneManager,
                                  const CompositorWorkspaceDef *probeWorkspaceDef );
        ~ParallaxCorrectedCubemap();

        /// Adds a cubemap probe.
        CubemapProbe* createProbe(void);
        void destroyProbe( CubemapProbe *probe );
        void destroyAllProbes(void);

        void updateSceneGraph(void);
        void updateRender(void);

        //CompositorWorkspaceListener overloads
        virtual void passPreExecute( CompositorPass *pass );
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
