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
#include "OgreMaterialManager.h"

#include "OgreMaterial.h"
#include "OgreStringVector.h"
#include "OgreLogManager.h"
#include "OgreArchive.h"
#include "OgreStringConverter.h"
#include "OgreBlendMode.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreTextureUnitState.h"
#include "OgreException.h"
#if OGRE_USE_NEW_COMPILERS == 1
#  include "OgreScriptCompiler.h"
#endif

namespace Ogre {

    //-----------------------------------------------------------------------
    template<> MaterialManager* Singleton<MaterialManager>::ms_Singleton = 0;
    MaterialManager* MaterialManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    MaterialManager& MaterialManager::getSingleton(void)
    {
        assert( ms_Singleton );  return ( *ms_Singleton );
    }
	String MaterialManager::DEFAULT_SCHEME_NAME = "Default";
    //-----------------------------------------------------------------------
    MaterialManager::MaterialManager()
    {
	    mDefaultMinFilter = FO_LINEAR;
	    mDefaultMagFilter = FO_LINEAR;
	    mDefaultMipFilter = FO_POINT;
		mDefaultMaxAniso = 1;

		// Create primary thread copies of script compiler / serializer
		// other copies for other threads may also be instantiated
		OGRE_THREAD_POINTER_SET(mSerializer, OGRE_NEW MaterialSerializer());

        // Loading order
        mLoadOrder = 100.0f;
		// Scripting is supported by this manager
#if OGRE_USE_NEW_COMPILERS == 0
		mScriptPatterns.push_back("*.program");
		mScriptPatterns.push_back("*.material");
		ResourceGroupManager::getSingleton()._registerScriptLoader(this);
#endif

		// Resource type
		mResourceType = "Material";

		// Register with resource group manager
		ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);

		// Default scheme
		mActiveSchemeIndex = 0;
		mActiveSchemeName = DEFAULT_SCHEME_NAME;
		mSchemes[mActiveSchemeName] = 0;

    }
    //-----------------------------------------------------------------------
    MaterialManager::~MaterialManager()
    {
        mDefaultSettings.setNull();
	    // Resources cleared by superclass
		// Unregister with resource group manager
		ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);
		ResourceGroupManager::getSingleton()._unregisterScriptLoader(this);

		// delete primary thread instances directly, other threads will delete
		// theirs automatically when the threads end (part of boost::thread_specific_ptr)
		OGRE_THREAD_POINTER_DELETE(mSerializer);

    }
	//-----------------------------------------------------------------------
	Resource* MaterialManager::createImpl(const String& name, ResourceHandle handle,
		const String& group, bool isManual, ManualResourceLoader* loader,
        const NameValuePairList* params)
	{
		return OGRE_NEW Material(this, name, handle, group, isManual, loader);
	}
    //-----------------------------------------------------------------------
	void MaterialManager::initialise(void)
	{
		// Set up default material - don't use name contructor as we want to avoid applying defaults
		mDefaultSettings = create("DefaultSettings", ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
        // Add a single technique and pass, non-programmable
        mDefaultSettings->createTechnique()->createPass();

	    // Set up a lit base white material
	    create("BaseWhite", ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
	    // Set up an unlit base white material
        MaterialPtr baseWhiteNoLighting = create("BaseWhiteNoLighting",
			ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
        baseWhiteNoLighting->setLightingEnabled(false);

	}
    //-----------------------------------------------------------------------
    void MaterialManager::parseScript(DataStreamPtr& stream, const String& groupName)
    {
#if OGRE_USE_NEW_COMPILERS == 1
		ScriptCompilerManager::getSingleton().parseScript(stream, groupName);
#else // OGRE_USE_NEW_COMPILERS
#  if OGRE_THREAD_SUPPORT
		// Delegate to serializer
		// check we have an instance for this thread (should always have one for main thread)
		if (!mSerializer.get())
		{
			// create a new instance for this thread - will get deleted when
			// the thread dies
			mSerializer.reset(OGRE_NEW MaterialSerializer());
		}
#  endif
        mSerializer->parseScript(stream, groupName);
#endif // OGRE_USE_NEW_COMPILERS

    }
    //-----------------------------------------------------------------------
	void MaterialManager::setDefaultTextureFiltering(TextureFilterOptions fo)
	{
        switch (fo)
        {
        case TFO_NONE:
            setDefaultTextureFiltering(FO_POINT, FO_POINT, FO_NONE);
            break;
        case TFO_BILINEAR:
            setDefaultTextureFiltering(FO_LINEAR, FO_LINEAR, FO_POINT);
            break;
        case TFO_TRILINEAR:
            setDefaultTextureFiltering(FO_LINEAR, FO_LINEAR, FO_LINEAR);
            break;
        case TFO_ANISOTROPIC:
            setDefaultTextureFiltering(FO_ANISOTROPIC, FO_ANISOTROPIC, FO_LINEAR);
            break;
        }
	}
    //-----------------------------------------------------------------------
	void MaterialManager::setDefaultAnisotropy(unsigned int maxAniso)
	{
		mDefaultMaxAniso = maxAniso;
	}
    //-----------------------------------------------------------------------
	unsigned int MaterialManager::getDefaultAnisotropy() const
	{
		return mDefaultMaxAniso;
	}
    //-----------------------------------------------------------------------
    void MaterialManager::setDefaultTextureFiltering(FilterType ftype, FilterOptions opts)
    {
        switch (ftype)
        {
        case FT_MIN:
            mDefaultMinFilter = opts;
            break;
        case FT_MAG:
            mDefaultMagFilter = opts;
            break;
        case FT_MIP:
            mDefaultMipFilter = opts;
            break;
        }
    }
    //-----------------------------------------------------------------------
    void MaterialManager::setDefaultTextureFiltering(FilterOptions minFilter,
        FilterOptions magFilter, FilterOptions mipFilter)
    {
        mDefaultMinFilter = minFilter;
        mDefaultMagFilter = magFilter;
        mDefaultMipFilter = mipFilter;
    }
    //-----------------------------------------------------------------------
    FilterOptions MaterialManager::getDefaultTextureFiltering(FilterType ftype) const
    {
        switch (ftype)
        {
        case FT_MIN:
            return mDefaultMinFilter;
        case FT_MAG:
            return mDefaultMagFilter;
        case FT_MIP:
            return mDefaultMipFilter;
        }
        // to keep compiler happy
        return mDefaultMinFilter;
    }
    //-----------------------------------------------------------------------
	unsigned short MaterialManager::_getSchemeIndex(const String& schemeName)
	{
		unsigned short ret = 0;
		SchemeMap::iterator i = mSchemes.find(schemeName);
		if (i != mSchemes.end())
		{
			ret = i->second;
		}
		else
		{
			// Create new
			ret = static_cast<unsigned short>(mSchemes.size());
			mSchemes[schemeName] = ret;
		}
		return ret;

	}
	//-----------------------------------------------------------------------
	const String& MaterialManager::_getSchemeName(unsigned short index)
	{
		for (SchemeMap::iterator i = mSchemes.begin(); i != mSchemes.end(); ++i)
		{
			if (i->second == index)
				return i->first;
		}
		return DEFAULT_SCHEME_NAME;
	}
    //-----------------------------------------------------------------------
	unsigned short MaterialManager::_getActiveSchemeIndex(void) const
	{
		return mActiveSchemeIndex;
	}
    //-----------------------------------------------------------------------
	const String& MaterialManager::getActiveScheme(void) const
	{
		return mActiveSchemeName;
	}
    //-----------------------------------------------------------------------
	void MaterialManager::setActiveScheme(const String& schemeName)
	{
		// Allow the creation of new scheme indexes on demand
		// even if they're not specified in any Technique
		mActiveSchemeIndex = _getSchemeIndex(schemeName);
		mActiveSchemeName = schemeName;
	}
    //-----------------------------------------------------------------------
	void MaterialManager::addListener(Listener* l)
	{
		mListenerList.push_back(l);
	}
	//---------------------------------------------------------------------
	void MaterialManager::removeListener(Listener* l)
	{
		mListenerList.remove(l);
	}
	//---------------------------------------------------------------------
	Technique* MaterialManager::_arbitrateMissingTechniqueForActiveScheme(
		Material* mat, unsigned short lodIndex, const Renderable* rend)
	{
		for (ListenerList::iterator i = mListenerList.begin(); i != mListenerList.end(); ++i)
		{
			Technique* t = (*i)->handleSchemeNotFound(mActiveSchemeIndex, 
				mActiveSchemeName, mat, lodIndex, rend);
			if (t)
				return t;
		}

		return 0;

	}

}
