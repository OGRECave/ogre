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
#if 0
#include "OgreStableHeaders.h"
#include "OgreItem.h"

#include "OgreMeshManager.h"
#include "OgreMesh2.h"
#include "OgreSubMesh2.h"
#include "OgreSubItem.h"
#include "OgreException.h"
#include "OgreSceneManager.h"
#include "OgreLogManager.h"
#include "Animation/OgreSkeletonInstance.h"
#include "OgreCamera.h"
#include "OgreAxisAlignedBox.h"
#include "OgreVector4.h"
#include "OgreRoot.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreOldSkeletonInstance.h"
#include "OgreOptimisedUtil.h"
#include "OgreSceneNode.h"
#include "OgreLodStrategy.h"
#include "OgreLodListener.h"
#include "OgreMaterialManager.h"

namespace Ogre {
    extern const FastArray<Real> c_DefaultLodMesh;
    //-----------------------------------------------------------------------
    Item::Item( IdType id, ObjectMemoryManager *objectMemoryManager )
        : MovableObject( id, objectMemoryManager, 0 ),
          mSkeletonInstance( 0 ),
          mInitialised( false )
    {
        mObjectData.mQueryFlags[mObjectData.mIndex] = SceneManager::QUERY_ENTITY_DEFAULT_MASK;
    }
    //-----------------------------------------------------------------------
    Item::Item( IdType id, ObjectMemoryManager *objectMemoryManager, const MeshPtr& mesh ) :
        MovableObject( id, objectMemoryManager, 0 ),
        mMesh( mesh ),
        mSkeletonInstance( 0 ),
        mInitialised( false )
    {
        _initialise();
        mObjectData.mQueryFlags[mObjectData.mIndex] = SceneManager::QUERY_ENTITY_DEFAULT_MASK;
    }
    //-----------------------------------------------------------------------
    void Item::_initialise(bool forceReinitialise)
    {
        if( forceReinitialise )
            _deinitialise();

        if (mInitialised)
            return;

        if (mMesh->isBackgroundLoaded() && !mMesh->isLoaded())
        {
            // register for a callback when mesh is finished loading
            // do this before asking for load to happen to avoid race
            mMesh->addListener(this);
        }
        
        // On-demand load
        mMesh->load();
        // If loading failed, or deferred loading isn't done yet, defer
        // Will get a callback in the case of deferred loading
        // Skeletons are cascade-loaded so no issues there
        if (!mMesh->isLoaded())
            return;

        // Is mesh skeletally animated?
        if( mMesh->hasSkeleton() && !mMesh->getSkeleton().isNull() && mManager )
        {
            const SkeletonDef *skeletonDef = mMesh->getSkeleton().get();
            mSkeletonInstance = mManager->createSkeletonInstance( skeletonDef );
        }

        mLodMesh = mMesh->_getLodValueArray();

        // Build main subItem list
        buildSubItemList( mMesh, &mSubItemList );

        {
            //Without filling the renderables list, the RenderQueue won't
            //catch our sub entities and thus we won't be rendered
            mRenderables.reserve( mSubItemList.size() );
            SubItemList::iterator itor = mSubItemList.begin();
            SubItemList::iterator end  = mSubItemList.end();
            while( itor != end )
            {
                mRenderables.push_back( &(*itor) );
                ++itor;
            }
        }

        Aabb aabb( mMesh->getBounds().getCenter(), mMesh->getBounds().getHalfSize() );
        mObjectData.mLocalAabb->setFromAabb( aabb, mObjectData.mIndex );
        mObjectData.mWorldAabb->setFromAabb( aabb, mObjectData.mIndex );
        mObjectData.mLocalRadius[mObjectData.mIndex] = aabb.getRadius();
        mObjectData.mWorldRadius[mObjectData.mIndex] = aabb.getRadius();

        mInitialised = true;
    }
    //-----------------------------------------------------------------------
    void Item::_deinitialise(void)
    {
        if (!mInitialised)
            return;

        // Delete submeshes
        mSubItemList.clear();
        mRenderables.clear();

        OGRE_DELETE mSkeletonInstance;
        mSkeletonInstance = 0;

        mInitialised = false;
    }
    //-----------------------------------------------------------------------
    Item::~Item()
    {
        _deinitialise();
        // Unregister our listener
        mMesh->removeListener(this);
    }
    //-----------------------------------------------------------------------
    const MeshPtr& Item::getMesh(void) const
    {
        return mMesh;
    }
    //-----------------------------------------------------------------------
    SubItem* Item::getSubItem(size_t index)
    {
        if (index >= mSubItemList.size())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Index out of bounds.",
                        "Item::getSubItem");
        return &mSubItemList[index];
    }
    //-----------------------------------------------------------------------
    const SubItem* Item::getSubItem(size_t index) const
    {
        if (index >= mSubItemList.size())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
            "Index out of bounds.",
            "Item::getSubItem");
        return &mSubItemList[index];
    }
    //-----------------------------------------------------------------------
    SubItem* Item::getSubItem(const String& name)
    {
        size_t index = mMesh->_getSubMeshIndex(name);
        return getSubItem(index);
    }
    //-----------------------------------------------------------------------
    const SubItem* Item::getSubItem(const String& name) const
    {
        size_t index = mMesh->_getSubMeshIndex(name);
        return getSubItem(index);
    }
    //-----------------------------------------------------------------------
    size_t Item::getNumSubItems(void) const
    {
        return mSubItemList.size();
    }
    //-----------------------------------------------------------------------
    void Item::setDatablock( HlmsDatablock *datablock )
    {
        SubItemList::iterator itor = mSubItemList.begin();
        SubItemList::iterator end  = mSubItemList.end();

        while( itor != end )
        {
            itor->setDatablock( datablock );
            ++itor;
        }
    }
    //-----------------------------------------------------------------------
    Item* Item::clone( const String& newName ) const
    {
        if (!mManager)
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                        "Cannot clone an Item that wasn't created through a "
                        "SceneManager", "Item::clone");
        }
        Item* newEnt = mManager->createItem( newName, getMesh()->getName() );

        if( mInitialised )
        {
            // Copy material settings
            SubItemList::const_iterator i;
            unsigned int n = 0;
            for (i = mSubItemList.begin(); i != mSubItemList.end(); ++i, ++n)
                newEnt->getSubItem(n)->setDatablock( i->getDatablock() );
        }

        return newEnt;
    }
    //-----------------------------------------------------------------------
    void Item::setMaterialName( const String& name, const String& groupName /* = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME */)
    {
        // Set for all subentities
        SubItemList::iterator i;
        for (i = mSubItemList.begin(); i != mSubItemList.end(); ++i)
        {
            i->setMaterialName(name, groupName);
        }

    }


    void Item::setMaterial( const MaterialPtr& material )
    {
        // Set for all subentities
        SubItemList::iterator i;
        for (i = mSubItemList.begin(); i != mSubItemList.end(); ++i)
        {
            i->setMaterial(material);
        }
    }
	//-----------------------------------------------------------------------
    void Item::setUpdateBoundingBoxFromSkeleton(bool update)
	{
		mUpdateBoundingBoxFromSkeleton = update;
		if (mMesh->isLoaded() && mMesh->getBoneBoundingRadius() == Real(0))
		{
			mMesh->_computeBoneBoundingRadius();
		}
	}
    //-----------------------------------------------------------------------
    const String& Item::getMovableType(void) const
    {
        return ItemFactory::FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------
    void Item::buildSubItemList( MeshPtr& mesh, SubItemList* sublist )
    {
        // Create SubEntities
        unsigned short i, numSubMeshes;
        SubMesh* subMesh;

        numSubMeshes = mesh->getNumSubMeshes();
        sublist->reserve( numSubMeshes );
        for (i = 0; i < numSubMeshes; ++i)
        {
            subMesh = mesh->getSubMesh(i);
            sublist->push_back( SubItem( this, subMesh ) );
            if (subMesh->isMatInitialised())
                sublist->back().setMaterialName(subMesh->getMaterialName(), mesh->getGroup());
        }
    }
    //-----------------------------------------------------------------------
    EdgeData* Item::getEdgeList(void)
    {
#if OGRE_NO_MESHLOD
        unsigned short mMeshLodIndex = 0;
#endif
        // Get from Mesh
        return mMesh->getEdgeList(mCurrentMeshLod);
    }
    //-----------------------------------------------------------------------
    bool Item::hasEdgeList(void)
    {
#if OGRE_NO_MESHLOD
        unsigned short mMeshLodIndex = 0;
#endif
        // check if mesh has an edge list attached
        // give mesh a chance to built it if scheduled
        return (mMesh->getEdgeList(mCurrentMeshLod) != NULL);
    }
    //-----------------------------------------------------------------------
    void Item::_notifyAttached( Node* parent )
    {
        MovableObject::_notifyAttached(parent);

        if( mSkeletonInstance )
            mSkeletonInstance->setParentNode( parent );
    }
    //-----------------------------------------------------------------------
    void Item::shareSkeletonInstanceWith(Item* Item)
    {
        if (Item->getMesh()->getOldSkeleton() != getMesh()->getOldSkeleton())
        {
            OGRE_EXCEPT(Exception::ERR_RT_ASSERTION_FAILED,
                "The supplied Item has a different skeleton.",
                "Item::shareSkeletonWith");
        }
        if (!mSkeletonInstance)
        {
            OGRE_EXCEPT(Exception::ERR_RT_ASSERTION_FAILED,
                "This Item has no skeleton.",
                "Item::shareSkeletonWith");
        }
        if (mSharedSkeletonEntities != NULL && Item->mSharedSkeletonEntities != NULL)
        {
            OGRE_EXCEPT(Exception::ERR_RT_ASSERTION_FAILED,
                "Both entities already shares their SkeletonInstances! At least "
                "one of the instances must not share it's instance.",
                "Item::shareSkeletonWith");
        }

        //check if we already share our skeletoninstance, we don't want to delete it if so
        if (mSharedSkeletonEntities != NULL)
        {
            Item->shareSkeletonInstanceWith(this);
        }
        else
        {
            OGRE_DELETE mSkeletonInstance;
            OGRE_FREE_SIMD(mBoneMatrices, MEMCATEGORY_ANIMATION);
            OGRE_DELETE mAnimationState;
            // using OGRE_FREE since unsigned long is not a destructor
            OGRE_FREE(mFrameBonesLastUpdated, MEMCATEGORY_ANIMATION);
            mSkeletonInstance = Item->mSkeletonInstance;
            mNumBoneMatrices = Item->mNumBoneMatrices;
            mBoneMatrices = Item->mBoneMatrices;
            mAnimationState = Item->mAnimationState;
            mFrameBonesLastUpdated = Item->mFrameBonesLastUpdated;
            if (Item->mSharedSkeletonEntities == NULL)
            {
                Item->mSharedSkeletonEntities = OGRE_NEW_T(ItemSet, MEMCATEGORY_ANIMATION)();
                Item->mSharedSkeletonEntities->insert(Item);
            }
            mSharedSkeletonEntities = Item->mSharedSkeletonEntities;
            mSharedSkeletonEntities->insert(this);
        }
    }
    //-----------------------------------------------------------------------
    void Item::stopSharingSkeletonInstance()
    {
        if (mSharedSkeletonEntities == NULL)
        {
            OGRE_EXCEPT(Exception::ERR_RT_ASSERTION_FAILED,
                "This Item is not sharing it's skeletoninstance.",
                "Item::shareSkeletonWith");
        }
        //check if there's no other than us sharing the skeleton instance
        if (mSharedSkeletonEntities->size() == 1)
        {
            //just reset
            OGRE_DELETE_T(mSharedSkeletonEntities, ItemSet, MEMCATEGORY_ANIMATION);
            mSharedSkeletonEntities = 0;
        }
        else
        {
            mSkeletonInstance = OGRE_NEW OldSkeletonInstance(mMesh->getOldSkeleton());
            mSkeletonInstance->load();
            mAnimationState = OGRE_NEW AnimationStateSet();
            mMesh->_initAnimationState(mAnimationState);
            mFrameBonesLastUpdated = OGRE_NEW_T(unsigned long, MEMCATEGORY_ANIMATION)(std::numeric_limits<unsigned long>::max());
            mNumBoneMatrices = mSkeletonInstance->getNumBones();
            mBoneMatrices = static_cast<Matrix4*>(OGRE_MALLOC_SIMD(sizeof(Matrix4) * mNumBoneMatrices, MEMCATEGORY_ANIMATION));

            mSharedSkeletonEntities->erase(this);
            if (mSharedSkeletonEntities->size() == 1)
            {
                (*mSharedSkeletonEntities->begin())->stopSharingSkeletonInstance();
            }
            mSharedSkeletonEntities = 0;
        }
    }
    //-----------------------------------------------------------------------
    void Item::refreshAvailableAnimationState(void)
    {
        mMesh->_refreshAnimationState(mAnimationState);
    }
    //-----------------------------------------------------------------------
    VertexData* Item::getVertexDataForBinding(void)
    {
        Item::VertexDataBindChoice c =
            chooseVertexDataForBinding(mMesh->getSharedVertexDataAnimationType() != VAT_NONE);
        switch(c)
        {
        case BIND_ORIGINAL:
            return mMesh->sharedVertexData;
        case BIND_HARDWARE_MORPH:
            return mHardwareVertexAnimVertexData;
        case BIND_SOFTWARE_MORPH:
            return mSoftwareVertexAnimVertexData;
        case BIND_SOFTWARE_SKELETAL:
            return mSkelAnimVertexData;
        };
        // keep compiler happy
        return mMesh->sharedVertexData;
    }
    //-----------------------------------------------------------------------
    Item::VertexDataBindChoice Item::chooseVertexDataForBinding(bool vertexAnim)
    {
        if (hasSkeleton())
        {
            if (!isHardwareAnimationEnabled())
            {
                // all software skeletal binds same vertex data
                // may be a 2-stage s/w transform including morph earlier though
                return BIND_SOFTWARE_SKELETAL;
            }
            else if (vertexAnim)
            {
                // hardware morph animation
                return BIND_HARDWARE_MORPH;
            }
            else
            {
                // hardware skeletal, no morphing
                return BIND_ORIGINAL;
            }
        }
        else if (vertexAnim)
        {
            // morph only, no skeletal
            if (isHardwareAnimationEnabled())
            {
                return BIND_HARDWARE_MORPH;
            }
            else
            {
                return BIND_SOFTWARE_MORPH;
            }

        }
        else
        {
            return BIND_ORIGINAL;
        }

    }
    //---------------------------------------------------------------------
    void Item::visitRenderables(Renderable::Visitor* visitor,
        bool debugRenderables)
    {
        // Visit each SubItem
        for (SubItemList::iterator i = mSubItemList.begin(); i != mSubItemList.end(); ++i)
        {
            visitor->visit( &(*i), 0, false);
        }
        // if manual LOD is in use, visit those too
        ushort lodi = 1;
        for (LODItemList::iterator e = mLodItemList.begin();
            e != mLodItemList.end(); ++e, ++lodi)
        {
            
            size_t nsub = (*e)->getNumSubEntities();
            for (size_t s = 0; s < nsub; ++s)
            {
                visitor->visit((*e)->getSubItem(s), lodi, false);
            }
        }

    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String ItemFactory::FACTORY_TYPE_NAME = "Item";
    //-----------------------------------------------------------------------
    const String& ItemFactory::getType(void) const
    {
        return FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------
    MovableObject* ItemFactory::createInstanceImpl( IdType id,
                                            ObjectMemoryManager *objectMemoryManager,
                                            const NameValuePairList* params )
    {
        // must have mesh parameter
        MeshPtr pMesh;
        if (params != 0)
        {
            String groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME;

            NameValuePairList::const_iterator ni;

            ni = params->find("resourceGroup");
            if (ni != params->end())
            {
                groupName = ni->second;
            }

            ni = params->find("mesh");
            if (ni != params->end())
            {
                // Get mesh (load if required)
                pMesh = MeshManager::getSingleton().load(
                    ni->second,
                    // autodetect group location
                    groupName );
            }

        }
        if (pMesh.isNull())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "'mesh' parameter required when constructing an Item.",
                "ItemFactory::createInstance");
        }

        return OGRE_NEW Item( id, objectMemoryManager, pMesh );
    }
    //-----------------------------------------------------------------------
    void ItemFactory::destroyInstance( MovableObject* obj)
    {
        OGRE_DELETE obj;
    }


}
#endif
