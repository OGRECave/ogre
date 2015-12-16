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
#include "OgreException.h"
#include "OgreGpuProgramManager.h"
#include "OgreLogManager.h"
#include "OgreHighLevelGpuProgramManager.h"

namespace Ogre
{
    //---------------------------------------------------------------------------
    HighLevelGpuProgram::HighLevelGpuProgram(ResourceManager* creator, 
        const String& name, ResourceHandle handle, const String& group, 
        bool isManual, ManualResourceLoader* loader)
        : GpuProgram(creator, name, handle, group, isManual, loader), 
        mHighLevelLoaded(false), mAssemblerProgram(), mConstantDefsBuilt(false)
    {
    }

    //-----------------------------------------------------------------------------
    HighLevelGpuProgramPtr HighLevelGpuProgram::clone(const String& cloneName)
    {
        HighLevelGpuProgramPtr program = HighLevelGpuProgramManager::getSingleton().createProgram
            (cloneName
                , this->mGroup
                , this->mSyntaxCode
                , this->mType);

        program->setSource(this->mSource);

        const ParameterList& params = this->getParameters();
        ParameterList::const_iterator it_end = params.end();
        for (ParameterList::const_iterator it = params.begin(); it != it_end; it++)
        {
            const ParameterDef& param = *it;
            program->setParameter(param.name, this->getParameter(param.name));
        }

        return program;
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
            if (!mAssemblerProgram.isNull() && mAssemblerProgram.getPointer() != this)
            {
                mAssemblerProgram->load();
            }

        }
    }
    //---------------------------------------------------------------------------
    void HighLevelGpuProgram::unloadImpl()
    {   
        if (!mAssemblerProgram.isNull() && mAssemblerProgram.getPointer() != this)
        {
            mAssemblerProgram->getCreator()->remove(mAssemblerProgram->getHandle());
            mAssemblerProgram.setNull();
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
            loadHighLevel();
            // Errors during load may have prevented compile
            if (this->isSupported())
            {
                populateParameterNames(params);
            }
        }
        // Copy in default parameters if present
        if (!mDefaultParams.isNull())
            params->copyConstantsFrom(*(mDefaultParams.get()));
        return params;
    }
    size_t HighLevelGpuProgram::calculateSize(void) const
    {
        size_t memSize = 0;
        memSize += sizeof(bool);
        if(!mAssemblerProgram.isNull() && (mAssemblerProgram.getPointer() != this) )
            memSize += mAssemblerProgram->calculateSize();

        memSize += GpuProgram::calculateSize();

        return memSize;
    }

    //---------------------------------------------------------------------------
    void HighLevelGpuProgram::loadHighLevel(void)
    {
        if (!mHighLevelLoaded)
        {
            try 
            {
                loadHighLevelImpl();
                mHighLevelLoaded = true;
                if (!mDefaultParams.isNull())
                {
                    // Keep a reference to old ones to copy
                    GpuProgramParametersSharedPtr savedParams = mDefaultParams;
                    // reset params to stop them being referenced in the next create
                    mDefaultParams.setNull();

                    // Create new params
                    mDefaultParams = createParameters();

                    // Copy old (matching) values across
                    // Don't use copyConstantsFrom since program may be different
                    mDefaultParams->copyMatchingNamedConstantsFrom(*savedParams.get());

                }

            }
            catch (const Exception& e)
            {
                // will already have been logged
                LogManager::getSingleton().stream()
                    << "High-level program " << mName << " encountered an error "
                    << "during loading and is thus not supported.\n"
                    << e.getFullDescription();

                mCompileError = true;
                mCompileErrorMessage = e.getFullDescription();
            }
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
    //---------------------------------------------------------------------------
    void HighLevelGpuProgram::loadHighLevelImpl(void)
    {
        if (mLoadFromFile)
        {
            // find & load source code
            DataStreamPtr stream = 
                ResourceGroupManager::getSingleton().openResource(
                    mFilename, mGroup, true, this);

            mSource = stream->getAsString();
        }

        loadFromSource();


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
                                           mIntLogicalToPhysical, mUIntLogicalToPhysical,
                                           mBoolLogicalToPhysical);
    }
	//-----------------------------------------------------------------------
	String HighLevelGpuProgram::getFullSourceWithIncludes() const
	{
		return resolveIncludes(mSource, const_cast<HighLevelGpuProgram*>(this), mFilename);
	}
	//-----------------------------------------------------------------------
	String HighLevelGpuProgram::resolveIncludes(const String& inSource, Resource* resourceBeingLoaded, const String& fileName)
	{
		String outSource;
		// output will be at least this big
		outSource.reserve(inSource.length());

		size_t startMarker = 0;
		size_t i = inSource.find("#include");
		while (i != String::npos)
		{
			size_t includePos = i;
			size_t afterIncludePos = includePos + 8;
			size_t newLineBefore = inSource.rfind("\n", includePos);

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
			size_t newLineAfter = inSource.find("\n", afterIncludePos);
			// find include file string container
			String endDelimeter = "\"";
			size_t startIt = inSource.find("\"", afterIncludePos);
			if (startIt == String::npos || startIt > newLineAfter)
			{
				// try <>
				startIt = inSource.find("<", afterIncludePos);
				if (startIt == String::npos || startIt > newLineAfter)
				{
					OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
						"Badly formed #include directive (expected \" or <) in file "
						+ fileName + ": " + inSource.substr(includePos, newLineAfter - includePos),
						"CgProgram::preprocessor");
				}
				else
				{
					endDelimeter = ">";
				}
			}
			size_t endIt = inSource.find(endDelimeter, startIt + 1);
			if (endIt == String::npos || endIt <= startIt)
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
					"Badly formed #include directive (expected " + endDelimeter + ") in file "
					+ fileName + ": " + inSource.substr(includePos, newLineAfter - includePos),
					"CgProgram::preprocessor");
			}

			// extract filename
			String filename(inSource.substr(startIt + 1, endIt - startIt - 1));

			// open included file
			DataStreamPtr resource = ResourceGroupManager::getSingleton().
				openResource(filename, resourceBeingLoaded->getGroup(), true, resourceBeingLoaded);

			// replace entire include directive line
			// copy up to just before include
			if (newLineBefore != String::npos && newLineBefore >= startMarker)
				outSource.append(inSource.substr(startMarker, newLineBefore - startMarker + 1));

			size_t lineCount = 0;
			size_t lineCountPos = 0;

			// Count the line number of #include statement
			lineCountPos = outSource.find('\n');
			while (lineCountPos != String::npos)
			{
				lineCountPos = outSource.find('\n', lineCountPos + 1);
				lineCount++;
			}

			// Add #line to the start of the included file to correct the line count
			outSource.append("#line 1 \"" + filename + "\"\n");

			outSource.append(resource->getAsString());

			// Add #line to the end of the included file to correct the line count
			outSource.append("\n#line " + Ogre::StringConverter::toString(lineCount) +
				"\"" + fileName + "\"\n");

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
