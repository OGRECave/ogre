/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#include "OgreShaderProgramWriter.h"
#include "OgreShaderProgram.h"

namespace Ogre {
namespace RTShader {

//-----------------------------------------------------------------------
void ProgramWriter::writeProgramTitle(std::ostream& os, Program* program)
{
    os << "//-----------------------------------------------------------------------------" << std::endl;
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
    os << std::endl;
    os << "// Language: " <<  getTargetLanguage() << std::endl;
    os << "// Created by Ogre RT Shader Generator. All rights reserved." << std::endl;
    os << "//-----------------------------------------------------------------------------" << std::endl;
}

//-----------------------------------------------------------------------
void ProgramWriter::writeUniformParametersTitle(std::ostream& os, Program* program)
{
    os << "//-----------------------------------------------------------------------------" << std::endl;
    os << "//                         GLOBAL PARAMETERS" << std::endl;
    os << "//-----------------------------------------------------------------------------" << std::endl;
}
//-----------------------------------------------------------------------
void ProgramWriter::writeFunctionTitle(std::ostream& os, Function* function)
{
    os << "//-----------------------------------------------------------------------------" << std::endl;
    os << "// Function Name: " <<  function->getName() << std::endl;
    os << "// Function Desc: " <<  function->getDescription() << std::endl;
    os << "//-----------------------------------------------------------------------------" << std::endl;
}

}
}
