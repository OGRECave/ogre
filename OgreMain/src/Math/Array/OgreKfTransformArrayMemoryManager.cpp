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

#include "OgreStableHeaders.h"

#include "Math/Array/OgreKfTransformArrayMemoryManager.h"

#include "Math/Array/OgreKfTransform.h"

namespace Ogre
{
    const size_t KfTransformArrayMemoryManager::ElementsMemSize[KfTransformArrayMemoryManager::NumMemoryTypes] =
    {
        (3 + 4 + 3) * sizeof( Ogre::Real ),     //ArrayMemoryManager::KfTransformType
    };
    //-----------------------------------------------------------------------------------
    KfTransformArrayMemoryManager::KfTransformArrayMemoryManager( uint16 depthLevel, size_t hintMaxNodes,
                                                    size_t cleanupThreshold, size_t maxHardLimit,
                                                    RebaseListener *rebaseListener ) :
            ArrayMemoryManager( ElementsMemSize, 0, 0,
                                sizeof( ElementsMemSize ) / sizeof( size_t ), depthLevel,
                                hintMaxNodes, cleanupThreshold, maxHardLimit, rebaseListener )
    {
    }
    //-----------------------------------------------------------------------------------
    void KfTransformArrayMemoryManager::createNewNode( KfTransform **outTransform )
    {
        const size_t nextSlot = createNewSlot();
        for( size_t i=0; i<ARRAY_PACKED_REALS - 1; ++i )
            createNewSlot();

        const unsigned char nextSlotIdx = nextSlot % ARRAY_PACKED_REALS;
        const size_t nextSlotBase       = nextSlot - nextSlotIdx;

        //Set memory ptrs
        *outTransform = reinterpret_cast<KfTransform*>( mMemoryPools[KfTransformType] +
                                                    nextSlotBase * mElementsMemSizes[KfTransformType] );

        //Set default values
        (*outTransform)->mPosition      = ArrayVector3::ZERO;
        (*outTransform)->mOrientation   = ArrayQuaternion::IDENTITY;
        (*outTransform)->mScale         = ArrayVector3::UNIT_SCALE;
    }
}
