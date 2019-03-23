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

#include "OgreShaderPrecompiledHeaders.h"

namespace Ogre {
namespace RTShader {

//-----------------------------------------------------------------------------
ProgramSet::ProgramSet() {}

//-----------------------------------------------------------------------------
ProgramSet::~ProgramSet() {}

//-----------------------------------------------------------------------------
void ProgramSet::setCpuProgram(std::unique_ptr<Program>&& program)
{
    switch(program->getType())
    {
    case GPT_VERTEX_PROGRAM:
        mVSCpuProgram = std::move(program);
        break;
    case GPT_FRAGMENT_PROGRAM:
        mPSCpuProgram = std::move(program);
        break;
    default:
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "", "");
        break;
    }
}

//-----------------------------------------------------------------------------
Program* ProgramSet::getCpuProgram(GpuProgramType type) const
{
    switch(type)
    {
    case GPT_VERTEX_PROGRAM:
        return mVSCpuProgram.get();
    case GPT_FRAGMENT_PROGRAM:
        return mPSCpuProgram.get();
    default:
        return NULL;
    }
}
//-----------------------------------------------------------------------------
void ProgramSet::setGpuProgram(const GpuProgramPtr& program)
{
    switch(program->getType())
    {
    case GPT_VERTEX_PROGRAM:
        mVSGpuProgram = program;
        break;
    case GPT_FRAGMENT_PROGRAM:
        mPSGpuProgram = program;
        break;
    default:
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "", "");
        break;
    }
}

//-----------------------------------------------------------------------------
const GpuProgramPtr& ProgramSet::getGpuProgram(GpuProgramType type) const
{
    switch(type)
    {
    case GPT_VERTEX_PROGRAM:
        return mVSGpuProgram;
        break;
    case GPT_FRAGMENT_PROGRAM:
        return mPSGpuProgram;
        break;
    default:
        break;
    }

    static GpuProgramPtr nullPtr;
    return nullPtr;
}

}
}
