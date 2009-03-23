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
