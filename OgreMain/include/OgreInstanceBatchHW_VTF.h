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
#ifndef __InstanceBatchHW_VTF_H__
#define __InstanceBatchHW_VTF_H__

#include "OgreInstanceBatchVTF.h"

namespace Ogre
{
namespace v1
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */

    /** Instancing implementation using vertex texture through Vertex Texture Fetch (VTF) and
        hardware instancing.
        @see BaseInstanceBatchVTF and @see InstanceBatchHW

        The advantage over TextureVTF technique, is that this implements a basic culling algorithm
        to avoid useless processing in vertex shader and uses a lot less VRAM and memory bandwidth

        Basically it has the benefits of both TextureVTF (skeleton animations) and HWInstancingBasic
        (lower memory consumption and basic culling) techniques

        @remarks
            Design discussion webpage: http://www.ogre3d.org/forums/viewtopic.php?f=4&t=59902
        @author
            Matias N. Goldberg ("dark_sylinc")
        @version
            1.2
     */
    class _OgreExport InstanceBatchHW_VTF : public BaseInstanceBatchVTF
    {
    protected:
        struct TransformsToTexture
        {
            float * RESTRICT_ALIAS mDest; //Pointer to VTF texture
            size_t  mFloatsPerEntity;
            size_t  mEntitiesPerPadding;
            size_t  mWidthFloatsPadding;
            TransformsToTexture( float * RESTRICT_ALIAS dstPtr,
                                 size_t floatsPerEntity,
                                 size_t entitiesPerPadding,
                                 size_t widthFloatsPadding ) :
                    mDest( dstPtr ),
                    mFloatsPerEntity( floatsPerEntity ),
                    mEntitiesPerPadding( entitiesPerPadding ),
                    mWidthFloatsPadding( widthFloatsPadding ) {}
        };
        struct SendAllSingleTransformsToTexture : public TransformsToTexture
        {
            size_t  mInstancesWritten;
            SendAllSingleTransformsToTexture( float * RESTRICT_ALIAS dstPtr,
                                                size_t floatsPerEntity,
                                                size_t entitiesPerPadding,
                                                size_t widthFloatsPadding ) :
                    TransformsToTexture( dstPtr, floatsPerEntity,
                                         entitiesPerPadding, widthFloatsPadding ),
                    mInstancesWritten( 0 ) {}
            FORCEINLINE void operator () ( const MovableObject *mo );
        };
        struct SendAllAnimatedTransformsToTexture : public TransformsToTexture
        {
            size_t  mInstancesWritten;
            Mesh::IndexMap::const_iterator boneIdxStart;
            Mesh::IndexMap::const_iterator boneIdxEnd;
            SendAllAnimatedTransformsToTexture( float * RESTRICT_ALIAS dstPtr,
                                                size_t floatsPerEntity,
                                                size_t entitiesPerPadding,
                                                size_t widthFloatsPadding,
                                                const Mesh::IndexMap *indexMap ) :
                    TransformsToTexture( dstPtr, floatsPerEntity,
                                         entitiesPerPadding, widthFloatsPadding ),
                    mInstancesWritten( 0 ),
                    boneIdxStart( indexMap->begin() ),
                    boneIdxEnd( indexMap->end() ) {}
            FORCEINLINE void operator () ( const MovableObject *mo );
        };
        struct SendAllLUTToTexture : public TransformsToTexture
        {
            vector<bool>::type mWrittenPositions;
            Mesh::IndexMap::const_iterator boneIdxStart;
            Mesh::IndexMap::const_iterator boneIdxEnd;
            SendAllLUTToTexture( float * RESTRICT_ALIAS dstPtr,
                                    size_t floatsPerEntity,
                                    size_t entitiesPerPadding,
                                    size_t widthFloatsPadding,
                                    const Mesh::IndexMap *indexMap,
                                    size_t numLutEntries ) :
                    TransformsToTexture( dstPtr, floatsPerEntity,
                                         entitiesPerPadding, widthFloatsPadding ),
                    mWrittenPositions( numLutEntries, false ),
                    boneIdxStart( indexMap->begin() ),
                    boneIdxEnd( indexMap->end() ) {}
            FORCEINLINE void operator () ( const MovableObject *mo );
        };
        struct SendAllDualQuatTexture : public TransformsToTexture
        {
            size_t  mInstancesWritten;
            Mesh::IndexMap::const_iterator boneIdxStart;
            Mesh::IndexMap::const_iterator boneIdxEnd;
            SendAllDualQuatTexture( float * RESTRICT_ALIAS dstPtr,
                                    size_t floatsPerEntity,
                                    size_t entitiesPerPadding,
                                    size_t widthFloatsPadding,
                                    const Mesh::IndexMap *indexMap ) :
                    TransformsToTexture( dstPtr, floatsPerEntity,
                                         entitiesPerPadding, widthFloatsPadding ),
                    mInstancesWritten( 0 ),
                    boneIdxStart( indexMap->begin() ),
                    boneIdxEnd( indexMap->end() ) {}
            FORCEINLINE void operator () ( const MovableObject *mo );
        };

        //Pointer to the buffer containing the per instance vertex data
        HardwareVertexBufferSharedPtr mInstanceVertexBuffer;

        void setupVertices( const SubMesh* baseSubMesh );
        void setupIndices( const SubMesh* baseSubMesh );

        /** Creates 2 TEXCOORD semantics that will be used to sample the vertex texture */
        void createVertexSemantics( VertexData *thisVertexData, VertexData *baseVertexData,
            const HWBoneIdxVec &hwBoneIdx, const HWBoneWgtVec& hwBoneWgt );

        /** updates the vertex buffer containing the per instance data 
        */
        void fillVertexBufferOffsets(void);

        void fillVertexBufferLUT( const MovableObjectArray *culledInstances );

        virtual bool checkSubMeshCompatibility( const SubMesh* baseSubMesh );

        /** Keeps filling the VTF with world matrix data. Overloaded to avoid culled objects
            and update visible instances' animation
        */
        size_t updateVertexTexture(Camera *currentCamera , const Camera *lodCamera);

        /// Overloaded to reserve enough space in mCulledInstances
        virtual void createAllInstancedEntities(void);

        virtual bool matricesTogetherPerRow() const { return true; }
    public:
        InstanceBatchHW_VTF( IdType id, ObjectMemoryManager *objectMemoryManager,
                            InstanceManager *creator, MeshPtr &meshReference,
                            const MaterialPtr &material, size_t instancesPerBatch,
                            const Mesh::IndexMap *indexToBoneMap );
        virtual ~InstanceBatchHW_VTF();
        /** @see InstanceBatch::calculateMaxNumInstances */
        size_t calculateMaxNumInstances( const SubMesh *baseSubMesh, uint16 flags ) const;

        /** @copydoc InstanceBatchHW::_boundsDirty */
        void _boundsDirty(void);

        /** Overloaded to visibility on a per unit basis and finally updated the vertex texture */
        virtual void _updateRenderQueue( RenderQueue* queue, Camera *camera, const Camera *lodCamera );

        virtual void instanceBatchCullFrustumThreaded( const Camera *frustum,
                                                       const Camera *lodCamera,
                                                        uint32 combinedVisibilityFlags )
        {
            instanceBatchCullFrustumThreadedImpl( frustum, lodCamera, combinedVisibilityFlags );
        }
    };

}
}

#endif
