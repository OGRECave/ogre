/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#ifndef PCZ_SCENEMANAGER_H
#define PCZ_SCENEMANAGER_H

#include "OgrePCZPrerequisites.h"
#include "OgreSceneManager.h"
#include "OgrePCZone.h"

namespace Ogre
{
    /** \addtogroup Plugins Plugins
    *  @{
    */
    /** \defgroup PCZSceneManager PCZSceneManager
    * SceneManager that uses Portal-Connected-Zones to divide the scene spatially
    *  @{
    */
    class PCZoneFactoryManager;
    class PortalBase;

    typedef std::vector<SceneNode*> NodeList;
    typedef std::list<WireBoundingBox*> BoxList;

    /** Specialized SceneManager that uses Portal-Connected-Zones to divide the scene spatially.
    */

    class _OgrePCZPluginExport PCZSceneManager : public SceneManager, public ShadowTextureListener
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
        const String& getTypeName(void) const override;

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

        SceneNode* createSceneNodeImpl(void) override;
        SceneNode* createSceneNodeImpl(const String& name) override;
        /** Creates a specialized PCZCamera */
        Camera * createCamera( const String &name ) override;

        /** Deletes a scene node by name & corresponding PCZSceneNode */
        void destroySceneNode( const String &name ) override;

        /** Deletes a scene node & corresponding PCZSceneNode */
        void destroySceneNode(SceneNode* sn) override;

        /** Overridden to clean up zones
        */
        void clearScene(void) override;

        /** Overridden from SceneManager */
        void setWorldGeometryRenderQueue(uint8 qid);

        /// Overridden from basic scene manager
        void _renderScene(Camera *cam, Viewport *vp, bool includeOverlays) override;

        /** Enable/disable sky rendering */
        void enableSky(bool);

        /** Set the zone which contains the sky node */
        void setSkyZone(PCZone * zone);

        /** Update Scene Graph (does several things now) */
        void _updateSceneGraph( Camera * cam ) override;

        /** Recurses through the PCZTree determining which nodes are visible. */
        void _findVisibleObjects ( Camera * cam,
            VisibleObjectsBoundsInfo* visibleBounds, bool onlyShadowCasters ) override;

        /** Alerts each unculled object, notifying it that it will be drawn.
        * Useful for doing calculations only on nodes that will be drawn, prior
        * to drawing them...
        */
        virtual void _alertVisibleObjects( void );

        /** Creates a light for use in the scene.

                Lights can either be in a fixed position and independent of the
                scene graph, or they can be attached to SceneNodes so they derive
                their position from the parent node. Either way, they are created
                using this method so that the SceneManager manages their
                existence.
            @param
                name The name of the new light, to identify it later.
        */
        Light* createLight(const String& name) override;

        /** Returns a pointer to the named Light which has previously been added to the scene.
        @note Throws an exception if the named instance does not exist
        */
        Light* getLight(const String& name) const override;

        /** Returns whether a light with the given name exists.
        */
        bool hasLight(const String& name) const override;

        /** Removes the named light from the scene and destroys it.

                Any pointers held to this light after calling this method will be invalid.
        */
        void destroyLight(const String& name) override;

        /** Removes and destroys all lights in the scene.
        */
        void destroyAllLights(void) override;

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

        /** Create a zone with the given name  */
        PCZone * createZone(const String& zoneType, const String& instanceName);

        /** Destroy an existing zone within the scene */
        void destroyZone(PCZone* zone, bool destroySceneNodes);

        /** Make sure the home zone for the PCZSceneNode is up-to-date
        */
        void _updateHomeZone( PCZSceneNode *, bool );

        /// Find the smallest zone which contains the point
        PCZone * findZoneForPoint(Vector3 & point);

        /// Create any zone-specific data necessary for all zones for the given node
        void createZoneSpecificNodeData(PCZSceneNode *);

        /// Create any zone-specific data necessary for all nodes for the given zone
        void createZoneSpecificNodeData(PCZone *);

        /// Set the home zone for a scene node
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

        /** Get the default zone */
        PCZone * getDefaultZone(void)
        {
            return mDefaultZone;
        }
        
        /** Get a zone by name */
        PCZone * getZoneByName(const String & zoneName);
        
        /** Sets the portal visibility flag */
        void setShowPortals( bool b )
        {
            mShowPortals = b;
        };

        /** Sets the given option for the SceneManager

            Options are:
            "ShowPortals", bool *;
            "ShowBoundingBoxes", bool *;
        */
        bool setOption( const String &, const void * ) override;
        /** Gets the given option for the Scene Manager.

            See setOption
        */
        bool getOption( const String &, void * ) override;

        bool getOptionValues( const String & key, StringVector &refValueList ) override;
        bool getOptionKeys( StringVector &refKeys ) override;

        /** Overridden from SceneManager */
        AxisAlignedBoxSceneQuery* createAABBQuery(const AxisAlignedBox& box, uint32 mask = 0xFFFFFFFF) override;
        SphereSceneQuery* createSphereQuery(const Sphere& sphere, uint32 mask = 0xFFFFFFFF) override;
        PlaneBoundedVolumeListSceneQuery* createPlaneBoundedVolumeQuery(const PlaneBoundedVolumeList& volumes, uint32 mask = 0xFFFFFFFF) override;
        RaySceneQuery* createRayQuery(const Ray& ray, uint32 mask = 0xFFFFFFFF) override;
        IntersectionSceneQuery* createIntersectionQuery(uint32 mask = 0xFFFFFFFF) override;
        
        /// ZoneMap iterator for read-only access to the zonemap 
        typedef MapIterator<ZoneMap> ZoneIterator;
        ZoneIterator getZoneIterator(void) {return ZoneIterator(mZones.begin(), mZones.end());}

        /// Clear portal update flag from all zones 
        void _clearAllZonesPortalUpdateFlag(void);

        /// @see SceneManager::prepareShadowTextures.
        void prepareShadowTextures(Camera* cam, Viewport* vp, const LightList* lightList = 0) override;

    protected:
        /// Type of default zone to be used
        String mDefaultZoneTypeName;

        /// Name of data file for default zone
        String mDefaultZoneFileName;

        /// List of visible nodes since last _findVisibleObjects()
        NodeList mVisible;

        /// Camera of last _findVisibleObjects()
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

        /// Frame counter used in visibility determination
        unsigned long mFrameCount;

        /// ZoneFactoryManager instance
        PCZoneFactoryManager * mZoneFactoryManager;

        /// The zone of the active camera (for shadow texture casting use);
        PCZone* mActiveCameraZone;

        /** Internal method for locating a list of lights which could be affecting the frustum. 

            Custom scene managers are encouraged to override this method to make use of their
            scene partitioning scheme to more efficiently locate lights, and to eliminate lights
            which may be occluded by word geometry.
        */
        void findLightsAffectingFrustum(const Camera* camera) override;
        /// Internal method for creating shadow textures (texture-based shadows)
        void ensureShadowTexturesCreated() override;
        /// Internal method for firing the pre caster texture shadows event
        void shadowTextureCasterPreViewProj(Light* light, Camera* camera, size_t iteration) override;
    };

    /// Factory for PCZSceneManager
    class PCZSceneManagerFactory : public SceneManagerFactory
    {
    protected:
        void initMetaData(void) const override;
    public:
        PCZSceneManagerFactory() {}
        ~PCZSceneManagerFactory() {}
        /// Factory type name
        static const String FACTORY_TYPE_NAME;
        SceneManager* createInstance(const String& instanceName) override;
    };


    /** @} */
    /** @} */
}

#endif


