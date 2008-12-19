========================================================================
    About the "PCZTestApp" Project
========================================================================

The PCZTestApp is an application to test/demonstrate functionality of the
Portal Connected Zone (PCZ) Scene Manager.  The base code started from the
Ogre "Skybox" demo.  Standard "first person" camera controls apply, and
you can change to wireframe mode by pressing "r" (switch back by pressing
"r" again).  There are some other random controls (look at the code to 
see) but that's pretty much what I use.

Basically, the test app creates a building with many rooms in it and a 
terrain that the user can fly the camera around to see how the PCZSceneManager 
culls using zones for the rooms and the outside.

There is a simple Ray Scene query which highlights the 'furthest' object in 
front of the camera (not very visible inside the castle since the room the
camera is in is usually what gets highlighted).  It's just a simple test of
ray scene queries. Scene Queries will (at least, should) traverse portals.

Note that there is no collision detection and transitioning from zone to
zone will only work correctly if the user moves the camera through the
doorways (as opposed to going through the walls).  

/////////////////////////////////////////////////////////////////////////////
Change Log:

(12/03/2008) Fixed mesh name references in code to account for case sensitivity in Linux.

/////////////////////////////////////////////////////////////////////////////
