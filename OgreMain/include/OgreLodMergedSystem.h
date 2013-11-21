/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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

#ifndef _LodMergedSystem_H__
#define _LodMergedSystem_H__

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreHeaderPrefix.h"

#include "OgreIdString.h"

namespace Ogre {
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup LOD
	*  @{
	*/

	struct LodMerged
	{
		Real			lodValue;
		unsigned char	meshLodIndex;
		unsigned char	materialLodIndex;
	};
	inline bool operator < ( Real _left, const LodMerged &_right ) { return _left < _right.lodValue; }
	inline bool operator < ( const LodMerged &_left, Real _right ) { return _left.lodValue < _right; }
	inline bool operator < ( const LodMerged &_l, const LodMerged &_r )
																	{ return _l.lodValue < _r.lodValue; }

	typedef FastArray< FastArray<LodMerged> > LodMovableObjectArray;

	/**
    */
	class _OgreExport LodMergedSystem : public MovableAlloc
    {
		typedef vector<Real>::type LodValueVec;
		struct LodMeshPair
		{
			LodValueVec *meshList;
			LodValueVec *materialList;
		};
		typedef vector<LodMeshPair>::type LodMeshPairVec;

		map<IdString, LodMovableObjectArray*>::type	mLodMerged;

	public:
		virtual ~LodMergedSystem();

		const LodMovableObjectArray* getLodMerged( IdString hash, const LodMeshPairVec &meshesLod );
	};

	/** @} */
	/** @} */

}

#include "OgreHeaderSuffix.h"

#endif
