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

#include "OgreParticleSystemManager.h"
#include "OgreParticleEmitterFactory.h"
#include "OgreParticleAffectorFactory.h"
#include "OgreException.h"
#include "OgreRoot.h"
#include "OgreLogManager.h"
#include "OgreString.h"
#include "OgreParticleSystemRenderer.h"
#include "OgreBillboardParticleRenderer.h"
#include "OgreStringConverter.h"
#if OGRE_USE_NEW_COMPILERS == 1
#  include "OgreScriptCompiler.h"
#endif

namespace Ogre {
    //-----------------------------------------------------------------------
    // Shortcut to set up billboard particle renderer
    BillboardParticleRendererFactory* mBillboardRendererFactory = 0;
    //-----------------------------------------------------------------------
    template<> ParticleSystemManager* Singleton<ParticleSystemManager>::ms_Singleton = 0;
    ParticleSystemManager* ParticleSystemManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    ParticleSystemManager& ParticleSystemManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
    //-----------------------------------------------------------------------
    ParticleSystemManager::ParticleSystemManager()
    {
		OGRE_LOCK_AUTO_MUTEX
#if OGRE_USE_NEW_COMPILERS == 0
        mScriptPatterns.push_back("*.particle");
#endif
        ResourceGroupManager::getSingleton()._registerScriptLoader(this);
		mFactory = OGRE_NEW ParticleSystemFactory();
		Root::getSingleton().addMovableObjectFactory(mFactory);
    }
    //-----------------------------------------------------------------------
    ParticleSystemManager::~ParticleSystemManager()
    {
		OGRE_LOCK_AUTO_MUTEX

        // Destroy all templates
        ParticleTemplateMap::iterator t;
        for (t = mSystemTemplates.begin(); t != mSystemTemplates.end(); ++t)
        {
            OGRE_DELETE t->second;
        }
        mSystemTemplates.clear();
        ResourceGroupManager::getSingleton()._unregisterScriptLoader(this);
        // delete billboard factory
        if (mBillboardRendererFactory)
		{
            OGRE_DELETE mBillboardRendererFactory;
			mBillboardRendererFactory = 0;
		}

		if (mFactory)
		{
			// delete particle system factory
			Root::getSingleton().removeMovableObjectFactory(mFactory);
			OGRE_DELETE mFactory;
			mFactory = 0;
		}

    }
    //-----------------------------------------------------------------------
    const StringVector& ParticleSystemManager::getScriptPatterns(void) const
    {
        return mScriptPatterns;
    }
    //-----------------------------------------------------------------------
    Real ParticleSystemManager::getLoadingOrder(void) const
    {
        /// Load late
        return 1000.0f;
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::parseScript(DataStreamPtr& stream, const String& groupName)
    {
#if OGRE_USE_NEW_COMPILERS == 1
		ScriptCompilerManager::getSingleton().parseScript(stream, groupName);
#else // OGRE_USE_NEW_COMPILERS
        String line;
        ParticleSystem* pSys;
        std::vector<String> vecparams;

        pSys = 0;

        while(!stream->eof())
        {
            line = stream->getLine();
            // Ignore comments & blanks
            if (!(line.length() == 0 || line.substr(0,2) == "//"))
            {
                if (pSys == 0)
                {
                    // No current system
                    // So first valid data should be a system name
					if (StringUtil::startsWith(line, "particle_system "))
					{
						// chop off the 'particle_system ' needed by new compilers
						line = line.substr(16);
					}
                    pSys = createTemplate(line, groupName);
					pSys->_notifyOrigin(stream->getName());
                    // Skip to and over next {
                    skipToNextOpenBrace(stream);
                }
                else
                {
                    // Already in a system
                    if (line == "}")
                    {
                        // Finished system
                        pSys = 0;
                    }
                    else if (line.substr(0,7) == "emitter")
                    {
                        // new emitter
                        // Get typename
                        vecparams = StringUtil::split(line, "\t ");
                        if (vecparams.size() < 2)
                        {
                            // Oops, bad emitter
                            LogManager::getSingleton().logMessage("Bad particle system emitter line: '"
                                + line + "' in " + pSys->getName());
                            skipToNextCloseBrace(stream);

                        }
                        skipToNextOpenBrace(stream);
                        parseNewEmitter(vecparams[1], stream, pSys);

                    }
                    else if (line.substr(0,8) == "affector")
                    {
                        // new affector
                        // Get typename
                        vecparams = StringUtil::split(line, "\t ");
                        if (vecparams.size() < 2)
                        {
                            // Oops, bad affector
                            LogManager::getSingleton().logMessage("Bad particle system affector line: '"
                                + line + "' in " + pSys->getName());
                            skipToNextCloseBrace(stream);

                        }
                        skipToNextOpenBrace(stream);
                        parseNewAffector(vecparams[1],stream, pSys);
                    }
                    else
                    {
                        // Attribute
                        parseAttrib(line, pSys);
                    }

                }

            }


        }
#endif // OGRE_USE_NEW_COMPILERS
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::addEmitterFactory(ParticleEmitterFactory* factory)
    {
		OGRE_LOCK_AUTO_MUTEX
        String name = factory->getName();
        mEmitterFactories[name] = factory;
        LogManager::getSingleton().logMessage("Particle Emitter Type '" + name + "' registered");
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::addAffectorFactory(ParticleAffectorFactory* factory)
    {
		OGRE_LOCK_AUTO_MUTEX
        String name = factory->getName();
        mAffectorFactories[name] = factory;
        LogManager::getSingleton().logMessage("Particle Affector Type '" + name + "' registered");
    }
	//-----------------------------------------------------------------------
	void ParticleSystemManager::addRendererFactory(ParticleSystemRendererFactory* factory)
	{
		OGRE_LOCK_AUTO_MUTEX 
		String name = factory->getType();
        mRendererFactories[name] = factory;
        LogManager::getSingleton().logMessage("Particle Renderer Type '" + name + "' registered");
	}
	//-----------------------------------------------------------------------
    void ParticleSystemManager::addTemplate(const String& name, ParticleSystem* sysTemplate)
    {
		OGRE_LOCK_AUTO_MUTEX
		// check name
		if (mSystemTemplates.find(name) != mSystemTemplates.end())
		{
			OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, 
				"ParticleSystem template with name '" + name + "' already exists.", 
				"ParticleSystemManager::addTemplate");
		}

        mSystemTemplates[name] = sysTemplate;
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::removeTemplate(const String& name, bool deleteTemplate)
    {
		OGRE_LOCK_AUTO_MUTEX
        ParticleTemplateMap::iterator itr = mSystemTemplates.find(name);
        if (itr == mSystemTemplates.end())
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
                "ParticleSystem template with name '" + name + "' cannot be found.",
                "ParticleSystemManager::removeTemplate");

        if (deleteTemplate)
            OGRE_DELETE itr->second;

        mSystemTemplates.erase(itr);
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::removeAllTemplates(bool deleteTemplate)
    {
		OGRE_LOCK_AUTO_MUTEX
        if (deleteTemplate)
        {
            ParticleTemplateMap::iterator itr;
            for (itr = mSystemTemplates.begin(); itr != mSystemTemplates.end(); ++itr)
                OGRE_DELETE itr->second;
        }

        mSystemTemplates.clear();
    }
    //-----------------------------------------------------------------------
    ParticleSystem* ParticleSystemManager::createTemplate(const String& name, 
        const String& resourceGroup)
    {
		OGRE_LOCK_AUTO_MUTEX
		// check name
		if (mSystemTemplates.find(name) != mSystemTemplates.end())
		{
			OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, 
				"ParticleSystem template with name '" + name + "' already exists.", 
				"ParticleSystemManager::createTemplate");
		}

        ParticleSystem* tpl = OGRE_NEW ParticleSystem(name, resourceGroup);
        addTemplate(name, tpl);
        return tpl;

    }
    //-----------------------------------------------------------------------
    ParticleSystem* ParticleSystemManager::getTemplate(const String& name)
    {
		OGRE_LOCK_AUTO_MUTEX
        ParticleTemplateMap::iterator i = mSystemTemplates.find(name);
        if (i != mSystemTemplates.end())
        {
            return i->second;
        }
        else
        {
            return 0;
        }
    }
	//-----------------------------------------------------------------------
    ParticleSystem* ParticleSystemManager::createSystemImpl(const String& name,
		size_t quota, const String& resourceGroup)
    {
        ParticleSystem* sys = OGRE_NEW ParticleSystem(name, resourceGroup);
        sys->setParticleQuota(quota);
        return sys;
    }
    //-----------------------------------------------------------------------
    ParticleSystem* ParticleSystemManager::createSystemImpl(const String& name, 
		const String& templateName)
    {
        // Look up template
        ParticleSystem* pTemplate = getTemplate(templateName);
        if (!pTemplate)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot find required template '" + templateName + "'", "ParticleSystemManager::createSystem");
        }

        ParticleSystem* sys = createSystemImpl(name, pTemplate->getParticleQuota(), 
            pTemplate->getResourceGroupName());
        // Copy template settings
        *sys = *pTemplate;
        return sys;
        
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::destroySystemImpl(ParticleSystem* sys)
	{
		OGRE_DELETE sys;
	}
    //-----------------------------------------------------------------------
    ParticleEmitter* ParticleSystemManager::_createEmitter(
        const String& emitterType, ParticleSystem* psys)
    {
		OGRE_LOCK_AUTO_MUTEX
        // Locate emitter type
        ParticleEmitterFactoryMap::iterator pFact = mEmitterFactories.find(emitterType);

        if (pFact == mEmitterFactories.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot find requested emitter type.", 
                "ParticleSystemManager::_createEmitter");
        }

        return pFact->second->createEmitter(psys);
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::_destroyEmitter(ParticleEmitter* emitter)
    {
		OGRE_LOCK_AUTO_MUTEX
        // Destroy using the factory which created it
        ParticleEmitterFactoryMap::iterator pFact = mEmitterFactories.find(emitter->getType());

        if (pFact == mEmitterFactories.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot find emitter factory to destroy emitter.", 
                "ParticleSystemManager::_destroyEmitter");
        }

        pFact->second->destroyEmitter(emitter);
    }
    //-----------------------------------------------------------------------
    ParticleAffector* ParticleSystemManager::_createAffector(
        const String& affectorType, ParticleSystem* psys)
    {
		OGRE_LOCK_AUTO_MUTEX
        // Locate affector type
        ParticleAffectorFactoryMap::iterator pFact = mAffectorFactories.find(affectorType);

        if (pFact == mAffectorFactories.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot find requested affector type.", 
                "ParticleSystemManager::_createAffector");
        }

        return pFact->second->createAffector(psys);

    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::_destroyAffector(ParticleAffector* affector)
    {
		OGRE_LOCK_AUTO_MUTEX
        // Destroy using the factory which created it
        ParticleAffectorFactoryMap::iterator pFact = mAffectorFactories.find(affector->getType());

        if (pFact == mAffectorFactories.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot find affector factory to destroy affector.", 
                "ParticleSystemManager::_destroyAffector");
        }

        pFact->second->destroyAffector(affector);
    }
    //-----------------------------------------------------------------------
    ParticleSystemRenderer* ParticleSystemManager::_createRenderer(const String& rendererType)
	{
		OGRE_LOCK_AUTO_MUTEX
        // Locate affector type
        ParticleSystemRendererFactoryMap::iterator pFact = mRendererFactories.find(rendererType);

        if (pFact == mRendererFactories.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot find requested renderer type.", 
                "ParticleSystemManager::_createRenderer");
        }

        return pFact->second->createInstance(rendererType);
	}
	//-----------------------------------------------------------------------
    void ParticleSystemManager::_destroyRenderer(ParticleSystemRenderer* renderer)
	{
		OGRE_LOCK_AUTO_MUTEX
        // Destroy using the factory which created it
        ParticleSystemRendererFactoryMap::iterator pFact = mRendererFactories.find(renderer->getType());

        if (pFact == mRendererFactories.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot find renderer factory to destroy renderer.", 
                "ParticleSystemManager::_destroyRenderer");
        }

        pFact->second->destroyInstance(renderer);
	}
    //-----------------------------------------------------------------------
    void ParticleSystemManager::_initialise(void)
    {
		OGRE_LOCK_AUTO_MUTEX
        // Create Billboard renderer factory
        mBillboardRendererFactory = OGRE_NEW BillboardParticleRendererFactory();
        addRendererFactory(mBillboardRendererFactory);

    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::parseNewEmitter(const String& type, DataStreamPtr& stream, ParticleSystem* sys)
    {
        // Create new emitter
        ParticleEmitter* pEmit = sys->addEmitter(type);
        // Parse emitter details
        String line;

        while(!stream->eof())
        {
            line = stream->getLine();
            // Ignore comments & blanks
            if (!(line.length() == 0 || line.substr(0,2) == "//"))
            {
                if (line == "}")
                {
                    // Finished emitter
                    break;
                }
                else
                {
                    // Attribute
					StringUtil::toLowerCase(line);
                    parseEmitterAttrib(line, pEmit);
                }
            }
        }


        
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::parseNewAffector(const String& type, DataStreamPtr& stream, ParticleSystem* sys)
    {
        // Create new affector
        ParticleAffector* pAff = sys->addAffector(type);
        // Parse affector details
        String line;

        while(!stream->eof())
        {
            line = stream->getLine();
            // Ignore comments & blanks
            if (!(line.length() == 0 || line.substr(0,2) == "//"))
            {
                if (line == "}")
                {
                    // Finished affector
                    break;
                }
                else
                {
                    // Attribute
					StringUtil::toLowerCase(line);
                    parseAffectorAttrib(line, pAff);
                }
            }
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::parseAttrib(const String& line, ParticleSystem* sys)
    {
        // Split params on space
        std::vector<String> vecparams = StringUtil::split(line, "\t ", 1);

        // Look up first param (command setting)
        if (!sys->setParameter(vecparams[0], vecparams[1]))
        {
            // Attribute not supported by particle system, try the renderer
            ParticleSystemRenderer* renderer = sys->getRenderer();
            if (renderer)
            {
                if (!renderer->setParameter(vecparams[0], vecparams[1]))
                {
                    LogManager::getSingleton().logMessage("Bad particle system attribute line: '"
                        + line + "' in " + sys->getName() + " (tried renderer)");
                }
            }
            else
            {
                // BAD command. BAD!
                LogManager::getSingleton().logMessage("Bad particle system attribute line: '"
                    + line + "' in " + sys->getName() + " (no renderer)");
            }
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::parseEmitterAttrib(const String& line, ParticleEmitter* emit)
    {
        // Split params on first space
        std::vector<String> vecparams = StringUtil::split(line, "\t ", 1);

        // Look up first param (command setting)
        if (!emit->setParameter(vecparams[0], vecparams[1]))
        {
            // BAD command. BAD!
            LogManager::getSingleton().logMessage("Bad particle emitter attribute line: '"
                + line + "' for emitter " + emit->getType());
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::parseAffectorAttrib(const String& line, ParticleAffector* aff)
    {
        // Split params on space
        std::vector<String> vecparams = StringUtil::split(line, "\t ", 1);

        // Look up first param (command setting)
        if (!aff->setParameter(vecparams[0], vecparams[1]))
        {
            // BAD command. BAD!
            LogManager::getSingleton().logMessage("Bad particle affector attribute line: '"
                + line + "' for affector " + aff->getType());
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::skipToNextCloseBrace(DataStreamPtr& stream)
    {
        String line;
        while (!stream->eof() && line != "}")
        {
            line = stream->getLine();
        }

    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::skipToNextOpenBrace(DataStreamPtr& stream)
    {
        String line;
        while (!stream->eof() && line != "{")
        {
            line = stream->getLine();
        }

    }
	//-----------------------------------------------------------------------
	ParticleSystemManager::ParticleAffectorFactoryIterator 
	ParticleSystemManager::getAffectorFactoryIterator(void)
	{
		return ParticleAffectorFactoryIterator(
			mAffectorFactories.begin(), mAffectorFactories.end());
	}
	//-----------------------------------------------------------------------
	ParticleSystemManager::ParticleEmitterFactoryIterator 
	ParticleSystemManager::getEmitterFactoryIterator(void)
	{
		return ParticleEmitterFactoryIterator(
			mEmitterFactories.begin(), mEmitterFactories.end());
	}
	//-----------------------------------------------------------------------
	ParticleSystemManager::ParticleRendererFactoryIterator 
	ParticleSystemManager::getRendererFactoryIterator(void)
	{
		return ParticleRendererFactoryIterator(
			mRendererFactories.begin(), mRendererFactories.end());
	}
	//-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
	String ParticleSystemFactory::FACTORY_TYPE_NAME = "ParticleSystem";
    //-----------------------------------------------------------------------
	MovableObject* ParticleSystemFactory::createInstanceImpl( const String& name, 
			const NameValuePairList* params)
	{
		if (params != 0)
		{
			NameValuePairList::const_iterator ni = params->find("templateName");
			if (ni != params->end())
			{
				String templateName = ni->second;
				// create using manager
				return ParticleSystemManager::getSingleton().createSystemImpl(
						name, templateName);
			}
		}
		// Not template based, look for quota & resource name
		size_t quota = 500;
		String resourceGroup = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME;
		if (params != 0)
		{
			NameValuePairList::const_iterator ni = params->find("quota");
			if (ni != params->end())
			{
				quota = StringConverter::parseUnsignedInt(ni->second);
			}
			ni = params->find("resourceGroup");
			if (ni != params->end())
			{
				resourceGroup = ni->second;
			}
		}
		// create using manager
		return ParticleSystemManager::getSingleton().createSystemImpl(
				name, quota, resourceGroup);
				

	}
    //-----------------------------------------------------------------------
	const String& ParticleSystemFactory::getType(void) const
	{
		return FACTORY_TYPE_NAME;
	}
    //-----------------------------------------------------------------------
	void ParticleSystemFactory::destroyInstance( MovableObject* obj) 
	{
		// use manager
		ParticleSystemManager::getSingleton().destroySystemImpl(
			static_cast<ParticleSystem*>(obj));

	}
    //-----------------------------------------------------------------------
}
