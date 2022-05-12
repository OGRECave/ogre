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
#ifndef _OgreTangentSpaceCalc_H_
#define _OgreTangentSpaceCalc_H_

#include "OgrePrerequisites.h"
#include "OgreRenderOperation.h"
#include "OgreVector.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Math
    *  @{
    */
    /** Class for calculating a tangent space basis.
    */
    class _OgreExport TangentSpaceCalc
    {
    public:
        TangentSpaceCalc();

        typedef std::pair<size_t, size_t> VertexSplit;

        /// Information about a remapped index
        struct IndexRemap
        {
            /// Index data set (can be >0 if more than one index data was added)
            size_t indexSet;
            /// The position in the index buffer that's affected
            size_t faceIndex;
            /// The old and new vertex index
            VertexSplit splitVertex;
        
            IndexRemap() {} // to keep container happy
            IndexRemap(size_t i, size_t f, const VertexSplit& s) : indexSet(i), faceIndex(f), splitVertex(s) {}
        };
        /** List of indexes that were remapped (split vertices).
        */
        typedef std::list<IndexRemap> IndexRemapList;

        typedef std::list<VertexSplit> VertexSplits;

        /// The result of having built a tangent space basis
        struct Result
        {
            /** A list of vertex indices which were split off into new vertices
                because of mirroring. First item in each pair is the source vertex 
                index, the second value is the split vertex index.
            */
            VertexSplits vertexSplits;
            /** A list of indexes which were affected by splits. You can use this if you have other
                triangle-based data which you will need to alter to match. */
            IndexRemapList indexesRemapped;
        };

        /// Reset the calculation object
        void clear();

        /** Set the incoming vertex data (which will be modified) */
        void setVertexData(VertexData* v_in);

        /** Add a set of index data that references the vertex data.
            This might be modified if there are vertex splits.
        */
        void addIndexData(IndexData* i_in, RenderOperation::OperationType opType = RenderOperation::OT_TRIANGLE_LIST);

        /** Sets whether to store tangent space parity in the W of a 4-component tangent or not.
        @remarks
            The default element format to use is VET_FLOAT3 which is enough to accurately 
            deal with tangents that do not involve any texture coordinate mirroring. 
            If you wish to allow UV mirroring in your model, you must enable 4-component
            tangents using this method, and the 'w' coordinate will be populated
            with the parity of the triangle (+1 or -1), which will allow you to generate
            the bitangent properly.
        @param enabled true to enable 4-component tangents (default false). If you enable
            this, you will probably also want to enable mirror splitting (see setSplitMirrored), 
            and your shader must understand how to deal with the parity.
        */
        void setStoreParityInW(bool enabled) { mStoreParityInW = enabled; }

        /**  Gets whether to store tangent space parity in the W of a 4-component tangent or not. */
        bool getStoreParityInW() const { return mStoreParityInW; }

        /** Sets whether or not to split vertices when a mirrored tangent space
            transition is detected (matrix parity differs).
        @remarks
            This defaults to 'off' because it's the safest option; tangents will be
            interpolated in all cases even if they don't agree around a vertex, so
            artefacts will be smoothed out. When you're using art assets of 
            unknown quality this can avoid extra seams on the visible surface. 
            However, if your artists are good, they will be hiding texture seams
            in folds of the model and thus you can turn this option on, which will
            prevent the results of those seams from getting smoothed into other
            areas, which is exactly what you want.
        @note This option is automatically disabled if you provide any strip or
            fan based geometry.
        */
        void setSplitMirrored(bool split) { mSplitMirrored = split; }
        
        /** Gets whether or not to split vertices when a mirrored tangent space
            transition is detected.
        */
        bool getSplitMirrored() const { return mSplitMirrored; }

        /** Sets whether or not to split vertices when tangent space rotates
            more than 90 degrees around a vertex.
        @remarks
            This defaults to 'off' because it's the safest option; tangents will be
            interpolated in all cases even if they don't agree around a vertex, so
            artefacts will be smoothed out. When you're using art assets of 
            unknown quality this can avoid extra seams on the visible surface. 
            However, if your artists are good, they will be hiding texture inconsistencies
            in folds of the model and thus you can turn this option on, which will
            prevent the results of those seams from getting smoothed into other
            areas, which is exactly what you want.
        @note This option is automatically disabled if you provide any strip or
            fan based geometry.
        */
        void setSplitRotated(bool split) { mSplitRotated = split; }
        /** Sets whether or not to split vertices when tangent space rotates
        more than 90 degrees around a vertex.
        */
        bool getSplitRotated() const { return mSplitRotated; }

        /** Build a tangent space basis from the provided data.
        @remarks
            Only indexed triangle lists are allowed. Strips and fans cannot be
            supported because it may be necessary to split the geometry up to 
            respect deviances in the tangent space basis better.
        @param targetSemantic The semantic to store the tangents in. Defaults to 
            the explicit tangent binding, but note that this is only usable on more
            modern hardware (Shader Model 2), so if you need portability with older
            cards you should change this to a texture coordinate binding instead.
        @param sourceTexCoordSet The texture coordinate index which should be used as the source
            of 2D texture coordinates, with which to calculate the tangents.
        @param index The element index, ie the texture coordinate set which should be used to store the 3D
            coordinates representing a tangent vector per vertex, if targetSemantic is 
            VES_TEXTURE_COORDINATES. If this already exists, it will be overwritten.
        @return
            A structure containing the results of the tangent space build. Vertex data
            will always be modified but it's also possible that the index data
            could be adjusted. This happens when mirroring is used on a mesh, which
            causes the tangent space to be inverted on opposite sides of an edge.
            This is discontinuous, therefore the vertices have to be split along
            this edge, resulting in new vertices.
        */
        Result build(VertexElementSemantic targetSemantic = VES_TANGENT,
            unsigned short sourceTexCoordSet = 0, unsigned short index = 1);


    private:

        VertexData* mVData;
        typedef std::vector<IndexData*> IndexDataList;
        typedef std::vector<RenderOperation::OperationType> OpTypeList;
        IndexDataList mIDataList;
        OpTypeList mOpTypes;
        bool mSplitMirrored;
        bool mSplitRotated;
        bool mStoreParityInW;


        struct VertexInfo
        {
            Vector3 pos;
            Vector3 norm;
            Vector2 uv;
            Vector3 tangent;
            Vector3 binormal;
            // Which way the tangent space is oriented (+1 / -1) (set on first time found)
            int parity;
            // What index the opposite parity vertex copy is at (0 if not created yet)
            size_t oppositeParityIndex;

            VertexInfo() : tangent(Vector3::ZERO), binormal(Vector3::ZERO), 
                parity(0), oppositeParityIndex(0) {}
        };
        typedef std::vector<VertexInfo> VertexInfoArray;
        VertexInfoArray mVertexArray;

        void extendBuffers(VertexSplits& splits);
        void insertTangents(Result& res,
            VertexElementSemantic targetSemantic, 
            unsigned short sourceTexCoordSet, unsigned short index);

        void populateVertexArray(unsigned short sourceTexCoordSet);
        void processFaces(Result& result);
        /// Calculate face tangent space, U and V are weighted by UV area, N is normalised
        void calculateFaceTangentSpace(const size_t* vertInd, Vector3& tsU, Vector3& tsV, Vector3& tsN);
        Real calculateAngleWeight(size_t v0, size_t v1, size_t v2);
        int calculateParity(const Vector3& u, const Vector3& v, const Vector3& n);
        void addFaceTangentSpaceToVertices(size_t indexSet, size_t faceIndex, size_t *localVertInd, 
            const Vector3& faceTsU, const Vector3& faceTsV, const Vector3& faceNorm, Result& result);
        void normaliseVertices();
        void remapIndexes(Result& res);
        template <typename T>
        void remapIndexes(T* ibuf, size_t indexSet, Result& res)
        {
            for (IndexRemapList::iterator i = res.indexesRemapped.begin();
                i != res.indexesRemapped.end(); ++i)
            {
                IndexRemap& remap = *i;

                // Note that because this is a vertex split situation, and vertex
                // split is only for some faces, it's not a case of replacing all
                // instances of vertex index A with vertex index B
                // It actually matters which triangle we're talking about, so drive
                // the update from the face index

                if (remap.indexSet == indexSet)
                {
                    T* pBuf;
                    pBuf = ibuf + remap.faceIndex * 3;

                    for (int v = 0; v < 3; ++v, ++pBuf)
                    {
                        if (*pBuf == remap.splitVertex.first)
                        {
                            *pBuf = (T)remap.splitVertex.second;
                        }
                    }
                }


            }
        }
        

    };
    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
