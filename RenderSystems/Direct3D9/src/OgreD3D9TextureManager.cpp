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
#include "OgreD3D9TextureManager.h"
#include "OgreD3D9Texture.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreD3D9Mappings.h"
#include "OgreRoot.h"
#include "OgreD3D9RenderSystem.h"

namespace Ogre 
{
	D3D9TextureManager::D3D9TextureManager( LPDIRECT3DDEVICE9 pD3DDevice ) : TextureManager()
	{
		mpD3DDevice = pD3DDevice;
		if( !mpD3DDevice )
			OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Invalid Direct3DDevice passed", "D3D9TextureManager::D3D9TextureManager" );
        // register with group manager
        ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);
	}

	D3D9TextureManager::~D3D9TextureManager()
	{
        // unregister with group manager
        ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);

	}

    Resource* D3D9TextureManager::createImpl(const String& name, 
        ResourceHandle handle, const String& group, bool isManual, 
        ManualResourceLoader* loader, const NameValuePairList* createParams)
    {
        D3D9Texture* ret = new D3D9Texture(this, name, handle, group, isManual, loader, mpD3DDevice); 
		{
			OGRE_LOCK_MUTEX(mAllTexturesMutex)
			mAllTextures.insert(ret);
		}

		return ret;
    }

	void D3D9TextureManager::releaseDefaultPoolResources(void)
	{
		size_t count = 0;

		OGRE_LOCK_MUTEX(mAllTexturesMutex)

		for (D3D9TextureSet::iterator i = mAllTextures.begin(); 
			i != mAllTextures.end(); ++i)
		{
			if ((*i)->releaseIfDefaultPool())
				count++;
		}
		LogManager::getSingleton().logMessage("D3D9TextureManager released:");
		LogManager::getSingleton().logMessage(
			StringConverter::toString(count) + " unmanaged textures");
	}

	void D3D9TextureManager::recreateDefaultPoolResources(void)
	{
		size_t count = 0;

		OGRE_LOCK_MUTEX(mAllTexturesMutex)

		for (D3D9TextureSet::iterator i = mAllTextures.begin(); 
			i != mAllTextures.end(); ++i)
		{
			if((*i)->recreateIfDefaultPool(mpD3DDevice))
				count++;
		}
		LogManager::getSingleton().logMessage("D3D9TextureManager recreated:");
		LogManager::getSingleton().logMessage(
			StringConverter::toString(count) + " unmanaged textures");
	}


	PixelFormat D3D9TextureManager::getNativeFormat(TextureType ttype, PixelFormat format, int usage)
	{
		// Basic filtering
		D3DFORMAT d3dPF = D3D9Mappings::_getPF(D3D9Mappings::_getClosestSupportedPF(format));

		// Calculate usage
		DWORD d3dusage = 0;
		D3DPOOL pool = D3DPOOL_MANAGED;
		if (usage & TU_RENDERTARGET) 
		{
			d3dusage |= D3DUSAGE_RENDERTARGET;
			pool = D3DPOOL_DEFAULT;
		}
		if (usage & TU_DYNAMIC)
		{
			d3dusage |= D3DUSAGE_DYNAMIC;
			pool = D3DPOOL_DEFAULT;
		}

		// Use D3DX to adjust pixel format
		switch(ttype)
		{
		case TEX_TYPE_1D:
		case TEX_TYPE_2D:
			D3DXCheckTextureRequirements(mpD3DDevice, NULL, NULL, NULL, d3dusage, &d3dPF, pool);
			break;
		case TEX_TYPE_3D:
			D3DXCheckVolumeTextureRequirements(mpD3DDevice, NULL, NULL, NULL, NULL, d3dusage, &d3dPF, pool);
			break;
		case TEX_TYPE_CUBE_MAP:
			D3DXCheckCubeTextureRequirements(mpD3DDevice, NULL, NULL, d3dusage, &d3dPF, pool);
			break;
		};

		return D3D9Mappings::_getPF(d3dPF);


	}

    bool D3D9TextureManager::isHardwareFilteringSupported(TextureType ttype, PixelFormat format, int usage,
        bool preciseFormatOnly)
    {
        if (!preciseFormatOnly)
            format = getNativeFormat(ttype, format, usage);

        D3D9RenderSystem* rs = static_cast<D3D9RenderSystem*>(
            Root::getSingleton().getRenderSystem());

        return rs->_checkTextureFilteringSupported(ttype, format, usage);
    }

	void D3D9TextureManager::_notifyDestroyed(D3D9Texture* tex)
	{
		OGRE_LOCK_MUTEX(mAllTexturesMutex)
		mAllTextures.erase(tex);
	}

}
