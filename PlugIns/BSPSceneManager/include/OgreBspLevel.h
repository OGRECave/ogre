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
    /** \addtogroup Plugins Plugins
    *  @{
    */
    /** \defgroup BSPSceneManager BSPSceneManager
    * Binary Space Parition (BSP) based indoor level scene manager
    *  @{
    */

    /** Holds all the data associated with a Binary Space Parition
        (BSP) based indoor level.
        The data used here is populated by loading level files via
        the BspLevelManager::load method, although application users
        are more likely to call SceneManager::setWorldGeometry which will
        automatically arrange the loading of the level. Note that this assumes
        that you have asked for an indoor-specialised SceneManager (specify
        ST_INDOOR when calling Root::getSceneManager).
        Ogre currently only supports loading from Quake3 Arena level files,
        although any source that can be converted into this classes structure
        could also be used. The Quake3 level load process is in a different
        class called Quake3Level to keep the specifics separate.
    */
    class BspLevel : public Resource, public Renderable
    {
        friend class BspSceneManager;

        using Resource::load;
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

        void getRenderOperation(RenderOperation &op) override
        {
            op = mRenderOp;
        }

        void getWorldTransforms(Matrix4* xform) const override
        {
            *xform = Matrix4::IDENTITY;
        }

        Real getSquaredViewDepth(const Camera *cam) const override
        {
            // always visible
            return -1;
        }

        const LightList& getLights() const override
        {
            static LightList lights;
            return lights;
        }

        const MaterialPtr& getMaterial(void) const override
        {
            static MaterialPtr nullPtr;
            return nullPtr;
        }

        /** Utility class just to enable queueing of patches */
    protected:
        /** Caches a face group for imminent rendering. */
        uint32 cacheGeometry(uint32* pIndexes, const StaticFaceGroup* faceGroup);
        bool cacheGeometry(const std::vector<StaticFaceGroup*>& materialFaceGroup);

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
        int mLeafStart; /// The index at which leaf nodes begin

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

        /// Vertex data holding all the data for the level, but able to render parts of it
        RenderOperation mRenderOp;

        /** Array of indexes into the mFaceGroups array. This buffer is organised
            by leaf node so leaves can just use contiguous chunks of it and
            get repointed to the actual entries in mFaceGroups. */
        int* mLeafFaceGroups;
        int mNumLeafFaceGroups;

        /** Array of face groups, indexed into by contents of mLeafFaceGroups. */
        StaticFaceGroup* mFaceGroups;
        int mNumFaceGroups;

        /// Indexes for the whole level, will be copied to the real indexdata per frame
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
            overhead.
            Each row in the table is a 'from' cluster, with each bit in the row corresponding to a 'to' cluster,
            both ordered based on cluster index. A 0 in the bit indicates the 'to' cluster is not visible from the
            'from' cluster, whilst a 1 indicates it is.
            As many will notice, this is lifted directly from the Quake implementation of PVS.
        */
        struct VisData
        {
            unsigned char *tableData;
            int numClusters;            /// Number of clusters, therefore number of rows
            int rowLength;                /// Length in bytes of each row (num clusters / 8 rounded up)
        };

        VisData mVisData;


        /** Internal method for parsing chosen entities. */
        void loadEntities(const Quake3Level& q3lvl);

        typedef std::map<const MovableObject*, std::list<BspNode*> > MovableToNodeMap;
        /// Map for locating the nodes a movable is currently a member of
        MovableToNodeMap mMovableToNodeMap;

        void tagNodesWithMovable(BspNode* node, const MovableObject* mov, const Vector3& pos);

        /// Storage of patches
        typedef std::map<int, PatchSurface*> PatchMap;
        PatchMap mPatches;
        /// Total number of vertices required for all patches
        size_t mPatchVertexCount;
        /// Total number of indexes required for all patches
        size_t mPatchIndexCount;
        /// Sky enabled?
        bool mSkyEnabled;
        /// Sky material
        String mSkyMaterial;
        /// Sky details
        Real mSkyCurvature;


        void initQuake3Patches(const Quake3Level & q3lvl, VertexDeclaration* decl);
        void buildQuake3Patches(size_t vertOffset, size_t indexOffset);

        void quakeVertexToBspVertex(const bsp_vertex_t* src, BspVertex* dest);


    };
    typedef SharedPtr<BspLevel> BspLevelPtr;
    /** @} */
    /** @} */
}

#endif
