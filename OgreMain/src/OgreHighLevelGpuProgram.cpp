/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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
}
