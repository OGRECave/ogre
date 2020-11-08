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

#ifndef __Ogre_TerrainGroup_H__
#define __Ogre_TerrainGroup_H__

#include "OgreTerrainPrerequisites.h"
#include "OgreTerrain.h"
#include "OgreWorkQueue.h"
#include "OgreIteratorWrapper.h"
#include "OgreConfigFile.h"

namespace Ogre
{
    class TerrainAutoUpdateLod;

    /** \addtogroup Optional
    *  @{
    */
    /** \addtogroup Terrain
    *  Some details on the terrain component
    *  @{
    */

    /** Helper class to assist you in managing multiple terrain instances
        that are connected to each other. 
    @remarks
        This class makes it easy to specify the same options for a group of
        terrain instances and have them positioned relative to each other and
        associated via each other's neighbour connections. You can do all this
        manually but this class just makes things easier, so that you only 
        have to specify most options once. 
    @par
        Terrains are maintained in a grid of entries, and for simplicity
        the grid cells are indexed from 0 as a 'centre' slot, supporting both 
        positive and negative values. so (0,0) is the centre slot, (1,0) is the
        slot to the right of the centre, (1,0) is the slot above the centre, (-2,1) 
        is the slot two to the left of the centre and one up, etc. The maximum
        extent of each axis is -32768 to +32767, so in other words enough for
        over 4 billion entries. That should be enough for anyone!
    @par
        Note that this is not a 'paging' class as such. It's simply a way to make it easier to 
        perform common tasks with multiple terrain instances, which you choose when 
        to define, load and remove. Automatic paging is handled separately by the Paging
        component. 
    */
    class _OgreTerrainExport TerrainGroup : public WorkQueue::RequestHandler, 
        public WorkQueue::ResponseHandler, public TerrainAlloc
    {
    public:
        /** Constructor.
        @param sm The SceneManager which will parent the terrain instances. 
        @param align The alignment that all terrain instances will use
        @param terrainSize The size of each terrain down one edge in vertices (2^n+1)
        @param terrainWorldSize The world size of each terrain instance
        */
        TerrainGroup(SceneManager* sm, Terrain::Alignment align, uint16 terrainSize, 
            Real terrainWorldSize);
        /** Alternate constructor.
        @remarks
            You can ONLY use this constructor if you subsequently call loadGroupDefinition
            or loadLegacyTerrain to populate the rest.
        */
        TerrainGroup(SceneManager* sm);
        virtual ~TerrainGroup();

        /** Retrieve a shared structure which will provide the base settings for
            all terrains created via this group.
        @remarks
            All neighbouring terrains should have the same basic settings (particularly
            the size parameters) - to avoid having to set the terrain import information 
            more than once, you can retrieve the standard settings for this group
            here and modify them to your needs. Once you've done that you can 
            use the shortcut methods in this class to create new terrain instances
            using these base settings (plus any per-instance settings you might
            want to use). 
        @note 
            The structure returned from this method is intended for in-place modification, 
            that's why it is not const and there is no equivalent 'set' method.
            You should not, however, change the alignment or any of the size parameters 
            after you start constructing instances, since neighbouring terrains
            should have the same size & alignment.
        */
        virtual Terrain::ImportData& getDefaultImportSettings() { return mDefaultImportData; }

        /** Define the centre position of the grid of terrain.
        */
        virtual void setOrigin(const Vector3& pos);

        /** Retrieve the centre position of the grid of terrain.
        */
        virtual const Vector3& getOrigin() const { return mOrigin; }

        /** Retrieve the alignment of the grid of terrain (cannot be modified after construction).
        */
        virtual Terrain::Alignment getAlignment() const { return mAlignment; }

        /** Retrieve the world size of each terrain instance 
        */
        virtual Real getTerrainWorldSize() const { return mTerrainWorldSize; }
        /** Set the world size of terrain. 
        @note This will cause the terrains to change position due to their size change
        @param newWorldSize the new world size of each terrain instance 
        */
        virtual void setTerrainWorldSize(Real newWorldSize);
        /** Retrieve the size of each terrain instance in number of vertices down one side
        */
        virtual uint16 getTerrainSize() const { return mTerrainSize; }
        /** Set the size of each terrain instance in number of vertices down one side. 
        @note This will cause the height data in each nested terrain to be bilinear
            filtered to fit the new data size.
        @param newTerrainSize the new map size of each terrain instance 
        */
        virtual void setTerrainSize(uint16 newTerrainSize);
        /** Retrieve the SceneManager being used for this group.
        */
        virtual SceneManager* getSceneManager() const { return mSceneManager; }

        /** Set the naming convention for file names in this terrain group. 
        @remarks
            You can more easily generate file names for saved / loaded terrain slots
            if you define a naming prefix. When you call saveAllTerrains(), all the
            terrain instances currently loaded will be saved to a file named
            &lt;prefix&gt;_&lt;index&gt;.&lt;extension&gt;, where &lt;index&gt; is
            given by packing the x and y coordinates of the entry into a 32-bit
            index (@see packIndex). 
        */
        void setFilenameConvention(const String& prefix, const String& extension);
        /// @see setFilenameConvention
        void setFilenamePrefix(const String& prefix);
        /// @see setFilenameConvention
        void setFilenameExtension(const String& extension);
        /// @see setFilenameConvention
        const String& getFilenamePrefix() const { return mFilenamePrefix; }
        /// @see setFilenameConvention
        const String& getFilenameExtension() const { return mFilenameExtension; }

        /** Set the resource group in which files will be located. */
        void setResourceGroup(const String& grp) { mResourceGroup = grp; }
        /** Get the resource group in which files will be located. */
        const String& getResourceGroup() const { return mResourceGroup; }
        /** Define a 'slot' in the terrain grid - in this case to be loaded from 
            a generated file name.
        @remarks
            At this stage the terrain instance isn't actually present in the grid, 
            you're merely expressing an intention for it to take its place there
            once it's loaded. The reason we do it like this is to support
            background preparation of this terrain instance. 
        @note This method assumes that you want a file name to be generated from 
            the naming convention that you have supplied (@see setFilenameConvention).
            If a file of that name isn't found during loading, then a flat terrain is
            created instead at height 0.
        @param x, y The coordinates of the terrain slot relative to the centre slot (signed).
        */
        virtual void defineTerrain(long x, long y);

        /** Define a 'slot' in the terrain grid - in this case a flat terrain.
        @remarks
            At this stage the terrain instance isn't actually present in the grid, 
            you're merely expressing an intention for it to take its place there
            once it's loaded. The reason we do it like this is to support
            background preparation of this terrain instance. 
        @param x, y The coordinates of the terrain slot relative to the centre slot (signed).
        @param constantHeight The constant, uniform height that you want the terrain
            to start at
        */
        virtual void defineTerrain(long x, long y, float constantHeight);

        /** Define the content of a 'slot' in the terrain grid.
        @remarks
            At this stage the terrain instance isn't actually present in the grid, 
            you're merely expressing an intention for it to take its place there
            once it's loaded. The reason we do it like this is to support
            background preparation of this terrain instance. 
        @param x, y The coordinates of the terrain slot relative to the centre slot (signed).
        @param importData Import data - this data is copied during the
            call so  you may destroy your copy afterwards.
        */
        virtual void defineTerrain(long x, long y, const Terrain::ImportData* importData);

        /** Define the content of a 'slot' in the terrain grid.
        @remarks
            At this stage the terrain instance isn't actually present in the grid, 
            you're merely expressing an intention for it to take its place there
            once it's loaded. The reason we do it like this is to support
            background preparation of this terrain instance. 
        @param x, y The coordinates of the terrain slot relative to the centre slot (signed).
        @param img Heightfield image - this data is copied during the call so  you may destroy your copy afterwards.
        @param layers Optional texture layers to use (if not supplied, default import
            data layers will be used) - this data is copied during the
            call so  you may destroy your copy afterwards.
        */
        virtual void defineTerrain(long x, long y, const Image* img, const Terrain::LayerInstanceList* layers = 0);

        /** Define the content of a 'slot' in the terrain grid.
        @remarks
            At this stage the terrain instance isn't actually present in the grid, 
            you're merely expressing an intention for it to take its place there
            once it's loaded. The reason we do it like this is to support
            background preparation of this terrain instance. 
        @param x, y The coordinates of the terrain slot relative to the centre slot (signed).
        @param pFloat Heights array 
        @param layers Optional texture layers to use (if not supplied, default import
            data layers will be used) - this data is copied during the
            call so  you may destroy your copy afterwards.
        */
        virtual void defineTerrain(long x, long y, const float* pFloat, const Terrain::LayerInstanceList* layers = 0);

        /** Define the content of a 'slot' in the terrain grid.
        @remarks
            At this stage the terrain instance isn't actually present in the grid, 
            you're merely expressing an intention for it to take its place there
            once it's loaded. The reason we do it like this is to support
            background preparation of this terrain instance. 
        @param x, y The coordinates of the terrain slot relative to the centre slot (signed).
        @param filename The name of a file which fully defines the terrain (as 
            written by Terrain::save). Size settings from all files must agree.
        */
        virtual void defineTerrain(long x, long y, const String& filename);


        /** Load any terrain instances that have been defined but not loaded yet. 
        @param synchronous Whether we should force this to happen entirely in the
            primary thread
        */
        virtual void loadAllTerrains(bool synchronous = false);
        
        /** Load a specific terrain slot based on the definition that has already 
            been supplied.
        @param x, y The coordinates of the terrain slot relative to the centre slot (signed).
        @copydetails loadAllTerrains
        */
        virtual void loadTerrain(long x, long y, bool synchronous = false);
        
        /** Load a terrain.cfg as used by the terrain scene manager into a single terrain slot
         *
         * automatically configures the SM2Profile if it is used.
         * @attention not all of the legacy parameters/ parameter combinations are supported
         * @param cfgFilename .cfg file that specifices what textures/scale/mipmaps/etc to use.
         * @param x, y The coordinates of the terrain slot relative to the centre slot (signed).
         * @copydetails loadAllTerrains
         */
        void loadLegacyTerrain(const String& cfgFilename, long x = 0, long y = 0, bool synchronous = true);

        /// @overload
        void loadLegacyTerrain(const ConfigFile& cfg, long x = 0, long y = 0, bool synchronous = true);

        /** Unload a specific terrain slot.
        @remarks
            This destroys the Terrain instance but retains the slot definition (so
            it would be reloaded next time you call loadAllTerrains() if you did not
            remove it beforehand).
        @note
            While the definition of the terrain is kept, if you used import data
            to populate it, this will have been lost so repeat loading cannot occur. 
            The only way to support repeat loading is via the 'filename' option of
            defineTerrain instead.
        @param x, y The coordinates of the terrain slot relative to the centre slot (signed).
        */
        virtual void unloadTerrain(long x, long y);

        /** Remove a specific terrain slot.
        @remarks
            This destroys any Terrain instance at this position and also removes the 
            definition, so it essentially no longer exists. 
        @param x, y The coordinates of the terrain slot relative to the centre slot (signed).
        */
        virtual void removeTerrain(long x, long y);

        /** Remove all terrain instances. 
        */
        void removeAllTerrains();

        /** Save all terrain instances using the assigned file names, or
            via the filename convention.
        @see setFilenameConvention
        @see setResourceGroup
        @param onlyIfModified If true, only terrains that have been modified since load(),
            or since the last save(), will be saved. You want to set this to true if
            you loaded the terrain from these same files, but false if you 
            defined them using some other input data since the files wouldn't exist.
        @param replaceManualFilenames If true, replaces any manually defined filenames
            in the TerrainSlotDefinition with the generated names from the convention.
            This is recommended since it makes everything more consistent, although
            you might want to use manual filenames in the original definition to import 
            previously separate data. 
        */
        void saveAllTerrains(bool onlyIfModified, bool replaceManualFilenames = true);
        
        /** Definition of how to populate a 'slot' in the terrain group.
        */
        struct _OgreTerrainExport TerrainSlotDefinition
        {
            /// Filename, if this is to be loaded from a file
            String filename;
            /// Import data, if this is to be defined based on importing
            Terrain::ImportData* importData;

            TerrainSlotDefinition() :importData(0) {}
            ~TerrainSlotDefinition();

            /// Set to use import data 
            void useImportData();
            /// Set to use file name
            void useFilename();
            /// Destroy temp import resources
            void freeImportData();
        };

        /** Slot for a terrain instance, together with its definition. */
        struct _OgreTerrainExport TerrainSlot : public TerrainAlloc
        {
            /// The coordinates of the terrain slot relative to the centre slot (signed).
            long x, y;
            /// Definition used to load the terrain
            TerrainSlotDefinition def;
            /// Actual terrain instance
            Terrain* instance;

            TerrainSlot(long _x, long _y) : x(_x), y(_y), instance(0) {}
            virtual ~TerrainSlot();
            void freeInstance();
        };
        
        /** Get the definition of a slot in the terrain.
        @remarks
            Definitions exist before the actual instances to allow background loading.
        @param x, y The coordinates of the terrain slot relative to the centre slot (signed).
        @return The definition, or null if nothing is in this slot. While this return value is
            not const, you should be careful about modifying it; it will have no effect unless you load
            the terrain afterwards, and can cause a race condition if you modify it while a background
            load is in progress.
        */
        virtual TerrainSlotDefinition* getTerrainDefinition(long x, long y) const;
        
        /** Get the terrain instance at a given slot, if loaded. 
        @param x, y The coordinates of the terrain slot relative to the centre slot (signed).
        @return The terrain, or null if no terrain is loaded in this slot (call getTerrainDefinition if
            you want to access the definition before it is loaded).
        */
        virtual Terrain* getTerrain(long x, long y) const;

        /** Free as many resources as possible for optimal run-time memory use for all
            terrain tiles.
        @see Terrain::freeTemporaryResources
        */
        void freeTemporaryResources();

        /** Trigger the update process for all terrain instances. 
        @see Terrain::update
        */
        void update(bool synchronous = false);

        /** Performs an update on all terrain geometry.
        @see Terrain::updateGeometry
        */
        void updateGeometry();

        /** Updates derived data for all terrains (LOD, lighting) to reflect changed height data.
        @see Terrain::updateDerivedData
        */
        void updateDerivedData(bool synchronous = false, uint8 typeMask = 0xFF);
        
        /** Result from a terrain ray intersection with the terrain group. 
        */
        struct _OgreTerrainExport RayResult
        {
            /// Whether an intersection occurred
            bool hit;
            /// Which terrain instance was hit, if any
            Terrain* terrain;
            /// Position at which the intersection occurred
            Vector3 position;

            RayResult(bool _hit, Terrain* _terrain, const Vector3& _pos)
                : hit(_hit), terrain(_terrain), position(_pos) {}
        };
        
        /** Get the height data for a given world position (projecting the point
        down on to the terrain underneath). 
        @param x, y,z Position in world space. Positions will be clamped to the edge
        of the terrain
        @param ppTerrain Pointer to a pointer to a terrain which will be completed
            with the terrain that was found to resolve this query, or null if none were
        */
        float getHeightAtWorldPosition(Real x, Real y, Real z, Terrain** ppTerrain = 0);

        /** Get the height data for a given world position (projecting the point
        down on to the terrain). 
        @param pos Position in world space. Positions will be clamped to the edge
        of the terrain
        @param ppTerrain Pointer to a pointer to a terrain which will be completed
        with the terrain that was found to resolve this query, or null if none were
        */
        float getHeightAtWorldPosition(const Vector3& pos, Terrain** ppTerrain = 0);

        /** Test for intersection of a given ray with any terrain in the group. If the ray hits
         a terrain, the point of intersection and terrain instance is returned.
         @param ray The ray to test for intersection
         @param distanceLimit The distance from the ray origin at which we will stop looking,
            0 indicates no limit
         @return A result structure which contains whether the ray hit a terrain and if so, where.
         @remarks This can be called from any thread as long as no parallel write to
         the terrain data occurs.
         */
        RayResult rayIntersects(const Ray& ray, Real distanceLimit = 0) const; 
        
        typedef std::vector<Terrain*> TerrainList; 
        /** Test intersection of a box with the terrain. 
        @remarks
            Tests an AABB for overlap with a terrain bounding box. Note that this does not mean that the box
            touches the terrain itself, just the bounding box for the terrain. You can use this to get region
            results for further testing or use (e.g. painting areas). 
        @param box The AABB you want to test in world units
        @param resultList Pointer to a list of terrain pointers which will be updated to include just
            the terrains that overlap
        */
        void boxIntersects(const AxisAlignedBox& box, TerrainList* resultList) const;
        /** Test intersection of a sphere with the terrain. 
        @remarks
            Tests a sphere for overlap with a terrain bounding box. Note that this does not mean that the sphere
            touches the terrain itself, just the bounding box for the terrain. You can use this to get region
            results for further testing or use (e.g. painting areas). 
        @param sphere The sphere you want to test in world units
        @param resultList Pointer to a list of terrain pointers which will be updated to include just
            the terrains that overlap
        */
        void sphereIntersects(const Sphere& sphere, TerrainList* resultList) const;
        
        /** Convert a world position to terrain slot coordinates. 
        @param pos The world position
        @param x,y Pointers to the coordinates to be completed.
        */
        void convertWorldPositionToTerrainSlot(const Vector3& pos, long *x, long *y) const;

        /** Convert a slot location to a world position at the centre
        @param x,y The slot coordinates
        @param pos Pointer to the world position to be completed
        */
        void convertTerrainSlotToWorldPosition(long x, long y, Vector3* pos) const;
        
        /** Calls Terrain::isDerivedDataUpdateInProgress on each loaded instance and returns true
            if any of them are undergoing a derived update.
        */
        bool isDerivedDataUpdateInProgress() const;

        /// Packed map, signed 16 bits for each axis from -32767 to +32767
        typedef std::map<uint32, TerrainSlot*> TerrainSlotMap;
        typedef MapIterator<TerrainSlotMap> TerrainIterator;
        typedef ConstMapIterator<TerrainSlotMap> ConstTerrainIterator;

        /// Get an iterator over the defined terrains.
        TerrainIterator getTerrainIterator();
        /// Get an iterator over the defined terrains (const)
        ConstTerrainIterator getTerrainIterator() const;

        /// WorkQueue::RequestHandler override
        bool canHandleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ);
        /// WorkQueue::RequestHandler override
        WorkQueue::Response* handleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ);
        /// WorkQueue::ResponseHandler override
        bool canHandleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ);
        /// WorkQueue::ResponseHandler override
        void handleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ);

        /// Convert coordinates to a packed integer index
        uint32 packIndex(long x, long y) const;
        
        /// Convert a packed integer index to coordinates
        void unpackIndex(uint32 key, long *x, long *y);

        /// Generate a file name based on the current naming convention
        String generateFilename(long x, long y) const;

        /** Save the group data only in native form to a file.
        */
        void saveGroupDefinition(const String& filename);
        /** Save the group data only in native form to a serializing stream.
        */
        void saveGroupDefinition(StreamSerialiser& stream);
        /** Load the group definition only in native form from a file.
        */
        void loadGroupDefinition(const String& filename);
        /** Load the group definition only in native form from a serializing stream.
        */
        void loadGroupDefinition(StreamSerialiser& stream);


        static const uint16 WORKQUEUE_LOAD_REQUEST;
        static const uint32 CHUNK_ID;
        static const uint16 CHUNK_VERSION;

        /// Loads terrain's next LOD level.
        void increaseLodLevel(long x, long y, bool synchronous = false);
        /// Removes terrain's highest LOD level.
        void decreaseLodLevel(long x, long y);

        void setAutoUpdateLod(TerrainAutoUpdateLod* updater);
        /// Automatically checks if terrain's LOD level needs to be updated.
        void autoUpdateLod(long x, long y, bool synchronous, const Any &data);
        void autoUpdateLodAll(bool synchronous, const Any &data);

        /** Get the number of terrains that are still waiting for the Terrain::prepare() to be called.
         *
         * @note Terrain::prepare() happens in background thread so the actual call will be completed
         *       a bit before this returns the reduced number.
         *
         * @return Amount of terrain prepare requests pending.
         */
        size_t getNumTerrainPrepareRequests() const;

    protected:
        typedef std::map<TerrainSlot*, WorkQueue::RequestID> TerrainPrepareRequestMap;
        SceneManager *mSceneManager;
        Terrain::Alignment mAlignment;
        uint16 mTerrainSize;
        Real mTerrainWorldSize;
        Terrain::ImportData mDefaultImportData;
        Vector3 mOrigin;
        TerrainSlotMap mTerrainSlots;
        TerrainPrepareRequestMap mTerrainPrepareRequests;
        uint16 mWorkQueueChannel;
        String mFilenamePrefix;
        String mFilenameExtension;
        String mResourceGroup;
        TerrainAutoUpdateLod *mAutoUpdateLod;
        Terrain::DefaultGpuBufferAllocator mBufferAllocator;
        
        /// Get the position of a terrain instance
        Vector3 getTerrainSlotPosition(long x, long y);
        /// Retrieve a slot, potentially allocate one
        TerrainSlot* getTerrainSlot(long x, long y, bool createIfMissing);
        TerrainSlot* getTerrainSlot(long x, long y) const;
        void freeTerrainSlotInstance(TerrainSlot* slot);
        void connectNeighbour(TerrainSlot* slot, long offsetx, long offsety);

        void loadTerrainImpl(TerrainSlot* slot, bool synchronous);

        /// Structure for holding the load request
        struct LoadRequest
        {
            TerrainSlot* slot;
            TerrainGroup* origin;
            OGRE_DEPRECATED _OgreTerrainExport friend std::ostream& operator<<(std::ostream& o, const LoadRequest& r)
            { return o; }       
        };
        

    };


    /** @} */
    /** @} */

}

#endif

