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
#include "OgreVolumeChunk.h"

#include "OgreCamera.h"
#include "OgreLogManager.h"
#include "OgreConfigFile.h"
#include "OgreVolumeIsoSurfaceMC.h"
#include "OgreVolumeOctreeNodeSplitPolicy.h"
#include "OgreVolumeTextureSource.h"
#include "OgreVolumeChunkHandler.h"
#include "OgreVolumeMeshBuilder.h"
#include "OgreVolumeDualGridGenerator.h"
#include "OgreSceneNode.h"
#include "OgreViewport.h"
#include "OgreRoot.h"
#include "OgreVolumeChunk.h"
#include "OgreVolumeMeshBuilder.h"
#include "OgreVolumeOctreeNode.h"

namespace Ogre {
namespace Volume {

    const String Chunk::MOVABLE_TYPE_NAME = "VolumeChunk";
    ChunkHandler Chunk::mChunkHandler;
    
    //-----------------------------------------------------------------------

    void Chunk::loadChunk(SceneNode *parent, const Vector3 &from, const Vector3 &to, const Vector3 &totalFrom, const Vector3 &totalTo, const size_t level, const size_t maxLevels)
    {
        // This might already exist on update
        if (!mNode)
        {
            mNode = parent->createChildSceneNode();
        }
        if (mShared->parameters->createGeometryFromLevel == 0 || level <= mShared->parameters->createGeometryFromLevel)
        {
            mShared->chunksBeingProcessed++;

            // Call worker
            ChunkRequest req;
            req.totalFrom = totalFrom;
            req.totalTo = totalTo;
            req.level = level;
            req.maxLevels = maxLevels;
            req.isUpdate = mShared->parameters->updateFrom != Vector3::ZERO || mShared->parameters->updateTo != Vector3::ZERO;

            req.origin = this;
            req.root = OGRE_NEW OctreeNode(from, to);
            req.meshBuilder = OGRE_NEW MeshBuilder();
            req.dualGridGenerator = OGRE_NEW DualGridGenerator();

            mChunkHandler.addRequest(req);
        }
        else
        {
            mInvisible = false;
        }
    }

    //-----------------------------------------------------------------------

    bool Chunk::contributesToVolumeMesh(const Vector3 &from, const Vector3 &to) const
    {
        Real centralValue = mShared->parameters->src->getValue((to - from) / (Real)2.0 + from);
        return Math::Abs(centralValue) <= (to - from).length() * mShared->parameters->src->getVolumeSpaceToWorldSpaceFactor();
    }

    //-----------------------------------------------------------------------
    
    void Chunk::loadChildren(SceneNode *parent, const Vector3 &from, const Vector3 &to, const Vector3 &totalFrom, const Vector3 &totalTo, const size_t level, const size_t maxLevels)
    {
        // Now recursively create the more detailed children
        if (level > 2)
        {
            Vector3 newCenter, xWidth, yWidth, zWidth;
            OctreeNode::getChildrenDimensions(from, to, newCenter, xWidth, yWidth, zWidth);
            if (!mChildren)
            {
                mChildren = new Chunk*[OctreeNode::OCTREE_CHILDREN_COUNT];
                mChildren[0] = createInstance();
                mChildren[1] = createInstance();
                mChildren[2] = createInstance();
                mChildren[3] = createInstance();
                mChildren[4] = createInstance();
                mChildren[5] = createInstance();
                mChildren[6] = createInstance();
                mChildren[7] = createInstance();
                mChildren[0]->mShared = mShared;
                mChildren[1]->mShared = mShared;
                mChildren[2]->mShared = mShared;
                mChildren[3]->mShared = mShared;
                mChildren[4]->mShared = mShared;
                mChildren[5]->mShared = mShared;
                mChildren[6]->mShared = mShared;
                mChildren[7]->mShared = mShared;
            }
            mChildren[0]->doLoad(mNode, from, newCenter, totalFrom, totalTo, level - 1, maxLevels);
            mChildren[1]->doLoad(mNode, from + xWidth, newCenter + xWidth, totalFrom, totalTo, level - 1, maxLevels);
            mChildren[2]->doLoad(mNode, from + xWidth + zWidth, newCenter + xWidth + zWidth, totalFrom, totalTo, level - 1, maxLevels);
            mChildren[3]->doLoad(mNode, from + zWidth, newCenter + zWidth, totalFrom, totalTo, level - 1, maxLevels);
            mChildren[4]->doLoad(mNode, from + yWidth, newCenter + yWidth, totalFrom, totalTo, level - 1, maxLevels);
            mChildren[5]->doLoad(mNode, from + yWidth + xWidth, newCenter + yWidth + xWidth, totalFrom, totalTo, level - 1, maxLevels);
            mChildren[6]->doLoad(mNode, from + yWidth + xWidth + zWidth, newCenter + yWidth + xWidth + zWidth, totalFrom, totalTo, level - 1, maxLevels);
            mChildren[7]->doLoad(mNode, from + yWidth + zWidth, newCenter + yWidth + zWidth, totalFrom, totalTo, level - 1, maxLevels);
        }
        // Just load one child of the same size as the parent for the leafes because they actually don't need to be subdivided as they
        // are all rendered anyway.
        else if (level > 1)
        {
            if (!mChildren)
            {
                mChildren = new Chunk*[2];
                mChildren[0] = createInstance();
                mChildren[0]->mShared = mShared;
                mChildren[1] = 0; // Indicator that there are no more children.
            }
            mChildren[0]->doLoad(mNode, from, to, totalFrom, totalTo, level - 1, maxLevels);
        }
    }

    //-----------------------------------------------------------------------
    
    void Chunk::doLoad(SceneNode *parent, const Vector3 &from, const Vector3 &to, const Vector3 &totalFrom, const Vector3 &totalTo, const size_t level, const size_t maxLevels)
    {

        // Handle the situation where we update an existing tree
        if (mShared->parameters->updateFrom != Vector3::ZERO || mShared->parameters->updateTo != Vector3::ZERO)
        {
            // Early out if an update of a part of the tree volume is going on and this chunk is outside of the area.
            AxisAlignedBox chunkCube(from, to);
            AxisAlignedBox updatedCube(mShared->parameters->updateFrom, mShared->parameters->updateTo);
            if (!chunkCube.intersects(updatedCube))
            {
                return;
            }
            // Free memory from old mesh version
            if (mRenderOp.vertexData)
            {
                OGRE_DELETE mRenderOp.vertexData;
                mRenderOp.vertexData = 0;
            }
            if (mRenderOp.indexData)
            {
                OGRE_DELETE mRenderOp.indexData;
                mRenderOp.indexData = 0;
            }
        }

        // Set to invisible for now.
        setVisible(false);
        mInvisible = true;
        
        // Don't generate this chunk if it doesn't contribute to the whole volume.
        if (!contributesToVolumeMesh(from, to))
        {
            return;
        }
    
        loadChunk(parent, from, to, totalFrom, totalTo, level, maxLevels);
        
        loadChildren(parent, from, to, totalFrom, totalTo, level, maxLevels);
    }
    
    //-----------------------------------------------------------------------

    void Chunk::prepareGeometry(size_t level, OctreeNode *root, DualGridGenerator *dualGridGenerator, MeshBuilder *meshBuilder, const Vector3 &totalFrom, const Vector3 &totalTo)
    {
        OctreeNodeSplitPolicy policy(mShared->parameters->src,
            mShared->parameters->errorMultiplicator * mShared->parameters->baseError);
        mError = (Real)level * mShared->parameters->errorMultiplicator * mShared->parameters->baseError;
        root->split(&policy, mShared->parameters->src, mError);
        Real maxMSDistance = (Real)level * mShared->parameters->errorMultiplicator * mShared->parameters->baseError * mShared->parameters->skirtFactor;
        IsoSurface *is = OGRE_NEW IsoSurfaceMC(mShared->parameters->src);
        dualGridGenerator->generateDualGrid(root, is, meshBuilder, maxMSDistance, totalFrom, totalTo,
            mShared->parameters->createDualGridVisualization);
        OGRE_DELETE is;
    }
    
    //-----------------------------------------------------------------------

    void Chunk::loadGeometry(MeshBuilder *meshBuilder, DualGridGenerator *dualGridGenerator, OctreeNode *root, size_t level, bool isUpdate)
    {
        size_t chunkTriangles = meshBuilder->generateBuffers(mRenderOp);
        mInvisible = chunkTriangles == 0;

        if (mShared->parameters->lodCallback)
        {
            meshBuilder->executeCallback(mShared->parameters->lodCallback, this, level, mShared->chunksBeingProcessed);
        }

        mBox = meshBuilder->getBoundingBox();

        if (!mInvisible)
        {
            if (isUpdate)
            {
                mNode->detachObject(this);
            }
            mNode->attachObject(this);
        }

        setVisible(false);

        if (mShared->parameters->createDualGridVisualization)
        {
            mDualGrid = dualGridGenerator->getDualGrid(mShared->parameters->sceneManager);
            if (mDualGrid)
            {
                mNode->attachObject(mDualGrid);
                mDualGrid->setVisible(false);
            }
        }

        if (mShared->parameters->createOctreeVisualization)
        {
            mOctree = root->getOctreeGrid(mShared->parameters->sceneManager);
            mNode->attachObject(mOctree);
            mOctree->setVisible(false);
        }
        mShared->chunksBeingProcessed--;
    }
    
    //-----------------------------------------------------------------------

    Chunk::Chunk(void) : SimpleRenderable(0, new ObjectMemoryManager()), mNode(0), mError(false), mDualGrid(0), mOctree(0), mChildren(0),
        mInvisible(false), isRoot(false), mShared(0)
    {
    }
    
    //-----------------------------------------------------------------------

    Chunk::~Chunk(void)
    {
        OGRE_DELETE mRenderOp.indexData;
        OGRE_DELETE mRenderOp.vertexData;

        // Root might already be shutdown.
        if (Root::getSingletonPtr())
        {
            Root::getSingleton().removeFrameListener(this);
        }

        if (mChildren)
        {
            OGRE_DELETE mChildren[0];
            if (mChildren[1])
            {
                OGRE_DELETE mChildren[1];
                OGRE_DELETE mChildren[2];
                OGRE_DELETE mChildren[3];
                OGRE_DELETE mChildren[4];
                OGRE_DELETE mChildren[5];
                OGRE_DELETE mChildren[6];
                OGRE_DELETE mChildren[7];
            }
        }
        delete[] mChildren;
        if (isRoot)
        {
            delete mShared;
        }
    }
    
    //-----------------------------------------------------------------------

    const String& Chunk::getMovableType(void) const
    {
        return MOVABLE_TYPE_NAME;
    }
    
    //-----------------------------------------------------------------------

    Real Chunk::getSquaredViewDepth(const Camera* camera) const
    {
        return (mBox.getCenter() * mShared->parameters->scale).squaredDistance(camera->getPosition());
    }
    
    //-----------------------------------------------------------------------

    Real Chunk::getBoundingRadius() const
    {
        return mBox.getMinimum().distance(mBox.getCenter()) * mShared->parameters->scale;
    }
    
    //-----------------------------------------------------------------------

    void Chunk::load(SceneNode *parent, const Vector3 &from, const Vector3 &to, size_t level, const ChunkParameters *parameters)
    {
        if (parameters->baseError == (Real)0.0 || parameters->errorMultiplicator == (Real)0.0 ||
            parameters->sceneManager == 0 || parameters->src == 0)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "Invalid parameters given!",
                __FUNCTION__);
        }
        
        isRoot = true;

        // Don't recreate the shared parameters on update.
        if (parameters->updateFrom == Vector3::ZERO && parameters->updateTo == Vector3::ZERO)
        {
            mShared = new ChunkTreeSharedData(parameters);
            parent->scale(Vector3(parameters->scale));
        }

        mShared->chunksBeingProcessed = 0;
        
        doLoad(parent, from, to, from, to, level, level);

        // Wait for the threads.
        if (!parameters->async)
        {
            while(mShared->chunksBeingProcessed)
            {
                OGRE_THREAD_SLEEP(0);
                mChunkHandler.processWorkQueue();
            }
        }
        
    
        // Just add the frame listener on initial load
        if (parameters->updateFrom == Vector3::ZERO && parameters->updateTo == Vector3::ZERO)
        {
            Root::getSingleton().addFrameListener(this);
        }
    }
    
    //-----------------------------------------------------------------------

    void Chunk::load(SceneNode *parent, SceneManager *sceneManager, const String& filename, bool validSourceResult, MeshBuilderCallback *lodCallback, const String& resourceGroup)
    {
        ConfigFile config;
        config.loadFromResourceSystem(filename, resourceGroup);

        String source = config.getSetting("source");
        Vector3 dimensions = StringConverter::parseVector3(config.getSetting("sourceDimensions"));
        bool trilinearValue = StringConverter::parseBool(config.getSetting("trilinearValue"));
        bool trilinearGradient = StringConverter::parseBool(config.getSetting("trilinearGradient"));
        bool sobelGradient = StringConverter::parseBool(config.getSetting("sobelGradient"));
        bool async = StringConverter::parseBool(config.getSetting("async"));

        TextureSource *textureSource = new TextureSource(source, dimensions.x, dimensions.y, dimensions.z, trilinearValue, trilinearGradient, sobelGradient);
    
        Vector3 from = StringConverter::parseVector3(config.getSetting("scanFrom"));
        Vector3 to = StringConverter::parseVector3(config.getSetting("scanTo"));
        size_t level = StringConverter::parseUnsignedInt(config.getSetting("level"));
        Real scale = StringConverter::parseReal(config.getSetting("scale"));
        Real maxScreenSpaceError = StringConverter::parseReal(config.getSetting("maxScreenSpaceError"));
    
        ChunkParameters parameters;
        parameters.sceneManager = sceneManager;
        parameters.lodCallback = lodCallback;
        parameters.src = textureSource;
        parameters.scale = scale;
        parameters.maxScreenSpaceError = maxScreenSpaceError;
        parameters.createGeometryFromLevel = StringConverter::parseInt(config.getSetting("createGeometryFromLevel"));
        parameters.baseError = StringConverter::parseReal(config.getSetting("baseError"));
        parameters.errorMultiplicator = StringConverter::parseReal(config.getSetting("errorMultiplicator"));
        parameters.createOctreeVisualization = StringConverter::parseBool(config.getSetting("createOctreeVisualization"));
        parameters.createDualGridVisualization = StringConverter::parseBool(config.getSetting("createDualGridVisualization"));
        parameters.skirtFactor = StringConverter::parseReal(config.getSetting("skirtFactor"));
        parameters.async = async;
    
        load(parent, from, to, level, &parameters);
        
        if (!validSourceResult)
        {
            delete textureSource;
        }

        String material = config.getSetting("material");
        setMaterial(material);

        for (size_t i = 0; i < level; ++i)
        {
            StringStream stream;
            stream << "materialOfLevel" << i;
            String materialOfLevel = config.getSetting(stream.str());
            if (materialOfLevel != BLANKSTRING)
            {
                setMaterialOfLevel(i, materialOfLevel);
            }
        }
    }
    
    //-----------------------------------------------------------------------

    void Chunk::setDualGridVisible(const bool visible)
    {
        mShared->dualGridVisible = visible;
        if (mChildren)
        {
            mChildren[0]->setDualGridVisible(visible);
            if (mChildren[1])
            {
                mChildren[1]->setDualGridVisible(visible);
                mChildren[2]->setDualGridVisible(visible);
                mChildren[3]->setDualGridVisible(visible);
                mChildren[4]->setDualGridVisible(visible);
                mChildren[5]->setDualGridVisible(visible);
                mChildren[6]->setDualGridVisible(visible);
                mChildren[7]->setDualGridVisible(visible);
            }
        }
    }
    
    //-----------------------------------------------------------------------

    bool Chunk::getDualGridVisible(void) const
    {
        return mShared->dualGridVisible;
    }
    
    //-----------------------------------------------------------------------

    void Chunk::setOctreeVisible(const bool visible)
    {
        mShared->octreeVisible = visible;
        if (mChildren)
        {
            mChildren[0]->setOctreeVisible(visible);
            if (mChildren[1])
            {
                mChildren[1]->setOctreeVisible(visible);
                mChildren[2]->setOctreeVisible(visible);
                mChildren[3]->setOctreeVisible(visible);
                mChildren[4]->setOctreeVisible(visible);
                mChildren[5]->setOctreeVisible(visible);
                mChildren[6]->setOctreeVisible(visible);
                mChildren[7]->setOctreeVisible(visible);
            }
        }
    }
    
    //-----------------------------------------------------------------------

    bool Chunk::getOctreeVisible(void) const
    {
        return mShared->octreeVisible;
    }
    
    //-----------------------------------------------------------------------

    bool Chunk::getVolumeVisible(void) const
    {
        return mShared->volumeVisible;
    }
    
    //-----------------------------------------------------------------------

    bool Chunk::frameStarted(const FrameEvent& evt)
    {
    
        if (mInvisible)
        {
            return true;
        }

        // This might be a chunk on a lower LOD level without geometry, so lets just proceed here.
        if (!mRenderOp.vertexData && mChildren)
        {
            mChildren[0]->frameStarted(evt);
            if (mChildren[1])
            {
                mChildren[1]->frameStarted(evt);
                mChildren[2]->frameStarted(evt);
                mChildren[3]->frameStarted(evt);
                mChildren[4]->frameStarted(evt);
                mChildren[5]->frameStarted(evt);
                mChildren[6]->frameStarted(evt);
                mChildren[7]->frameStarted(evt);
            }
            return true;
        }

        Camera *currentCamera = mParentSceneManager->getCameraInProgress();
        if (!currentCamera || !currentCamera->getLastViewport())
        {
            setChunkVisible(true, false);
            return true;
        }
    
        Real k = ((Real)currentCamera->getLastViewport()->getActualHeight() / ((Real)2.0 * tan(currentCamera->getFOVy().valueRadians() / (Real)2.0)));

        // Get the distance to the center.
        Vector3 camPos = currentCamera->getRealPosition();
        Real d = (mBox.getCenter() * mShared->parameters->scale).distance(camPos);
        if (d < 1.0)
        {
            d = 1.0;
        }

        Real screenSpaceError = mError / d * k;

        if (screenSpaceError <= mShared->parameters->maxScreenSpaceError / mShared->parameters->scale)
        {
            setChunkVisible(true, false);
            if (mChildren)
            {
                mChildren[0]->setChunkVisible(false, true);
                if (mChildren[1])
                {
                    mChildren[1]->setChunkVisible(false, true);
                    mChildren[2]->setChunkVisible(false, true);
                    mChildren[3]->setChunkVisible(false, true);
                    mChildren[4]->setChunkVisible(false, true);
                    mChildren[5]->setChunkVisible(false, true);
                    mChildren[6]->setChunkVisible(false, true);
                    mChildren[7]->setChunkVisible(false, true);
                }
            }
        }
        else
        {
            setChunkVisible(false, false);
            if (mChildren)
            {
                mChildren[0]->frameStarted(evt);
                if (mChildren[1])
                {
                    mChildren[1]->frameStarted(evt);
                    mChildren[2]->frameStarted(evt);
                    mChildren[3]->frameStarted(evt);
                    mChildren[4]->frameStarted(evt);
                    mChildren[5]->frameStarted(evt);
                    mChildren[6]->frameStarted(evt);
                    mChildren[7]->frameStarted(evt);
                }
            }
            else
            {
                setChunkVisible(true, false);
            }
        }
    
        return true;
    }
    
    //-----------------------------------------------------------------------

    Chunk* Chunk::createInstance(void)
    {
        return OGRE_NEW Chunk();
    }
    
    //-----------------------------------------------------------------------

    void Chunk::setMaterial(const String& matName)
    {
        SimpleRenderable::setMaterial(matName);

        if (mChildren)
        {
            mChildren[0]->setMaterial(matName);
            if (mChildren[1])
            {
                mChildren[1]->setMaterial(matName);
                mChildren[2]->setMaterial(matName);
                mChildren[3]->setMaterial(matName);
                mChildren[4]->setMaterial(matName);
                mChildren[5]->setMaterial(matName);
                mChildren[6]->setMaterial(matName);
                mChildren[7]->setMaterial(matName);
            }
        }
    }
    
    //-----------------------------------------------------------------------

    void Chunk::setMaterialOfLevel(size_t level, const String& matName)
    {
        if (level == 0)
        {
            SimpleRenderable::setMaterial(matName);
        }
        
        if (level > 0 && mChildren)
        {
            mChildren[0]->setMaterialOfLevel(level - 1, matName);
            if (mChildren[1])
            {
                mChildren[1]->setMaterialOfLevel(level - 1, matName);
                mChildren[2]->setMaterialOfLevel(level - 1, matName);
                mChildren[3]->setMaterialOfLevel(level - 1, matName);
                mChildren[4]->setMaterialOfLevel(level - 1, matName);
                mChildren[5]->setMaterialOfLevel(level - 1, matName);
                mChildren[6]->setMaterialOfLevel(level - 1, matName);
                mChildren[7]->setMaterialOfLevel(level - 1, matName);
            }
        }
    }

    //-----------------------------------------------------------------------

    void Chunk::getChunksOfLevel(const size_t level, VecChunk &result) const
    {
        if (level == 0)
        {
            if (!mInvisible)
            {
                result.push_back(this);
            }
            return;
        }

        if (mChildren)
        {
            mChildren[0]->getChunksOfLevel(level - 1, result);
            if (mChildren[1])
            {
                mChildren[1]->getChunksOfLevel(level - 1, result);
                mChildren[2]->getChunksOfLevel(level - 1, result);
                mChildren[3]->getChunksOfLevel(level - 1, result);
                mChildren[4]->getChunksOfLevel(level - 1, result);
                mChildren[5]->getChunksOfLevel(level - 1, result);
                mChildren[6]->getChunksOfLevel(level - 1, result);
                mChildren[7]->getChunksOfLevel(level - 1, result);
            }
        }
    }
        
    //-----------------------------------------------------------------------

    void Chunk::setVolumeVisible(bool visible)
    {
        mShared->volumeVisible = visible;
        setVisible(visible);
        if (mChildren)
        {
            mChildren[0]->setVolumeVisible(visible);
            if (mChildren[1])
            {
                mChildren[1]->setVolumeVisible(visible);
                mChildren[2]->setVolumeVisible(visible);
                mChildren[3]->setVolumeVisible(visible);
                mChildren[4]->setVolumeVisible(visible);
                mChildren[5]->setVolumeVisible(visible);
                mChildren[6]->setVolumeVisible(visible);
                mChildren[7]->setVolumeVisible(visible);
            }
        }
    }
    
    //-----------------------------------------------------------------------

    ChunkParameters* Chunk::getChunkParameters(void)
    {
        return mShared->parameters;
    }
}
}
