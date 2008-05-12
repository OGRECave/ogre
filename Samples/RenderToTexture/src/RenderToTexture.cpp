/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/

/**
    \file 
        RenderToTexture.cpp
    \brief
        Shows OGRE's RenderToTexture feature, and using it to drive a reflection.
*/

#include "ExampleApplication.h"
#include "ExampleFrameListener.h"

class RenderToTextureFrameListener : public ExampleFrameListener
{
protected:
    Camera* mReflectCam;
    SceneNode* mPlaneNode;
public:
    RenderToTextureFrameListener(RenderWindow* window, Camera* maincam, Camera* reflectCam, 
        SceneNode* planeSceneNode)
        :ExampleFrameListener(window, maincam), 
        mReflectCam(reflectCam), mPlaneNode(planeSceneNode)
    {

    }
    bool frameRenderingQueued(const FrameEvent& evt)
    {
        if( ExampleFrameListener::frameRenderingQueued(evt) == false )
		return false;

        // Make sure reflection camera is updated too
        mReflectCam->setOrientation(mCamera->getOrientation());
        mReflectCam->setPosition(mCamera->getPosition());

        // Rotate plane
        mPlaneNode->yaw(Degree(30 * evt.timeSinceLastFrame), Node::TS_PARENT);

        return true;
    }
};

class RenderToTextureApplication : public ExampleApplication, public RenderTargetListener
{
public:
    RenderToTextureApplication() : mPlane(0) {}
    ~RenderToTextureApplication()
    {
        delete mPlane;
    }

protected:

    MovablePlane* mPlane;
    Entity* mPlaneEnt;
    Camera* mReflectCam;
    SceneNode* mPlaneNode;
    // render target events
    void preRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        // Hide plane 
        mPlaneEnt->setVisible(false);

    }
    void postRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        // Show plane 
        mPlaneEnt->setVisible(true);
    }

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));
        // Skybox
        mSceneMgr->setSkyBox(true, "Examples/MorningSkyBox");

        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        l->setType(Light::LT_DIRECTIONAL);
        Vector3 dir(0.5, -1, 0);
        dir.normalise();
        l->setDirection(dir);
        l->setDiffuseColour(1.0f, 1.0f, 0.8f);
        l->setSpecularColour(1.0f, 1.0f, 1.0f);


        // Create a prefab plane
        mPlane = new MovablePlane("ReflectPlane");
        mPlane->d = 0;
        mPlane->normal = Vector3::UNIT_Y;
        MeshManager::getSingleton().createPlane("ReflectionPlane", 
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
            *mPlane, 2000, 2000, 
            1, 1, true, 1, 1, 1, Vector3::UNIT_Z);
        mPlaneEnt = mSceneMgr->createEntity( "Plane", "ReflectionPlane" );

        // Create an entity from a model (will be loaded automatically)
        Entity* knotEnt = mSceneMgr->createEntity("Knot", "knot.mesh");

        // Create an entity from a model (will be loaded automatically)
        Entity* ogreHead = mSceneMgr->createEntity("Head", "ogrehead.mesh");

        knotEnt->setMaterialName("Examples/TextureEffect2");

        // Attach the rtt entity to the root of the scene
        SceneNode* rootNode = mSceneMgr->getRootSceneNode();
        mPlaneNode = rootNode->createChildSceneNode();

        // Attach both the plane entity, and the plane definition
        mPlaneNode->attachObject(mPlaneEnt);
        mPlaneNode->attachObject(mPlane);
        mPlaneNode->translate(0, -10, 0);
        // Tilt it a little to make it interesting
        mPlaneNode->roll(Degree(5));

        rootNode->createChildSceneNode( "Head" )->attachObject( ogreHead );

		TexturePtr texture = TextureManager::getSingleton().createManual( "RttTex", 
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, 
			512, 512, 0, PF_R8G8B8, TU_RENDERTARGET );
		RenderTarget *rttTex = texture->getBuffer()->getRenderTarget();
        {
            mReflectCam = mSceneMgr->createCamera("ReflectCam");
            mReflectCam->setNearClipDistance(mCamera->getNearClipDistance());
            mReflectCam->setFarClipDistance(mCamera->getFarClipDistance());
            mReflectCam->setAspectRatio(
                (Real)mWindow->getViewport(0)->getActualWidth() / 
                (Real)mWindow->getViewport(0)->getActualHeight());
			mReflectCam->setFOVy (mCamera->getFOVy());


            Viewport *v = rttTex->addViewport( mReflectCam );
            v->setClearEveryFrame( true );
            v->setBackgroundColour( ColourValue::Black );

            MaterialPtr mat = MaterialManager::getSingleton().create("RttMat",
                ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            TextureUnitState* t = mat->getTechnique(0)->getPass(0)->createTextureUnitState("RustedMetal.jpg");
            t = mat->getTechnique(0)->getPass(0)->createTextureUnitState("RttTex");
            // Blend with base texture
            t->setColourOperationEx(LBX_BLEND_MANUAL, LBS_TEXTURE, LBS_CURRENT, ColourValue::White, 
                ColourValue::White, 0.25);
			t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
			t->setProjectiveTexturing(true, mReflectCam);
            rttTex->addListener(this);

            // set up linked reflection
            mReflectCam->enableReflection(mPlane);
            // Also clip
            mReflectCam->enableCustomNearClipPlane(mPlane);
        }

        // Give the plane a texture
        mPlaneEnt->setMaterialName("RttMat");


        // Add a whole bunch of extra transparent entities
        Entity *cloneEnt;
        for (int n = 0; n < 10; ++n)
        {
            // Create a new node under the root
            SceneNode* node = mSceneMgr->createSceneNode();
            // Random translate
            Vector3 nodePos;
            nodePos.x = Math::SymmetricRandom() * 750.0;
            nodePos.y = Math::SymmetricRandom() * 100.0 + 25;
            nodePos.z = Math::SymmetricRandom() * 750.0;
            node->setPosition(nodePos);
            rootNode->addChild(node);
            // Clone knot
            char cloneName[12];
            sprintf(cloneName, "Knot%d", n);
            cloneEnt = knotEnt->clone(cloneName);
            // Attach to new node
            node->attachObject(cloneEnt);

        }

        mCamera->setPosition(-50, 100, 500);
        mCamera->lookAt(0,0,0);
    }

    void createFrameListener(void)
    {
        mFrameListener= new RenderToTextureFrameListener(mWindow, mCamera, mReflectCam, mPlaneNode);
        mRoot->addFrameListener(mFrameListener);

    }

};

#ifdef __cplusplus
extern "C" {
#endif

int main(int argc, char ** argv)
{

    // Create application object
    RenderToTextureApplication app;

    app.go();

    return 0;
}

#ifdef __cplusplus
}
#endif
