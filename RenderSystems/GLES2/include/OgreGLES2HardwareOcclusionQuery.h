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

#ifndef __GLES2HARDWAREOCCLUSIONQUERY_H__
#define __GLES2HARDWAREOCCLUSIONQUERY_H__

#include "OgreGLES2Prerequisites.h"
#include "OgreHardwareOcclusionQuery.h"
#include "OgreGLES2ManagedResource.h"

namespace Ogre { 


// If you use multiple rendering passes you can test only the first pass and all other passes don't have to be rendered 
// if the first pass result has too few pixels visible.

// Be sure to render all occluder first and whats out so the RenderQue don't switch places on 
// the occluding objects and the tested objects because it thinks it's more effective..

class _OgreGLES2Export GLES2HardwareOcclusionQuery : public HardwareOcclusionQuery MANAGED_RESOURCE
{
//----------------------------------------------------------------------
// Public methods
//--
public:
    /**
      * Default object constructor
      * 
      */
    GLES2HardwareOcclusionQuery();
    /**
      * Object destructor
      */
    ~GLES2HardwareOcclusionQuery();

    //------------------------------------------------------------------
    // Occlusion query functions (see base class documentation for this)
    //--
    void beginOcclusionQuery();
    void endOcclusionQuery();
    bool pullOcclusionQuery( unsigned int* NumOfFragments); 
    bool isStillOutstanding(void);


//----------------------------------------------------------------------
// private members
//--
private:

    GLuint mQueryID;
    
    void createQuery();
    
    void destroyQuery();
    
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
protected:
    
    /** See AndroidResource. */
    virtual void notifyOnContextLost();
    
    /** See AndroidResource. */
    virtual void notifyOnContextReset();
#endif
    
};

}

#endif 

