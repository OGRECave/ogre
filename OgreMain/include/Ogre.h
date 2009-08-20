/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
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
#include "OgreBillboardChain.h"
#include "OgreBillboardSet.h"
#include "OgreBone.h"
#include "OgreCamera.h"
#include "OgreConfigFile.h"
#include "OgreControllerManager.h"
#include "OgreDataStream.h"
#include "OgreEntity.h"
#include "OgreException.h"
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
#include "OgreMeshSerializer.h"
#include "OgreOverlay.h"
#include "OgreOverlayContainer.h"
#include "OgreOverlayElement.h"
#include "OgreOverlayManager.h"
#include "OgreParticleAffector.h"
#include "OgreParticleEmitter.h"
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
#include "OgreUserDefinedObject.h"
#include "OgreVector2.h"
#include "OgreViewport.h"
#include "OgreCompositor.h"
#include "OgreCompositorManager.h"
#include "OgreCompositorChain.h"
#include "OgreCompositorInstance.h"
#include "OgreCompositionTechnique.h"
#include "OgreCompositionPass.h"
#include "OgreCompositionTargetPass.h"
#include "OgreWindowEventUtilities.h"
#include "OgreShadowCameraSetup.h"
#include "OgreShadowCameraSetupFocused.h"
#include "OgreShadowCameraSetupLiSPSM.h"
#include "OgreShadowCameraSetupPlaneOptimal.h"
#include "OgreShadowCameraSetupPSSM.h"
// .... more to come

#endif
