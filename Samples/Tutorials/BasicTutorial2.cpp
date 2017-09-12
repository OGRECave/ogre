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

#include <Ogre.h>
#include <OgreApplicationContext.h>
#include <OgreInput.h>
#include <OgreRTShaderSystem.h>
#include <OgreApplicationContext.h>
#include <OgreCameraMan.h>

class TutorialApplication
        : public OgreBites::ApplicationContext
        , public OgreBites::InputListener
{
public:
    TutorialApplication();
    virtual ~TutorialApplication();

    void setup();
    bool keyPressed(const OgreBites::KeyboardEvent& evt);
};


TutorialApplication::TutorialApplication()
    : OgreBites::ApplicationContext("Tutorial Application")
{
    addInputListener(this);
}


TutorialApplication::~TutorialApplication()
{
}


void TutorialApplication::setup()
{
    // do not forget to call the base first
    OgreBites::ApplicationContext::setup();

    // get a pointer to the already created root
    Ogre::Root* root = getRoot();
    Ogre::SceneManager* scnMgr = root->createSceneManager(Ogre::ST_GENERIC);

    // register our scene with the RTSS
    Ogre::RTShader::ShaderGenerator* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
    shadergen->addSceneManager(scnMgr);

    // -- tutorial section start --
    //! [cameracreate]
    Ogre::SceneNode* camNode = scnMgr->getRootSceneNode()->createChildSceneNode();
    Ogre::Camera* cam = scnMgr->createCamera("myCam");
    //! [cameracreate]

    //! [cameraposition]
    camNode->setPosition(200, 300, 400);
    camNode->lookAt(Ogre::Vector3(0, 0, 0), Ogre::Node::TransformSpace::TS_WORLD);
    //! [cameraposition]

    //! [cameralaststep]
    cam->setNearClipDistance(5);
    camNode->attachObject(cam);
    //! [cameralaststep]

    //! [addviewport]
    Ogre::Viewport* vp = mWindow->addViewport(cam);
    //! [addviewport]

    //! [viewportback]
    vp->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
    //! [viewportback]

    //! [cameraratio]
    cam->setAspectRatio(
      Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight())
    );
    //! [cameraratio]

    //! [ninja]
    Ogre::Entity* ninjaEntity = scnMgr->createEntity("ninja.mesh");
    ninjaEntity->setCastShadows(true);

    scnMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ninjaEntity);
    //! [ninja]

    //! [plane]
    Ogre::Plane plane(Ogre::Vector3::UNIT_Y, 0);
    //! [plane]

    //! [planedefine]
    Ogre::MeshManager::getSingleton().createPlane(
            "ground",
            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            plane,
            1500, 1500, 20, 20,
            true,
            1, 5, 5,
            Ogre::Vector3::UNIT_Z);
    //! [planedefine]

    //! [planecreate]
    Ogre::Entity* groundEntity = scnMgr->createEntity("ground");
    scnMgr->getRootSceneNode()->createChildSceneNode()->attachObject(groundEntity);
    //! [planecreate]

    //! [planenoshadow]
    groundEntity->setCastShadows(false);
    //! [planenoshadow]

    //! [planesetmat]
    groundEntity->setMaterialName("Examples/Rockwall");
    //! [planesetmat]

    //! [lightingsset]
    scnMgr->setAmbientLight(Ogre::ColourValue(0, 0, 0));
    scnMgr->setShadowTechnique(Ogre::ShadowTechnique::SHADOWTYPE_STENCIL_MODULATIVE);
    //! [lightingsset]

    //! [spotlight]
    Ogre::Light* spotLight = scnMgr->createLight("SpotLight");
    //! [spotlight]

    //! [spotlightcolor]
    spotLight->setDiffuseColour(0, 0, 1.0);
    spotLight->setSpecularColour(0, 0, 1.0);
    //! [spotlightcolor]

    //! [spotlighttype]
    spotLight->setType(Ogre::Light::LT_SPOTLIGHT);
    //! [spotlighttype]

    //! [spotlightposrot]
    spotLight->setDirection(-1, -1, 0);
    spotLight->setPosition(Ogre::Vector3(200, 200, 0));
    //! [spotlightposrot]

    //! [spotlightrange]
    spotLight->setSpotlightRange(Ogre::Degree(35), Ogre::Degree(50));
    //! [spotlightrange]

    //! [directlight]
    Ogre::Light* directionalLight = scnMgr->createLight("DirectionalLight");
    directionalLight->setType(Ogre::Light::LT_DIRECTIONAL);
    //! [directlight]

    //! [directlightcolor]
    directionalLight->setDiffuseColour(Ogre::ColourValue(.4, 0, 0));
    directionalLight->setSpecularColour(Ogre::ColourValue(.4, 0, 0));
    //! [directlightcolor]

    //! [directlightdir]
    directionalLight->setDirection(Ogre::Vector3(0, -1, 1));
    //! [directlightdir]

    //! [pointlight]
    Ogre::Light* pointLight = scnMgr->createLight("PointLight");
    pointLight->setType(Ogre::Light::LT_POINT);
    //! [pointlight]

    //! [pointlightcolor]
    pointLight->setDiffuseColour(.3, .3, .3);
    pointLight->setSpecularColour(.3, .3, .3);
    //! [pointlightcolor]

    //! [pointlightpos]
    pointLight->setPosition(Ogre::Vector3(0, 150, 250));
    //! [pointlightpos]
    // -- tutorial section end --
}


bool TutorialApplication::keyPressed(const OgreBites::KeyboardEvent& evt)
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
