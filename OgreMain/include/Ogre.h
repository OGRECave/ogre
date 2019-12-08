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
#ifndef _Ogre_H__
#define _Ogre_H__
// This file includes all the other files which you will need to build a client application
#include "OgrePrerequisites.h"

#include "OgreAnimation.h"
#include "OgreAnimationState.h"
#include "OgreAnimationTrack.h"
#include "OgreAny.h"
#include "OgreArchive.h"
#include "OgreArchiveManager.h"
#include "OgreAxisAlignedBox.h"
#include "OgreBillboard.h"
#include "OgreBillboardChain.h"
#include "OgreBillboardSet.h"
#include "OgreBone.h"
#include "OgreCamera.h"
#include "OgreCompositor.h"
#include "OgreCompositorManager.h"
#include "OgreCompositorChain.h"
#include "OgreCompositorInstance.h"
#include "OgreCompositionTechnique.h"
#include "OgreCompositionPass.h"
#include "OgreCompositionTargetPass.h"
#include "OgreConfigFile.h"
#include "OgreControllerManager.h"
#include "OgreDataStream.h"
#include "OgreEntity.h"
#include "OgreException.h"
#include "OgreFrameListener.h"
#include "OgreFrustum.h"
#include "OgreGpuProgram.h"
#include "OgreGpuProgramManager.h"
#include "OgreHardwareBufferManager.h"
#include "OgreHardwareIndexBuffer.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreHardwareOcclusionQuery.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreInstanceBatch.h"
#include "OgreInstancedEntity.h"
#include "OgreInstanceManager.h"
#include "OgreKeyFrame.h"
#include "OgreLight.h"
#include "OgreLogManager.h"
#include "OgreManualObject.h"
#include "OgreMaterial.h"
#include "OgreMaterialManager.h"
#include "OgreMaterialSerializer.h"
#include "OgreMath.h"
#include "OgreMatrix3.h"
#include "OgreMatrix4.h"
#include "OgreMesh.h"
#include "OgreMeshManager.h"
#include "OgreMovablePlane.h"
#include "OgreMeshSerializer.h"
#include "OgreParticleAffector.h"
#include "OgreParticleEmitter.h"
#include "OgreParticleSystem.h"
#include "OgreParticleSystemManager.h"
#include "OgrePass.h"
#include "OgrePatchMesh.h"
#include "OgrePatchSurface.h"
#include "OgreProfiler.h"
#include "OgreRadixSort.h"
#include "OgreRenderQueueInvocation.h"
#include "OgreRenderQueueListener.h"
#include "OgreRenderObjectListener.h"
#include "OgreRenderSystem.h"
#include "OgreRenderTargetListener.h"
#include "OgreRenderTexture.h"
#include "OgreRenderWindow.h"
#include "OgreResourceBackgroundQueue.h"
#include "OgreResourceGroupManager.h"
#include "OgreRibbonTrail.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "OgreSceneManagerEnumerator.h"
#include "OgreSceneNode.h"
#include "OgreShadowCameraSetup.h"
#include "OgreShadowCameraSetupFocused.h"
#include "OgreShadowCameraSetupLiSPSM.h"
#include "OgreShadowCameraSetupPlaneOptimal.h"
#include "OgreShadowCameraSetupPSSM.h"
#include "OgreSimpleRenderable.h"
#include "OgreSkeleton.h"
#include "OgreSkeletonInstance.h"
#include "OgreSkeletonManager.h"
#include "OgreSkeletonSerializer.h"
#include "OgreStaticGeometry.h"
#include "OgreString.h"
#include "OgreStringConverter.h"
#include "OgreStringVector.h"
#include "OgreSubEntity.h"
#include "OgreSubMesh.h"
#include "OgreTechnique.h"
#include "OgreTextureManager.h"
#include "OgreTextureUnitState.h"
#include "OgreTimer.h"
#include "OgreVector.h"
#include "OgreViewport.h"
#include "OgreComponents.h"
// .... more to come

#endif
