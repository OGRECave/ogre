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

#include "OgreGLGpuNvparseProgram.h"
#include "OgreException.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreLogManager.h"
#include "nvparse.h"

using namespace Ogre;

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
	const FloatConstantList& floatList = 
		params->getFloatConstantList();
	size_t index = 0;
	for (FloatConstantList::const_iterator i = floatList.begin();
		i != floatList.end(); ++i, ++index)
	{
		GLenum combinerStage = GL_COMBINER0_NV + (unsigned int)(index / 2);
		GLenum pname = GL_CONSTANT_COLOR0_NV + (index % 2);
		glCombinerStageParameterfvNV(combinerStage, pname, &(*i));
		
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

