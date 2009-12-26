/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
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
        mHighLevelLoaded(false), mAssemblerProgram(0), mConstantDefsBuilt(false)
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
        if (!mAssemblerProgram.isNull())
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
		OGRE_LOCK_AUTO_MUTEX

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
		params->_setLogicalIndexes(mFloatLogicalToPhysical, mIntLogicalToPhysical);
	}
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	HighLevelGpuProgramPtr& HighLevelGpuProgramPtr::operator=(const GpuProgramPtr& r)
	{
		// Can assign direct
		if (pRep == static_cast<HighLevelGpuProgram*>(r.getPointer()))
			return *this;
		release();
		// lock & copy other mutex pointer
        OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
        {
		    OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
		    OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
		    pRep = static_cast<HighLevelGpuProgram*>(r.getPointer());
		    pUseCount = r.useCountPointer();
		    if (pUseCount)
		    {
			    ++(*pUseCount);
		    }
        }
		else
		{
			// RHS must be a null pointer
			assert(r.isNull() && "RHS must be null if it has no mutex!");
			setNull();
		}
		return *this;
	}


}
