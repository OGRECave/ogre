/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#ifndef _ShaderProgramManager_
#define _ShaderProgramManager_

#include "OgrePrerequisites.h"
#include "OgreShaderProgram.h"
#include "OgreShaderProgramWriter.h"

namespace Ogre {	
class Pass;

namespace CRTShader {
class RenderState;
class ProgramSet;

/************************************************************************/
/*                                                                      */
/************************************************************************/
class ProgramManager : public Singleton<ProgramManager>
{
// Interface.
public:
	ProgramManager	();
	~ProgramManager	();


	static ProgramManager&			getSingleton	();	
	static ProgramManager*			getSingletonPtr	();

	void							acquireGpuPrograms		(Pass* pass, RenderState* renderState);
	ProgramSet*						getProgramSet			(RenderState* renderState);
	
protected:

	//-----------------------------------------------------------------------------
	typedef std::map<uint32, ProgramSet*>				ProgramSetMap;
	typedef ProgramSetMap::iterator						ProgramSetIterator;
	typedef ProgramSetMap::const_iterator				ProgramSetConstIterator;

	//-----------------------------------------------------------------------------
	typedef std::map<String, Program*>					NameToProgramMap;
	typedef NameToProgramMap::iterator					NameToProgramIterator;
	typedef std::map<String, ProgramWriter*>			NameToProgramWriterMap;
	typedef NameToProgramWriterMap::iterator			NameToProgramWriterIterator;

	
protected:
	void			destroyProgramSets		();
	void			destroyPrograms			();	
	void			destroyProgramsWriters	();

	Program*		createProgram			(const String& name, const String& desc, GpuProgramType type);
	Program*		getProgram				(const String& name);
	bool			destroyProgram			(const String& name);

	bool			createGpuPrograms		(ProgramSet* programSet);

	GpuProgramPtr	createGpuProgram		(Program* shaderProgram, 
		const String& language,
		const String& profiles,
		const String& cachePath);
	


protected:
	NameToProgramMap			mNameToProgramMap;				// Map between a name and shader program.					
	NameToProgramWriterMap		mNameToProgramWritersMap;		// Map between a name and shader program writer.					
	ProgramSetMap				mHashToProgramSetMap;			// Map between hash code of render state to program set.


private:
	friend class ProgramSet;
	friend class RenderState;
};

}
}

#endif

