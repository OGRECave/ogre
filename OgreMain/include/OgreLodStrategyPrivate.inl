/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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
    inline void LodStrategy::lodSet( ObjectData &objData, Real lodValues[ARRAY_PACKED_REALS] )
    {
        for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
        {
            MovableObject *owner = objData.mOwner[j];

            //This may look like a lot of ugly indirections, but mLodMerged is a pointer that allows
            //sharing with many MovableObjects (it should perfectly fit even in small caches).
            {
                FastArray<Real>::const_iterator it = std::lower_bound( owner->mLodMesh->begin(),
                                                                        owner->mLodMesh->end(),
                                                                        lodValues[j] );
                owner->mCurrentMeshLod = std::max<int>( it - owner->mLodMesh->begin() - 1, 0 );
            }

            RenderableArray::iterator itor = owner->mRenderables.begin();
            RenderableArray::iterator end  = owner->mRenderables.end();

            while( itor != end )
            {
                const FastArray<Real> *lodVec = (*itor)->mLodMaterial;
                FastArray<Real>::const_iterator it = std::lower_bound( lodVec->begin(), lodVec->end(),
                                                                       lodValues[j] );
                (*itor)->mCurrentMaterialLod = (uint8)std::max<int>( it - lodVec->begin() - 1, 0 );
                ++itor;
            }
        }
    }
}
