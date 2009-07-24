/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#ifndef __GLESPBRenderTexture_H__
#define __GLESPBRenderTexture_H__

#include "OgreGLESRenderTexture.h"

namespace Ogre {

    /** RenderTexture that uses a PBuffer (offscreen rendering context) for rendering.
    */
    class GLESPBRTTManager;
    class GLESPBuffer;
    class GLESContext;
    class _OgrePrivate GLESPBRenderTexture: public GLESRenderTexture
    {
        public:
            GLESPBRenderTexture(GLESPBRTTManager *manager, const String &name, const GLESSurfaceDesc &target, bool writeGamma, uint fsaa);
            virtual ~GLESPBRenderTexture();
            virtual void getCustomAttribute(const String& name, void* pData);

        protected:
            GLESPBRTTManager *mManager;
            PixelComponentType mPBFormat;
    };

    /** Manager for rendertextures and PBuffers (offscreen rendering contexts)
    */
    class _OgrePrivate GLESPBRTTManager: public GLESRTTManager
    {
        public:
            GLESPBRTTManager(GLESSupport *support, RenderTarget *mainwindow);
            virtual ~GLESPBRTTManager();

        /** @copydoc GLRTTManager::createRenderTexture
        */
            virtual RenderTexture *createRenderTexture(const String &name,
                                                       const GLESSurfaceDesc &target,
                                                       bool writeGamma, uint fsaa);

            /** @copydoc GLRTTManager::checkFormat
             */
            virtual bool checkFormat(PixelFormat format);

            /** @copydoc GLRTTManager::bind
             */
            virtual void bind(RenderTarget *target);

            /** @copydoc GLRTTManager::unbind
             */
            virtual void unbind(RenderTarget *target);

            /** Create PBuffer for a certain pixel format and size
             */
            void requestPBuffer(PixelComponentType ctype, size_t width, size_t height);

            /** Release PBuffer for a certain pixel format
             */
            void releasePBuffer(PixelComponentType ctype);

        protected:
            /** GLESSupport reference, used to create PBuffers */
            GLESSupport *mSupport;
            /** Primary window reference */
            RenderTarget *mMainWindow;
            /** Primary window context */
            GLESContext *mMainContext;
            /** Reference to a PBuffer, with refcount */
            struct PBRef
            {
                PBRef(): pb(0),refcount(0) {}
                GLESPBuffer* pb;
                size_t refcount;
            };
            /** Type to map each component type to a PBuffer */
            PBRef mPBuffers[PCT_COUNT];
    };
}

#endif
