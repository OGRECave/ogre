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
#include "OgreSubMesh.h"

#include "OgreMesh.h"
#include "OgreException.h"
#include "OgreMaterialManager.h"
#include "OgreHardwareBufferManager.h"

#include "Vao/OgreVaoManager.h"

namespace Ogre {
namespace v1 {
    //-----------------------------------------------------------------------
    SubMesh::SubMesh()
        : useSharedVertices(true)
        , renderOpMeshIndex(++RenderOperation::MeshIndexId)
        , operationType(RenderOperation::OT_TRIANGLE_LIST)
        , vertexData(0)
        , parent(0)
        , mMatInitialised(false)
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

        removeLodLevels();
    }

    //-----------------------------------------------------------------------
    void SubMesh::setMaterialName( const String& name, const String& groupName /* = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME */)
    {
        mMaterialName = name;
        mMatInitialised = true;
    }
    //-----------------------------------------------------------------------
    const String& SubMesh::getMaterialName() const
    {
        return mMaterialName;
    }
    //-----------------------------------------------------------------------
    bool SubMesh::isMatInitialised(void) const
    {
        return mMatInitialised;

    }
    //-----------------------------------------------------------------------
    void SubMesh::_getRenderOperation(RenderOperation& ro, ushort lodIndex)
    {
        assert( !lodIndex || (lodIndex-1) < mLodFaceList.size() );

        ro.useIndexes = indexData->indexCount != 0;
        if (lodIndex > 0)
        {
            // lodIndex - 1 because we don't store full detail version in mLodFaceList
            ro.indexData = mLodFaceList[lodIndex-1];
        }
        else
        {
            ro.indexData = indexData;
        }
        ro.operationType = operationType;
        ro.vertexData = useSharedVertices? parent->sharedVertexData : vertexData;
        ro.meshIndex = renderOpMeshIndex;
    }
    //-----------------------------------------------------------------------
    void SubMesh::addBoneAssignment(const VertexBoneAssignment& vertBoneAssign)
    {
        if (useSharedVertices)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "This SubMesh uses shared geometry,  you "
                "must assign bones to the Mesh, not the SubMesh", "SubMesh.addBoneAssignment");
        }
        mBoneAssignments.insert(
            VertexBoneAssignmentList::value_type(vertBoneAssign.vertexIndex, vertBoneAssign));
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
    void SubMesh::removeTextureAlias(const String& aliasName)
    {
        mTextureAliases.erase(aliasName);
    }
    //---------------------------------------------------------------------
    void SubMesh::removeAllTextureAliases(void)
    {
        mTextureAliases.clear();
    }
    //---------------------------------------------------------------------
    bool SubMesh::updateMaterialUsingTextureAliases(void)
    {
        bool newMaterialCreated = false;
        // if submesh has texture aliases
        // ask the material manager if the current summesh material exists
        if (hasTextureAliases() && MaterialManager::getSingleton().resourceExists(mMaterialName))
        {
            // get the current submesh material
            MaterialPtr material = MaterialManager::getSingleton().getByName( mMaterialName );
            // get test result for if change will occur when the texture aliases are applied
            if (material->applyTextureAliases(mTextureAliases, false))
            {
                Ogre::String newMaterialName;

                // If this material was already derived from another material
                // due to aliasing, let's strip off the aliasing suffix and
                // generate a new one using our current aliasing table.

                Ogre::String::size_type pos = mMaterialName.find("?TexAlias(", 0);
                if( pos != Ogre::String::npos )
                    newMaterialName = mMaterialName.substr(0, pos);
                else
                    newMaterialName = mMaterialName;

                newMaterialName += "?TexAlias(";
                // Iterate deterministically over the aliases (always in the same
                // order via std::map's sorted iteration nature).
                AliasTextureIterator aliasIter = getAliasTextureIterator();
                while( aliasIter.hasMoreElements() )
                {
                    newMaterialName += aliasIter.peekNextKey();
                    newMaterialName += "=";
                    newMaterialName += aliasIter.getNext();
                    newMaterialName += " ";
                }
                newMaterialName += ")";
                    
                // Reuse the material if it's already been created. This decreases batch
                // count and keeps material explosion under control.
                if(!MaterialManager::getSingleton().resourceExists(newMaterialName))
                {
                    Ogre::MaterialPtr newMaterial = Ogre::MaterialManager::getSingleton().create(
                        newMaterialName, material->getGroup());
                    // copy parent material details to new material
                    material->copyDetailsTo(newMaterial);
                    // apply texture aliases to new material
                    newMaterial->applyTextureAliases(mTextureAliases);
                }
                // place new material name in submesh
                setMaterialName(newMaterialName);
                newMaterialCreated = true;
            }
        }

        return newMaterialCreated;
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
        set<uint32>::type mIndices;

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

            for (set<uint32>::type::const_iterator i = mIndices.begin ();
                 i != mIndices.end (); ++i)
            {
                float *v;
                poselem->baseVertexPointerToElement (vdata + *i * vsz, &v);
                extend (v);
            }
        }

        Cluster split (int split_axis, const VertexElement *poselem,
                       uint8 *vdata, size_t vsz)
        {
            Real r = (mMin [split_axis] + mMax [split_axis]) * 0.5f;
            Cluster newbox;

            // Separate all points that are inside the new bbox
            for (set<uint32>::type::iterator i = mIndices.begin ();
                 i != mIndices.end (); )
            {
                float *v;
                poselem->baseVertexPointerToElement (vdata + *i * vsz, &v);
                if (v [split_axis] > r)
                {
                    newbox.mIndices.insert (*i);
                    set<uint32>::type::iterator x = i++;
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

        vector<Cluster>::type boxes;
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
            for (vector<Cluster>::type::iterator b = boxes.begin ();
                 b != boxes.end (); ++b)
            {
                if (b->empty ())
                    continue;
                Real v = b->volume ();
                if (v > split_volume)
                {
                    split_volume = v;
                    split_box = &*b;
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
        for (vector<Cluster>::type::const_iterator b = boxes.begin ();
             b != boxes.end (); ++b)
        {
            Real rating = 0;
            Vector3 best_vertex;

            for (set<uint32>::type::const_iterator i = b->mIndices.begin ();
                 i != b->mIndices.end (); ++i)
            {
                float *v;
                poselem->baseVertexPointerToElement (vdata + *i * vsz, &v);

                Vector3 vv (v [0], v [1], v [2]);
                Real r = (vv - center).squaredLength ();

                for (vector<Vector3>::type::const_iterator e = extremityPoints.begin ();
                     e != extremityPoints.end (); ++e)
                    r += (*e - vv).squaredLength ();
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

        SubMesh* newSub;
        if(!parentMesh)
            newSub = parent->createSubMesh(newName);
        else
            newSub = parentMesh->createSubMesh(newName);

        newSub->mMaterialName = this->mMaterialName;
        newSub->mMatInitialised = this->mMatInitialised;
        newSub->operationType = this->operationType;
        newSub->useSharedVertices = this->useSharedVertices;
        newSub->extremityPoints = this->extremityPoints;

        if (!this->useSharedVertices)
        {
            // Copy unique vertex data
            newSub->vertexData = this->vertexData->clone();
            // Copy unique index map
            newSub->blendIndexToBoneIndexMap = this->blendIndexToBoneIndexMap;
        }

        // Copy index data
        OGRE_DELETE newSub->indexData;
        newSub->indexData = this->indexData->clone();
        // Copy any bone assignments
        newSub->mBoneAssignments = this->mBoneAssignments;
        newSub->mBoneAssignmentsOutOfDate = this->mBoneAssignmentsOutOfDate;
        // Copy texture aliases
        newSub->mTextureAliases = this->mTextureAliases;

        // Copy lod face lists
        newSub->mLodFaceList.reserve(this->mLodFaceList.size());
        SubMesh::LODFaceList::const_iterator facei;
        for (facei = this->mLodFaceList.begin(); facei != this->mLodFaceList.end(); ++facei) {
            IndexData* newIndexData = (*facei)->clone();
            newSub->mLodFaceList.push_back(newIndexData);
        }
        return newSub;
    }
    //---------------------------------------------------------------------
    void SubMesh::arrangeEfficientForOldInterface( bool halfPos, bool halfTexCoords )
    {
        char *data = arrangeEfficient( halfPos, halfTexCoords, 0 );

        HardwareBufferManagerBase *hwManager = vertexData->_getHardwareBufferManager();
        vertexData->vertexBufferBinding = hwManager->createVertexBufferBinding();
        HardwareVertexBufferSharedPtr vbuf = hwManager->createVertexBuffer(
                                                vertexData->vertexDeclaration->getVertexSize( 0 ),
                                                vertexData->vertexCount,
                                                parent->mVertexBufferUsage,
                                                parent->mVertexBufferShadowBuffer );
        vertexData->vertexBufferBinding->setBinding( 0, vbuf );

        void *dstBuffer = vbuf->lock( HardwareBuffer::HBL_DISCARD );
        memcpy( dstBuffer, data, vbuf->getSizeInBytes() );
        vbuf->unlock();

        OGRE_FREE_SIMD( data, MEMCATEGORY_GEOMETRY );
    }
    //---------------------------------------------------------------------
    bool sortVertexElementsBySemantic2( const VertexElement2 &l, const VertexElement2 &r )
    {
        return l.mSemantic < r.mSemantic;
    }
    bool sortVertexElementsBySemantic( const VertexElement &l, const VertexElement &r )
    {
        return l.getSemantic() < r.getSemantic() && l.getIndex() < l.getIndex();
    }

    char* SubMesh::arrangeEfficient( bool halfPos, bool halfTexCoords,
                                     VertexElement2Vec *outVertexElements )
    {
        typedef FastArray<VertexElement> VertexElementArray;

        VertexElement2Vec vertexElements;
        VertexElementArray srcElements;
        bool hasTangents = false;

        VertexElement const *tangentElement  = 0;
        VertexElement const *binormalElement = 0;

        {
            //Get an AZDO-friendly vertex declaration out of the original declaration.
            const VertexDeclaration::VertexElementList &origElements = vertexData->vertexDeclaration->
                                                                                        getElements();
            srcElements.reserve( origElements.size() );
            VertexDeclaration::VertexElementList::const_iterator itor = origElements.begin();
            VertexDeclaration::VertexElementList::const_iterator end  = origElements.end();

            while( itor != end )
            {
                const VertexElement &origElement = *itor;

                if( origElement.getSemantic() == VES_TANGENT ||
                    origElement.getSemantic() == VES_BINORMAL )
                {
                    hasTangents = true;
                    if( origElement.getSemantic() == VES_TANGENT )
                        tangentElement = &origElement;
                    else
                        binormalElement = &origElement;
                }
                else
                {
                    vertexElements.push_back( VertexElement2( origElement.getType(),
                                                              origElement.getSemantic() ) );
                    srcElements.push_back( *itor );
                }

                if( origElement.getBaseType( origElement.getType() ) != VET_FLOAT1 )
                {
                    //We can't convert to half if it wasn't in floating point
                    if( origElement.getSemantic() == VES_POSITION )
                        halfPos = false;
                    else if( origElement.getSemantic() == VES_TEXTURE_COORDINATES )
                        halfTexCoords = false;
                }
                else if( origElement.getSemantic() == VES_POSITION )
                {
                    //All attributes must be aligned at least to 4 bytes.
                    VertexElement2 &lastInserted = vertexElements.back();
                    lastInserted.mType = VET_HALF4;
                }

                ++itor;
            }

            //If it has tangents, prepare the normal to hold QTangents.
            if( hasTangents == true )
            {
                VertexElement2Vec::iterator it = std::find( vertexElements.begin(),
                                                            vertexElements.end(),
                                                            VertexElement2( VET_FLOAT3,
                                                                            VES_NORMAL ) );
                if( it != vertexElements.end() )
                    it->mType = VET_SHORT4;
            }
        }

        std::sort( vertexElements.begin(), vertexElements.end(), sortVertexElementsBySemantic2 );
        std::sort( srcElements.begin(), srcElements.end(), sortVertexElementsBySemantic );

        //Prepare for the transfer between buffers.
        size_t vertexSize = VaoManager::calculateVertexSize( vertexElements );
        char *data = static_cast<char*>( OGRE_MALLOC_SIMD( vertexSize * vertexData->vertexCount,
                                                           MEMCATEGORY_GEOMETRY ) );

        FastArray<char*> srcPtrs;
        FastArray<size_t> vertexBuffSizes;
        srcPtrs.reserve( vertexData->vertexBufferBinding->getBufferCount() );
        for( size_t i=0; i<vertexData->vertexBufferBinding->getBufferCount(); ++i )
        {
            const HardwareVertexBufferSharedPtr &vBuffer = vertexData->vertexBufferBinding->
                                                                                getBuffer( i );
            srcPtrs.push_back( static_cast<char*>( vBuffer->lock( HardwareBuffer::HBL_READ_ONLY ) ) );
            vertexBuffSizes.push_back( vBuffer->getVertexSize() );
        }

        //Perform the transfer. Note that vertexElements & srcElements do not match.
        //As vertexElements is modified for smaller types and may include padding
        //for alignment reasons.
        for( size_t i=0; i<vertexData->vertexCount; ++i )
        {
            size_t acumOffset = 0;
            VertexElement2Vec::const_iterator itor = vertexElements.begin();
            VertexElement2Vec::const_iterator end  = vertexElements.end();
            VertexElementArray::const_iterator itSrc = srcElements.begin();

            while( itor != end )
            {
                const VertexElement2 &vElement = *itor;
                size_t veSize = VertexElement::getTypeSize( itSrc->getType() );

                if( (halfPos && vElement.mSemantic == VES_POSITION) ||
                    (halfTexCoords && vElement.mSemantic == VES_TEXTURE_COORDINATES) )
                {
                    //Convert float to half.
                    float fpData[4];
                    fpData[0] = fpData[1] = fpData[2] = 0.0f;
                    fpData[3] = 1.0f;
                    memcpy( fpData, srcPtrs[itSrc->getSource()] + itSrc->getOffset(), veSize );

                    for( size_t j=0; j<VertexElement::getTypeCount( vElement.mType ); ++j )
                    {
                        *reinterpret_cast<uint16*>(data + acumOffset + j) =
                                                            Bitwise::floatToHalf( fpData[j] );
                    }
                }
                else if( vElement.mSemantic == VES_NORMAL && hasTangents &&
                         vElement.mType == VET_FLOAT3 )
                {
                    //Convert TBN matrix (between 6 to 9 floats, 24-36 bytes)
                    //to a QTangent (4 shorts, 8 bytes)
                    assert( veSize == sizeof(float) * 3 );
                    assert( tangentElement->getSize() < sizeof(float) * 4 &&
                            tangentElement->getSize() >= sizeof(float) * 3 );

                    float normal[3];
                    float tangent[4];
                    tangent[3] = 1.0f;
                    memcpy( normal, srcPtrs[itSrc->getSource()] + itSrc->getOffset(), veSize );
                    memcpy( tangent,
                            srcPtrs[tangentElement->getSource()] + tangentElement->getOffset(),
                            tangentElement->getSize() );

                    Vector3 vNormal( normal[0], normal[1], normal[2] );
                    Vector3 vTangent( tangent[0], tangent[1], tangent[2] );

                    if( binormalElement )
                    {
                        assert( binormalElement->getSize() == sizeof(float) * 3 );
                        float binormal[3];
                        memcpy( binormal,
                                srcPtrs[binormalElement->getSource()] + binormalElement->getOffset(),
                                binormalElement->getSize() );

                        Vector3 vBinormal( binormal[0], binormal[1], binormal[2] );

                        //It is reflected.
                        Vector3 naturalBinormal = vTangent.crossProduct( vNormal );
                        if( naturalBinormal.dotProduct( vBinormal ) <= 0 )
                            tangent[3] = -1.0f;
                    }

                    Matrix3 tbn;
                    tbn.SetColumn( 0, vNormal );
                    tbn.SetColumn( 1, vTangent );
                    tbn.SetColumn( 2, vTangent.crossProduct( vNormal ) );

                    //See Spherical Skinning with Dual-Quaternions and QTangents,
                    //Ivo Zoltan Frey, SIGRAPH 2011 Vancounver.
                    //http://www.crytek.com/download/izfrey_siggraph2011.ppt

                    Quaternion qTangent( tbn );
                    qTangent.normalise();

                    //Bias = 1 / [2^(bits-1) - 1]
                    const Real bias = 1.0f / 32767.0f;

                    //Make sure QTangent is always positive
                    //Because '-0' sign information is lost when using integers,
                    //we need to apply a "bias"; while making sure the Quatenion
                    //stays normalized.
                    //if( qTangent.w < 0 )
                    //  qTangent = -qTangent;
                    if( qTangent.w < bias )
                    {
                        Real normFactor = Math::Sqrt( 1 - bias * bias );
                        qTangent.w = bias;
                        qTangent.x *= normFactor;
                        qTangent.y *= normFactor;
                        qTangent.z *= normFactor;
                    }

                    //Now negate if we require reflection
                    if( tangent[3] < 0 )
                        qTangent = -qTangent;

                    uint16 *dstData16 = reinterpret_cast<uint16*>(data + acumOffset);

                    dstData16[0] = Math::Clamp( qTangent.w * 32767.0f, -32768.0f, 32767.0f );
                    dstData16[1] = Math::Clamp( qTangent.x * 32767.0f, -32768.0f, 32767.0f );
                    dstData16[2] = Math::Clamp( qTangent.y * 32767.0f, -32768.0f, 32767.0f );
                    dstData16[3] = Math::Clamp( qTangent.z * 32767.0f, -32768.0f, 32767.0f );
                }
                else
                {
                    //Raw. Transfer as is.
                    memcpy( data + acumOffset,
                            srcPtrs[itSrc->getSource()] + itSrc->getOffset(),
                            veSize );
                }

                acumOffset += veSize;
                ++itor;
            }

            data += vertexSize;
            for( size_t j=0; j<srcPtrs.size(); ++j )
                srcPtrs[j] += vertexBuffSizes[j];
        }

        //Cleanup
        for( size_t i=0; i<vertexData->vertexBufferBinding->getBufferCount(); ++i )
            vertexData->vertexBufferBinding->getBuffer( i )->unlock();

        HardwareBufferManagerBase *hwManager = vertexData->_getHardwareBufferManager();
        hwManager->destroyVertexBufferBinding( vertexData->vertexBufferBinding );
        vertexData->vertexBufferBinding = 0;

        {
            //Recreate the vertex declaration
            vertexData->vertexDeclaration->removeAllElements();

            size_t acumOffset = 0;
            unsigned short repeatCounts[VES_COUNT];
            memset( repeatCounts, 0, sizeof( repeatCounts ) );
            VertexElement2Vec::const_iterator itor = vertexElements.begin();
            VertexElement2Vec::const_iterator end  = vertexElements.end();

            while( itor != end )
            {
                vertexData->vertexDeclaration->addElement( 0, acumOffset, itor->mType, itor->mSemantic,
                                                           repeatCounts[itor->mSemantic-1]++ );
                acumOffset += VertexElement::getTypeSize( itor->mType );
                ++itor;
            }
        }

        if( outVertexElements )
            outVertexElements->swap( vertexElements );

        return data;
    }
}
}
