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

#include "OgreGLGpuProgram.h"
#include "OgreException.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreGLSLProgramCommon.h"

namespace Ogre {

GLenum GLArbGpuProgram::getProgramType() const
{
    switch (mType)
    {
        case GPT_VERTEX_PROGRAM:
        default:
            return GL_VERTEX_PROGRAM_ARB;
        case GPT_GEOMETRY_PROGRAM:
            return GL_GEOMETRY_PROGRAM_NV;
        case GPT_FRAGMENT_PROGRAM:
            return GL_FRAGMENT_PROGRAM_ARB;
    }
}

GLGpuProgram::GLGpuProgram(ResourceManager* creator, const String& name,
    ResourceHandle handle, const String& group, bool isManual, 
    ManualResourceLoader* loader) 
    : GpuProgram(creator, name, handle, group, isManual, loader)
{
    if (createParamDictionary("GLGpuProgram"))
    {
        setupBaseParamDictionary();
    }
}

GLGpuProgram::~GLGpuProgram()
{
    // have to call this here reather than in Resource destructor
    // since calling virtual methods in base destructors causes crash
    unload(); 
}

bool GLGpuProgramBase::isAttributeValid(VertexElementSemantic semantic, uint index)
{
    // default implementation
    switch(semantic)
    {
        case VES_POSITION:
        case VES_NORMAL:
        case VES_DIFFUSE:
        case VES_SPECULAR:
        case VES_TEXTURE_COORDINATES:
            return false;
        case VES_BLEND_WEIGHTS:
        case VES_BLEND_INDICES:
        case VES_BINORMAL:
        case VES_TANGENT:
            return true; // with default binding
    }

    return false;
}

GLArbGpuProgram::GLArbGpuProgram(ResourceManager* creator, const String& name, 
    ResourceHandle handle, const String& group, bool isManual, 
    ManualResourceLoader* loader) 
    : GLGpuProgram(creator, name, handle, group, isManual, loader)
{
    glGenProgramsARB(1, &mProgramID);
}

GLArbGpuProgram::~GLArbGpuProgram()
{
    // have to call this here reather than in Resource destructor
    // since calling virtual methods in base destructors causes crash
    unload(); 
}

void GLArbGpuProgram::bindProgram(void)
{
    glEnable(getProgramType());
    glBindProgramARB(getProgramType(), mProgramID);
}

void GLArbGpuProgram::unbindProgram(void)
{
    glBindProgramARB(getProgramType(), 0);
    glDisable(getProgramType());
}

void GLArbGpuProgram::bindProgramParameters(GpuProgramParametersSharedPtr params, uint16 mask)
{
    GLenum type = getProgramType();
    
    // only supports float constants
    GpuLogicalBufferStructPtr floatStruct = params->getFloatLogicalBufferStruct();

    for (GpuLogicalIndexUseMap::const_iterator i = floatStruct->map.begin();
        i != floatStruct->map.end(); ++i)
    {
        if (i->second.variability & mask)
        {
            GLuint logicalIndex = static_cast<GLuint>(i->first);
            const float* pFloat = params->getFloatPointer(i->second.physicalIndex);
            // Iterate over the params, set in 4-float chunks (low-level)
            for (size_t j = 0; j < i->second.currentSize; j+=4)
            {
                glProgramLocalParameter4fvARB(type, logicalIndex, pFloat);
                pFloat += 4;
                ++logicalIndex;
            }
        }
    }
}

void GLArbGpuProgram::unloadImpl(void)
{
    glDeleteProgramsARB(1, &mProgramID);
}

void GLArbGpuProgram::loadFromSource(void)
{
    if (GL_INVALID_OPERATION == glGetError()) {
        LogManager::getSingleton().logMessage("Invalid Operation before loading program "+mName, LML_CRITICAL);

    }
    glBindProgramARB(getProgramType(), mProgramID);
    glProgramStringARB(getProgramType(), GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei)mSource.length(), mSource.c_str());

    if (GL_INVALID_OPERATION == glGetError())
    {
        GLint errPos;
        glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errPos);
        const char* errStr = (const char*)glGetString(GL_PROGRAM_ERROR_STRING_ARB);
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "'" + mName + "' " + errStr);
    }
    glBindProgramARB(getProgramType(), 0);
}

    
}
