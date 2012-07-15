/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2012 Torus Knot Software Ltd
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
#ifndef _ShaderProgramWriter_
#define _ShaderProgramWriter_

#include "OgreShaderPrerequisites.h"
#include "OgreGpuProgram.h"
#include "OgreString.h"
#include "OgreShaderProgram.h"
#include "OgreShaderParameter.h"
#include "OgreShaderFunction.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
#include "OgreStringSerialiser.h"
#endif

namespace Ogre {
namespace RTShader {

/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** Base class interface for shader program writers.
The main usage of this class is to generate a shader source code from the given CPU program.
In order to support specific shader language one should subclass this interface and implement the pure methods.
*/

class _OgreRTSSExport ProgramWriter : public RTShaderSystemAlloc
{
// Interface.
public:

	/** Class destructor */
	virtual ~ProgramWriter() {}


	/** Write the program shader source code.
	@param os The output stream to write to code into.
	@param program The source CPU program for the GPU program code.
	*/
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
	virtual void writeSourceCode(StringSerialiser& os, Program* program) = 0;
#else
	virtual void writeSourceCode(std::ostream& os, Program* program) = 0;
#endif

	/** Return the target language of this writer. */
	virtual const String& getTargetLanguage() const = 0;

// Protected methods.
protected:
	/** Write the program title. */
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
	void writeProgramTitle(StringSerialiser& os, Program* program);
#else
	void writeProgramTitle(std::ostream& os, Program* program);
#endif

	/** Write the uniform parameters title. */
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
	void writeUniformParametersTitle(StringSerialiser& os, Program* program);
#else
	void writeUniformParametersTitle(std::ostream& os, Program* program);
#endif

	/** Write a function title. */
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
	void writeFunctionTitle(StringSerialiser& os, Function* function);
#else
	void writeFunctionTitle(std::ostream& os, Function* function);
#endif
};

/** @} */
/** @} */

}
}



#endif
