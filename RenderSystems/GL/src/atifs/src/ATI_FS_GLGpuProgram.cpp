/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#include "ps_1_4.h"
#include "OgreException.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreLogManager.h"
#include "ATI_FS_GLGpuProgram.h"

using namespace Ogre;


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
	const GpuLogicalBufferStruct* floatStruct = params->getFloatLogicalBufferStruct();

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


void ATI_FS_GLGpuProgram::bindProgramPassIterationParameters(GpuProgramParametersSharedPtr params)
{
	if (params->hasPassIterationNumber())
	{
		size_t physicalIndex = params->getPassIterationNumberIndex();
		size_t logicalIndex = params->getFloatLogicalIndexForPhysicalIndex(physicalIndex);
		const float* pFloat = params->getFloatPointer(physicalIndex);
		glSetFragmentShaderConstantATI( GL_CON_0_ATI + (GLuint)logicalIndex, pFloat);
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
			// compile was successfull so send the machine instructions thru GL to GPU
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
		// an error occured when compiling the ps_1_4 source code
		char buff[50];
        sprintf(buff,"error on line %d in pixel shader source\n", PS1_4Assembler.mCurrentLine);

		LogManager::getSingleton().logMessage("Warning: atifs compiler reported the following errors:");
		LogManager::getSingleton().logMessage(buff + mName);

		OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
			"Cannot Compile ATI fragment shader : " + mName + "\n\n" + buff , mName);// + 
    }


}

