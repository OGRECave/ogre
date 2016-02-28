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

#ifndef __Ogre_PageContentCollection_H__
#define __Ogre_PageContentCollection_H__

#include "OgrePagingPrerequisites.h"


namespace Ogre
{
    /** \addtogroup Optional Components
    *  @{
    */
    /** \addtogroup Paging
    *  Some details on paging component
    *  @{
    */


    /** Definition of the interface for a collection of PageContent instances. 
    @remarks
        This class acts as a grouping level for PageContent instances. Rather than 
        PageContent instances being held in a list directly under Page, which might 
        be the most obvious solution, this intermediate class is here to allow
        the collection of relevant PageContent instances to be modified at runtime
        if required. For example, potentially you might want to define Page-level LOD
        in which different collections of PageContent are loaded at different times.
    */
    class _OgrePagingExport PageContentCollection : public PageAlloc
    {
    protected:
        PageContentCollectionFactory* mCreator;
        Page* mParent;
    public:
        static const uint32 CHUNK_ID;
        static const uint16 CHUNK_VERSION;

        PageContentCollection(PageContentCollectionFactory* creator);
        virtual ~PageContentCollection();

        PageManager* getManager() const;
        Page* getParentPage() const { return mParent; }
        SceneManager* getSceneManager() const;

        /// Get the type of the collection, which will match it's factory
        virtual const String& getType() const;

        /// Internal method to notify a collection that it is attached
        virtual void _notifyAttached(Page* parent);
        /// Save the collection to a stream
        virtual void save(StreamSerialiser& stream) = 0;
        /// Called when the frame starts
        virtual void frameStart(Real timeSinceLastFrame) = 0;
        /// Called when the frame ends
        virtual void frameEnd(Real timeElapsed) = 0;
        /// Notify a section of the current camera
        virtual void notifyCamera(Camera* cam) = 0;


        /// Prepare data - may be called in the background
        virtual bool prepare(StreamSerialiser& ser) = 0;
        /// Load - will be called in main thread
        virtual void load() = 0;
        /// Unload - will be called in main thread
        virtual void unload() = 0;
        /// Unprepare data - may be called in the background
        virtual void unprepare() = 0;


    };


    /** @} */
    /** @} */
}

#endif
