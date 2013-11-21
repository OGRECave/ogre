/*
 -----------------------------------------------------------------------------
 This source file is part of OGRE
 (Object-oriented Graphics Rendering Engine)
 For the latest info, see http://www.ogre3d.org/
 
 Copyright (c) 2000-2012 Torus Knot Software Ltd
 
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

#include "OgreStableHeaders.h"
#include "OgreCommon.h"
#include "OgreGLES2UniformCacheImp.h"

namespace Ogre {
    
    GLES2UniformCacheImp::GLES2UniformCacheImp(void)
    {
        clearCache();
    }
    
    void GLES2UniformCacheImp::clearCache()
    {
        mUniformValueMap.clear();
    }
    
    GLES2UniformCacheImp::~GLES2UniformCacheImp(void)
    {
        mUniformValueMap.clear();
    }
    
    bool GLES2UniformCacheImp::updateUniform(GLint location, const void *value, GLsizei length)
    {
        uint32 current = mUniformValueMap[location];
        uint32 hash = Ogre::FastHash(static_cast<const char*>(value), length);
        // First check if the uniform name is in the map. If not, this is new so insert it into the map.
        if (!current || (current != hash))
        {
            // Haven't cached this state yet or the value has changed
            mUniformValueMap[location] = hash;
			return true;
        }

        return false;
    }
}
