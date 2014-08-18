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
    struct _OgreExport HlmsMacroblock
    {
        uint8               mId;
        bool                mDepthCheck;
        bool                mDepthWrite;
        CompareFunction     mDepthFunc;

        /// When polygons are coplanar, you can get problems with 'depth fighting' where
        /// the pixels from the two polys compete for the same screen pixel. This is particularly
        /// a problem for decals (polys attached to another surface to represent details such as
        /// bulletholes etc.).
        ///
        /// A way to combat this problem is to use a depth bias to adjust the depth buffer value
        /// used for the decal such that it is slightly higher than the true value, ensuring that
        /// the decal appears on top. There are two aspects to the biasing, a constant
        /// bias value and a slope-relative biasing value, which varies according to the
        ///  maximum depth slope relative to the camera, ie:
        /// <pre>finalBias = maxSlope * slopeScaleBias + constantBias</pre>
        /// Note that slope scale bias, whilst more accurate, may be ignored by old hardware.
        ///
        /// The constant bias value, expressed as a factor of the minimum observable depth
        float               mDepthBiasConstant;
        /// The slope-relative bias value, expressed as a factor of the depth slope
        float               mDepthBiasSlopeScale;

        bool                mAlphaToCoverageEnabled;
        bool                mScissorTestEnabled;
        CullingMode         mCullMode;
        PolygonMode         mPolygonMode;
        void                *mRsData;       ///Render-System specific data

        HlmsMacroblock();

        bool operator != ( const HlmsMacroblock &_r ) const
        {
            //Don't include the ID in the comparision
            return memcmp( &mDepthCheck, &_r.mDepthCheck,
                           ( (const char*)&mPolygonMode - (const char*)&mDepthCheck ) +
                                                            sizeof( PolygonMode ) ) != 0;
        }
    };

    /** A blend block contains settings that rarely change, and thus are common to many materials.
        The reasons this structure isn't joined with @HlmsMacroblock is that:
            * The D3D11 API makes this distinction (much higher API overhead if we
              change i.e. depth settings) due to D3D11_RASTERIZER_DESC.
            * This block contains information of whether the material is transparent.
              Transparent materials are sorted differently than opaque ones.
        Up to 32 different blocks are allowed!
    */
    struct _OgreExport HlmsBlendblock
    {
        uint8               mId;
        /// Used to determine if separate alpha blending should be used for color and alpha channels
        bool                mSeparateBlend;

        SceneBlendFactor    mSourceBlendFactor;
        SceneBlendFactor    mDestBlendFactor;
        SceneBlendFactor    mSourceBlendFactorAlpha;
        SceneBlendFactor    mDestBlendFactorAlpha;

        // Blending operations
        SceneBlendOperation mBlendOperation;
        SceneBlendOperation mBlendOperationAlpha;

        void                *mRsData;       ///Render-System specific data

        HlmsBlendblock();

        bool operator != ( const HlmsBlendblock &_r ) const
        {
            //Don't include the ID in the comparision
            return memcmp( &mSeparateBlend, &_r.mSeparateBlend,
                           ( (const char*)&mBlendOperationAlpha - (const char*)&mSeparateBlend) +
                                                                sizeof( SceneBlendOperation ) ) != 0;
        }
    };

    /** An hlms datablock contains individual information about a specific material. It consists of:
            * A const pointer to an @HlmsMacroblock we do not own and may be shared by other datablocks.
            * A const pointer to an @HlmsBlendblock we do not own and may be shared by other datablocks.
            * The original properties from which this datablock was constructed.
            * This type may be derived to contain additional information.
        Derived types can cache information present in mOriginalProperties as strings, like diffuse
        colour values, etc.

        A datablock is the internal representation of the surface parameters (depth settings,
        textures to be used, diffuse colour, specular colour, etc).
        The notion of a datablock is the closest you'll get to a "material"
    @remarks
        Macro- & Blendblocks are immutable, hence const pointers. Trying to const cast these
        pointers in order to modify them may work on certain RenderSystems (i.e. GLES2) but
        will seriously break on other RenderSystems (i.e. D3D11).
    @par
        If you need to change a macroblock, create a new one (HlmsManager keeps them cached
        if already created) and change the entire pointer.
    */
    class _OgreExport HlmsDatablock : public PassAlloc
    {
    protected:
        //Non-hot variables first (can't put them last as HlmsDatablock may be derived and
        //it's better if mShadowConstantBias is together with the derived type's variables
        /// List of renderables currently using this datablock
        vector<Renderable*>::type mLinkedRenderables;
        Hlms    *mCreator;
        IdString mName;

        /** Updates the mHlmsHash & mHlmsCasterHash for all linked renderables, which may have
            if a sensitive setting has changed that would need a different shader to be created
        @remarks
            The operation itself isn't expensive, but the need to call this function indicates
            that another shader will be created (unless already cached too). If so, doing that
            will be slow.
        */
        void flushRenderables(void);

    public:
        uint32  mTextureHash;       //TextureHash comes before macroblock for alignment reasons
        uint16  mMacroblockHash;    //Not all bits are used
        uint8   mType;              /// @See HlmsTypes
        bool    mIsTransparent;     /// Cached based on mBlendblock data
        HlmsMacroblock const *mMacroblock;
        HlmsBlendblock const *mBlendblock;  ///Don't set this directly, use @setBlendblock
    protected:
        bool    mAlphaTest;
    public:
        float   mAlphaTestThreshold;
        float   mShadowConstantBias;

    public:
        HlmsDatablock( IdString name, Hlms *creator,
                       const HlmsMacroblock *macroblock,
                       const HlmsBlendblock *blendblock,
                       const HlmsParamVec &params );
        virtual ~HlmsDatablock();

        /// Calculates the hashes needed for sorting by the RenderQueue (i.e. mTextureHash)
        virtual void calculateHash() {}

        IdString getName(void) const                { return mName; }
        Hlms* getCreator(void) const                { return mCreator; }

        /// Call this function to set mBlendblock & mIsTransparent automatically based on input
        void setBlendblock( HlmsBlendblock const *blendblock );

        void _linkRenderable( Renderable *renderable );
        void _unlinkRenderable( Renderable *renderable );

        const vector<Renderable*>::type& getLinkedRenderables(void) const { return mLinkedRenderables; }

        /// Enables or disables alpha testing. Calling this function triggers
        /// a HlmsDatablock::flushRenderables
        void setAlphaTest( bool bEnabled );
        bool getAlphaTest(void) const               { return mAlphaTest; }
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
