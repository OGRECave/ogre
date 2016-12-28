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

#include "OgreGLSLProgramCommon.h"
#include "OgreGLSLPreprocessor.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    GLSLProgramCommon::CmdPreprocessorDefines GLSLProgramCommon::msCmdPreprocessorDefines;
    GLSLProgramCommon::CmdAttach GLSLProgramCommon::msCmdAttach;
    GLSLProgramCommon::CmdColumnMajorMatrices GLSLProgramCommon::msCmdColumnMajorMatrices;
    GLSLProgramCommon::CmdInputOperationType GLSLProgramCommon::msInputOperationTypeCmd;
    GLSLProgramCommon::CmdOutputOperationType GLSLProgramCommon::msOutputOperationTypeCmd;
    GLSLProgramCommon::CmdMaxOutputVertices GLSLProgramCommon::msMaxOutputVerticesCmd;

    //-----------------------------------------------------------------------
    void GLSLProgramCommon::loadFromSource(void)
    {
        // Preprocess the GLSL shader in order to get a clean source
        CPreprocessor cpp;

        // Pass all user-defined macros to preprocessor
        if (!mPreprocessorDefines.empty ())
        {
            String::size_type pos = 0;
            while (pos != String::npos)
            {
                // Find delims
                String::size_type endPos = mPreprocessorDefines.find_first_of(";,=", pos);
                if (endPos != String::npos)
                {
                    String::size_type macro_name_start = pos;
                    size_t macro_name_len = endPos - pos;
                    pos = endPos;

                    // Check definition part
                    if (mPreprocessorDefines[pos] == '=')
                    {
                        // set up a definition, skip delim
                        ++pos;
                        String::size_type macro_val_start = pos;
                        size_t macro_val_len;

                        endPos = mPreprocessorDefines.find_first_of(";,", pos);
                        if (endPos == String::npos)
                        {
                            macro_val_len = mPreprocessorDefines.size () - pos;
                            pos = endPos;
                        }
                        else
                        {
                            macro_val_len = endPos - pos;
                            pos = endPos+1;
                        }
                        cpp.Define (
                            mPreprocessorDefines.c_str () + macro_name_start, macro_name_len,
                            mPreprocessorDefines.c_str () + macro_val_start, macro_val_len);
                    }
                    else
                    {
                        // No definition part, define as "1"
                        ++pos;
                        cpp.Define (
                            mPreprocessorDefines.c_str () + macro_name_start, macro_name_len, 1);
                    }
                }
                else
                {
                    if(pos < mPreprocessorDefines.size())
                        cpp.Define (mPreprocessorDefines.c_str () + pos, mPreprocessorDefines.size() - pos, 1);

                    pos = endPos;
                }
            }
        }

        size_t out_size = 0;
        const char *src = mSource.c_str ();
        size_t src_len = mSource.size ();
        char *out = cpp.Parse (src, src_len, out_size);
        if (!out || !out_size)
            // Failed to preprocess, break out
            OGRE_EXCEPT (Exception::ERR_RENDERINGAPI_ERROR,
            "Failed to preprocess shader " + mName,
            __FUNCTION__);

        mSource = String (out, out_size);
        if (out < src || out > src + src_len)
            free (out);
    }
    //---------------------------------------------------------------------------
    void GLSLProgramCommon::unloadImpl()
    {
        // We didn't create mAssemblerProgram through a manager, so override this
        // implementation so that we don't try to remove it from one. Since getCreator()
        // is used, it might target a different matching handle!
        mAssemblerProgram.setNull();

        unloadHighLevel();
    }

    //-----------------------------------------------------------------------
    void GLSLProgramCommon::populateParameterNames(GpuProgramParametersSharedPtr params)
    {
        getConstantDefinitions();
        params->_setNamedConstants(mConstantDefs);
        // Don't set logical / physical maps here, as we can't access parameters by logical index in GLHL.
    }
    //-----------------------------------------------------------------------
    GLSLProgramCommon::GLSLProgramCommon(ResourceManager* creator, 
        const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader)
        : HighLevelGpuProgram(creator, name, handle, group, isManual, loader)
        , mCompiled(0)
        , mInputOperationType(RenderOperation::OT_TRIANGLE_LIST)
        , mOutputOperationType(RenderOperation::OT_TRIANGLE_LIST)
        , mMaxOutputVertices(3)
        , mColumnMajorMatrices(true)
    {
    }
    //-----------------------------------------------------------------------
    String GLSLProgramCommon::CmdAttach::doGet(const void *target) const
    {
        return (static_cast<const GLSLProgramCommon*>(target))->getAttachedShaderNames();
    }
    //-----------------------------------------------------------------------
    void GLSLProgramCommon::CmdAttach::doSet(void *target, const String& shaderNames)
    {
        //get all the shader program names: there could be more than one
        StringVector vecShaderNames = StringUtil::split(shaderNames, " \t", 0);

        size_t programNameCount = vecShaderNames.size();
        for ( size_t i = 0; i < programNameCount; ++i )
        {
            static_cast<GLSLProgramCommon*>(target)->attachChildShader(vecShaderNames[i]);
        }
    }
    //-----------------------------------------------------------------------
    String GLSLProgramCommon::CmdPreprocessorDefines::doGet(const void *target) const
    {
        return static_cast<const GLSLProgramCommon*>(target)->getPreprocessorDefines();
    }
    void GLSLProgramCommon::CmdPreprocessorDefines::doSet(void *target, const String& val)
    {
        static_cast<GLSLProgramCommon*>(target)->setPreprocessorDefines(val);
    }

    //-----------------------------------------------------------------------
    void GLSLProgramCommon::attachChildShader(const String& name)
    {
        // is the name valid and already loaded?
        // check with the high level program manager to see if it was loaded
        HighLevelGpuProgramPtr hlProgram = HighLevelGpuProgramManager::getSingleton().getByName(name);
        if (!hlProgram.isNull())
        {
            if (hlProgram->getSyntaxCode() == "glsl")
            {
                // make sure attached program source gets loaded and compiled
                // don't need a low level implementation for attached shader objects
                // loadHighLevelImpl will only load the source and compile once
                // so don't worry about calling it several times
                GLSLProgramCommon* childShader = static_cast<GLSLProgramCommon*>(hlProgram.getPointer());
                // load the source and attach the child shader only if supported
                if (isSupported())
                {
                    childShader->loadHighLevelImpl();
                    // add to the container
                    mAttachedGLSLPrograms.push_back( childShader );
                    mAttachedShaderNames += name + " ";
                }
            }
        }
    }
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
        else if (val == "line_strip")
        {
            return RenderOperation::OT_LINE_STRIP;
        }
        else if (val == "triangle_strip")
        {
            return RenderOperation::OT_TRIANGLE_STRIP;
        }
        else if (val == "triangle_fan")
        {
            return RenderOperation::OT_TRIANGLE_FAN;
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
        case RenderOperation::OT_LINE_STRIP:
            return "line_strip";
            break;
        case RenderOperation::OT_TRIANGLE_STRIP:
            return "triangle_strip";
            break;
        case RenderOperation::OT_TRIANGLE_FAN:
            return "triangle_fan";
            break;
        case RenderOperation::OT_TRIANGLE_LIST:
        default:
            return "triangle_list";
            break;
        }
    }
    //-----------------------------------------------------------------------
    String GLSLProgramCommon::CmdColumnMajorMatrices::doGet(const void *target) const
    {
        return StringConverter::toString(static_cast<const GLSLProgramCommon*>(target)->getColumnMajorMatrices());
    }
    void GLSLProgramCommon::CmdColumnMajorMatrices::doSet(void *target, const String& val)
    {
        static_cast<GLSLProgramCommon*>(target)->setColumnMajorMatrices(StringConverter::parseBool(val));
    }
    //-----------------------------------------------------------------------
    String GLSLProgramCommon::CmdInputOperationType::doGet(const void* target) const
    {
        const GLSLProgramCommon* t = static_cast<const GLSLProgramCommon*>(target);
        return operationTypeToString(t->getInputOperationType());
    }
    void GLSLProgramCommon::CmdInputOperationType::doSet(void* target, const String& val)
    {
        GLSLProgramCommon* t = static_cast<GLSLProgramCommon*>(target);
        t->setInputOperationType(parseOperationType(val));
    }
    //-----------------------------------------------------------------------
    String GLSLProgramCommon::CmdOutputOperationType::doGet(const void* target) const
    {
        const GLSLProgramCommon* t = static_cast<const GLSLProgramCommon*>(target);
        return operationTypeToString(t->getOutputOperationType());
    }
    void GLSLProgramCommon::CmdOutputOperationType::doSet(void* target, const String& val)
    {
        GLSLProgramCommon* t = static_cast<GLSLProgramCommon*>(target);
        t->setOutputOperationType(parseOperationType(val));
    }
    //-----------------------------------------------------------------------
    String GLSLProgramCommon::CmdMaxOutputVertices::doGet(const void* target) const
    {
        const GLSLProgramCommon* t = static_cast<const GLSLProgramCommon*>(target);
        return StringConverter::toString(t->getMaxOutputVertices());
    }
    void GLSLProgramCommon::CmdMaxOutputVertices::doSet(void* target, const String& val)
    {
        GLSLProgramCommon* t = static_cast<GLSLProgramCommon*>(target);
        t->setMaxOutputVertices(StringConverter::parseInt(val));
    }
}
