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

#ifndef __GLPBRENDERTEXTURE_H__
#define __GLPBRENDERTEXTURE_H__

#include "OgreGLRenderTexture.h"
#include "OgreGLPBuffer.h"
namespace Ogre {
    
    /** RenderTexture that uses a PBuffer (offscreen rendering context) for rendering.
    */
    class GLPBRTTManager;
    class _OgrePrivate GLPBRenderTexture: public GLRenderTexture
    {
    public:
        GLPBRenderTexture(GLPBRTTManager *manager, const String &name, const GLSurfaceDesc &target, bool writeGamma, uint fsaa);
        virtual ~GLPBRenderTexture();
        
        virtual void getCustomAttribute(const String& name, void* pData);
    protected:
        GLPBRTTManager *mManager;
        PixelComponentType mPBFormat;
    };
        
    /** Manager for rendertextures and PBuffers (offscreen rendering contexts)
    */
    class _OgrePrivate GLPBRTTManager: public GLRTTManager
    {
    public:
        GLPBRTTManager(GLSupport *support, RenderTarget *mainwindow);
        virtual ~GLPBRTTManager();
        
        /** @copydoc GLRTTManager::createRenderTexture
        */
        virtual RenderTexture *createRenderTexture(const String &name, 
			const GLSurfaceDesc &target, bool writeGamma, uint fsaa);
        
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
        
        /** Get GL rendering context for a certain component type and size.
        */
        GLContext *getContextFor(PixelComponentType ctype, size_t width, size_t height);
    protected:
        /** GLSupport reference, used to create PBuffers */
        GLSupport *mSupport;
		/** Primary window reference */
		RenderTarget *mMainWindow;
		/** Primary window context */
		GLContext *mMainContext;
        /** Reference to a PBuffer, with refcount */
        struct PBRef
        {
            PBRef(): pb(0),refcount(0) {}
            GLPBuffer* pb;
            size_t refcount;
        };
        /** Type to map each component type to a PBuffer */
        PBRef mPBuffers[PCT_COUNT];
    };
}

#endif // __GLPBRENDERTEXTURE_H__
