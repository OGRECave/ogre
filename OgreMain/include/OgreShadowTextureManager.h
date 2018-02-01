/*-------------------------------------------------------------------------
This source file is a part of OGRE
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
THE SOFTWARE

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-------------------------------------------------------------------------*/
#ifndef __ShadowTextureManager_H__
#define __ShadowTextureManager_H__

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreSingleton.h"
#include "OgrePixelFormat.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */
    typedef std::vector<TexturePtr> ShadowTextureList;

    /** Structure containing the configuration for one shadow texture. */
    struct ShadowTextureConfig
    {
        unsigned int width;
        unsigned int height;
        PixelFormat format;
        unsigned int fsaa;
        uint16      depthBufferPoolId;

        ShadowTextureConfig()
            : width(512), height(512), format(PF_X8R8G8B8), fsaa(0), depthBufferPoolId(1) {}
    };

    typedef std::vector<ShadowTextureConfig> ShadowTextureConfigList;
    typedef ConstVectorIterator<ShadowTextureConfigList> ConstShadowTextureConfigIterator;

    _OgreExport bool operator== ( const ShadowTextureConfig& lhs, const ShadowTextureConfig& rhs );
    _OgreExport bool operator!= ( const ShadowTextureConfig& lhs, const ShadowTextureConfig& rhs );


    /** Class to manage the available shadow textures which may be shared between
        many SceneManager instances if formats agree.
    @remarks
        The management of the list of shadow textures has been separated out into
        a dedicated class to enable the clean management of shadow textures
        across many scene manager instances. Where multiple scene managers are
        used with shadow textures, the configuration of those shadows may or may
        not be consistent - if it is, it is good to centrally manage the textures
        so that creation and destruction responsibility is clear.
    */
    class _OgreExport ShadowTextureManager : public Singleton<ShadowTextureManager>, public ShadowDataAlloc
    {
    protected:
        ShadowTextureList mTextureList;
        ShadowTextureList mNullTextureList;
        size_t mCount;

    public:
        ShadowTextureManager();
        virtual ~ShadowTextureManager();

        /** Populate an incoming list with shadow texture references as requested
            in the configuration list.
        */
        virtual void getShadowTextures(const ShadowTextureConfigList& config, 
            ShadowTextureList& listToPopulate);

        /** Get an appropriately defined 'null' texture, i.e. one which will always
            result in no shadows.
        */
        virtual TexturePtr getNullShadowTexture(PixelFormat format);

        /** Remove any shadow textures that are no longer being referenced.
        @remarks
            This should be called fairly regularly since references may take a 
            little while to disappear in some cases (if referenced by materials)
        */
        virtual void clearUnused();
        /** Dereference all the shadow textures kept in this class and remove them
            from TextureManager; note that it is up to the SceneManagers to clear 
            their local references.
        */
        virtual void clear();

        /// @copydoc Singleton::getSingleton()
        static ShadowTextureManager& getSingleton(void);
        /// @copydoc Singleton::getSingleton()
        static ShadowTextureManager* getSingletonPtr(void);

    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif

