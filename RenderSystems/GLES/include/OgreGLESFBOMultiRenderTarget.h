/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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

#ifndef __GLESMULTIRENDERTARGET_H__
#define __GLESMULTIRENDERTARGET_H__

#include "OgreGLESFrameBufferObject.h"

namespace Ogre {
    
    class GLESFBOManager;

	/** MultiRenderTarget for GL ES. Requires the FBO extension.
	*/
	class _OgrePrivate GLESFBOMultiRenderTarget : public MultiRenderTarget
	{
	public:
		GLESFBOMultiRenderTarget(GLESFBOManager *manager, const String &name);
		~GLESFBOMultiRenderTarget();

		virtual void getCustomAttribute( const String& name, void *pData );

		bool requiresTextureFlipping() const { return true; }
	private:
		virtual void bindSurfaceImpl(size_t attachment, RenderTexture *target);
		virtual void unbindSurfaceImpl(size_t attachment); 
		GLESFrameBufferObject fbo;
	};

}

#endif // __GLESMULTIRENDERTARGET_H__
