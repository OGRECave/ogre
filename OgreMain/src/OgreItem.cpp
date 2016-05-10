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
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR     
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR     WISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR      DEALINGS IN
  THE SOFTWARE.
  -----------------------------------------------------------------------------
*/

#include "OgreStableHeaders.h"
#include "OgreItem.h"

#include "OgreMeshManager.h"
#include "OgreMesh2.h"
#include "OgreSubMesh2.h"
#include "OgreSubItem.h"
#include "OgreHlmsManager.h"
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
#include "OgreOptimisedUtil.h"
#include "OgreSceneNode.h"
#include "OgreLodStrategy.h"
#include "OgreLodListener.h"
#include "OgreMaterialManager.h"
#include "OgreMeshManager2.h"

namespace Ogre {
    extern const FastArray<Real> c_DefaultLodMesh;
    //-----------------------------------------------------------------------
    Item::Item( IdType id, ObjectMemoryManager *objectMemoryManager, SceneManager *manager )
        : MovableObject( id, objectMemoryManager, manager, 0 ),
          mInitialised( false )
    {
        mObjectData.mQueryFlags[mObjectData.mIndex] = SceneManager::QUERY_ENTITY_DEFAULT_MASK;
    }
    //-----------------------------------------------------------------------
    Item::Item( IdType id, ObjectMemoryManager *objectMemoryManager, SceneManager *manager,
                const MeshPtr& mesh ) :
        MovableObject( id, objectMemoryManager, manager, 0 ),
        mMesh( mesh ),
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
        buildSubItems();

        {
            //Without filling the renderables list, the RenderQueue won't
            //catch our sub entities and thus we won't be rendered
            mRenderables.reserve( mSubItems.size() );
            SubItemVec::iterator itor = mSubItems.begin();
            SubItemVec::iterator end  = mSubItems.end();
            while( itor != end )
            {
                mRenderables.push_back( &(*itor) );
                ++itor;
            }
        }

        Aabb aabb( mMesh->getAabb() );
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
        mSubItems.clear();
        mRenderables.clear();

        // If mesh is skeletally animated: destroy instance
        assert( mManager || !mSkeletonInstance );
        if( mSkeletonInstance )
        {
            mSkeletonInstance->_decrementRefCount();
            if( mSkeletonInstance->_getRefCount() == 0u )
                mManager->destroySkeletonInstance( mSkeletonInstance );

            mSkeletonInstance = 0;
        }

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
        if (index >= mSubItems.size())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Index out of bounds.",
                        "Item::getSubItem");
        return &mSubItems[index];
    }
    //-----------------------------------------------------------------------
    const SubItem* Item::getSubItem(size_t index) const
    {
        if (index >= mSubItems.size())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
            "Index out of bounds.",
            "Item::getSubItem");
        return &mSubItems[index];
    }
    //-----------------------------------------------------------------------
    size_t Item::getNumSubItems(void) const
    {
        return mSubItems.size();
    }
    //-----------------------------------------------------------------------
    void Item::setDatablock( HlmsDatablock *datablock )
    {
        SubItemVec::iterator itor = mSubItems.begin();
        SubItemVec::iterator end  = mSubItems.end();

        while( itor != end )
        {
            itor->setDatablock( datablock );
            ++itor;
        }
    }
    //-----------------------------------------------------------------------
    void Item::setDatablock( IdString datablockName )
    {
        HlmsManager *hlmsManager = Root::getSingleton().getHlmsManager();
        HlmsDatablock *datablock = hlmsManager->getDatablock( datablockName );

        setDatablock( datablock );
    }
#if 0
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
            SubItemVec::const_iterator i;
            unsigned int n = 0;
            for (i = mSubItems.begin(); i != mSubItems.end(); ++i, ++n)
                newEnt->getSubItem(n)->setDatablock( i->getDatablock() );
        }

        return newEnt;
    }
#endif
    //-----------------------------------------------------------------------
    void Item::setDatablockOrMaterialName( const String& name, const String& groupName /* = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME */)
    {
        // Set for all subentities
        SubItemVec::iterator i;
        for (i = mSubItems.begin(); i != mSubItems.end(); ++i)
        {
            i->setDatablockOrMaterialName(name, groupName);
        }
    }
    //-----------------------------------------------------------------------
    void Item::setMaterialName( const String& name, const String& groupName /* = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME */)
    {
        // Set for all subentities
        SubItemVec::iterator i;
        for (i = mSubItems.begin(); i != mSubItems.end(); ++i)
        {
            i->setMaterialName(name, groupName);
        }
    }
    //-----------------------------------------------------------------------
    void Item::setMaterial( const MaterialPtr& material )
    {
        // Set for all subentities
        SubItemVec::iterator i;
        for (i = mSubItems.begin(); i != mSubItems.end(); ++i)
        {
            i->setMaterial(material);
        }
    }
    //-----------------------------------------------------------------------
    const String& Item::getMovableType(void) const
    {
        return ItemFactory::FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------
    void Item::buildSubItems(void)
    {
        // Create SubEntities
        size_t numSubMeshes = mMesh->getNumSubMeshes();
        mSubItems.reserve( numSubMeshes );
        for( size_t i=0; i<numSubMeshes; ++i )
        {
            SubMesh *subMesh = mMesh->getSubMesh(i);
            mSubItems.push_back( SubItem( this, subMesh ) );

            //Try first Hlms materials, then the low level ones.
            mSubItems.back().setDatablockOrMaterialName( subMesh->mMaterialName, mMesh->getGroup() );
        }
    }
    //-----------------------------------------------------------------------
    void Item::useSkeletonInstanceFrom(Item* master)
    {
        if( mMesh->getSkeletonName() != master->mMesh->getSkeletonName() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "Cannot share skeleton instance if meshes use different skeletons",
                         "Item::useSkeletonInstanceFrom" );
        }

        if( mSkeletonInstance )
        {
            mSkeletonInstance->_decrementRefCount();
            if( mSkeletonInstance->_getRefCount() == 0u )
                mManager->destroySkeletonInstance( mSkeletonInstance );
        }

        mSkeletonInstance = master->mSkeletonInstance;
        mSkeletonInstance->_incrementRefCount();
    }
    //-----------------------------------------------------------------------
    void Item::stopUsingSkeletonInstanceFromMaster()
    {
        if( mSkeletonInstance )
        {
            assert( mSkeletonInstance->_getRefCount() > 1u &&
                    "This skeleton is Item is not sharing its skeleton!" );

            mSkeletonInstance->_decrementRefCount();
            if( mSkeletonInstance->_getRefCount() == 0u )
                mManager->destroySkeletonInstance( mSkeletonInstance );

            const SkeletonDef *skeletonDef = mMesh->getSkeleton().get();
            mSkeletonInstance = mManager->createSkeletonInstance( skeletonDef );
        }
    }
    //-----------------------------------------------------------------------
    bool Item::sharesSkeletonInstance() const             
    { 
        return mSkeletonInstance && mSkeletonInstance->_getRefCount() > 1u;
    }
    //-----------------------------------------------------------------------
    void Item::_notifyParentNodeMemoryChanged(void)
    {
        if( mSkeletonInstance /*&& !mSharedTransformEntity*/ )
        {
            mSkeletonInstance->setParentNode( mSkeletonInstance->getParentNode() );
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
                                                    SceneManager *manager,
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
                pMesh = MeshManager::getSingleton().load( ni->second, groupName );
            }

        }
        if (pMesh.isNull())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "'mesh' parameter required when constructing an Item.",
                "ItemFactory::createInstance");
        }

        return OGRE_NEW Item( id, objectMemoryManager, manager, pMesh );
    }
    //-----------------------------------------------------------------------
    void ItemFactory::destroyInstance( MovableObject* obj)
    {
        OGRE_DELETE obj;
    }


}
