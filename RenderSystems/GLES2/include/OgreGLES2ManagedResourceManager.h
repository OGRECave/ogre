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
#ifndef __GLES2ManagedResourceManager_H__
#define __GLES2ManagedResourceManager_H__

#include "OgreGLES2Prerequisites.h"

namespace Ogre {

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN

    class EGLContext;
    class GLES2ManagedResource;
    
    class _OgrePrivate GLES2ManagedResourceManager : public ResourceAlloc
    {
    // Interface.
    public:

        // Called immediately after the Android context has entered a lost state.
        void notifyOnContextLost();
        
        // Called immediately after the Android context has been reset.
        void notifyOnContextReset();
        
        GLES2ManagedResourceManager();
        ~GLES2ManagedResourceManager();

    // Friends.
    protected:
        friend class GLES2ManagedResource;
    
    // Types.
    protected:
        typedef std::vector<GLES2ManagedResource*>  ResourceContainer;
        typedef ResourceContainer::iterator     ResourceContainerIterator;

    // Protected methods.
    protected:
        
        // Called when new resource created.
        void _notifyResourceCreated     (GLES2ManagedResource* pResource);

        // Called when resource is about to be destroyed.
        void _notifyResourceDestroyed   (GLES2ManagedResource* pResource);
                
    // Attributes.
    protected:      
        ResourceContainer           mResources;
    };

#endif

}

#endif
