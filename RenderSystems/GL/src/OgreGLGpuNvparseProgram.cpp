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

#include "OgreGLGpuNvparseProgram.h"
#include "OgreException.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreLogManager.h"
#include "nvparse.h"

namespace Ogre {

GLGpuNvparseProgram::GLGpuNvparseProgram(ResourceManager* creator, 
        const String& name, ResourceHandle handle, 
        const String& group, bool isManual, ManualResourceLoader* loader) 
        : GLGpuProgram(creator, name, handle, group, isManual, loader)
{
    mProgramID = glGenLists(1);
}

GLGpuNvparseProgram::~GLGpuNvparseProgram()
{
    // have to call this here reather than in Resource destructor
    // since calling virtual methods in base destructors causes crash
    unload(); 
}

void GLGpuNvparseProgram::bindProgram(void)
{
     glCallList(mProgramID);
     glEnable(GL_TEXTURE_SHADER_NV);
     glEnable(GL_REGISTER_COMBINERS_NV);
     glEnable(GL_PER_STAGE_CONSTANTS_NV);
}

void GLGpuNvparseProgram::unbindProgram(void)
{

    glDisable(GL_TEXTURE_SHADER_NV);
    glDisable(GL_REGISTER_COMBINERS_NV);
    glDisable(GL_PER_STAGE_CONSTANTS_NV);
}

void GLGpuNvparseProgram::bindProgramParameters(GpuProgramParametersSharedPtr params, uint16 mask)
{
    // NB, register combiners uses 2 constants per texture stage (0 and 1)
    // We have stored these as (stage * 2) + const_index in the physical buffer
    // There are no other parameters in a register combiners shader
    const float* floatList = params->getFloatPointer(0);
    for (size_t i = 0; i < params->getConstantList().size()/4; ++i)
    {
        GLenum combinerStage = GL_COMBINER0_NV + (unsigned int)(i / 2);
        GLenum pname = GL_CONSTANT_COLOR0_NV + (i % 2);
        glCombinerStageParameterfvNV(combinerStage, pname, floatList + i);
    }
}
void GLGpuNvparseProgram::unloadImpl(void)
{
    glDeleteLists(mProgramID,1);
}

void GLGpuNvparseProgram::loadFromSource(void)
{
    glNewList(mProgramID, GL_COMPILE);

    String::size_type pos = mSource.find("!!");

    while (pos != String::npos) {
        String::size_type newPos = mSource.find("!!", pos + 1);

        String script = mSource.substr(pos, newPos - pos);
        nvparse(script.c_str(), 0);

        for (char* const * errors= nvparse_get_errors(); *errors; errors++)
        {
            LogManager::getSingleton().logMessage("Warning: nvparse reported the following errors:");
            LogManager::getSingleton().logMessage("\t" + String(*errors));
        }
        
        pos = newPos;
    }

    glEndList();
}

}