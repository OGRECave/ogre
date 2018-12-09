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
#ifndef __SceneQuery_H__
#define __SceneQuery_H__

#include "OgrePrerequisites.h"
#include "OgreSphere.h"
#include "OgreRay.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */
    /** A class for performing queries on a scene.
    @remarks
        This is an abstract class for performing a query on a scene, i.e. to retrieve
        a list of objects and/or world geometry sections which are potentially intersecting a
        given region. Note the use of the word 'potentially': the results of a scene query
        are generated based on bounding volumes, and as such are not correct at a triangle
        level; the user of the SceneQuery is expected to filter the results further if
        greater accuracy is required.
    @par
        Different SceneManagers will implement these queries in different ways to
        exploit their particular scene organisation, and thus will provide their own
        concrete subclasses. In fact, these subclasses will be derived from subclasses
        of this class rather than directly because there will be region-type classes
        in between.
    @par
        These queries could have just been implemented as methods on the SceneManager,
        however, they are wrapped up as objects to allow 'compilation' of queries
        if deemed appropriate by the implementation; i.e. each concrete subclass may
        precalculate information (such as fixed scene partitions involved in the query)
        to speed up the repeated use of the query.
    @par
        You should never try to create a SceneQuery object yourself, they should be created
        using the SceneManager interfaces for the type of query required, e.g.
        SceneManager::createSphereSceneQuery.
    */
    class _OgreExport SceneQuery : public SceneMgtAlloc
    {
    public:
        /** This type can be used by collaborating applications & SceneManagers to 
            agree on the type of world geometry to be returned from queries. Not all
            these types will be supported by all SceneManagers; once the application
            has decided which SceneManager specialisation to use, it is expected that 
            it will know which type of world geometry abstraction is available to it.
        */
        enum WorldFragmentType {
            /// Return no world geometry hits at all
            WFT_NONE,
            /// Return pointers to convex plane-bounded regions
            WFT_PLANE_BOUNDED_REGION,
            /// Return a single intersection point (typically RaySceneQuery only)
            WFT_SINGLE_INTERSECTION,
            /// Custom geometry as defined by the SceneManager
            WFT_CUSTOM_GEOMETRY,
            /// General RenderOperation structure
            WFT_RENDER_OPERATION
        };

        /** Represents part of the world geometry that is a result of a SceneQuery. 
        @remarks
            Since world geometry is normally vast and sprawling, we need a way of
            retrieving parts of it based on a query. That is what this struct is for;
            note there are potentially as many data structures for world geometry as there
            are SceneManagers, however this structure includes a few common abstractions as 
            well as a more general format.
        @par
            The type of world fragment that is returned from a query depends on the
            SceneManager, and the option set using SceneQuery::setWorldFragmentType. 
            You can see what fragment types are supported on the query in question by
            calling SceneQuery::getSupportedWorldFragmentTypes().
        */
        struct WorldFragment {
            /// The type of this world fragment
            WorldFragmentType fragmentType;
            /// Single intersection point, only applicable for WFT_SINGLE_INTERSECTION
            Vector3 singleIntersection;
            /// Planes bounding a convex region, only applicable for WFT_PLANE_BOUNDED_REGION
            std::vector<Plane>* planes;
            /// Custom geometry block, only applicable for WFT_CUSTOM_GEOMETRY
            void* geometry;
            /// General render operation structure, fallback if nothing else is available
            RenderOperation* renderOp;
            
        };
    protected:
        SceneManager* mParentSceneMgr;
        uint32 mQueryMask;
        uint32 mQueryTypeMask;
        std::set<WorldFragmentType> mSupportedWorldFragments;
        WorldFragmentType mWorldFragmentType;
    
    public:
        /** Standard constructor, should be called by SceneManager. */
        SceneQuery(SceneManager* mgr);
        virtual ~SceneQuery();

        /** Sets the mask for results of this query.
        @remarks
            This method allows you to set a 'mask' to limit the results of this
            query to certain types of result. The actual meaning of this value is
            up to the application; basically MovableObject instances will only be returned
            from this query if a bitwise AND operation between this mask value and the
            MovableObject::getQueryFlags value is non-zero. The application will
            have to decide what each of the bits means.
        */
        virtual void setQueryMask(uint32 mask);
        /** Returns the current mask for this query. */
        virtual uint32 getQueryMask(void) const;

        /** Sets the type mask for results of this query.
        @remarks
            This method allows you to set a 'type mask' to limit the results of this
            query to certain types of objects. Whilst setQueryMask deals with flags
            set per instance of object, this method deals with setting a mask on 
            flags set per type of object. Both may exclude an object from query
            results.
        */
        virtual void setQueryTypeMask(uint32 mask);
        /** Returns the current mask for this query. */
        virtual uint32 getQueryTypeMask(void) const;

        /** Tells the query what kind of world geometry to return from queries;
            often the full renderable geometry is not what is needed. 
        @remarks
            The application receiving the world geometry is expected to know 
            what to do with it; inevitably this means that the application must 
            have knowledge of at least some of the structures
            used by the custom SceneManager.
        @par
            The default setting is WFT_NONE.
        */
        virtual void setWorldFragmentType(enum WorldFragmentType wft);

        /** Gets the current world fragment types to be returned from the query. */
        virtual WorldFragmentType getWorldFragmentType(void) const;

        /** Returns the types of world fragments this query supports. */
        virtual const std::set<WorldFragmentType>* getSupportedWorldFragmentTypes(void) const
            {return &mSupportedWorldFragments;}

        
    };

    /** This optional class allows you to receive per-result callbacks from
        SceneQuery executions instead of a single set of consolidated results.
    @remarks
        You should override this with your own subclass. Note that certain query
        classes may refine this listener interface.
    */
    class _OgreExport SceneQueryListener
    {
    public:
        virtual ~SceneQueryListener() { }
        /** Called when a MovableObject is returned by a query.
        @remarks
            The implementor should return 'true' to continue returning objects,
            or 'false' to abandon any further results from this query.
        */
        virtual bool queryResult(MovableObject* object) = 0;
        /** Called when a WorldFragment is returned by a query.
        @remarks
            The implementor should return 'true' to continue returning objects,
            or 'false' to abandon any further results from this query.
        */
        virtual bool queryResult(SceneQuery::WorldFragment* fragment) = 0;

    };

    typedef std::list<MovableObject*> SceneQueryResultMovableList;
    typedef std::list<SceneQuery::WorldFragment*> SceneQueryResultWorldFragmentList;
    /** Holds the results of a scene query. */
    struct _OgreExport SceneQueryResult : public SceneMgtAlloc
    {
        /// List of movable objects in the query (entities, particle systems etc)
        SceneQueryResultMovableList movables;
        /// List of world fragments
        SceneQueryResultWorldFragmentList worldFragments;
    };

    /** Abstract class defining a query which returns single results from a region. 
    @remarks
        This class is simply a generalisation of the subtypes of query that return 
        a set of individual results in a region. See the SceneQuery class for abstract
        information, and subclasses for the detail of each query type.
    */
    class _OgreExport RegionSceneQuery
        : public SceneQuery, public SceneQueryListener
    {
    protected:
        SceneQueryResult* mLastResult;
    public:
        /** Standard constructor, should be called by SceneManager. */
        RegionSceneQuery(SceneManager* mgr);
        virtual ~RegionSceneQuery();
        /** Executes the query, returning the results back in one list.
        @remarks
            This method executes the scene query as configured, gathers the results
            into one structure and returns a reference to that structure. These
            results will also persist in this query object until the next query is
            executed, or clearResults() is called. An more lightweight version of
            this method that returns results through a listener is also available.
        */
        virtual SceneQueryResult& execute(void);

        /** Executes the query and returns each match through a listener interface. 
        @remarks
            Note that this method does not store the results of the query internally 
            so does not update the 'last result' value. This means that this version of
            execute is more lightweight and therefore more efficient than the version 
            which returns the results as a collection.
        */
        virtual void execute(SceneQueryListener* listener) = 0;
        
        /** Gets the results of the last query that was run using this object, provided
            the query was executed using the collection-returning version of execute. 
        */
        virtual SceneQueryResult& getLastResults(void) const;
        /** Clears the results of the last query execution.
        @remarks
            You only need to call this if you specifically want to free up the memory
            used by this object to hold the last query results. This object clears the
            results itself when executing and when destroying itself.
        */
        virtual void clearResults(void);

        /** Self-callback in order to deal with execute which returns collection. */
        bool queryResult(MovableObject* first);
        /** Self-callback in order to deal with execute which returns collection. */
        bool queryResult(SceneQuery::WorldFragment* fragment);
    };

    /** Specialises the SceneQuery class for querying within an axis aligned box. */
    class _OgreExport AxisAlignedBoxSceneQuery : public RegionSceneQuery
    {
    protected:
        AxisAlignedBox mAABB;
    public:
        AxisAlignedBoxSceneQuery(SceneManager* mgr);
        virtual ~AxisAlignedBoxSceneQuery();

        /** Sets the size of the box you wish to query. */
        void setBox(const AxisAlignedBox& box);

        /** Gets the box which is being used for this query. */
        const AxisAlignedBox& getBox(void) const;

    };

    /** Specialises the SceneQuery class for querying within a sphere. */
    class _OgreExport SphereSceneQuery : public RegionSceneQuery
    {
    protected:
        Sphere mSphere;
    public:
        SphereSceneQuery(SceneManager* mgr);
        virtual ~SphereSceneQuery();
        /** Sets the sphere which is to be used for this query. */
        void setSphere(const Sphere& sphere);

        /** Gets the sphere which is being used for this query. */
        const Sphere& getSphere() const;

    };

    /** Specialises the SceneQuery class for querying within a plane-bounded volume. 
    */
    class _OgreExport PlaneBoundedVolumeListSceneQuery : public RegionSceneQuery
    {
    protected:
        PlaneBoundedVolumeList mVolumes;
    public:
        PlaneBoundedVolumeListSceneQuery(SceneManager* mgr);
        virtual ~PlaneBoundedVolumeListSceneQuery();
        /** Sets the volume which is to be used for this query. */
        void setVolumes(const PlaneBoundedVolumeList& volumes);

        /** Gets the volume which is being used for this query. */
        const PlaneBoundedVolumeList& getVolumes() const;

    };


    /*
    /// Specialises the SceneQuery class for querying within a pyramid. 
    class _OgreExport PyramidSceneQuery : public RegionSceneQuery
    {
    public:
        PyramidSceneQuery(SceneManager* mgr);
        virtual ~PyramidSceneQuery();
    };
    */

    /** Alternative listener class for dealing with RaySceneQuery.
    @remarks
        Because the RaySceneQuery returns results in an extra bit of information, namely
        distance, the listener interface must be customised from the standard SceneQueryListener.
    */
    class _OgreExport RaySceneQueryListener 
    {
    public:
        virtual ~RaySceneQueryListener() { }
        /** Called when a movable objects intersects the ray.
        @remarks
            As with SceneQueryListener, the implementor of this method should return 'true'
            if further results are required, or 'false' to abandon any further results from
            the current query.
        */
        virtual bool queryResult(MovableObject* obj, Real distance) = 0;

        /** Called when a world fragment is intersected by the ray. 
        @remarks
            As with SceneQueryListener, the implementor of this method should return 'true'
            if further results are required, or 'false' to abandon any further results from
            the current query.
        */
        virtual bool queryResult(SceneQuery::WorldFragment* fragment, Real distance) = 0;

    };
      
    /** This struct allows a single comparison of result data no matter what the type */
    struct _OgreExport RaySceneQueryResultEntry
    {
        /// Distance along the ray
        Real distance;
        /// The movable, or NULL if this is not a movable result
        MovableObject* movable;
        /// The world fragment, or NULL if this is not a fragment result
        SceneQuery::WorldFragment* worldFragment;
        /// Comparison operator for sorting
        bool operator < (const RaySceneQueryResultEntry& rhs) const
        {
            return this->distance < rhs.distance;
        }

    };
    typedef std::vector<RaySceneQueryResultEntry> RaySceneQueryResult;

    /** Specialises the SceneQuery class for querying along a ray. */
    class _OgreExport RaySceneQuery : public SceneQuery, public RaySceneQueryListener
    {
    protected:
        Ray mRay;
        bool mSortByDistance;
        ushort mMaxResults;
        RaySceneQueryResult mResult;

    public:
        RaySceneQuery(SceneManager* mgr);
        virtual ~RaySceneQuery();
        /** Sets the ray which is to be used for this query. */
        virtual void setRay(const Ray& ray);
        /** Gets the ray which is to be used for this query. */
        virtual const Ray& getRay(void) const;
        /** Sets whether the results of this query will be sorted by distance along the ray.
        @remarks
            Often you want to know what was the first object a ray intersected with, and this 
            method allows you to ask the query to sort the results so that the nearest results
            are listed first.
        @par
            Note that because the query returns results based on bounding volumes, the ray may not
            actually intersect the detail of the objects returned from the query, just their 
            bounding volumes. For this reason the caller is advised to use more detailed 
            intersection tests on the results if a more accurate result is required; OGRE uses 
            bounds checking in order to give the most speedy results since not all applications 
            need extreme accuracy.
        @param sort If true, results will be sorted.
        @param maxresults If sorting is enabled, this value can be used to constrain the maximum number
            of results that are returned. Please note (as above) that the use of bounding volumes mean that
            accuracy is not guaranteed; if in doubt, allow more results and filter them in more detail.
            0 means unlimited results.
        */
        virtual void setSortByDistance(bool sort, ushort maxresults = 0);
        /** Gets whether the results are sorted by distance. */
        virtual bool getSortByDistance(void) const;
        /** Gets the maximum number of results returned from the query (only relevant if 
        results are being sorted) */
        virtual ushort getMaxResults(void) const;
        /** Executes the query, returning the results back in one list.
        @remarks
            This method executes the scene query as configured, gathers the results
            into one structure and returns a reference to that structure. These
            results will also persist in this query object until the next query is
            executed, or clearResults() is called. An more lightweight version of
            this method that returns results through a listener is also available.
        */
        virtual RaySceneQueryResult& execute(void);

        /** Executes the query and returns each match through a listener interface. 
        @remarks
            Note that this method does not store the results of the query internally 
            so does not update the 'last result' value. This means that this version of
            execute is more lightweight and therefore more efficient than the version 
            which returns the results as a collection.
        */
        virtual void execute(RaySceneQueryListener* listener) = 0;

        /** Gets the results of the last query that was run using this object, provided
            the query was executed using the collection-returning version of execute. 
        */
        virtual RaySceneQueryResult& getLastResults(void);
        /** Clears the results of the last query execution.
        @remarks
            You only need to call this if you specifically want to free up the memory
            used by this object to hold the last query results. This object clears the
            results itself when executing and when destroying itself.
        */
        virtual void clearResults(void);

        /** Self-callback in order to deal with execute which returns collection. */
        bool queryResult(MovableObject* obj, Real distance);
        /** Self-callback in order to deal with execute which returns collection. */
        bool queryResult(SceneQuery::WorldFragment* fragment, Real distance);




    };

    /** Alternative listener class for dealing with IntersectionSceneQuery.
    @remarks
        Because the IntersectionSceneQuery returns results in pairs, rather than singularly,
        the listener interface must be customised from the standard SceneQueryListener.
    */
    class _OgreExport IntersectionSceneQueryListener 
    {
    public:
        virtual ~IntersectionSceneQueryListener() { }
        /** Called when 2 movable objects intersect one another.
        @remarks
            As with SceneQueryListener, the implementor of this method should return 'true'
            if further results are required, or 'false' to abandon any further results from
            the current query.
        */
        virtual bool queryResult(MovableObject* first, MovableObject* second) = 0;

        /** Called when a movable intersects a world fragment. 
        @remarks
            As with SceneQueryListener, the implementor of this method should return 'true'
            if further results are required, or 'false' to abandon any further results from
            the current query.
        */
        virtual bool queryResult(MovableObject* movable, SceneQuery::WorldFragment* fragment) = 0;

        /* NB there are no results for world fragments intersecting other world fragments;
           it is assumed that world geometry is either static or at least that self-intersections
           are irrelevant or dealt with elsewhere (such as the custom scene manager) */
        
    
    };
        
    typedef std::pair<MovableObject*, MovableObject*> SceneQueryMovableObjectPair;
    typedef std::pair<MovableObject*, SceneQuery::WorldFragment*> SceneQueryMovableObjectWorldFragmentPair;
    typedef std::list<SceneQueryMovableObjectPair> SceneQueryMovableIntersectionList;
    typedef std::list<SceneQueryMovableObjectWorldFragmentPair> SceneQueryMovableWorldFragmentIntersectionList;
    /** Holds the results of an intersection scene query (pair values). */
    struct _OgreExport IntersectionSceneQueryResult : public SceneMgtAlloc
    {
        /// List of movable / movable intersections (entities, particle systems etc)
        SceneQueryMovableIntersectionList movables2movables;
        /// List of movable / world intersections
        SceneQueryMovableWorldFragmentIntersectionList movables2world;
        
        

    };

    /** Separate SceneQuery class to query for pairs of objects which are
        possibly intersecting one another.
    @remarks
        This SceneQuery subclass considers the whole world and returns pairs of objects
        which are close enough to each other that they may be intersecting. Because of
        this slightly different focus, the return types and listener interface are
        different for this class.
    */
    class _OgreExport IntersectionSceneQuery
        : public SceneQuery, public IntersectionSceneQueryListener 
    {
    protected:
        IntersectionSceneQueryResult* mLastResult;
    public:
        IntersectionSceneQuery(SceneManager* mgr);
        virtual ~IntersectionSceneQuery();

        /** Executes the query, returning the results back in one list.
        @remarks
            This method executes the scene query as configured, gathers the results
            into one structure and returns a reference to that structure. These
            results will also persist in this query object until the next query is
            executed, or clearResults() is called. An more lightweight version of
            this method that returns results through a listener is also available.
        */
        virtual IntersectionSceneQueryResult& execute(void);

        /** Executes the query and returns each match through a listener interface. 
        @remarks
            Note that this method does not store the results of the query internally 
            so does not update the 'last result' value. This means that this version of
            execute is more lightweight and therefore more efficient than the version 
            which returns the results as a collection.
        */
        virtual void execute(IntersectionSceneQueryListener* listener) = 0;

        /** Gets the results of the last query that was run using this object, provided
            the query was executed using the collection-returning version of execute. 
        */
        virtual IntersectionSceneQueryResult& getLastResults(void) const;
        /** Clears the results of the last query execution.
        @remarks
            You only need to call this if you specifically want to free up the memory
            used by this object to hold the last query results. This object clears the
            results itself when executing and when destroying itself.
        */
        virtual void clearResults(void);

        /** Self-callback in order to deal with execute which returns collection. */
        bool queryResult(MovableObject* first, MovableObject* second);
        /** Self-callback in order to deal with execute which returns collection. */
        bool queryResult(MovableObject* movable, SceneQuery::WorldFragment* fragment);
    };
    
    /** @} */
    /** @} */

}
    
#include "OgreHeaderSuffix.h"

#endif
