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

#ifndef __Ogre_PageContent_H__
#define __Ogre_PageContent_H__

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


	/** Interface definition for a unit of content within a page. 
	*/
	class _OgrePagingExport PageContent : public PageAlloc
	{
	protected:
		PageContentFactory* mCreator;
		PageContentCollection* mParent;
	public:
		PageContent(PageContentFactory* creator);
		virtual ~PageContent();

		PageManager* getManager() const;
		SceneManager* getSceneManager() const;

		/// Internal method to notify a page that it is attached
		virtual void _notifyAttached(PageContentCollection* parent);
		/// Get the type of the content, which will match it's factory
		virtual const String& getType() const;

		/// Save the content to a stream
		virtual void save(StreamSerialiser& stream) = 0;
		/// Called when the frame starts
		virtual void frameStart(Real timeSinceLastFrame) {}
		/// Called when the frame ends
		virtual void frameEnd(Real timeElapsed) {}
		/// Notify a section of the current camera
		virtual void notifyCamera(Camera* cam) {}

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
