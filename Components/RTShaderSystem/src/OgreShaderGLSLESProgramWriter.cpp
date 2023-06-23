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

        String GLSLESProgramWriter::TargetLanguage =  "glsles";

        //-----------------------------------------------------------------------
        GLSLESProgramWriter::GLSLESProgramWriter()
        {
            mIsGLSLES = true;
            auto* rs = Root::getSingleton().getRenderSystem();
            mGLSLVersion = rs ? rs->getNativeShadingLanguageVersion() : 100;
            initializeStringMaps();
        }

        //-----------------------------------------------------------------------
        GLSLESProgramWriter::~GLSLESProgramWriter()
        {
        }
        //-----------------------------------------------------------------------
        void GLSLESProgramWriter::writeSourceCode(
            std::ostream& os,
            Program* program)
        {
            // Write the current version (this forces the driver to fulfill the glsl es standard)
            os << "#version "<< mGLSLVersion;

            // Starting with ES 3.0 the version must contain the string "es" after the version number with a space separating them
            if(mGLSLVersion > 100)
                os << " es";

            os << std::endl;

            for(const auto& p : program->getParameters())
            {
                if(p->getType() != GCT_SAMPLER_EXTERNAL_OES)
                    continue;
                if(mGLSLVersion > 100)
                    os << "#extension GL_OES_EGL_image_external_essl3 : require\n";
                else
                    os << "#extension GL_OES_EGL_image_external : require\n";

                break;
            }

            // Generate source code header.
            writeProgramTitle(os, program);
            os<< std::endl;

            // Embed dependencies.
            writeProgramDependencies(os, program);
            os << std::endl;
            writeMainSourceCode(os, program);
        }

    }
}
