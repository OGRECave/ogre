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
class ProgramManager : public Singleton<ProgramManager>, public RTShaderSystemAlloc
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

	/** Acquire CPU/GPU programs set associated with the given render state and bind them to the pass.
	@param pass The pass to bind the programs to.
	@param renderState The render state that describes the program that need to be generated.
	*/
	void							acquirePrograms			(Pass* pass, RenderState* renderState);

	/** Release CPU/GPU programs set associated with the given render state.	
	@param renderState The render state that describes the programs set.
	*/
	void							releasePrograms			(RenderState* renderState);
	
protected:

	//-----------------------------------------------------------------------------
	typedef map<uint32, ProgramSet*>::type				ProgramSetMap;
	typedef ProgramSetMap::iterator						ProgramSetIterator;
	typedef ProgramSetMap::const_iterator				ProgramSetConstIterator;

	//-----------------------------------------------------------------------------
	typedef vector<Program*>::type						ProgramList;
	typedef ProgramList::iterator						ProgramListIterator;
	typedef map<String, ProgramWriter*>::type			ProgramWriterMap;
	typedef ProgramWriterMap::iterator					ProgramWriterIterator;

	//-----------------------------------------------------------------------------
	typedef map<String, ProgramProcessor*>::type 		ProgramProcessorMap;
	typedef ProgramProcessorMap::iterator 				ProgramProcessorIterator;
	typedef ProgramProcessorMap::const_iterator			ProgramProcessorConstIterator;
	typedef vector<ProgramProcessor*>::type 			ProgramProcessorList;

	
protected:
	/** Create default program processors. */
	void			createDefaultProgramProcessors	();
	
	/** Destroy default program processors. */
	void			destroyDefaultProgramProcessors	();

	/** Destroy all program sets. */
	void			destroyProgramSets		();

	/** Destroy all program writers. */
	void			destroyProgramWriters	();

	/** Create CPU program . 	
	@param type The type of the program to create.
	*/
	Program*		createCpuProgram		(GpuProgramType type);

	/** Destroy a CPU program by name.
	@param shaderProgram The CPU program instance to destroy.
	*/
	bool			destroyCpuProgram		(Program* shaderProgram);

	/** Create GPU programs for the given program set based on the CPU programs it contains.
	@param programSet The program set container.
	*/
	bool			createGpuPrograms		(ProgramSet* programSet);

	/** Create GPU program based on the give CPU program.
	@param shaderProgram The CPU program instance.
	@param programWriter The program writer instance.
	@param language The target shader language.
	@param profiles The profiles string for program compilation.
	@param cachePath The output path to write the program into.
	*/
	GpuProgramPtr	createGpuProgram		(Program* shaderProgram, 
		ProgramWriter* programWriter,
		const String& language,
		const String& profiles,
		const String& cachePath);

	/** 
	Add program processor instance to this manager.
	@param processor The instance to add.
	*/
	void			addProgramProcessor		(ProgramProcessor* processor);

	/** 
	Remove program processor instance from this manager. 
	@param processor The instance to remove.
	*/
	void			removeProgramProcessor	(ProgramProcessor* processor);

	/** Destroy a GPU program by name.
	@param name The name of the program to destroy.
	@param type The type of the program to destroy.
	*/
	void			destroyGpuProgram		(const String& name, GpuProgramType type);
	
	/** Return the current number of program set. */
	size_t			getProgramSetCount		() const { return mHashToProgramSetMap.size(); }

	/** Return the number of created vertex shaders. */
	size_t			getVertexShaderCount		() const { return mVertexShaderCount; }

	/** Return the number of created fragment shaders. */
	size_t			getFragmentShaderCount		() const { return mFragmentShaderCount; }

protected:
	

protected:
	ProgramList					mCpuProgramsList;				// CPU programs list.					
	ProgramWriterMap			mProgramWritersMap;				// Map between target language and shader program writer.					
	ProgramProcessorMap			mProgramProcessorsMap;			// Map between target language and shader program processor.
	ProgramSetMap				mHashToProgramSetMap;			// Map between hash code of render state to program set.
	size_t						mVertexShaderCount;				// Vertex shader count.
	size_t						mFragmentShaderCount;			// Fragment shader count.
	ProgramProcessorList		mDefaultProgramProcessors;		// The default program processors.


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

