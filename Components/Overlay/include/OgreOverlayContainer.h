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

#ifndef __OverlayContainer_H__
#define __OverlayContainer_H__

#include "OgreOverlayPrerequisites.h"
#include "OgreOverlayElement.h"
#include "OgreIteratorWrapper.h"


namespace Ogre {


    /** \addtogroup Optional
    *  @{
    */
    /** \addtogroup Overlays
    *  @{
    */
    /** A 2D element which contains other OverlayElement instances.
    @remarks
        This is a specialisation of OverlayElement for 2D elements that contain other
        elements. These are also the smallest elements that can be attached directly
        to an Overlay.
    @remarks
        OverlayContainers should be managed using OverlayManager. This class is responsible for
        instantiating / deleting elements, and also for accepting new types of element
        from plugins etc.
    */
    class _OgreOverlayExport OverlayContainer : public OverlayElement
    {
    public:
        typedef std::map<String, OverlayElement*> ChildMap;
        typedef MapIterator<ChildMap> ChildIterator;
        typedef std::map<String, OverlayContainer*> ChildContainerMap;
        typedef MapIterator<ChildContainerMap> ChildContainerIterator;
    protected:
        /// Map of all children
        ChildMap mChildren;
        /// Map of container children (subset of mChildren)
        ChildContainerMap mChildContainers;

        bool mChildrenProcessEvents;
 
    public:
        /// Constructor: do not call direct, use OverlayManager::createOverlayElement
        OverlayContainer(const String& name);
        virtual ~OverlayContainer();

        /** Adds another OverlayElement to this container. */
        virtual void addChild(OverlayElement* elem);
        /** Adds another OverlayElement to this container. */
        virtual void addChildImpl(OverlayElement* elem);
        /** Add a nested container to this container. */
        virtual void addChildImpl(OverlayContainer* cont);
        /** Removes a named element from this container. */
        virtual void removeChild(const String& name);
        /** Gets the named child of this container. */
        virtual OverlayElement* getChild(const String& name);

        /** @copydoc OverlayElement::initialise */
        void initialise(void);

        void _addChild(OverlayElement* elem);
        void _removeChild(OverlayElement* elem) { _removeChild(elem->getName()); }
        void _removeChild(const String& name);

        /** Gets all the children of this object. */
        const ChildMap& getChildren() const { return mChildren; }

        /// @deprecated use getChildren(
        OGRE_DEPRECATED virtual ChildIterator getChildIterator(void);

        /** Gets an iterator for just the container children of this object.
        @remarks
            Good for cascading updates without having to use RTTI
        */
        virtual ChildContainerIterator getChildContainerIterator(void);

        /** Tell the object and its children to recalculate */
        virtual void _positionsOutOfDate(void);

        /** Overridden from OverlayElement. */
        virtual void _update(void);

        /** Overridden from OverlayElement. */
        virtual ushort _notifyZOrder(ushort newZOrder);

        /** Overridden from OverlayElement. */
        virtual void _notifyViewport();

        /** Overridden from OverlayElement. */
        virtual void _notifyWorldTransforms(const Matrix4& xform);

        /** Overridden from OverlayElement. */
        virtual void _notifyParent(OverlayContainer* parent, Overlay* overlay);

        /** Overridden from OverlayElement. */
        virtual void _updateRenderQueue(RenderQueue* queue);

        /** Overridden from OverlayElement. */
        inline bool isContainer() const
        { return true; }

        /** Should this container pass events to their children */
        virtual inline bool isChildrenProcessEvents() const
        { return true; }

        /** Should this container pass events to their children */
        virtual inline void setChildrenProcessEvents(bool val)
        { mChildrenProcessEvents = val; }

        /** This returns a OverlayElement at position x,y. */
        virtual OverlayElement* findElementAt(Real x, Real y);      // relative to parent

        void copyFromTemplate(OverlayElement* templateOverlay);
        virtual OverlayElement* clone(const String& instanceName);

    };


    /** @} */
    /** @} */

}


#endif

