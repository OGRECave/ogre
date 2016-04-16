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
#ifndef _OgreItem_H_
#define _OgreItem_H_

#include "OgrePrerequisites.h"
#include "OgreCommon.h"

#include "OgreMovableObject.h"
#include "OgreQuaternion.h"
#include "OgreVector3.h"
#include "OgreHardwareBufferManager.h"
#include "OgreRenderable.h"
#include "OgreResourceGroupManager.h"
#include "OgreSubItem.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */
    /** Defines an instance of a discrete, movable object based on a Mesh.
    @remarks
        Ogre generally divides renderable objects into 2 groups, discrete
        (separate) and relatively small objects which move around the world,
        and large, sprawling geometry which makes up generally immovable
        scenery, aka 'level geometry'.
    @par
        The Mesh and SubMesh classes deal with the definition of the geometry
        used by discrete movable objects. Entities are actual instances of
        objects based on this geometry in the world. Therefore there is
        usually a single set Mesh for a car, but there may be multiple
        entities based on it in the world. Entities are able to override
        aspects of the Mesh it is defined by, such as changing material
        properties per instance (so you can have many cars using the same
        geometry but different textures for example). Because a Mesh is split
        into SubMeshes for this purpose, the Item class is a grouping class
        (much like the Mesh class) and much of the detail regarding
        individual changes is kept in the SubItem class. There is a 1:1
        relationship between SubItem instances and the SubMesh instances
        associated with the Mesh the Item is based on.
    @par
        Item and SubItem classes are never created directly. Use the
        createItem method of the SceneManager (passing a model name) to
        create one.
    @par
        Entities are included in the scene by associating them with a
        SceneNode, using the attachItem method. See the SceneNode class
        for full information.
    @note
        No functions were declared virtual to improve performance.
    */
    class _OgreExport Item : public MovableObject, public Resource::Listener
    {
        // Allow ItemFItemy full access
        friend class ItemFactory;
        friend class SubItem;
    public:
        //typedef set<Item*>::type ItemSet;

    protected:

        /** Private constructor (instances cannot be created directly).
        */
        Item( IdType id, ObjectMemoryManager *objectMemoryManager, SceneManager *manager );
        /** Private constructor.
        */
        Item( IdType id, ObjectMemoryManager *objectMemoryManager, SceneManager *manager,
              const MeshPtr& mesh );

        /** The Mesh that this Item is based on.
        */
        MeshPtr mMesh;

        /** List of SubEntities (point to SubMeshes).
        */
        typedef vector<SubItem>::type SubItemVec;
        SubItemVec mSubItems;

        /** A set of all the entities which shares a single OldSkeletonInstance.
            This is only created if the Item is in fact sharing it's OldSkeletonInstance with
            other Entities.
        */
        //ItemSet* mSharedSkeletonEntities;

        /// Has this Item been initialised yet?
        bool mInitialised;

        /** Builds a list of SubItems based on the SubMeshes contained in the Mesh. */
        void buildSubItems(void);

    public:
        /** Default destructor.
        */
        ~Item();

        /** Gets the Mesh that this Item is based on.
        */
        const MeshPtr& getMesh(void) const;

        /** Gets a pointer to a SubItem, ie a part of an Item.
        */
        SubItem* getSubItem(size_t index);
        const SubItem* getSubItem(size_t index) const;

        /** Retrieves the number of SubItem objects making up this Item.
        */
        size_t getNumSubItems(void) const;

        /// Sets the given HLMS databloock to all SubEntities
        void setDatablock( HlmsDatablock *datablock );

        /// Sets the given HLMS datablock to all SubEntities
        void setDatablock( IdString datablockName );

        /** Clones this Item and returns a pointer to the clone.
        @remarks
            Useful method for duplicating an Item. The new Item must be
            given a unique name, and is not attached to the scene in any way
            so must be attached to a SceneNode to be visible (exactly as
            entities returned from SceneManager::createItem).
        @param newName
            Name for the new Item.
        */
        Item* clone( const String& newName ) const;

        /** Sets the material to use for the whole of this Item.
        @remarks
            This is a shortcut method to set all the materials for all
            subentities of this Item. Only use this method is you want to
            set the same material for all subentities or if you know there
            is only one. Otherwise call getSubItem() and call the same
            method on the individual SubItem.
        */
        void setDatablockOrMaterialName( const String& name,
                                         const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME );

        /** Sets the material to use for the whole of this Item.
        @remarks
            This is a shortcut method to set all the materials for all
            subentities of this Item. Only use this method is you want to
            set the same material for all subentities or if you know there
            is only one. Otherwise call getSubItem() and call the same
            method on the individual SubItem.
        */
        void setMaterialName( const String& name, const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME );
        
        /** Sets the material to use for the whole of this Item.
        @remarks
            This is a shortcut method to set all the materials for all
            subentities of this Item. Only use this method is you want to
            set the same material for all subentities or if you know there
            is only one. Otherwise call getSubItem() and call the same
            method on the individual SubItem.
        */
        void setMaterial(const MaterialPtr& material);

        /** @copydoc MovableObject::getMovableType */
        const String& getMovableType(void) const;

        /** Returns whether or not this Item is skeletally animated. */
        bool hasSkeleton(void) const                    { return mSkeletonInstance != 0; }

        /** Starts using the SkeletonInstance from 'master' instead of using our own.
            Useful when multiple Items can use exactly the same skeleton (e.g. on
            RPG games the master could be the player's character, then the clothes,
            and armor the slaves sharing the skeleton instance with its master)
        @remarks
            Calling
            armour->useSkeletonInstanceFrom( player );
            armour->useSkeletonInstanceFrom( newPlayer );

            is valid and more efficient than calling:
            armour->useSkeletonInstanceFrom( player );
            armour->stopUsingSkeletonInstanceFromMaster();
            armour->useSkeletonInstanceFrom( newPlayer );
        @par
            Due to the nature of skeleton sharing, all slaves will end up in the position
            of the master. Therefore it is highly recommended you attach the slaves and
            master to the same SceneNode, or that you keep all SceneNodes' transforms in
            sync.
            Failure to do so could cause Ogre to cull some of those objects because
            it thinks they're located somewhere else.
        */
        void useSkeletonInstanceFrom( Item* master );

        /// Stops sharing the SkeletonInstance with other Items. @see useSkeletonInstanceFrom
        void stopUsingSkeletonInstanceFromMaster();

        /** Returns whether this Item shares it's SkeltonInstance with other Item instances.
        */
        bool sharesSkeletonInstance() const;

        /** Returns whether or not this Item is either morph or pose animated.
        */
        bool hasVertexAnimation(void) const;                

        /** Returns a pointer to the set of entities which share a OldSkeletonInstance.
            If this instance does not share it's OldSkeletonInstance with other instances @c NULL will be returned
        */
        //const ItemSet* getSkeletonInstanceSharingSet() const    { return mSharedSkeletonEntities; }

        /** Has this Item been initialised yet?
        @remarks
            If this returns false, it means this Item hasn't been completely
            constructed yet from the underlying resources (Mesh, Skeleton), which 
            probably means they were delay-loaded and aren't available yet. This
            Item won't render until it has been successfully initialised, nor
            will many of the manipulation methods function.
        */
        bool isInitialised(void) const { return mInitialised; }

        /** Try to initialise the Item from the underlying resources.
        @remarks
            This method builds the internal structures of the Item based on it
            resources (Mesh, Skeleton). This may or may not succeed if the 
            resources it references have been earmarked for background loading,
            so you should check isInitialised afterwards to see if it was successful.
        @param forceReinitialise
            If @c true, this forces the Item to tear down it's
            internal structures and try to rebuild them. Useful if you changed the
            content of a Mesh or Skeleton at runtime.
        */
        void _initialise(bool forceReinitialise = false);
        /** Tear down the internal structures of this Item, rendering it uninitialised. */
        void _deinitialise(void);

        virtual void _notifyParentNodeMemoryChanged(void);
    };

    /** FItemy object for creating Item instances */
    class _OgreExport ItemFactory : public MovableObjectFactory
    {
    protected:
        virtual MovableObject* createInstanceImpl( IdType id, ObjectMemoryManager *objectMemoryManager,
                                                   SceneManager *manager,
                                                   const NameValuePairList* params = 0 );
    public:
        ItemFactory() {}
        ~ItemFactory() {}

        static String FACTORY_TYPE_NAME;

        const String& getType(void) const;
        void destroyInstance( MovableObject* obj);

    };
    /** @} */
    /** @} */

} // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif // __Item_H__
