/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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
#include "OgreTerrainMaterialGeneratorA.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "OgreSceneManagerEnumerator.h"
#include "OgreTextureManager.h"
#include "OgreTexture.h"
#include "OgreTerrain.h"
#include "OgreManualObject.h"
#include "OgreCamera.h"
#include "OgreViewport.h"
#include "OgreRenderTexture.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreRenderSystem.h"


namespace Ogre
{
	//---------------------------------------------------------------------
	TerrainMaterialGenerator::TerrainMaterialGenerator() 
		: mActiveProfile(0)
		, mChangeCounter(0)
		, mDebugLevel(0) 
		, mCompositeMapSM(0)
		, mCompositeMapCam(0)
		, mCompositeMapRTT(0)
		, mCompositeMapPlane(0)
		, mCompositeMapLight(0)
	{

	}
	//---------------------------------------------------------------------
	TerrainMaterialGenerator::~TerrainMaterialGenerator()
	{
		for (ProfileList::iterator i = mProfiles.begin(); i != mProfiles.end(); ++i)
			OGRE_DELETE *i;

		if (mCompositeMapRTT && TextureManager::getSingletonPtr())
		{
			TextureManager::getSingleton().remove(mCompositeMapRTT->getHandle());
			mCompositeMapRTT = 0;
		}
		if (mCompositeMapSM && Root::getSingletonPtr())
		{
			// will also delete cam and objects etc
			Root::getSingleton().destroySceneManager(mCompositeMapSM);
			mCompositeMapSM = 0;
			mCompositeMapCam = 0;
			mCompositeMapPlane = 0;
			mCompositeMapLight = 0;
		}
	}
	//---------------------------------------------------------------------
	void TerrainMaterialGenerator::_renderCompositeMap(size_t size, 
		const Rect& rect, const MaterialPtr& mat, const TexturePtr& destCompositeMap)
	{
		if (!mCompositeMapSM)
		{
			// dedicated SceneManager
			mCompositeMapSM = Root::getSingleton().createSceneManager(DefaultSceneManagerFactory::FACTORY_TYPE_NAME);
			float camDist = 100;
			float halfCamDist = camDist * 0.5f;
			mCompositeMapCam = mCompositeMapSM->createCamera("cam");
			mCompositeMapCam->setPosition(0, 0, camDist);
			mCompositeMapCam->lookAt(Vector3::ZERO);
			mCompositeMapCam->setProjectionType(PT_ORTHOGRAPHIC);
			mCompositeMapCam->setNearClipDistance(10);
			mCompositeMapCam->setFarClipDistance(500);
			mCompositeMapCam->setOrthoWindow(camDist, camDist);

			// Just in case material relies on light auto params
			mCompositeMapLight = mCompositeMapSM->createLight();
			mCompositeMapLight->setType(Light::LT_DIRECTIONAL);

			RenderSystem* rSys = Root::getSingleton().getRenderSystem();
			Real hOffset = rSys->getHorizontalTexelOffset() / (Real)size;
			Real vOffset = rSys->getVerticalTexelOffset() / (Real)size;


			// set up scene
			mCompositeMapPlane = mCompositeMapSM->createManualObject();
			mCompositeMapPlane->begin(mat->getName());
			mCompositeMapPlane->position(-halfCamDist, halfCamDist, 0);
			mCompositeMapPlane->textureCoord(0 - hOffset, 0 - vOffset);
			mCompositeMapPlane->position(-halfCamDist, -halfCamDist, 0);
			mCompositeMapPlane->textureCoord(0 - hOffset, 1 - vOffset);
			mCompositeMapPlane->position(halfCamDist, -halfCamDist, 0);
			mCompositeMapPlane->textureCoord(1 - hOffset, 1 - vOffset);
			mCompositeMapPlane->position(halfCamDist, halfCamDist, 0);
			mCompositeMapPlane->textureCoord(1 - hOffset, 0 - vOffset);
			mCompositeMapPlane->quad(0, 1, 2, 3);
			mCompositeMapPlane->end();
			mCompositeMapSM->getRootSceneNode()->attachObject(mCompositeMapPlane);

		}

		// update
		mCompositeMapPlane->setMaterialName(0, mat->getName());
		mCompositeMapLight->setDirection(TerrainGlobalOptions::getLightMapDirection());
		mCompositeMapLight->setDiffuseColour(TerrainGlobalOptions::getCompositeMapDiffuse());
		mCompositeMapSM->setAmbientLight(TerrainGlobalOptions::getCompositeMapAmbient());


		// check for size change (allow smaller to be reused)
		if (mCompositeMapRTT && size != mCompositeMapRTT->getWidth())
		{
			TextureManager::getSingleton().remove(mCompositeMapRTT->getHandle());
			mCompositeMapRTT = 0;
		}

		if (!mCompositeMapRTT)
		{
			mCompositeMapRTT = TextureManager::getSingleton().createManual(
				mCompositeMapSM->getName() + "/compRTT", 
				ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, size, size, 0, PF_BYTE_RGBA, 
				TU_RENDERTARGET).get();
			RenderTarget* rtt = mCompositeMapRTT->getBuffer()->getRenderTarget();
			// don't render all the time, only on demand
			rtt->setAutoUpdated(false);
			Viewport* vp = rtt->addViewport(mCompositeMapCam);
			// don't render overlays
			vp->setOverlaysEnabled(false);

		}

		// calculate the area we need to update
		Real vpleft = (Real)rect.left / (Real)size;
		Real vptop = (Real)rect.top / (Real)size;
		Real vpright = (Real)rect.right / (Real)size;
		Real vpbottom = (Real)rect.bottom / (Real)size;
		Real vpwidth = (Real)rect.width()/ (Real)size;
		Real vpheight = (Real)rect.height() / (Real)size;

		RenderTarget* rtt = mCompositeMapRTT->getBuffer()->getRenderTarget();
		Viewport* vp = rtt->getViewport(0);
		mCompositeMapCam->setWindow(vpleft, vptop, vpright, vpbottom);

		rtt->update();

		// We have an RTT, we want to copy the results into a regular texture
		// That's because in non-update scenarios we don't want to keep an RTT
		// around. We use a single RTT to serve all terrain pages which is more
		// efficient.
		Image::Box box(rect.left, rect.top, rect.right, rect.bottom);
		destCompositeMap->getBuffer()->blit(mCompositeMapRTT->getBuffer(), box, box);

		
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	void TerrainMaterialGenerator::Profile::updateCompositeMap(const Terrain* terrain, const Rect& rect)
	{
		// convert point-space rect into image space
		size_t compSize = terrain->getCompositeMap()->getWidth();
		Rect imgRect;
		Vector3 inVec, outVec;
		inVec.x = rect.left;
		inVec.y = rect.bottom - 1; // this is 'top' in image space, also make inclusive
		terrain->convertPosition(Terrain::POINT_SPACE, inVec, Terrain::TERRAIN_SPACE, outVec);
		imgRect.left = outVec.x * compSize;
		imgRect.top = (1.0 - outVec.y) * compSize;
		inVec.x = rect.right - 1;
		inVec.y = rect.top; // this is 'bottom' in image space
		terrain->convertPosition(Terrain::POINT_SPACE, inVec, Terrain::TERRAIN_SPACE, outVec);
		imgRect.right = outVec.x * (Real)compSize + 1; 
		imgRect.bottom = (1.0 - outVec.y) * compSize + 1;

		imgRect.left = std::max(0L, imgRect.left);
		imgRect.top = std::max(0L, imgRect.top);
		imgRect.right = std::min((long)compSize, imgRect.right);
		imgRect.bottom = std::min((long)compSize, imgRect.bottom);
		

		mParent->_renderCompositeMap(
			compSize, imgRect, 
			terrain->getCompositeMapMaterial(), 
			terrain->getCompositeMap());

	}


}

