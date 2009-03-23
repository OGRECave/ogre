/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Copyright (c) 2006 Matthias Fink, netAllied GmbH <matthias.fink@web.de>								
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
#ifndef __Polygon_H__
#define __Polygon_H__

#include "OgrePrerequisites.h"
#include "OgreVector3.h"


namespace Ogre
{


	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Math
	*  @{
	*/
	/** The class represents a polygon in 3D space.
	@remarks
		It is made up of 3 or more vertices in a single plane, listed in 
		counter-clockwise order.
	*/
	class _OgreExport Polygon
	{

	public:
		typedef vector<Vector3>::type				VertexList;

		typedef multimap<Vector3, Vector3>::type		EdgeMap;
		typedef std::pair< Vector3, Vector3>		Edge;

	protected:
		VertexList		mVertexList;
		mutable Vector3	mNormal;
		mutable bool	mIsNormalSet;
		/** Updates the normal.
		*/
		void updateNormal(void) const;


	public:
		Polygon();
		~Polygon();
		Polygon( const Polygon& cpy );

		/** Inserts a vertex at a specific position.
		@note Vertices must be coplanar.
		*/
		void insertVertex(const Vector3& vdata, size_t vertexIndex);
		/** Inserts a vertex at the end of the polygon.
		@note Vertices must be coplanar.
		*/
		void insertVertex(const Vector3& vdata);

		/** Returns a vertex.
		*/
		const Vector3& getVertex(size_t vertex) const;

		/** Sets a specific vertex of a polygon.
		@note Vertices must be coplanar.
		*/
		void setVertex(const Vector3& vdata, size_t vertexIndex);

		/** Removes duplicate vertices from a polygon.
		*/
		void removeDuplicates(void);

		/** Vertex count.
		*/
		size_t getVertexCount(void) const;

		/** Returns the polygon normal.
		*/
		const Vector3& getNormal(void) const;

		/** Deletes a specific vertex.
		*/
		void deleteVertex(size_t vertex);

		/** Determines if a point is inside the polygon.
		@remarks
			A point is inside a polygon if it is both on the polygon's plane, 
			and within the polygon's bounds. Polygons are assumed to be convex
			and planar.
		*/
		bool isPointInside(const Vector3& point) const;

		/** Stores the edges of the polygon in ccw order.
			The vertices are copied so the user has to take the 
			deletion into account.
		*/
		void storeEdges(EdgeMap *edgeMap) const;

		/** Resets the object.
		*/
		void reset(void);

		/** Determines if the current object is equal to the compared one.
		*/
		bool operator == (const Polygon& rhs) const;

		/** Determines if the current object is not equal to the compared one.
		*/
		bool operator != (const Polygon& rhs) const
		{ return !( *this == rhs ); }

		/** Prints out the polygon data.
		*/
		_OgreExport friend std::ostream& operator<< ( std::ostream& strm, const Polygon& poly );

	};
	/** @} */
	/** @} */

}

#endif
