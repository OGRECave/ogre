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
#include "VolumeCSG.h"

#include "OgreVolumeCSGSource.h"
#include "OgreVolumeCacheSource.h"
#include "OgreVolumeTextureSource.h"
#include "OgreVolumeMeshBuilder.h"
#include "OgreMath.h"

using namespace Ogre;
using namespace OgreBites;
using namespace Ogre::Volume;

void Sample_VolumeCSG::setupContent(void)
{
    setupControls();
    setupShaderGenerator();
    Real size = (Real)31.0;
    Vector3 to(size);
            
    // Light
    Light* directionalLight0 = mSceneMgr->createLight("directionalLight0");
    directionalLight0->setType(Light::LT_DIRECTIONAL);
    directionalLight0->setDiffuseColour((Real)1, (Real)0.98, (Real)0.73);
    directionalLight0->setSpecularColour((Real)0.1, (Real)0.1, (Real)0.1);

    auto ln = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    ln->attachObject(directionalLight0);
    ln->setDirection(Vector3(1, -1, 1));
   
    // Spheres
    CSGSphereSource sphere1((Real)5.0, Vector3((Real)5.5));
    CSGSphereSource sphere2((Real)5.0, Vector3((Real)25.5, (Real)5.5, (Real)5.5));
    CSGSphereSource sphere3((Real)5.0, Vector3((Real)25.5, (Real)5.5, (Real)25.5));
    CSGSphereSource sphere4((Real)5.0, Vector3((Real)5.5, (Real)5.5, (Real)25.5));

    // Cubes
    Real halfWidth = (Real)(2.5 / 2.0);
    CSGCubeSource cube1(Vector3((Real)5.5 - halfWidth), Vector3((Real)25.5 + halfWidth, (Real)5.5 + halfWidth, (Real)25.5 + halfWidth));
    CSGCubeSource cube2(Vector3((Real)5.5 + halfWidth, (Real)0.0, (Real)5.5 + halfWidth), Vector3((Real)25.5 - halfWidth, to.y, (Real)25.5 - halfWidth));
    CSGDifferenceSource difference1(&cube1, &cube2);

    // Inner rounded cube
    Real innerHalfWidth = (Real)(7.0 / 2.0);
    Vector3 center((Real)15.5, (Real)5.5, (Real)15.5);
    CSGCubeSource cube3(center - innerHalfWidth, center + innerHalfWidth);
    CSGSphereSource sphere5(innerHalfWidth + (Real)0.75, center);
    CSGIntersectionSource intersection1(&cube3, &sphere5);

    // A plane with noise
    CSGPlaneSource plane1((Real)1.0, Vector3::UNIT_Y);
    Real frequencies[] = {(Real)1.01, (Real)0.48};
    Real amplitudes[] = {(Real)0.25, (Real)0.5};
    CSGNoiseSource noise1(&plane1, frequencies, amplitudes, 2, 100);

    // Combine everything
    CSGUnionSource union1(&sphere1, &sphere2);
    CSGUnionSource union2(&union1, &sphere3);
    CSGUnionSource union3(&union2, &sphere4);
    CSGUnionSource union4(&union3, &difference1);
    CSGUnionSource union5(&union4, &intersection1);
    CSGUnionSource union6(&union5, &noise1);
    Source *src = &union6;
    
    mVolumeRoot = OGRE_NEW Chunk();
    SceneNode *volumeRootNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("VolumeParent");

    
    ChunkParameters parameters;
    parameters.sceneManager = mSceneMgr;
    parameters.src = src;
    parameters.baseError = (Real)0.25;

    mVolumeRoot->load(volumeRootNode, Vector3::ZERO, to, 1, &parameters);

    MaterialPtr mat = MaterialManager::getSingleton().getByName("Ogre/RTShader/TriplanarTexturing", "General");
    mVolumeRoot->setMaterial(mat);

    // Camera
    mCameraNode->setPosition(to + (Real)7.5);
    mCameraNode->lookAt(center + (Real)11.0, Node::TS_PARENT);
    mCamera->setNearClipDistance((Real)0.5);

    mRotation = (Real)0.0;

}
    
//-----------------------------------------------------------------------

void Sample_VolumeCSG::setupControls(void)
{
    mTrayMgr->showCursor();
    mCameraMan->setStyle(OgreBites::CS_MANUAL);
    mCameraMan->setTopSpeed((Real)25.0);
    // make room for the volume
    mTrayMgr->showLogo(TL_TOPRIGHT);
    mTrayMgr->showFrameStats(TL_TOPRIGHT);
    mTrayMgr->toggleAdvancedFrameStats();
}
    
//-----------------------------------------------------------------------

void Sample_VolumeCSG::setupShaderGenerator()
{
    RTShader::ShaderGenerator* mGen = RTShader::ShaderGenerator::getSingletonPtr();
        
    RTShader::RenderState* pMainRenderState = 
        mGen->createOrRetrieveRenderState(RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME).first;
    pMainRenderState->reset();
            
    mGen->invalidateScheme(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);

    // Make this viewport work with shader generator scheme.
    mViewport->setMaterialScheme(RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
}
    
//-----------------------------------------------------------------------

void Sample_VolumeCSG::cleanupContent(void)
{   
    OGRE_DELETE mVolumeRoot;
    mVolumeRoot = 0;
}
    
//-----------------------------------------------------------------------

Sample_VolumeCSG::Sample_VolumeCSG(void) : mVolumeRoot(0), mHideAll(false)
{
    mInfo["Title"] = "Volume CSG";
    mInfo["Description"] = "Demonstrates a volumetric constructive solid geometry scene, showing sphere, cube, plane, union, difference and intersection. The triplanar texturing is generated by the RTSS.";
    mInfo["Thumbnail"] = "thumb_volumecsg.png";
    mInfo["Category"] = "Geometry";
}
    
//-----------------------------------------------------------------------

bool Sample_VolumeCSG::keyPressed(const KeyboardEvent& evt)
{
	Keycode key = evt.keysym.sym;
    if (key == 'h')
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
    
bool Sample_VolumeCSG::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    Vector3 center((Real)15.5, (Real)5.5, (Real)15.5);
    mRotation += Radian(evt.timeSinceLastFrame * (Real)0.5);
    Real r = (Real)35.0;
    mCameraNode->setPosition(
        Math::Sin(mRotation) * r + center.x,
        (Real)15.0 + center.y,
        Math::Cos(mRotation) * r + center.z
    );
    mCameraNode->lookAt(center, Node::TS_PARENT);
    return SdkSample::frameRenderingQueued(evt);
}

//-----------------------------------------------------------------------

void Sample_VolumeCSG::_shutdown()
{
    RTShader::RenderState* pMainRenderState = 
        RTShader::ShaderGenerator::getSingleton().createOrRetrieveRenderState(RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME).first;
    pMainRenderState->reset();
        
    SdkSample::_shutdown();
}
