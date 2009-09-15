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
#ifndef _RenderOperation_H__
#define _RenderOperation_H__

#include "OgrePrerequisites.h"
#include "OgreVertexIndexData.h"

namespace Ogre {


	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup RenderSystem
	*  @{
	*/
	/** 'New' rendering operation using vertex buffers. */
	class _OgrePrivate RenderOperation {
	public:
		/// The rendering operation type to perform
		enum OperationType {
			/// A list of points, 1 vertex per point
            OT_POINT_LIST = 1,
			/// A list of lines, 2 vertices per line
            OT_LINE_LIST = 2,
			/// A strip of connected lines, 1 vertex per line plus 1 start vertex
            OT_LINE_STRIP = 3,
			/// A list of triangles, 3 vertices per triangle
            OT_TRIANGLE_LIST = 4,
			/// A strip of triangles, 3 vertices for the first triangle, and 1 per triangle after that 
            OT_TRIANGLE_STRIP = 5,
			/// A fan of triangles, 3 vertices for the first triangle, and 1 per triangle after that
            OT_TRIANGLE_FAN = 6
        };

		/// Vertex source data
		VertexData *vertexData;

		/// The type of operation to perform
		OperationType operationType;

		/** Specifies whether to use indexes to determine the vertices to use as input. If false, the vertices are
		 simply read in sequence to define the primitives. If true, indexes are used instead to identify vertices
		 anywhere in the buffer, and allowing vertices to be used more than once.
	   	 If true, then the indexBuffer, indexStart and numIndexes properties must be valid. */
		bool useIndexes;

		/// Index data - only valid if useIndexes is true
		IndexData *indexData;
		/// Debug pointer back to renderable which created this
		const Renderable* srcRenderable;



        RenderOperation() :
            vertexData(0), operationType(OT_TRIANGLE_LIST), useIndexes(true),
                indexData(0), srcRenderable(0) {}


	};
	/** @} */
	/** @} */
}



#endif
