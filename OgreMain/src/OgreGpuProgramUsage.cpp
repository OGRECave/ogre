
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
#include "OgreGpuProgramUsage.h"
#include "OgreGpuProgramManager.h"
#include "OgreException.h"

namespace Ogre
{
    //-----------------------------------------------------------------------------
    GpuProgramUsage::GpuProgramUsage(GpuProgramType gptype, Pass* parent) :
        mType(gptype), mParent(parent), mProgram(), mRecreateParams(false)
    {
    }
	//-----------------------------------------------------------------------------
	GpuProgramUsage::GpuProgramUsage(const GpuProgramUsage& oth, Pass* parent)
        : mType(oth.mType)
		, mParent(parent)
        , mProgram(oth.mProgram)
        // nfz: parameters should be copied not just use a shared ptr to the original
		, mParameters(OGRE_NEW GpuProgramParameters(*oth.mParameters))
		, mRecreateParams(false)
	{
	}
	//---------------------------------------------------------------------
	GpuProgramUsage::~GpuProgramUsage()
	{
		if (!mProgram.isNull())
			mProgram->removeListener(this);
	}
	//-----------------------------------------------------------------------------
	void GpuProgramUsage::setProgramName(const String& name, bool resetParams)
	{
		if (!mProgram.isNull())
		{
			mProgram->removeListener(this);
			mRecreateParams = true;
		}

		mProgram = GpuProgramManager::getSingleton().getByName(name);

        if (mProgram.isNull())
        {
			String progType = (mType == GPT_VERTEX_PROGRAM ? "vertex" : 
				(mType == GPT_GEOMETRY_PROGRAM ? "geometry" : "fragment"));
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                "Unable to locate " + progType + " program called " + name + ".",
                "GpuProgramUsage::setProgramName");
        }

        // Reset parameters 
        if (resetParams || mParameters.isNull() || mRecreateParams)
		{
			recreateParameters();
		}

		// Listen in on reload events so we can regenerate params
		mProgram->addListener(this);

	}
    //-----------------------------------------------------------------------------
    void GpuProgramUsage::setParameters(GpuProgramParametersSharedPtr params)
    {
        mParameters = params;
    }
    //-----------------------------------------------------------------------------
    GpuProgramParametersSharedPtr GpuProgramUsage::getParameters(void)
    {
        if (mParameters.isNull())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "You must specify a program before "
                "you can retrieve parameters.", "GpuProgramUsage::getParameters");
        }

        return mParameters;
    }
    //-----------------------------------------------------------------------------
	void GpuProgramUsage::setProgram(GpuProgramPtr& prog) 
	{
        mProgram = prog;
        // Reset parameters 
        mParameters = mProgram->createParameters();
    }
    //-----------------------------------------------------------------------------
    void GpuProgramUsage::_load(void)
    {
        if (!mProgram->isLoaded())
            mProgram->load();

		// check type
		if (mProgram->isLoaded() && mProgram->getType() != mType)
		{
			String myType = (mType == GPT_VERTEX_PROGRAM ? "vertex" : 
				(mType == GPT_GEOMETRY_PROGRAM ? "geometry" : "fragment"));
			String yourType = (mProgram->getType() == GPT_VERTEX_PROGRAM ? "vertex" : 
				(mProgram->getType() == GPT_GEOMETRY_PROGRAM ? "geometry" : "fragment"));
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				mProgram->getName() + "is a " + yourType + " program, but you are assigning it to a " 
				+ myType + " program slot. This is invalid.",
				"GpuProgramUsage::setProgramName");

		}
    }
    //-----------------------------------------------------------------------------
    void GpuProgramUsage::_unload(void)
    {
        // TODO?
    }
	//---------------------------------------------------------------------
	void GpuProgramUsage::unloadingComplete(Resource* prog)
	{
		mRecreateParams = true;

	}
	//---------------------------------------------------------------------
	void GpuProgramUsage::loadingComplete(Resource* prog)
	{
		// Need to re-create parameters
		if (mRecreateParams)
			recreateParameters();

	}
	//---------------------------------------------------------------------
	void GpuProgramUsage::recreateParameters()
	{
		// Keep a reference to old ones to copy
		GpuProgramParametersSharedPtr savedParams = mParameters;

		// Create new params
		mParameters = mProgram->createParameters();

		// Copy old (matching) values across
		// Don't use copyConstantsFrom since program may be different
		if (!savedParams.isNull())
			mParameters->copyMatchingNamedConstantsFrom(*savedParams.get());

		mRecreateParams = false;

	}



}
