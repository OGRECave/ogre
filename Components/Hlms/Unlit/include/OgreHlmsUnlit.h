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
#ifndef _OgreHlmsUnlit_H_
#define _OgreHlmsUnlit_H_

#include "OgreHlmsUnlitPrerequisites.h"
#include "OgreHlmsBufferManager.h"
#include "OgreConstBufferPool.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    class CompositorShadowNode;
    struct QueuedRenderable;

    /** \addtogroup Component
    *  @{
    */
    /** \addtogroup Material
    *  @{
    */

    class HlmsUnlitDatablock;

    /** Physically based shading implementation specfically designed for OpenGL ES 2.0 and other
        RenderSystems which do not support uniform buffers.
    */
    class _OgreHlmsUnlitExport HlmsUnlit : public HlmsBufferManager, public ConstBufferPool
    {
        typedef vector<HlmsDatablock*>::type HlmsDatablockVec;

        struct PassData
        {
            Matrix4 viewProjMatrix[2];
        };

        PassData                mPreparedPass;

        ConstBufferPool::BufferPool const *mLastBoundPool;

        uint32 mLastTextureHash;


        virtual const HlmsCache* createShaderCacheEntry( uint32 renderableHash,
                                                         const HlmsCache &passCache,
                                                         uint32 finalHash,
                                                         const QueuedRenderable &queuedRenderable );

        virtual HlmsDatablock* createDatablockImpl( IdString datablockName,
                                                    const HlmsMacroblock *macroblock,
                                                    const HlmsBlendblock *blendblock,
                                                    const HlmsParamVec &paramVec );

        void setTextureProperty( IdString propertyName, HlmsUnlitDatablock *datablock,
                                 uint8 texType );

        virtual void calculateHashForPreCreate( Renderable *renderable, PiecesMap *inOutPieces );

        FORCEINLINE uint32 fillBuffersFor( const HlmsCache *cache,
                                           const QueuedRenderable &queuedRenderable,
                                           bool casterPass, uint32 lastCacheHash,
                                           CommandBuffer *commandBuffer, bool isV1 );

    public:
        HlmsUnlit( Archive *dataFolder );
        ~HlmsUnlit();

        virtual void _changeRenderSystem( RenderSystem *newRs );

        virtual HlmsCache preparePassHash( const Ogre::CompositorShadowNode *shadowNode,
                                           bool casterPass, bool dualParaboloid,
                                           SceneManager *sceneManager );

        virtual uint32 fillBuffersFor( const HlmsCache *cache, const QueuedRenderable &queuedRenderable,
                                       bool casterPass, uint32 lastCacheHash,
                                       uint32 lastTextureHash );

        virtual uint32 fillBuffersForV1( const HlmsCache *cache,
                                         const QueuedRenderable &queuedRenderable,
                                         bool casterPass, uint32 lastCacheHash,
                                         CommandBuffer *commandBuffer );

        virtual uint32 fillBuffersForV2( const HlmsCache *cache,
                                         const QueuedRenderable &queuedRenderable,
                                         bool casterPass, uint32 lastCacheHash,
                                         CommandBuffer *commandBuffer );

        /// Changes the default suggested size for the texture buffer.
        /// Actual size may be lower if the GPU can't honour the request.
        void setTextureBufferDefaultSize( size_t defaultSize );
    };

    struct _OgreHlmsUnlitExport UnlitProperty
    {
        static const IdString HwGammaRead;
        static const IdString HwGammaWrite;
        static const IdString SignedIntTex;
        static const IdString MaterialsPerBuffer;
        static const IdString AnimationMatricesPerBuffer;

        static const IdString TexMatrixCount;
        static const IdString TexMatrixCount0;
        static const IdString TexMatrixCount1;
        static const IdString TexMatrixCount2;
        static const IdString TexMatrixCount3;
        static const IdString TexMatrixCount4;
        static const IdString TexMatrixCount5;
        static const IdString TexMatrixCount6;
        static const IdString TexMatrixCount7;

        /// Whether uses material's colour.
        static const IdString Diffuse;

        /// Number of texture arrays actually baked.
        static const IdString NumArrayTextures;
        static const IdString NumTextures;

        /// Number of diffuse maps.
        static const IdString DiffuseMap;

        //static const IdString DiffuseMap0;
        //static const IdString DiffuseMap0Array;

        /// UV source # assigned to each texture.
        static const IdString UvDiffuse0;
        static const IdString UvDiffuse1;
        static const IdString UvDiffuse2;
        static const IdString UvDiffuse3;
        static const IdString UvDiffuse4;
        static const IdString UvDiffuse5;
        static const IdString UvDiffuse6;
        static const IdString UvDiffuse7;
        static const IdString UvDiffuse8;
        static const IdString UvDiffuse9;
        static const IdString UvDiffuse10;
        static const IdString UvDiffuse11;
        static const IdString UvDiffuse12;
        static const IdString UvDiffuse13;
        static const IdString UvDiffuse14;
        static const IdString UvDiffuse15;

        static const IdString UvDiffuseSwizzle0;
        static const IdString UvDiffuseSwizzle1;
        static const IdString UvDiffuseSwizzle2;
        static const IdString UvDiffuseSwizzle3;
        static const IdString UvDiffuseSwizzle4;
        static const IdString UvDiffuseSwizzle5;
        static const IdString UvDiffuseSwizzle6;
        static const IdString UvDiffuseSwizzle7;
        static const IdString UvDiffuseSwizzle8;
        static const IdString UvDiffuseSwizzle9;
        static const IdString UvDiffuseSwizzle10;
        static const IdString UvDiffuseSwizzle11;
        static const IdString UvDiffuseSwizzle12;
        static const IdString UvDiffuseSwizzle13;
        static const IdString UvDiffuseSwizzle14;
        static const IdString UvDiffuseSwizzle15;

        static const IdString BlendModeIndex0;
        static const IdString BlendModeIndex1;
        static const IdString BlendModeIndex2;
        static const IdString BlendModeIndex3;
        static const IdString BlendModeIndex4;
        static const IdString BlendModeIndex5;
        static const IdString BlendModeIndex6;
        static const IdString BlendModeIndex7;
        static const IdString BlendModeIndex8;
        static const IdString BlendModeIndex9;
        static const IdString BlendModeIndex10;
        static const IdString BlendModeIndex11;
        static const IdString BlendModeIndex12;
        static const IdString BlendModeIndex13;
        static const IdString BlendModeIndex14;
        static const IdString BlendModeIndex15;

        static const IdString OutUvCount;

        struct DiffuseMapPtr
        {
            IdString const *uvSource;
            IdString const *uvSourceSwizzle;
            IdString const *blendModeIndex;
        };

        static const DiffuseMapPtr DiffuseMapPtrs[NUM_UNLIT_TEXTURE_TYPES];
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
