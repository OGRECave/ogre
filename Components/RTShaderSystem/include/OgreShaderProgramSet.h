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
#ifndef _ShaderProgramSet_
#define _ShaderProgramSet_

#include "OgreShaderPrerequisites.h"
#include "OgreGpuProgram.h"

namespace Ogre {
namespace RTShader {

/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** Container class for shader based programs. 
Holds both a CPU representation and GPU representation of vertex and fragment program.
*/
class _OgreRTSSExport ProgramSet : public RTShaderSystemAlloc
{

    // Interface.
public:
    /** Class default constructor */
    ProgramSet();

    /** Class destructor */
    ~ProgramSet();

    /** Return a GPU shader program. 
    @param programType the gpu program type to get
    */
    GpuProgramPtr getGpuProgram(GpuProgramType programType);

    /** Return a shader CPU progrm. 
    @param programType The type of the program to get
    */
    Program* getCpuProgram(GpuProgramType programType);

  /** Get the vertex shader CPU program. */
    OGRE_DEPRECATED_EX("ProgramSet::getCpuVertexProgram is deprecated, Use ProgramSet::getCpuProgram(GpuProgramType) instead")
    Program* getCpuVertexProgram();

    /** Get the fragment shader CPU program. */
    OGRE_DEPRECATED_EX("ProgramSet::getCpuFragmentProgram is deprecated, Use ProgramSet::getCpuProgram(GpuProgramType) instead")
    Program* getCpuFragmentProgram();




    // Protected methods.
protected:

    /** Set a CPU program 
    @param programType the program type to set
    @param cpuProgram the program to set
    */

    void setCpuProgram(GpuProgramType programType, Program* cpuProgram);
    /** Set a GPU program 
    @param programType the program type to set
    @param gpuProgram the program to set
    */
    void setGpuProgram(GpuProgramType programType, GpuProgramPtr gpuProgram);
    void setGpuFragmentProgram(GpuProgramPtr psGpuProgram);


    // Attributes.
protected:
    //Shader's CPU program.
    vector<Program *>::type mCpuPrograms;
    
    //Shader's GPU program.
    vector<GpuProgramPtr>::type mGpuPrograms;


private:
    friend class ProgramManager;
    friend class TargetRenderState;

};

/** @} */
/** @} */

}
}

#endif

