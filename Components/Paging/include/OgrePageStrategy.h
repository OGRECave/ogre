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

#ifndef __Ogre_PageStrategy_H__
#define __Ogre_PageStrategy_H__

#include "OgrePagingPrerequisites.h"


namespace Ogre
{
    /** \addtogroup Optional
    *  @{
    */
    /** \addtogroup Paging
    *  Some details on paging component
    *  @{
    */


    /** Abstract marker class representing the data held against the PagedWorldSection
    which is specifically used by the PageStrategy.
    */
    class _OgrePagingExport PageStrategyData : public PageAlloc
    {
    public:
        PageStrategyData() {}
        virtual ~PageStrategyData() {}

        /// Load this data from a stream (returns true if successful)
        virtual bool load(StreamSerialiser& stream) = 0;
        /// Save this data to a stream
        virtual void save(StreamSerialiser& stream) = 0;


    };


    /** Defines the interface to a strategy class which is responsible for deciding
        when Page instances are requested for addition and removal from the 
        paging system.

        The interface is deliberately light, with no specific mention of requesting
        new Page instances. It is entirely up to the PageStrategy to respond
        to the events raised on it and to call methods on other classes (such as
        requesting new pages). 
    */
    class _OgrePagingExport PageStrategy : public PageAlloc
    {
    protected:
        String mName;
        PageManager* mManager;
    public:
        PageStrategy(const String& name, PageManager* manager)
            : mName(name), mManager(manager)
        {

        }

        virtual ~PageStrategy() {}

        const String& getName() const { return mName; }
        PageManager* getManager() const { return mManager; }

        /// Called when the frame starts
        virtual void frameStart(Real timeSinceLastFrame, PagedWorldSection* section) {}
        /// Called when the frame ends
        virtual void frameEnd(Real timeElapsed, PagedWorldSection* section) {}
        /** Called when a camera is used for any kind of rendering.

            This is probably the primary way in which the strategy will request
            new pages. 
        @param cam Camera which is being used for rendering. Class should not
            rely on this pointer remaining valid permanently because no notification 
            will be given when the camera is destroyed.
        @param section
        */
        virtual void notifyCamera(Camera* cam, PagedWorldSection* section) {}

        /** Create a PageStrategyData instance containing the data specific to this
            PageStrategy. 
        @par
            This data will be held by a given PagedWorldSection and the structure of
            the data will be specific to the PageStrategy subclass.
        */
        virtual PageStrategyData* createData() OGRE_NODISCARD = 0;

        /** Destroy a PageStrategyData instance containing the data specific to this
        PageStrategy. 
        @par
        This data will be held by a given PagedWorldSection and the structure of
        the data will be specific to the PageStrategy subclass.
        */
        virtual void destroyData(PageStrategyData* d) = 0;

        /** Update the contents of the passed in SceneNode to reflect the 
            debug display of a given page. 

            The PageStrategy is to have complete control of the contents of this
            SceneNode, it must not be altered / added to by others.
        */
        virtual void updateDebugDisplay(Page* p, SceneNode* sn) = 0;

        /** Get the page ID for a given world position. 
        @return The page ID
        */
        virtual PageID getPageID(const Vector3& worldPos, PagedWorldSection* section) = 0;
    };

    /*@}*/
    /*@}*/
}

#endif
