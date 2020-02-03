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
#include "OgreStableHeaders.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreGpuProgramManager.h"

namespace Ogre
{
    /// Command object for setting macro defines
    class CmdPreprocessorDefines : public ParamCommand
    {
    public:
        //-----------------------------------------------------------------------
        String doGet(const void* target) const
        {
            return static_cast<const HighLevelGpuProgram*>(target)->getPreprocessorDefines();
        }
        void doSet(void* target, const String& val)
        {
            static_cast<HighLevelGpuProgram*>(target)->setPreprocessorDefines(val);
        }
    };
    static CmdPreprocessorDefines msCmdPreprocessorDefines;

    void HighLevelGpuProgram::setupBaseParamDictionary()
    {
        GpuProgram::setupBaseParamDictionary();
        ParamDictionary* dict = getParamDictionary();

        dict->addParameter(ParameterDef("preprocessor_defines", "", PT_STRING), &msCmdPreprocessorDefines);
    }

    //---------------------------------------------------------------------------
    HighLevelGpuProgram::HighLevelGpuProgram(ResourceManager* creator, 
        const String& name, ResourceHandle handle, const String& group, 
        bool isManual, ManualResourceLoader* loader)
        : GpuProgram(creator, name, handle, group, isManual, loader), 
        mHighLevelLoaded(false), mConstantDefsBuilt(false), mAssemblerProgram()
    {
    }
    //---------------------------------------------------------------------------
    void HighLevelGpuProgram::loadImpl()
    {
        if (isSupported())
        {
            // load self 
            loadHighLevel();

            // create low-level implementation
            createLowLevelImpl();
            // load constructed assembler program (if it exists)
            if (mAssemblerProgram)
            {
                mAssemblerProgram->load();
            }

        }
    }
    //---------------------------------------------------------------------------
    void HighLevelGpuProgram::unloadImpl()
    {   
        if (mAssemblerProgram)
        {
            mAssemblerProgram->getCreator()->remove(mAssemblerProgram);
            mAssemblerProgram.reset();
        }

        unloadHighLevel();
        resetCompileError();
    }
    //---------------------------------------------------------------------------
    HighLevelGpuProgram::~HighLevelGpuProgram()
    {
        // superclasses will trigger unload
    }
    //---------------------------------------------------------------------------
    GpuProgramParametersSharedPtr HighLevelGpuProgram::createParameters(void)
    {
        // Lock mutex before allowing this since this is a top-level method
        // called outside of the load()
        OGRE_LOCK_AUTO_MUTEX;

        // Make sure param defs are loaded
        GpuProgramParametersSharedPtr params = GpuProgramManager::getSingleton().createParameters();
        // Only populate named parameters if we can support this program
        if (this->isSupported())
        {
            safePrepare(); // loads source
            loadHighLevel();
            // Errors during load may have prevented compile
            if (this->isSupported())
            {
                populateParameterNames(params);
            }
        }
        // Copy in default parameters if present
        if (mDefaultParams)
            params->copyConstantsFrom(*(mDefaultParams.get()));
        return params;
    }
    size_t HighLevelGpuProgram::calculateSize(void) const
    {
        size_t memSize = GpuProgram::calculateSize();
        if(mAssemblerProgram)
            memSize += mAssemblerProgram->calculateSize();

        return memSize;
    }

    std::vector<std::pair<const char*, const char*>> HighLevelGpuProgram::parseDefines(String& defines)
    {
        std::vector<std::pair<const char*, const char*>> ret;
        if (defines.empty())
            return ret;

        String::size_type pos = 0;
        while (pos != String::npos)
        {
            // Find delims
            String::size_type endPos = defines.find_first_of(";,=", pos);
            if (endPos != String::npos)
            {
                String::size_type macro_name_start = pos;
                pos = endPos;

                // Check definition part
                if (defines[pos] == '=')
                {
                    // Setup null character for macro name
                    defines[pos] = '\0';
                    // set up a definition, skip delim
                    ++pos;
                    String::size_type macro_val_start = pos;

                    endPos = defines.find_first_of(";,", pos);
                    if (endPos == String::npos)
                    {
                        pos = endPos;
                    }
                    else
                    {
                        defines[endPos] = '\0';
                        pos = endPos+1;
                    }

                    ret.push_back({&defines[macro_name_start], &defines[macro_val_start]});
                }
                else
                {
                    // Setup null character for macro name
                    defines[pos] = '\0';
                    // No definition part, define as "1"
                    ++pos;
                    ret.push_back({&defines[macro_name_start], "1"});
                }
            }
            else
            {
                if(pos < defines.size())
                    ret.push_back({&defines[pos], "1"});

                pos = endPos;
            }
        }

        return ret;
    }

    //---------------------------------------------------------------------------
    void HighLevelGpuProgram::loadHighLevel(void)
    {
        if (!mHighLevelLoaded)
        {
            GpuProgram::loadImpl();
            mHighLevelLoaded = !mCompileError;
        }
    }
    //---------------------------------------------------------------------------
    void HighLevelGpuProgram::unloadHighLevel(void)
    {
        if (mHighLevelLoaded)
        {
            unloadHighLevelImpl();
            // Clear saved constant defs
            mConstantDefsBuilt = false;
            createParameterMappingStructures(true);

            mHighLevelLoaded = false;
        }
    }
    //---------------------------------------------------------------------
    const GpuNamedConstants& HighLevelGpuProgram::getConstantDefinitions() const
    {
        if (!mConstantDefsBuilt)
        {
            buildConstantDefinitions();
            mConstantDefsBuilt = true;
        }
        return *mConstantDefs.get();

    }
    //---------------------------------------------------------------------
    void HighLevelGpuProgram::populateParameterNames(GpuProgramParametersSharedPtr params)
    {
        getConstantDefinitions();
        params->_setNamedConstants(mConstantDefs);
        // also set logical / physical maps for programs which use this
        params->_setLogicalIndexes(mFloatLogicalToPhysical, mDoubleLogicalToPhysical,
                                   mIntLogicalToPhysical);
    }

    //-----------------------------------------------------------------------
    String HighLevelGpuProgram::_resolveIncludes(const String& inSource, Resource* resourceBeingLoaded, const String& fileName)
    {
        String outSource;
        // output will be at least this big
        outSource.reserve(inSource.length());

        size_t startMarker = 0;
        size_t i = inSource.find("#include");

        bool supportsFilename = StringUtil::endsWith(fileName, "cg");
        String lineFilename = supportsFilename ? StringUtil::format(" \"%s\"", fileName.c_str()) : " 0";

        while (i != String::npos)
        {
            size_t includePos = i;
            size_t afterIncludePos = includePos + 8;
            size_t newLineBefore = inSource.rfind('\n', includePos);

            // check we're not in a comment
            size_t lineCommentIt = inSource.rfind("//", includePos);
            if (lineCommentIt != String::npos)
            {
                if (newLineBefore == String::npos || lineCommentIt > newLineBefore)
                {
                    // commented
                    i = inSource.find("#include", afterIncludePos);
                    continue;
                }

            }
            size_t blockCommentIt = inSource.rfind("/*", includePos);
            if (blockCommentIt != String::npos)
            {
                size_t closeCommentIt = inSource.rfind("*/", includePos);
                if (closeCommentIt == String::npos || closeCommentIt < blockCommentIt)
                {
                    // commented
                    i = inSource.find("#include", afterIncludePos);
                    continue;
                }

            }

            // find following newline (or EOF)
            size_t newLineAfter = inSource.find('\n', afterIncludePos);
            // find include file string container
            String endDelimeter = "\"";
            size_t startIt = inSource.find('\"', afterIncludePos);
            if (startIt == String::npos || startIt > newLineAfter)
            {
                // try <>
                startIt = inSource.find('<', afterIncludePos);
                if (startIt == String::npos || startIt > newLineAfter)
                {
                    OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                                "Badly formed #include directive (expected \" or <) in file " + fileName + ": " +
                                    inSource.substr(includePos, newLineAfter - includePos));
                }
                else
                {
                    endDelimeter = ">";
                }
            }
            size_t endIt = inSource.find(endDelimeter, startIt+1);
            if (endIt == String::npos || endIt <= startIt)
            {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Badly formed #include directive (expected " + endDelimeter +
                                                               ") in file " + fileName + ": " +
                                                               inSource.substr(includePos, newLineAfter - includePos));
            }

            // extract filename
            String filename(inSource.substr(startIt+1, endIt-startIt-1));

            // open included file
            DataStreamPtr resource = ResourceGroupManager::getSingleton().
                openResource(filename, resourceBeingLoaded->getGroup(), resourceBeingLoaded);

            // replace entire include directive line
            // copy up to just before include
            if (newLineBefore != String::npos && newLineBefore >= startMarker)
                outSource.append(inSource.substr(startMarker, newLineBefore-startMarker+1));

            // Count the line number of #include statement, account for new line after the statement
            size_t lineCount = std::count(inSource.begin(), inSource.begin() + newLineAfter, '\n') + 1;

            // use include filename if supported (cg) - else use include line as id (glsl)
            String incLineFilename = supportsFilename ? StringUtil::format(" \"%s\"", filename.c_str()) : StringUtil::format(" %zu", lineCount);

            // Add #line to the start of the included file to correct the line count)
            outSource.append("#line 1 " + incLineFilename + "\n");

            outSource.append(resource->getAsString());

            // Add #line to the end of the included file to correct the line count
            outSource.append("\n#line " + std::to_string(lineCount) + lineFilename);

            startMarker = newLineAfter;

            if (startMarker != String::npos)
                i = inSource.find("#include", startMarker);
            else
                i = String::npos;

        }
        // copy any remaining characters
        outSource.append(inSource.substr(startMarker));

        return outSource;
    }
}
