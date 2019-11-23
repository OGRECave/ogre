/*-------------------------------------------------------------------------
This source file is a part of OGRE
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
THE SOFTWARE
-------------------------------------------------------------------------*/

//! [starter]

#include <exception>
#include <iostream>

#include "Ogre.h"
#include "OgreApplicationContext.h"
#include "OgreInput.h"
#include "OgreRTShaderSystem.h"
#include "OgreCameraMan.h"

using namespace Ogre;
using namespace OgreBites;

class TutorialApplication
        : public ApplicationContext
        , public InputListener
{
public:
    TutorialApplication();
    virtual ~TutorialApplication();

    void setup();
    bool keyPressed(const KeyboardEvent& evt);
};


TutorialApplication::TutorialApplication()
    : ApplicationContext("OgreTutorialApp")
{
}


TutorialApplication::~TutorialApplication()
{
}


void TutorialApplication::setup()
{
    // do not forget to call the base first
    ApplicationContext::setup();
    addInputListener(this);

    // get a pointer to the already created root
    Root* root = getRoot();
    SceneManager* scnMgr = root->createSceneManager();

    // register our scene with the RTSS
    RTShader::ShaderGenerator* shadergen = RTShader::ShaderGenerator::getSingletonPtr();
    shadergen->addSceneManager(scnMgr);

    // -- tutorial section start --
    //! [cameracreate]
    SceneNode* camNode = scnMgr->getRootSceneNode()->createChildSceneNode();
    Camera* cam = scnMgr->createCamera("myCam");
    //! [cameracreate]

    //! [cameraposition]
    camNode->setPosition(200, 300, 400);
    camNode->lookAt(Vector3(0, 0, 0), Node::TransformSpace::TS_WORLD);
    //! [cameraposition]

    //! [cameralaststep]
    cam->setNearClipDistance(5);
    camNode->attachObject(cam);
    //! [cameralaststep]

    //! [addviewport]
    Viewport* vp = getRenderWindow()->addViewport(cam);
    //! [addviewport]

    //! [viewportback]
    vp->setBackgroundColour(ColourValue(0, 0, 0));
    //! [viewportback]

    //! [cameraratio]
    cam->setAspectRatio(Real(vp->getActualWidth()) / Real(vp->getActualHeight()));
    //! [cameraratio]

    //! [lightingsset]
    scnMgr->setAmbientLight(ColourValue(0, 0, 0));
    scnMgr->setShadowTechnique(ShadowTechnique::SHADOWTYPE_STENCIL_ADDITIVE);
    //! [lightingsset]

    //! [ninja]
    Entity* ninjaEntity = scnMgr->createEntity("ninja.mesh");
    ninjaEntity->setCastShadows(true);

    scnMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ninjaEntity);
    //! [ninja]

    //! [plane]
    Plane plane(Vector3::UNIT_Y, 0);
    //! [plane]

    //! [planedefine]
    MeshManager::getSingleton().createPlane(
            "ground", RGN_DEFAULT,
            plane,
            1500, 1500, 20, 20,
            true,
            1, 5, 5,
            Vector3::UNIT_Z);
    //! [planedefine]

    //! [planecreate]
    Entity* groundEntity = scnMgr->createEntity("ground");
    scnMgr->getRootSceneNode()->createChildSceneNode()->attachObject(groundEntity);
    //! [planecreate]

    //! [planenoshadow]
    groundEntity->setCastShadows(false);
    //! [planenoshadow]

    //! [planesetmat]
    groundEntity->setMaterialName("Examples/Rockwall");
    //! [planesetmat]

    //! [spotlight]
    Light* spotLight = scnMgr->createLight("SpotLight");
    //! [spotlight]

    //! [spotlightcolor]
    spotLight->setDiffuseColour(0, 0, 1.0);
    spotLight->setSpecularColour(0, 0, 1.0);
    //! [spotlightcolor]

    //! [spotlighttype]
    spotLight->setType(Light::LT_SPOTLIGHT);
    //! [spotlighttype]

    //! [spotlightposrot]
    SceneNode* spotLightNode = scnMgr->getRootSceneNode()->createChildSceneNode();
    spotLightNode->attachObject(spotLight);
    spotLightNode->setDirection(-1, -1, 0);
    spotLightNode->setPosition(Vector3(200, 200, 0));
    //! [spotlightposrot]

    //! [spotlightrange]
    spotLight->setSpotlightRange(Degree(35), Degree(50));
    //! [spotlightrange]

    //! [directlight]
    Light* directionalLight = scnMgr->createLight("DirectionalLight");
    directionalLight->setType(Light::LT_DIRECTIONAL);
    //! [directlight]

    //! [directlightcolor]
    directionalLight->setDiffuseColour(ColourValue(0.4, 0, 0));
    directionalLight->setSpecularColour(ColourValue(0.4, 0, 0));
    //! [directlightcolor]

    //! [directlightdir]
    SceneNode* directionalLightNode = scnMgr->getRootSceneNode()->createChildSceneNode();
    directionalLightNode->attachObject(directionalLight);
    directionalLightNode->setDirection(Vector3(0, -1, 1));
    //! [directlightdir]

    //! [pointlight]
    Light* pointLight = scnMgr->createLight("PointLight");
    pointLight->setType(Light::LT_POINT);
    //! [pointlight]

    //! [pointlightcolor]
    pointLight->setDiffuseColour(0.3, 0.3, 0.3);
    pointLight->setSpecularColour(0.3, 0.3, 0.3);
    //! [pointlightcolor]

    //! [pointlightpos]
    SceneNode* pointLightNode = scnMgr->getRootSceneNode()->createChildSceneNode();
    pointLightNode->attachObject(pointLight);
    pointLightNode->setPosition(Vector3(0, 150, 250));
    //! [pointlightpos]
    // -- tutorial section end --
}


bool TutorialApplication::keyPressed(const KeyboardEvent& evt)
{
    if (evt.keysym.sym == SDLK_ESCAPE)
    {
        getRoot()->queueEndRendering();
    }
    return true;
}


int main(int argc, char **argv)
{
    try
    {
        TutorialApplication app;
        app.initApp();
        app.getRoot()->startRendering();
        app.closeApp();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error occurred during execution: " << e.what() << '\n';
        return 1;
    }

    return 0;
}

//! [starter]
