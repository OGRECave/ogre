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
#ifndef __D3D8TEXTURE_H__
#define __D3D8TEXTURE_H__

#include "OgreD3D10Prerequisites.h"
#include "OgreTexture.h"
#include "OgreRenderTexture.h"

namespace Ogre {
	class D3D10Texture : public Texture
	{
	protected:
		/// D3DDevice pointer
		D3D10Device	&	mDevice;	


		/// D3D10 pointer
		//LPDIRECT3D10				*mpD3D;
		// 1D texture pointer
		ID3D10Texture1D *mp1DTex;
		// 2D texture pointer
		ID3D10Texture2D *mp2DTex;
		/// cubic texture pointer
		ID3D10Texture3D	*mp3DTex;	
		/// actual texture pointer
		ID3D10Resource 	*mpTex;		

		ID3D10ShaderResourceView* mpShaderResourceView;

		// is dynamic
		bool mIsDynamic; 

		/// cube texture individual face names
		String							mCubeFaceNames[6];
		/// device creation parameters
		//D3DDEVICE_CREATION_PARAMETERS	mDevCreParams;
		/// back buffer pixel format
		DXGI_FORMAT						mBBPixelFormat;
		/// The memory pool being used
		//D3DPOOL							mD3DPool;
		/// device capabilities pointer
		//D3DCAPS9						mDevCaps;
		// Dynamic textures?
		bool                            mDynamicTextures;
		/// Vector of pointers to subsurfaces
		typedef vector<HardwarePixelBufferSharedPtr>::type SurfaceList;
		SurfaceList						mSurfaceList;

		D3D10_SHADER_RESOURCE_VIEW_DESC mSRVDesc;
		/// internal method, load a normal texture
		void _loadTex();

		/// internal method, create a blank normal 1D Dtexture
		void _create1DTex();
		/// internal method, create a blank normal 2D texture
		void _create2DTex();
		/// internal method, create a blank cube texture
		void _create3DTex();

		/// internal method, return a D3D pixel format for texture creation
		DXGI_FORMAT _chooseD3DFormat();

		/// @copydoc Texture::createInternalResourcesImpl
		void createInternalResourcesImpl(void);
		/// free internal resources
		void freeInternalResourcesImpl(void);
		/// internal method, set Texture class source image protected attributes
		void _setSrcAttributes(unsigned long width, unsigned long height, unsigned long depth, PixelFormat format);
		/// internal method, set Texture class final texture protected attributes
		void _setFinalAttributes(unsigned long width, unsigned long height, unsigned long depth, PixelFormat format);

		/// internal method, the cube map face name for the spec. face index
		String _getCubeFaceName(unsigned char face) const
		{ assert(face < 6); return mCubeFaceNames[face]; }

		/// internal method, create D3D10HardwarePixelBuffers for every face and
		/// mipmap level. This method must be called after the D3D texture object was created
		void _createSurfaceList(void);

		/// overriden from Resource
		void loadImpl();
	public:
		/// constructor 
		D3D10Texture(ResourceManager* creator, const String& name, ResourceHandle handle,
			const String& group, bool isManual, ManualResourceLoader* loader, 
			D3D10Device & device);
		/// destructor
		~D3D10Texture();

		/// overriden from Texture
		void copyToTexture( TexturePtr& target );
		/// overriden from Texture
		void loadImage( const Image &img );


		/// @copydoc Texture::getBuffer
		HardwarePixelBufferSharedPtr getBuffer(size_t face, size_t mipmap);

		ID3D10Resource *getTextureResource() 
		{ assert(mpTex); return mpTex; }
		/// retrieves a pointer to the actual texture
		ID3D10ShaderResourceView *getTexture() 
		{ assert(mpShaderResourceView); return mpShaderResourceView; }
		/*/// retrieves a pointer to the normal 1D/2D texture
		IDirect3DTexture9 *getNormTexture()
		{ assert(mpNormTex); return mpNormTex; }
		/// retrieves a pointer to the cube texture
		IDirect3DCubeTexture9 *getCubeTexture()
		{ assert(mpCubeTex); return mpCubeTex; }
		*/


		/// For dealing with lost devices - release the resource if in the default pool (and return true)
		bool releaseIfDefaultPool(void);
		/// For dealing with lost devices - recreate the resource if in the default pool (and return true)
		bool recreateIfDefaultPool(D3D10Device & device);

		ID3D10Texture1D * GetTex1D() {return mp1DTex;};
		ID3D10Texture2D * GetTex2D() {return mp2DTex;};
		ID3D10Texture3D	* GetTex3D() {return mp3DTex;};

		D3D10_SHADER_RESOURCE_VIEW_DESC getShaderResourceViewDesc() const;


	};

	/** Specialisation of SharedPtr to allow SharedPtr to be assigned to D3D10TexturePtr 
	@note Has to be a subclass since we need operator=.
	We could templatise this instead of repeating per Resource subclass, 
	except to do so requires a form VC6 does not support i.e.
	ResourceSubclassPtr<T> : public SharedPtr<T>
	*/
	class D3D10TexturePtr : public SharedPtr<D3D10Texture> 
	{
	public:
		D3D10TexturePtr() : SharedPtr<D3D10Texture>() {}
		explicit D3D10TexturePtr(D3D10Texture* rep) : SharedPtr<D3D10Texture>(rep) {}
		D3D10TexturePtr(const D3D10TexturePtr& r) : SharedPtr<D3D10Texture>(r) {} 
		D3D10TexturePtr(const ResourcePtr& r) : SharedPtr<D3D10Texture>()
		{
			// lock & copy other mutex pointer
			OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
			{
				OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
					OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
					pRep = static_cast<D3D10Texture*>(r.getPointer());
				pUseCount = r.useCountPointer();
				if (pUseCount)
				{
					++(*pUseCount);
				}
			}
		}

		/// Operator used to convert a ResourcePtr to a D3D10TexturePtr
		D3D10TexturePtr& operator=(const ResourcePtr& r)
		{
			if (pRep == static_cast<D3D10Texture*>(r.getPointer()))
				return *this;
			release();
			// lock & copy other mutex pointer
			OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
			{
				OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
					OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
					pRep = static_cast<D3D10Texture*>(r.getPointer());
				pUseCount = r.useCountPointer();
				if (pUseCount)
				{
					++(*pUseCount);
				}
			}
			return *this;
		}
		/// Operator used to convert a TexturePtr to a D3D10TexturePtr
		D3D10TexturePtr& operator=(const TexturePtr& r)
		{
			if (pRep == static_cast<D3D10Texture*>(r.getPointer()))
				return *this;
			release();
			// lock & copy other mutex pointer
			OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
			{
				OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
					OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
					pRep = static_cast<D3D10Texture*>(r.getPointer());
				pUseCount = r.useCountPointer();
				if (pUseCount)
				{
					++(*pUseCount);
				}
			}
			return *this;
		}
	};

	/// RenderTexture implementation for D3D10
	class D3D10RenderTexture : public RenderTexture
	{
		D3D10Device & mDevice;
		ID3D10RenderTargetView * mRenderTargetView;
		ID3D10DepthStencilView * mDepthStencilView;
	public:
		D3D10RenderTexture(const String &name, D3D10HardwarePixelBuffer *buffer, D3D10Device & device );
		virtual ~D3D10RenderTexture();

		void rebind(D3D10HardwarePixelBuffer *buffer);

		virtual void getCustomAttribute( const String& name, void *pData );

		bool requiresTextureFlipping() const { return false; }
	};

}

#endif
