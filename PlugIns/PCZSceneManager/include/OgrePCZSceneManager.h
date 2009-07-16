/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
PCZSceneManager.h  -  Portal Connected Zone Scene Manager

-----------------------------------------------------------------------------
begin                : Mon Feb 19 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update	 :
-----------------------------------------------------------------------------
*/

#ifndef PCZ_SCENEMANAGER_H
#define PCZ_SCENEMANAGER_H

#include <OgreSceneManager.h>
#include <OgreSphere.h>

#include "OgrePCZPrerequisites.h"
#include "OgrePCZSceneNode.h"
#include "OgrePCZone.h"
#include "OgrePCZoneFactory.h"
#include "OgrePortal.h"
#include "OgreAntiPortal.h"

namespace Ogre
{

    class PCZone;
    class PCZCamera;
    class PCZIntersectionSceneQuery;
    class PCZRaySceneQuery;
    class PCZSphereSceneQuery;
    class PCZAxisAlignedBoxSceneQuery;
    class PCZPlaneBoundedVolumeListSceneQuery;

	typedef vector<SceneNode*>::type NodeList;
	typedef list<WireBoundingBox*>::type BoxList;

    /** Specialized SceneManager that uses Portal-Connected-Zones to divide the scene spatially.
    */

    class _OgrePCZPluginExport PCZSceneManager : public SceneManager
    {
        friend class PCZIntersectionSceneQuery;
        friend class PCZRaySceneQuery;
        friend class PCZSphereSceneQuery;
        friend class PCZAxisAlignedBoxSceneQuery;
        friend class PCZPlaneBoundedVolumeListSceneQuery;

    public:
        /** Standard Constructor.  */
        PCZSceneManager(const String& name);
        /** Standard destructor */
        ~PCZSceneManager();

	    /// @copydoc SceneManager::getTypeName
	    const String& getTypeName(void) const;

        /** Initializes the manager 
        */
        void init(const String &defaultZoneTypeName,
				  const String &filename = "none");

		/** Create a new portal instance
		*/
		Portal* createPortal(const String& name, PortalBase::PORTAL_TYPE type = PortalBase::PORTAL_TYPE_QUAD);

		/** Delete a portal instance by pointer
		*/
		void destroyPortal(Portal * p);

		/** Delete a portal instance by name
		*/
		void destroyPortal(const String & portalName);

		/** Create a new anti portal instance */
		AntiPortal* createAntiPortal(const String& name, PortalBase::PORTAL_TYPE type = PortalBase::PORTAL_TYPE_QUAD);

		/** Delete a anti portal instance by pointer */
		void destroyAntiPortal(AntiPortal * p);

		/** Delete a anti portal instance by name */
		void destroyAntiPortal(const String& portalName);

		/** Create a zone from a file (type of file
		  * depends on the zone type
		 */
		PCZone * createZoneFromFile(const String &zoneTypeName,
									const String &zoneName,
									PCZSceneNode * parentNode,
									const String & filename);

		/** Set the "main" geometry of the zone */
		virtual void setZoneGeometry(const String & zoneName,
									 PCZSceneNode * parentNode,
									 const String &filename);

        /// override this to ensure specialised PCZSceneNode is used.
        virtual SceneNode* createSceneNodeImpl(void);
        /// override this to ensure their specialised PCZSceneNode is used.
        virtual SceneNode* createSceneNodeImpl(const String& name);
        /** Creates a PCZSceneNode  */
        virtual	SceneNode * createSceneNode ( void );
        /** Creates a PCZSceneNode */
        virtual SceneNode * createSceneNode ( const String &name );
        /** Creates a specialized PCZCamera */
        virtual Camera * createCamera( const String &name );

        /** Deletes a scene node & corresponding PCZSceneNode */
        virtual void destroySceneNode( const String &name );

		/** overridden to clean up zones
		*/
		virtual void clearScene(void);

		/** Overridden from SceneManager */
		void setWorldGeometryRenderQueue(uint8 qid);

		// Overridden from basic scene manager
		void _renderScene(Camera *cam, Viewport *vp, bool includeOverlays);

		/* enable/disable sky rendering */
		void enableSky(bool);

		/* Set the zone which contains the sky node */
		void setSkyZone(PCZone * zone);

        /** Update Scene Graph (does several things now) */
        virtual void _updateSceneGraph( Camera * cam );

        /** Recurses through the PCZTree determining which nodes are visible. */
        virtual void _findVisibleObjects ( Camera * cam, 
		    VisibleObjectsBoundsInfo* visibleBounds, bool onlyShadowCasters );

        /** Alerts each unculled object, notifying it that it will be drawn.
        * Useful for doing calculations only on nodes that will be drawn, prior
        * to drawing them...
        */
        virtual void _alertVisibleObjects( void );

        /** Creates a light for use in the scene.
            @remarks
                Lights can either be in a fixed position and independent of the
                scene graph, or they can be attached to SceneNodes so they derive
                their position from the parent node. Either way, they are created
                using this method so that the SceneManager manages their
                existence.
            @param
                name The name of the new light, to identify it later.
        */
        virtual Light* createLight(const String& name);

        /** Returns a pointer to the named Light which has previously been added to the scene.
		@note Throws an exception if the named instance does not exist
        */
        virtual Light* getLight(const String& name) const;

		/** Returns whether a light with the given name exists.
		*/
		virtual bool hasLight(const String& name) const;

		/** Removes the named light from the scene and destroys it.
            @remarks
                Any pointers held to this light after calling this method will be invalid.
        */
        virtual void destroyLight(const String& name);

        /** Removes and destroys all lights in the scene.
        */
        virtual void destroyAllLights(void);

		/** Check/Update the zone data for every portal in the scene.
		 *  Essentially, this routine checks each portal for intersections
		 *  with other portals and updates if a crossing occurs 
		 */
		void _updatePortalZoneData(void);

		/** Mark nodes dirty for every zone with moving portal in the scene */
		void _dirtyNodeByMovingPortals(void);

		/** Update the PCZSceneNodes 
		*/
		void _updatePCZSceneNodes(void);

        /** Calculate which zones are affected by each light
        */
        void _calcZonesAffectedByLights(Camera * cam);

		/* Attempt to automatically connect unconnected portals to proper target zones 
		 * by looking for matching portals in other zones which are at the same location
		 */
		void connectPortalsToTargetZonesByLocation(void);

        /** Checks the given SceneNode, and determines if it needs to be moved
        * to a different PCZone or be added to the visitors list of other PCZone(s).
        */
        void _updatePCZSceneNode( PCZSceneNode * );

        /** Removes the given PCZSceneNode */
        void removeSceneNode( SceneNode * );

		/** add a PCZSceneNode to the scene by putting it in a zone 
		 * NOTE: This zone will be the scene node's home zone
		 */
		void addPCZSceneNode(PCZSceneNode * sn, PCZone * zone);

		/* Create a zone with the given name  */
		PCZone * createZone(const String& zoneType, const String& instanceName);

		/* destroy an existing zone within the scene */
		void destroyZone(PCZone* zone, bool destroySceneNodes);

        /** Make sure the home zone for the PCZSceneNode is up-to-date
        */
        void _updateHomeZone( PCZSceneNode *, bool );

		// Find the smallest zone which contains the point
		PCZone * findZoneForPoint(Vector3 & point);

		// create any zone-specific data necessary for all zones for the given node
		void createZoneSpecificNodeData(PCZSceneNode *);

		// create any zone-specific data necessary for all nodes for the given zone
		void createZoneSpecificNodeData(PCZone *);

		// set the home zone for a scene node
		void setNodeHomeZone(SceneNode *, PCZone *);

        /** Recurses the scene, adding any nodes intersecting with the box into the given list.
        It ignores the exclude scene node.
        */
        void findNodesIn( const AxisAlignedBox &box, 
						  PCZSceneNodeList &list, 
						  PCZone * startZone,
						  PCZSceneNode *exclude = 0 );

        /** Recurses the scene, adding any nodes intersecting with the sphere into the given list.
        It will start in the start SceneNode if given, otherwise, it will start at the root node.
        */
        void findNodesIn( const Sphere &sphere, 
						  PCZSceneNodeList &list, 
						  PCZone * startZone,
						  PCZSceneNode *start = 0 );

        /** Recurses the PCZTree, adding any nodes intersecting with the volume into the given list.
        It will start in the start SceneNode if given, otherwise, it will start at the root node.
        */
        void findNodesIn( const PlaneBoundedVolume &volume, 
						  PCZSceneNodeList &list, 
						  PCZone * startZone,
						  PCZSceneNode *start=0 );

        /** Recurses the scene, starting in the given startZone, adding any nodes intersecting with 
		the ray into the given list.
        It will start in the start SceneNode if given, otherwise, it will start at the root node.
        */
        void findNodesIn( const Ray &ray, 
						  PCZSceneNodeList &list, 
						  PCZone * startZone,
						  PCZSceneNode *start=0 );

		/* Get the default zone */
		PCZone * getDefaultZone(void)
		{
			return mDefaultZone;
		}
		
		/* Get a zone by name */
		PCZone * getZoneByName(const String & zoneName);
		
        /** Sets the portal visibility flag */
        void setShowPortals( bool b )
        {
            mShowPortals = b;
        };

        /** Sets the given option for the SceneManager
                @remarks
            Options are:
            "ShowPortals", bool *;
            "ShowBoundingBoxes", bool *;
        */
        virtual bool setOption( const String &, const void * );
        /** Gets the given option for the Scene Manager.
            @remarks
            See setOption
        */
        virtual bool getOption( const String &, void * );

        bool getOptionValues( const String & key, StringVector &refValueList );
        bool getOptionKeys( StringVector &refKeys );

        /** Overridden from SceneManager */
        AxisAlignedBoxSceneQuery* createAABBQuery(const AxisAlignedBox& box, unsigned long mask = 0xFFFFFFFF);
        SphereSceneQuery* createSphereQuery(const Sphere& sphere, unsigned long mask = 0xFFFFFFFF);
        PlaneBoundedVolumeListSceneQuery* createPlaneBoundedVolumeQuery(const PlaneBoundedVolumeList& volumes, unsigned long mask = 0xFFFFFFFF);
        RaySceneQuery* createRayQuery(const Ray& ray, unsigned long mask = 0xFFFFFFFF);
        IntersectionSceneQuery* createIntersectionQuery(unsigned long mask = 0xFFFFFFFF);
		
		// ZoneMap iterator for read-only access to the zonemap 
		typedef MapIterator<ZoneMap> ZoneIterator;
		ZoneIterator getZoneIterator(void) {return ZoneIterator(mZones.begin(), mZones.end());}

		// clear portal update flag from all zones 
		void _clearAllZonesPortalUpdateFlag(void);   

    protected:
		// type of default zone to be used
		String mDefaultZoneTypeName;

		// name of data file for default zone
		String mDefaultZoneFileName;

		/// list of visible nodes since last _findVisibleObjects()
		NodeList mVisible;

		/// camera of last _findVisibleObjects()
		Camera* mLastActiveCamera;

        /// The root PCZone;
        PCZone *mDefaultZone;

		/// The list of all PCZones
		ZoneMap mZones;

		/// Master list of Portals in the world (includes all portals)
		PortalList mPortals;

		/// Master list of AntiPortals in the world.
		AntiPortalList mAntiPortals;

        /// Portals visibility flag
        bool mShowPortals;

        /// frame counter used in visibility determination
        unsigned long mFrameCount;

		// ZoneFactoryManager instance
		PCZoneFactoryManager * mZoneFactoryManager;

		/// The zone of the active camera (for shadow texture casting use);
		PCZone* mActiveCameraZone;

		/** Internal method for locating a list of lights which could be affecting the frustum. 
		@remarks
			Custom scene managers are encouraged to override this method to make use of their
			scene partitioning scheme to more efficiently locate lights, and to eliminate lights
			which may be occluded by word geometry.
		*/
		virtual void findLightsAffectingFrustum(const Camera* camera);
		/// Internal method for creating shadow textures (texture-based shadows)
		virtual void ensureShadowTexturesCreated();
		/// Internal method for destroying shadow textures (texture-based shadows)
		virtual void destroyShadowTextures(void);
		/// Internal method for preparing shadow textures ready for use in a regular render
		virtual void prepareShadowTextures(Camera* cam, Viewport* vp);
		/// Internal method for firing the pre caster texture shadows event
		virtual void fireShadowTexturesPreCaster(Light* light, Camera* camera, size_t iteration);
    };

    /// Factory for PCZSceneManager
    class PCZSceneManagerFactory : public SceneManagerFactory
    {
    protected:
	    void initMetaData(void) const;
    public:
	    PCZSceneManagerFactory() {}
	    ~PCZSceneManagerFactory() {}
	    /// Factory type name
	    static const String FACTORY_TYPE_NAME;
	    SceneManager* createInstance(const String& instanceName);
	    void destroyInstance(SceneManager* instance);
    };



}

#endif


