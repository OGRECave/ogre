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

#ifndef __GL3PlusTextureManager_H__
#define __GL3PlusTextureManager_H__

#include "OgreGL3PlusPrerequisites.h"
#include "OgreTextureManager.h"
#include "OgreGL3PlusTexture.h"
#include "OgreGL3PlusSupport.h"

#include <vector>

namespace Ogre {
    typedef vector<GL3PlusTexturePtr>::type GL3PlusTexturePtrList;
    typedef vector<TexturePtr>::type TexturePtrList;

    /** GL3Plus-specific implementation of a TextureManager */
    class _OgreGL3PlusExport GL3PlusTextureManager : public TextureManager
    {
    public:
        GL3PlusTextureManager(GL3PlusSupport& support);
        virtual ~GL3PlusTextureManager();

        GLuint getWarningTextureID() { return mWarningTextureID; }

        /// @copydoc TextureManager::getNativeFormat
        PixelFormat getNativeFormat(TextureType ttype, PixelFormat format, int usage);

        /// @copydoc TextureManager::isHardwareFilteringSupported
        bool isHardwareFilteringSupported(TextureType ttype, PixelFormat format, int usage,
                                          bool preciseFormatOnly = false);

        // void bindImages();

        /* TexturePtr createManual(const String & name, const String& group, */
        /*                         TextureType texType, uint width, uint height, uint depth, int numMipmaps, */
        /*                         PixelFormat format, int usage, ManualResourceLoader* loader, bool hwGamma,  */
        /*                         uint fsaa, const String& fsaaHint); */

    protected:
        /// @copydoc ResourceManager::createImpl
        Resource* createImpl(const String& name, ResourceHandle handle,
                             const String& group, bool isManual, ManualResourceLoader* loader,
                             const NameValuePairList* createParams);

        /// Internal method to create a warning texture (bound when a texture unit is blank)
        void createWarningTexture();

        GL3PlusSupport& mGLSupport;
        GLuint mWarningTextureID;

    private:
        /// Register a texture as an image texture used in image load/store.
        // void registerImage(TexturePtr texture);

        /// Collection of textures associated with image load/store variables.
        // TexturePtrList mImages;
    };
}

#endif
