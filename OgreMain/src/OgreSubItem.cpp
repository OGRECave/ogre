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
#include "OgreSubItem.h"

#include "OgreItem.h"
#include "OgreMaterialManager.h"
#include "OgreSubMesh2.h"
#include "OgreLogManager.h"
#include "OgreHlmsDatablock.h"
#include "OgreException.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    SubItem::SubItem( Item* parent, SubMesh* subMeshBasis ) :
        RenderableAnimated(),
        mParentItem( parent ),
        mSubMesh( subMeshBasis )
    {
        //mMaterialPtr = MaterialManager::getSingleton().getByName(mMaterialName, subMeshBasis->parent->getGroup());
        mMaterialLodIndex = 0;
        mVaoPerLod[VpNormal] = subMeshBasis->mVao[VpNormal];
        mVaoPerLod[VpShadow] = subMeshBasis->mVao[VpShadow];

        if( !subMeshBasis->mBlendIndexToBoneIndexMap.empty() )
        {
            mHasSkeletonAnimation = true;
            mBlendIndexToBoneIndexMap = &subMeshBasis->mBlendIndexToBoneIndexMap;
        }
    }
    //-----------------------------------------------------------------------
    SubItem::~SubItem()
    {
    }
    //-----------------------------------------------------------------------
    SubMesh* SubItem::getSubMesh(void) const
    {
        return mSubMesh;
    }
    //-----------------------------------------------------------------------------
    void SubItem::_setHlmsHashes( uint32 hash, uint32 casterHash )
    {
        if( mHlmsDatablock->getAlphaTest() != CMPF_ALWAYS_PASS )
        {
            if( mVaoPerLod[VpShadow].empty() || mVaoPerLod[VpShadow][0] != mSubMesh->mVao[VpNormal][0] )
            {
                //Has alpha testing. Disable the optimized shadow mapping buffers.
                mVaoPerLod[VpShadow] = mSubMesh->mVao[VpNormal];
            }
        }
        else
        {
            if( mVaoPerLod[VpShadow].empty() || mVaoPerLod[VpShadow][0] != mSubMesh->mVao[VpShadow][0] )
            {
                //Restore the optimized shadow mapping buffers.
                mVaoPerLod[VpShadow] = mSubMesh->mVao[VpShadow];
            }
        }

        Renderable::_setHlmsHashes( hash, casterHash );
    }
    //-----------------------------------------------------------------------
    const LightList& SubItem::getLights(void) const
    {
        return mParentItem->queryLights();
    }
    //-----------------------------------------------------------------------------
    void SubItem::getRenderOperation( v1::RenderOperation& op, bool casterPass )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                     "Items do not implement getRenderOperation. You've put an Item in "
                     "the wrong RenderQueue ID (which is set to be compatible with "
                     "v1::Entity). Do not mix Items and Entities",
                     "SubItem::getRenderOperation" );
    }
    //-----------------------------------------------------------------------------
    void SubItem::getWorldTransforms(Matrix4* xform) const
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                     "Items do not implement getWorldTransforms. You've put an Item in "
                     "the wrong RenderQueue ID (which is set to be compatible with "
                     "v1::Entity). Do not mix Items and Entities",
                     "SubItem::getWorldTransforms" );
    }
    //-----------------------------------------------------------------------------
    bool SubItem::getCastsShadows(void) const
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                     "Items do not implement getCastsShadows. You've put an Item in "
                     "the wrong RenderQueue ID (which is set to be compatible with "
                     "v1::Entity). Do not mix Items and Entities",
                     "SubItem::getCastsShadows" );
    }
}
