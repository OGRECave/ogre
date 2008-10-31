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
#ifndef __D3D9TEXTUREMANAGER_H__
#define __D3D9TEXTUREMANAGER_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreTextureManager.h"

#include <d3d9.h>

// If we're using the GCC 3.1 C++ Std lib
#if OGRE_COMPILER == OGRE_COMPILER_GNUC && OGRE_COMP_VER >= 310 && !defined(STLPORT)
// We need to define a hash function for D3D9Texture*
// For gcc 4.3 see http://gcc.gnu.org/gcc-4.3/changes.html
#   if OGRE_COMP_VER >= 430
#       include <backward/hash_map>
#   else
#       include <ext/hash_map>
#   endif
namespace Ogre
{
    class D3D9Texture;
}
namespace __gnu_cxx
{
	template <> struct hash< Ogre::D3D9Texture* >
	{
		size_t operator()( Ogre::D3D9Texture* const & ptr ) const
		{
            hash<char*> H;
            return H((char*)ptr);
		}
	};
}

#endif

namespace Ogre 
{
	class D3D9TextureManager : public TextureManager
	{
	protected:
		LPDIRECT3DDEVICE9 mpD3DDevice;
        /// @copydoc ResourceManager::createImpl
        Resource* createImpl(const String& name, ResourceHandle handle, 
            const String& group, bool isManual, ManualResourceLoader* loader, 
            const NameValuePairList* createParams);

		typedef HashSet<D3D9Texture*> D3D9TextureSet;
		/** Record of all textures that exist, irrespective of visibility from
		ResourceManager. When dealing with lost devices, we need to deal with
		all textures that exist, even if they're unreferenced from the resource
		manager's list but still referenced somewhere else. So keep another
		log of them here.
		*/
		D3D9TextureSet mAllTextures;
		OGRE_MUTEX(mAllTexturesMutex)


	public:
		D3D9TextureManager( LPDIRECT3DDEVICE9 pD3DDevice );
		~D3D9TextureManager();

		/** Release all textures in the default memory pool. 
		@remarks
			Method for dealing with lost devices.
		*/
		void releaseDefaultPoolResources(void);
		/** Recreate all textures in the default memory pool. 
		@remarks
			Method for dealing with lost devices.
		*/
		void recreateDefaultPoolResources(void);

		/// @copydoc TextureManager::getNativeFormat
		PixelFormat getNativeFormat(TextureType ttype, PixelFormat format, int usage);

        /// @copydoc TextureManager::isHardwareFilteringSupported
        bool isHardwareFilteringSupported(TextureType ttype, PixelFormat format, int usage,
            bool preciseFormatOnly = false);

		/// Internal method to capture actual destruction (rather than removal)
		void _notifyDestroyed(D3D9Texture*);

	};
}
#endif
