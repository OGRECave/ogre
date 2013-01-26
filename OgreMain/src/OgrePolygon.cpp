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
#include "OgreStableHeaders.h"
#include "OgrePolygon.h"
#include "OgreException.h"
#include "OgreVector3.h"

namespace Ogre
{

	//-----------------------------------------------------------------------
	Polygon::Polygon()
	: mNormal( Vector3::ZERO )
	, mIsNormalSet(false)
	{
		// reserve space for 6 vertices to reduce allocation cost
		mVertexList.reserve(6);
	}
	//-----------------------------------------------------------------------
	Polygon::~Polygon()
	{
	}
	//-----------------------------------------------------------------------
	Polygon::Polygon( const Polygon& cpy )
	{
		mVertexList = cpy.mVertexList;
		mNormal = cpy.mNormal;
		mIsNormalSet = cpy.mIsNormalSet;
	}
	//-----------------------------------------------------------------------
	void Polygon::insertVertex(const Vector3& vdata, size_t vertex )
	{
		// TODO: optional: check planarity
		OgreAssert(vertex <= getVertexCount(), "Insert position out of range" );

		VertexList::iterator it = mVertexList.begin();

		std::advance(it, vertex);
		mVertexList.insert(it, vdata);

	}
	//-----------------------------------------------------------------------
	void Polygon::insertVertex(const Vector3& vdata)
	{
		mVertexList.push_back(vdata);
	}
	//-----------------------------------------------------------------------
	const Vector3& Polygon::getVertex( size_t vertex ) const
	{
		OgreAssert(vertex < getVertexCount(), "Search position out of range");

		return mVertexList[vertex];
	}
	//-----------------------------------------------------------------------
	void Polygon::setVertex(const Vector3& vdata, size_t vertex )
	{
		// TODO: optional: check planarity
		OgreAssert(vertex < getVertexCount(), "Search position out of range" );

		// set new vertex
		mVertexList[ vertex ] = vdata;
	}
	//-----------------------------------------------------------------------
	void Polygon::removeDuplicates( void )
	{
		for ( size_t i = 0; i < getVertexCount(); ++i )
		{
			const Vector3& a = getVertex( i );
			const Vector3& b = getVertex( (i + 1)%getVertexCount() );

			if (a.positionEquals(b))
			{
				deleteVertex(i);
				--i;
			}
		}
	}
	//-----------------------------------------------------------------------
	size_t Polygon::getVertexCount( void ) const
	{
		return mVertexList.size();
	}
	//-----------------------------------------------------------------------
	const Vector3& Polygon::getNormal( void ) const
	{
		OgreAssert( getVertexCount() >= 3, "Insufficient vertex count!" );

		updateNormal();

		return mNormal;
	}
	//-----------------------------------------------------------------------
	void Polygon::updateNormal( void ) const
	{
		OgreAssert( getVertexCount() >= 3, "Insufficient vertex count!" );

		if (mIsNormalSet)
			return;

		// vertex order is ccw
		const Vector3& a = getVertex( 0 );
		const Vector3& b = getVertex( 1 );
		const Vector3& c = getVertex( 2 );

		// used method: Newell
		mNormal.x = 0.5f * ( (a.y - b.y) * (a.z + b.z) +
							   (b.y - c.y) * (b.z + c.z) + 
							   (c.y - a.y) * (c.z + a.z));

		mNormal.y = 0.5f * ( (a.z - b.z) * (a.x + b.x) +
							   (b.z - c.z) * (b.x + c.x) + 
							   (c.z - a.z) * (c.x + a.x));

		mNormal.z = 0.5f * ( (a.x - b.x) * (a.y + b.y) +
							   (b.x - c.x) * (b.y + c.y) + 
							   (c.x - a.x) * (c.y + a.y));

		mNormal.normalise();

		mIsNormalSet = true;

	}
	//-----------------------------------------------------------------------
	void Polygon::deleteVertex( size_t vertex )
	{
		OgreAssert( vertex < getVertexCount(), "Search position out of range" );

		VertexList::iterator it = mVertexList.begin();
		std::advance(it, vertex);

		mVertexList.erase( it );
	}
	//-----------------------------------------------------------------------
	void Polygon::storeEdges( Polygon::EdgeMap *edgeMap ) const
	{
		OgreAssert( edgeMap != NULL, "EdgeMap ptr is NULL" );

		size_t vertexCount = getVertexCount();

		for ( size_t i = 0; i < vertexCount; ++i )
		{
			edgeMap->insert( Edge( getVertex( i ), getVertex( ( i + 1 ) % vertexCount ) ) );
		}
	}
	//-----------------------------------------------------------------------
	void Polygon::reset( void )
	{
		// could use swap() to free memory here, but assume most may be reused so avoid realloc
		mVertexList.clear();

		mIsNormalSet = false;
	}
	//-----------------------------------------------------------------------
	bool Polygon::operator == (const Polygon& rhs) const
	{
		if ( getVertexCount() != rhs.getVertexCount() )
			return false;

		// Compare vertices. They may differ in its starting position.
		// find start
		size_t start = 0;
		bool foundStart = false;
		for (size_t i = 0; i < getVertexCount(); ++i )
		{	
			if (getVertex(0).positionEquals(rhs.getVertex(i)))
			{
				start = i;
				foundStart = true;
				break;
			}
		}

		if (!foundStart)
			return false;

		for (size_t i = 0; i < getVertexCount(); ++i )
		{
			const Vector3& vA = getVertex( i );
			const Vector3& vB = rhs.getVertex( ( i + start) % getVertexCount() );

			if (!vA.positionEquals(vB))
				return false;
		}

		return true;
	}
	//-----------------------------------------------------------------------
	std::ostream& operator<< ( std::ostream& strm, const Polygon& poly )
	{
		strm << "NUM VERTICES: " << poly.getVertexCount() << std::endl;

		for (size_t j = 0; j < poly.getVertexCount(); ++j )
		{
			strm << "VERTEX " << j << ": " << poly.getVertex( j ) << std::endl;
		}

		return strm;
	}
	//-----------------------------------------------------------------------
	bool Polygon::isPointInside(const Vector3& point) const
	{
		// sum the angles 
		Real anglesum = 0;
		size_t n = getVertexCount();
		for (size_t i = 0; i < n; i++) 
		{
			const Vector3& p1 = getVertex(i);
			const Vector3& p2 = getVertex((i + 1) % n);

			Vector3 v1 = p1 - point;
			Vector3 v2 = p2 - point;

			Real len1 = v1.length();
			Real len2 = v2.length();

			if (Math::RealEqual(len1 * len2, 0.0f, 1e-4f))
			{
				// We are on a vertex so consider this inside
				return true; 
			}
			else
			{
				Real costheta = v1.dotProduct(v2) / (len1 * len2);
				anglesum += acos(costheta);
			}
		}

		// result should be 2*PI if point is inside poly
		return Math::RealEqual(anglesum, Math::TWO_PI, 1e-4f);

	}
}
