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
#include "OgreGLSLExtSupport.h"
#include "OgreGLSLLinkProgramManager.h"
#include "OgreGLSLPreprocessor.h"

namespace Ogre {
    namespace GLSL {
    //-----------------------------------------------------------------------
    static RenderOperation::OperationType parseOperationType(const String& val)
    {
        if (val == "point_list")
        {
            return RenderOperation::OT_POINT_LIST;
        }
        else if (val == "line_list")
        {
            return RenderOperation::OT_LINE_LIST;
        }
        else if (val == "line_list_adj")
        {
            return RenderOperation::OT_LINE_LIST_ADJ;
        }
        else if (val == "line_strip")
        {
            return RenderOperation::OT_LINE_STRIP;
        }
        else if (val == "line_strip_adj")
        {
            return RenderOperation::OT_LINE_STRIP_ADJ;
        }
        else if (val == "triangle_strip")
        {
            return RenderOperation::OT_TRIANGLE_STRIP;
        }
        else if (val == "triangle_strip_adj")
        {
            return RenderOperation::OT_TRIANGLE_STRIP_ADJ;
        }
        else if (val == "triangle_fan")
        {
            return RenderOperation::OT_TRIANGLE_FAN;
        }
        else if (val == "triangle_list_adj")
        {
            return RenderOperation::OT_TRIANGLE_LIST_ADJ;
        }
        else
        {
            //Triangle list is the default fallback. Keep it this way?
            return RenderOperation::OT_TRIANGLE_LIST;
        }
    }
    //-----------------------------------------------------------------------
    static const char* operationTypeToString(RenderOperation::OperationType val)
    {
        switch (val)
        {
        case RenderOperation::OT_POINT_LIST:
            return "point_list";
            break;
        case RenderOperation::OT_LINE_LIST:
            return "line_list";
            break;
        case RenderOperation::OT_LINE_LIST_ADJ:
            return "line_list_adj";
            break;
        case RenderOperation::OT_LINE_STRIP:
            return "line_strip";
            break;
        case RenderOperation::OT_LINE_STRIP_ADJ:
            return "line_strip_adj";
            break;
        case RenderOperation::OT_TRIANGLE_STRIP:
            return "triangle_strip";
            break;
        case RenderOperation::OT_TRIANGLE_STRIP_ADJ:
            return "triangle_strip_adj";
            break;
        case RenderOperation::OT_TRIANGLE_FAN:
            return "triangle_fan";
            break;
        case RenderOperation::OT_TRIANGLE_LIST_ADJ:
            return "triangle_list_adj";
            break;
        case RenderOperation::OT_TRIANGLE_LIST:
        default:
            return "triangle_list";
            break;
        }
    }
    /// Command object for setting the input operation type (geometry shader only)
    class CmdInputOperationType : public ParamCommand
    {
    public:
        String doGet(const void* target) const override
        {
            const GLSLProgram* t = static_cast<const GLSLProgram*>(target);
            return operationTypeToString(t->getInputOperationType());
        }
        void doSet(void* target, const String& val) override
        {
            GLSLProgram* t = static_cast<GLSLProgram*>(target);
            t->setInputOperationType(parseOperationType(val));
        }
    };
    /// Command object for setting the output operation type (geometry shader only)
    class CmdOutputOperationType : public ParamCommand
    {
    public:
        String doGet(const void* target) const override
        {
            const GLSLProgram* t = static_cast<const GLSLProgram*>(target);
            return operationTypeToString(t->getOutputOperationType());
        }
        void doSet(void* target, const String& val) override
        {
            GLSLProgram* t = static_cast<GLSLProgram*>(target);
            t->setOutputOperationType(parseOperationType(val));
        }
    };
    /// Command object for setting the maximum output vertices (geometry shader only)
    class CmdMaxOutputVertices : public ParamCommand
    {
    public:
        String doGet(const void* target) const override
        {
            const GLSLProgram* t = static_cast<const GLSLProgram*>(target);
            return StringConverter::toString(t->getMaxOutputVertices());
        }
        void doSet(void* target, const String& val) override
        {
            GLSLProgram* t = static_cast<GLSLProgram*>(target);
            t->setMaxOutputVertices(StringConverter::parseInt(val));
        }
    };
    static CmdInputOperationType msInputOperationTypeCmd;
    static CmdOutputOperationType msOutputOperationTypeCmd;
    static CmdMaxOutputVertices msMaxOutputVerticesCmd;
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
    void GLSLProgram::loadFromSource()
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
            case GPT_MESH_PROGRAM:
            case GPT_TASK_PROGRAM:
                break;
            }
            mGLShaderHandle = (size_t)glCreateShaderObjectARB(shaderType);
        }

        // Add preprocessor extras and main source
        if (!mSource.empty())
        {
            const char *source = mSource.c_str();
            glShaderSourceARB((GLhandleARB)mGLShaderHandle, 1, &source, NULL);
        }

        glCompileShaderARB((GLhandleARB)mGLShaderHandle);
        // check for compile errors
        int compiled = 0;
        glGetObjectParameterivARB((GLhandleARB)mGLShaderHandle, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

        String compileInfo = getObjectInfo(mGLShaderHandle);

        if (!compiled)
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, getResourceLogName() + " " + compileInfo, "compile");

        // probably we have warnings
        if (!compileInfo.empty())
            LogManager::getSingleton().stream(LML_WARNING) << getResourceLogName() << " " << compileInfo;
    }
    //-----------------------------------------------------------------------
    void GLSLProgram::unloadHighLevelImpl(void)
    {
        if (isSupported())
        {
            glDeleteObjectARB((GLhandleARB)mGLShaderHandle);
            mGLShaderHandle = 0;

            // destroy all programs using this shader
            GLSLLinkProgramManager::getSingletonPtr()->destroyAllByShader(this);
        }
    }
    //-----------------------------------------------------------------------
    void GLSLProgram::buildConstantDefinitions()
    {
        // We need an accurate list of all the uniforms in the shader, but we
        // can't get at them until we link all the shaders into a program object.


        // Therefore instead, parse the source code manually and extract the uniforms
        createParameterMappingStructures(true);
        mLogicalToPhysical.reset();

        GLSLLinkProgramManager::getSingleton().extractUniformsFromGLSL(
            mSource, *mConstantDefs, getResourceLogName());

        // Also parse any attached sources
        for (auto childShader : mAttachedGLSLPrograms)
        {
            GLSLLinkProgramManager::getSingleton().extractUniformsFromGLSL(
                childShader->getSource(), *mConstantDefs, childShader->getName());

        }
    }

    //-----------------------------------------------------------------------
    GLSLProgram::GLSLProgram(ResourceManager* creator, 
        const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader)
        : GLSLShaderCommon(creator, name, handle, group, isManual, loader)
        , mInputOperationType(RenderOperation::OT_TRIANGLE_LIST)
        , mOutputOperationType(RenderOperation::OT_TRIANGLE_LIST)
        , mMaxOutputVertices(3)
    {
        // add parameter command "attach" to the material serializer dictionary
        if (createParamDictionary("GLSLProgram"))
        {
            setupBaseParamDictionary();
            ParamDictionary* dict = getParamDictionary();

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
        mPassFFPStates = Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_FIXED_FUNCTION);
    }

    //-----------------------------------------------------------------------
    void GLSLProgram::attachToProgramObject( const uint programObject )
    {
        // attach child objects
        for (auto childShader : mAttachedGLSLPrograms)
        {
            childShader->attachToProgramObject(programObject);
        }
        glAttachObjectARB( (GLhandleARB)programObject, (GLhandleARB)mGLShaderHandle );
        GLenum glErr = glGetError();
        if(glErr != GL_NO_ERROR)
        {
            reportGLSLError( glErr, "GLSLProgram::attachToProgramObject",
                "Error attaching " + mName + " shader object to GLSL Program Object", programObject );
        }

    }
    //-----------------------------------------------------------------------
    void GLSLProgram::detachFromProgramObject( const uint programObject )
    {
        glDetachObjectARB((GLhandleARB)programObject, (GLhandleARB)mGLShaderHandle);

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
    //-----------------------------------------------------------------------------
    void GLSLProgram::bindProgram(void)
    {
        // Tell the Link Program Manager what shader is to become active
        GLSLLinkProgramManager::getSingleton().setActiveShader( mType, this );
    }
    //-----------------------------------------------------------------------------
    void GLSLProgram::unbindProgram(void)
    {
        // Tell the Link Program Manager what shader is to become inactive
        GLSLLinkProgramManager::getSingleton().setActiveShader( mType, NULL );
        // change back to fixed pipeline
        glUseProgramObjectARB(0);
    }

    //-----------------------------------------------------------------------------
    void GLSLProgram::bindProgramParameters(GpuProgramParametersSharedPtr params, uint16 mask)
    {
        // link can throw exceptions, ignore them at this point
        try
        {
            // activate the link program object
            GLSLLinkProgram* linkProgram = GLSLLinkProgramManager::getSingleton().getActiveLinkProgram();
            // pass on parameters from params to program object uniforms
            linkProgram->updateUniforms(params, mask, mType);
        }
        catch (Exception&) {}

    }
    //-----------------------------------------------------------------------------
    bool GLSLProgram::isAttributeValid(VertexElementSemantic semantic, uint index)
    {
        // get link program - only call this in the context of bound program
        GLSLLinkProgram* linkProgram = GLSLLinkProgramManager::getSingleton().getActiveLinkProgram();

        if (linkProgram->isAttributeValid(semantic, index))
        {
            return true;
        }
        else
        {
            // fall back to default implementation, allow default bindings
            return GLGpuProgramBase::isAttributeValid(semantic, index);
        }
    }
}
}
