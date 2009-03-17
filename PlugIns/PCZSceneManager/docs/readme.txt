========================================================================
    About the "Plugin_PCZSceneManager" Project
========================================================================

The Portal-Connected-Zone Scene Manager (PCZSM) is a plugin for Ogre3D (see
www.ogre3d.org for more information about Ogre3d) which allows traversal of
a scene which is non-homogeneous in structure.  Specifically, the PCZSM uses
"Zones" which have their own hierarchy.  Zones are connected to other zones
by "Portals" which can be envisioned as 4-point convex planar polygons (or
Spheres or AAB's).  

This document gives basic information on the usage of the PCZSceneManager.
It is not complete, and will change & expand as needed.  Note that as of
this writing, the PCZSM has not undergone very extensive testing, nor is
it optimized very much.  Assistance in these two areas would be greatly
appreciated.  For support or to report bugs, please see the Developer Forum 
at Ogre3D.org

Cheers,

Eric "Chaster" Cha

/////////////////////////////////////////////////////////////////////////////
			USING THE PCZ_SCENEMANAGER
/////////////////////////////////////////////////////////////////////////////

NOTE: For an example of PCZSM usage, see the PCZTestApp Application.  It
is probably a lot easier to understand than trying to figure it all out
from this. 

LOADING & INITIALIZATION:

The PCZSM is loaded just like any other Scene Manager plugin.  Included in
the standard PCZSM plugin is the "default" zone.  If the user wishes to
utilize the OctreeZone or TerrainZone, the "Plugin_OctreeZone" should be
loaded *after* the PCZSM plugin is loaded.  

Before using the PCZSM, the PCZSceneManager::init(zoneType) function should be called. 
During intialization, the user specifies what type of zone (ZoneType_Default,
ZoneType_OCtree, ZoneType_Terrain) the PCZSM should use for the default zone.
The default zone is the zone where entities are placed if they are not 
specified to be in other zones.  

CREATING ZONES:

Once the PCZSM has been initialized, the user can proceed with creating
zones (PCZSceneManager::createZone(zoneType, zoneName)).  Zones can be 
anything from the outdoors (i.e. a Terrain) to a room in a building, or
a tunnel in a dungeon.  Zones can be of any size or shape, and can move.  

NOTE: In the PCZSM, sky rendering (domes, boxes, planes) is associated 
with a specific zone.  For example, if the user has a building on a terrain,
the sky could be associated with the terrain (i.e. the "outdoors") using
the function PCZSceneManager::setSkyZone(zone).  This tells the PCZSM to
only draw the sky when the designated zone is visible.  (i.e. only draw
the sky when the 'outdoor' zone is visible).  Usually, the Sky should be
associated with the default zone (which is usually used as the "all
encompassing exterior zone").

CREATING PORTALS:

Once the user has created a zone (in addition to the default zone), 
they can create portals to attach two zones together.  

*** NEW AS OF 9/25/07: 

To create a portal, the user just calls PCZSceneManager::createPortal(). 
NOTE: The user should NOT just instantiate a portal manually (i.e. portal = new Portal)
because the clean up of portals is handled by the scene manager directly.
Instantiating portals using the C++ 'new' command will result in memory leaks
and errors if the scene is destroyed and recreated.

To destroy a portal use PCZSceneManager::destroyPortal(Portal *p) or 
PCZSceneManager::destroyPortal(String & portalName).

***

Then set the portal corner points, attaches it to a node, and then adds it 
to the zone (see PCZTestApp -> RoomObject.cpp -> createPortals() function).  

NOTE: Portals currently only connect different zones.  The user can't
connect portals to the same zone yet (i.e. no teleporters).  This functionality
could be added later.

UPDATE as of 3/17/09: Portals can be "closed" (and opened) by calling Portal::setEnable(false)
(and Portal::setEnable(false)).  Disabling a portal prevents the scene manager from traversing
through the portal and also prevents scenenodes & ray queries from crossing the portal.
Basically, it turns a portal "off".  Disabling an antiportal (see note below about 
Creating antiportals) prevents the antiportal from blocking scene traversal through
regular portals.

NEW IN VERSION 1.2: Portals can take 3 different forms: quad portals, AAB portals, 
and Sphere portals.  AAB and Sphere portals do not add any culling planes to the
frustum, and just serve to serve as enclosures for zones which aren't naturally
surrounded by geometry.  They function a little different than traditional quad
portals in that they are volumetric, instead of planar (crossing is determined
by going from "inside" to "outside" or vice versa).

Quad Portals require 4 corner points which are co-planar and form a polygon which
is convex. Quad Portal corners are specified in right-handed counter-
clockwise winding order so that the norm of the portal would be facing 
the viewer.  

AAB Portals require 2 corner points (minimum & maximum corners) and form an axis-
aligned box around the zone.  IMPORTANT: The AAB portals that are associated with
a node require a node which has the correct size AAB (node->_getWorldAABB()).

Sphere Portals require 2 corner points (center point, and point on the surface of 
the sphere).  

The "direction" norm of AAB and Sphere portals is specified as either 
Vector3::UNIT_NEGATIVE_Z or Vector3::UNIT_Z.  The first corresponds to a portal
with norm facing 'inward' and the latter corresponds to a portal with norm facing
'outward'.  

NOTE regarding Portal Norms: The Norm of a portal should always point away from the 
zone the portal leads to.  Another way to think of this is a node will only cross
a portal if it crosses the portal traveling opposite direction of the norm).

Portals also (currently) require a "matching" portal for proper
scene traversal.  In other words, portals always exist in pairs - one in 
each zone connected and co-existing in the same location, but facing in 
opposite directions.  

It is REQUIRED that a portal be associated with a scene node.  Use Portal::setNode() 
to associate the portal with a scene node.  Once the portal is associated with a 
scene node, it will move with the scene node (including rotations or translations).
Because of this, it is also highly recommended (although not required) that
the node a portal is associated with be located at the center of the portal.

NOTE: Scaling of a portal is not yet *tested*.  Scaling a node should scale 
the portal (but don't cry to me if it doesn't work right yet...)   

Once all portals in the scene have been created, the user can either manually 
assign their zone targets (i.e. the zone which they connect to) or they can
call PCZSceneManager::connectPortalsToTargetZonesByLocation() to do it 
automatically.  Note that this function requires all portals to have a matching
portal in the target zone.  

ANTIPORTALS  ** NEW As of 3/17/09 **

Antiportals are a new feature (thanks to Lf3thn4d). Antiportals prevent traversal of 
portals located behind them (as viewed from the camera).  They are created and manipulated
the same as regular portals (except you don't need to create them in pairs, since they
block scene traversal to other zones instead of connecting them).  Only Quad Antiportals
are supported.  

To Create an antiportal, it's very similar to regular portals.  All you need to do is call
PCZSceneManager::createAntiPortal("name of the antiportal"), set the corner values,
attach it to a node, and add it to the proper zone.  

CREATING OBJECTS/ENTITIES:

Once the zones and portals have been created, the user can create objects/entities. 
The user should use SceneManager::createSceneNode() to create all scene nodes.

NOTE: ALL OBJECTS *MUST* BE ATTACHED TO SCENENODES!!!  Unlike the other SM's
available in Ogre3d, the PCZSM relies on Scene Nodes to determine zone
locality of all entities - including cameras and lights.  Consequently,
when a camera (or light) is created, the user should also attach the camera
(or light) to a scene node and use that node to manipulate the object.

SceneNodes can be assigned (by the user) to a zone upon creation of the 
SceneNode.  Use the functions "PCZSceneNode::setHomeZone(PCZone * zone)" followed
by "PCZone::_addNode(PCZSceneNode * node)" to do this.  

If the user doesn't do this, the PCZSM will try to figure out which zone the 
node belongs in using volumetric testing, but since there are situations when 
this can fail, it is highly recommended that the user does so explicitly instead 
of relying on the SM to figure it out.  Note that this only has to be done when 
adding a node to the scene.  Once the node is in the proper zone, the SM will 
handle moving it to other zones as necessary.

NOTE: In order for the automatic zone assignment function to work, zones must
have an "enclosure" object/node assigned to them (using the "PCZone::setEnclosureNode()
function).  The enclosure node (or more specifically, the object attached to it)
supplies the axis-aligned bounding box that determines the bounds of the zone.  
So for example, the enclosure node/object for a room would be the model of the 
walls, ceiling, and floor (assuming they are all modeled as one object or at least
all attached to the same node).  See the PCZTestApp for an example.

SCENE QUERIES:

I have implemented Scene Query functions for Default & Octree Zones.  In general,
they are used the same way as Scene Queries for any other Scene Manager, with
one difference.  The user must specify the "start zone" for any scene query
using XXXSceneQuery::setStartZone(zone) where "XXX" is Ray, Sphere, AxisAlignedBox, etc.

KNOWN BUGS:

* Light traversal is not quite correct.  In order to avoid infinite recursion, I had to
  put in a hack which can potentially result in lighting not traversing into some zones properly.
  It will probably not be noticeable in most situations, but could potentially show up in
  very complex portal/zone setups.

/////////////////////////////////////////////////////////////////////////////
