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

namespace Ogre
{
	//-----------------------------------------------------------------------
	FORCEINLINE void InstancedEntity::writeSingleTransform3x4( float * RESTRICT_ALIAS xform ) const
	{
#if defined( OGRE_USE_SIMD ) && OGRE_CPU == OGRE_CPU_X86
		const Matrix4& mat = mParentNode->_getFullTransform();
		_mm_stream_ps( xform,    _mm_load_ps( mat[0] ) );
		_mm_stream_ps( xform+4,  _mm_load_ps( mat[1] ) );
		_mm_stream_ps( xform+8,  _mm_load_ps( mat[2] ) );
#else
		const Matrix4& mat = mParentNode->_getFullTransform();
		for( int i=0; i<3; ++i )
		{
			Real const * RESTRICT_ALIAS row = mat[i];
			*xform++ = static_cast<float>( *row++ );
			*xform++ = static_cast<float>( *row++ );
			*xform++ = static_cast<float>( *row++ );
			*xform++ = static_cast<float>( *row++ );
		}
#endif
	}
	//-----------------------------------------------------------------------
	FORCEINLINE void InstancedEntity::writeAnimatedTransform3x4( float * RESTRICT_ALIAS xform,
															Mesh::IndexMap::const_iterator itor,
															Mesh::IndexMap::const_iterator end ) const
	{
		/*const Mesh::IndexMap *indexMap = mBatchOwner->_getIndexToBoneMap();
		Mesh::IndexMap::const_iterator itor = indexMap->begin();
		Mesh::IndexMap::const_iterator end  = indexMap->end();*/

		while( itor != end )
		{
#if defined( OGRE_USE_SIMD ) && OGRE_CPU == OGRE_CPU_X86
			Matrix4 const * RESTRICT_ALIAS mat = reinterpret_cast<Matrix4 const * RESTRICT_ALIAS>
													(mBoneWorldMatrices[*itor++][0]);
			_mm_stream_ps( xform,    _mm_load_ps( (*mat)[0] ) );
			_mm_stream_ps( xform+4,  _mm_load_ps( (*mat)[1] ) );
			_mm_stream_ps( xform+8,  _mm_load_ps( (*mat)[2] ) );
			xform += 12;
#else
			Real const * RESTRICT_ALIAS row = reinterpret_cast<Real const * RESTRICT_ALIAS>
													(mBoneWorldMatrices[*itor++][0]);
			for( int i=0; i<3; ++i )
			{
				*xform++ = static_cast<float>( *row++ );
				*xform++ = static_cast<float>( *row++ );
				*xform++ = static_cast<float>( *row++ );
				*xform++ = static_cast<float>( *row++ );
			}
#endif
		}
	}
}
