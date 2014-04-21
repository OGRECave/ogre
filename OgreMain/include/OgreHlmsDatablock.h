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
#ifndef _OgreHlmsDatablock_H_
#define _OgreHlmsDatablock_H_

#include "OgreStringVector.h"
#include "OgreHlmsCommon.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

    /** A macro block contains settings that will rarely change, and thus are common to many materials.
        This is very analogous to D3D11_RASTERIZER_DESC. @See HlmsDatablock
        Up to 32 different blocks are allowed!
    */
    struct HlmsMacroblock
    {
        bool                mDepthCheck;
        bool                mDepthWrite;
        CompareFunction     mDepthFunc;
        float               mDepthBiasConstant;
        float               mDepthBiasSlopeScale;

        bool                mAlphaToCoverageEnabled;
        CullingMode         mCullMode;
        PolygonMode         mPolygonMode;

        HlmsMacroblock();
    };

    /** A blend block contains settings that rarely change, and thus are common to many materials.
        The reasons this structure isn't joined with @HlmsMacroblock is that:
            * The D3D11 API makes this distinction (much higher API overhead if we
              change i.e. depth settings) due to D3D11_RASTERIZER_DESC.
            * This block contains information of whether the material is transparent.
              Transparent materials are sorted differently than opaque ones.
        Up to 32 different blocks are allowed!
    */
    struct HlmsBlendblock
    {
        /// Used to determine if separate alpha blending should be used for color and alpha channels
        bool                mSeparateBlend;

        SceneBlendFactor    mSourceBlendFactor;
        SceneBlendFactor    mDestBlendFactor;
        SceneBlendFactor    mSourceBlendFactorAlpha;
        SceneBlendFactor    mDestBlendFactorAlpha;

        // Blending operations
        SceneBlendOperation mBlendOperation;
        SceneBlendOperation mBlendOperationAlpha;
    };

    /** An hlms datablock contains individual information about a specific material. It consists of:
            * A const pointer to an @HlmsMacroblock we do not own and may be shared by other datablocks.
            * A const pointer to an @HlmsBlendblock we do not own and may be shared by other datablocks.
            * The original properties from which this datablock was constructed.
            * This type may be derived to contain additional information.
        Derived types can cache information present in mOriginalProperties as strings, like diffuse
        colour values, etc.
    */
    class _OgreExport HlmsDatablock : public PassAlloc
    {
    public:
        uint32  mTextureHash;           //TextureHash comes before macroblock for alignment reasons
        uint16  mMacroblockHash;        //Not all bits are used
        uint8   mType;      /// @See HlmsTypes
        bool    mIsOpaque;  /// Cached based on mBlendblock data
        HlmsMacroblock const *mMacroblock;
        HlmsBlendblock const *mBlendblock;
        HlmsParamVec mOriginalParams;

        float   mShadowConstantBias;

        HlmsDatablock( HlmsTypes type,
                       const HlmsMacroblock *macroblock, uint8 macroblockId,
                       const HlmsBlendblock *blendblock, uint8 blendblockId,
                       const HlmsParamVec &params );
        virtual ~HlmsDatablock() {}
        virtual void calculateHash() = 0;
    };

    /** Contains information needed by PBS (Physically Based Shading) for OpenGL ES 2.0
    */
    class _OgreExport HlmsPbsEs2Datablock : public HlmsDatablock
    {
    public:
        uint8   mFresnelTypeSizeBytes;              //4 if mFresnel is float, 12 if it is vec3
        float   mRoughness;
        float   mkDr, mkDg, mkDb;                   //kD
        float   mkSr, mkSg, mkSb;                   //kS
        float   mFresnelR, mFresnelG, mFresnelB;    //F0

        TexturePtr  mDiffuseTex;
        TexturePtr  mNormalmapTex;
        TexturePtr  mSpecularTex;
        TexturePtr  mReflectionTex;
        /*TexturePtr  mDetailMask;
        TexturePtr  mDetailMap[4];*/

        HlmsPbsEs2Datablock( const HlmsMacroblock *macroblock, uint8 macroblockId,
                             const HlmsBlendblock *blendblock, uint8 blendblockId,
                             const HlmsParamVec &params );

        virtual void calculateHash();
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
