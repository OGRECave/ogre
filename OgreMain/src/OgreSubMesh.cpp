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

namespace Ogre {
    //-----------------------------------------------------------------------
    SubMesh::SubMesh()
        : vertexData(0)
        , parent(0)
        , useSharedVertices(true)
        , operationType(RenderOperation::OT_TRIANGLE_LIST)
        , mBoneAssignmentsOutOfDate(false)
        , mVertexAnimationType(VAT_NONE)
        , mVertexAnimationIncludesNormals(false)
        , mBuildEdgesEnabled(true)
    {
        indexData = OGRE_NEW IndexData();
    }
    //-----------------------------------------------------------------------
    SubMesh::~SubMesh()
    {
        removeLodLevels();
        OGRE_DELETE vertexData;
        OGRE_DELETE indexData;
    }

    //-----------------------------------------------------------------------
    void SubMesh::setMaterialName( const String& name, const String& groupName /* = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME */)
    {
        mMaterial = MaterialManager::getSingleton().getByName(name, groupName);
    }
    //-----------------------------------------------------------------------
    const String& SubMesh::getMaterialName() const
    {
        return mMaterial ? mMaterial->getName() : BLANKSTRING;
    }
    //-----------------------------------------------------------------------
    void SubMesh::_getRenderOperation(RenderOperation& ro, ushort lodIndex)
    {
        if (lodIndex > 0 && static_cast< size_t >( lodIndex - 1 ) < mLodFaceList.size())
        {
            // lodIndex - 1 because we don't store full detail version in mLodFaceList
            ro.indexData = mLodFaceList[lodIndex-1];
        }
        else
        {
            ro.indexData = indexData;
        }
        ro.useIndexes = ro.indexData->indexCount != 0;
        ro.operationType = operationType;
        ro.vertexData = useSharedVertices? parent->sharedVertexData : vertexData;

    }
    //-----------------------------------------------------------------------
    void SubMesh::addBoneAssignment(const VertexBoneAssignment& vertBoneAssign)
    {
        OgreAssert(!useSharedVertices,
                   "This SubMesh uses shared geometry, you must assign bones to the Mesh, not the SubMesh");
        mBoneAssignments.emplace(vertBoneAssign.vertexIndex, vertBoneAssign);
        mBoneAssignmentsOutOfDate = true;
    }
    //-----------------------------------------------------------------------
    void SubMesh::clearBoneAssignments(void)
    {
        mBoneAssignments.clear();
        mBoneAssignmentsOutOfDate = true;
    }

    //-----------------------------------------------------------------------
    void SubMesh::_compileBoneAssignments(void)
    {
        unsigned short maxBones =
            parent->_rationaliseBoneAssignments(vertexData->vertexCount, mBoneAssignments);

        if (maxBones != 0)
        {
            parent->compileBoneAssignments(mBoneAssignments, maxBones, 
                blendIndexToBoneIndexMap, vertexData);
        }

        mBoneAssignmentsOutOfDate = false;
    }
    //---------------------------------------------------------------------
    SubMesh::BoneAssignmentIterator SubMesh::getBoneAssignmentIterator(void)
    {
        return BoneAssignmentIterator(mBoneAssignments.begin(),
            mBoneAssignments.end());
    }
    //---------------------------------------------------------------------
    SubMesh::AliasTextureIterator SubMesh::getAliasTextureIterator(void) const
    {
        return AliasTextureIterator(mTextureAliases.begin(),
            mTextureAliases.end());
    }
    //---------------------------------------------------------------------
    void SubMesh::addTextureAlias(const String& aliasName, const String& textureName)
    {
        mTextureAliases[aliasName] = textureName;
    }
    //---------------------------------------------------------------------
    void SubMesh::removeAllTextureAliases(void)
    {
        mTextureAliases.clear();
    }
    //---------------------------------------------------------------------
    void SubMesh::removeLodLevels(void)
    {
        LODFaceList::iterator lodi, lodend;
        lodend = mLodFaceList.end();
        for (lodi = mLodFaceList.begin(); lodi != lodend; ++lodi)
        {
            OGRE_DELETE *lodi;
        }

        mLodFaceList.clear();

    }
    //---------------------------------------------------------------------
    VertexAnimationType SubMesh::getVertexAnimationType(void) const
    {
        if(parent->_getAnimationTypesDirty())
        {
            parent->_determineAnimationTypes();
        }
        return mVertexAnimationType;
    }
    //---------------------------------------------------------------------
    /* To find as many points from different domains as we need,
     * such that those domains are from different parts of the mesh,
     * we implement a simplified Heckbert quantization algorithm.
     *
     * This struct is like AxisAlignedBox with some specialized methods
     * for doing quantization.
     */
    struct Cluster
    {
        Vector3 mMin, mMax;
        std::set<uint32> mIndices;

        Cluster ()
        { }

        bool empty () const
        {
            if (mIndices.empty ())
                return true;
            if (mMin == mMax)
                return true;
            return false;
        }

        float volume () const
        {
            return (mMax.x - mMin.x) * (mMax.y - mMin.y) * (mMax.z - mMin.z);
        }

        void extend (float *v)
        {
            if (v [0] < mMin.x) mMin.x = v [0];
            if (v [1] < mMin.y) mMin.y = v [1];
            if (v [2] < mMin.z) mMin.z = v [2];
            if (v [0] > mMax.x) mMax.x = v [0];
            if (v [1] > mMax.y) mMax.y = v [1];
            if (v [2] > mMax.z) mMax.z = v [2];
        }

        void computeBBox (const VertexElement *poselem, uint8 *vdata, size_t vsz)
        {
            mMin.x = mMin.y = mMin.z = Math::POS_INFINITY;
            mMax.x = mMax.y = mMax.z = Math::NEG_INFINITY;

            for (unsigned int idx : mIndices)
            {
                float *v;
                poselem->baseVertexPointerToElement (vdata + idx * vsz, &v);
                extend (v);
            }
        }

        Cluster split (int split_axis, const VertexElement *poselem,
                       uint8 *vdata, size_t vsz)
        {
            Real r = (mMin [split_axis] + mMax [split_axis]) * 0.5f;
            Cluster newbox;

            // Separate all points that are inside the new bbox
            for (std::set<uint32>::iterator i = mIndices.begin ();
                 i != mIndices.end (); )
            {
                float *v;
                poselem->baseVertexPointerToElement (vdata + *i * vsz, &v);
                if (v [split_axis] > r)
                {
                    newbox.mIndices.insert (*i);
                    std::set<uint32>::iterator x = i++;
                    mIndices.erase(x);
                }
                else
                    ++i;
            }

            computeBBox (poselem, vdata, vsz);
            newbox.computeBBox (poselem, vdata, vsz);

            return newbox;
        }
    };
    //---------------------------------------------------------------------
    void SubMesh::generateExtremes(size_t count)
    {
        extremityPoints.clear();

		if (count == 0)
			return;

        /* Currently this uses just one criteria: the points must be
         * as far as possible from each other. This at least ensures
         * that the extreme points characterise the submesh as
         * detailed as it's possible.
         */

        VertexData *vert = useSharedVertices ?
            parent->sharedVertexData : vertexData;
        const VertexElement *poselem = vert->vertexDeclaration->
            findElementBySemantic (VES_POSITION);
        HardwareVertexBufferSharedPtr vbuf = vert->vertexBufferBinding->
            getBuffer (poselem->getSource ());
        uint8 *vdata = (uint8 *)vbuf->lock (HardwareBuffer::HBL_READ_ONLY);
        size_t vsz = vbuf->getVertexSize ();

        std::vector<Cluster> boxes;
        boxes.reserve (count);

        // First of all, find min and max bounding box of the submesh
        boxes.push_back (Cluster ());

        if (indexData->indexCount > 0)
        {

            uint elsz = indexData->indexBuffer->getType () == HardwareIndexBuffer::IT_32BIT ?
                4 : 2;
            uint8 *idata = (uint8 *)indexData->indexBuffer->lock (
                indexData->indexStart * elsz, indexData->indexCount * elsz,
                HardwareIndexBuffer::HBL_READ_ONLY);

            for (size_t i = 0; i < indexData->indexCount; i++)
            {
                int idx = (elsz == 2) ? ((uint16 *)idata) [i] : ((uint32 *)idata) [i];
                boxes [0].mIndices.insert (idx);
            }
            indexData->indexBuffer->unlock ();

        }
        else
        {
            // just insert all indexes
            for (size_t i = vertexData->vertexStart; i < vertexData->vertexCount; i++)
            {
                boxes [0].mIndices.insert (static_cast<int>(i));
            }

        }

        boxes [0].computeBBox (poselem, vdata, vsz);

        // Remember the geometrical center of the submesh
        Vector3 center = (boxes [0].mMax + boxes [0].mMin) * 0.5;

        // Ok, now loop until we have as many boxes, as we need extremes
        while (boxes.size () < count)
        {
            // Find the largest box with more than one vertex :)
            Cluster *split_box = NULL;
            Real split_volume = -1;
            for (auto & boxe : boxes)
            {
                if (boxe.empty ())
                    continue;
                Real v = boxe.volume ();
                if (v > split_volume)
                {
                    split_volume = v;
                    split_box = &boxe;
                }
            }

            // If we don't have what to split, break
            if (!split_box)
                break;

            // Find the coordinate axis to split the box into two
            int split_axis = 0;
            Real split_length = split_box->mMax.x - split_box->mMin.x;
            for (int i = 1; i < 3; i++)
            {
                Real l = split_box->mMax [i] - split_box->mMin [i];
                if (l > split_length)
                {
                    split_length = l;
                    split_axis = i;
                }
            }

            // Now split the box into halves
            boxes.push_back (split_box->split (split_axis, poselem, vdata, vsz));
        }

        // Fine, now from every cluster choose the vertex that is most
        // distant from the geometrical center and from other extremes.
        for (const auto & boxe : boxes)
        {
            Real rating = 0;
            Vector3 best_vertex;

            for (unsigned int i : boxe.mIndices)
            {
                float *v;
                poselem->baseVertexPointerToElement (vdata + i * vsz, &v);

                Vector3 vv (v [0], v [1], v [2]);
                Real r = (vv - center).squaredLength ();

                for (auto extremityPoint : extremityPoints)
                    r += (extremityPoint - vv).squaredLength ();
                if (r > rating)
                {
                    rating = r;
                    best_vertex = vv;
                }
            }

            if (rating > 0)
                extremityPoints.push_back (best_vertex);
        }

        vbuf->unlock ();
    }
    //---------------------------------------------------------------------
    void SubMesh::setBuildEdgesEnabled(bool b)
    {
        mBuildEdgesEnabled = b;
        if(parent)
        {
            parent->freeEdgeList();
            parent->setAutoBuildEdgeLists(true);
        }
    }
    //---------------------------------------------------------------------
    SubMesh * SubMesh::clone(const String& newName, Mesh *parentMesh)
    {
        // This is a bit like a copy constructor, but with the additional aspect of registering the clone with
        //  the MeshManager

        if(parentMesh == NULL)
            parentMesh = parent;

        HardwareBufferManagerBase* bufferManager = parentMesh->getHardwareBufferManager();
        SubMesh* newSub = parentMesh->createSubMesh(newName);

        newSub->mMaterial = this->mMaterial;
        newSub->operationType = this->operationType;
        newSub->useSharedVertices = this->useSharedVertices;
        newSub->extremityPoints = this->extremityPoints;

        if (!this->useSharedVertices)
        {
            // Copy unique vertex data
            newSub->vertexData = this->vertexData->clone(true, bufferManager);
            // Copy unique index map
            newSub->blendIndexToBoneIndexMap = this->blendIndexToBoneIndexMap;
        }

        // Copy index data
        OGRE_DELETE newSub->indexData;
        newSub->indexData = this->indexData->clone(true, bufferManager);
        // Copy any bone assignments
        newSub->mBoneAssignments = this->mBoneAssignments;
        newSub->mBoneAssignmentsOutOfDate = this->mBoneAssignmentsOutOfDate;
        // Copy texture aliases
        newSub->mTextureAliases = this->mTextureAliases;

        // Copy lod face lists
        newSub->mLodFaceList.reserve(this->mLodFaceList.size());
        SubMesh::LODFaceList::const_iterator facei;
        for (facei = this->mLodFaceList.begin(); facei != this->mLodFaceList.end(); ++facei) {
            IndexData* newIndexData = (*facei)->clone(true, bufferManager);
            newSub->mLodFaceList.push_back(newIndexData);
        }
        return newSub;
    }
}


