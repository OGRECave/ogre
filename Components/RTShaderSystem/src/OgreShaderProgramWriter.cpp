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
#include "OgreShaderProgramWriter.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
#	define ENDL "\n"
#else
#	define ENDL std::endl
#endif

namespace Ogre {
namespace RTShader {

//-----------------------------------------------------------------------
void ProgramWriter::writeProgramTitle(
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
	StringSerialiser& os,
#else
	std::ostream &os,
#endif
	Program* program)
{
	os << "//-----------------------------------------------------------------------------" << ENDL;
	os << "// Program Type: ";
	switch(program->getType())
	{
	case GPT_VERTEX_PROGRAM:
		os << "Vertex shader";
		break;
	case GPT_FRAGMENT_PROGRAM:
		os << "Fragment shader";
		break;
	case GPT_GEOMETRY_PROGRAM:
		os << "Geometry shader";
		break;	
	default:
		break;
	}
	os << ENDL;
	os << "// Language: " <<  getTargetLanguage() << ENDL;
	os << "// Created by Ogre RT Shader Generator. All rights reserved." << ENDL;
	os << "//-----------------------------------------------------------------------------" << ENDL;
}

//-----------------------------------------------------------------------
void ProgramWriter::writeUniformParametersTitle(
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
	StringSerialiser& os,
#else
	std::ostream &os,
#endif
	Program* program)
{
	os << "//-----------------------------------------------------------------------------" << ENDL;
	os << "//                         GLOBAL PARAMETERS" << ENDL;
	os << "//-----------------------------------------------------------------------------" << ENDL;
}
//-----------------------------------------------------------------------
void ProgramWriter::writeFunctionTitle(
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
	StringSerialiser& os,
#else
	std::ostream &os,
#endif 
	Function* function)
{
	os << "//-----------------------------------------------------------------------------" << ENDL;
	os << "// Function Name: " <<  function->getName() << ENDL;
	os << "// Function Desc: " <<  function->getDescription() << ENDL;
	os << "//-----------------------------------------------------------------------------" << ENDL;
}

}
}
