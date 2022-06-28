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
#include "OgreGLSLESCgProgram.h"
#include "OgreResourceGroupManager.h"
#include "OgreStringConverter.h"
#include "OgreGpuProgramManager.h"
#include "OgreLogManager.h"
#include "hlsl2glsl.h"  // use the code from here: http://code.google.com/p/hlsl2glslfork/

namespace Ogre {

    //-----------------------------------------------------------------------
    GLSLESCgProgram::CmdProfiles GLSLESCgProgram::msCmdProfiles;
    //-----------------------------------------------------------------------
    // for use with Hlsl2Glsl_SetUserAttributeNames to map ogre att names
    EAttribSemantic gAttribSemantic[] = {
        EAttrSemPosition,
        EAttrSemNormal,
        EAttrSemColor0,
        EAttrSemColor1,
        EAttrSemColor2,
        EAttrSemColor3,
        EAttrSemTex0,
        EAttrSemTex1,
        EAttrSemTex2,
        EAttrSemTex3,
        EAttrSemTex4,
        EAttrSemTex5,
        EAttrSemTex6,
        EAttrSemTex7,
        EAttrSemTex8,
        EAttrSemTex9,
        EAttrSemTangent,
        EAttrSemBinormal,
        EAttrSemBlendWeight,
        EAttrSemBlendIndices
    };
    int gNumberOfAttribSemantic = sizeof(gAttribSemantic) / sizeof(EAttribSemantic);

    // for use with Hlsl2Glsl_SetUserAttributeNames to map ogre att names
    const char* gAttribString[] = {
        "vertex",
        "normal",
        "colour",
        "colour1",
        "colour2",
        "colour3",
        "uv",
        "uv1",
        "uv2",
        "uv3",
        "uv4",
        "uv5",
        "uv6",
        "uv7",
        "uv8",
        "uv9",
        "tangent",
        "binormal",
        "blendWeights",
        "blendIndices"
    };
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    GLSLESCgProgram::GLSLESCgProgram(ResourceManager* creator, 
        const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader)
        : GLSLESProgram(creator, name, handle, group, isManual, loader)
    {

        // Add parameter "entry_point" and "profiles" to the material serializer dictionary
        if (createParamDictionary("GLSLESCgProgram"))
        {
            setupBaseParamDictionary();
            ParamDictionary* dict = getParamDictionary();

            dict->addParameter(ParameterDef("profiles", 
                "Space-separated list of Cg profiles supported by this profile.",
                PT_STRING),&msCmdProfiles);

        }
    }
    //---------------------------------------------------------------------------
    GLSLESCgProgram::~GLSLESCgProgram()
    {
    }
    //-----------------------------------------------------------------------
    String GLSLESCgProgram::deleteRegisterFromCg(const String& inSource)
    {
#define REGISTER_STRING ": register"
        String outSource;
        // output will be at least this big
        outSource.reserve(inSource.length());

        size_t startMarker = 0;
        size_t i = inSource.find(REGISTER_STRING);
        while (i != String::npos)
        {
            size_t registerPos = i;
            size_t afterRegisterPos = registerPos + 8;
            size_t newLineBefore = inSource.rfind("\n", registerPos);

            // check we're not in a comment
            size_t lineCommentIt = inSource.rfind("//", registerPos);
            if (lineCommentIt != String::npos)
            {
                if (newLineBefore == String::npos || lineCommentIt > newLineBefore)
                {
                    // commented
                    i = inSource.find(REGISTER_STRING, afterRegisterPos);
                    continue;
                }

            }
            size_t blockCommentIt = inSource.rfind("/*", registerPos);
            if (blockCommentIt != String::npos)
            {
                size_t closeCommentIt = inSource.rfind("*/", registerPos);
                if (closeCommentIt == String::npos || closeCommentIt < blockCommentIt)
                {
                    // commented
                    i = inSource.find(REGISTER_STRING, afterRegisterPos);
                    continue;
                }

            }

            // find following newline (or EOF)
            size_t newLineAfter = inSource.find("\n", afterRegisterPos);
            // find register file string container
            String endDelimiter = "\"";
            size_t startIt = inSource.find("\"", afterRegisterPos);
            if (startIt == String::npos || startIt > newLineAfter)
            {
                // try <>
                startIt = inSource.find("(", afterRegisterPos);
                if (startIt == String::npos || startIt > newLineAfter)
                {
                    OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Badly formed register directive (expected () in file "
                        + mFilename + ": " + inSource.substr(registerPos, newLineAfter-registerPos),
                        "GLSLESCgProgram::deleteRegisterFromCg");
                }
                else
                {
                    endDelimiter = ")";
                }
            }
            size_t endIt = inSource.find(endDelimiter, startIt+1);
            if (endIt == String::npos || endIt <= startIt)
            {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                    "Badly formed register directive (expected " + endDelimiter + ") in file "
                    + mFilename + ": " + inSource.substr(registerPos, newLineAfter-registerPos),
                    "GLSLESCgProgram::deleteRegisterFromCg");
            }

            // delete the register
            if (newLineBefore != String::npos && registerPos >= startMarker)
                outSource.append(inSource.substr(startMarker, registerPos-startMarker));

            startMarker = endIt + 1;

            if (startMarker != String::npos)
                i = inSource.find(REGISTER_STRING, startMarker);
            else
                i = String::npos;

        }
        // copy any remaining characters
        outSource.append(inSource.substr(startMarker));

        return outSource;
    }
    //-----------------------------------------------------------------------
    void GLSLESCgProgram::loadFromSource( void )
    {
        GLSLESProgram::prepareImpl(); // loads + preprocesses source

        // check if syntax is supported
        if (!isSyntaxSupported()) 
        {
            mSource = "";
            LogManager::getSingleton().logMessage(
                "File:" + mFilename + 
                " has unsupported syntax for hlsl2glsl.");
            return;
        }

        // delete ": register(xx)" that hlsl2glsl doesn't know to handle
        String sourceToUse = deleteRegisterFromCg(sourceToUse);


        // select the program type
        EShLanguage lang = EShLangCount;
        switch(getType())
        {
        case GPT_VERTEX_PROGRAM:
            lang = EShLangVertex;
            break;
        case GPT_FRAGMENT_PROGRAM:
            lang = EShLangFragment;
            break;
        default:
            mSource = "";
            LogManager::getSingleton().logMessage(
                "File:" + mFilename + 
                " has unsupported program type.");
            return;
        }

       
        int options = 0;
          //  options |= ETranslateOpUsePrecision; this option doesn't work good with the ogre glsl parameter parser

        // create the parser
        ShHandle  parser = Hlsl2Glsl_ConstructCompiler(lang);

        // set the user attribute to be ogre's
        Hlsl2Glsl_SetUserAttributeNames (parser, gAttribSemantic, gAttribString, gNumberOfAttribSemantic);

        int res = 0;
        // parse the file
        res = Hlsl2Glsl_Parse(parser, sourceToUse.c_str(), ETargetGLSL_ES_100, nullptr, options);
        // convert from cg to glsl
        res = res && Hlsl2Glsl_Translate(parser,  mEntryPoint.c_str(), ETargetGLSL_ES_100, options);

        // check for error
        if (res == 0)
        {
            const char*  parserErrors = Hlsl2Glsl_GetInfoLog(parser);

            LogManager::getSingleton().logMessage(
                "File:" + mFilename + 
                " failed to convert from Cg to glsl with the following errors:\n" + parserErrors);

            Hlsl2Glsl_DestructCompiler(parser);

            mSource = "";
            return;
        }


        String glslSource;
        // add the glsl precision thing
        glslSource = glslSource + "#version 100\n";
        glslSource = glslSource + "precision mediump int;\n";
        glslSource = glslSource + "precision mediump float;\n";
        //glslSource = glslSource + "precision lowp sampler2D;\n";
        //glslSource = glslSource + "precision lowp samplerCube;\n";

        // get the glsl code from the parser
        glslSource = glslSource + Hlsl2Glsl_GetShader(parser);

        // update mSource to now be glsl code
        mSource = glslSource;

        // delete the parser
        Hlsl2Glsl_DestructCompiler(parser);

    }
    //-----------------------------------------------------------------------
    const String& GLSLESCgProgram::getLanguage( void ) const
    {
        static const String language = "cg";

        return language;
    }
    //-----------------------------------------------------------------------
    void GLSLESCgProgram::setProfiles( const StringVector& profiles )
    {
        mProfiles.clear();
        StringVector::const_iterator i, iend;
        iend = profiles.end();
        for (i = profiles.begin(); i != iend; ++i)
        {
            mProfiles.push_back(*i);
        }
    }
    //-----------------------------------------------------------------------
    bool GLSLESCgProgram::isSyntaxSupported()
    {
        bool syntaxSupported = false;
        StringVector::iterator i, iend;
        iend = mProfiles.end();
        GpuProgramManager& gpuMgr = GpuProgramManager::getSingleton();
        for (i = mProfiles.begin(); i != iend; ++i)
        {
            if (gpuMgr.isSyntaxSupported(*i))
            {
                syntaxSupported = true;
                break;
            }
        }
        return syntaxSupported;
    }
    //-----------------------------------------------------------------------
    String GLSLESCgProgram::CmdProfiles::doGet(const void *target) const
    {
        return StringConverter::toString(
            static_cast<const GLSLESCgProgram*>(target)->getProfiles() );
    }
    void GLSLESCgProgram::CmdProfiles::doSet(void *target, const String& val)
    {
        static_cast<GLSLESCgProgram*>(target)->setProfiles(StringUtil::split(val));
    }
    //-----------------------------------------------------------------------

}
