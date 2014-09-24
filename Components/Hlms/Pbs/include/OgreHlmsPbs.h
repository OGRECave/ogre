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
#ifndef _OgreHlmsPbs_H_
#define _OgreHlmsPbs_H_

#include "OgreHlmsPbsPrerequisites.h"
#include "OgreHlms.h"
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

    class HlmsPbsDatablock;

    /** Physically based shading implementation specfically designed for OpenGL ES 2.0 and other
        RenderSystems which do not support uniform buffers.
    */
    class _OgreHlmsPbsExport HlmsPbs : public Hlms, public ConstBufferPool
    {
        typedef vector<ConstBufferPacked*>::type ConstBufferPackedVec;
        typedef vector<TexBufferPacked*>::type TexBufferPackedVec;
        typedef vector<HlmsDatablock*>::type HlmsDatablockVec;

        struct PassData
        {
            FastArray<TexturePtr>   shadowMaps;
            FastArray<float>    vertexShaderSharedBuffer;
            FastArray<float>    pixelShaderSharedBuffer;

            Matrix4 viewProjMatrix;
            Matrix4 viewMatrix;
        };

        PassData                mPreparedPass;
        ConstBufferPacked       *mPassBuffer;

        uint32                  mCurrentConstBuffer;    /// Resets every to zero every new frame.
        uint32                  mCurrentTexBuffer;      /// Resets every to zero every new frame.
        ConstBufferPackedVec    mConstBuffers;
        TexBufferPackedVec      mTexBuffers;

        uint32  *mStartMappedConstBuffer;
        uint32  *mCurrentMappedConstBuffer;

        float   *mStartMappedTexBuffer;
        float   *mCurrentMappedTexBuffer;

        /// Resets every to zero every new buffer (@see unmapTexBuffer and @see mapNextTexBuffer).
        size_t  mTexLastOffset;

        virtual const HlmsCache* createShaderCacheEntry( uint32 renderableHash,
                                                         const HlmsCache &passCache,
                                                         uint32 finalHash,
                                                         const QueuedRenderable &queuedRenderable );

        virtual HlmsDatablock* createDatablockImpl( IdString datablockName,
                                                    const HlmsMacroblock *macroblock,
                                                    const HlmsBlendblock *blendblock,
                                                    const HlmsParamVec &paramVec );

        void setDetailMapProperties( bool diffuseMaps, HlmsPbsDatablock *datablock, PiecesMap *inOutPieces );

        void uploadDirtyDatablocks(void);

        virtual void calculateHashForPreCreate( Renderable *renderable, PiecesMap *inOutPieces );
        virtual void calculateHashForPreCaster( Renderable *renderable, PiecesMap *inOutPieces );

        /// For compatibility reasons with D3D11 and GLES3, Const buffers are mapped.
        /// Once we're done with it (even if we didn't fully use it) we discard it
        /// and get a new one. We will at least have to get a new one on every pass.
        /// This is affordable since common Const buffer limits are of 64kb.
        /// At the next frame we restart mCurrentConstBuffer to 0.
        void unmapConstBuffer(void);
        void mapNextConstBuffer(void);

        /// Texture buffers are treated differently than Const buffers. We first map it.
        /// Once we're done with it, we save our progress (in mTexLastOffset) and in the
        /// next pass start where we left off (i.e. if we wrote to the first 2MB chunk,
        /// start mapping from 2MB onwards). Only when the buffer is full, we get a new
        /// Tex Buffer.
        /// At the next frame we restart mCurrentTexBuffer to 0.
        ///
        /// Tex Buffers can be as big as 128MB, thus "restarting" with another 128MB
        /// buffer on every pass is too expensive. This strategy benefits low level RS
        /// like GL3+ and D3D11.1* (Windows 8) and D3D12; whereas on D3D11 and GLES3
        /// drivers dynamic mapping may discover we're writing to a region not in use
        /// or may internally use a new buffer (wasting memory space).
        ///
        /// (*) D3D11.1 allows using MAP_NO_OVERWRITE for texture buffers.
        void unmapTexBuffer(void);
        void mapNextTexBuffer(void);

    public:
        HlmsPbs( Archive *dataFolder );
        ~HlmsPbs();

        virtual void _changeRenderSystem( RenderSystem *newRs );

        virtual HlmsCache preparePassHash( const Ogre::CompositorShadowNode *shadowNode,
                                           bool casterPass, bool dualParaboloid,
                                           SceneManager *sceneManager );

        virtual uint32 fillBuffersFor( const HlmsCache *cache, const QueuedRenderable &queuedRenderable,
                                       bool casterPass, const HlmsCache *lastCache,
                                       uint32 lastTextureHash );

        virtual void frameEnded(void);
    };

    struct _OgreHlmsPbsExport PbsProperty
    {
        static const IdString HwGammaRead;
        static const IdString HwGammaWrite;
        static const IdString SignedIntTex;

        static const IdString NumTextures;
        static const IdString DiffuseMap;
        static const IdString NormalMapTex;
        static const IdString SpecularMap;
        static const IdString RoughnessMap;
        static const IdString EnvProbeMap;
        static const IdString DetailWeightMap;

        static const IdString NormalMap;

        static const IdString FresnelScalar;

        static const IdString NormalWeight;
        static const IdString NormalWeightTex;
        static const IdString NormalWeightDetail0;
        static const IdString NormalWeightDetail1;
        static const IdString NormalWeightDetail2;
        static const IdString NormalWeightDetail3;

        static const IdString DetailWeights;
        static const IdString DetailOffsetsD;
        static const IdString DetailOffsetsD0;
        static const IdString DetailOffsetsD1;
        static const IdString DetailOffsetsD2;
        static const IdString DetailOffsetsD3;
        static const IdString DetailOffsetsN;
        static const IdString DetailOffsetsN0;
        static const IdString DetailOffsetsN1;
        static const IdString DetailOffsetsN2;
        static const IdString DetailOffsetsN3;

        static const IdString UvDiffuse;
        static const IdString UvNormal;
        static const IdString UvSpecular;
        static const IdString UvRoughness;
        static const IdString UvDetailWeight;

        static const IdString UvDetail0;
        static const IdString UvDetail1;
        static const IdString UvDetail2;
        static const IdString UvDetail3;

        static const IdString UvDetailNm0;
        static const IdString UvDetailNm1;
        static const IdString UvDetailNm2;
        static const IdString UvDetailNm3;

        static const IdString DetailMapsDiffuse;
        static const IdString DetailMapsNormal;

        static const IdString BlendModeIndex0;
        static const IdString BlendModeIndex1;
        static const IdString BlendModeIndex2;
        static const IdString BlendModeIndex3;

        static const IdString DetailDiffuseSwizzle0;
        static const IdString DetailDiffuseSwizzle1;
        static const IdString DetailDiffuseSwizzle2;
        static const IdString DetailDiffuseSwizzle3;

        static const IdString DetailNormalSwizzle0;
        static const IdString DetailNormalSwizzle1;
        static const IdString DetailNormalSwizzle2;
        static const IdString DetailNormalSwizzle3;

        static const IdString *UvSourcePtrs[NUM_PBSM_SOURCES];
        static const IdString *BlendModes[4];
        static const IdString *DetailDiffuseSwizzles[4];
        static const IdString *DetailNormalSwizzles[4];
        static const IdString *DetailNormalWeights[4];
        static const IdString *DetailOffsetsDPtrs[4];
        static const IdString *DetailOffsetsNPtrs[4];
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
