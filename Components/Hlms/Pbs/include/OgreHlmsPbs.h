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
#include "OgreHlmsBufferManager.h"
#include "OgreConstBufferPool.h"
#include "OgreMatrix4.h"
#include "OgreHeaderPrefix.h"
#include "OgreRoot.h"

namespace Ogre
{
    class CompositorShadowNode;
    struct QueuedRenderable;
#ifdef OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS
    class PlanarReflections;
#endif

    /** \addtogroup Component
    *  @{
    */
    /** \addtogroup Material
    *  @{
    */

    class HlmsPbsDatablock;

    typedef vector<TexturePtr>::type TextureVec;

    /** Physically based shading implementation specfically designed for
        OpenGL 3+, D3D11 and other RenderSystems which support uniform buffers.
    */
    class _OgreHlmsPbsExport HlmsPbs : public HlmsBufferManager, public ConstBufferPool
    {
    public:
        enum ShadowFilter
        {
            /// Standard quality. Very fast.
            PCF_2x2,

            /// Good quality. Still quite fast on most modern hardware.
            PCF_3x3,

            /// High quality. Very slow in old hardware (i.e. DX10 level hw and below)
            /// Use RSC_TEXTURE_GATHER to check whether it will be slow or not.
            PCF_4x4,

            /// High quality. Produces soft shadows. It's much more expensive but given
            /// its blurry results, you can reduce resolution and/or use less PSSM splits
            /// which gives you very competing performance with great results.
            /// ESM stands for Exponential Shadow Maps.
            ExponentialShadowMaps,

            NumShadowFilter
        };

        enum AmbientLightMode
        {
            /// Use fixed-colour ambient lighting when upper hemisphere = lower hemisphere,
            /// use hemisphere lighting when they don't match.
            /// Disables ambient lighting if the colours are black.
            AmbientAuto,

            /// Force fixed-colour ambient light. Only uses the upper hemisphere paramter.
            AmbientFixed,

            /// Force hemisphere ambient light. Useful if you plan on adjusting the colours
            /// dynamically very often and this might cause swapping shaders.
            AmbientHemisphere,

            /// Disable ambient lighting.
            AmbientNone
        };

    protected:
        typedef vector<ConstBufferPacked*>::type ConstBufferPackedVec;
        typedef vector<HlmsDatablock*>::type HlmsDatablockVec;

        struct PassData
        {
            FastArray<Texture*> shadowMaps;
            FastArray<float>    vertexShaderSharedBuffer;
            FastArray<float>    pixelShaderSharedBuffer;

            Matrix4 viewMatrix;
        };

        PassData                mPreparedPass;
        ConstBufferPackedVec    mPassBuffers;
        HlmsSamplerblock const  *mShadowmapSamplerblock;    /// GL3+ only when not using depth textures
        HlmsSamplerblock const  *mShadowmapCmpSamplerblock; /// For depth textures & D3D11
        HlmsSamplerblock const  *mShadowmapEsmSamplerblock; /// For ESM.
        HlmsSamplerblock const  *mCurrentShadowmapSamplerblock;
        TexturePtr              mTargetEnvMap;
        ParallaxCorrectedCubemap    *mParallaxCorrectedCubemap;

        uint32                  mCurrentPassBuffer;     /// Resets to zero every new frame.

        TexBufferPacked         *mGridBuffer;
        TexBufferPacked         *mGlobalLightListBuffer;

        uint32                  mTexUnitSlotStart;

        TextureVec const        *mPrePassTextures;
        TexturePtr              mPrePassMsaaDepthTexture;
        TextureVec const        *mSsrTexture;
        IrradianceVolume        *mIrradianceVolume;
#ifdef OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS
        //TODO: After texture refactor it should be possible to abstract this,
        //so we don't have to be aware of PlanarReflections class.
        PlanarReflections       *mPlanarReflections;
        HlmsSamplerblock const  *mPlanarReflectionsSamplerblock;
        /// Whether the current active pass can use mPlanarReflections (i.e. we can't
        /// use the reflections if they were built for a different camera angle)
        bool                    mHasPlanarReflections;
        uint8                   mLastBoundPlanarReflection;
#endif
        TexturePtr              mAreaLightMasks;
        HlmsSamplerblock const  *mAreaLightMasksSamplerblock;
        LightArray				mAreaLights;
        bool                    mUsingAreaLightMasks;

        bool                    mUsingLtcMatrix;
        TexturePtr              mLtcMatrixTexture;

        TexturePtr              mDecalsTextures[3];
        HlmsSamplerblock const  *mDecalsSamplerblock;

        ConstBufferPool::BufferPool const *mLastBoundPool;

        uint32 mLastTextureHash;
#if !OGRE_NO_FINE_LIGHT_MASK_GRANULARITY
        bool mFineLightMaskGranularity;
#endif
        bool mDebugPssmSplits;

        ShadowFilter    mShadowFilter;
        uint16          mEsmK; /// K parameter for ESM.
        AmbientLightMode mAmbientLightMode;

        virtual const HlmsCache* createShaderCacheEntry( uint32 renderableHash,
                                                         const HlmsCache &passCache,
                                                         uint32 finalHash,
                                                         const QueuedRenderable &queuedRenderable );

        virtual HlmsDatablock* createDatablockImpl( IdString datablockName,
                                                    const HlmsMacroblock *macroblock,
                                                    const HlmsBlendblock *blendblock,
                                                    const HlmsParamVec &paramVec );

        void setDetailMapProperties( HlmsPbsDatablock *datablock, PiecesMap *inOutPieces );
        void setTextureProperty( const char *propertyName, HlmsPbsDatablock *datablock,
                                 PbsTextureTypes texType );
        void setDetailTextureProperty( const char *propertyName, HlmsPbsDatablock *datablock,
                                       PbsTextureTypes baseTexType, uint8 detailIdx );

        virtual void calculateHashForPreCreate( Renderable *renderable, PiecesMap *inOutPieces );
        virtual void calculateHashForPreCaster( Renderable *renderable, PiecesMap *inOutPieces );

        virtual void notifyPropertiesMergedPreGenerationStep(void);

        static bool requiredPropertyByAlphaTest( IdString propertyName );

        virtual void destroyAllBuffers(void);

        FORCEINLINE uint32 fillBuffersFor( const HlmsCache *cache,
                                           const QueuedRenderable &queuedRenderable,
                                           bool casterPass, uint32 lastCacheHash,
                                           CommandBuffer *commandBuffer, bool isV1 );

    public:
        HlmsPbs( Archive *dataFolder, ArchiveVec *libraryFolders );
        virtual ~HlmsPbs();

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

        virtual void postCommandBufferExecution( CommandBuffer *commandBuffer );
        virtual void frameEnded(void);

        void loadLtcMatrix(void);

        /** Fill the provided string and string vector with all the sub-folder needed to instantiate
            an HlmsPbs object with the default distribution of the HlmsResources.
            These paths are dependent of the current RenderSystem.

            This method can only be called after a valid RenderSysttem has been chosen.

            All output parameter's content will be replaced with the new set of paths.
        @param outDataFolderPath
            Path (as a String) used for creating the "dataFolder" Archive the constructor will need
        @param outLibraryFoldersPaths
            Vector of String used for creating the ArchiveVector "libraryFolders" the constructor will need
        */
        static void getDefaultPaths( String& outDataFolderPath, StringVector& outLibraryFoldersPaths );

#if !OGRE_NO_FINE_LIGHT_MASK_GRANULARITY
        /// Toggles whether light masks will be obeyed per object by doing:
        /// if( movableObject->getLightMask() & light->getLightMask() )
        ///     doLighting( movableObject light );
        /// Note this toggle only affects forward lights
        /// (i.e. Directional lights + shadow casting lights).
        /// You may want to see ForwardPlusBase::setFineLightMaskGranularity
        /// for control over Forward+ lights.
        void setFineLightMaskGranularity( bool useFineGranularity )
                                                    { mFineLightMaskGranularity = useFineGranularity; }
        bool getFineLightMaskGranularity(void) const{ return mFineLightMaskGranularity; }
#endif

        void setDebugPssmSplits( bool bDebug );
        bool getDebugPssmSplits(void) const                 { return mDebugPssmSplits; }

        void setShadowSettings( ShadowFilter filter );
        ShadowFilter getShadowFilter(void) const            { return mShadowFilter; }

        /** Sets the 'K' parameter of ESM filter. Defaults to 600.
            Small values will give weak shadows, and light bleeding (specially if the
            caster is close to the receiver; particularly noticeable at contact points).
            It also gives the chance of over darkening to appear (the shadow of a small
            caster in front of a large caster looks darker; thus the large caster appers
            like if it were made of glass instead of being solid).

            Large values give strong, dark shadows; but the higher the value, the more
            you push floating point limits.
            This value is related to K in MiscUtils::setGaussianLogFilterParams. You don't
            have to set them to the same value; but you'll notice that if you change this
            value here, you'll likely have to change the log filter's too.
        @param K
            In range (0; infinite).
        */
        void setEsmK( uint16 K );
        uint16 getEsmK(void) const                          { return mEsmK; }

        void setAmbientLightMode( AmbientLightMode mode );
        AmbientLightMode getAmbientLightMode(void) const    { return mAmbientLightMode; }

        void setParallaxCorrectedCubemap( ParallaxCorrectedCubemap *pcc )
                                                            { mParallaxCorrectedCubemap = pcc; }
        ParallaxCorrectedCubemap* getParallaxCorrectedCubemap(void) const
                                                            { return mParallaxCorrectedCubemap; }

        void setIrradianceVolume( IrradianceVolume *irradianceVolume )
                                                    { mIrradianceVolume = irradianceVolume; }
        IrradianceVolume* getIrradianceVolume(void) const  { return mIrradianceVolume; }

        void setAreaLightMasks( const TexturePtr &areaLightMask );
        const TexturePtr& getAreaLightMasks(void) const     { return mAreaLightMasks; }

#ifdef OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS
        void setPlanarReflections( PlanarReflections *planarReflections );
        PlanarReflections* getPlanarReflections(void) const;
#endif

#if !OGRE_NO_JSON
        /// @copydoc Hlms::_loadJson
        virtual void _loadJson( const rapidjson::Value &jsonValue, const HlmsJson::NamedBlocks &blocks,
                                HlmsDatablock *datablock, HlmsJsonListener *listener,
                                const String &additionalTextureExtension ) const;
        /// @copydoc Hlms::_saveJson
        virtual void _saveJson( const HlmsDatablock *datablock, String &outString,
                                HlmsJsonListener *listener,
                                const String &additionalTextureExtension ) const;

        /// @copydoc Hlms::_collectSamplerblocks
        virtual void _collectSamplerblocks( set<const HlmsSamplerblock*>::type &outSamplerblocks,
                                            const HlmsDatablock *datablock ) const;
#endif
    };

    struct _OgreHlmsPbsExport PbsProperty
    {
        static const IdString HwGammaRead;
        static const IdString HwGammaWrite;
        static const IdString SignedIntTex;
        static const IdString MaterialsPerBuffer;
        static const IdString LowerGpuOverhead;
        static const IdString DebugPssmSplits;
        static const IdString HasPlanarReflections;

        static const IdString NumTextures;
        static const char *DiffuseMap;
        static const char *NormalMapTex;
        static const char *SpecularMap;
        static const char *RoughnessMap;
        static const char *EmissiveMap;
        static const char *EnvProbeMap;
        static const char *DetailWeightMap;
        static const char *DetailMapN;
        static const char *DetailMapNmN;

        static const IdString DetailMap0;
        static const IdString DetailMap1;
        static const IdString DetailMap2;
        static const IdString DetailMap3;

        static const IdString NormalMap;

        static const IdString FresnelScalar;
        static const IdString UseTextureAlpha;
        static const IdString TransparentMode;
        static const IdString FresnelWorkflow;
        static const IdString MetallicWorkflow;
        static const IdString TwoSidedLighting;
        static const IdString ReceiveShadows;
        static const IdString UsePlanarReflections;

        static const IdString NormalWeight;
        static const IdString NormalWeightTex;
        static const IdString NormalWeightDetail0;
        static const IdString NormalWeightDetail1;
        static const IdString NormalWeightDetail2;
        static const IdString NormalWeightDetail3;

        static const IdString DetailWeights;
        static const IdString DetailOffsets0;
        static const IdString DetailOffsets1;
        static const IdString DetailOffsets2;
        static const IdString DetailOffsets3;

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

        static const IdString UvEmissive;
        static const IdString EmissiveConstant;
        static const IdString DetailMapsDiffuse;
        static const IdString DetailMapsNormal;
        static const IdString FirstValidDetailMapNm;

        static const IdString BlendModeIndex0;
        static const IdString BlendModeIndex1;
        static const IdString BlendModeIndex2;
        static const IdString BlendModeIndex3;

        static const IdString Pcf3x3;
        static const IdString Pcf4x4;
        static const IdString PcfIterations;
        static const IdString ExponentialShadowMaps;

        static const IdString AmbientHemisphere;
        static const IdString EnvMapScale;
        static const IdString AmbientFixed;
        static const IdString TargetEnvprobeMap;
        static const IdString ParallaxCorrectCubemaps;
        static const IdString UseParallaxCorrectCubemaps;
        static const IdString IrradianceVolumes;

        static const IdString BrdfDefault;
        static const IdString BrdfCookTorrance;
        static const IdString BrdfBlinnPhong;
        static const IdString FresnelSeparateDiffuse;
        static const IdString GgxHeightCorrelated;
        static const IdString LegacyMathBrdf;
        static const IdString RoughnessIsShininess;

        static const IdString *UvSourcePtrs[NUM_PBSM_SOURCES];
        static const IdString *BlendModes[4];
        static const IdString *DetailNormalWeights[4];
        static const IdString *DetailOffsetsPtrs[4];
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
