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
#ifndef __D3D9TEXTURE_H__
#define __D3D9TEXTURE_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreTexture.h"
#include "OgreRenderTexture.h"
#include "OgreImage.h"
#include "OgreException.h"
#include "OgreD3D9HardwarePixelBuffer.h"
#include "OgreD3D9Resource.h"

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>

namespace Ogre {
	class D3D9Texture : public Texture, public D3D9Resource
	{
	protected:	

		struct TextureResources
		{
			/// 1D/2D normal texture pointer
			IDirect3DTexture9* pNormTex;	
			/// cubic texture pointer
			IDirect3DCubeTexture9* pCubeTex;	
			/// Volume texture
			IDirect3DVolumeTexture9* pVolumeTex;
			/// actual texture pointer
			IDirect3DBaseTexture9* pBaseTex;
			/// Optional FSAA surface
			IDirect3DSurface9* pFSAASurface;			
		};
		
		typedef map<IDirect3DDevice9*, TextureResources*>::type	DeviceToTextureResourcesMap;
		typedef DeviceToTextureResourcesMap::iterator			DeviceToTextureResourcesIterator;

		/// Map between device to texture resources.
		DeviceToTextureResourcesMap	mMapDeviceToTextureResources;


		/// Vector of pointers to subsurfaces
		typedef vector<HardwarePixelBufferSharedPtr>::type SurfaceList;
		SurfaceList	mSurfaceList;
		/// cube texture individual face names
		String							mCubeFaceNames[6];	
		/// The memory pool being used
		D3DPOOL							mD3DPool;
		// Dynamic textures?
		bool                            mDynamicTextures;
		
		/// Is hardware gamma supported (read)?
		bool mHwGammaReadSupported;
		/// Is hardware gamma supported (write)?
		bool mHwGammaWriteSupported;
		D3DMULTISAMPLE_TYPE mFSAAType;
		DWORD mFSAAQuality;
		
        // needed to store data between prepareImpl and loadImpl
        typedef SharedPtr<vector<MemoryDataStreamPtr>::type > LoadedStreams;

		/// internal method, load a cube texture
		void _loadCubeTex(IDirect3DDevice9* d3d9Device, const LoadedStreams &loadedStreams);
		/// internal method, load a normal texture
		void _loadNormTex(IDirect3DDevice9* d3d9Device, const LoadedStreams &loadedStreams);
		/// internal method, load a volume texture
		void _loadVolumeTex(IDirect3DDevice9* d3d9Device, const LoadedStreams &loadedStreams);

		/// internal method, prepare a cube texture
		LoadedStreams _prepareCubeTex();
		/// internal method, prepare a normal texture
		LoadedStreams _prepareNormTex();
		/// internal method, prepare a volume texture
		LoadedStreams _prepareVolumeTex();

		/// internal method, create a blank normal 1D/2D texture		
		void _createNormTex(IDirect3DDevice9* d3d9Device);
		/// internal method, create a blank cube texture		
		void _createCubeTex(IDirect3DDevice9* d3d9Device);
		/// internal method, create a blank cube texture		
		void _createVolumeTex(IDirect3DDevice9* d3d9Device);

		/// internal method, return a D3D pixel format for texture creation
		D3DFORMAT _chooseD3DFormat(IDirect3DDevice9* d3d9Device);

		/// @copydoc Resource::calculateSize
		size_t calculateSize(void) const;
		/// @copydoc Texture::createInternalResources
		void createInternalResources(void);
		/// @copydoc Texture::freeInternalResources
		void freeInternalResources(void);
		/// @copydoc Texture::createInternalResourcesImpl
		void createInternalResourcesImpl(void);
		/// Creates this texture resources on the specified device.
		void createInternalResourcesImpl(IDirect3DDevice9* d3d9Device);
		/// free internal resources
		void freeInternalResourcesImpl(void);
		/// internal method, set Texture class source image protected attributes
		void _setSrcAttributes(unsigned long width, unsigned long height, unsigned long depth, PixelFormat format);
		/// internal method, set Texture class final texture protected attributes
		void _setFinalAttributes(IDirect3DDevice9* d3d9Device, TextureResources* textureResources, 
			unsigned long width, unsigned long height, unsigned long depth, PixelFormat format);
		/// internal method, return the best by hardware supported filter method
		D3DTEXTUREFILTERTYPE _getBestFilterMethod(IDirect3DDevice9* d3d9Device);
		/// internal method, return true if the device/texture combination can use dynamic textures
		bool _canUseDynamicTextures(IDirect3DDevice9* d3d9Device, DWORD srcUsage, D3DRESOURCETYPE srcType, D3DFORMAT srcFormat);
		/// internal method, return true if the device/texture combination can auto gen. mip maps
		bool _canAutoGenMipmaps(IDirect3DDevice9* d3d9Device, DWORD srcUsage, D3DRESOURCETYPE srcType, D3DFORMAT srcFormat);
		/// internal method, return true if the device/texture combination can use hardware gamma
		bool _canUseHardwareGammaCorrection(IDirect3DDevice9* d3d9Device, DWORD srcUsage, D3DRESOURCETYPE srcType, D3DFORMAT srcFormat, bool forwriting);
		
		/// internal method, the cube map face name for the spec. face index
		String _getCubeFaceName(unsigned char face) const
		{ assert(face < 6); return mCubeFaceNames[face]; }
		
		/// internal method, create D3D9HardwarePixelBuffers for every face and
		/// mipmap level. This method must be called after the D3D texture object was created
		void _createSurfaceList(IDirect3DDevice9* d3d9Device, TextureResources* textureResources);

        /// overriden from Resource
        void loadImpl();		 
		/// Loads this texture into the specified device.
		void loadImpl(IDirect3DDevice9* d3d9Device);
        /// overriden from Resource
        void prepareImpl();
        /// overriden from Resource
        void unprepareImpl();

		/// gets the texture resources attached to the given device.
		TextureResources* getTextureResources(IDirect3DDevice9* d3d9Device);

		/// allocates new texture resources structure attached to the given device.
		TextureResources* allocateTextureResources(IDirect3DDevice9* d3d9Device);

		/// creates this texture resources according to the current settings.
		void createTextureResources(IDirect3DDevice9* d3d9Device);

		/// frees the given texture resources.
		void freeTextureResources(IDirect3DDevice9* d3d9Device, TextureResources* textureResources);

        /** Vector of pointers to streams that were pulled from disk by
            prepareImpl  but have yet to be pushed into texture memory
            by loadImpl.  Should be cleared on load and on unprepare.
        */
        LoadedStreams mLoadedStreams;

		friend class D3D9HardwarePixelBuffer;
	public:
		/// constructor 
        D3D9Texture(ResourceManager* creator, const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader);
		/// destructor
		~D3D9Texture();

		/// overridden from Texture
		void copyToTexture( TexturePtr& target );


		/// @copydoc Texture::getBuffer
		HardwarePixelBufferSharedPtr getBuffer(size_t face, size_t mipmap);
		
		/// retrieves a pointer to the actual texture
		IDirect3DBaseTexture9 *getTexture();		
		/// retrieves a pointer to the normal 1D/2D texture
		IDirect3DTexture9 *getNormTexture();
		/// retrieves a pointer to the cube texture
		IDirect3DCubeTexture9 *getCubeTexture();

		/** Indicates whether the hardware gamma is actually enabled and supported. 
		@remarks
			Because hardware gamma might not actually be supported, we need to 
			ignore it sometimes. Because D3D doesn't encode sRGB in the format but
			as a sampler state, and we don't want to change the original requested
			hardware gamma flag (e.g. serialisation) we need another indicator.
		*/
		bool isHardwareGammaReadToBeUsed() const { return mHwGamma && mHwGammaReadSupported; }
					
		/// Will this texture need to be in the default pool?
		bool useDefaultPool();

		// Called immediately after the Direct3D device has been created.
		virtual void notifyOnDeviceCreate(IDirect3DDevice9* d3d9Device);

		// Called before the Direct3D device is going to be destroyed.
		virtual void notifyOnDeviceDestroy(IDirect3DDevice9* d3d9Device);

		// Called immediately after the Direct3D device has entered a lost state.
		virtual void notifyOnDeviceLost(IDirect3DDevice9* d3d9Device);

		// Called immediately after the Direct3D device has been reset
		virtual void notifyOnDeviceReset(IDirect3DDevice9* d3d9Device);

		// Reload this texture.
		void reloadTexture();		
    };

    /** Specialisation of SharedPtr to allow SharedPtr to be assigned to D3D9TexturePtr 
    @note Has to be a subclass since we need operator=.
    We could templatise this instead of repeating per Resource subclass, 
    except to do so requires a form VC6 does not support i.e.
    ResourceSubclassPtr<T> : public SharedPtr<T>
    */
    class D3D9TexturePtr : public SharedPtr<D3D9Texture> 
    {
    public:
        D3D9TexturePtr() : SharedPtr<D3D9Texture>() {}
        explicit D3D9TexturePtr(D3D9Texture* rep) : SharedPtr<D3D9Texture>(rep) {}
        D3D9TexturePtr(const D3D9TexturePtr& r) : SharedPtr<D3D9Texture>(r) {} 
        D3D9TexturePtr(const ResourcePtr& r) : SharedPtr<D3D9Texture>()
        {
			// lock & copy other mutex pointer
            OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
            {
			    OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
			    OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
                pRep = static_cast<D3D9Texture*>(r.getPointer());
                pUseCount = r.useCountPointer();
                if (pUseCount)
                {
                    ++(*pUseCount);
                }
            }
        }
		D3D9TexturePtr(const TexturePtr& r) : SharedPtr<D3D9Texture>()
		{
			*this = r;
		}

        /// Operator used to convert a ResourcePtr to a D3D9TexturePtr
        D3D9TexturePtr& operator=(const ResourcePtr& r)
        {
            if (pRep == static_cast<D3D9Texture*>(r.getPointer()))
                return *this;
            release();
			// lock & copy other mutex pointer
            OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
            {
			    OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
			    OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
                pRep = static_cast<D3D9Texture*>(r.getPointer());
                pUseCount = r.useCountPointer();
                if (pUseCount)
                {
                    ++(*pUseCount);
                }
            }
			else
			{
				// RHS must be a null pointer
				assert(r.isNull() && "RHS must be null if it has no mutex!");
				setNull();
			}
            return *this;
        }
        /// Operator used to convert a TexturePtr to a D3D9TexturePtr
        D3D9TexturePtr& operator=(const TexturePtr& r)
        {
            if (pRep == static_cast<D3D9Texture*>(r.getPointer()))
                return *this;
            release();
			// lock & copy other mutex pointer
            OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
            {
			    OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
			    OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
                pRep = static_cast<D3D9Texture*>(r.getPointer());
                pUseCount = r.useCountPointer();
                if (pUseCount)
                {
                    ++(*pUseCount);
                }
            }
			else
			{
				// RHS must be a null pointer
				assert(r.isNull() && "RHS must be null if it has no mutex!");
				setNull();
			}
            return *this;
        }
    };

    /// RenderTexture implementation for D3D9
    class D3D9RenderTexture : public RenderTexture
    {
    public:
		D3D9RenderTexture(const String &name, D3D9HardwarePixelBuffer *buffer, bool writeGamma, uint fsaa);
        ~D3D9RenderTexture() {}

		virtual void update(bool swap);

		virtual void getCustomAttribute( const String& name, void *pData );

		bool requiresTextureFlipping() const { return false; }

		/// Override needed to deal with FSAA
		void swapBuffers(bool waitForVSync = true);

	};

}

#endif
