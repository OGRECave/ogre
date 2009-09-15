/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
	//---------------------------------------------------------------------
	GpuSharedParametersPtr GpuProgramManager::createSharedParameters(const String& name)
	{
		if (mSharedParametersMap.find(name) != mSharedParametersMap.end())
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"The shared parameter set '" + name + "' already exists!", 
				"GpuProgramManager::createSharedParameters");
		}
		GpuSharedParametersPtr ret(OGRE_NEW GpuSharedParameters(name));
		mSharedParametersMap[name] = ret;
		return ret;
	}
	//---------------------------------------------------------------------
	GpuSharedParametersPtr GpuProgramManager::getSharedParameters(const String& name) const
	{
		SharedParametersMap::const_iterator i = mSharedParametersMap.find(name);
		if (i == mSharedParametersMap.end())
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"No shared parameter set with name '" + name + "'!", 
				"GpuProgramManager::createSharedParameters");
		}
		return i->second;
	}
	//---------------------------------------------------------------------
	const GpuProgramManager::SharedParametersMap& 
	GpuProgramManager::getAvailableSharedParameters() const
	{
		return mSharedParametersMap;
	}
	//---------------------------------------------------------------------

}
