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
*/
#ifndef _BspLevel_H__
#define _BspLevel_H__

#include "OgreBspPrerequisites.h"
#include "OgreResource.h"
#include "OgreStaticFaceGroup.h"
#include "OgreSceneManager.h"
#include "OgreBspNode.h"
#include "OgreHardwareBufferManager.h"
#include "OgreDefaultHardwareBufferManager.h"
#include "OgreQuake3Level.h"


namespace Ogre {

    /** Holds all the data associated with a Binary Space Parition
        (BSP) based indoor level.
        The data used here is populated by loading level files via
        the BspLevelManager::load method, although application users
        are more likely to call SceneManager::setWorldGeometry which will
        automatically arrange the loading of the level. Note that this assumes
        that you have asked for an indoor-specialised SceneManager (specify
        ST_INDOOR when calling Root::getSceneManager).</p>
        Ogre currently only supports loading from Quake3 Arena level files,
        although any source that can be converted into this classes structure
        could also be used. The Quake3 level load process is in a different
        class called Quake3Level to keep the specifics separate.</p>
    */
    class BspLevel : public Resource
    {
        friend class BspSceneManager;
    public:
        /** Default constructor - used by BspResourceManager (do not call directly) */
        BspLevel(ResourceManager* creator, const String& name, ResourceHandle handle,
            const String& group, bool isManual = false, ManualResourceLoader* loader = 0);
        ~BspLevel();

        /** Determines if one leaf node is visible from another. */
        bool isLeafVisible(const BspNode* from, const BspNode* to) const;

        /** Returns a pointer to the root node (BspNode) of the BSP tree. */
        const BspNode* getRootNode(void);

        /** Walks the entire BSP tree and returns the leaf
            which contains the given point.
        */
        BspNode* findLeaf(const Vector3& point) const;

        /** Ensures that the MovableObject is attached to the right leaves of the 
            BSP tree.
        */
        void _notifyObjectMoved(const MovableObject* mov, 
            const Vector3& pos);
		/** Internal method, makes sure an object is removed from the leaves when detached from a node. */
		void _notifyObjectDetached(const MovableObject* mov);
        /** Gets a pointer to the start of the leaf nodes. */
        BspNode* getLeafStart(void) {return &mRootNode[mLeafStart]; }
        /** Gets the number of leaf nodes */
        int getNumLeaves(void) const { return mNumLeaves; }

        /** Calculate the number of loading stages required for a given level */
        static size_t calculateLoadingStages(const String& levelName);
        /** Calculate the number of loading stages required for a given level */
        static size_t calculateLoadingStages(DataStreamPtr& stream);

		/** Load direct from stream */
		void load(DataStreamPtr& stream);

		/** Is sky enabled? */
		bool isSkyEnabled(void) const;
		/** Get Sky material name */
		const String& getSkyMaterialName(void) const;
		/** Get sky curvature */
		Real getSkyCurvature(void) const;

        /** Utility class just to enable queueing of patches */
    protected:
        /** @copydoc Resource::loadImpl. */
        void loadImpl(void);
        /** @copydoc Resource::unloadImpl. */
        void unloadImpl(void);
        /** @copydoc Resource::calculateSize. */
        size_t calculateSize(void) const;
        /** Pointer to the root node of the BSP tree;
            This pointer actually has a dual purpose; to avoid allocating lots of small chunks of
            memory, the BspLevel actually allocates all nodes required through this pointer. So this
            pointer is the handle for the allocation of memory for all nodes. It also happens to point
            to the root node, since the first one in the memory chunk is the root node.
        */
        BspNode* mRootNode;
        int mNumNodes;
        int mNumLeaves;
		int mNumBrushes;
        int mLeafStart; // the index at which leaf nodes begin

        /** Vertex format for fixed geometry.
            Note that in this case vertex components (position, normal, texture coords etc)
            are held interleaved in the same buffer. However, the format here is different from 
            the format used by Quake because older Direct3d drivers like the vertex elements
            to be in a particular order within the buffer. See VertexDeclaration for full
            details of this marvellous(not) feature.
        */
        struct BspVertex
        {
            float position[3];
            float normal[3];
            int colour;
            float texcoords[2];
            float lightmap[2];
        };
        /*
        /// Array of vertices for whole level.
        BspVertex* mVertices;
        int mNumVertices;
        */
        /// Vertex data holding all the data for the level, but able to render parts of it
        VertexData* mVertexData;

        /** Array of indexes into the mFaceGroups array. This buffer is organised
            by leaf node so leaves can just use contiguous chunks of it and
            get repointed to the actual entries in mFaceGroups. */
        int* mLeafFaceGroups;
        int mNumLeafFaceGroups;

        /** Array of face groups, indexed into by contents of mLeafFaceGroups. */
        StaticFaceGroup* mFaceGroups;
        int mNumFaceGroups;


        /*
        /// Array of elements i.e. vertex indexes as used by face groups.
        int* mElements;
        int mNumElements;
        */

        /// indexes for the whole level, will be copied to the real indexdata per frame
        size_t mNumIndexes;
        // system-memory buffer
        HardwareIndexBufferSharedPtr mIndexes;

        /// Brushes as used for collision, main memory is here
        BspNode::Brush *mBrushes;

        /** Vector of player start points */
        std::vector<ViewPoint> mPlayerStarts;

        /** Internal utility function for loading data from Quake3. */
        void loadQuake3Level(const Quake3Level& q3lvl);
        /** Internal lookup table to determine visibility between leaves.
            Leaf nodes are assigned to 'clusters' of nodes, which are used to group nodes together for
            visibility testing. This data holds a lookup table which is used to determine if one cluster of leaves
            is visible from another cluster. Whilst it would be possible to expand all this out so that
            each node had a list of pointers to other visible nodes, this would be very expensive in terms
            of storage (using the cluster method there is a table which is 1-bit squared per cluster, rounded
            up to the nearest byte obviously, which uses far less space than 4-bytes per linked node per source
            node). Of course the limitation here is that you have to each leaf in turn to determine if it is visible
            rather than just following a list, but since this is only done once per frame this is not such a big
            overhead.</p>
            Each row in the table is a 'from' cluster, with each bit in the row corresponding to a 'to' cluster,
            both ordered based on cluster index. A 0 in the bit indicates the 'to' cluster is not visible from the
            'from' cluster, whilst a 1 indicates it is.</p>
            As many will notice, this is lifted directly from the Quake implementation of PVS.
        */
        struct VisData
        {
            unsigned char *tableData;
            int numClusters;            // Number of clusters, therefore number of rows
            int rowLength;                // Length in bytes of each row (num clusters / 8 rounded up)
        };

        VisData mVisData;


        /** Internal method for parsing chosen entities. */
        void loadEntities(const Quake3Level& q3lvl);

        typedef std::map<const MovableObject*, std::list<BspNode*> > MovableToNodeMap;
        /// Map for locating the nodes a movable is currently a member of
        MovableToNodeMap mMovableToNodeMap;

        void tagNodesWithMovable(BspNode* node, const MovableObject* mov, const Vector3& pos);

        // Storage of patches 
        typedef std::map<int, PatchSurface*> PatchMap;
        PatchMap mPatches;
        // Total number of vertices required for all patches
        size_t mPatchVertexCount;
        // Total number of indexes required for all patches
        size_t mPatchIndexCount;
		// Sky enabled?
		bool mSkyEnabled;
		// Sky material
		String mSkyMaterial;
		// Sky details
		Real mSkyCurvature;


        void initQuake3Patches(const Quake3Level & q3lvl, VertexDeclaration* decl);
        void buildQuake3Patches(size_t vertOffset, size_t indexOffset);

        void quakeVertexToBspVertex(const bsp_vertex_t* src, BspVertex* dest);


    };
    /** Specialisation of SharedPtr to allow SharedPtr to be assigned to BspLevelPtr 
    @note Has to be a subclass since we need operator=.
    We could templatise this instead of repeating per Resource subclass, 
    except to do so requires a form VC6 does not support i.e.
    ResourceSubclassPtr<T> : public SharedPtr<T>
    */
    class BspLevelPtr : public SharedPtr<BspLevel> 
    {
    public:
        BspLevelPtr() : SharedPtr<BspLevel>() {}
        explicit BspLevelPtr(BspLevel* rep) : SharedPtr<BspLevel>(rep) {}
        BspLevelPtr(const BspLevelPtr& r) : SharedPtr<BspLevel>(r) {} 
        BspLevelPtr(const ResourcePtr& r) : SharedPtr<BspLevel>()
        {
			// lock & copy other mutex pointer
			OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
			OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
            pRep = static_cast<BspLevel*>(r.getPointer());
            pUseCount = r.useCountPointer();
            if (pUseCount)
            {
                ++(*pUseCount);
            }
        }

        /// Operator used to convert a ResourcePtr to a BspLevelPtr
        BspLevelPtr& operator=(const ResourcePtr& r)
        {
            if (pRep == static_cast<BspLevel*>(r.getPointer()))
                return *this;
            release();
			// lock & copy other mutex pointer
			OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
			OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
            pRep = static_cast<BspLevel*>(r.getPointer());
            pUseCount = r.useCountPointer();
            if (pUseCount)
            {
                ++(*pUseCount);
            }
            return *this;
        }
    };

}

#endif
