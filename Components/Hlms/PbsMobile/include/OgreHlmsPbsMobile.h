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
#ifndef _OgreHlmsPbsMobile_H_
#define _OgreHlmsPbsMobile_H_

#include "OgreHlmsPbsMobilePrerequisites.h"
#include "OgreHlms.h"
#include "OgreMatrix4.h"
#include "OgreHeaderPrefix.h"
#include "OgreMatrix4.h"

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

    class HlmsPbsMobileDatablock;

    /** Physically based shading implementation specfically designed for OpenGL ES 2.0 and other
        RenderSystems which do not support uniform buffers.
    */
    class _OgreHlmsPbsMobileExport HlmsPbsMobile : public Hlms
    {
        struct PassData
        {
            FastArray<TexturePtr>   shadowMaps;
            FastArray<float>    vertexShaderSharedBuffer;
            FastArray<float>    pixelShaderSharedBuffer;

            Matrix4 viewProjMatrix;
            Matrix4 viewMatrix;
        };

        PassData    mPreparedPass;

        virtual const HlmsCache* createShaderCacheEntry( uint32 renderableHash,
                                                         const HlmsCache &passCache,
                                                         uint32 finalHash,
                                                         const QueuedRenderable &queuedRenderable );

        virtual HlmsDatablock* createDatablockImpl( IdString datablockName,
                                                    const HlmsMacroblock *macroblock,
                                                    const HlmsBlendblock *blendblock,
                                                    const HlmsParamVec &paramVec );

        void setDetailMapProperties( bool diffuseMaps, HlmsPbsMobileDatablock *datablock, PiecesMap *inOutPieces );

        virtual void calculateHashForPreCreate( Renderable *renderable, PiecesMap *inOutPieces );
        virtual void calculateHashForPreCaster( Renderable *renderable, PiecesMap *inOutPieces );

    public:
        HlmsPbsMobile( Archive *dataFolder, ArchiveVec *libraryFolders );
        ~HlmsPbsMobile();

        virtual HlmsCache preparePassHash( const Ogre::CompositorShadowNode *shadowNode,
                                           bool casterPass, bool dualParaboloid,
                                           SceneManager *sceneManager );

        virtual uint32 fillBuffersFor( const HlmsCache *cache, const QueuedRenderable &queuedRenderable,
                                       bool casterPass, uint32 lastCacheHash,
                                       uint32 lastTextureHash );

        virtual uint32 fillBuffersFor( const HlmsCache *cache, const QueuedRenderable &queuedRenderable,
                                       bool casterPass, uint32 lastCacheHash,
                                       CommandBuffer *commandBuffer );
    };

    struct _OgreHlmsPbsMobileExport PbsMobileProperty
    {
        static const IdString HwGammaRead;
        static const IdString HwGammaWrite;
        static const IdString SignedIntTex;

        static const IdString DiffuseMap;
        static const IdString NormalMapTex;
        static const IdString SpecularMap;
        static const IdString RoughnessMap;
        static const IdString EnvProbeMap;
        static const IdString DetailWeightMap;

        static const IdString NormalMap;

        static const IdString UvAtlas;
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
