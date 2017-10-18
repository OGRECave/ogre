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

#include "OgreGLTextureManager.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreGLCopyingRenderTexture.h"
#include "OgreGLTexture.h"
#include "OgreGLPixelFormat.h"

namespace Ogre {
    //-----------------------------------------------------------------------------
    GLTextureManager::GLTextureManager(GLRenderSystem* renderSystem)
        : TextureManager(), mRenderSystem(renderSystem)
    {
        // register with group manager
        ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);
    }
    //-----------------------------------------------------------------------------
    GLTextureManager::~GLTextureManager()
    {
        // unregister with group manager
        ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);
    }
    //-----------------------------------------------------------------------------
    Resource* GLTextureManager::createImpl(const String& name, ResourceHandle handle, 
        const String& group, bool isManual, ManualResourceLoader* loader, 
        const NameValuePairList* createParams)
    {
        return new GLTexture(this, name, handle, group, isManual, loader, mRenderSystem);
    }

    //-----------------------------------------------------------------------------
    PixelFormat GLTextureManager::getNativeFormat(TextureType ttype, PixelFormat format, int usage)
    {
        // Adjust requested parameters to capabilities
        const RenderSystemCapabilities *caps = Root::getSingleton().getRenderSystem()->getCapabilities();

        // Check compressed texture support
        // if a compressed format not supported, revert to PF_A8R8G8B8
        if(PixelUtil::isCompressed(format) &&
            !caps->hasCapability( RSC_TEXTURE_COMPRESSION_DXT ))
        {
            return PF_BYTE_RGBA;
        }
        // if floating point textures not supported, revert to PF_A8R8G8B8
        if(PixelUtil::isFloatingPoint(format) &&
            !caps->hasCapability( RSC_TEXTURE_FLOAT ))
        {
            return PF_BYTE_RGBA;
        }
        
        if(GLPixelUtil::getGLInternalFormat(format) == GL_NONE)
        {
            return PF_BYTE_RGBA;
        }

        // Check if this is a valid rendertarget format
        if( usage & TU_RENDERTARGET )
        {
            /// Get closest supported alternative
            /// If mFormat is supported it's returned
            return GLRTTManager::getSingleton().getSupportedAlternative(format);
        }

        // Supported
        return format;

        
    }
    //-----------------------------------------------------------------------------
    bool GLTextureManager::isHardwareFilteringSupported(TextureType ttype, PixelFormat format, int usage,
            bool preciseFormatOnly)
    {
        if (format == PF_UNKNOWN)
            return false;

        // Check native format
        PixelFormat nativeFormat = getNativeFormat(ttype, format, usage);
        if (preciseFormatOnly && format != nativeFormat)
            return false;

        // Assume non-floating point is supported always
        if (!PixelUtil::isFloatingPoint(nativeFormat))
            return true;

        // Hack: there are no elegant GL API to detects texture filtering supported,
        // just hard code for cards based on vendor specifications.

        // TODO: Add cards that 16 bits floating point flitering supported by
        // hardware below
        static const String sFloat16SupportedCards[] =
        {
            // GeForce 8 Series
            "*GeForce*8800*",

            // GeForce 7 Series
            "*GeForce*7950*",
            "*GeForce*7900*",
            "*GeForce*7800*",
            "*GeForce*7600*",
            "*GeForce*7500*",
            "*GeForce*7300*",

            // GeForce 6 Series
            "*GeForce*6800*",
            "*GeForce*6700*",
            "*GeForce*6600*",
            "*GeForce*6500*",

            ""                      // Empty string means end of list
        };

        // TODO: Add cards that 32 bits floating point flitering supported by
        // hardware below
        static const String sFloat32SupportedCards[] =
        {
            // GeForce 8 Series
            "*GeForce*8800*",

            ""                      // Empty string means end of list
        };

        PixelComponentType pct = PixelUtil::getComponentType(nativeFormat);
        const String* supportedCards;
        switch (pct)
        {
        case PCT_FLOAT16:
            supportedCards = sFloat16SupportedCards;
            break;
        case PCT_FLOAT32:
            supportedCards = sFloat32SupportedCards;
            break;
        default:
            return false;
        }

        const GLubyte* pcRenderer = glGetString(GL_RENDERER);
        String str = (const char*)pcRenderer;

        for (; !supportedCards->empty(); ++supportedCards)
        {
            if (StringUtil::match(str, *supportedCards))
            {
                return true;
            }
        }

        return false;
    }

}
