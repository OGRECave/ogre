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

#include "OgreGLES2GpuProgram.h"
#include "OgreLogManager.h"

using namespace Ogre;

GLenum getGLShaderType(GpuProgramType programType);

GLenum getGLShaderType(GpuProgramType programType)
{
    switch (programType)
    {
        case GPT_VERTEX_PROGRAM:
        default:
            return GL_VERTEX_SHADER;
        case GPT_FRAGMENT_PROGRAM:
            return GL_FRAGMENT_SHADER;
    }
}

GLES2GpuProgram::GLES2GpuProgram(ResourceManager* creator, const String& name, 
    ResourceHandle handle, const String& group, bool isManual, 
    ManualResourceLoader* loader) 
    : GpuProgram(creator, name, handle, group, isManual, loader)
{
    if (createParamDictionary("GLES2GpuProgram"))
    {
        setupBaseParamDictionary();
    }
}

GLES2GpuProgram::~GLES2GpuProgram()
{
    // have to call this here reather than in Resource destructor
    // since calling virtual methods in base destructors causes crash
    unload(); 
}

//-----------------------------------------------------------------------------
size_t GLES2GpuProgram::calculateSize(void) const
{
    size_t memSize = 0;

    // Delegate Names
    memSize += sizeof(GLuint);
    memSize += sizeof(GLenum);
    memSize += GpuProgram::calculateSize();

    return memSize;
}


    