//
//
//
//	Filename : RoomObject.cpp

#include "Ogre.h"
#include "OgreMaterial.h"
#include "RoomObject.h"
#include "OgrePortal.h"

using namespace Ogre;
static int count = 0;

PCZSceneNode * RoomObject::createTestBuilding(Ogre::SceneManager *scene, const String & name)
{
	count++;
	mPortalCount = 0;
	Ogre::PCZSceneManager * pczSM = (Ogre::PCZSceneManager*)scene;

	// set points to building exterior size
	createPoints(Vector3(60.0, 40.0, 60.0), Vector3(4.0, 10.0, 4.0));

	// create the building exterior
	Entity *exterior = pczSM->createEntity( name + "_building_exterior", "building_exterior.mesh" );

	// make the enclosure a child node of the root scene node
	PCZSceneNode * exteriorNode, * baseNode;
	baseNode = (PCZSceneNode*)scene->getRootSceneNode()->createChildSceneNode(name +"_base_node");
    exteriorNode = (PCZSceneNode*)baseNode->createChildSceneNode(name +"_building_exterior_node",  Vector3( 0.0f, 0.0f, 0.0f ) );
	exteriorNode->attachObject(exterior);
	pczSM->addPCZSceneNode(exteriorNode, pczSM->getDefaultZone());

	// create portals for the building exterior
	createPortals(scene, 
				  exterior, 
				  exteriorNode, 
				  pczSM->getDefaultZone(),
				  DOOR_FRONT|DOOR_BACK|DOOR_LEFT|DOOR_RIGHT, 
				  true);

	// reset points to room size
	createPoints(Vector3(20.0, 10.0, 20.0), Vector3(4.0, 10.0, 4.0));

	// create an interior room
    Entity *room = pczSM->createEntity( name +"_room1", "room_nzpz.mesh" );

	// add the room as a child node to the enclosure node
	PCZSceneNode * roomNode;
	roomNode = (PCZSceneNode*)baseNode->createChildSceneNode( name +"_room1_node", Vector3( 0.0f, 0.0f, 20.0f ) );
	roomNode->attachObject(room);

	// room needs it's own zone
	Ogre::String zoneType = "ZoneType_Default";
	Ogre::String zoneName = name +"_room1_zone";
	PCZone * newZone = pczSM->createZone(zoneType, zoneName);
	newZone->setEnclosureNode(roomNode);
	pczSM->addPCZSceneNode(roomNode, newZone);

	// create portals for the room
	createPortals(scene, 
				  room, 
				  roomNode, 
				  newZone,
				  DOOR_FRONT|DOOR_BACK, 
				  false);

	// create another interior room
    room = pczSM->createEntity( name +"_room2", "room_nxpxnypynzpz.mesh" );

	// add the room as a child node to the enclosure node
	roomNode = (PCZSceneNode*)baseNode->createChildSceneNode( name +"_room2_node", Vector3( 0.0f, 0.0f, 0.0f ) );
	roomNode->attachObject(room);

	// room needs it's own zone
	zoneName = name +"_room2_zone";
	newZone = pczSM->createZone(zoneType, zoneName);
	newZone->setEnclosureNode(roomNode);
	pczSM->addPCZSceneNode(roomNode, newZone);

	// create portals for the room
	createPortals(scene, 
				  room, 
				  roomNode, 
				  newZone,
				  DOOR_FRONT|DOOR_BACK|DOOR_LEFT|DOOR_RIGHT|DOOR_TOP|DOOR_BOT, 
				  false);

	// create another interior room
    room = pczSM->createEntity( name +"_room3", "room_nzpz.mesh" );

	// add the room as a child node to the enclosure node
	roomNode = (PCZSceneNode*)baseNode->createChildSceneNode( name +"_room3_node", Vector3( 0.0f, 0.0f, -20.0f ) );
	roomNode->attachObject(room);

	// room needs it's own zone
	zoneName = name +"_room3_zone";
	newZone = pczSM->createZone(zoneType, zoneName);
	newZone->setEnclosureNode(roomNode);
	pczSM->addPCZSceneNode(roomNode, newZone);

	// create portals for the room
	createPortals(scene, 
				  room, 
				  roomNode, 
				  newZone,
				  DOOR_FRONT|DOOR_BACK, 
				  false);

	// create another interior room
    room = pczSM->createEntity( name +"_room4", "room_nxpx.mesh" );

	// add the room as a child node to the enclosure node
	roomNode = (PCZSceneNode*)baseNode->createChildSceneNode( name +"_room4_node", Vector3( -20.0f, 0.0f, 0.0f ) );
	roomNode->attachObject(room);

	// room needs it's own zone
	zoneName = name +"_room4_zone";
	newZone = pczSM->createZone(zoneType, zoneName);
	newZone->setEnclosureNode(roomNode);
	pczSM->addPCZSceneNode(roomNode, newZone);

	// create portals for the room
	createPortals(scene, 
				  room, 
				  roomNode, 
				  newZone,
				  DOOR_LEFT|DOOR_RIGHT, 
				  false);

	// create another interior room
    room = pczSM->createEntity( name +"_room5", "room_nxpx.mesh" );

	// add the room as a child node to the enclosure node
	roomNode = (PCZSceneNode*)baseNode->createChildSceneNode( name +"_room5_node", Vector3( 20.0f, 0.0f, 0.0f ) );
	roomNode->attachObject(room);

	// room needs it's own zone
	zoneName = name +"_room5_zone";
	newZone = pczSM->createZone(zoneType, zoneName);
	newZone->setEnclosureNode(roomNode);
	pczSM->addPCZSceneNode(roomNode, newZone);

	// create portals for the room
	createPortals(scene, 
				  room, 
				  roomNode, 
				  newZone,
				  DOOR_LEFT|DOOR_RIGHT, 
				  false);

	// create another interior room
    room = pczSM->createEntity( name +"_room6", "ROOM_NY.mesh" );

	// add the room as a child node to the enclosure node
	roomNode = (PCZSceneNode*)baseNode->createChildSceneNode( name +"_room6_node", Vector3( 0.0f, 10.0f, 0.0f ) );
	roomNode->attachObject(room);

	// room needs it's own zone
	zoneName = name +"_room6_zone";
	newZone = pczSM->createZone(zoneType, zoneName);
	newZone->setEnclosureNode(roomNode);
	pczSM->addPCZSceneNode(roomNode, newZone);

	// create portals for the room
	createPortals(scene, 
				  room, 
				  roomNode, 
				  newZone,
				  DOOR_BOT, 
				  false);

	// create another interior room
    room = pczSM->createEntity( name +"_room7", "ROOM_PY.mesh" );

	// add the room as a child node to the enclosure node
	roomNode = (PCZSceneNode*)baseNode->createChildSceneNode( name +"_room7_node", Vector3( 0.0f, -50.0f, 0.0f ) );
	roomNode->attachObject(room);

	// room needs it's own zone
	zoneName = name +"_room7_zone";
	newZone = pczSM->createZone(zoneType, zoneName);
	newZone->setEnclosureNode(roomNode);
	pczSM->addPCZSceneNode(roomNode, newZone);

	// create portals for the room
	createPortals(scene, 
				  room, 
				  roomNode, 
				  newZone,
				  DOOR_TOP, 
				  false);

	// reset points to tall room size
	createPoints(Vector3(20.0, 40.0, 20.0), Vector3(4.0, 10.0, 4.0));

	// create another interior room
    room = pczSM->createEntity( name +"_room8", "room_nypy_4y.mesh" );

	// add the room as a child node to the enclosure node
	roomNode = (PCZSceneNode*)baseNode->createChildSceneNode( name +"_room8_node", Vector3( 0.0f, -25.0f, 0.0f ) );
	roomNode->attachObject(room);

	// room needs it's own zone
	zoneName = name +"_room8_zone";
	newZone = pczSM->createZone(zoneType, zoneName);
	newZone->setEnclosureNode(roomNode);
	pczSM->addPCZSceneNode(roomNode, newZone);

	// create portals for the room
	createPortals(scene, 
				  room, 
				  roomNode, 
				  newZone,
				  DOOR_BOT|DOOR_TOP, 
				  false);


	// resolve portal zone pointers
	pczSM->connectPortalsToTargetZonesByLocation();

	return baseNode;
}


Ogre::ManualObject* RoomObject::createRoom(Ogre::SceneManager *scene, 
										   const Ogre::String &name, 
										   short doorFlags,
										   bool isEnclosure,
										   Ogre::Vector3 dimensions,
										   Ogre::Vector3 doorDimensions)
{
	addMaterial(name, Ogre::ColourValue(1,1,1,.75), Ogre::SBT_TRANSPARENT_ALPHA);

	Ogre::ManualObject* room = scene->createManualObject(name); 

	room->begin(name, Ogre::RenderOperation::OT_TRIANGLE_LIST); 

	// create points
	createPoints(dimensions, doorDimensions);

//	Ogre::Real fade=.5;
	Ogre::Real solid=.8;
	Ogre::ColourValue color = ColourValue(0, 0, solid, solid);

	// copy to room
	for (int i=0;i<32;i++)
	{
		room->position(points[i]);
		room->colour(color);
	}

	createWalls(room, doorFlags, isEnclosure);

	room->end(); 

	return room;
}

void RoomObject::addMaterial(const Ogre::String &mat, 
							 const Ogre::ColourValue &clr, 
							 Ogre::SceneBlendType sbt)
{
	static int init=false;
	if(init)
		return;
	else
		init=true;

	Ogre::MaterialPtr matptr = Ogre::MaterialManager::getSingleton().create(mat, "General"); 
	matptr->setReceiveShadows(false); 
	matptr->getTechnique(0)->setLightingEnabled(true);
	matptr->getTechnique(0)->getPass(0)->setDiffuse(clr); 
	matptr->getTechnique(0)->getPass(0)->setAmbient(clr); 
	matptr->getTechnique(0)->getPass(0)->setSelfIllumination(clr); 
	matptr->getTechnique(0)->getPass(0)->setSceneBlending(sbt);
	matptr->getTechnique(0)->getPass(0)->setLightingEnabled(false);
	matptr->getTechnique(0)->getPass(0)->setVertexColourTracking(Ogre::TVC_DIFFUSE);
}

void RoomObject::createPoints(Ogre::Vector3 dimensions,
							  Ogre::Vector3 doorDimensions)
{
	Ogre::Real l = dimensions.x/2;
	Ogre::Real h = dimensions.y/2;
	Ogre::Real w = dimensions.z/2;
	
//			 4		 7
//            *-------*
//			 /|      /|
//			/ |		/ |			y
//		   / 5|	  3/ 6|			|
//		 0*---*---*---*			*-- x 
//		  |  /    |  /		   /
//        | /     | /		  z 
//        |/	  |/
//		 1*-------*2

	points[0] = Ogre::Vector3(-l, h, w);//0
	points[1] = Ogre::Vector3(-l, -h, w);//1
	points[2] = Ogre::Vector3(l, -h, w);//2
	points[3] = Ogre::Vector3(l, h, w);//3

	points[4] = Ogre::Vector3(-l, h, -w);//4
	points[5] = Ogre::Vector3(-l, -h, -w);//5
	points[6] = Ogre::Vector3(l, -h, -w);//6
	points[7] = Ogre::Vector3(l, h, -w);//7

	// doors
	Ogre::Real l2 = doorDimensions.x/2;
	Ogre::Real h2 = doorDimensions.y/2;
	Ogre::Real w2 = doorDimensions.z/2;

	// front door
	points[8] = Ogre::Vector3(-l2, h2, w);//8
	points[9] = Ogre::Vector3(-l2, -h2, w);//9
	points[10] = Ogre::Vector3(l2, -h2, w);//10
	points[11] = Ogre::Vector3(l2, h2, w);//11

	// back door
	points[12] = Ogre::Vector3(-l2, h2, -w);//12
	points[13] = Ogre::Vector3(-l2, -h2, -w);//13
	points[14] = Ogre::Vector3(l2, -h2, -w);//14
	points[15] = Ogre::Vector3(l2, h2, -w);//15

	// top door
	points[16] = Ogre::Vector3(-l2, h, -w2);//16
	points[17] = Ogre::Vector3(-l2, h, w2);//17
	points[18] = Ogre::Vector3(l2, h, w2);//18
	points[19] = Ogre::Vector3(l2, h, -w2);//19

	// bottom door
	points[20] = Ogre::Vector3(-l2, -h, -w2);//20
	points[21] = Ogre::Vector3(-l2, -h, w2);//21
	points[22] = Ogre::Vector3(l2, -h, w2);//22
	points[23] = Ogre::Vector3(l2, -h, -w2);//23

	// left door
	points[24] = Ogre::Vector3(-l, h2, w2);//24
	points[25] = Ogre::Vector3(-l, -h2, w2);//25
	points[26] = Ogre::Vector3(-l, -h2, -w2);//26
	points[27] = Ogre::Vector3(-l, h2, -w2);//27

	// right door
	points[28] = Ogre::Vector3(l, h2, w2);//28
	points[29] = Ogre::Vector3(l, -h2, w2);//29
	points[30] = Ogre::Vector3(l, -h2, -w2);//30
	points[31] = Ogre::Vector3(l, h2, -w2);//31
}

void RoomObject::createWalls(Ogre::ManualObject* room,
							 short doorFlags,
							 bool isEnclosure)
{

	if (isEnclosure)
	{
		if(doorFlags & DOOR_FRONT)
		{
			// make front wall outward facing with door
			room->quad(0, 8, 11, 3);
			room->quad(1, 9, 8, 0);
			room->quad(2, 10, 9, 1);
			room->quad(3, 11, 10, 2);
		}
		else
		{
			// make front wall outward facing without door
			room->quad(0, 1, 2, 3);
		}
		if(doorFlags & DOOR_BACK)
		{
			// make back wall outward facing with door
			room->quad(7, 15, 12, 4);
			room->quad(6, 14, 15, 7);
			room->quad(5, 13, 14, 6);
			room->quad(4, 12, 13, 5);
		}
		else
		{
			// make back wall outward facing without door
			room->quad(7, 6, 5, 4);
		}
		if(doorFlags & DOOR_TOP)
		{
			// make top wall outward facing with door
			room->quad(0, 17, 16, 4);
			room->quad(4, 16, 19, 7);
			room->quad(7, 19, 18, 3);
			room->quad(3, 18, 17, 0);
		}
		else
		{
			// make top wall outward facing without door
			room->quad(0, 3, 7, 4);
		}
		if(doorFlags & DOOR_BOT)
		{
			// make bottom wall outward facing with door
			room->quad(5, 20, 21, 1);
			room->quad(6, 23, 20, 5);
			room->quad(2, 22, 23, 6);
			room->quad(1, 21, 22, 2);
		}
		else
		{
			// make bottom wall outward facing without door
			room->quad(2, 1, 5, 6);
		}
		if(doorFlags & DOOR_LEFT)
		{
			// make left wall outward facing with door
			room->quad(0, 24, 25, 1);
			room->quad(4, 27, 24, 0);
			room->quad(5, 26, 27, 4);
			room->quad(1, 25, 26, 5);
		}
		else
		{
			// make left side wall outward facing without door
			room->quad(1, 0, 4, 5);
		}
		if(doorFlags & DOOR_RIGHT)
		{
			// make right wall outward facing with door
			room->quad(2, 29, 28, 3);
			room->quad(6, 30, 29, 2);
			room->quad(7, 31, 30, 6);
			room->quad(3, 28, 31, 7);
		}
		else
		{
			// make right side wall outward facing without door
			room->quad(3, 2, 6, 7);
		}
	}
	else
	{
		// front back
		if(doorFlags & DOOR_FRONT)
		{
			// make front wall inward facing with door
			room->quad(3, 11, 8, 0);
			room->quad(0, 8, 9, 1);
			room->quad(1, 9, 10, 2);
			room->quad(2, 10, 11, 3);
		}
		else
		{
			// make front wall inward facing without door
			room->quad(3, 2, 1, 0);
		}
		if(doorFlags & DOOR_BACK)
		{
			// make back wall inward facing with door
			room->quad(4, 12, 15, 7);
			room->quad(7, 15, 14, 6);
			room->quad(6, 14, 13, 5);
			room->quad(5, 13, 12, 4);
		}
		else
		{
			// make back wall inward facing without door
			room->quad(4, 5, 6, 7);
		}
		// top bottom
		if(doorFlags & DOOR_TOP)
		{
			// make top wall inward facing with door
			room->quad(4, 16, 17, 0);
			room->quad(7, 19, 16, 4);
			room->quad(3, 18, 19, 7);
			room->quad(0, 17, 18, 3);
		}
		else
		{
			// make top wall inward facing without door
			room->quad(4, 7, 3, 0);
		}
		if(doorFlags & DOOR_BOT)
		{
			// make bottom wall inward facing with door
			room->quad(1, 21, 20, 5);
			room->quad(5, 20, 23, 6);
			room->quad(6, 23, 22, 2);
			room->quad(2, 22, 21, 1);
		}
		else
		{
			// make bottom wall inward facing without door
			room->quad(6, 5, 1, 2);
		}
		// end caps
		if(doorFlags & DOOR_LEFT)
		{
			// make left wall inward facing with door
			room->quad(1, 25, 24, 0);
			room->quad(0, 24, 27, 4);
			room->quad(4, 27, 26, 5);
			room->quad(5, 26, 25, 1);
		}
		else
		{
			// make left side wall inward facing without door
			room->quad(5, 4, 0, 1);
		}
		if(doorFlags & DOOR_RIGHT)
		{
			// make right wall inward facing with door
			room->quad(3, 28, 29, 2);
			room->quad(2, 29, 30, 6);
			room->quad(6, 30, 31, 7);
			room->quad(7, 31, 28, 3);
		}
		else
		{
			// make right side wall inward facing without door
			room->quad(7, 6, 2, 3);
		}
	}
}

// Create portals for every door
void RoomObject::createPortals(Ogre::SceneManager *scene,
							   Ogre::ManualObject* room,
							   Ogre::SceneNode * roomNode,
							   Ogre::PCZone * zone,
							   short doorFlags,
							   bool isEnclosure)
{
	Ogre::String portalName;
	Vector3 corners[4];

	if (isEnclosure)
	{
		if(doorFlags & DOOR_FRONT)
		{
			// set the corners to the front door corners
			corners[0] = points[8];
			corners[1] = points[9];
			corners[2] = points[10];
			corners[3] = points[11];
			// create the portal
			portalName = room->getName() + Ogre::String("_FrontDoorPortal");
			Portal * p = ((PCZSceneManager*)scene)->createPortal(portalName);
			p->setCorners(corners);
			// associate the portal with the roomnode
			p->setNode(roomNode);
			// add the portal to the zone
			zone->_addPortal(p);
			// update derived values for the portal
			p->updateDerivedValues();
		}
		if(doorFlags & DOOR_BACK)
		{
			// set the corners to the front door corners
			corners[0] = points[15];
			corners[1] = points[14];
			corners[2] = points[13];
			corners[3] = points[12];
			// create the portal
			portalName = room->getName() + Ogre::String("_BackDoorPortal");
			Portal * p = ((PCZSceneManager*)scene)->createPortal(portalName);
			p->setCorners(corners);
			// associate the portal with the roomnode
			p->setNode(roomNode);
			// add the portal to the zone
			zone->_addPortal(p);
			// update derived values for the portal
			p->updateDerivedValues();
		}
		if(doorFlags & DOOR_TOP)
		{
			// set the corners to the front door corners
			corners[0] = points[16];
			corners[1] = points[17];
			corners[2] = points[18];
			corners[3] = points[19];
			// create the portal
			portalName = room->getName() + Ogre::String("_TopDoorPortal");
			Portal * p = ((PCZSceneManager*)scene)->createPortal(portalName);
			p->setCorners(corners);
			// associate the portal with the roomnode
			p->setNode(roomNode);
			// add the portal to the zone
			zone->_addPortal(p);
			// update derived values for the portal
			p->updateDerivedValues();
		}
		if(doorFlags & DOOR_BOT)
		{
			// set the corners to the front door corners
			corners[0] = points[23];
			corners[1] = points[22];
			corners[2] = points[21];
			corners[3] = points[20];
			// create the portal
			portalName = room->getName() + Ogre::String("_BottomDoorPortal");
			Portal * p = ((PCZSceneManager*)scene)->createPortal(portalName);
			p->setCorners(corners);
			// associate the portal with the roomnode
			p->setNode(roomNode);
			// add the portal to the zone
			zone->_addPortal(p);
			// update derived values for the portal
			p->updateDerivedValues();
		}
		if(doorFlags & DOOR_LEFT)
		{
			// set the corners to the front door corners
			corners[0] = points[27];
			corners[1] = points[26];
			corners[2] = points[25];
			corners[3] = points[24];
			// create the portal
			portalName = room->getName() + Ogre::String("_LeftDoorPortal");
			Portal * p = ((PCZSceneManager*)scene)->createPortal(portalName);
			p->setCorners(corners);
			// associate the portal with the roomnode
			p->setNode(roomNode);
			// add the portal to the zone
			zone->_addPortal(p);
			// update derived values for the portal
			p->updateDerivedValues();
		}
		if(doorFlags & DOOR_RIGHT)
		{
			// set the corners to the front door corners
			corners[0] = points[28];
			corners[1] = points[29];
			corners[2] = points[30];
			corners[3] = points[31];
			// create the portal
			portalName = room->getName() + Ogre::String("_RightDoorPortal");
			Portal * p = ((PCZSceneManager*)scene)->createPortal(portalName);
			p->setCorners(corners);
			// associate the portal with the roomnode
			p->setNode(roomNode);
			// add the portal to the zone
			zone->_addPortal(p);
			// update derived values for the portal
			p->updateDerivedValues();
		}
	}
	else
	{
		if(doorFlags & DOOR_FRONT)
		{
			// set the corners to the front door corners
			corners[0] = points[11];
			corners[1] = points[10];
			corners[2] = points[9];
			corners[3] = points[8];
			// create the portal
			portalName = room->getName() + Ogre::String("_FrontDoorPortal");
			Portal * p = ((PCZSceneManager*)scene)->createPortal(portalName);
			p->setCorners(corners);
			// associate the portal with the roomnode
			p->setNode(roomNode);
			// add the portal to the zone
			zone->_addPortal(p);
			// update derived values for the portal
			p->updateDerivedValues();
		}
		if(doorFlags & DOOR_BACK)
		{
			// set the corners to the front door corners
			corners[0] = points[12];
			corners[1] = points[13];
			corners[2] = points[14];
			corners[3] = points[15];
			// create the portal
			portalName = room->getName() + Ogre::String("_BackDoorPortal");
			Portal * p = ((PCZSceneManager*)scene)->createPortal(portalName);
			p->setCorners(corners);
			// associate the portal with the roomnode
			p->setNode(roomNode);
			// add the portal to the zone
			zone->_addPortal(p);
			// update derived values for the portal
			p->updateDerivedValues();
		}
		if(doorFlags & DOOR_TOP)
		{
			// set the corners to the front door corners
			corners[0] = points[19];
			corners[1] = points[18];
			corners[2] = points[17];
			corners[3] = points[16];
			// create the portal
			portalName = room->getName() + Ogre::String("_TopDoorPortal");
			Portal * p = ((PCZSceneManager*)scene)->createPortal(portalName);
			p->setCorners(corners);
			// associate the portal with the roomnode
			p->setNode(roomNode);
			// add the portal to the zone
			zone->_addPortal(p);
			// update derived values for the portal
			p->updateDerivedValues();
		}
		if(doorFlags & DOOR_BOT)
		{
			// set the corners to the front door corners
			corners[0] = points[20];
			corners[1] = points[21];
			corners[2] = points[22];
			corners[3] = points[23];
			// create the portal
			portalName = room->getName() + Ogre::String("_BottomDoorPortal");
			Portal * p = ((PCZSceneManager*)scene)->createPortal(portalName);
			p->setCorners(corners);
			// associate the portal with the roomnode
			p->setNode(roomNode);
			// add the portal to the zone
			zone->_addPortal(p);
			// update derived values for the portal
			p->updateDerivedValues();
		}
		if(doorFlags & DOOR_LEFT)
		{
			// set the corners to the front door corners
			corners[0] = points[24];
			corners[1] = points[25];
			corners[2] = points[26];
			corners[3] = points[27];
			// create the portal
			portalName = room->getName() + Ogre::String("_LeftDoorPortal");
			Portal * p = ((PCZSceneManager*)scene)->createPortal(portalName);
			p->setCorners(corners);
			// associate the portal with the roomnode
			p->setNode(roomNode);
			// add the portal to the zone
			zone->_addPortal(p);
			// update derived values for the portal
			p->updateDerivedValues();
		}
		if(doorFlags & DOOR_RIGHT)
		{
			// set the corners to the front door corners
			corners[0] = points[31];
			corners[1] = points[30];
			corners[2] = points[29];
			corners[3] = points[28];
			// create the portal
			portalName = room->getName() + Ogre::String("_RightDoorPortal");
			Portal * p = ((PCZSceneManager*)scene)->createPortal(portalName);
			p->setCorners(corners);
			// associate the portal with the roomnode
			p->setNode(roomNode);
			// add the portal to the zone
			zone->_addPortal(p);
			// update derived values for the portal
			p->updateDerivedValues();
		}
	}
}

// Create portals for every door
void RoomObject::createPortals(Ogre::SceneManager *scene,
							   Ogre::Entity* room,
							   Ogre::SceneNode * roomNode,
							   Ogre::PCZone * zone,
							   short doorFlags,
							   bool isEnclosure)
{
	Ogre::String portalName;
	Vector3 corners[4];

	if (isEnclosure)
	{
		if(doorFlags & DOOR_FRONT)
		{
			// set the corners to the front door corners
			corners[0] = points[8];
			corners[1] = points[9];
			corners[2] = points[10];
			corners[3] = points[11];
			// create the portal
			portalName = room->getName() + Ogre::String("_FrontDoorPortal");
			Portal * p = ((PCZSceneManager*)scene)->createPortal(portalName);
			p->setCorners(corners);
			// associate the portal with the roomnode
			p->setNode(roomNode);
			// add the portal to the zone
			zone->_addPortal(p);
			// update derived values for the portal
			p->updateDerivedValues();
		}
		if(doorFlags & DOOR_BACK)
		{
			// set the corners to the front door corners
			corners[0] = points[15];
			corners[1] = points[14];
			corners[2] = points[13];
			corners[3] = points[12];
			// create the portal
			portalName = room->getName() + Ogre::String("_BackDoorPortal");
			Portal * p = ((PCZSceneManager*)scene)->createPortal(portalName);
			p->setCorners(corners);
			// associate the portal with the roomnode
			p->setNode(roomNode);
			// add the portal to the zone
			zone->_addPortal(p);
			// update derived values for the portal
			p->updateDerivedValues();
		}
		if(doorFlags & DOOR_TOP)
		{
			// set the corners to the front door corners
			corners[0] = points[16];
			corners[1] = points[17];
			corners[2] = points[18];
			corners[3] = points[19];
			// create the portal
			portalName = room->getName() + Ogre::String("_TopDoorPortal");
			Portal * p = ((PCZSceneManager*)scene)->createPortal(portalName);
			p->setCorners(corners);
			// associate the portal with the roomnode
			p->setNode(roomNode);
			// add the portal to the zone
			zone->_addPortal(p);
			// update derived values for the portal
			p->updateDerivedValues();
		}
		if(doorFlags & DOOR_BOT)
		{
			// set the corners to the front door corners
			corners[0] = points[23];
			corners[1] = points[22];
			corners[2] = points[21];
			corners[3] = points[20];
			// create the portal
			portalName = room->getName() + Ogre::String("_BottomDoorPortal");
			Portal * p = ((PCZSceneManager*)scene)->createPortal(portalName);
			p->setCorners(corners);
			// associate the portal with the roomnode
			p->setNode(roomNode);
			// add the portal to the zone
			zone->_addPortal(p);
			// update derived values for the portal
			p->updateDerivedValues();
		}
		if(doorFlags & DOOR_LEFT)
		{
			// set the corners to the front door corners
			corners[0] = points[27];
			corners[1] = points[26];
			corners[2] = points[25];
			corners[3] = points[24];
			// create the portal
			portalName = room->getName() + Ogre::String("_LeftDoorPortal");
			Portal * p = ((PCZSceneManager*)scene)->createPortal(portalName);
			p->setCorners(corners);
			// associate the portal with the roomnode
			p->setNode(roomNode);
			// add the portal to the zone
			zone->_addPortal(p);
			// update derived values for the portal
			p->updateDerivedValues();
		}
		if(doorFlags & DOOR_RIGHT)
		{
			// set the corners to the front door corners
			corners[0] = points[28];
			corners[1] = points[29];
			corners[2] = points[30];
			corners[3] = points[31];
			// create the portal
			portalName = room->getName() + Ogre::String("_RightDoorPortal");
			Portal * p = ((PCZSceneManager*)scene)->createPortal(portalName);
			p->setCorners(corners);
			// associate the portal with the roomnode
			p->setNode(roomNode);
			// add the portal to the zone
			zone->_addPortal(p);
			// update derived values for the portal
			p->updateDerivedValues();
		}
	}
	else
	{
		if(doorFlags & DOOR_FRONT)
		{
			// set the corners to the front door corners
			corners[0] = points[11];
			corners[1] = points[10];
			corners[2] = points[9];
			corners[3] = points[8];
			// create the portal
			portalName = room->getName() + Ogre::String("_FrontDoorPortal");
			Portal * p = ((PCZSceneManager*)scene)->createPortal(portalName);
			p->setCorners(corners);
			// associate the portal with the roomnode
			p->setNode(roomNode);
			// add the portal to the zone
			zone->_addPortal(p);
			// update derived values for the portal
			p->updateDerivedValues();
		}
		if(doorFlags & DOOR_BACK)
		{
			// set the corners to the front door corners
			corners[0] = points[12];
			corners[1] = points[13];
			corners[2] = points[14];
			corners[3] = points[15];
			// create the portal
			portalName = room->getName() + Ogre::String("_BackDoorPortal");
			Portal * p = ((PCZSceneManager*)scene)->createPortal(portalName);
			p->setCorners(corners);
			// associate the portal with the roomnode
			p->setNode(roomNode);
			// add the portal to the zone
			zone->_addPortal(p);
			// update derived values for the portal
			p->updateDerivedValues();
		}
		if(doorFlags & DOOR_TOP)
		{
			// set the corners to the front door corners
			corners[0] = points[19];
			corners[1] = points[18];
			corners[2] = points[17];
			corners[3] = points[16];
			// create the portal
			portalName = room->getName() + Ogre::String("_TopDoorPortal");
			Portal * p = ((PCZSceneManager*)scene)->createPortal(portalName);
			p->setCorners(corners);
			// associate the portal with the roomnode
			p->setNode(roomNode);
			// add the portal to the zone
			zone->_addPortal(p);
			// update derived values for the portal
			p->updateDerivedValues();
		}
		if(doorFlags & DOOR_BOT)
		{
			// set the corners to the front door corners
			corners[0] = points[20];
			corners[1] = points[21];
			corners[2] = points[22];
			corners[3] = points[23];
			// create the portal
			portalName = room->getName() + Ogre::String("_BottomDoorPortal");
			Portal * p = ((PCZSceneManager*)scene)->createPortal(portalName);
			p->setCorners(corners);
			// associate the portal with the roomnode
			p->setNode(roomNode);
			// add the portal to the zone
			zone->_addPortal(p);
			// update derived values for the portal
			p->updateDerivedValues();
		}
		if(doorFlags & DOOR_LEFT)
		{
			// set the corners to the front door corners
			corners[0] = points[24];
			corners[1] = points[25];
			corners[2] = points[26];
			corners[3] = points[27];
			// create the portal
			portalName = room->getName() + Ogre::String("_LeftDoorPortal");
			Portal * p = ((PCZSceneManager*)scene)->createPortal(portalName);
			p->setCorners(corners);
			// associate the portal with the roomnode
			p->setNode(roomNode);
			// add the portal to the zone
			zone->_addPortal(p);
			// update derived values for the portal
			p->updateDerivedValues();
		}
		if(doorFlags & DOOR_RIGHT)
		{
			// set the corners to the front door corners
			corners[0] = points[31];
			corners[1] = points[30];
			corners[2] = points[29];
			corners[3] = points[28];
			// create the portal
			portalName = room->getName() + Ogre::String("_RightDoorPortal");
			Portal * p = ((PCZSceneManager*)scene)->createPortal(portalName);
			p->setCorners(corners);
			// associate the portal with the roomnode
			p->setNode(roomNode);
			// add the portal to the zone
			zone->_addPortal(p);
			// update derived values for the portal
			p->updateDerivedValues();
		}
	}
}
