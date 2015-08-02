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

#include "OgreShaderProgramSet.h"
#include "OgreShaderProgramManager.h"

namespace Ogre {
namespace RTShader {

        //-----------------------------------------------------------------------------
        ProgramSet::ProgramSet() :
              mGpuPrograms(GPT_COUNT)
            , mCpuPrograms(GPT_COUNT, NULL)
        {

        }

        //-----------------------------------------------------------------------------
        ProgramSet::~ProgramSet()
        {
            for (int i = GPT_FIRST; i < GPT_COUNT; i++)
            {
                //Remove CPU programs
                if (mCpuPrograms[i] != NULL)
                {
                    ProgramManager::getSingleton().destroyCpuProgram(mCpuPrograms[i]);
                    mCpuPrograms[i] = NULL;
                }

                //remove GPU program
                mGpuPrograms[i].setNull();
            }

        }
        ////-----------------------------------------------------------------------------
        GpuProgramPtr ProgramSet::getGpuProgram(GpuProgramType programType)
        {
            return mGpuPrograms[programType];
        }
        ////-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
        Program* ProgramSet::getCpuProgram(GpuProgramType programType)
        {
            return mCpuPrograms[programType];
        }
        ////-----------------------------------------------------------------------------       
        Program* ProgramSet::getCpuFragmentProgram()
        {
            return getCpuProgram(GPT_FRAGMENT_PROGRAM);
        }
        ////-----------------------------------------------------------------------------
        Program* ProgramSet::getCpuVertexProgram()
        {
            return getCpuProgram(GPT_VERTEX_PROGRAM);
        }
        ////-----------------------------------------------------------------------------
        void ProgramSet::setGpuFragmentProgram(GpuProgramPtr psGpuProgram)
        {
            setGpuProgram(GPT_FRAGMENT_PROGRAM, psGpuProgram);
        }
        ////-----------------------------------------------------------------------------
        void ProgramSet::setCpuProgram(GpuProgramType programType, Program* cpuProgram)
        {
            mCpuPrograms[programType] = cpuProgram;

            
        }

        ////-----------------------------------------------------------------------------
        void ProgramSet::setGpuProgram(GpuProgramType programType, GpuProgramPtr gpuProgram)
        {
            mGpuPrograms[programType] = gpuProgram;
        }

//-----------------------------------------------------------------------------


}
}

