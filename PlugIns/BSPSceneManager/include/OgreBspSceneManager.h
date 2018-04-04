/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#ifndef __BspSceneManager_H__
#define __BspSceneManager_H__


#include "OgreBspPrerequisites.h"
#include "OgreSceneManager.h"
#include "OgreStaticFaceGroup.h"
#include "OgreRenderOperation.h"
#include "OgreBspLevel.h"
#include <set>


namespace Ogre {
    /** \addtogroup Plugins
    *  @{
    */
    /** \addtogroup BSPSceneManager
    *  @{
    */

    /** Specialisation of the SceneManager class to deal with indoor scenes
        based on a BSP tree.
        This class refines the behaviour of the default SceneManager to manage
        a scene whose bulk of geometry is made up of an indoor environment which
        is organised by a Binary Space Partition (BSP) tree.
        A BSP tree progressively subdivides the space using planes which are the nodes of the tree.
        At some point we stop subdividing and everything in the remaining space is part of a 'leaf' which
        contains a number of polygons. Typically we traverse the tree to locate the leaf in which a
        point in space is (say the camera origin) and work from there. A second structure, the
        Potentially Visible Set, tells us which other leaves can been seen from this
        leaf, and we test their bounding boxes against the camera frustum to see which
        we need to draw. Leaves are also a good place to start for collision detection since
        they divide the level into discrete areas for testing.
        This BSP and PVS technique has been made famous by engines such as Quake and Unreal. Ogre
        provides support for loading Quake3 level files to populate your world through this class,
        by calling the BspSceneManager::setWorldGeometry. Note that this interface is made
        available at the top level of the SceneManager class so you don't have to write your code
        specifically for this class - just call Root::getSceneManager passing a SceneType of ST_INTERIOR
        and in the current implementation you will get a BspSceneManager silently disguised as a
        standard SceneManager.
    */
    class BspSceneManager : public SceneManager
    {
    protected:

        /// World geometry
        BspLevelPtr mLevel;

        // State variables for rendering WIP
        // Set of face groups (by index) already included
        typedef std::set<int> FaceGroupSet;
        FaceGroupSet mFaceGroupSet;
        // Material -> face group hashmap
        typedef std::map<Material*, std::vector<StaticFaceGroup*>, materialLess > MaterialFaceGroupMap;
        MaterialFaceGroupMap mMatFaceGroupMap;

        RenderOperation mRenderOp;

        // Debugging features
        bool mShowNodeAABs;
        RenderOperation mAABGeometry;

        /** Walks the BSP tree looking for the node which the camera
            is in, and tags any geometry which is in a visible leaf for
            later processing.
            @param camera Pointer to the viewpoint.
            @return The BSP node the camera was found in, for info.
        */
        BspNode* walkTree(Camera* camera, VisibleObjectsBoundsInfo* visibleBounds, bool onlyShadowCasters);
        /** Tags geometry in the leaf specified for later rendering. */
        void processVisibleLeaf(BspNode* leaf, Camera* cam, 
            VisibleObjectsBoundsInfo* visibleBounds, bool onlyShadowCasters);

        /** Caches a face group for imminent rendering. */
        unsigned int cacheGeometry(unsigned int* pIndexes, const StaticFaceGroup* faceGroup);

        /** Frees up allocated memory for geometry caches. */
        void freeMemory(void);

        /** Adds a bounding box to draw if turned on. */
        void addBoundingBox(const AxisAlignedBox& aab, bool visible);

        /** Renders the static level geometry tagged in walkTree. */
        void renderStaticGeometry(void);

        /** @copydoc SceneManager::clearScene */
        void clearScene(void);

        // Overridden so we can manually render world geometry
        bool fireRenderQueueEnded(uint8 id, const String& invocation);

        typedef std::set<const MovableObject*> MovablesForRendering;
        MovablesForRendering mMovablesForRendering;

    public:
        BspSceneManager(const String& name);
        ~BspSceneManager();


        /// @copydoc SceneManager::getTypeName
        const String& getTypeName(void) const;

        /** Specialised from SceneManager to support Quake3 bsp files. */
        void setWorldGeometry(const String& filename);

        /** Specialised from SceneManager to support Quake3 bsp files. */
        size_t estimateWorldGeometry(const String& filename);
        
        /** Specialised from SceneManager to support Quake3 bsp files. */
        void setWorldGeometry(DataStreamPtr& stream, 
            const String& typeName = BLANKSTRING);

        /** Specialised from SceneManager to support Quake3 bsp files. */
        size_t estimateWorldGeometry(DataStreamPtr& stream, 
            const String& typeName = BLANKSTRING);

        /** Tells the manager whether to draw the axis-aligned boxes that surround
            nodes in the Bsp tree. For debugging purposes.
        */
        void showNodeBoxes(bool show);

        /** Specialised to suggest viewpoints. */
        ViewPoint getSuggestedViewpoint(bool random = false);

        const BspLevelPtr& getLevel(void) {return mLevel; }

        void setLevel(const BspLevelPtr& level);

        /** Overridden from SceneManager. */
        void _findVisibleObjects(Camera* cam, VisibleObjectsBoundsInfo* visibleBounds, 
            bool onlyShadowCasters);

        /** Creates a specialized BspSceneNode */
        SceneNode * createSceneNodeImpl ( void );
        /** Creates a specialized BspSceneNode */
        SceneNode * createSceneNodeImpl ( const String &name );

        /** Internal method for tagging BspNodes with objects which intersect them. */
        void _notifyObjectMoved(const MovableObject* mov, const Vector3& pos);
        /** Internal method for notifying the level that an object has been detached from a node */
        void _notifyObjectDetached(const MovableObject* mov);

        /* Creates an AxisAlignedBoxSceneQuery for this scene manager. 
        @remarks
            This method creates a new instance of a query object for this scene manager, 
            for an axis aligned box region. See SceneQuery and AxisAlignedBoxSceneQuery 
            for full details.
        @par
            The instance returned from this method must be destroyed by calling
            SceneManager::destroyQuery when it is no longer required.
        @param box Details of the box which describes the region for this query.
        @param mask The query mask to apply to this query; can be used to filter out
            certain objects; see SceneQuery for details.
        */
        /*
        virtual AxisAlignedBoxSceneQuery* 
            createAABBQuery(const AxisAlignedBox& box, uint32 mask = 0xFFFFFFFF);
        */
        /* Creates a SphereSceneQuery for this scene manager. 
        @remarks
            This method creates a new instance of a query object for this scene manager, 
            for a spherical region. See SceneQuery and SphereSceneQuery 
            for full details.
        @par
            The instance returned from this method must be destroyed by calling
            SceneManager::destroyQuery when it is no longer required.
        @param sphere Details of the sphere which describes the region for this query.
        @param mask The query mask to apply to this query; can be used to filter out
            certain objects; see SceneQuery for details.
        */
        /*
        virtual SphereSceneQuery* 
            createSphereQuery(const Sphere& sphere, uint32 mask = 0xFFFFFFFF);
        */
        /** Creates a RaySceneQuery for this scene manager. 
        @remarks
            This method creates a new instance of a query object for this scene manager, 
            looking for objects which fall along a ray. See SceneQuery and RaySceneQuery 
            for full details.
        @par
            The instance returned from this method must be destroyed by calling
            SceneManager::destroyQuery when it is no longer required.
        @param ray Details of the ray which describes the region for this query.
        @param mask The query mask to apply to this query; can be used to filter out
            certain objects; see SceneQuery for details.
        */
        virtual RaySceneQuery* 
            createRayQuery(const Ray& ray, uint32 mask = 0xFFFFFFFF);
        /** Creates an IntersectionSceneQuery for this scene manager. 
        @remarks
            This method creates a new instance of a query object for locating
            intersecting objects. See SceneQuery and IntersectionSceneQuery
            for full details.
        @par
            The instance returned from this method must be destroyed by calling
            SceneManager::destroyQuery when it is no longer required.
        @param mask The query mask to apply to this query; can be used to filter out
            certain objects; see SceneQuery for details.
        */
        virtual IntersectionSceneQuery* 
            createIntersectionQuery(uint32 mask = 0xFFFFFFFF);

    };

    /** BSP specialisation of IntersectionSceneQuery */
    class BspIntersectionSceneQuery : public DefaultIntersectionSceneQuery
    {
    public:
        BspIntersectionSceneQuery(SceneManager* creator);

        /** See IntersectionSceneQuery. */
        void execute(IntersectionSceneQueryListener* listener);

    };

    /** BSP specialisation of RaySceneQuery */
    class BspRaySceneQuery : public DefaultRaySceneQuery
    {
    public:
        BspRaySceneQuery(SceneManager* creator);
        ~BspRaySceneQuery();

        /** See RaySceneQuery. */
        void execute(RaySceneQueryListener* listener);
    protected:
        /// Set for eliminating duplicates since objects can be in > 1 node
        std::set<MovableObject*> mObjsThisQuery;
        /// list of the last single intersection world fragments (derived)
        std::vector<SceneQuery::WorldFragment*> mSingleIntersections;

        void clearTemporaries(void);
        /** Internal processing of a single node.
        @return true if we should continue tracing, false otherwise
        */
        bool processNode(const BspNode* node, const Ray& tracingRay, RaySceneQueryListener* listener,
            Real maxDistance = Math::POS_INFINITY, Real traceDistance = 0.0f);
        /** Internal processing of a single leaf.
        @return true if we should continue tracing, false otherwise
        */
        bool processLeaf(const BspNode* node, const Ray& tracingRay, RaySceneQueryListener* listener,
            Real maxDistance = Math::POS_INFINITY, Real traceDistance = 0.0f);

    };

    /// Factory for BspSceneManager
    class BspSceneManagerFactory : public SceneManagerFactory
    {
    protected:
        void initMetaData(void) const;
    public:
        BspSceneManagerFactory() {}
        ~BspSceneManagerFactory() {}
        /// Factory type name
        static const String FACTORY_TYPE_NAME;
        SceneManager* createInstance(const String& instanceName);
        void destroyInstance(SceneManager* instance);
    };
    /** @} */
    /** @} */
}

#endif
