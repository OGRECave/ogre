/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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

#include "Compositor/OgreCompositorShadowNode.h"
#include "Compositor/OgreCompositorWorkspace.h"

#include "Compositor/Pass/PassScene/OgreCompositorPassScene.h"

#include "OgreTextureManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreRenderSystem.h"
#include "OgreSceneManager.h"

#include "OgreShadowCameraSetupFocused.h"
#include "OgreShadowCameraSetupLiSPSM.h"
#include "OgreShadowCameraSetupPSSM.h"

namespace Ogre
{
	CompositorShadowNode::CompositorShadowNode( IdType id, const CompositorShadowNodeDef *definition,
												CompositorWorkspace *workspace,
												RenderSystem *renderSys ) :
			CompositorNode( id, definition->getName(), definition, workspace, renderSys ),
			mDefinition( definition )
	{
		mShadowMapCameras.reserve( definition->mShadowMapTexDefinitions.size() );
		mLocalTextures.reserve( definition->mShadowMapTexDefinitions.size() );

		//Create the local textures
		CompositorShadowNodeDef::ShadowMapTexDefVec::const_iterator itor =
															definition->mShadowMapTexDefinitions.begin();
		CompositorShadowNodeDef::ShadowMapTexDefVec::const_iterator end  =
															definition->mShadowMapTexDefinitions.end();

		while( itor != end )
		{
			CompositorChannel newChannel;

			//When format list is empty, then this definition is for a shadow map atlas.
			if( !itor->formatList.empty() )
			{
				String textureName = (itor->name + IdString( id )).getFriendlyText();
				if( itor->formatList.size() == 1 )
				{
					//Normal RT
					TexturePtr tex = TextureManager::getSingleton().createManual( textureName,
													ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME,
													TEX_TYPE_2D, itor->width, itor->height, 0,
													itor->formatList[0], TU_RENDERTARGET, 0,
													itor->hwGammaWrite, itor->fsaa );
					RenderTexture* rt = tex->getBuffer()->getRenderTarget();
					newChannel.target = rt;
					newChannel.textures.push_back( tex );
				}
				else
				{
					//MRT
					MultiRenderTarget* mrt = mRenderSystem->createMultiRenderTarget( textureName );
					PixelFormatList::const_iterator pixIt = itor->formatList.begin();
					PixelFormatList::const_iterator pixEn = itor->formatList.end();

					newChannel.target = mrt;

					while( pixIt != pixEn )
					{
						size_t rtNum = pixIt - itor->formatList.begin();
						TexturePtr tex = TextureManager::getSingleton().createManual(
													textureName + StringConverter::toString( rtNum ),
													ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME,
													TEX_TYPE_2D, itor->width, itor->height, 0,
													*pixIt, TU_RENDERTARGET, 0, itor->hwGammaWrite,
													itor->fsaa );
						RenderTexture* rt = tex->getBuffer()->getRenderTarget();
						mrt->bindSurface( rtNum, rt );
						newChannel.textures.push_back( tex );
						++pixIt;
					}
				}
			}

			// Push a null RT & Texture so we preserve the index order from getTextureSource.
			mLocalTextures.push_back( newChannel );

			// One map, one camera
			const size_t shadowMapIdx = itor - definition->mShadowMapTexDefinitions.begin();
			SceneManager *sceneManager = workspace->getSceneManager();
			ShadowMapCamera shadowMapCamera;
			shadowMapCamera.camera = sceneManager->createCamera( "ShadowNode Camera ID " +
												StringConverter::toString( id ) + " Map " +
												StringConverter::toString( shadowMapIdx ) );
			switch( itor->shadowMapTechnique )
			{
			case SHADOWMAP_DEFAULT:
				shadowMapCamera.shadowCameraSetup =
								ShadowCameraSetupPtr( OGRE_NEW DefaultShadowCameraSetup() );
				break;
			/*case SHADOWMAP_PLANEOPTIMAL:
				break;*/
			case SHADOWMAP_FOCUSED:
				shadowMapCamera.shadowCameraSetup =
								ShadowCameraSetupPtr( OGRE_NEW FocusedShadowCameraSetup() );
				break;
			case SHADOWMAP_LiPSSM:
				shadowMapCamera.shadowCameraSetup =
								ShadowCameraSetupPtr( OGRE_NEW LiSPSMShadowCameraSetup() );
				break;
			case SHADOWMAP_PSSM:
				shadowMapCamera.shadowCameraSetup =
								ShadowCameraSetupPtr( OGRE_NEW PSSMShadowCameraSetup() );
				break;
			default:
				OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
							"Shadow Map technique not implemented or not recognized.",
							"CompositorShadowNode::CompositorShadowNode");
				break;
			}

			mShadowMapCameras.push_back( shadowMapCamera );

			++itor;
		}

		// Shadow Nodes don't have input; and global textures should be ready by
		// the time we get created. Therefore, we can safely initialize now as our
		// output may be used in regular nodes and we're created on-demand (as soon
		// as a Node discovers it needs us for the first time, we get created)
		createPasses();
	}
	//-----------------------------------------------------------------------------------
	CompositorShadowNode::~CompositorShadowNode()
	{
	}
	//-----------------------------------------------------------------------------------
	void CompositorShadowNode::_update( Camera* camera )
	{
		//Setup all the cameras
		CompositorShadowNodeDef::ShadowMapTexDefVec::const_iterator itor =
															mDefinition->mShadowMapTexDefinitions.begin();
		CompositorShadowNodeDef::ShadowMapTexDefVec::const_iterator end  =
															mDefinition->mShadowMapTexDefinitions.end();

		ShadowMapCameraVec::const_iterator itShadowCamera = mShadowMapCameras.begin();
		const SceneManager *sceneManager = camera->getSceneManager();

		size_t nextLightIdx = 0;
		Light const *nextLight = 0;
		size_t lastLightMap	= -1;
		size_t iterations	= 1;

		uint32 combinedVisibilityFlags = camera->getViewport()->getVisibilityMask() &
											sceneManager->getVisibilityMask();

		while( itor != end )
		{
			//iterations will be zero when the light idx difference is zero (eg. same light,
			//different split count, same light and bigger than one if there are gaps
			//(eg. render light map 0 & 2, not 1)
			//The iterators are assumed to be sorted by light index.
			iterations = itor->light - lastLightMap;
			for( size_t i=0; i<iterations; ++i )
			{
				nextLight = sceneManager->nextClosestShadowLight( nextLightIdx,
																	combinedVisibilityFlags );
			}

			if( nextLight )
			{
				itShadowCamera->shadowCameraSetup->getShadowCamera(
														sceneManager, camera, nextLight,
														itShadowCamera->camera, itor->split );
				lastLightMap = itor->light;
			}
			else
			{
				//No more shadow mapping lights. We should avoid render
				//those maps and have to set them to blank textures
				break;
			}

			++itShadowCamera;
			++itor;
		}

		//Now render all passes
		CompositorNode::_update();
	}
	//-----------------------------------------------------------------------------------
	void CompositorShadowNode::postInitializePassScene( CompositorPassScene *pass )
	{
		const CompositorPassSceneDef *passDef = pass->getDefinition();
		const ShadowMapCamera &smCamera = mShadowMapCameras[passDef->mShadowMapIdx];

		assert( (!smCamera.camera->getViewport() ||
				smCamera.camera->getViewport() == pass->getViewport()) &&
				"Two scene passes to the same shadow map have different viewport!" );

		smCamera.camera->_notifyViewport( pass->getViewport() );
		pass->_setCustomCamera( smCamera.camera );
	}
}
