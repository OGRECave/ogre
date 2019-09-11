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

#include "ps_1_4.h"
#include "OgreException.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreLogManager.h"
#include "ATI_FS_GLGpuProgram.h"


namespace Ogre {

ATI_FS_GLGpuProgram::ATI_FS_GLGpuProgram(ResourceManager* creator, 
        const String& name, ResourceHandle handle, 
        const String& group, bool isManual, ManualResourceLoader* loader) :
    GLGpuProgram(creator, name, handle, group, isManual, loader)
{
    mProgramType = GL_FRAGMENT_SHADER_ATI;
    mProgramID = glGenFragmentShadersATI(1);
}

ATI_FS_GLGpuProgram::~ATI_FS_GLGpuProgram()
{
    // have to call this here reather than in Resource destructor
    // since calling virtual methods in base destructors causes crash
    unload(); 
}

void ATI_FS_GLGpuProgram::bindProgram(void)
{
    glEnable(mProgramType);
    glBindFragmentShaderATI(mProgramID);
}

void ATI_FS_GLGpuProgram::unbindProgram(void)
{
    glDisable(mProgramType);
}


void ATI_FS_GLGpuProgram::bindProgramParameters(GpuProgramParametersSharedPtr params, uint16 mask)
{
    // only supports float constants
    GpuLogicalBufferStructPtr floatStruct = params->getFloatLogicalBufferStruct();

    for (GpuLogicalIndexUseMap::const_iterator i = floatStruct->map.begin();
        i != floatStruct->map.end(); ++i)
    {
        if (i->second.variability & mask)
        {
            size_t logicalIndex = i->first;
            const float* pFloat = params->getFloatPointer(i->second.physicalIndex);
            // Iterate over the params, set in 4-float chunks (low-level)
            for (size_t j = 0; j < i->second.currentSize; j+=4)
            {
                glSetFragmentShaderConstantATI(GL_CON_0_ATI + logicalIndex, pFloat);
                pFloat += 4;
                ++logicalIndex;
            }
        }
    }

}

void ATI_FS_GLGpuProgram::unloadImpl(void)
{
    glDeleteFragmentShaderATI(mProgramID);
}


void ATI_FS_GLGpuProgram::loadFromSource(void)
{

    PS_1_4 PS1_4Assembler;
    // attempt to compile the source
#ifdef _DEBUG
    PS1_4Assembler.test(); // run compiler tests in debug mode
#endif

    bool Error = !PS1_4Assembler.compile(mSource.c_str());

    if(!Error) { 
        glBindFragmentShaderATI(mProgramID);
        glBeginFragmentShaderATI();
            // compile was successful so send the machine instructions thru GL to GPU
            Error = !PS1_4Assembler.bindAllMachineInstToFragmentShader();
        glEndFragmentShaderATI();

        // check GL for GPU machine instruction bind erros
        if (Error)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                "Cannot Bind ATI fragment shader :" + mName, mName); 
        }

    }
    else
    {
        // an error occurred when compiling the ps_1_4 source code
        char buff[50];
        sprintf(buff,"error on line %d in pixel shader source\n", PS1_4Assembler.mCurrentLine);

        LogManager::getSingleton().logMessage("Warning: atifs compiler reported the following errors:");
        LogManager::getSingleton().logMessage(buff + mName);

        OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
            "Cannot Compile ATI fragment shader : " + mName + "\n\n" + buff , mName);// + 
    }


}

}
