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

/*
-----------------------------------------------------------------------------
Filename:    BspCollision.cpp
Description: Somewhere to play in the sand...
-----------------------------------------------------------------------------
*/

#include "OgreReferenceAppLayer.h"

#include "ExampleRefAppApplication.h"
#include "OgreStringConverter.h"

// Hacky globals
ApplicationObject *ball;

SceneNode* targetNode;
RaySceneQuery* rsq = 0;


// Event handler to add ability to alter curvature
class BspCollisionListener : public ExampleRefAppFrameListener
{
protected:
public:
    BspCollisionListener(RenderWindow* win, CollideCamera* cam)
        : ExampleRefAppFrameListener(win, cam)
    {
    }


    bool frameEnded(const FrameEvent& evt)
    {
        // local just to stop toggles flipping too fast
        static Real timeUntilNextToggle = 0;

        // Deal with time delays that are too large
        // If we exceed this limit, we ignore
        static const Real MAX_TIME_INCREMENT = 0.5f;
        if (evt.timeSinceLastEvent > MAX_TIME_INCREMENT)
        {
            return true;
        }
        
        if (timeUntilNextToggle >= 0) 
            timeUntilNextToggle -= evt.timeSinceLastFrame;

        // Call superclass
        bool ret = ExampleRefAppFrameListener::frameEnded(evt);        

		if (mKeyboard->isKeyDown(OIS::KC_SPACE) && timeUntilNextToggle <= 0)
        {
            timeUntilNextToggle = 2;
            ball->setPosition(mCamera->getPosition() + 
                mCamera->getDirection() * mCamera->getNearClipDistance() * 2);
            ball->setLinearVelocity(mCamera->getDirection() * 200);
            ball->setAngularVelocity(Vector3::ZERO);
        }

        // Move the targeter
        rsq->setRay(mCamera->getRealCamera()->getCameraToViewportRay(0.5, 0.5));
        RaySceneQueryResult& rsqResult = rsq->execute();
        RaySceneQueryResult::iterator ri = rsqResult.begin();
        if (ri != rsqResult.end())
        {
            RaySceneQueryResultEntry& res = *ri;
            targetNode->setPosition(rsq->getRay().getPoint(res.distance));

        }
        return ret;
    }
};

class BspCollisionApplication : public ExampleRefAppApplication
{
public:
    BspCollisionApplication() {
    
    }

    ~BspCollisionApplication() 
    {
		delete rsq;
    }

protected:
    
    void chooseSceneManager(void)
    {
        mSceneMgr = mRoot->createSceneManager("BspSceneManager");
    }
    void createWorld(void)
    {
        // Create BSP-specific world
        mWorld = new World(mSceneMgr, World::WT_REFAPP_BSP);
    }
    void createScene(void)
    {
        mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_MODULATIVE);
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));
        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        l->setPosition(-100,50,100);
        l->setAttenuation(8000,1,0,0);


        // Setup World
        mWorld->setGravity(Vector3(0, 0, -60));
        mWorld->getSceneManager()->setWorldGeometry("ogretestmap.bsp");

        // modify camera for close work
        mCamera->setNearClipDistance(10);
        mCamera->setFarClipDistance(20000);

        // Also change position, and set Quake-type orientation
        // Get random player start point
        ViewPoint vp = mSceneMgr->getSuggestedViewpoint(true);
        mCamera->setPosition(vp.position);
        mCamera->pitch(Degree(90)); // Quake uses X/Y horizon, Z up
        mCamera->rotate(vp.orientation);
        // Don't yaw along variable axis, causes leaning
        mCamera->setFixedYawAxis(true, Vector3::UNIT_Z);
        // Look at the boxes
		mCamera->lookAt(-150,40,30);

        ball = mWorld->createBall("ball", 7, vp.position + Vector3(0,0,80));
        ball->setDynamicsEnabled(true);
        ball->getEntity()->setMaterialName("Ogre/Eyes");

		OgreRefApp::Box* box = mWorld->createBox("shelf", 75, 125, 5, Vector3(-150, 40, 30));
        box->getEntity()->setMaterialName("Examples/Rocky");

        static const Real BOX_SIZE = 15.0f;
        static const int num_rows = 3;

        for (int row = 0; row < num_rows; ++row)
        {
            for (int i = 0; i < (num_rows-row); ++i)
            {
                Real row_size = (num_rows - row) * BOX_SIZE * 1.25;
                String name = "box";
                name += StringConverter::toString((row*num_rows) + i);
                box = mWorld->createBox(name, BOX_SIZE,BOX_SIZE,BOX_SIZE , 
                    Vector3(-150, 
                        40 - (row_size * 0.5) + (i * BOX_SIZE * 1.25) , 
                        32.5 + (BOX_SIZE / 2) + (row * BOX_SIZE)));
                box->setDynamicsEnabled(false, true);
                box->getEntity()->setMaterialName("Examples/10PointBlock");
            }
        }
        mCamera->setCollisionEnabled(false);
        mCamera->getRealCamera()->setQueryFlags(0);

        // Create the targeting sphere
        Entity* targetEnt = mSceneMgr->createEntity("testray", "sphere.mesh");
        MaterialPtr mat = MaterialManager::getSingleton().create("targeter", 
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        Pass* pass = mat->getTechnique(0)->getPass(0);
        TextureUnitState* tex = pass->createTextureUnitState();
        tex->setColourOperationEx(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, 
            ColourValue::Red);
        pass->setLightingEnabled(false);
        pass->setSceneBlending(SBT_ADD);
        pass->setDepthWriteEnabled(false);


        targetEnt->setMaterialName("targeter");
        targetEnt->setCastShadows(false);
        targetEnt->setQueryFlags(0);
        targetNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        targetNode->scale(0.025, 0.025, 0.025);
        targetNode->attachObject(targetEnt);

        rsq = mSceneMgr->createRayQuery(Ray());
        rsq->setSortByDistance(true, 1);
        rsq->setWorldFragmentType(SceneQuery::WFT_SINGLE_INTERSECTION);
    }
    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new BspCollisionListener(mWindow, mCamera);
        mRoot->addFrameListener(mFrameListener);
    }

public:

};



#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"


INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char **argv)
#endif
{
    // Create application object
    BspCollisionApplication app;

    try {
        app.go();
    } catch( Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: " << e.getFullDescription();
#endif
    }


    return 0;
}







