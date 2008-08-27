
/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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
    GpuProgramUsage::GpuProgramUsage(GpuProgramType gptype) :
        mType(gptype), mProgram()
    {
    }
	//-----------------------------------------------------------------------------
	GpuProgramUsage::GpuProgramUsage(const GpuProgramUsage& oth)
        : mType(oth.mType)
        , mProgram(oth.mProgram)
        // nfz: parameters should be copied not just use a shared ptr to the original
		, mParameters(OGRE_NEW GpuProgramParameters(*oth.mParameters))
	{
	}
	//-----------------------------------------------------------------------------
	void GpuProgramUsage::setProgramName(const String& name, bool resetParams)
	{
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
        if (resetParams || mParameters.isNull())
            mParameters = mProgram->createParameters();

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
    }
    //-----------------------------------------------------------------------------
    void GpuProgramUsage::_unload(void)
    {
        // TODO?
    }

}
