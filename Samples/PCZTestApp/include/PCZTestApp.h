/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/

#include <SdkSample.h>

#include <OgrePortal.h>
#include <OgrePCZone.h>
#include <OgreAntiPortal.h>
#include <OgrePCZSceneQuery.h>

#include "RoomObject.h"

using namespace Ogre;
using namespace OgreBites;

class Sample_PCZTest : public SdkSample
{
    PCZSceneNode * buildingNode;
    Vector3 buildingTranslate;
    RaySceneQuery* raySceneQuery;
    MovableObject* targetMO;
public:
    Sample_PCZTest() : raySceneQuery(0), targetMO(0)
    {
        mInfo["Title"] = "PCZTest";
        mInfo["Description"] = "Demonstrates use of the Portal Connected Zone(PCZ) Scene Manager Plugin.";
        mInfo["Thumbnail"] = "thumb_pcz.png";
        mInfo["Category"] = "Environment";
        mInfo["Help"] = "Note that there is no collision detection and transitioning from zone to zone "
                        "will only work correctly if the user moves the camera through the doorways (as "
                        "opposed to going through the walls).";
    }

    void cleanupContent()
    {
        delete raySceneQuery;
    }

    void createSceneManager(void)
    {
        // Create the SceneManager, in this case a generic one
        mSceneMgr = mRoot->createSceneManager("PCZSceneManager");
        // initialize the scene manager using terrain as default zone
        String zoneTypeName = "ZoneType_Default";
        ((PCZSceneManager*)mSceneMgr)->init(zoneTypeName);

#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
        mShaderGenerator->addSceneManager(mSceneMgr);
#endif

        if(mOverlaySystem)
            mSceneMgr->addRenderQueueListener(mOverlaySystem);
    }

    // utility function to create terrain zones easily
    PCZone * createTerrainZone(String & zoneName, String & terrain_cfg)
    {
        // load terrain into the terrain zone
        PCZone * terrainZone = ((PCZSceneManager*)mSceneMgr)->createZone(String("ZoneType_Terrain"), zoneName);
        terrainZone->notifyCameraCreated(mCamera);
        ((PCZSceneManager*)mSceneMgr)->setZoneGeometry( zoneName, (PCZSceneNode*)mSceneMgr->getRootSceneNode(), terrain_cfg );

        // create aab portal(s) around the terrain
        String portalName;
        Vector3 corners[2];
        AxisAlignedBox aabb;

        // make portal from terrain to default
        Portal * p;
        terrainZone->getAABB(aabb);
        portalName = Ogre::String("PortalFrom"+zoneName+"ToDefault_Zone");
        p = ((PCZSceneManager*)mSceneMgr)->createPortal(portalName, Ogre::Portal::PORTAL_TYPE_AABB);
        corners[0] = aabb.getMinimum();
        corners[1] = aabb.getMaximum();
        p->setCorner(0, corners[0]);
        p->setCorner(1, corners[1]);
        p->setDirection(Ogre::Vector3::NEGATIVE_UNIT_Z); // this indicates an "inward" pointing normal
        // associate the portal with the terrain's main node
        p->setNode(terrainZone->getEnclosureNode());
        // IMPORTANT: Update the derived values of the portal
        p->updateDerivedValues();
        // add the portal to the zone
        terrainZone->_addPortal(p);
    
        // make portal from default to terrain
        portalName = Ogre::String("PortalFromDefault_ZoneTo"+zoneName);
        Portal * p2;
        p2 = ((PCZSceneManager*)mSceneMgr)->createPortal(portalName, Ogre::Portal::PORTAL_TYPE_AABB);
        corners[0] = aabb.getMinimum();
        corners[1] = aabb.getMaximum();
        p2->setCorner(0, corners[0]);
        p2->setCorner(1, corners[1]);
        p2->setDirection(Ogre::Vector3::UNIT_Z); // this indicates an "outward" pointing normal
        // associate the portal with the terrain's main node
        p2->setNode(terrainZone->getEnclosureNode());
        // IMPORTANT: Update the derived values of the portal
        p2->updateDerivedValues();
        // add the portal to the zone
        ((PCZSceneManager*)mSceneMgr)->getDefaultZone()->_addPortal(p2);

        // connect the portals manually
        p->setTargetPortal(p2);
        p2->setTargetPortal(p);
        p->setTargetZone(((PCZSceneManager*)mSceneMgr)->getDefaultZone());
        p2->setTargetZone(terrainZone);

        return terrainZone;
    }

    PCZSceneNode* createAntiPortal(const String& name)
    {
        // Create antiportal test.
        PCZSceneNode* antiPortalNode = (PCZSceneNode*)mSceneMgr->getRootSceneNode()->createChildSceneNode();
        PCZone* defaultZone = ((PCZSceneManager*)mSceneMgr)->getDefaultZone();
        AntiPortal* antiPortal = ((PCZSceneManager*)mSceneMgr)->createAntiPortal(name);
        antiPortal->setCorner(0, Vector3(100.0f, 100.0f, 0.0f));
        antiPortal->setCorner(1, Vector3(100.0f, -100.0f, 0.0f));
        antiPortal->setCorner(2, Vector3(-100.0f, -100.0f, 0.0f));
        antiPortal->setCorner(3, Vector3(-100.0f, 100.0f, 0.0f));
        antiPortalNode->attachObject(antiPortal);
        defaultZone->_addAntiPortal(antiPortal);
        ((PCZSceneManager*)mSceneMgr)->addPCZSceneNode(antiPortalNode, defaultZone);

        // Anti portal prop.
        Entity* planeEnt = mSceneMgr->createEntity(name + "Entity", SceneManager::PT_PLANE);
        planeEnt->setMaterialName("TransparentGlassTinted");
        antiPortalNode->attachObject(planeEnt);

        return antiPortalNode;
    }

    void setupContent(void)
    {
        mCameraMan->setTopSpeed(15);
        mCamera->setNearClipDistance(2);

        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.25, 0.25, 0.25));

        // Create a skybox
        mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox", 500 );
        // put the skybox node in the default zone
        ((PCZSceneManager*)mSceneMgr)->setSkyZone(0);

        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        l->setAttenuation(500, 0.5, 1.0, 0.0);
        // Accept default settings: point light, white diffuse, just set position
        // attach light to a scene node so the PCZSM can handle it properly (zone-wise)
        SceneNode * lightNode = mCameraNode->createChildSceneNode("light_Node");
        lightNode->attachObject(l);

        // Fog
        // NB it's VERY important to set this before calling setWorldGeometry 
        // because the vertex program picked will be different
        ColourValue fadeColour(0.101, 0.125, 0.1836);
        mSceneMgr->setFog( FOG_LINEAR, fadeColour, .001, 500, 1000);
        mViewport->setBackgroundColour(fadeColour);

        // create a terrain zone
//        String terrain_cfg("terrain.cfg");
//      String zoneName("Terrain1_Zone");
//      PCZone * terrainZone = createTerrainZone(zoneName, terrain_cfg);

/*      // Create another terrain zone
        terrain_cfg = "terrain.cfg";
        zoneName = "Terrain2_Zone";
        terrainZone = createTerrainZone(zoneName, terrain_cfg);
        // move second terrain next to first terrain
        terrainZone->getEnclosureNode()->setPosition(1500, 0, 0);

        // Create another terrain zone
        terrain_cfg = "terrain.cfg";
        zoneName = "Terrain3_Zone";
        terrainZone = createTerrainZone(zoneName, terrain_cfg);
        // move terrain next to first terrain
        terrainZone->getEnclosureNode()->setPosition(0, 0, 1500);

        // Create another terrain zone
        terrain_cfg = "terrain.cfg";
        zoneName = "Terrain4_Zone";
        terrainZone = createTerrainZone(zoneName, terrain_cfg);
        // move terrain next to first terrain
        terrainZone->getEnclosureNode()->setPosition(-1500, 0, 0);

        // Create another terrain zone
        terrain_cfg = "terrain.cfg";
        zoneName = "Terrain5_Zone";
        terrainZone = createTerrainZone(zoneName, terrain_cfg);
        // move terrain next to first terrain
        terrainZone->getEnclosureNode()->setPosition(0, 0, -1500);

        // Create another terrain zone
        terrain_cfg = "terrain.cfg";
        zoneName = "Terrain6_Zone";
        terrainZone = createTerrainZone(zoneName, terrain_cfg);
        // move terrain next to first terrain
        terrainZone->getEnclosureNode()->setPosition(1500, 0, 1500);

        // Create another terrain zone
        terrain_cfg = "terrain.cfg";
        zoneName = "Terrain7_Zone";
        terrainZone = createTerrainZone(zoneName, terrain_cfg);
        // move terrain next to first terrain
        terrainZone->getEnclosureNode()->setPosition(-1500, 0, -1500);

        // Create another terrain zone
        terrain_cfg = "terrain.cfg";
        zoneName = "Terrain8_Zone";
        terrainZone = createTerrainZone(zoneName, terrain_cfg);
        // move terrain next to first terrain
        terrainZone->getEnclosureNode()->setPosition(-1500, 0, 1500);

        // Create another terrain zone
        terrain_cfg = "terrain.cfg";
        zoneName = "Terrain9_Zone";
        terrainZone = createTerrainZone(zoneName, terrain_cfg);
        // move terrain next to first terrain
        terrainZone->getEnclosureNode()->setPosition(1500, 0, -1500);
*/
        // set far clip plane to one terrain zone width (we have a LOT of terrain here, so we need to do far clipping!)
        mCamera->setFarClipDistance(1500);

        // create test buildinig
        RoomObject roomObj;
        buildingNode = roomObj.createTestBuilding(mSceneMgr, String("1"));
        buildingNode->setPosition(500, 165, 570);
        //Ogre::Radian r = Radian(3.1416/7.0);
        //buildingNode->rotate(Vector3::UNIT_Y, r);

        // create another test buildinig
        RoomObject roomObj2;
        buildingNode = roomObj2.createTestBuilding(mSceneMgr, String("2"));
        buildingNode->setPosition(400, 165, 570);
        //Ogre::Radian r = Radian(3.1416/7.0);
        //buildingNode->rotate(Vector3::UNIT_Y, r);

        PCZSceneNode* antiPortalNode1 = createAntiPortal("AntiPortal1");
        antiPortalNode1->setPosition(Vector3(450, 200, 800));
        antiPortalNode1->setScale(0.5f, 0.5f, 0.5f);

        PCZSceneNode* antiPortalNode2 = createAntiPortal("AntiPortal2");
        antiPortalNode2->setPosition(Vector3(460, 200, 700));
        antiPortalNode2->setScale(0.5f, 0.5f, 0.5f);

        // Position camera in the center of the building
        mCameraNode->setPosition(buildingNode->getPosition());
        // Update bounds for camera
        mCameraNode->_updateBounds();

        // create the ray scene query
        raySceneQuery = mSceneMgr->createRayQuery(
            Ray(mCamera->getParentNode()->getPosition(), Vector3::NEGATIVE_UNIT_Z));
        raySceneQuery->setSortByDistance(true, 5);

    }

    /**
        a simple Ray Scene query which highlights the 'furthest' object in
        front of the camera (not very visible inside the castle since the room the
        camera is in is usually what gets highlighted).  It's just a simple test of
        ray scene queries. Scene Queries will (at least, should) traverse portals.
     */
    bool frameRenderingQueued( const FrameEvent& evt )
    {
        SdkSample::frameRenderingQueued(evt);
#if 0
        buildingTranslate = Vector3(0,0,0);
        if( mKeyboard->isKeyDown( OIS::KC_LBRACKET ) )
        {
            buildingTranslate = Vector3(0,-10,0);
        }
        if( mKeyboard->isKeyDown( OIS::KC_RBRACKET ) )
        {
            buildingTranslate = Vector3(0,10,0);
        }
#endif
        // test the ray scene query by showing bounding box of whatever the camera is pointing directly at
        // (takes furthest hit)
        static Ray updateRay;
        updateRay.setOrigin(mCamera->getParentSceneNode()->getPosition());
        updateRay.setDirection(mCamera->getParentSceneNode()->getOrientation()*Vector3::NEGATIVE_UNIT_Z);
        raySceneQuery->setRay(updateRay);
        PCZone * zone = ((PCZSceneNode*)(mCamera->getParentSceneNode()))->getHomeZone();
        ((PCZRaySceneQuery*)raySceneQuery)->setStartZone(zone);
        ((PCZRaySceneQuery*)raySceneQuery)->setExcludeNode(mCamera->getParentSceneNode());
        RaySceneQueryResult& qryResult = raySceneQuery->execute();
        RaySceneQueryResult::iterator i = qryResult.begin();
        if (i != qryResult.end())
        {
            RaySceneQueryResult::reverse_iterator ri = qryResult.rbegin();
            MovableObject * mo = ri->movable;
            if (targetMO != mo)
            {
                if (targetMO != 0)
                {
                    targetMO->getParentSceneNode()->showBoundingBox(false);
                }
                targetMO = mo;
                targetMO->getParentSceneNode()->showBoundingBox(true);
            }
        }

        return true;
    }
};
