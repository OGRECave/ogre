/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#include "SamplePlugin.h"
#include "VolumeTerrain.h"

#include "OgreVolumeTextureSource.h"
#include "OgreVolumeCSGSource.h"

#include "OgreRay.h"

using namespace Ogre;
using namespace OgreBites;
using namespace Ogre::Volume;

const Real Sample_VolumeTerrain::MOUSE_MODIFIER_TIME_LIMIT = (Real)0.033333;

void Sample_VolumeTerrain::setupContent(void)
{
    setupControls();

    // Skydome
    mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);

    // Light
    Light* directionalLight0 = mSceneMgr->createLight("directionalLight0");
    directionalLight0->setType(Light::LT_DIRECTIONAL);
    directionalLight0->setDiffuseColour((Real)1, (Real)0.98, (Real)0.73);
    directionalLight0->setSpecularColour((Real)0.1, (Real)0.1, (Real)0.1);
    SceneNode* ln = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    ln->setDirection(1, -1, 1);
    ln->attachObject(directionalLight0);

    mSceneMgr->setAmbientLight(Ogre::ColourValue::White);
   
    // Volume
    mVolumeRoot = OGRE_NEW Chunk();
    mVolumeRootNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("VolumeParent");
    Timer t;
    mVolumeRoot->load(mVolumeRootNode, mSceneMgr, "volumeTerrain.cfg", true);
    LogManager::getSingleton().stream() << "Loaded volume terrain in " << t.getMillisecondsCPU() << " ms";

    // Make this viewport work with shader generator scheme.
    mViewport->setMaterialScheme(MSN_SHADERGEN);

    // Camera
    mCameraNode->setPosition((Real)3264, (Real)2700, (Real)3264);
    mCameraNode->lookAt(Vector3::ZERO, Node::TS_PARENT);
    mCamera->setNearClipDistance((Real)0.5);

}
    
//-----------------------------------------------------------------------

void Sample_VolumeTerrain::setupControls(void)
{
    mMouseState = 0;
    mTrayMgr->showCursor();
    setDragLook(true);
    mCameraMan->setTopSpeed((Real)100.0);
    // make room for the volume
    mTrayMgr->showLogo(TL_TOPRIGHT);
    mTrayMgr->showFrameStats(TL_TOPRIGHT);
    mTrayMgr->toggleAdvancedFrameStats();

    mTrayMgr->createTextBox(TL_TOPLEFT, "VolumeTerrainHelp", "Usage:\n\nHold the left mouse button, press\nwasd for movement and move the\nmouse for the direction.\nYou can add spheres with the\nmiddle mouse button and remove\nspheres with the right one.", 310, 150);
}

//-----------------------------------------------------------------------

void Sample_VolumeTerrain::cleanupContent(void)
{   
    delete mVolumeRoot->getChunkParameters()->src;
    OGRE_DELETE mVolumeRoot;
    mVolumeRoot = 0;
}
    
//-----------------------------------------------------------------------

Sample_VolumeTerrain::Sample_VolumeTerrain(void) : mVolumeRoot(0), mHideAll(false)
{
    mInfo["Title"] = "Volume Terrain";
    mInfo["Description"] = "Demonstrates a volumetric terrain defined by an 3D texture and manipulation of the volume. The middle mouse button adds a sphere, a rightclick removes one.";
    mInfo["Thumbnail"] = "thumb_volumeterrain.png";
    mInfo["Category"] = "Geometry";
}
    
//-----------------------------------------------------------------------

bool Sample_VolumeTerrain::keyPressed(const KeyboardEvent& evt)
{
    switch(evt.keysym.sym)
    {
    case SDLK_F10:
        mVolumeRoot->setVolumeVisible(!mVolumeRoot->getVolumeVisible());
        break;
    case SDLK_F11:
        mVolumeRoot->setOctreeVisible(!mVolumeRoot->getOctreeVisible());
        break;
    case SDLK_F12:
        mVolumeRoot->setDualGridVisible(!mVolumeRoot->getDualGridVisible());
        break;
    case 'h':
        if (mHideAll)
        {
            mTrayMgr->showAll();
        }
        else
        {
            mTrayMgr->hideAll();
        }
        mHideAll = !mHideAll;
        break;
    default:
        break;
    }
    return SdkSample::keyPressed(evt);
}

//-----------------------------------------------------------------------

void Sample_VolumeTerrain::shootRay(Ray ray, bool doUnion)
{
    Vector3 intersection;
    Real scale = mVolumeRoot->getChunkParameters()->scale;
    bool intersects = mVolumeRoot->getChunkParameters()->src->getFirstRayIntersection(ray, intersection, scale);
    if (intersects)
    {
        Real radius = (Real)2.5;
        CSGSphereSource sphere(radius, intersection);
        CSGOperationSource *operation = doUnion ? static_cast<CSGOperationSource*>(new CSGUnionSource()) : new CSGDifferenceSource();
        static_cast<TextureSource*>(mVolumeRoot->getChunkParameters()->src)->combineWithSource(operation, &sphere, intersection, radius * (Real)1.5);
        
        mVolumeRoot->getChunkParameters()->updateFrom = intersection - radius * (Real)1.5;
        mVolumeRoot->getChunkParameters()->updateTo = intersection + radius * (Real)1.5;
        mVolumeRoot->load(mVolumeRootNode, Vector3::ZERO, Vector3(384), 5, mVolumeRoot->getChunkParameters());
        delete operation;
    }
}

//-----------------------------------------------------------------------

bool Sample_VolumeTerrain::touchPressed(const TouchFingerEvent& evt)
{
    Ray ray = mCamera->getCameraToViewportRay(evt.x, evt.y);
    shootRay(ray, true);

    return SdkSample::touchPressed(evt);
}

//-----------------------------------------------------------------------

bool Sample_VolumeTerrain::mousePressed(const MouseButtonEvent& evt)
{
    if (mMouseState == 0)
    {
        if (evt.button == BUTTON_MIDDLE)
        {
            mMouseState = 1;
            mMouseCountdown = MOUSE_MODIFIER_TIME_LIMIT;
        }
        if (evt.button == BUTTON_RIGHT)
        {
            mMouseState = 2;
            mMouseCountdown = MOUSE_MODIFIER_TIME_LIMIT;
        }
    }

    return SdkSample::mousePressed(evt);
}

//-----------------------------------------------------------------------

bool Sample_VolumeTerrain::mouseReleased(const MouseButtonEvent& evt)
{
    if (evt.button == BUTTON_MIDDLE || evt.button == BUTTON_RIGHT)
    {
        mMouseState = 0;
    }

    return SdkSample::mouseReleased(evt);
}

//-----------------------------------------------------------------------

bool Sample_VolumeTerrain::mouseMoved(const MouseMotionEvent& evt)
{
    int width = mWindow->getWidth(), height = mWindow->getHeight();
    
    mMouseX = (Real)evt.x / (Real)width;
    mMouseY = (Real)evt.y / (Real)height;
    return SdkSample::mouseMoved(evt);
}

//-----------------------------------------------------------------------

bool Sample_VolumeTerrain::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    if (mMouseState != 0)
    {
        mMouseCountdown -= evt.timeSinceLastEvent;
        if (mMouseCountdown <= (Real)0.0)
        {
            mMouseCountdown = MOUSE_MODIFIER_TIME_LIMIT;
            Ray ray = mCamera->getCameraToViewportRay(mMouseX, mMouseY);
            shootRay(ray, mMouseState == 1);
        }
    }
    return SdkSample::frameRenderingQueued(evt);
}
