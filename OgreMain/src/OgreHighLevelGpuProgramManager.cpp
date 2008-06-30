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
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreUnifiedHighLevelGpuProgram.h"

namespace Ogre {

	String sNullLang = "null";
	class NullProgram : public HighLevelGpuProgram
	{
	protected:
		/** Internal load implementation, must be implemented by subclasses.
		*/
		void loadFromSource(void) {}
		/** Internal method for creating an appropriate low-level program from this
		high-level program, must be implemented by subclasses. */
		void createLowLevelImpl(void) {}
		/// Internal unload implementation, must be implemented by subclasses
		void unloadHighLevelImpl(void) {}
		/// Populate the passed parameters with name->index map, must be overridden
		void populateParameterNames(GpuProgramParametersSharedPtr params)
		{
			// Skip the normal implementation
			// Ensure we don't complain about missing parameter names
			params->setIgnoreMissingParams(true);

		}
		void buildConstantDefinitions() const
		{
			// do nothing
		}
	public:
		NullProgram(ResourceManager* creator, 
			const String& name, ResourceHandle handle, const String& group, 
			bool isManual, ManualResourceLoader* loader)
			: HighLevelGpuProgram(creator, name, handle, group, isManual, loader){}
		~NullProgram() {}
		/// Overridden from GpuProgram - never supported
		bool isSupported(void) const { return false; }
		/// Overridden from GpuProgram
		const String& getLanguage(void) const { return sNullLang; }

		/// Overridden from StringInterface
		bool setParameter(const String& name, const String& value)
		{
			// always silently ignore all parameters so as not to report errors on
			// unsupported platforms
			return true;
		}

	};
	class NullProgramFactory : public HighLevelGpuProgramFactory
	{
	public:
		NullProgramFactory() {}
		~NullProgramFactory() {}
		/// Get the name of the language this factory creates programs for
		const String& getLanguage(void) const 
		{ 
			return sNullLang;
		}
		HighLevelGpuProgram* create(ResourceManager* creator, 
			const String& name, ResourceHandle handle,
			const String& group, bool isManual, ManualResourceLoader* loader)
		{
			return OGRE_NEW NullProgram(creator, name, handle, group, isManual, loader);
		}
		void destroy(HighLevelGpuProgram* prog)
		{
			OGRE_DELETE prog;
		}

	};
	//-----------------------------------------------------------------------
	template<> HighLevelGpuProgramManager* 
	Singleton<HighLevelGpuProgramManager>::ms_Singleton = 0;
    HighLevelGpuProgramManager* HighLevelGpuProgramManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    HighLevelGpuProgramManager& HighLevelGpuProgramManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
	//-----------------------------------------------------------------------
	HighLevelGpuProgramManager::HighLevelGpuProgramManager()
	{
        // Loading order
        mLoadOrder = 50.0f;
        // Resource type
        mResourceType = "HighLevelGpuProgram";

        ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);    

		mNullFactory = OGRE_NEW NullProgramFactory();
		addFactory(mNullFactory);
		mUnifiedFactory = OGRE_NEW UnifiedHighLevelGpuProgramFactory();
		addFactory(mUnifiedFactory);
	}
	//-----------------------------------------------------------------------
	HighLevelGpuProgramManager::~HighLevelGpuProgramManager()
	{
		OGRE_DELETE mUnifiedFactory;
		OGRE_DELETE mNullFactory;
        ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);    
	}
    //---------------------------------------------------------------------------
	void HighLevelGpuProgramManager::addFactory(HighLevelGpuProgramFactory* factory)
	{
		// deliberately allow later plugins to override earlier ones
		mFactories[factory->getLanguage()] = factory;
	}
    //---------------------------------------------------------------------------
    void HighLevelGpuProgramManager::removeFactory(HighLevelGpuProgramFactory* factory)
    {
        // Remove only if equal to registered one, since it might overridden
        // by other plugins
        FactoryMap::iterator it = mFactories.find(factory->getLanguage());
        if (it != mFactories.end() && it->second == factory)
        {
            mFactories.erase(it);
        }
    }
    //---------------------------------------------------------------------------
	HighLevelGpuProgramFactory* HighLevelGpuProgramManager::getFactory(const String& language)
	{
		FactoryMap::iterator i = mFactories.find(language);

		if (i == mFactories.end())
		{
			// use the null factory to create programs that will never be supported
			i = mFactories.find(sNullLang);
		}
		return i->second;
	}
    //---------------------------------------------------------------------------
    Resource* HighLevelGpuProgramManager::createImpl(const String& name, ResourceHandle handle, 
        const String& group, bool isManual, ManualResourceLoader* loader,
        const NameValuePairList* params)
    {
        NameValuePairList::const_iterator paramIt;

        if (!params || (paramIt = params->find("language")) == params->end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "You must supply a 'language' parameter",
                "HighLevelGpuProgramManager::createImpl");
        }

        return getFactory(paramIt->second)->create(this, name, getNextHandle(), 
            group, isManual, loader);
    }
    //---------------------------------------------------------------------------
    HighLevelGpuProgramPtr HighLevelGpuProgramManager::createProgram(
			const String& name, const String& groupName, 
            const String& language, GpuProgramType gptype)
    {
        ResourcePtr ret = ResourcePtr(
            getFactory(language)->create(this, name, getNextHandle(), 
            groupName, false, 0));

        HighLevelGpuProgramPtr prg = ret;
        prg->setType(gptype);
        prg->setSyntaxCode(language);

        addImpl(ret);
        // Tell resource group manager
        ResourceGroupManager::getSingleton()._notifyResourceCreated(ret);
        return prg;
    }
    //---------------------------------------------------------------------------
    HighLevelGpuProgramFactory::~HighLevelGpuProgramFactory() 
    {
    }
}
