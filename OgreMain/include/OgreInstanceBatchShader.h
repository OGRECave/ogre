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
#ifndef __InstanceBatchShader_H__
#define __InstanceBatchShader_H__

#include "OgreInstanceBatch.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */

    /** This is the same technique the old InstancedGeometry implementation used (with improvements).
        Basically it creates a large vertex buffer with many repeating entities, and sends per instance
        data through shader constants. Because SM 2.0 & 3.0 have up to 256 shader constant registers,
        this means there can be approx up to 84 instances per batch, assuming they're not skinned
        But using shader constants for other stuff (i.e. lighting) also affects negatively this number
        A mesh with skeletally animated 2 bones reduces the number 84 to 42 instances per batch.

        The main advantage of this technique is that it's supported on a high variety of hardware
        (SM 2.0 cards are required) and the same shader can be used for both skeletally animated
        normal entities and instanced entities without a single change required.

        Unlike the old @c InstancedGeometry implementation, the developer doesn't need to worry about
        reaching the 84 instances limit, the InstanceManager automatically takes care of splitting
        and creating new batches. But beware internally, this means less performance improvement.
        Another improvement is that vertex buffers are shared between batches, which significantly
        reduces GPU VRAM usage.
     */
    class _OgreExport InstanceBatchShader : public InstanceBatch
    {
        size_t  mNumWorldMatrices;

        void setupVertices( const SubMesh* baseSubMesh ) override;
        void setupIndices( const SubMesh* baseSubMesh ) override;

        /** When the mesh is (hardware) skinned, a different code path is called so that
            we reuse the index buffers and modify them in place. For example Instance #2
            with reference to bone #5 would have BlendIndex = 2 + 5 = 7
            Everything is copied identically except the VES_BLEND_INDICES semantic
        */
        void setupHardwareSkinned( const SubMesh* baseSubMesh, VertexData *thisVertexData,
                                    VertexData *baseVertexData );

    public:
        InstanceBatchShader( InstanceManager *creator, MeshPtr &meshReference, const MaterialPtr &material,
                            size_t instancesPerBatch, const Mesh::IndexMap *indexToBoneMap,
                            const String &batchName );

        /** @see InstanceBatch::calculateMaxNumInstances */
        size_t calculateMaxNumInstances( const SubMesh *baseSubMesh, uint16 flags ) const override;

        /** @see InstanceBatch::buildFrom */
        void buildFrom( const SubMesh *baseSubMesh, const RenderOperation &renderOperation ) override;

        //Renderable overloads
        void getWorldTransforms( Matrix4* xform ) const override;
        unsigned short getNumWorldTransforms(void) const override;
    };
}

#endif
