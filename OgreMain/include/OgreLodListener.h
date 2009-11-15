/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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

#ifndef __LodListener_H__
#define __LodListener_H__


#include "OgrePrerequisites.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup LOD
	*  @{
	*/
	/// Struct containing information about a lod change event for movable objects.
    struct MovableObjectLodChangedEvent
    {
        /// The movable object whose level of detail has changed.
        MovableObject *movableObject;

        /// The camera with respect to which the level of detail has changed.
        Camera *camera;
    };

    /// Struct containing information about a mesh lod change event for entities.
    struct EntityMeshLodChangedEvent
    {
        /// The entity whose level of detail has changed.
        Entity *entity;

        /// The camera with respect to which the level of detail has changed.
        Camera *camera;

        /// Lod value as determined by lod strategy.
        Real lodValue;

        /// Previous level of detail index.
        ushort previousLodIndex;

        /// New level of detail index.
        ushort newLodIndex;
    };

    /// Struct containing information about a material lod change event for entities.
    struct EntityMaterialLodChangedEvent
    {
        /// The sub-entity whose material's level of detail has changed.
        SubEntity *subEntity;

        /// The camera with respect to which the level of detail has changed.
        Camera *camera;

        /// Lod value as determined by lod strategy.
        Real lodValue;

        /// Previous level of detail index.
        ushort previousLodIndex;

        /// New level of detail index.
        ushort newLodIndex;
    };


    /** A interface class defining a listener which can be used to receive
        notifications of lod events.
        @remarks
            A 'listener' is an interface designed to be called back when
            particular events are called. This class defines the
            interface relating to lod events. In order to receive
            notifications of lod events, you should create a subclass of
            LodListener and override the methods for which you would like
            to customise the resulting processing. You should then call
            SceneManager::addLodListener passing an instance of this class.
            There is no limit to the number of lod listeners you can register,
            allowing you to register multiple listeners for different purposes.

            For some uses, it may be advantageous to also subclass
            RenderQueueListener as this interface makes available information
            regarding render queue invocations.

            It is important not to modify the scene graph during rendering, so,
            for each event, there are two methods, a prequeue method and a
            postqueue method.  The prequeue method is invoked during rendering,
            and as such should not perform any changes, but if the event is
            relevant, it may return true indicating the postqueue method should
            also be called.  The postqueue method is invoked at an appropriate
            time after rendering and scene changes may be safely made there.
    */
    class _OgreExport LodListener
    {
    public:

        virtual ~LodListener() {}

        /**
        Called before a movable object's lod has changed.
        @remarks
            Do not change the Ogre state from this method, 
            instead return true and perform changes in 
            postqueueMovableObjectLodChanged.
        @return
            True to indicate the event should be queued and
            postqueueMovableObjectLodChanged called after
            rendering is complete.
        */
        virtual bool prequeueMovableObjectLodChanged(const MovableObjectLodChangedEvent& evt)
        { (void)evt; return false; }

        /**
        Called after a movable object's lod has changed.
        @remarks
            May be called even if not requested from prequeueMovableObjectLodChanged
            as only one event queue is maintained per SceneManger instance.
        */
        virtual void postqueueMovableObjectLodChanged(const MovableObjectLodChangedEvent& evt)
        { (void)evt; }

        /**
        Called before an entity's mesh lod has changed.
        @remarks
            Do not change the Ogre state from this method, 
            instead return true and perform changes in 
            postqueueEntityMeshLodChanged.

            It is possible to change the event notification 
            and even alter the newLodIndex field (possibly to 
            prevent the lod from changing, or to skip an 
            index).
        @return
            True to indicate the event should be queued and
            postqueueEntityMeshLodChanged called after
            rendering is complete.
        */
        virtual bool prequeueEntityMeshLodChanged(EntityMeshLodChangedEvent& evt)
        { (void)evt; return false; }

        /**
        Called after an entity's mesh lod has changed.
        @remarks
            May be called even if not requested from prequeueEntityMeshLodChanged
            as only one event queue is maintained per SceneManger instance.
        */
        virtual void postqueueEntityMeshLodChanged(const EntityMeshLodChangedEvent& evt)
        { (void)evt; }

        /**
        Called before an entity's material lod has changed.
        @remarks
            Do not change the Ogre state from this method, 
            instead return true and perform changes in 
            postqueueMaterialLodChanged.

            It is possible to change the event notification 
            and even alter the newLodIndex field (possibly to 
            prevent the lod from changing, or to skip an 
            index).
        @return
            True to indicate the event should be queued and
            postqueueMaterialLodChanged called after
            rendering is complete.
        */
        virtual bool prequeueEntityMaterialLodChanged(EntityMaterialLodChangedEvent& evt)
        { (void)evt; return false; }

        /**
        Called after an entity's material lod has changed.
        @remarks
            May be called even if not requested from prequeueEntityMaterialLodChanged
            as only one event queue is maintained per SceneManger instance.
        */
        virtual void postqueueEntityMaterialLodChanged(const EntityMaterialLodChangedEvent& evt)
        { (void)evt; }

    };
	/** @} */
	/** @} */
}

#endif
