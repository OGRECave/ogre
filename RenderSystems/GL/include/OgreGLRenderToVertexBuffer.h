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
#ifndef __GLRenderToVertexBuffer_H__
#define __GLRenderToVertexBuffer_H__

#include "OgreRenderToVertexBuffer.h"
#include "OgreGLPrerequisites.h"

namespace Ogre {
    /**
        An object which renders geometry to a vertex.
    @remarks
        This is especially useful together with geometry shaders, as you can
        render procedural geometry which will get saved to a vertex buffer for
        reuse later, without regenerating it again. You can also create shaders
        that run on previous results of those shaders, creating stateful 
        shaders.
    */
	class _OgrePrivate GLRenderToVertexBuffer : public RenderToVertexBuffer
    {    
    public:
		/** C'tor */
		GLRenderToVertexBuffer();
		/** D'tor */
        virtual ~GLRenderToVertexBuffer();

        /**
            Get the render operation for this buffer 
        */
        virtual void getRenderOperation(RenderOperation& op);

        /**
            Update the contents of this vertex buffer by rendering
        */
        virtual void update(SceneManager* sceneMgr);
	protected:
		void reallocateBuffer(size_t index);
		void bindVerticesOutput(Pass* pass);
		GLint getGLSemanticType(VertexElementSemantic semantic);
		String getSemanticVaryingName(VertexElementSemantic semantic, unsigned short index);
		HardwareVertexBufferSharedPtr mVertexBuffers[2];
		size_t mFrontBufferIndex;
		GLuint mPrimitivesDrawnQuery;
    };
}

#endif
