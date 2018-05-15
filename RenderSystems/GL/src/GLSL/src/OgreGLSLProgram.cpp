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
#include "OgreGpuProgram.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreStringConverter.h"
#include "OgreGpuProgramManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreLogManager.h"

#include "OgreGLSLProgram.h"
#include "OgreGLSLGpuProgram.h"
#include "OgreGLSLExtSupport.h"
#include "OgreGLSLLinkProgramManager.h"
#include "OgreGLSLPreprocessor.h"

namespace Ogre {
    namespace GLSL {
    //---------------------------------------------------------------------------
    GLSLProgram::~GLSLProgram()
    {
        // Have to call this here rather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        if (isLoaded())
        {
            unload();
        }
        else
        {
            unloadHighLevel();
        }
    }
    //---------------------------------------------------------------------------
    bool GLSLProgram::compile(bool checkErrors)
    {
        // only create a shader object if glsl is supported
        if (isSupported())
        {
            // create shader object
            GLenum shaderType = 0x0000;
            switch (mType)
            {
            case GPT_VERTEX_PROGRAM:
                shaderType = GL_VERTEX_SHADER_ARB;
                break;
            case GPT_FRAGMENT_PROGRAM:
                shaderType = GL_FRAGMENT_SHADER_ARB;
                break;
            case GPT_GEOMETRY_PROGRAM:
                shaderType = GL_GEOMETRY_SHADER_EXT;
                break;
            case GPT_COMPUTE_PROGRAM:
            case GPT_DOMAIN_PROGRAM:
            case GPT_HULL_PROGRAM:
                break;
            }
            mGLHandle = glCreateShaderObjectARB(shaderType);
        }

        // Add preprocessor extras and main source
        if (!mSource.empty())
        {
            const char *source = mSource.c_str();
            glShaderSourceARB(mGLHandle, 1, &source, NULL);
        }

        glCompileShaderARB(mGLHandle);
        // check for compile errors
        int compiled = 0;
        glGetObjectParameterivARB(mGLHandle, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

        if(!checkErrors)
            return compiled == 1;

        String compileInfo = getObjectInfo(mGLHandle);

        if (!compiled)
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, getResourceLogName() + " " + compileInfo, "compile");

        // probably we have warnings
        if (!compileInfo.empty())
            LogManager::getSingleton().stream(LML_WARNING) << getResourceLogName() << " " << compileInfo;

        return (compiled == 1);
    }

    //-----------------------------------------------------------------------
    void GLSLProgram::createLowLevelImpl(void)
    {
        mAssemblerProgram = GpuProgramPtr(OGRE_NEW GLSLGpuProgram( this ));
        // Shader params need to be forwarded to low level implementation
        mAssemblerProgram->setAdjacencyInfoRequired(isAdjacencyInfoRequired());
    }
    //-----------------------------------------------------------------------
    void GLSLProgram::unloadHighLevelImpl(void)
    {
        if (isSupported())
        {
            glDeleteObjectARB(mGLHandle);           
            mGLHandle = 0;

            // destroy all programs using this shader
            GLSLLinkProgramManager::getSingletonPtr()->destroyAllByShader(this);
        }
    }

    //-----------------------------------------------------------------------
    void GLSLProgram::populateParameterNames(GpuProgramParametersSharedPtr params)
    {
        getConstantDefinitions();
        params->_setNamedConstants(mConstantDefs);
        // Don't set logical / physical maps here, as we can't access parameters by logical index in GLHL.
    }
    //-----------------------------------------------------------------------
    void GLSLProgram::buildConstantDefinitions() const
    {
        // We need an accurate list of all the uniforms in the shader, but we
        // can't get at them until we link all the shaders into a program object.


        // Therefore instead, parse the source code manually and extract the uniforms
        createParameterMappingStructures(true);
        GLSLLinkProgramManager::getSingleton().extractUniformsFromGLSL(
            mSource, *mConstantDefs, mName);

        // Also parse any attached sources
        for (GLSLProgramContainer::const_iterator i = mAttachedGLSLPrograms.begin();
            i != mAttachedGLSLPrograms.end(); ++i)
        {
            GLSLShaderCommon* childShader = *i;

            GLSLLinkProgramManager::getSingleton().extractUniformsFromGLSL(
                childShader->getSource(), *mConstantDefs, childShader->getName());

        }
    }

    //-----------------------------------------------------------------------
    GLSLProgram::GLSLProgram(ResourceManager* creator, 
        const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader)
        : GLSLShaderCommon(creator, name, handle, group, isManual, loader)
        , mGLHandle(0)
    {
        // add parameter command "attach" to the material serializer dictionary
        if (createParamDictionary("GLSLProgram"))
        {
            setupBaseParamDictionary();
            ParamDictionary* dict = getParamDictionary();

            dict->addParameter(ParameterDef("preprocessor_defines", 
                "Preprocessor defines use to compile the program.",
                PT_STRING),&msCmdPreprocessorDefines);
            dict->addParameter(ParameterDef("attach", 
                "name of another GLSL program needed by this program",
                PT_STRING),&msCmdAttach);
            dict->addParameter(ParameterDef("column_major_matrices", 
                "Whether matrix packing in column-major order.",
                PT_BOOL),&msCmdColumnMajorMatrices);
            dict->addParameter(
                ParameterDef("input_operation_type",
                "The input operation type for this geometry program. \
                Can be 'point_list', 'line_list', 'line_strip', 'triangle_list', \
                'triangle_strip' or 'triangle_fan'", PT_STRING),
                &msInputOperationTypeCmd);
            dict->addParameter(
                ParameterDef("output_operation_type",
                "The input operation type for this geometry program. \
                Can be 'point_list', 'line_strip' or 'triangle_strip'",
                 PT_STRING),
                 &msOutputOperationTypeCmd);
            dict->addParameter(
                ParameterDef("max_output_vertices", 
                "The maximum number of vertices a single run of this geometry program can output",
                PT_INT),&msMaxOutputVerticesCmd);
        }
        // Manually assign language now since we use it immediately
        mSyntaxCode = "glsl";
    }

    //-----------------------------------------------------------------------
    void GLSLProgram::attachToProgramObject( const GLhandleARB programObject )
    {
        // attach child objects
        for (auto childShader : mAttachedGLSLPrograms)
        {
            childShader->attachToProgramObject(programObject);
        }
        glAttachObjectARB( programObject, mGLHandle );
        GLenum glErr = glGetError();
        if(glErr != GL_NO_ERROR)
        {
            reportGLSLError( glErr, "GLSLProgram::attachToProgramObject",
                "Error attaching " + mName + " shader object to GLSL Program Object", programObject );
        }

    }
    //-----------------------------------------------------------------------
    void GLSLProgram::detachFromProgramObject( const GLhandleARB programObject )
    {
        glDetachObjectARB(programObject, mGLHandle);

        GLenum glErr = glGetError();
        if(glErr != GL_NO_ERROR)
        {
            reportGLSLError( glErr, "GLSLProgram::detachFromProgramObject",
                "Error detaching " + mName + " shader object from GLSL Program Object", programObject );
        }
        // attach child objects
        GLSLProgramContainerIterator childprogramcurrent = mAttachedGLSLPrograms.begin();
        GLSLProgramContainerIterator childprogramend = mAttachedGLSLPrograms.end();

        while (childprogramcurrent != childprogramend)
        {
            GLSLShaderCommon* childShader = *childprogramcurrent;
            childShader->detachFromProgramObject( programObject );
            ++childprogramcurrent;
        }

    }

    //-----------------------------------------------------------------------
    const String& GLSLProgram::getLanguage(void) const
    {
        static const String language = "glsl";

        return language;
    }

}
}
