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

#include "OgreTextureBox.h"
#include "OgreTextureGpuListener.h"
#include "OgreVector2.h"
#include "OgreWindow.h"

#include "Vao/OgreVaoManager.h"

#include "OgreException.h"

namespace Ogre
{
    extern const IdString CustomAttributeIdString_GLCONTEXT;

    VulkanTextureGpuWindow::VulkanTextureGpuWindow(
        GpuPageOutStrategy::GpuPageOutStrategy pageOutStrategy, VaoManager *vaoManager, IdString name,
        uint32 textureFlags, TextureTypes::TextureTypes initialType, TextureGpuManager *textureManager,
        VulkanWindow *window ) :
        VulkanTextureGpuRenderTarget( pageOutStrategy, vaoManager, name, textureFlags, initialType,
                                      textureManager ),
        mWindow( window ),
        mCurrentSwapchainIdx( 0u )
    {
        mTextureType = TextureTypes::Type2D;
    }
    //-----------------------------------------------------------------------------------
    VulkanTextureGpuWindow::~VulkanTextureGpuWindow() { destroyInternalResourcesImpl(); }
    //-----------------------------------------------------------------------------------
    VkSemaphore VulkanTextureGpuWindow::getImageAcquiredSemaphore( void )
    {
        return mWindow->getImageAcquiredSemaphore();
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpuWindow::_setCurrentSwapchain( VkImage image, uint32 swapchainIdx )
    {
        mFinalTextureName = image;
        mCurrentSwapchainIdx = swapchainIdx;
    }
    //-----------------------------------------------------------------------------------
    VkImage VulkanTextureGpuWindow::getWindowFinalTextureName( size_t idx ) const
    {
        return mWindow->getSwapchainImage( idx );
    }
    //-----------------------------------------------------------------------------------
    size_t VulkanTextureGpuWindow::getWindowNumSurfaces( void ) const
    {
        return mWindow->getNumSwapchains();
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpuWindow::createInternalResourcesImpl( void )
    {
        if( mSampleDescription.isMultisample() )
            createMsaaSurface();
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpuWindow::destroyInternalResourcesImpl( void )
    {
        mFinalTextureName = 0;
        destroyMsaaSurface();
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpuWindow::notifyDataIsReady( void )
    {
        assert( mResidencyStatus == GpuResidency::Resident );
        notifyAllListenersTextureChanged( TextureGpuListener::ReadyForRendering );
    }
    //-----------------------------------------------------------------------------------
    bool VulkanTextureGpuWindow::_isDataReadyImpl( void ) const
    {
        return mResidencyStatus == GpuResidency::Resident;
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpuWindow::swapBuffers( void ) { mWindow->swapBuffers(); }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpuWindow::getCustomAttribute( IdString name, void *pData )
    {
        if( name == "Window" )
            *static_cast<Window **>( pData ) = mWindow;
    }
    //-----------------------------------------------------------------------------------
    bool VulkanTextureGpuWindow::isOpenGLRenderWindow( void ) const { return true; }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpuWindow::_setToDisplayDummyTexture( void ) {}
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpuWindow::_notifyTextureSlotChanged( const TexturePool *newPool, uint16 slice )
    {
        OGRE_EXCEPT( Exception::ERR_INVALID_CALL, "",
                     "VulkanTextureGpuWindow::_notifyTextureSlotChanged" );
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpuWindow::setTextureType( TextureTypes::TextureTypes textureType )
    {
        OGRE_EXCEPT( Exception::ERR_INVALID_CALL,
                     "You cannot call setTextureType if isRenderWindowSpecific is true",
                     "VulkanTextureGpuWindow::setTextureType" );
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpuWindow::getSubsampleLocations( vector<Vector2>::type locations )
    {
        locations.reserve( mSampleDescription.getColourSamples() );
        if( mSampleDescription.getColourSamples() <= 1u )
        {
            locations.push_back( Vector2( 0.0f, 0.0f ) );
        }
        else
        {
            assert( mSampleDescription.getMsaaPattern() != MsaaPatterns::Undefined );

            OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "",
                         "VulkanTextureGpuWindow::getSubsampleLocations" );
        }
    }
}  // namespace Ogre
