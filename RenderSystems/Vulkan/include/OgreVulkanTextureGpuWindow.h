/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-present Torus Knot Software Ltd

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

#ifndef _OgreVulkanTextureGpuWindow_H_
#define _OgreVulkanTextureGpuWindow_H_

#include "OgreVulkanTextureGpu.h"

#include "OgreHeaderPrefix.h"

namespace Ogre
{
    class _OgreVulkanExport VulkanTextureGpuWindow : public VulkanTextureGpuRenderTarget
    {
        VulkanWindow *mWindow;

        uint32 mCurrentSwapchainIdx;

        virtual void createInternalResourcesImpl( void );
        virtual void destroyInternalResourcesImpl( void );

    public:
        VulkanTextureGpuWindow( GpuPageOutStrategy::GpuPageOutStrategy pageOutStrategy,
                                VaoManager *vaoManager, IdString name, uint32 textureFlags,
                                TextureTypes::TextureTypes initialType,
                                TextureGpuManager *textureManager, VulkanWindow *window );
        virtual ~VulkanTextureGpuWindow();

        virtual void setTextureType( TextureTypes::TextureTypes textureType );

        virtual void getSubsampleLocations( vector<Vector2>::type locations );

        virtual void notifyDataIsReady( void );
        virtual bool _isDataReadyImpl( void ) const;

        /// @copydoc VulkanWindow::getImageAcquiredSemaphore
        VkSemaphore getImageAcquiredSemaphore( void );

        void _setCurrentSwapchain( VkImage image, uint32 swapchainIdx );
        uint32 getCurrentSwapchainIdx( void ) const { return mCurrentSwapchainIdx; }

        VkImage getWindowFinalTextureName( size_t idx ) const;
        size_t getWindowNumSurfaces( void ) const;

        virtual void swapBuffers( void );

        virtual void getCustomAttribute( IdString name, void *pData );

        virtual bool isOpenGLRenderWindow( void ) const;

        virtual void _setToDisplayDummyTexture( void );
        virtual void _notifyTextureSlotChanged( const TexturePool *newPool, uint16 slice );
    };
}  // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif
