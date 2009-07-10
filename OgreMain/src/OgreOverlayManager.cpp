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

#include "OgreOverlayManager.h"
#include "OgreStringVector.h"
#include "OgreOverlayContainer.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreSceneManagerEnumerator.h"
#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include "OgreEntity.h"
#include "OgreException.h"
#include "OgreViewport.h"
#include "OgreOverlayElementFactory.h"

namespace Ogre {

    //---------------------------------------------------------------------
    template<> OverlayManager *Singleton<OverlayManager>::ms_Singleton = 0;
    OverlayManager* OverlayManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    OverlayManager& OverlayManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
    //---------------------------------------------------------------------
    OverlayManager::OverlayManager() 
      : mLastViewportWidth(0), 
        mLastViewportHeight(0), 
        mViewportDimensionsChanged(false)
    {

        // Scripting is supported by this manager
        mScriptPatterns.push_back("*.overlay");
		ResourceGroupManager::getSingleton()._registerScriptLoader(this);

    }
    //---------------------------------------------------------------------
    OverlayManager::~OverlayManager()
    {
		destroyAllOverlayElements(false);
		destroyAllOverlayElements(true);
        destroyAll();

        // Unregister with resource group manager
		ResourceGroupManager::getSingleton()._unregisterScriptLoader(this);
    }
    //---------------------------------------------------------------------
    const StringVector& OverlayManager::getScriptPatterns(void) const
    {
        return mScriptPatterns;
    }
    //---------------------------------------------------------------------
    Real OverlayManager::getLoadingOrder(void) const
    {
        // Load late
        return 1100.0f;
    }
    //---------------------------------------------------------------------
    Overlay* OverlayManager::create(const String& name)
    {
        Overlay* ret = 0;
        OverlayMap::iterator i = mOverlayMap.find(name);

        if (i == mOverlayMap.end())
        {
            ret = OGRE_NEW Overlay(name);
            assert(ret && "Overlay creation failed");
            mOverlayMap[name] = ret;
        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, 
                "Overlay with name '" + name + "' already exists!",
                "OverlayManager::create");
        }

        return ret;

    }
    //---------------------------------------------------------------------
    Overlay* OverlayManager::getByName(const String& name)
    {
        OverlayMap::iterator i = mOverlayMap.find(name);
        if (i == mOverlayMap.end())
        {
            return 0;
        }
        else
        {
            return i->second;
        }

    }
    //---------------------------------------------------------------------
    void OverlayManager::destroy(const String& name)
    {
        OverlayMap::iterator i = mOverlayMap.find(name);
        if (i == mOverlayMap.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                "Overlay with name '" + name + "' not found.",
                "OverlayManager::destroy");
        }
        else
        {
            OGRE_DELETE i->second;
            mOverlayMap.erase(i);
        }
    }
    //---------------------------------------------------------------------
    void OverlayManager::destroy(Overlay* overlay)
    {
        for (OverlayMap::iterator i = mOverlayMap.begin();
            i != mOverlayMap.end(); ++i)
        {
            if (i->second == overlay)
            {
                OGRE_DELETE i->second;
                mOverlayMap.erase(i);
                return;
            }
        }

        OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
            "Overlay not found.",
            "OverlayManager::destroy");
    }
    //---------------------------------------------------------------------
    void OverlayManager::destroyAll(void)
    {
        for (OverlayMap::iterator i = mOverlayMap.begin();
            i != mOverlayMap.end(); ++i)
        {
            OGRE_DELETE i->second;
        }
        mOverlayMap.clear();
		mLoadedScripts.clear();
    }
    //---------------------------------------------------------------------
    OverlayManager::OverlayMapIterator OverlayManager::getOverlayIterator(void)
    {
        return OverlayMapIterator(mOverlayMap.begin(), mOverlayMap.end());
    }
    //---------------------------------------------------------------------
    void OverlayManager::parseScript(DataStreamPtr& stream, const String& groupName)
    {
		// check if we've seen this script before (can happen if included 
		// multiple times)
		if (!stream->getName().empty() && 
			mLoadedScripts.find(stream->getName()) != mLoadedScripts.end())
		{
			LogManager::getSingleton().logMessage( 
				"Skipping loading overlay include: '"
				+ stream->getName() + " as it is already loaded.");
			return;
		}
	    String line;
	    Overlay* pOverlay = 0;
		bool skipLine;

	    while(!stream->eof())
	    {
			bool isTemplate = false;
			skipLine = false;
		    line = stream->getLine();
		    // Ignore comments & blanks
		    if (!(line.length() == 0 || line.substr(0,2) == "//"))
		    {
				if (line.substr(0,8) == "#include")
				{
                    vector<String>::type params = StringUtil::split(line, "\t\n ()<>");
                    DataStreamPtr includeStream = 
                        ResourceGroupManager::getSingleton().openResource(
                            params[1], groupName);
					parseScript(includeStream, groupName);
					continue;
				}
			    if (!pOverlay)
			    {
				    // No current overlay

					// check to see if there is a template
					if (line.substr(0,8) == "template")
					{
						isTemplate = true;

					}
					else
					{
			
						// So first valid data should be overlay name
						if (StringUtil::startsWith(line, "overlay "))
						{
							// chop off the 'particle_system ' needed by new compilers
							line = line.substr(8);
						}
						pOverlay = create(line);
						pOverlay->_notifyOrigin(stream->getName());
						// Skip to and over next {
						skipToNextOpenBrace(stream);
						skipLine = true;
					}
			    }
			    if ((pOverlay && !skipLine) || isTemplate)
			    {
				    // Already in overlay
                    vector<String>::type params = StringUtil::split(line, "\t\n ()");


				    if (line == "}")
				    {
					    // Finished overlay
					    pOverlay = 0;
						isTemplate = false;
				    }
				    else if (parseChildren(stream,line, pOverlay, isTemplate, NULL))
						
				    {

				    }
				    else
				    {
					    // Attribute
						if (!isTemplate)
						{
							parseAttrib(line, pOverlay);
						}
				    }

			    }

		    }


	    }

		// record as parsed
		mLoadedScripts.insert(stream->getName());

    }
    //---------------------------------------------------------------------
    void OverlayManager::_queueOverlaysForRendering(Camera* cam, 
        RenderQueue* pQueue, Viewport* vp)
    {
        // Flag for update pixel-based GUIElements if viewport has changed dimensions
        if (mLastViewportWidth != vp->getActualWidth() || 
            mLastViewportHeight != vp->getActualHeight())
        {
            mViewportDimensionsChanged = true;
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
            if ((vp->getOrientation() == Viewport::OR_LANDSCAPELEFT) ||
                (vp->getOrientation() == Viewport::OR_LANDSCAPERIGHT)) {
                mLastViewportWidth = vp->getActualHeight();
                mLastViewportHeight = vp->getActualWidth();
            } else {
                mLastViewportWidth = vp->getActualWidth();
                mLastViewportHeight = vp->getActualHeight();
            }
#else
            mLastViewportWidth = vp->getActualWidth();
            mLastViewportHeight = vp->getActualHeight();
#endif
        }
        else
        {
            mViewportDimensionsChanged = false;
        }

        OverlayMap::iterator i, iend;
        iend = mOverlayMap.end();
        for (i = mOverlayMap.begin(); i != iend; ++i)
        {
            Overlay* o = i->second;
            o->_findVisibleObjects(cam, pQueue);
        }
    }
    //---------------------------------------------------------------------
    void OverlayManager::parseNewElement( DataStreamPtr& stream, String& elemType, String& elemName, 
            bool isContainer, Overlay* pOverlay, bool isTemplate, String templateName, OverlayContainer* container)
    {
        String line;

		OverlayElement* newElement = NULL;
		newElement = 
				OverlayManager::getSingleton().createOverlayElementFromTemplate(templateName, elemType, elemName, isTemplate);

			// do not add a template to an overlay

		// add new element to parent
		if (container)
		{
			// Attach to container
			container->addChild(newElement);
		}
		// do not add a template to the overlay. For templates overlay = 0
		else if (pOverlay)	
		{
			pOverlay->add2D((OverlayContainer*)newElement);
		}

        while(!stream->eof())
        {
            line = stream->getLine();
            // Ignore comments & blanks
            if (!(line.length() == 0 || line.substr(0,2) == "//"))
            {
                if (line == "}")
                {
                    // Finished element
                    break;
                }
                else
                {
                    if (isContainer && parseChildren(stream,line, pOverlay, isTemplate, static_cast<OverlayContainer*>(newElement)))
                    {
					    // nested children... don't reparse it
                    }
                    else
                    {
                        // Attribute
                        parseElementAttrib(line, pOverlay, newElement);
                    }
                }
            }
        }
    }

    //---------------------------------------------------------------------
    bool OverlayManager::parseChildren( DataStreamPtr& stream, const String& line,
            Overlay* pOverlay, bool isTemplate, OverlayContainer* parent)
	{
		bool ret = false;
		uint skipParam =0;
		vector<String>::type params = StringUtil::split(line, "\t\n ()");

		if (isTemplate)
		{
			if (params[0] == "template")
			{
				skipParam++;		// the first param = 'template' on a new child element
			}
		}
						
		// top level component cannot be an element, it must be a container unless it is a template
		if (params[0+skipParam] == "container" || (params[0+skipParam] == "element" && (isTemplate || parent != NULL)) )
		{
			String templateName;
			ret = true;
			// nested container/element
			if (params.size() > 3+skipParam)
			{
				if (params.size() != 5+skipParam)
				{
					LogManager::getSingleton().logMessage( 
						"Bad element/container line: '"
						+ line + "' in " + parent->getTypeName()+ " " + parent->getName() +
						", expecting ':' templateName");
					skipToNextCloseBrace(stream);
					// barf 
					return ret;
				}
				if (params[3+skipParam] != ":")
				{
					LogManager::getSingleton().logMessage( 
						"Bad element/container line: '"
						+ line + "' in " + parent->getTypeName()+ " " + parent->getName() +
						", expecting ':' for element inheritance");
					skipToNextCloseBrace(stream);
					// barf 
					return ret;
				}

				templateName = params[4+skipParam];
			}

			else if (params.size() != 3+skipParam)
			{
				LogManager::getSingleton().logMessage( 
					"Bad element/container line: '"
						+ line + "' in " + parent->getTypeName()+ " " + parent->getName() +
					", expecting 'element type(name)'");
				skipToNextCloseBrace(stream);
				// barf 
				return ret;
			}
       
			skipToNextOpenBrace(stream);
			parseNewElement(stream, params[1+skipParam], params[2+skipParam], true, pOverlay, isTemplate, templateName, (OverlayContainer*)parent);

		}


		return ret;
	}

    //---------------------------------------------------------------------
    void OverlayManager::parseAttrib( const String& line, Overlay* pOverlay)
    {
        // Split params on first space
        vector<String>::type vecparams = StringUtil::split(line, "\t ", 1);

        // Look up first param (command setting)
		StringUtil::toLowerCase(vecparams[0]);
        if (vecparams[0] == "zorder")
        {
            pOverlay->setZOrder(StringConverter::parseUnsignedInt(vecparams[1]));
        }
        else
        {
            LogManager::getSingleton().logMessage("Bad overlay attribute line: '"
                + line + "' for overlay " + pOverlay->getName());
        }
    }
    //---------------------------------------------------------------------
    void OverlayManager::parseElementAttrib( const String& line, Overlay* pOverlay, OverlayElement* pElement )
    {
        // Split params on first space
        vector<String>::type vecparams = StringUtil::split(line, "\t ", 1);

        // Look up first param (command setting)
		StringUtil::toLowerCase(vecparams[0]);
        if (!pElement->setParameter(vecparams[0], vecparams[1]))
        {
            // BAD command. BAD!
            LogManager::getSingleton().logMessage("Bad element attribute line: '"
                + line + "' for element " + pElement->getName() + " in overlay " + 
                (!pOverlay ? StringUtil::BLANK : pOverlay->getName()));
        }
    }
    //-----------------------------------------------------------------------
    void OverlayManager::skipToNextCloseBrace(DataStreamPtr& stream)
    {
        String line;
        while (!stream->eof() && line != "}")
        {
            line = stream->getLine();
        }

    }
    //-----------------------------------------------------------------------
    void OverlayManager::skipToNextOpenBrace(DataStreamPtr& stream)
    {
        String line;
        while (!stream->eof() && line != "{")
        {
            line = stream->getLine();
        }

    }
    //---------------------------------------------------------------------
    bool OverlayManager::hasViewportChanged(void) const
    {
        return mViewportDimensionsChanged;
    }
    //---------------------------------------------------------------------
    int OverlayManager::getViewportHeight(void) const
    {
        return mLastViewportHeight;
    }
    //---------------------------------------------------------------------
    int OverlayManager::getViewportWidth(void) const
    {
        return mLastViewportWidth;
    }
    //---------------------------------------------------------------------
    Real OverlayManager::getViewportAspectRatio(void) const
    {
        return (Real)mLastViewportWidth / (Real)mLastViewportHeight;
    }
    //---------------------------------------------------------------------
	//---------------------------------------------------------------------
	OverlayManager::ElementMap& OverlayManager::getElementMap(bool isTemplate)
	{
		return (isTemplate)?mTemplates:mInstances;
	}

	//---------------------------------------------------------------------
	OverlayElement* OverlayManager::createOverlayElementFromTemplate(const String& templateName, const String& typeName, const String& instanceName, bool isTemplate)
	{

		OverlayElement* newObj  = NULL;

		if (templateName.empty())
		{
			newObj = createOverlayElement(typeName, instanceName, isTemplate);
		}
		else
		{
			// no template 
			OverlayElement* templateGui = getOverlayElement(templateName, true);

			String typeNameToCreate;
			if (typeName.empty())
			{
				typeNameToCreate = templateGui->getTypeName();
			}
			else
			{
				typeNameToCreate = typeName;
			}

			newObj = createOverlayElement(typeNameToCreate, instanceName, isTemplate);

			((OverlayContainer*)newObj)->copyFromTemplate(templateGui);
		}

		return newObj;
	}


	//---------------------------------------------------------------------
	OverlayElement* OverlayManager::cloneOverlayElementFromTemplate(const String& templateName, const String& instanceName)
	{
		OverlayElement* templateGui = getOverlayElement(templateName, true);
		return templateGui->clone(instanceName);
	}

	//---------------------------------------------------------------------
	OverlayElement* OverlayManager::createOverlayElement(const String& typeName, const String& instanceName, bool isTemplate)
	{
		return createOverlayElementImpl(typeName, instanceName, getElementMap(isTemplate));
	}

	//---------------------------------------------------------------------
	OverlayElement* OverlayManager::createOverlayElementImpl(const String& typeName, const String& instanceName, ElementMap& elementMap)
	{
		// Check not duplicated
		ElementMap::iterator ii = elementMap.find(instanceName);
		if (ii != elementMap.end())
		{
			OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, "OverlayElement with name " + instanceName +
				" already exists.", "OverlayManager::createOverlayElement" );
		}
		OverlayElement* newElem = createOverlayElementFromFactory(typeName, instanceName);

		// Register
		elementMap.insert(ElementMap::value_type(instanceName, newElem));

		return newElem;


	}

	//---------------------------------------------------------------------
	OverlayElement* OverlayManager::createOverlayElementFromFactory(const String& typeName, const String& instanceName)
	{
		// Look up factory
		FactoryMap::iterator fi = mFactories.find(typeName);
		if (fi == mFactories.end())
		{
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Cannot locate factory for element type " + typeName,
				"OverlayManager::createOverlayElement");
		}

		// create
		return fi->second->createOverlayElement(instanceName);
	}

	//---------------------------------------------------------------------
	OverlayElement* OverlayManager::getOverlayElement(const String& name, bool isTemplate)
	{
		return getOverlayElementImpl(name, getElementMap(isTemplate));
	}
	//---------------------------------------------------------------------
	OverlayElement* OverlayManager::getOverlayElementImpl(const String& name, ElementMap& elementMap)
	{
		// Locate instance
		ElementMap::iterator ii = elementMap.find(name);
		if (ii == elementMap.end())
		{
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "OverlayElement with name " + name +
				" not found.", "OverlayManager::getOverlayElementImpl" );
		}

		return ii->second;
	}
	//---------------------------------------------------------------------
	void OverlayManager::destroyOverlayElement(const String& instanceName, bool isTemplate)
	{
		destroyOverlayElementImpl(instanceName, getElementMap(isTemplate));
	}

	//---------------------------------------------------------------------
	void OverlayManager::destroyOverlayElement(OverlayElement* pInstance, bool isTemplate)
	{
		destroyOverlayElementImpl(pInstance->getName(), getElementMap(isTemplate));
	}

	//---------------------------------------------------------------------
	void OverlayManager::destroyOverlayElementImpl(const String& instanceName, ElementMap& elementMap)
	{
		// Locate instance
		ElementMap::iterator ii = elementMap.find(instanceName);
		if (ii == elementMap.end())
		{
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "OverlayElement with name " + instanceName +
				" not found.", "OverlayManager::destroyOverlayElement" );
		}
		// Look up factory
		const String& typeName = ii->second->getTypeName();
		FactoryMap::iterator fi = mFactories.find(typeName);
		if (fi == mFactories.end())
		{
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Cannot locate factory for element type " + typeName,
				"OverlayManager::destroyOverlayElement");
		}

		fi->second->destroyOverlayElement(ii->second);
		elementMap.erase(ii);
	}
	//---------------------------------------------------------------------
	void OverlayManager::destroyAllOverlayElements(bool isTemplate)
	{
		destroyAllOverlayElementsImpl(getElementMap(isTemplate));
	}
	//---------------------------------------------------------------------
	void OverlayManager::destroyAllOverlayElementsImpl(ElementMap& elementMap)
	{
		ElementMap::iterator i;

		while ((i = elementMap.begin()) != elementMap.end())
		{
			OverlayElement* element = i->second;

			// Get factory to delete
			FactoryMap::iterator fi = mFactories.find(element->getTypeName());
			if (fi == mFactories.end())
			{
				OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Cannot locate factory for element " 
					+ element->getName(),
					"OverlayManager::destroyAllOverlayElements");
			}

			// remove from parent, if any
			OverlayContainer* parent;
			if ((parent = element->getParent()) != 0)
			{
				parent->_removeChild(element->getName());
			}

			// children of containers will be auto-removed when container is destroyed.
			// destroy the element and remove it from the list
			fi->second->destroyOverlayElement(element);
			elementMap.erase(i);
		}
	}
	//---------------------------------------------------------------------
	void OverlayManager::addOverlayElementFactory(OverlayElementFactory* elemFactory)
	{
		// Add / replace
		mFactories[elemFactory->getTypeName()] = elemFactory;

		LogManager::getSingleton().logMessage("OverlayElementFactory for type " + elemFactory->getTypeName()
			+ " registered.");
	}
}

