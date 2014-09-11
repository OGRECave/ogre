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
#ifndef __Ogre_Volume_Chunk_H__
#define __Ogre_Volume_Chunk_H__

#include "OgreSimpleRenderable.h"
#include "OgreSceneNode.h"
#include "OgreResourceGroupManager.h"
#include "OgreFrameListener.h"

#include "OgreVolumePrerequisites.h"
#include "OgreVolumeChunkHandler.h"
#include "OgreVolumeSource.h"
#include "OgreVolumeOctreeNode.h"
#include "OgreVolumeDualGridGenerator.h"
#include "OgreVolumeMeshBuilder.h"


namespace Ogre {
namespace Volume {

    class ChunkHandler;

    /** Parameters for loading the volume.
    */
    typedef struct ChunkParameters
    {
        /// The scenemanager to construct the entity with.
        SceneManager *sceneManager;

        /// The volume source.
        Source *src;

        /// The smallest allowed geometric error of the highest LOD.
        Real baseError;

        /// The error multiplicator per LOD level with 1.0 as default.
        Real errorMultiplicator;

        /// Whether to create the octree debug visualization entity with false as default.
        bool createOctreeVisualization;

        /// Whether to create the dualgrid debug visualization entity with false as default.
        bool createDualGridVisualization;

        /// Factor for the skirt length generation.
        Real skirtFactor;
        
        /// Callback for a specific LOD level.
        MeshBuilderCallback *lodCallback;
        
        /// The scale of the volume with 1.0 as default.
        Real scale;

        /// The maximum accepted screen space error when choosing the LOD levels to render.
        Real maxScreenSpaceError;
        
        /// The first LOD level to create geometry for. For scenarios where the lower levels won't be visible anyway. 0 is the default and switches this off.
        size_t createGeometryFromLevel;

        /// If an existing chunktree is to be partially updated, set this to the back lower left point of the (sub-)cube to be reloaded. Else, set both update vectors to zero (initial load). 1.5 is the default.
        Vector3 updateFrom;
        
        /// If an existing chunktree is to be partially updated, set this to the front upper right point of the (sub-)cube to be reloaded. Else, set both update vectors to zero (initial load).
        Vector3 updateTo;

        /// Whether to load the chunks async. if set to false, the call to load waits for the whole chunk. false is the default.
        bool async;

        /** Constructor.
        */
        ChunkParameters(void) :
            sceneManager(0), src(0), baseError((Real)0.0), errorMultiplicator((Real)1.0), createOctreeVisualization(false),
            createDualGridVisualization(false), skirtFactor(0), lodCallback(0), scale((Real)1.0), maxScreenSpaceError(0), createGeometryFromLevel(0),
            updateFrom(Vector3::ZERO), updateTo(Vector3::ZERO), async(false)
        {
        }
    } ChunkParameters;
    
    /** Internal shared values of the chunks which are equal in the whole tree.
    */
    typedef struct ChunkTreeSharedData
    {
        /// Flag whether the octree is visible or not.
        bool octreeVisible;
        
        /// Flag whether the dualgrid is visible or not.
        bool dualGridVisible;

        /// Another visibility flag to be user setable.
        bool volumeVisible;

        /// The amount of chunks being processed (== loading).
        int chunksBeingProcessed;

        /// The parameters with which the chunktree got loaded.
        ChunkParameters *parameters;

        /** Constructor.
        */
        ChunkTreeSharedData(const ChunkParameters *params) : octreeVisible(false), dualGridVisible(false), volumeVisible(true), chunksBeingProcessed(0)
        {
            this->parameters = new ChunkParameters(*params);
        }

        /** Destructor.
        */
        ~ChunkTreeSharedData(void)
        {
            delete parameters;
        }

    } ChunkTreeSharedData;

    /** A single volume chunk mesh.
    */
    class _OgreVolumeExport Chunk : public SimpleRenderable, public FrameListener
    {
    
    /// So the actual loading functions can be called.
    friend class ChunkHandler;

    protected:

        /// To handle the WorkQueue.
        static ChunkHandler mChunkHandler;
                
        /// To attach this node to.
        SceneNode *mNode;

        /// Holds the error associated with this chunk.
        Real mError;

        /// Holds the dualgrid debug visualization.
        Entity *mDualGrid;
        
        /// The debug visualization of the octree.
        Entity *mOctree;
        
        /// The more detailed children chunks.
        Chunk **mChildren;

        /// Flag whether this node will never be shown.
        bool mInvisible;
        
        /// Whether this chunk is the root of the tree.
        bool isRoot;

        /// Holds some shared data among all chunks of the tree.
        ChunkTreeSharedData *mShared;

        /** Loads a single chunk of the tree.
        @param parent
            The parent scene node for the volume
        @param from
            The back lower left corner of the cell.
        @param to
            The front upper right corner of the cell.
        @param totalFrom
            The back lower left corner of the world.
        @param totalTo
            The front upper rightcorner of the world.
        @param level
            The current LOD level.
        @param maxLevels
            The maximum amount of levels.
        */
        virtual void loadChunk(SceneNode *parent, const Vector3 &from, const Vector3 &to, const Vector3 &totalFrom, const Vector3 &totalTo, const size_t level, const size_t maxLevels);
                
        /** Whether the center of the given cube (from -> to) will contribute something
        to the total volume mesh.
        @param from
            The back lower left corner of the cell.
        @param to
            The front upper right corner of the cell.
        @return
            true if triangles might be generated
        */
        virtual bool contributesToVolumeMesh(const Vector3 &from, const Vector3 &to) const;
        
        /** Loads the tree children of the current node.
        @param parent
            The parent scene node for the volume
        @param from
            The back lower left corner of the cell.
        @param to
            The front upper right corner of the cell.
        @param totalFrom
            The back lower left corner of the world.
        @param totalTo
            The front upper rightcorner of the world.
        @param level
            The current LOD level.
        @param maxLevels
            The maximum amount of levels.
        */
        virtual void loadChildren(SceneNode *parent, const Vector3 &from, const Vector3 &to, const Vector3 &totalFrom, const Vector3 &totalTo, const size_t level, const size_t maxLevels);

        /** Actually loads the volume tree with all LODs.
        @param parent
            The parent scene node for the volume
        @param from
            The back lower left corner of the cell.
        @param to
            The front upper right corner of the cell.
        @param totalFrom
            The back lower left corner of the world.
        @param totalTo
            The front upper rightcorner of the world.
        @param level
            The current LOD level.
        @param maxLevels
            The maximum amount of levels.
        */
        virtual void doLoad(SceneNode *parent, const Vector3 &from, const Vector3 &to, const Vector3 &totalFrom, const Vector3 &totalTo, const size_t level, const size_t maxLevels);
        
        /** Prepares the geometry of the chunk request. To be called in a different thread.
        @param level
            The current LOD level.
        @param root
            The root of the upcoming Octree (in here) of the chunk.
        @param dualGridGenerator
            The DualGrid.
        @param meshBuilder
            The MeshBuilder which will contain the geometry.
        @param totalFrom
            The back lower left corner of the world.
        @param totalTo
            The front upper rightcorner of the world.
        */
        virtual void prepareGeometry(size_t level, OctreeNode *root, DualGridGenerator *dualGridGenerator, MeshBuilder *meshBuilder, const Vector3 &totalFrom, const Vector3 &totalTo);

        /** Loads the actual geometry when the processing is done.
        @param meshBuilder
            The MeshBuilder holding the geometry.
        @param dualGridGenerator
            The DualGridGenerator to build up the debug visualization of the DualGrid.
        @param root
            The root node of the Octree to build up the debug visualization of the Otree.
        @param level
            The current LOD level.
        @param isUpdate
            Whether this loading is updating an existing ChunkTree.
        */
        virtual void loadGeometry(MeshBuilder *meshBuilder, DualGridGenerator *dualGridGenerator, OctreeNode *root, size_t level, bool isUpdate);

        /** Sets the visibility of this chunk.
        @param visible
            Whether this chunk is visible or not.
        @param applyToChildren
            Whether all children and subchildren should have their visibility changed, too.
        */
        inline void setChunkVisible(const bool visible, const bool applyToChildren)
        {
            if (mInvisible)
            {
                return;
            }
            if (mShared->volumeVisible)
            {
                mVisible = visible;
            }
            if (mOctree)
            {
                mOctree->setVisible(mShared->octreeVisible && visible);
            }
            if (mDualGrid)
            {
                mDualGrid->setVisible(mShared->dualGridVisible && visible);
            }
            if (applyToChildren && mChildren)
            {
                mChildren[0]->setChunkVisible(visible, applyToChildren);
                if (mChildren[1])
                {
                    mChildren[1]->setChunkVisible(visible, applyToChildren);
                    mChildren[2]->setChunkVisible(visible, applyToChildren);
                    mChildren[3]->setChunkVisible(visible, applyToChildren);
                    mChildren[4]->setChunkVisible(visible, applyToChildren);
                    mChildren[5]->setChunkVisible(visible, applyToChildren);
                    mChildren[6]->setChunkVisible(visible, applyToChildren);
                    mChildren[7]->setChunkVisible(visible, applyToChildren);
                }
            }
        }

    public:
        
        /// The type name.
        static const String MOVABLE_TYPE_NAME;
        
        /** Constructor.
        */
        Chunk(void);

        /** Destructor.
        */
        virtual ~Chunk(void);

        /** Overridden from MovableObject.
        */
        virtual const String& getMovableType(void) const;
        
        /** Overridden from Renderable.
        */
        virtual Real getSquaredViewDepth(const Camera* camera) const;

        /** Overridden from  MovableObject.
        */
        virtual Real getBoundingRadius() const;

        /** Loads the volume mesh with all LODs.
        @param parent
            The parent scene node for the volume
        @param from
            The back lower left corner of the cell.
        @param to
            The front upper right corner of the cell.
        @param level
            The amount of LOD level.
        @param parameters
            The parameters to use while loading.
        */
        virtual void load(SceneNode *parent, const Vector3 &from, const Vector3 &to, size_t level, const ChunkParameters *parameters);

        /** Loads a TextureSource volume scene from a config file.
        @param parent
            The parent scene node for the volume.
        @param sceneManager
            The scenemanager to construct the entity with.
        @param filename
            The filename of the configuration file.
        @param validSourceResult
            If you want to use the loaded source afterwards of the parameters, set this to true.  Beware, that you
            will have to delete the pointer on your own then! On false here, it internally frees the
            memory for you
        @param lodCallback
            Callback for a specific LOD level.
        @param resourceGroup
            The resource group where to search for the configuration file.
        */
        virtual void load(SceneNode *parent, SceneManager *sceneManager, const String& filename, bool validSourceResult = false, MeshBuilderCallback *lodCallback = 0, const String& resourceGroup = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        
        /** Shows the debug visualization entity of the dualgrid.
        @param visible
            Whether the grid should be visible.
        */
        virtual void setDualGridVisible(const bool visible);
        
        /** Gets whether the debug visualization entity of the
        dualgrid is visible.
        @return
            true if visible.
        */
        virtual bool getDualGridVisible(void) const;
        
        /** Shows the debug visualization entity of the octree.
        @param visible
            Whether the octree should be visible.
        */
        virtual void setOctreeVisible(const bool visible);

        /** Gets whether the debug visualization entity of the
        octree is visible.
        @return
            true if visible.
        */
        virtual bool getOctreeVisible(void) const;
        
        /** Sets whether the volume mesh is visible.
        @param visible
            true if visible
        */
        virtual void setVolumeVisible(const bool visible);
        
        /** Gets whether the volume mesh is visible.
        @return
            true if visible
        */
        virtual bool getVolumeVisible(void) const;
        
        /** Overridden from FrameListener.
        */
        virtual bool frameStarted(const FrameEvent& evt);
        
        /** Overridable factory method.
        @return
            The created chunk.
        */
        virtual Chunk* createInstance(void);
        
        /** Overridden from SimpleRenderable.
            Sets the material of this chunk and all of his children.
        */
        virtual void setMaterial(const String& matName);

        /** Sets the material of all chunks of a specific level in the tree.
        This allows LODs where the lower levels (== less detail and more far away)
        have simpler materials.
        @param level
            The tree level getting the material, 0 based. 0 means the chunk with the lowest level of detail.
        @param matName
            The material name to set.
        */
        virtual void setMaterialOfLevel(size_t level, const String& matName);

        /** A list of Chunks.
        */
        typedef vector<const Chunk*>::type VecChunk;

        /** Gathers all visible chunks (containing triangles) of a specific LOD level.
        @param level
            The desired chunk level, 0 based. 0 means the chunk with the lowest level of detail. If the chunks are loaded with
            a level amount of 5, valid values here are 0-4.
        @param result
            Vector where the chunks will be added to.
        */
        virtual void getChunksOfLevel(const size_t level, VecChunk &result) const;
                
        /** Gets the parameters with which the chunktree got loaded.
        @return
            The parameters.
        */
        ChunkParameters* getChunkParameters(void);

    };
}
}

#endif