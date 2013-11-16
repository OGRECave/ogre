/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd
Copyright (c) 2006 Matthias Fink, netAllied GmbH <matthias.fink@web.de>								

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
#ifndef __ConvexBody_H__
#define __ConvexBody_H__

#include "OgrePrerequisites.h"
#include "OgrePolygon.h"
#if OGRE_THREAD_SUPPORT
#include "Threading/OgreThreadHeaders.h"
#endif
#include "OgreHeaderPrefix.h"


namespace Ogre
{

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Math
	*  @{
	*/
	/** Holds a solid representation of a convex body.
		@remarks
			Administers a convex body. All polygons of the body are convex and
			planar. Several operations may be applied, ranging from intersection
			to join where each result it itself a convex body.
	*/
	class _OgreExport ConvexBody
	{
	public:
		typedef vector< Polygon* >::type	PolygonList;

	protected:
		PolygonList mPolygons;

		// Static 'free list' of polygons to save reallocation, shared between all bodies
		static PolygonList msFreePolygons;
#if OGRE_THREAD_SUPPORT
		OGRE_STATIC_MUTEX(msFreePolygonsMutex);
#endif

	public:
		ConvexBody();
		~ConvexBody();
		ConvexBody( const ConvexBody& cpy );

		/** Build a new polygon representation from a frustum.
		*/
		void define(const Frustum& frustum);

		/** Build a new polygon representation from an AAB.
		*/
		void define(const AxisAlignedBox& aab);

		/** Clips the body with a frustum. The resulting holes
			are filled with new polygons.
		*/
		void clip( const Frustum& frustum );

		/** Clips the body with an AAB. The resulting holes
			are filled with new polygons.
		*/
		void clip( const AxisAlignedBox& aab );

		/** Clips the body with another body.
		*/
		void clip(const ConvexBody& body);

		/** Clips the object by the positive half space of a plane
		*/
		void clip(const Plane& pl, bool keepNegative = true);

		/** Extends the existing body to incorporate the passed in point as a
			convex hull.
		@remarks
			You must already have constructed a basic body using a 'construct' 
			method.
		*/
		void extend(const Vector3& pt);

		/** Resets the object.
		*/
		void reset( void );

		/** Returns the current number of polygons.
		*/
		size_t getPolygonCount( void ) const;

		/** Returns the number of vertices for a polygon
		*/
		size_t getVertexCount( size_t poly ) const;

		/** Returns a polygon.
		*/
		const Polygon& getPolygon( size_t poly ) const;

		/** Returns a specific vertex of a polygon.
		*/
		const Vector3& getVertex( size_t poly, size_t vertex ) const;

		/** Returns the normal of a specified polygon.
		*/
		const Vector3& getNormal( size_t poly );

		/** Returns an AABB representation.
		*/
		AxisAlignedBox getAABB( void ) const;

		/** Checks if the body has a closed hull.
		*/
		bool hasClosedHull( void ) const;

		/** Merges all neighboring polygons into one single polygon if they are
			lay in the same plane.
		*/
		void mergePolygons( void );

		/** Determines if the current object is equal to the compared one.
		*/
		bool operator == ( const ConvexBody& rhs ) const;

		/** Determines if the current object is not equal to the compared one.
		*/
		bool operator != ( const ConvexBody& rhs ) const
		{ return !( *this == rhs ); }

		/** Prints out the body with all its polygons.
		*/
		_OgreExport friend std::ostream& operator<< ( std::ostream& strm, const ConvexBody& body );

		/** Log details of this body */
		void logInfo() const;

		/// Initialise the internal polygon pool used to minimise allocations
		static void _initialisePool();
		/// Tear down the internal polygon pool used to minimise allocations
		static void _destroyPool();


	protected:
		/** Get a new polygon from the pool.
		*/
		static Polygon* allocatePolygon();
		/** Release a polygon back tot he pool. */
		static void freePolygon(Polygon* poly);
		/** Inserts a polygon at a particular point in the body.
		@note
			After this method is called, the ConvexBody 'owns' this Polygon
			and will be responsible for deleting it.
		*/
		void insertPolygon(Polygon* pdata, size_t poly);
		/** Inserts a polygon at the end.
		@note
			After this method is called, the ConvexBody 'owns' this Polygon
			and will be responsible for deleting it.
		*/
		void insertPolygon(Polygon* pdata);

		/** Inserts a vertex for a polygon at a particular point.
		@note
			No checks are done whether the assembled polygon is (still) planar, 
			the caller must ensure that this is the case.
		*/
		void insertVertex(size_t poly, const Vector3& vdata, size_t vertex);
		/** Inserts a vertex for a polygon at the end.
		@note
			No checks are done whether the assembled polygon is (still) planar, 
			the caller must ensure that this is the case.
		*/
		void insertVertex(size_t poly, const Vector3& vdata);
		/** Deletes a specific polygon.
		*/
		void deletePolygon(size_t poly);

		/** Removes a specific polygon from the body without deleting it.
		@note
			The retrieved polygon needs to be deleted later by the caller.
		*/
		Polygon* unlinkPolygon(size_t poly);

		/** Moves all polygons from the parameter body to this instance.
		@note Both the passed in object and this instance are modified
		*/
		void moveDataFromBody(ConvexBody& body);

		/** Deletes a specific vertex of a specific polygon.
		*/
		void deleteVertex(size_t poly, size_t vertex);

		/** Replace a polygon at a particular index.
		@note Again, the passed in polygon is owned by this object after this
			call returns, and this object is resonsible for deleting it.
		*/
		void setPolygon(Polygon* pdata, size_t poly );

		/** Replace a specific vertex of a polygon.
		@note
			No checks are done whether the assembled polygon is (still) planar, 
			the caller must ensure that this is the case.
		*/
		void setVertex( size_t poly, const Vector3& vdata, size_t vertex );

		/** Returns the single edges in an EdgeMap (= edges where one side is a vertex and the
			other is empty space (a hole in the body)).
		*/
		Polygon::EdgeMap getSingleEdges() const;

		/** Stores the edges of a specific polygon in a passed in structure.
		*/
		void storeEdgesOfPolygon(size_t poly, Polygon::EdgeMap *edgeMap) const;
			
		/** Allocates space for an specified amount of polygons with
			each of them having a specified number of vertices.
			@note
				Old data (if available) will be erased.
		*/
		void allocateSpace(size_t numPolygons, size_t numVertices);

		/** Searches for a pair (an edge) in the intersectionList with an entry
			that equals vec, and removes it from the passed in list.
		@param vec The vertex to search for in intersectionEdges
		@param intersectionEdges A list of edges, which is updated if a match is found
		@param vNext A reference to a vector which will be filled with the other
			vertex at the matching edge, if found.
		@return True if a match was found
		*/
		bool findAndEraseEdgePair(const Vector3& vec, 
			Polygon::EdgeMap& intersectionEdges, Vector3& vNext ) const;

	};
	/** @} */
	/** @} */

}

#include "OgreHeaderSuffix.h"

#endif 

