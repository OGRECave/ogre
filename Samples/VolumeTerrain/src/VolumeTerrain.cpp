/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#include "SamplePlugin.h"
#include "VolumeTerrain.h"

#include "OgreRay.h"

using namespace Ogre;
using namespace OgreBites;
using namespace Ogre::Volume;

void Sample_VolumeTerrain::setupContent(void)
{
    setupControls();
        
    // Skydome
    mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);

    // Light
    Light* directionalLight0 = mSceneMgr->createLight("directionalLight0");
    directionalLight0->setType(Light::LT_DIRECTIONAL);
    directionalLight0->setDirection(Vector3((Real)1, (Real)-1, (Real)1));
    directionalLight0->setDiffuseColour((Real)1, (Real)0.98, (Real)0.73);
    directionalLight0->setSpecularColour((Real)0.1, (Real)0.1, (Real)0.1);
   
    // Volume
    mVolumeRoot = OGRE_NEW Chunk();
    SceneNode *volumeRootNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("VolumeParent");
    mVolumeRoot->load(volumeRootNode, mSceneMgr, "volumeTerrain.cfg", &mSource);

    // Camera
    mCamera->setPosition((Real)(2560 - 384), (Real)2000, (Real)(2560 - 384));
    mCamera->lookAt((Real)0, (Real)100, (Real)0);
    mCamera->setNearClipDistance((Real)0.5);

}
    
//-----------------------------------------------------------------------

void Sample_VolumeTerrain::setupControls(void)
{
    mTrayMgr->showCursor();
#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
        setDragLook(true);
#endif
    mCameraMan->setTopSpeed((Real)100.0);
    // make room for the volume
    mTrayMgr->showLogo(TL_TOPRIGHT);
    mTrayMgr->showFrameStats(TL_TOPRIGHT);
    mTrayMgr->toggleAdvancedFrameStats();

    mTrayMgr->createTextBox(TL_TOPLEFT, "VolumeTerrainHelp", "Usage:\n\nHold the left mouse button,\n press wasd for movement\nand move the mouse for\n the direction.\nYou can place white spheres\nwith the middle mouse\nbutton like in an ego-shooter.", 265, 160);
}
    
//-----------------------------------------------------------------------

void Sample_VolumeTerrain::cleanupContent(void)
{   
    OGRE_DELETE mVolumeRoot;
    delete mSource;
    mVolumeRoot = 0;
}
    
//-----------------------------------------------------------------------

Sample_VolumeTerrain::Sample_VolumeTerrain(void) : mVolumeRoot(0), mHideAll(false)
{
    mInfo["Title"] = "Volume Terrain";
    mInfo["Description"] = "Demonstrates a volumetric terrain defined by an 3D texture and ray intersection with a volume.";
    mInfo["Thumbnail"] = "thumb_volumeterrain.png";
    mInfo["Category"] = "Geometry";
}
    
//-----------------------------------------------------------------------

bool Sample_VolumeTerrain::keyPressed(const OIS::KeyEvent& evt)
{
    if (evt.key == OIS::KC_F10)
    {
        mVolumeRoot->setVolumeVisible(!mVolumeRoot->getVolumeVisible());
    }
    if (evt.key == OIS::KC_F11)
    {
        mVolumeRoot->setOctreeVisible(!mVolumeRoot->getOctreeVisible());
    }
    if (evt.key == OIS::KC_F12)
    {
        mVolumeRoot->setDualGridVisible(!mVolumeRoot->getDualGridVisible());
    }
    if (evt.key == OIS::KC_H)
    {
        if (mHideAll)
        {
            mTrayMgr->showAll();
        }
        else
        {
            mTrayMgr->hideAll();
        }
        mHideAll = !mHideAll;
    }
    return SdkSample::keyPressed(evt);
}

//-----------------------------------------------------------------------

void Sample_VolumeTerrain::shootRay(Ray ray)
{
        Vector3 intersection;
        bool intersects = mSource->getFirstRayIntersection(ray, intersection);
        static int intersectionName = 0;
        if (intersects)
        {
            intersectionName++;
            SceneNode *n = mSceneMgr->getRootSceneNode()->createChildSceneNode();
            n->translate(intersection * mVolumeRoot->getScale());
            n->scale(Vector3((Real)0.05));
            Entity * mcSphere = mSceneMgr->createEntity("intersection" + StringConverter::toString(intersectionName), "sphere.mesh");
            n->attachObject(mcSphere);
        }
}

//-----------------------------------------------------------------------

#if (OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS) || (OGRE_PLATFORM == OGRE_PLATFORM_ANDROID)
bool Sample_VolumeTerrain::touchPressed(const OIS::MultiTouchEvent& evt)
{
    Ray ray(mCamera->getPosition() / mVolumeRoot->getScale(), -mCamera->getOrientation().zAxis());
    shootRay(ray);

    return SdkSample::touchPressed(evt);
}

#else

bool Sample_VolumeTerrain::mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
{
    if (id == OIS::MB_Middle)
    {
        Ray ray(mCamera->getPosition() / mVolumeRoot->getScale(), -mCamera->getOrientation().zAxis());
        shootRay(ray);
    }

    return SdkSample::mousePressed(evt, id);
}
#endif
//-----------------------------------------------------------------------

#ifndef OGRE_STATIC_LIB

SamplePlugin* sp;
Sample* s;
    
//-----------------------------------------------------------------------

extern "C" _OgreSampleExport void dllStartPlugin()
{
    s = new Sample_VolumeTerrain();
    sp = OGRE_NEW SamplePlugin(s->getInfo()["Title"] + " Sample");
    sp->addSample(s);
    Root::getSingleton().installPlugin(sp);
}
    
//-----------------------------------------------------------------------

extern "C" _OgreSampleExport void dllStopPlugin()
{
    Root::getSingleton().uninstallPlugin(sp); 
    OGRE_DELETE sp;
    delete s;
}

#endif
