/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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
#ifndef __D3D9TEXTURE_H__
#define __D3D9TEXTURE_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreTexture.h"
#include "OgreRenderTexture.h"
#include "OgreImage.h"
#include "OgreException.h"
#include "OgreD3D9HardwarePixelBuffer.h"
#include "OgreD3D9Resource.h"

namespace Ogre {
    class _OgreD3D9Export D3D9Texture : public Texture, public D3D9Resource
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
        
        typedef std::map<IDirect3DDevice9*, TextureResources*> DeviceToTextureResourcesMap;
        typedef DeviceToTextureResourcesMap::iterator           DeviceToTextureResourcesIterator;

        /// Map between device to texture resources.
        DeviceToTextureResourcesMap mMapDeviceToTextureResources;

        /// The memory pool being used
        D3DPOOL                         mD3DPool;
        // Dynamic textures?
        bool                            mDynamicTextures;
        
        /// Is hardware gamma supported (read)?
        bool mHwGammaReadSupported;
        /// Is hardware gamma supported (write)?
        bool mHwGammaWriteSupported;
        D3DMULTISAMPLE_TYPE mFSAAType;
        DWORD mFSAAQuality;

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

        /// internal method, create D3D9HardwarePixelBuffers for every face and
        /// mipmap level. This method must be called after the D3D texture object was created
        void _createSurfaceList(IDirect3DDevice9* d3d9Device, TextureResources* textureResources);

        /// overridden from Resource
        void loadImpl();

        /// gets the texture resources attached to the given device.
        TextureResources* getTextureResources(IDirect3DDevice9* d3d9Device);

        /// allocates new texture resources structure attached to the given device.
        TextureResources* allocateTextureResources(IDirect3DDevice9* d3d9Device);

        /// creates this texture resources according to the current settings.
        void createTextureResources(IDirect3DDevice9* d3d9Device);

        /// frees the given texture resources.
        void freeTextureResources(IDirect3DDevice9* d3d9Device, TextureResources* textureResources);

        void determinePool();

        friend class D3D9HardwarePixelBuffer;
    public:
        /// constructor 
        D3D9Texture(ResourceManager* creator, const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader);
        /// destructor
        ~D3D9Texture();

        /// overridden from Texture
        void copyToTexture( TexturePtr& target );

        const HardwarePixelBufferSharedPtr& getBuffer(size_t face, size_t mipmap);
        
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
    };

    /// RenderTexture implementation for D3D9
    class _OgreD3D9Export D3D9RenderTexture : public RenderTexture
    {
    public:
        D3D9RenderTexture(const String &name, D3D9HardwarePixelBuffer *buffer, bool writeGamma, uint fsaa);
        ~D3D9RenderTexture() {}

        virtual void update(bool swap);

        virtual void getCustomAttribute( const String& name, void *pData );

        bool requiresTextureFlipping() const { return false; }

        /// Override needed to deal with FSAA
        void swapBuffers();

    };

}

#endif
