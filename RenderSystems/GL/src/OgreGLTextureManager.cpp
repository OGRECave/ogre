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

#include "OgreGLTextureManager.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreGLRenderTexture.h"

namespace Ogre {
    //-----------------------------------------------------------------------------
    GLTextureManager::GLTextureManager(GLSupport& support)
        : TextureManager(), mGLSupport(support), mWarningTextureID(0)
    {
        // register with group manager
        ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);

		createWarningTexture();
    }
    //-----------------------------------------------------------------------------
    GLTextureManager::~GLTextureManager()
    {
        // unregister with group manager
        ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);
		// Delete warning texture
		glDeleteTextures(1, &mWarningTextureID);
    }
    //-----------------------------------------------------------------------------
    Resource* GLTextureManager::createImpl(const String& name, ResourceHandle handle, 
        const String& group, bool isManual, ManualResourceLoader* loader, 
        const NameValuePairList* createParams)
    {
        return new GLTexture(this, name, handle, group, isManual, loader, mGLSupport);
    }

	//-----------------------------------------------------------------------------
	void GLTextureManager::createWarningTexture()
	{
		// Generate warning texture
		size_t width = 8;
		size_t height = 8;
		uint32 *data = new uint32[width*height];		// 0xXXRRGGBB
		// Yellow/black stripes
		for(size_t y=0; y<height; ++y)
		{
			for(size_t x=0; x<width; ++x)
			{
				data[y*width+x] = (((x+y)%8)<4)?0x000000:0xFFFF00;
			}
		}
		// Create GL resource
        glGenTextures(1, &mWarningTextureID);
		glBindTexture(GL_TEXTURE_2D, mWarningTextureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, (void*)data);
		// Free memory
		delete [] data;
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
			return PF_A8R8G8B8;
		}
		// if floating point textures not supported, revert to PF_A8R8G8B8
		if(PixelUtil::isFloatingPoint(format) &&
            !caps->hasCapability( RSC_TEXTURE_FLOAT ))
		{
			return PF_A8R8G8B8;
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

        // Check natively format
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
