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

/** \addtogroup Optional
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

    /** Get the shader CPU program. */
    Program* getCpuProgram(GpuProgramType type) const;

    /** Get the shader GPU program. */
    const GpuProgramPtr& getGpuProgram(GpuProgramType type) const;

    // Protected methods.
private:
    void setCpuProgram(std::unique_ptr<Program>&& program);
    void setGpuProgram(const GpuProgramPtr& program);

    // Vertex shader CPU program.
    std::unique_ptr<Program> mVSCpuProgram;
    // Fragment shader CPU program.
    std::unique_ptr<Program> mPSCpuProgram;
    // Vertex shader GPU program.
    GpuProgramPtr mVSGpuProgram;
    // Fragment shader CPU program.
    GpuProgramPtr mPSGpuProgram;

    friend class ProgramManager;
    friend class TargetRenderState;

};

/** @} */
/** @} */

}
}

#endif

