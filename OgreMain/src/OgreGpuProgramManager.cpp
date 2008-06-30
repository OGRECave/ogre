/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"
#include "OgreGpuProgramManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"


namespace Ogre {
    //-----------------------------------------------------------------------
    template<> GpuProgramManager* Singleton<GpuProgramManager>::ms_Singleton = 0;
    GpuProgramManager* GpuProgramManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    GpuProgramManager& GpuProgramManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
	//---------------------------------------------------------------------------
	GpuProgramManager::GpuProgramManager()
	{
		// Loading order
		mLoadOrder = 50.0f;
		// Resource type
		mResourceType = "GpuProgram";

		// subclasses should register with resource group manager
	}
	//---------------------------------------------------------------------------
	GpuProgramManager::~GpuProgramManager()
	{
		// subclasses should unregister with resource group manager
	}
	//---------------------------------------------------------------------------
    GpuProgramPtr GpuProgramManager::load(const String& name,
		const String& groupName, const String& filename, 
		GpuProgramType gptype, const String& syntaxCode)
    {
		GpuProgramPtr prg;
		{
			OGRE_LOCK_AUTO_MUTEX
			prg = getByName(name);
			if (prg.isNull())
			{
				prg = createProgram(name, groupName, filename, gptype, syntaxCode);
			}

		}
        prg->load();
        return prg;
    }
    //---------------------------------------------------------------------------
	GpuProgramPtr GpuProgramManager::loadFromString(const String& name, 
		const String& groupName, const String& code, 
        GpuProgramType gptype, const String& syntaxCode)
    {
		GpuProgramPtr prg;
		{
			OGRE_LOCK_AUTO_MUTEX
			prg = getByName(name);
			if (prg.isNull())
			{
				prg = createProgramFromString(name, groupName, code, gptype, syntaxCode);
			}

		}
        prg->load();
        return prg;
    }
    //---------------------------------------------------------------------------
    ResourcePtr GpuProgramManager::create(const String& name, const String& group, 
        GpuProgramType gptype, const String& syntaxCode, bool isManual, 
        ManualResourceLoader* loader)
    {
        // Call creation implementation
        ResourcePtr ret = ResourcePtr(
            createImpl(name, getNextHandle(), group, isManual, loader, gptype, syntaxCode));

        addImpl(ret);
        // Tell resource group manager
        ResourceGroupManager::getSingleton()._notifyResourceCreated(ret);
        return ret;
    }
    //---------------------------------------------------------------------------
	GpuProgramPtr GpuProgramManager::createProgram(const String& name, 
		const String& groupName, const String& filename, 
		GpuProgramType gptype, const String& syntaxCode)
    {
		GpuProgramPtr prg = create(name, groupName, gptype, syntaxCode);
        // Set all prarmeters (create does not set, just determines factory)
		prg->setType(gptype);
		prg->setSyntaxCode(syntaxCode);
		prg->setSourceFile(filename);
        return prg;
    }
    //---------------------------------------------------------------------------
	GpuProgramPtr GpuProgramManager::createProgramFromString(const String& name, 
		const String& groupName, const String& code, GpuProgramType gptype, 
		const String& syntaxCode)
    {
		GpuProgramPtr prg = create(name, groupName, gptype, syntaxCode);
        // Set all prarmeters (create does not set, just determines factory)
		prg->setType(gptype);
		prg->setSyntaxCode(syntaxCode);
		prg->setSource(code);
        return prg;
    }
    //---------------------------------------------------------------------------
		const GpuProgramManager::SyntaxCodes& GpuProgramManager::getSupportedSyntax(void) const
        {
				// Use the current render system
			  RenderSystem* rs = Root::getSingleton().getRenderSystem();

				// Get the supported syntaxed from RenderSystemCapabilities 
				return rs->getCapabilities()->getSupportedShaderProfiles();
        }

    //---------------------------------------------------------------------------
    bool GpuProgramManager::isSyntaxSupported(const String& syntaxCode) const
        {
				// Use the current render system
			  RenderSystem* rs = Root::getSingleton().getRenderSystem();

				// Get the supported syntaxed from RenderSystemCapabilities 
				return rs->getCapabilities()->isShaderProfileSupported(syntaxCode);

    }
    //---------------------------------------------------------------------------
    ResourcePtr GpuProgramManager::getByName(const String& name, bool preferHighLevelPrograms)
    {
        ResourcePtr ret;
        if (preferHighLevelPrograms)
        {
            ret = HighLevelGpuProgramManager::getSingleton().getByName(name);
            if (!ret.isNull())
                return ret;
        }
        return ResourceManager::getByName(name);
    }
	//-----------------------------------------------------------------------------
	GpuProgramParametersSharedPtr GpuProgramManager::createParameters(void)
	{
		return GpuProgramParametersSharedPtr(OGRE_NEW GpuProgramParameters());
	}

}
