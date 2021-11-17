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

#include "OgreVulkanTextureGpuWindow.h"

#include "OgreVulkanTextureGpuManager.h"
#include "OgreVulkanWindow.h"
#include "OgreVector.h"

#include "OgreException.h"

namespace Ogre
{
    extern const String CustomAttributeIdString_GLCONTEXT;

    VulkanTextureGpuWindow::VulkanTextureGpuWindow(String name,
        TextureType initialType, TextureManager *textureManager,
        VulkanWindow *window ) :
        VulkanTextureGpu( textureManager, name, 0, "", true, 0),
        mWindow( window ),
        mCurrentImageIdx( 0u )
    {
        mTextureType = TEX_TYPE_2D;
        mNumMipmaps = 0;
    }
    //-----------------------------------------------------------------------------------
    VulkanTextureGpuWindow::~VulkanTextureGpuWindow() { unload(); }
    //-----------------------------------------------------------------------------------
    VkSemaphore VulkanTextureGpuWindow::getImageAcquiredSemaphore( void )
    {
        return mWindow->getImageAcquiredSemaphore();
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpuWindow::_setCurrentImage( VkImage image, uint32 imageIdx )
    {
        mFinalTextureName = image;
        mCurrentImageIdx = imageIdx;
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpuWindow::createInternalResourcesImpl( void )
    {
        if( mFSAA > 1 )
            createMsaaSurface();

        // create surface to handle download
        auto buf = std::make_shared<VulkanHardwarePixelBuffer>(this, mWidth, mHeight, mDepth, 1, 0);
        mSurfaceList.push_back(buf);
        mCurrLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        mNextLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpuWindow::freeInternalResourcesImpl( void )
    {
        mFinalTextureName = 0;
        destroyMsaaSurface();
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpuWindow::getCustomAttribute( const String& name, void *pData )
    {
        //if( name == "Window" )
        //    *static_cast<Window **>( pData ) = mWindow;
    }
}  // namespace Ogre
