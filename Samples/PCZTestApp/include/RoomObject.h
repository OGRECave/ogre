//
//  Creates a "test" room (& building) for the PCZSceneManager.  The box-shaped room
//  can have "doorways" (rectangular openings) in any of the 6 walls, but
//  each doorway must have a separate portal object created for it (not
//  done in this class).
//
//  Filename : RoomObject.h

#ifndef _ROOM_OBJECT_H_
#define _ROOM_OBJECT_H_

#include "OgreSceneNode.h"
#include "OgrePCZSceneManager.h"

class RoomObject
{
    enum RoomWalls
    {
        TOP_WALL,
        BOT_WALL,
        FRONT_WALL,
        BACK_WALL,
        LEFT_WALL,
        RIGHT_WALL
    };

private:
    void    addMaterial(const Ogre::String& mat, 
                        const Ogre::ColourValue &clr, 
                        Ogre::SceneBlendType sbt);
    void    createPoints(Ogre::Vector3 dimensions,
                         Ogre::Vector3 doorDimensions);
    void    createWalls(Ogre::ManualObject* room,
                        short doorFlags,
                        bool isEnclosure);
    void    createPortals(Ogre::SceneManager *scene,
                          Ogre::ManualObject* room,
                          Ogre::SceneNode * roomNode,
                          Ogre::PCZone * zone,
                          short doorFlags,
                          bool isEnclosure);
    void    createPortals(Ogre::SceneManager *scene,
                          Ogre::Entity* room,
                          Ogre::SceneNode * roomNode,
                          Ogre::PCZone * zone,
                          short doorFlags,
                          bool isEnclosure);

private:
    Ogre::Vector3   points[32];
    int mPortalCount;
public:
    enum RoomDoors
    {
        DOOR_NONE   = 0x00,
        DOOR_TOP    = 0x01,
        DOOR_BOT    = 0x02,
        DOOR_FRONT  = 0x04,
        DOOR_BACK   = 0x08,
        DOOR_LEFT   = 0x10,
        DOOR_RIGHT  = 0x20,
        DOOR_ALL    = 0xFF
    };

    Ogre::PCZSceneNode * createTestBuilding(Ogre::SceneManager *scene, const Ogre::String & name);

    Ogre::ManualObject*createRoom(Ogre::SceneManager *scene, 
                                  const Ogre::String &name, 
                                  short doorFlags,
                                  bool isEnclosure,
                                  Ogre::Vector3 dimensions,
                                  Ogre::Vector3 doorDimensions);
};

#endif //--_ROOM_OBJECT_H_
