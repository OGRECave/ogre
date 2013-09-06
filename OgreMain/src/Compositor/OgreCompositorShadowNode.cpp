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
			mDefinition( definition ),
			mLastCamera( 0 )
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
					rt->setDepthBufferPool( itor->depthBufferId );
					newChannel.target = rt;
					newChannel.textures.push_back( tex );
				}
				else
				{
					//MRT
					MultiRenderTarget* mrt = mRenderSystem->createMultiRenderTarget( textureName );
					PixelFormatList::const_iterator pixIt = itor->formatList.begin();
					PixelFormatList::const_iterator pixEn = itor->formatList.end();

					mrt->setDepthBufferPool( itor->depthBufferId );
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
	void CompositorShadowNode::buildClosestLightList( const Camera *newCamera )
	{
		if( mLastCamera == newCamera )
			return;

		const Real EPSILON = 1e-6f;

		mLastCamera = newCamera;
		mShadowMapLightIndex.clear();

		const Viewport *viewport = newCamera->getViewport();
		const SceneManager *sceneManager = newCamera->getSceneManager();
		const LightListInfo &globalLightList = sceneManager->getGlobalLightList();

		uint32 combinedVisibilityFlags = viewport->getVisibilityMask() &
											sceneManager->getVisibilityMask();

		const size_t numLights = std::min( mDefinition->mNumLights, globalLightList.lights.size() );
		mShadowMapLightIndex.reserve( numLights );

		const Vector3 &camPos( newCamera->getDerivedPosition() );

		size_t minIdx = -1;
		Real minMaxDistance = -std::numeric_limits<Real>::max();

		//O(N*M) Complexity. Not my brightest moment. Feel free to improve this snippet,
		//but profile it! (M tends to be very small, usually way below 8)
		for( size_t i=0; i<numLights; ++i )
		{
			Real minDistance = std::numeric_limits<Real>::max();
			uint32 const * RESTRICT_ALIAS visibilityMask = globalLightList.visibilityMask;
			Sphere const * RESTRICT_ALIAS boundingSphere = globalLightList.boundingSphere;
			for( size_t j=0; j<globalLightList.lights.size(); ++j )
			{
				if( *visibilityMask & combinedVisibilityFlags &&
					*visibilityMask & MovableObject::LAYER_SHADOW_CASTER )
				{
					const Real fDist = camPos.distance( boundingSphere->getCenter() ) -
										boundingSphere->getRadius();
					if( fDist < minDistance && fDist > minMaxDistance )
					{
						bool bNewIdx = true;
						if( Math::Abs( fDist - minMaxDistance ) < EPSILON && minIdx != j )
						{
							//Rare case where two or more lights are equally distant
							//from the camera. Check whether we've already added it
							LightIndexVec::const_iterator it = std::find( mShadowMapLightIndex.begin(),
																		mShadowMapLightIndex.end(), j );
							if( it != mShadowMapLightIndex.end() )
								bNewIdx = false;
						}

						if( bNewIdx )
						{
							minIdx = j;
							minDistance		= fDist;
							minMaxDistance	= fDist;
						}
					}
				}

				if( minIdx != -1 &&
					(mShadowMapLightIndex.empty() || minIdx != mShadowMapLightIndex.back()) )
				{
					mShadowMapLightIndex.push_back( minIdx );
				}

				++visibilityMask;
				++boundingSphere;
			}
		}
	}
	//-----------------------------------------------------------------------------------
	void CompositorShadowNode::_update( Camera* camera )
	{
		ShadowMapCameraVec::const_iterator itShadowCamera = mShadowMapCameras.begin();
		SceneManager *sceneManager	= camera->getSceneManager();
		const Viewport *viewport	= camera->getViewport();

		buildClosestLightList( camera );

		const LightListInfo &globalLightList = sceneManager->getGlobalLightList();

		//Setup all the cameras
		CompositorShadowNodeDef::ShadowMapTexDefVec::const_iterator itor =
															mDefinition->mShadowMapTexDefinitions.begin();
		CompositorShadowNodeDef::ShadowMapTexDefVec::const_iterator end  =
															mDefinition->mShadowMapTexDefinitions.end();

		while( itor != end )
		{
			if( itor->light < mShadowMapLightIndex.size() )
			{
				Light const *light = globalLightList.lights[mShadowMapLightIndex[itor->light]];

				Camera *texCamera = itShadowCamera->camera;

				//Use the material scheme of the main viewport 
				//This is required to pick up the correct shadow_caster_material and similar properties.
				texCamera->getViewport()->setMaterialScheme( viewport->getMaterialScheme() );

				// Associate main view camera as LOD camera
				texCamera->setLodCamera( camera );

				// set base
				if( light->getType() != Light::LT_POINT )
					texCamera->setDirection( light->getDerivedDirection() );
				if( light->getType() != Light::LT_DIRECTIONAL )
					texCamera->setPosition( light->getDerivedPosition() );

				itShadowCamera->shadowCameraSetup->getShadowCamera( sceneManager, camera, light,
																	texCamera, itor->split );
			}
			//Else... this shadow map shouldn't be rendered(TODO) and when used, return a blank one.
			//The Nth closest lights don't cast shadows

			++itShadowCamera;
			++itor;
		}

		SceneManager::IlluminationRenderStage previous = sceneManager->_getCurrentRenderStage();
		sceneManager->_setCurrentRenderStage( SceneManager::IRS_RENDER_TO_TEXTURE );

		//TODO: Set SceneManager::mShadowTextureCurrentCasterLightList & mShadowTextureIndexLightList (or refactor)

		//Now render all passes
		CompositorNode::_update();

		sceneManager->_setCurrentRenderStage( previous );
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
