/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2009 Torus Knot Software Ltd
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
#ifndef _ShaderProgramManager_
#define _ShaderProgramManager_

#include "OgreShaderPrerequisites.h"
#include "OgreShaderProgram.h"
#include "OgreShaderProgramWriter.h"

namespace Ogre {	
namespace RTShader {

/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** A singleton manager class that manages shader based programs.
*/
class OGRE_RTSHADERSYSTEM_API ProgramManager : public Singleton<ProgramManager>
{
// Interface.
public:

	/** Class default constructor */
	ProgramManager	();

	/** Class destructor */
	~ProgramManager	();


	/** Override standard Singleton retrieval.
	@remarks
	Why do we do this? Well, it's because the Singleton
	implementation is in a .h file, which means it gets compiled
	into anybody who includes it. This is needed for the
	Singleton template to work, but we actually only want it
	compiled into the implementation of the class based on the
	Singleton, not all of them. If we don't change this, we get
	link errors when trying to use the Singleton-based class from
	an outside dll.
	@par
	This method just delegates to the template version anyway,
	but the implementation stays in this single compilation unit,
	preventing link errors.
	*/
	static ProgramManager&			getSingleton	();	

	/** Override standard Singleton retrieval.
	@remarks
	Why do we do this? Well, it's because the Singleton
	implementation is in a .h file, which means it gets compiled
	into anybody who includes it. This is needed for the
	Singleton template to work, but we actually only want it
	compiled into the implementation of the class based on the
	Singleton, not all of them. If we don't change this, we get
	link errors when trying to use the Singleton-based class from
	an outside dll.
	@par
	This method just delegates to the template version anyway,
	but the implementation stays in this single compilation unit,
	preventing link errors.
	*/
	static ProgramManager*			getSingletonPtr				();

	/** Acquire GPU programs associated with the given render state and bind them to the pass.
	@param pass The pass to bind the program to.
	@param renderState The render state that describes to program that need to be generated.
	*/
	void							acquireGpuPrograms			(Pass* pass, RenderState* renderState);
	
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

	/** Destroy all program sets. */
	void			destroyProgramSets		();

	/** Destroy all programs. */
	void			destroyPrograms			();	

	/** Destroy all program writers. */
	void			destroyProgramsWriters	();

	/** Create CPU program . 
	@param name The name of the program to create.
	@param desc The description of the program to create.
	@param type The type of the program to create.
	*/
	Program*		createCpuProgram		(const String& name, const String& desc, GpuProgramType type);

	/** Get a CPU program by name. Return NULL if no matching program found.
	@param name The name of the program to get.
	*/
	Program*		getCpuProgram			(const String& name);

	/** Destroy a CPU program by name.
	@param name The name of the program to destroy.
	*/
	bool			destroyCpuProgram		(const String& name);

	/** Create GPU programs for the given program set based on the CPU programs it contains.
	@param programSet The program set container.
	*/
	bool			createGpuPrograms		(ProgramSet* programSet);

	/** Create GPU program based on the give CPU program.
	@param shaderProgram The CPU program instance.
	@param language The target shader language.
	@param profiles The profiles string for program compilation.
	@param cachePath The output path to write the program into.
	*/
	GpuProgramPtr	createGpuProgram		(Program* shaderProgram, 
		const String& language,
		const String& profiles,
		const String& cachePath);

	/** Destroy a GPU program by name.
	@param name The name of the program to destroy.
	*/
	void			destroyGpuProgram		(const String& name);
	
	/** Return the current number of program set. */
	size_t			getProgramSetCount		() const { return mHashToProgramSetMap.size(); }

protected:
	NameToProgramMap			mNameToProgramMap;				// Map between a name and shader program.					
	NameToProgramWriterMap		mNameToProgramWritersMap;		// Map between a name and shader program writer.					
	ProgramSetMap				mHashToProgramSetMap;			// Map between hash code of render state to program set.


private:
	friend class ProgramSet;
	friend class RenderState;
	friend class ShaderGenerator;
};

/** @} */
/** @} */

}
}

#endif

