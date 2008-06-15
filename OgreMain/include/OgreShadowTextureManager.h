/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License (LGPL) as 
published by the Free Software Foundation; either version 2.1 of the 
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public 
License for more details.

You should have received a copy of the GNU Lesser General Public License 
along with this library; if not, write to the Free Software Foundation, 
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA or go to
http://www.gnu.org/copyleft/lesser.txt

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-------------------------------------------------------------------------*/
#ifndef __ShadowTextureManager_H__
#define __ShadowTextureManager_H__

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreSingleton.h"
#include "OgrePixelFormat.h"
#include "OgreTexture.h"
#include "OgreIteratorWrappers.h"


namespace Ogre
{
	typedef std::vector<TexturePtr> ShadowTextureList;

	/** Structure containing the configuration for one shadow texture. */
	struct ShadowTextureConfig
	{
		unsigned int width;
		unsigned int height;
		PixelFormat format;

		ShadowTextureConfig()
			: width(512), height(512), format(PF_X8R8G8B8) {}
	};

	typedef std::vector<ShadowTextureConfig> ShadowTextureConfigList;
	typedef ConstVectorIterator<ShadowTextureConfigList> ConstShadowTextureConfigIterator;

	inline _OgreExport bool operator== ( const ShadowTextureConfig& lhs, const ShadowTextureConfig& rhs );
	inline _OgreExport bool operator!= ( const ShadowTextureConfig& lhs, const ShadowTextureConfig& rhs );


	/** Class to manage the available shadow textures which may be shared between
		many SceneManager instances if formats agree.
	@remarks
		The management of the list of shadow textures has been separated out into
		a dedicated class to enable the clean management of shadow textures
		across many scene manager instances. Where multiple scene managers are
		used with shadow textures, the configuration of those shadows may or may
		not be consistent - if it is, it is good to centrally manage the textures
		so that creation and destruction responsibility is clear.
	*/
	class _OgreExport ShadowTextureManager : public Singleton<ShadowTextureManager>, public ShadowDataAlloc
	{
	protected:
		ShadowTextureList mTextureList;
		ShadowTextureList mNullTextureList;
		size_t mCount;

	public:
		ShadowTextureManager();
		virtual ~ShadowTextureManager();

		/** Populate an incoming list with shadow texture references as requested
			in the configuration list.
		*/
		virtual void getShadowTextures(const ShadowTextureConfigList& config, 
			ShadowTextureList& listToPopulate);

		/** Get an appropriately defined 'null' texture, i.e. one which will always
			result in no shadows.
		*/
		virtual TexturePtr getNullShadowTexture(PixelFormat format);

		/** Remove any shadow textures that are no longer being referenced.
		@remarks
			This should be called fairly regularly since references may take a 
			little while to disappear in some cases (if referenced by materials)
		*/
		virtual void clearUnused();
		/** Dereference all the shadow textures kept in this class and remove them
			from TextureManager; note that it is up to the SceneManagers to clear 
			their local references.
		*/
		virtual void clear();

        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static ShadowTextureManager& getSingleton(void);
        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static ShadowTextureManager* getSingletonPtr(void);

	};

}


#endif

