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
#include "OgreStableHeaders.h"
#include "OgreMesh2.h"

#include "OgreSubMesh2.h"
#include "OgreLogManager.h"
#include "OgreMeshSerializer.h"
#include "OgreHardwareBufferManager.h"
#include "OgreIteratorWrappers.h"
#include "OgreException.h"
#include "OgreMeshManager.h"
#include "OgreOptimisedUtil.h"
#include "OgreSkeleton.h"
#include "OgreLodStrategyManager.h"
#include "OgrePixelCountLodStrategy.h"

#include "Animation/OgreSkeletonDef.h"
#include "Animation/OgreSkeletonManager.h"

#include "Vao/OgreIndexBufferPacked.h"
#include "Vao/OgreVertexArrayObject.h"

#include "OgreOldSkeletonManager.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    Mesh::Mesh( ResourceManager* creator, const String& name, ResourceHandle handle,
                const String& group, VaoManager *vaoManager, bool isManual, ManualResourceLoader* loader )
        : Resource(creator, name, handle, group, isManual, loader),
        mBoundRadius( 0.0f ),
        mLodStrategyName( LodStrategyManager::getSingleton().getDefaultStrategy()->getName() ),
        mNumLods( 1 ),
        mVaoManager( vaoManager ),
        mVertexBufferDefaultType( BT_IMMUTABLE ),
        mIndexBufferDefaultType( BT_IMMUTABLE ),
        mVertexBufferShadowBuffer( true ),
        mIndexBufferShadowBuffer( true )
    {
        mLodValues.push_back( LodStrategyManager::getSingleton().getDefaultStrategy()->getBaseValue() );
    }
    //-----------------------------------------------------------------------
    Mesh::~Mesh()
    {
        // have to call this here reather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        unload();
    }
    //-----------------------------------------------------------------------
    SubMesh* Mesh::createSubMesh()
    {
        SubMesh* sub = OGRE_NEW SubMesh();
        sub->mParent = this;

        mSubMeshes.push_back( sub );

        if( isLoaded() )
            _dirtyState();

        return sub;
    }
    //-----------------------------------------------------------------------
    void Mesh::destroySubMesh(unsigned short index)
    {
        if( index >= mSubMeshes.size() )
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Index out of bounds.",
                        "Mesh::removeSubMesh");
        }

        mSubMeshes.erase( mSubMeshes.begin() + index );

        if (isLoaded())
            _dirtyState();
    }
    //-----------------------------------------------------------------------
    unsigned short Mesh::getNumSubMeshes() const
    {
        return static_cast< unsigned short >( mSubMeshes.size() );
    }
    //-----------------------------------------------------------------------
    SubMesh* Mesh::getSubMesh(unsigned short index) const
    {
        if (index >= mSubMeshes.size())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Index out of bounds.",
                "Mesh::getSubMesh");
        }

        return mSubMeshes[index];
    }
    //-----------------------------------------------------------------------
    void Mesh::postLoadImpl(void)
    {
#if !OGRE_NO_MESHLOD
        // The loading process accesses LOD usages directly, so
        // transformation of user values must occur after loading is complete.

        LodStrategy *lodStrategy = LodStrategyManager::getSingleton().getDefaultStrategy();

        /*assert( mLodValues.size() == mMeshLodUsageList.size()  );
        LodValueArray::iterator lodValueIt = mLodValues.begin();
        // Transform user LOD values (starting at index 1, no need to transform base value)
        for (MeshLodUsageList::iterator i = mMeshLodUsageList.begin(); i != mMeshLodUsageList.end(); ++i)
        {
            i->value = lodStrategy->transformUserValue(i->userValue);
            *lodValueIt++ = i->value;
        }*/
#endif
    }
    //-----------------------------------------------------------------------
    void Mesh::prepareImpl()
    {
        // Load from specified 'name'
        if (getCreator()->getVerbose())
            LogManager::getSingleton().logMessage("Mesh: Loading "+mName+".");

        mFreshFromDisk =
            ResourceGroupManager::getSingleton().openResource(
                mName, mGroup, true, this);
 
        // fully prebuffer into host RAM
        mFreshFromDisk = DataStreamPtr(OGRE_NEW MemoryDataStream(mName,mFreshFromDisk));
    }
    //-----------------------------------------------------------------------
    void Mesh::unprepareImpl()
    {
        mFreshFromDisk.setNull();
    }
    //-----------------------------------------------------------------------
    void Mesh::loadImpl()
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "Mesh::loadImpl()");

        /*MeshSerializer serializer;
        serializer.setListener(MeshManager::getSingleton().getListener());

        // If the only copy is local on the stack, it will be cleaned
        // up reliably in case of exceptions, etc
        DataStreamPtr data(mFreshFromDisk);
        mFreshFromDisk.setNull();

        if (data.isNull()) {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
                        "Data doesn't appear to have been prepared in " + mName,
                        "Mesh::loadImpl()");
        }

        serializer.importMesh(data, this);*/
    }
    //-----------------------------------------------------------------------
    void Mesh::unloadImpl()
    {
        // Teardown submeshes
        SubMeshVec::const_iterator itor = mSubMeshes.begin();
        SubMeshVec::const_iterator end  = mSubMeshes.end();
        while( itor != end )
            OGRE_DELETE *itor++;

        mSubMeshes.clear();
#if !OGRE_NO_MESHLOD
        // Removes all LOD data
        removeLodLevels();
#endif

        // Removes reference to skeleton
        setSkeletonName( BLANKSTRING );
    }
    //-----------------------------------------------------------------------
    /*MeshPtr Mesh::clone(const String& newName, const String& newGroup)
    {
        // This is a bit like a copy constructor, but with the additional aspect of registering the clone with
        //  the MeshManager

        // New Mesh is assumed to be manually defined rather than loaded since you're cloning it for a reason
        String theGroup;
        if (newGroup == BLANKSTRING)
        {
            theGroup = this->getGroup();
        }
        else
        {
            theGroup = newGroup;
        }
        MeshPtr newMesh = MeshManager::getSingleton().createManual(newName, theGroup);

        // Copy submeshes first
        vector<SubMesh*>::type::iterator subi;
        for (subi = mSubMeshes.begin(); subi != mSubMeshes.end(); ++subi)
        {
            (*subi)->clone("", newMesh.get());

        }

        // Copy shared geometry and index map, if any
        if (sharedVertexData)
        {
            newMesh->sharedVertexData = sharedVertexData->clone();
            newMesh->sharedBlendIndexToBoneIndexMap = sharedBlendIndexToBoneIndexMap;
        }

        // Copy submesh names
        newMesh->mSubMeshNameMap = mSubMeshNameMap ;
        // Copy any bone assignments
        newMesh->mBoneAssignments = mBoneAssignments;
        newMesh->mBoneAssignmentsOutOfDate = mBoneAssignmentsOutOfDate;
        // Copy bounds
        newMesh->mAABB = mAABB;
        newMesh->mBoundRadius = mBoundRadius;
        newMesh->mBoneBoundingRadius = mBoneBoundingRadius;
        newMesh->mAutoBuildEdgeLists = mAutoBuildEdgeLists;
        newMesh->mEdgeListsBuilt = mEdgeListsBuilt;

		newMesh->mLodStrategyName = mLodStrategyName;
		newMesh->mHasManualLodLevel = mHasManualLodLevel;
        newMesh->mNumLods = mNumLods;
        newMesh->mMeshLodUsageList  = mMeshLodUsageList;
        newMesh->mLodValues         = mLodValues;

        // Unreference edge lists, otherwise we'll delete the same lot twice, build on demand
        MeshLodUsageList::iterator lodi, lodOldi;
        lodOldi = mMeshLodUsageList.begin();
        for (lodi = newMesh->mMeshLodUsageList.begin(); lodi != newMesh->mMeshLodUsageList.end(); ++lodi, ++lodOldi)
		{
            MeshLodUsage& newLod = *lodi;
            MeshLodUsage& lod = *lodOldi;
            newLod.manualName = lod.manualName;
            newLod.userValue = lod.userValue;
            newLod.value = lod.value;
            if (lod.edgeData)
                newLod.edgeData = lod.edgeData->clone();
        }

        newMesh->mVertexBufferUsage = mVertexBufferUsage;
        newMesh->mIndexBufferUsage = mIndexBufferUsage;
        newMesh->mVertexBufferShadowBuffer = mVertexBufferShadowBuffer;
        newMesh->mIndexBufferShadowBuffer = mIndexBufferShadowBuffer;

        newMesh->mSkeletonName = mSkeletonName;
        newMesh->mOldSkeleton = mOldSkeleton;
        newMesh->mSkeleton    = mSkeleton;

        // Keep prepared shadow volume info (buffers may already be prepared)
        newMesh->mPreparedForShadowVolumes = mPreparedForShadowVolumes;

        newMesh->mEdgeListsBuilt = mEdgeListsBuilt;
        
        // Clone vertex animation
        for (AnimationList::iterator i = mAnimationsList.begin();
            i != mAnimationsList.end(); ++i)
        {
            Animation *newAnim = i->second->clone(i->second->getName());
            newMesh->mAnimationsList[i->second->getName()] = newAnim;
        }
        // Clone pose list
        for (PoseList::iterator i = mPoseList.begin(); i != mPoseList.end(); ++i)
        {
            Pose* newPose = (*i)->clone();
            newMesh->mPoseList.push_back(newPose);
        }
        newMesh->mSharedVertexDataAnimationType = mSharedVertexDataAnimationType;
        newMesh->mAnimationTypesDirty = true;

        newMesh->load();
        newMesh->touch();

        return newMesh;
    }*/
    //-----------------------------------------------------------------------
    const Aabb& Mesh::getAabb(void) const
    {
        return mAabb;
    }
    //-----------------------------------------------------------------------
    void Mesh::_setBounds(const Aabb& bounds, bool pad)
    {
        mAabb           = bounds;
        mBoundRadius    = mAabb.getRadius();

        if (pad)
        {
            // Pad out the AABB a little, helps with most bounds tests
            mAabb.mHalfSize += 2.0f * mAabb.mHalfSize *
                                v1::MeshManager::getSingleton().getBoundsPaddingFactor();
            // Pad out the sphere a little too
            mBoundRadius = mBoundRadius + (mBoundRadius * v1::MeshManager::getSingleton().getBoundsPaddingFactor());
        }
    }
    //-----------------------------------------------------------------------
    void Mesh::_setBoundingSphereRadius(Real radius)
    {
        mBoundRadius = radius;
    }
    //-----------------------------------------------------------------------
    void Mesh::_updateBoundsFromVertexBuffers(bool pad)
    {
        /*bool extendOnly = false; // First time we need full AABB of the given submesh, but on the second call just extend that one.

        for (size_t i = 0; i < mSubMeshes.size(); i++)
        {
            mSubMeshes[i]->_calcBoundsFromVertexBuffer( mAABB, mBoundRadius, extendOnly );
            extendOnly = true;
        }

        if (pad)
        {
            Vector3 max = mAABB.getMaximum();
            Vector3 min = mAABB.getMinimum();
            // Pad out the AABB a little, helps with most bounds tests
            Vector3 scaler = (max - min) * MeshManager::getSingleton().getBoundsPaddingFactor();
            mAABB.setExtents(min - scaler, max + scaler);
            // Pad out the sphere a little too
            mBoundRadius = mBoundRadius + (mBoundRadius * MeshManager::getSingleton().getBoundsPaddingFactor());
        }*/
    }
    //-----------------------------------------------------------------------
    void Mesh::setSkeletonName(const String& skelName)
    {
        if (skelName != mSkeletonName)
        {
            mSkeletonName = skelName;

            if (skelName.empty())
            {
                // No skeleton
                mSkeleton.setNull();
            }
            else
            {
                // Load skeleton
                try {
                    v1::SkeletonPtr oldSkeleton = v1::OldSkeletonManager::getSingleton().load(skelName, mGroup).staticCast<v1::Skeleton>();

                    //TODO: put mOldSkeleton in legacy mode only.
                    mSkeleton = SkeletonManager::getSingleton().getSkeletonDef( oldSkeleton.get() );
                }
                catch (...)
                {
                    mSkeleton.setNull();
                    // Log this error
                    String msg = "Unable to load skeleton ";
                    msg += skelName + " for Mesh " + mName
                        + ". This Mesh will not be animated. "
                        + "You can ignore this message if you are using an offline tool.";
                    LogManager::getSingleton().logMessage(msg);

                }


            }
            if (isLoaded())
                _dirtyState();
        }
    }
    //---------------------------------------------------------------------
    void Mesh::_notifySkeleton( v1::SkeletonPtr& pSkel )
    {
        mSkeletonName = pSkel->getName();
        mSkeleton = SkeletonManager::getSingleton().getSkeletonDef( pSkel.get() );
    }
    //---------------------------------------------------------------------
    const String& Mesh::getSkeletonName(void) const
    {
        return mSkeletonName;
    }
    //---------------------------------------------------------------------
    ushort Mesh::getNumLodLevels(void) const
    {
        return mNumLods;
    }
    //---------------------------------------------------------------------
    void Mesh::_setLodInfo(unsigned short numLevels)
    {
        /*assert(!mEdgeListsBuilt && "Can't modify LOD after edge lists built");

        // Basic prerequisites
        assert(numLevels > 0 && "Must be at least one level (full detail level must exist)");

        mNumLods = numLevels;
        mLodValues.resize(numLevels);*/
        // Resize submesh face data lists too
        /*for (SubMeshList::iterator i = mSubMeshes.begin(); i != mSubMeshes.end(); ++i)
        {
            (*i)->mLodFaceList.resize(numLevels - 1);
        }*/
    }
    //---------------------------------------------------------------------
    /*void Mesh::_setSubMeshLodFaceList(unsigned short subIdx, unsigned short level,
        IndexData* facedata)
    {
        assert(!mEdgeListsBuilt && "Can't modify LOD after edge lists built");

        // Basic prerequisites
        assert(mMeshLodUsageList[level].manualName.empty() && "Not using generated LODs!");
        assert(subIdx < mSubMeshes.size() && "Index out of bounds");
        assert(level != 0 && "Can't modify first LOD level (full detail)");
        assert(level-1 < (unsigned short)mSubMeshes[subIdx]->mLodFaceList.size() && "Index out of bounds");

        SubMesh* sm = mSubMeshes[subIdx];
        sm->mLodFaceList[level - 1] = facedata;
    }*/
    //--------------------------------------------------------------------
    void Mesh::removeLodLevels(void)
    {
#if !OGRE_NO_MESHLOD
        // Remove data from SubMeshes
        /*SubMeshList::iterator isub, isubend;
        isubend = mSubMeshes.end();
        for (isub = mSubMeshes.begin(); isub != isubend; ++isub)
        {
            (*isub)->removeLodLevels();
        }

        freeEdgeList();
        mMeshLodUsageList.clear();
        mLodValues.clear();

        LodStrategy *lodStrategy = LodStrategyManager::getSingleton().getDefaultStrategy();

        // Reinitialise
        mNumLods = 1;
        mMeshLodUsageList.resize(1);
        mMeshLodUsageList[0].edgeData = NULL;
        // TODO: Shouldn't we rebuild edge lists after freeing them?
        mLodValues.push_back( lodStrategy->getBaseValue() );*/
#endif
    }

    //---------------------------------------------------------------------
    Real Mesh::getBoundingSphereRadius(void) const
    {
        return mBoundRadius;
    }
    //---------------------------------------------------------------------
    void Mesh::setVertexBufferPolicy( BufferType bufferType, bool shadowBuffer )
    {
        mVertexBufferDefaultType    = bufferType;
        mVertexBufferShadowBuffer   = shadowBuffer;
    }
    //---------------------------------------------------------------------
    void Mesh::setIndexBufferPolicy( BufferType bufferType, bool shadowBuffer )
    {
        mIndexBufferDefaultType     = bufferType;
        mIndexBufferShadowBuffer    = shadowBuffer;
    }
    //---------------------------------------------------------------------
    void Mesh::nameSubMesh(const String& name, ushort index)
    {
        mSubMeshNameMap[name] = index;
    }
    //---------------------------------------------------------------------
    void Mesh::unnameSubMesh(const String& name)
    {
        SubMeshNameMap::iterator i = mSubMeshNameMap.find( name );
        if( i != mSubMeshNameMap.end() )
            mSubMeshNameMap.erase(i);
    }
    //---------------------------------------------------------------------
    size_t Mesh::calculateSize(void) const
    {
        // calculate GPU size
        size_t retVal = 0;

        SubMeshVec::const_iterator itor = mSubMeshes.begin();
        SubMeshVec::const_iterator end  = mSubMeshes.end();

        while( itor != end )
        {
            SubMesh *s = *itor;
            VertexArrayObjectArray::const_iterator itVao = s->mVao.begin();
            VertexArrayObjectArray::const_iterator enVao = s->mVao.end();

            while( itVao != enVao )
            {
                VertexArrayObject *vao = *itVao;
                VertexBufferPackedVec::const_iterator itVertexBuf = vao->getVertexBuffers().begin();
                VertexBufferPackedVec::const_iterator enVertexBuf = vao->getVertexBuffers().end();

                while( itVertexBuf != enVertexBuf )
                {
                    retVal += (*itVertexBuf)->getTotalSizeBytes();
                    ++itVertexBuf;
                }

                if( vao->getIndexBuffer() )
                    retVal += vao->getIndexBuffer()->getTotalSizeBytes();
                ++itVao;
            }
            ++itor;
        }

        return retVal;
    }
    //---------------------------------------------------------------------
    void Mesh::importV1( v1::Mesh *mesh, bool halfPos, bool halfTexCoords )
    {
        mAabb.setExtents( mesh->getBounds().getMinimum(), mesh->getBounds().getMaximum() );
        mBoundRadius = mesh->getBoundingSphereRadius();

        try
        {
            unsigned short sourceCoordSet;
            unsigned short index;
            bool alreadyHasTangents = mesh->suggestTangentVectorBuildParams( VES_TANGENT,
                                                                             sourceCoordSet, index );
            if( !alreadyHasTangents )
                mesh->buildTangentVectors( VES_TANGENT, sourceCoordSet, index, false, false, true );
        }
        catch( Exception &e )
        {
        }

        for( size_t i=0; i<mesh->getNumSubMeshes(); ++i )
        {
            SubMesh *subMesh = createSubMesh();
            subMesh->importFromV1( mesh->getSubMesh( i ), halfPos, halfTexCoords );
        }

        v1::SkeletonPtr v1Skeleton = mesh->getOldSkeleton();
        if( !v1Skeleton.isNull() )
        {
            mSkeleton = SkeletonManager::getSingleton().getSkeletonDef( v1Skeleton.get() );
            mSkeletonName = mSkeleton->getName();
        }

        mIsManual = true;
        setToLoaded();
    }
    //---------------------------------------------------------------------
}

