/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2012 Torus Knot Software Ltd

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
#include "OgreWorkQueue.h"

#include "OgreVolumePrerequisites.h"
#include "OgreVolumeSource.h"
#include "OgreVolumeOctreeNode.h"
#include "OgreVolumeDualGridGenerator.h"
#include "OgreVolumeMeshBuilder.h"


namespace Ogre {
namespace Volume {
    
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

        /// On which LOD level the callback should be called.
        size_t lodCallbackLod;

        /// The scale of the volume with 1.0 as default.
        Real scale;

        /// The maximum accepted screen space error when chosing the LOD levels to render.
        Real maxScreenSpaceError;
        
        /** Constructor.
        */
        ChunkParameters(void) :
            sceneManager(0), src(0), baseError(0.0), errorMultiplicator(1.0), createOctreeVisualization(false), createDualGridVisualization(false),
            lodCallback(0), lodCallbackLod(0), scale(1.0)
        {
        }
    } ChunkParameters;
    
    /** Forward declaration.
    */
    class Chunk;

    /** Data being passed arround while loading.
    */
    typedef struct ChunkRequest
    {

        /// The back lower left corner of the world.
        Vector3 totalFrom;

        /// The front upper rightcorner of the world.
        Vector3 totalTo;

        /// The parameters to use while loading.
        const ChunkParameters *parameters;

        /// The current LOD level.
        size_t level;

        /// The maximum amount of levels.
        size_t maxLevels;

        /// The MeshBuilder to use.
        MeshBuilder *mb;

        /// The DualGridGenerator to use.
        DualGridGenerator *dualGridGenerator;

        /// The octree node to use.
        OctreeNode *root;

        /// The chunk which created this request.
        Chunk *origin;

        /** Stream operator <<.
        @param o
            The used stream.
        @param r
            The streamed ChunkRequest.
        */
        _OgreVolumeExport friend std::ostream& operator<<(std::ostream& o, const ChunkRequest& r)
        { return o; }
    } ChunkRequest;

    /** A single volume chunk mesh.
    */
    class _OgreVolumeExport Chunk : public SimpleRenderable, public FrameListener, public WorkQueue::RequestHandler, public WorkQueue::ResponseHandler
    {
    protected:
        
        /// The workqueue load request.
        static const uint16 WORKQUEUE_LOAD_REQUEST;

        /// Holds the amount of generated triangles.
        static size_t mGeneratedTriangles;

        /// The maximum accepted screen space error.
        Real mMaxScreenSpaceError;
        
        /// The scale.
        Real mScale;

        /// The amount of chunks currently being processed.
        static size_t mChunksBeingProcessed;

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

        /// Flag whether the octree is visible or not.
        bool mOctreeVisible;
        
        /// Flag whether the dualgrid is visible or not.
        bool mDualGridVisible;

        /// Flag whether this node will never be shown.
        bool mInvisible;

        /// Another visibility flag to be user setable.
        bool mVolumeVisible;
                
        /** Actually Loads the volume mesh with all LODs.
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
        @param parameters
            The parameters to use while loading.
        */
        virtual void doLoad(SceneNode *parent, const Vector3 &from, const Vector3 &to, const Vector3 &totalFrom, const Vector3 &totalTo, const size_t level, const size_t maxLevels, const ChunkParameters *parameters);
        
        /** Prepares the geometry of the chunk request. To be called in a different thread.
        @param chunkRequest
            The chunk loading request with the data to be prepared.
        */
        virtual void prepareGeometry(const ChunkRequest *chunkRequest);

        /** Loads the actual geometry when the processing is done.
        @param chunkRequest
            The chunk loading request with the processed data.
        */
        virtual void loadGeometry(const ChunkRequest *chunkRequest);

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
            if (mVolumeVisible)
            {
                mVisible = visible;
            }
            if (mOctree)
            {
                mOctree->setVisible(mOctreeVisible && visible);
            }
            if (mDualGrid)
            {
                mDualGrid->setVisible(mDualGridVisible && visible);
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
        @param lodCallback
            Callback for a specific LOD level.
        @param lodCallbackLod
            On which LOD level the callback should be called.
        @param resourceGroup
            The resource group where to search for the configuration file.
        */
        virtual void load(SceneNode *parent, SceneManager *sceneManager, const String& filename, MeshBuilderCallback *lodCallback = 0, size_t lodCallbackLod = 0, const String& resourceGroup = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        
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

        /// Implementation for WorkQueue::RequestHandler
        WorkQueue::Response* handleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ);
        
        /// Implementation for WorkQueue::ResponseHandler
        void handleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ);

        /** Gets the scale of the chunk.
        @return
            The scale.
        */
        Real getScale(void) const;

    };
}
}

#endif