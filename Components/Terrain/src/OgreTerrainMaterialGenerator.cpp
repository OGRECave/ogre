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
#include "OgreTerrainMaterialGeneratorA.h"
#include "OgreRoot.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreTextureManager.h"
#include "OgreTexture.h"
#include "OgreTerrain.h"
#include "OgreManualObject.h"
#include "OgreCamera.h"
#include "OgreViewport.h"
#include "OgreRenderSystem.h"
#include "OgreRenderTarget.h"
#include "OgreRenderTexture.h"
#include "OgreSceneNode.h"
#include "OgreRectangle2D.h"

#if OGRE_COMPILER == OGRE_COMPILER_MSVC
// we do lots of conversions here, casting them all is tedious & cluttered, we know what we're doing
#   pragma warning (disable : 4244)
#endif


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
            mLightNode = 0;
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
            mCompositeMapCam = mCompositeMapSM->createCamera("cam");
            mCompositeMapSM->getRootSceneNode()->attachObject(mCompositeMapCam);
            mCompositeMapCam->setProjectionType(PT_ORTHOGRAPHIC);
            mCompositeMapCam->setNearClipDistance(0.5);
            mCompositeMapCam->setFarClipDistance(1.5);
            mCompositeMapCam->setOrthoWindow(2, 2);

            // Just in case material relies on light auto params
            mCompositeMapLight = mCompositeMapSM->createLight();
            mCompositeMapLight->setType(Light::LT_DIRECTIONAL);
            mLightNode = mCompositeMapSM->getRootSceneNode()->createChildSceneNode();
            mLightNode->attachObject(mCompositeMapLight);

            RenderSystem* rSys = Root::getSingleton().getRenderSystem();
            Real hOffset = rSys->getHorizontalTexelOffset() / (Real)size;
            Real vOffset = rSys->getVerticalTexelOffset() / (Real)size;


            // set up scene
            mCompositeMapPlane = new Rectangle2D(true);
            mCompositeMapPlane->setCorners(-1, 1, 1, -1);
            mCompositeMapPlane->setUVs({0 - hOffset, 0 - vOffset}, {0 - hOffset, 1 - vOffset},
                                       {1 - hOffset, 0 - vOffset}, {1 - hOffset, 1 - vOffset});
            mCompositeMapPlane->setBoundingBox(AxisAlignedBox::BOX_INFINITE);

            mCompositeMapSM->getRootSceneNode()->attachObject(mCompositeMapPlane);

        }

        // update
        mCompositeMapPlane->setMaterial(mat);
        TerrainGlobalOptions& globalopts = TerrainGlobalOptions::getSingleton();
        mLightNode->setDirection(globalopts.getLightMapDirection(), Node::TS_WORLD);
        mCompositeMapLight->setDiffuseColour(globalopts.getCompositeMapDiffuse());
        mCompositeMapSM->setAmbientLight(globalopts.getCompositeMapAmbient());


        // check for size change (allow smaller to be reused)
        if (mCompositeMapRTT && size != mCompositeMapRTT->getWidth())
        {
            TextureManager::getSingleton().remove(mCompositeMapRTT->getHandle());
            mCompositeMapRTT = 0;
        }

        if (!mCompositeMapRTT)
        {
            mCompositeMapRTT = TextureManager::getSingleton().createManual(
                mCompositeMapSM->getName() + "/compRTT", mat->getGroup(),
                TEX_TYPE_2D, static_cast<uint>(size), static_cast<uint>(size), 0,
                PF_BYTE_RGBA, TU_RENDERTARGET).get();
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

        RenderTarget* rtt = mCompositeMapRTT->getBuffer()->getRenderTarget();
        mCompositeMapCam->setWindow(vpleft, vptop, vpright, vpbottom);

        rtt->update();

        // We have an RTT, we want to copy the results into a regular texture
        // That's because in non-update scenarios we don't want to keep an RTT
        // around. We use a single RTT to serve all terrain pages which is more
        // efficient.
        Box box(rect);
        destCompositeMap->getBuffer()->blit(mCompositeMapRTT->getBuffer(), box, box);

        
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void TerrainMaterialGenerator::Profile::updateCompositeMap(const Terrain* terrain, const Rect& rect)
    {
        // convert point-space rect into image space
        long compSize = terrain->getCompositeMap()->getWidth();
        Rect imgRect;
        Vector3 inVec, outVec;
        inVec.x = rect.left;
        inVec.y = rect.bottom - 1; // this is 'top' in image space, also make inclusive
        terrain->convertPosition(Terrain::POINT_SPACE, inVec, Terrain::TERRAIN_SPACE, outVec);
        imgRect.left = outVec.x * compSize;
        imgRect.top = (1.0f - outVec.y) * compSize;
        inVec.x = rect.right - 1;
        inVec.y = rect.top; // this is 'bottom' in image space
        terrain->convertPosition(Terrain::POINT_SPACE, inVec, Terrain::TERRAIN_SPACE, outVec);
        imgRect.right = outVec.x * (Real)compSize + 1; 
        imgRect.bottom = (1.0 - outVec.y) * compSize + 1;

        imgRect = imgRect.intersect({0, 0, compSize, compSize});

        mParent->_renderCompositeMap(
            compSize, imgRect, 
            terrain->getCompositeMapMaterial(), 
            terrain->getCompositeMap());

    }


}

