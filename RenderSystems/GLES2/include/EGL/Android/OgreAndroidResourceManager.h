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
#ifndef __AndroidResourceManager_H__
#define __AndroidResourceManager_H__

#include "OgreGLES2Prerequisites.h"

namespace Ogre {
	class AndroidEGLContext;
    class AndroidResource;
    
	class _OgrePrivate AndroidResourceManager : public ResourceAlloc
	{
	// Interface.
	public:

		// Called immediately after the Android context has entered a lost state.
        void notifyOnContextLost();
        
		// Called immediately after the Android context has been reset.
        void notifyOnContextReset();
		
		AndroidResourceManager();
		~AndroidResourceManager();		

	// Friends.
	protected:
		friend class AndroidResource;
	
	// Types.
	protected:
		typedef vector<AndroidResource*>::type	ResourceContainer;
		typedef ResourceContainer::iterator		ResourceContainerIterator;

	// Protected methods.
	protected:
		
		// Called when new resource created.
		void _notifyResourceCreated		(AndroidResource* pResource);

		// Called when resource is about to be destroyed.
		void _notifyResourceDestroyed	(AndroidResource* pResource);
				
	// Attributes.
	protected:		
		ResourceContainer			mResources;
	};
}

#endif
