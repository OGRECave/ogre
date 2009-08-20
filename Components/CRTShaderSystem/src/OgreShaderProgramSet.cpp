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

#include "OgreShaderProgramSet.h"
#include "OgreShaderProgramManager.h"


namespace Ogre {
namespace CRTShader {

//-----------------------------------------------------------------------------
ProgramSet::ProgramSet()
{
	
}

//-----------------------------------------------------------------------------
ProgramSet::~ProgramSet()
{
	if (mVSCpuProgram != NULL)
	{
		ProgramManager::getSingleton().destroyProgram(mVSCpuProgram->getName());
		mVSCpuProgram = NULL;
	}

	if (mPSCpuProgram != NULL)
	{
		ProgramManager::getSingleton().destroyProgram(mPSCpuProgram->getName());
		mPSCpuProgram = NULL;
	}
}

//-----------------------------------------------------------------------------
void ProgramSet::setCpuVertexProgram(Program* vsCpuProgram)
{
	mVSCpuProgram = vsCpuProgram;
}

//-----------------------------------------------------------------------------
Program* ProgramSet::getCpuVertexProgram()
{
	return mVSCpuProgram;
}

//-----------------------------------------------------------------------------
void ProgramSet::setCpuFragmentProgram(Program* psCpuProgram)
{
	mPSCpuProgram = psCpuProgram;
}

//-----------------------------------------------------------------------------
Program* ProgramSet::getCpuFragmentProgram()
{
	return mPSCpuProgram;
}

//-----------------------------------------------------------------------------
void ProgramSet::setGpuVertexProgram(GpuProgramPtr vsGpuProgram)
{
	mVSGpuProgram = vsGpuProgram;
}

//-----------------------------------------------------------------------------
GpuProgramPtr ProgramSet::getGpuVertexProgram()
{
	return mVSGpuProgram;
}

//-----------------------------------------------------------------------------
void ProgramSet::setGpuFragmentProgram(GpuProgramPtr psGpuProgram)
{
	mPSGpuProgram = psGpuProgram;
}

//-----------------------------------------------------------------------------
GpuProgramPtr ProgramSet::getGpuFragmentProgram()
{
	return mPSGpuProgram;
}

}
}
