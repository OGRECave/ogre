/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#ifndef __TextureUnitState_H__
#define __TextureUnitState_H__

#include "OgrePrerequisites.h"
#include "OgreCommon.h"
#include "OgreBlendMode.h"
#include "OgreMatrix4.h"
#include "OgreTexture.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
    enum TexCoordCalcMethod : uint8;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Materials
    *  @{
    */

    /** Class which determines how a TextureUnitState accesses data from a Texture

        There are a number of parameters affecting how texture data is fetched.
        Most notably @ref FilterOptions and @ref TextureAddressingMode.
     */
    class _OgreExport Sampler {
    public:
        /** Texture addressing mode for each texture coordinate. */
        struct UVWAddressingMode
        {
            TextureAddressingMode u, v, w;
        };

        /// must be created through TextureManager
        Sampler();
        virtual ~Sampler();

        /** Set the texture filtering for this unit, using the simplified interface.

            You also have the option of specifying the minification, magnification
            and mip filter individually if you want more control over filtering
            options. See the alternative setTextureFiltering methods for details.
        @param filterType
            The high-level filter type to use.
        */
        void setFiltering(TextureFilterOptions filterType);
        /** Set a single filtering option on this texture unit.
        @param ftype
            The filtering type to set.
        @param opts
            The filtering option to set.
        */
        void setFiltering(FilterType ftype, FilterOptions opts);
        /** Set a the detailed filtering options on this texture unit.
        @param minFilter
            The filtering to use when reducing the size of the texture.
            Can be Ogre::FO_POINT, Ogre::FO_LINEAR or Ogre::FO_ANISOTROPIC.
        @param magFilter
            The filtering to use when increasing the size of the texture.
            Can be Ogre::FO_POINT, Ogre::FO_LINEAR or Ogre::FO_ANISOTROPIC.
        @param mipFilter
            The filtering to use between mip levels.
            Can be Ogre::FO_NONE (turns off mipmapping), Ogre::FO_POINT or Ogre::FO_LINEAR (trilinear filtering).
        */
        void setFiltering(FilterOptions minFilter, FilterOptions magFilter, FilterOptions mipFilter);
        /// Get the texture filtering for the given type.
        FilterOptions getFiltering(FilterType ftype) const;

        /** Gets the texture addressing mode for a given coordinate,
            i.e. what happens at uv values above 1.0.
        @note
            The default is TAM_WRAP i.e. the texture repeats over values of 1.0.
        */
        const UVWAddressingMode& getAddressingMode() const { return mAddressMode; }

        /** Sets the texture addressing mode, i.e. what happens at uv values above 1.0.

            The default is TAM_WRAP i.e. the texture repeats over values of 1.0.
            This is a shortcut method which sets the addressing mode for all
            coordinates at once; you can also call the more specific method
            to set the addressing mode per coordinate.

            This is a shortcut method which sets the addressing mode for all
            coordinates at once; you can also call the more specific method
            to set the addressing mode per coordinate.
        */
        void setAddressingMode(TextureAddressingMode tam) { setAddressingMode({tam, tam, tam}); }

        /** Sets the texture addressing mode, i.e. what happens at uv values above 1.0.

            The default is #TAM_WRAP i.e. the texture repeats over values of 1.0.
        */
        void setAddressingMode(TextureAddressingMode u, TextureAddressingMode v,
                               TextureAddressingMode w)
        {
            setAddressingMode({u, v, w});
        }
        /// @overload
        void setAddressingMode(const UVWAddressingMode& uvw);

        /** Sets the anisotropy level to be used for this texture level.

        The degree of anisotropy is the ratio between the height of the texture segment visible in a
        screen space region versus the width - so for example a floor plane, which stretches on into
        the distance and thus the vertical texture coordinates change much faster than the
        horizontal ones, has a higher anisotropy than a wall which is facing you head on (which has
        an anisotropy of 1 if your line of sight is perfectly perpendicular to it).The maximum value
        is determined by the hardware, but it is usually 8 or 16.

        In order for this to be used, you have to set the minification and/or the magnification
        option on this texture to Ogre::FO_ANISOTROPIC.
        @param maxAniso
            The maximal anisotropy level, should be between 2 and the maximum
            supported by hardware (1 is the default, ie. no anisotropy).
        */
        void setAnisotropy(unsigned int maxAniso)
        {
            mMaxAniso = maxAniso;
            mDirty = true;
        }
        /// Get this layer texture anisotropy level.
        unsigned int getAnisotropy() const { return mMaxAniso; }

        /** Sets the bias value applied to the mipmap calculation.

            You can alter the mipmap calculation by biasing the result with a
            single floating point value. After the mip level has been calculated,
            this bias value is added to the result to give the final mip level.
            Lower mip levels are larger (higher detail), so a negative bias will
            force the larger mip levels to be used, and a positive bias
            will cause smaller mip levels to be used. The bias values are in
            mip levels, so a -1 bias will force mip levels one larger than by the
            default calculation.

            In order for this option to be used, your hardware has to support mipmap biasing
            (exposed through Ogre::RSC_MIPMAP_LOD_BIAS), and your minification filtering has to be
            set to point or linear.
        */
        void setMipmapBias(float bias)
        {
            mMipmapBias = bias;
            mDirty = true;
        }
        /** Gets the bias value applied to the mipmap calculation.
        @see TextureUnitState::setTextureMipmapBias
        */
        float getMipmapBias(void) const { return mMipmapBias; }

        /** Enables or disables the comparison test for depth textures.
         *
         * When enabled, sampling the texture returns how the sampled value compares against a
         * reference value instead of the sampled value itself. Combined with linear filtering this
         * can be used to implement hardware PCF for shadow maps.
         */
        void setCompareEnabled(bool enabled)
        {
            mCompareEnabled = enabled;
            mDirty = true;
        }
        bool getCompareEnabled() const { return mCompareEnabled; }

        void setCompareFunction(CompareFunction function)
        {
            mCompareFunc = function;
            mDirty = true;
        }
        CompareFunction getCompareFunction() const { return mCompareFunc; }

        /** Sets the texture border colour.

            The default is ColourValue::Black, and this value only used when addressing mode
            is TAM_BORDER.
        */
        void setBorderColour(const ColourValue& colour)
        {
            mBorderColour = colour;
            mDirty = true;
        }
        const ColourValue& getBorderColour(void) const { return mBorderColour; }

    protected:
        ColourValue mBorderColour;
        /// Texture anisotropy.
        unsigned int mMaxAniso;
        /// Mipmap bias (always float, not Real).
        float mMipmapBias;
        UVWAddressingMode mAddressMode;
        /// Texture filtering - minification.
        FilterOptions mMinFilter;
        /// Texture filtering - magnification.
        FilterOptions mMagFilter;
        /// Texture filtering - mipmapping.
        FilterOptions mMipFilter;
        CompareFunction mCompareFunc;
        bool mCompareEnabled : 1;
        bool mDirty : 1; // flag for derived classes to sync with implementation
    };
    typedef std::shared_ptr<Sampler> SamplerPtr;

    /** Class representing the state of a single texture unit during a Pass of a
        Technique, of a Material.

        Texture units are pipelines for retrieving texture data for rendering onto
        your objects in the world. Using them is common to both the fixed-function and 
        the programmable (vertex and fragment program) pipeline, but some of the 
        settings will only have an effect in the fixed-function pipeline (for example, 
        setting a texture rotation will have no effect if you use the programmable
        pipeline, because this is overridden by the fragment program). The effect
        of each setting as regards the 2 pipelines is commented in each setting.

        When I use the term 'fixed-function pipeline' I mean traditional rendering
        where you do not use vertex or fragment programs (shaders). Programmable 
        pipeline means that for this pass you are using vertex or fragment programs.
    */
    class _OgreExport TextureUnitState : public TextureUnitStateAlloc
    {
        friend class RenderSystem;
    public:
        /** Definition of the broad types of texture effect you can apply to a texture unit.
        */
        enum TextureEffectType
        {
            /// Generate all texture coords based on angle between camera and vertex.
            ET_ENVIRONMENT_MAP,
            /// Generate texture coords based on a frustum.
            ET_PROJECTIVE_TEXTURE,
            /// Constant u/v scrolling effect.
            ET_UVSCROLL,
            /// Constant u scrolling effect.
            ET_USCROLL,
            /// Constant u/v scrolling effect.
            ET_VSCROLL,
            /// Constant rotation.
            ET_ROTATE,
            /// More complex transform.
            ET_TRANSFORM

        };

        /// %Texture coordinate generation method for environment mapping.
        enum EnvMapType
        {
            /// 2D texture coordinates using view space position. Same as ENV_CURVED on all backends.
            ENV_PLANAR,
            /// 2D texture coordinates using spherical reflection mapping
            ENV_CURVED,
            /// Cubic texture coordinates using the reflection vector
            ENV_REFLECTION,
            /// Cubic texture coordinates using the normal vector
            ENV_NORMAL
        };

        /** Useful enumeration when dealing with procedural transforms.
        */
        enum TextureTransformType
        {
            TT_TRANSLATE_U,
            TT_TRANSLATE_V,
            TT_SCALE_U,
            TT_SCALE_V,
            TT_ROTATE
        };


        static const Ogre::TextureAddressingMode TAM_WRAP = Ogre::TAM_WRAP;
        static const Ogre::TextureAddressingMode TAM_MIRROR = Ogre::TAM_MIRROR;
        static const Ogre::TextureAddressingMode TAM_CLAMP = Ogre::TAM_CLAMP;
        static const Ogre::TextureAddressingMode TAM_BORDER = Ogre::TAM_BORDER;

        OGRE_DEPRECATED typedef Ogre::TextureAddressingMode TextureAddressingMode;
        OGRE_DEPRECATED typedef Sampler::UVWAddressingMode UVWAddressingMode;

        /** Enum identifying the frame indexes for faces of a cube map (not the composite 3D type.
        */
        enum TextureCubeFace
        {
            CUBE_FRONT = 0,
            CUBE_BACK = 1,
            CUBE_LEFT = 2,
            CUBE_RIGHT = 3,
            CUBE_UP = 4,
            CUBE_DOWN = 5
        };

        /** Internal structure defining a texture effect.
        */
        struct TextureEffect {
            TextureEffectType type;
            int subtype;
            Real arg1, arg2;
            WaveformType waveType;
            Real base;
            Real frequency;
            Real phase;
            Real amplitude;
            ControllerFloat* controller;
            const Frustum* frustum;
        };

        /** Texture effects in a multimap paired array.
        */
        typedef std::multimap<TextureEffectType, TextureEffect> EffectMap;

        /** Default constructor.
        */
        TextureUnitState(Pass* parent);

        TextureUnitState(Pass* parent, const TextureUnitState& oth );

        TextureUnitState & operator = ( const TextureUnitState& oth );

        /** Default destructor.
        */
        ~TextureUnitState();

        /** Name-based constructor.
        @param parent the parent Pass object.
        @param texName
            The basic name of the texture e.g. brickwall.jpg, stonefloor.png.
        @param texCoordSet
            The index of the texture coordinate set to use.
        */
        TextureUnitState( Pass* parent, const String& texName, uint8 texCoordSet = 0);

        /** Get the name of current texture image for this layer.

            This will either always be a single name for this layer,
            or will be the name of the current frame for an animated
            or otherwise multi-frame texture.
        */
        const String& getTextureName(void) const;

        /** Sets this texture layer to use a single texture, given the
            name of the texture to use on this layer.
        */
        void setTextureName( const String& name);

        /// @overload
        void setTextureName( const String& name, TextureType ttype);

        /** Sets this texture layer to use a single texture, given the
            pointer to the texture to use on this layer.
        */
        void setTexture( const TexturePtr& texPtr);

        /**
        @deprecated use setTextureName()
        */
        OGRE_DEPRECATED void setCubicTextureName( const String& name, bool forUVW = false )
        {
            setTextureName(name, TEX_TYPE_CUBE_MAP);
        }

        /**
        @deprecated use setLayerArrayNames()
         */
        OGRE_DEPRECATED void setCubicTextureName( const String* const names, bool forUVW = false );

        /**
        @deprecated use setTexture()
        */
        OGRE_DEPRECATED void setCubicTexture( const TexturePtr* const texPtrs, bool forUVW = false )
        {
            setTexture(*texPtrs);
        }

        /** Sets the names of the texture images for an animated texture.

            Animated textures are just a series of images making up the frames of the animation. All the images
            must be the same size, and their names must have a frame number appended before the extension, e.g.
            if you specify a name of "flame.jpg" with 3 frames, the image names must be "flame_0.jpg", "flame_1.jpg"
            and "flame_2.jpg".

            You can change the active frame on a texture layer by calling the Ogre::TextureUnitState::setCurrentFrame method.
        @note
            If you can't make your texture images conform to the naming standard laid out here, you
            can call the alternative setAnimatedTextureName method which takes an array of names instead.
        @param name
            The base name of the textures to use e.g. flame.jpg for frames flame_0.jpg, flame_1.jpg etc.
        @param numFrames
            The number of frames in the sequence.
        @param duration
            The length of time it takes to display the whole animation sequence, in seconds.
            If 0, no automatic transition occurs.
        */
        void setAnimatedTextureName( const String& name, size_t numFrames, Real duration = 0 );

        /// @overload
        /// @deprecated use setAnimatedTextureName( const std::vector<String>&, Real )
        void setAnimatedTextureName( const String* const names, size_t numFrames, Real duration = 0 );

        /// @overload
        void setAnimatedTextureName( const std::vector<String>& names, Real duration = 0 )
        {
            setAnimatedTextureName(names.data(), names.size(), duration);
        }

        /// Sets this texture layer to use an array of texture maps
        void setLayerArrayNames(TextureType type, const std::vector<String>& names);

        /** Returns the width and height of the texture in the given frame.
        */
        std::pair<uint32, uint32> getTextureDimensions(unsigned int frame = 0) const;

        /** Changes the active frame in an animated or multi-image texture.

            An animated texture (or a cubic texture where the images are not combined for 3D use) is made up of
            a number of frames. This method sets the active frame.
        */
        void setCurrentFrame( unsigned int frameNumber );

        /** Gets the active frame in an animated or multi-image texture layer.
        */
        unsigned int getCurrentFrame(void) const;

        /** Gets the name of the texture associated with a frame number.
            Throws an exception if frameNumber exceeds the number of stored frames.
        */
        const String& getFrameTextureName(unsigned int frameNumber) const;

        /** Sets the name of the texture associated with a frame.
        @param name
            The name of the texture.
        @param frameNumber
            The frame the texture name is to be placed in.
        @note
            Throws an exception if frameNumber exceeds the number of stored frames.
        */
        void setFrameTextureName(const String& name, unsigned int frameNumber);

        /** Add a Texture name to the end of the frame container.
        @param name
            The name of the texture.
        */
        void addFrameTextureName(const String& name);
        /** Deletes a specific texture frame.  The texture used is not deleted but the
            texture will no longer be used by the Texture Unit.  An exception is raised
            if the frame number exceeds the number of actual frames.
        @param frameNumber
            The frame number of the texture to be deleted.
        */
        void deleteFrameTextureName(const size_t frameNumber);
        /** Gets the number of frames for a texture.
        */
        unsigned int getNumFrames(void) const;

        /** Enum identifying the type of content this texture unit contains.
        */
        enum ContentType : uint8
        {
            /// The default option, this derives texture content from a texture name, loaded by
            /// ordinary means from a file or having been manually created with a given name.
            CONTENT_NAMED = 0,
            /// A shadow texture, automatically bound by engine
            CONTENT_SHADOW = 1,
            /// This option allows you to reference a texture from a compositor, and is only valid
            /// when the pass is rendered within a compositor sequence.
            CONTENT_COMPOSITOR = 2
        };

        /** Set the type of content this TextureUnitState references.

            The default is to reference a standard named texture, but this unit
            can also reference automated content like a shadow texture.
        */
        void setContentType(ContentType ct);
        /** Get the type of content this TextureUnitState references. */
        ContentType getContentType(void) const;

        /// @deprecated use getTextureType()
        OGRE_DEPRECATED bool isCubic(void) const { return getTextureType() == TEX_TYPE_CUBE_MAP; }

        /// @deprecated use getTextureType()
        OGRE_DEPRECATED bool is3D(void) const { return getTextureType() == TEX_TYPE_CUBE_MAP; }

        /** Returns the type of this texture.
        */
        TextureType getTextureType(void) const;

        /// @copydoc Texture::setFormat
        void setDesiredFormat(PixelFormat desiredFormat);

        /// @copydoc Texture::getDesiredFormat
        PixelFormat getDesiredFormat(void) const;

        /// @copydoc Texture::setNumMipmaps
        void setNumMipmaps(int numMipmaps);

        /** Gets how many mipmaps have been requested for the texture.
        */
        int getNumMipmaps(void) const;

        /// @deprecated use setDesiredFormat(PF_A8)
        OGRE_DEPRECATED void setIsAlpha(bool isAlpha);

        /// @copydoc Texture::getGamma
        float getGamma() const;
        /// @copydoc Texture::setGamma
        void setGamma(float gamma);

        /// @copydoc Texture::setHardwareGammaEnabled
        void setHardwareGammaEnabled(bool enabled);
        /// @copydoc Texture::isHardwareGammaEnabled
        bool isHardwareGammaEnabled() const;

        /** Gets the index of the set of texture co-ords this layer uses.
        @note
        Only applies to the fixed function pipeline and has no effect if a fragment program is used.
        */
        uint8 getTextureCoordSet(void) const;

        /** Sets which texture coordinate set is to be used for this texture layer.

            A mesh can define multiple sets of texture coordinates, this sets which one this
            material uses.
        */
        void setTextureCoordSet(uint8 set);

        /// Enables Unordered Access to the provided mipLevel of the texture
        void setUnorderedAccessMipLevel(int mipLevel) { mUnorderedAccessMipLevel = mipLevel; }
        int getUnorderedAccessMipLevel() const { return mUnorderedAccessMipLevel; }

        /** Sets a matrix used to transform any texture coordinates on this layer.

            Texture coordinates can be modified on a texture layer to create effects like scrolling
            textures. A texture transform can either be applied to a layer which takes the source coordinates
            from a fixed set in the geometry, or to one which generates them dynamically (e.g. environment mapping).

            It's obviously a bit impractical to create scrolling effects by calling this method manually since you
            would have to call it every frame with a slight alteration each time, which is tedious. Instead
            you can use setTransformAnimation which will manage the
            effect over time for you.

            In addition, if you can et the individual texture transformations rather than concatenating them
            yourself.
            
            @see Ogre::TextureUnitState::setTextureScroll
            @see Ogre::TextureUnitState::setTextureScale
            @see Ogre::TextureUnitState::setTextureRotate
        */
        void setTextureTransform(const Matrix4& xform);

        /** Gets the current texture transformation matrix.

            Causes a reclaculation of the matrix if any parameters have been changed via
            setTextureScroll, setTextureScale and setTextureRotate.
        */
        const Matrix4& getTextureTransform(void) const;

        /** Sets the translation offset of the texture, ie scrolls the texture.

            This method sets the translation element of the texture transformation, and is easier to
            use than setTextureTransform if you are combining translation, scaling and rotation in your
            texture transformation. 
            If you want to animate these values use Ogre::TextureUnitState::setScrollAnimation
        @param u
            The amount the texture should be moved horizontally (u direction).
        @param v
            The amount the texture should be moved vertically (v direction).
        */
        void setTextureScroll(Real u, Real v);

        /** As setTextureScroll, but sets only U value.
        */
        void setTextureUScroll(Real value);
        /// Get texture uscroll value.
        Real getTextureUScroll(void) const;

        /** As setTextureScroll, but sets only V value.
        */
        void setTextureVScroll(Real value);
        /// Get texture vscroll value.
        Real getTextureVScroll(void) const;

        /** As setTextureScale, but sets only U value.
        */
        void setTextureUScale(Real value);
        /// Get texture uscale value.
        Real getTextureUScale(void) const;

        /** As setTextureScale, but sets only V value.
        */
        void setTextureVScale(Real value);
        /// Get texture vscale value.
        Real getTextureVScale(void) const;

        /** Sets the scaling factor applied to texture coordinates.

            This method sets the scale element of the texture transformation, and is easier to use than
            setTextureTransform if you are combining translation, scaling and rotation in your texture transformation.

            If you want to animate these values use Ogre::TextureUnitState::setTransformAnimation
        @param uScale
            The value by which the texture is to be scaled horizontally.
        @param vScale
            The value by which the texture is to be scaled vertically.
        */
        void setTextureScale(Real uScale, Real vScale);

        /** Sets the anticlockwise rotation factor applied to texture coordinates.

            This sets a fixed rotation angle - if you wish to animate this, use Ogre::TextureUnitState::setRotateAnimation
        @param angle
            The angle of rotation (anticlockwise).
        */
        void setTextureRotate(const Radian& angle);
        /// Get texture rotation effects angle value.
        const Radian& getTextureRotate(void) const;

        /// get the associated sampler
        const SamplerPtr& getSampler() const { return mSampler; }
        void setSampler(const SamplerPtr& sampler)
        {
            OgreAssert(sampler, "sampler must not be NULL");
            mSampler = sampler;
        }

        /// @copydoc Sampler::setAddressingMode
        const Sampler::UVWAddressingMode& getTextureAddressingMode(void) const
        {
            return mSampler->getAddressingMode();
        }
        /// @copydoc Sampler::setAddressingMode
        void setTextureAddressingMode( Ogre::TextureAddressingMode tam) { _getLocalSampler()->setAddressingMode(tam); }
        /// @copydoc Sampler::setAddressingMode
        void setTextureAddressingMode(Ogre::TextureAddressingMode u, Ogre::TextureAddressingMode v,
                                      Ogre::TextureAddressingMode w)
        {
            _getLocalSampler()->setAddressingMode(u, v, w);
        }
        /// @copydoc Sampler::setAddressingMode
        void setTextureAddressingMode( const Sampler::UVWAddressingMode& uvw) { _getLocalSampler()->setAddressingMode(uvw); }
        /// @copydoc Sampler::setBorderColour
        void setTextureBorderColour(const ColourValue& colour) { _getLocalSampler()->setBorderColour(colour); }
        /// @copydoc Sampler::getBorderColour
        const ColourValue& getTextureBorderColour() const { return mSampler->getBorderColour(); }
        /// @copydoc Sampler::setFiltering(TextureFilterOptions)
        void setTextureFiltering(TextureFilterOptions filterType)
        {
            _getLocalSampler()->setFiltering(filterType);
        }
        /// @copydoc Sampler::setFiltering(FilterType, FilterOptions)
        void setTextureFiltering(FilterType ftype, FilterOptions opts)
        {
            _getLocalSampler()->setFiltering(ftype, opts);
        }
        /// @copydoc Sampler::setFiltering(FilterOptions, FilterOptions, FilterOptions)
        void setTextureFiltering(FilterOptions minFilter, FilterOptions magFilter, FilterOptions mipFilter)
        {
            _getLocalSampler()->setFiltering(minFilter, magFilter, mipFilter);
        }
        /// @copydoc Sampler::getFiltering
        FilterOptions getTextureFiltering(FilterType ftype) const { return mSampler->getFiltering(ftype); }
        /// @copydoc Sampler::setCompareEnabled
        void setTextureCompareEnabled(bool enabled) { _getLocalSampler()->setCompareEnabled(enabled); }
        /// @copydoc Sampler::getCompareEnabled
        bool getTextureCompareEnabled() const { return mSampler->getCompareEnabled(); }
        /// @copydoc Sampler::setCompareFunction
        void setTextureCompareFunction(CompareFunction function) { _getLocalSampler()->setCompareFunction(function); }
        /// @copydoc Sampler::getCompareFunction
        CompareFunction getTextureCompareFunction() const { return mSampler->getCompareFunction(); }
        /// @copydoc Sampler::setAnisotropy
        void setTextureAnisotropy(unsigned int maxAniso) { _getLocalSampler()->setAnisotropy(maxAniso); }
        /// @copydoc Sampler::getAnisotropy
        unsigned int getTextureAnisotropy() const { return mSampler->getAnisotropy(); }
        /// @copydoc Sampler::setMipmapBias
        void setTextureMipmapBias(float bias) { _getLocalSampler()->setMipmapBias(bias); }
        /// @copydoc Sampler::getMipmapBias
        float getTextureMipmapBias(void) const { return mSampler->getMipmapBias(); }


        /** Setting advanced blending options.

            This is an extended version of the Ogre::TextureUnitState::setColourOperation method which allows
            extremely detailed control over the blending applied between this and earlier layers.
            See the Warning below about the issues between mulitpass and multitexturing that
            using this method can create.

            Texture colour operations determine how the final colour of the surface appears when
            rendered. Texture units are used to combine colour values from various sources (ie. the
            diffuse colour of the surface from lighting calculations, combined with the colour of
            the texture). This method allows you to specify the 'operation' to be used, ie. the
            calculation such as adds or multiplies, and which values to use as arguments, such as
            a fixed value or a value from a previous calculation.

            The defaults for each layer are:
            - op = Ogre::LBX_MODULATE
            - source1 = Ogre::LBS_TEXTURE
            - source2 = Ogre::LBS_CURRENT

            ie. each layer takes the colour results of the previous layer, and multiplies them
            with the new texture being applied. Bear in mind that colours are RGB values from
            0.0 - 1.0 so multiplying them together will result in values in the same range,
            'tinted' by the multiply. Note however that a straight multiply normally has the
            effect of darkening the textures - for this reason there are brightening operations
            like Ogre::LBX_MODULATE_X2. See the Ogre::LayerBlendOperation and Ogre::LayerBlendSource enumerated
            types for full details.

            The final 3 parameters are only required if you decide to pass values manually
            into the operation, i.e. you want one or more of the inputs to the colour calculation
            to come from a fixed value that you supply. Hence you only need to fill these in if
            you supply Ogre::LBS_MANUAL to the corresponding source, or use the Ogre::LBX_BLEND_MANUAL
            operation.
        @warning
            Ogre tries to use multitexturing hardware to blend texture layers
            together. However, if it runs out of texturing units (e.g. 2 of a GeForce2, 4 on a
            GeForce3) it has to fall back on multipass rendering, i.e. rendering the same object
            multiple times with different textures. This is both less efficient and there is a smaller
            range of blending operations which can be performed. For this reason, if you use this method
            you MUST also call Ogre::TextureUnitState::setColourOpMultipassFallback to specify which effect you
            want to fall back on if sufficient hardware is not available.
        @warning
            If you wish to avoid having to do this, use the simpler Ogre::TextureUnitState::setColourOperation method
            which allows less flexible blending options but sets up the multipass fallback automatically,
            since it only allows operations which have direct multipass equivalents.
        @param op
            The operation to be used, e.g. modulate (multiply), add, subtract.
        @param source1
            The source of the first colour to the operation e.g. texture colour.
        @param source2
            The source of the second colour to the operation e.g. current surface colour.
        @param arg1
            Manually supplied colour value (only required if source1 = LBS_MANUAL).
        @param arg2
            Manually supplied colour value (only required if source2 = LBS_MANUAL).
        @param manualBlend
            Manually supplied 'blend' value - only required for operations
            which require manual blend e.g. LBX_BLEND_MANUAL.
        */
        void setColourOperationEx(
            LayerBlendOperationEx op,
            LayerBlendSource source1 = LBS_TEXTURE,
            LayerBlendSource source2 = LBS_CURRENT,

            const ColourValue& arg1 = ColourValue::White,
            const ColourValue& arg2 = ColourValue::White,

            Real manualBlend = 0.0);

        /** Determines how this texture layer is combined with the one below it (or the diffuse colour of
            the geometry if this is layer 0).

            This method is the simplest way to blend texture layers, because it requires only one parameter,
            gives you the most common blending types, and automatically sets up 2 blending methods: one for
            if single-pass multitexturing hardware is available, and another for if it is not and the blending must
            be achieved through multiple rendering passes. It is, however, quite limited and does not expose
            the more flexible multitexturing operations, simply because these can't be automatically supported in
            multipass fallback mode. If want to use the fancier options, use Ogre::TextureUnitState::setColourOperationEx,
            but you'll either have to be sure that enough multitexturing units will be available, or you should
            explicitly set a fallback using Ogre::TextureUnitState::setColourOpMultipassFallback.
        @note
            The default method is Ogre::LBO_MODULATE for all layers.
        @param op
            One of the Ogre::LayerBlendOperation enumerated blending types.
        */
        void setColourOperation( const LayerBlendOperation op);

        /** Sets the multipass fallback operation for this layer, if you used TextureUnitState::setColourOperationEx
            and not enough multitexturing hardware is available.

            Because some effects exposed using Ogre::TextureUnitState::setColourOperationEx are only supported under
            multitexturing hardware, if the hardware is lacking the system must fallback on multipass rendering,
            which unfortunately doesn't support as many effects. This method is for you to specify the fallback
            operation which most suits you.

            You'll notice that the interface is the same as the Ogre::TMaterial::setSceneBlending method; this is
            because multipass rendering IS effectively scene blending, since each layer is rendered on top
            of the last using the same mechanism as making an object transparent, it's just being rendered
            in the same place repeatedly to get the multitexture effect.

            If you use the simpler (and hence less flexible) Ogre::TextureUnitState::setColourOperation method you
            don't need to call this as the system sets up the fallback for you.
        @note
            This option has no effect in the programmable pipeline, because there is no multipass fallback
            and multitexture blending is handled by the fragment shader.
        */
        void setColourOpMultipassFallback( const SceneBlendFactor sourceFactor, const SceneBlendFactor destFactor);

        /** Get multitexturing colour blending mode.
        */
        const LayerBlendModeEx& getColourBlendMode(void) const;

        /** Get multitexturing alpha blending mode.
        */
        const LayerBlendModeEx& getAlphaBlendMode(void) const;

        /** Get the multipass fallback for colour blending operation source factor.
        */
        SceneBlendFactor getColourBlendFallbackSrc(void) const;

        /** Get the multipass fallback for colour blending operation destination factor.
        */
        SceneBlendFactor getColourBlendFallbackDest(void) const;

        /** Sets the alpha operation to be applied to this texture.

            This works in exactly the same way as setColourOperationEx, except
            that the effect is applied to the level of alpha (i.e. transparency)
            of the texture rather than its colour. When the alpha of a texel (a pixel
            on a texture) is 1.0, it is opaque, whereas it is fully transparent if the
            alpha is 0.0. Please refer to the Ogre::TextureUnitState::setColourOperationEx method for more info.
        @param op
            The operation to be used, e.g. modulate (multiply), add, subtract
        @param source1
            The source of the first alpha value to the operation e.g. texture alpha
        @param source2
            The source of the second alpha value to the operation e.g. current surface alpha
        @param arg1
            Manually supplied alpha value (only required if source1 = Ogre::LBS_MANUAL)
        @param arg2
            Manually supplied alpha value (only required if source2 = Ogre::LBS_MANUAL)
        @param manualBlend
            Manually supplied 'blend' value - only required for operations
            which require manual blend e.g. Ogre::LBX_BLEND_MANUAL
        */
        void setAlphaOperation(LayerBlendOperationEx op,
            LayerBlendSource source1 = LBS_TEXTURE,
            LayerBlendSource source2 = LBS_CURRENT,
            Real arg1 = 1.0,
            Real arg2 = 1.0,
            Real manualBlend = 0.0);

        /** Generic method for setting up texture effects.

            Allows you to specify effects directly by using the #TextureEffectType enumeration. The
            arguments that go with it depend on the effect type. Only one effect of
            each type can be applied to a texture layer.
        @par
            This method is used internally by Ogre but it is better generally for applications to use the
            more intuitive specialised methods such as #setEnvironmentMap and #setTextureScroll.
        */
        void addEffect(TextureEffect& effect);

        /** Turns on/off texture coordinate effect that makes this layer an environment map.

            Environment maps make an object look reflective by using the object's vertex normals relative
            to the camera view to generate texture coordinates.

            The vectors generated can either be used to address a single 2D texture which
            is a 'fish-eye' lens view of a scene, or a 3D cubic environment map which requires 6 textures
            for each side of the inside of a cube.

            This effect works best if the object has lots of gradually changing normals. The texture also
            has to be designed for this effect - see the example spheremap.png included with the sample
            application for a 2D environment map; a cubic map can be generated by rendering 6 views of a
            scene to each of the cube faces with orthogonal views.

            Enabling this disables any other texture coordinate generation effects.
            However it can be combined with texture coordinate modification functions, which then operate on the
            generated coordinates rather than static model texture coordinates.
        @param enable
            True to enable, false to disable
        @param texGenType texture coordinate generation type
        */
        void setEnvironmentMap(bool enable, EnvMapType texGenType = ENV_CURVED);

        /** Sets up an animated scroll for the texture layer.

            Useful for creating constant scrolling effects on a texture layer (for varying scrolls, see Ogre::TextureUnitState::setTransformAnimation).
        @param uSpeed
            The number of horizontal loops per second (+ve=moving right, -ve = moving left).
        @param vSpeed
            The number of vertical loops per second (+ve=moving up, -ve= moving down).
        */
        void setScrollAnimation(Real uSpeed, Real vSpeed);

        /** Sets up an animated texture rotation for this layer.

            Useful for constant rotations (for varying rotations, see Ogre::TextureUnitState::setTransformAnimation).
        @param speed
            The number of complete anticlockwise revolutions per second (use -ve for clockwise)
        */
        void setRotateAnimation(Real speed);

        /** Sets up a general time-relative texture modification effect.

            This can be called multiple times for different values of ttype, but only the latest effect
            applies if called multiple time for the same ttype.
        @param ttype
            The type of transform, either translate (scroll), scale (stretch) or rotate (spin).
        @param waveType
            The shape of the wave, see Ogre::WaveformType enum for details.
        @param base
            The base value for the function (range of output = {base, base + amplitude}).
        @param frequency
            The speed of the wave in cycles per second.
        @param phase
            The offset of the start of the wave, e.g. 0.5 to start half-way through the wave.
        @param amplitude
            Scales the output so that instead of lying within 0..1 it lies within 0..1*amplitude for exaggerated effects.
        */
        void setTransformAnimation( const TextureTransformType ttype,
            const WaveformType waveType, Real base = 0, Real frequency = 1, Real phase = 0, Real amplitude = 1 );


        /** Enables or disables projective texturing on this texture unit.

            Projective texturing allows you to generate texture coordinates 
            based on a Frustum, which gives the impression that a texture is
            being projected onto the surface. Note that once you have called
            this method, the texture unit continues to monitor the Frustum you 
            passed in and the projection will change if you can alter it. It also
            means that you must ensure that the Frustum object you pass a pointer
            to remains in existence for as long as this TextureUnitState does.
        @par
            This effect cannot be combined with other texture generation effects, 
            such as environment mapping. It also has no effect on passes which 
            have a vertex program enabled - projective texturing has to be done
            in the vertex program instead.
        @param enabled
            Whether to enable / disable.
        @param projectionSettings
            The Frustum which will be used to derive the 
            projection parameters.
        */
        void setProjectiveTexturing(bool enabled, const Frustum* projectionSettings = 0);

        /** Removes all effects applied to this texture layer.
        */
        void removeAllEffects(void);

        /** Removes a single effect applied to this texture layer.
        @note
            Because you can only have 1 effect of each type (e.g. 1 texture coordinate generation) applied
            to a layer, only the effect type is required.
        */
        void removeEffect( const TextureEffectType type );

        /** Determines if this texture layer is currently blank.
        @note
            This can happen if a texture fails to load or some other non-fatal error. Worth checking after
            setting texture name.
        */
        bool isBlank(void) const;

        /** Sets this texture layer to be blank.
        */
        void setBlank(void);

        /** Tests if the texture associated with this unit has failed to load.
        */
        bool isTextureLoadFailing() const { return mTextureLoadFailed; }

        /** Tells the unit to retry loading the texture if it had failed to load.
        */
        void retryTextureLoad() { mTextureLoadFailed = false; }

        /// Get texture effects in a multimap paired array.
        const EffectMap& getEffects(void) const;
        /// Get the animated-texture animation duration.
        Real getAnimationDuration(void) const;

        /// Returns true if this texture unit is using the default Sampler
        bool isDefaultFiltering() const;

        /** Set the compositor reference for this texture unit state.

            Only valid when content type is compositor.
        @param compositorName
            The name of the compositor to reference.
        @param textureName
            The name of the texture to reference.
        @param mrtIndex
            The index of the wanted texture, if referencing an MRT.
        */
        void setCompositorReference(const String& compositorName, const String& textureName, uint32 mrtIndex = 0);

        /** Gets the name of the compositor that this texture references. */
        const String& getReferencedCompositorName() const { return mCompositorRefName; }
        /** Gets the name of the texture in the compositor that this texture references. */
        const String& getReferencedTextureName() const { return mCompositorRefTexName; }
        /** Gets the MRT index of the texture in the compositor that this texture references. */ 
        uint32 getReferencedMRTIndex() const { return mCompositorRefMrtIndex; }
    
        /// Gets the parent Pass object.
        Pass* getParent(void) const { return mParent; }

        /** Internal method for preparing this object for load, as part of Material::prepare. */
        void _prepare(void);
        /** Internal method for undoing the preparation this object as part of Material::unprepare. */
        void _unprepare(void);
        /** Internal method for loading this object as part of Material::load. */
        void _load(void);
        /** Internal method for unloading this object as part of Material::unload. */
        void _unload(void);

        /// Is this loaded?
        bool isLoaded(void) const;
        /** Tells the class that it needs recompilation. */
        void _notifyNeedsRecompile(void);

        /** Set the name of the Texture Unit State.

            The name of the Texture Unit State is optional.  Its useful in material scripts where a material could inherit
            from another material and only want to modify a particular Texture Unit State.
        */
        void setName(const String& name);
        /// Get the name of the Texture Unit State.
        const String& getName(void) const { return mName; }

        /// @deprecated use setName()
        OGRE_DEPRECATED void setTextureNameAlias(const String& name) { setName(name); }
        /// @deprecated use getName()
        OGRE_DEPRECATED const String& getTextureNameAlias(void) const { return getName();}

        /** Notify this object that its parent has changed. */
        void _notifyParent(Pass* parent);

        /** Get the texture pointer for the current frame. */
        const TexturePtr& _getTexturePtr(void) const;
        /** Get the texture pointer for a given frame. */
        const TexturePtr& _getTexturePtr(size_t frame) const;
    
        /** Set the texture pointer for the current frame (internal use only!). */
        void _setTexturePtr(const TexturePtr& texptr);
        /** Set the texture pointer for a given frame (internal use only!). */
        void _setTexturePtr(const TexturePtr& texptr, size_t frame);

        size_t calculateSize(void) const;

        /** Gets the animation controller (as created because of setAnimatedTexture)
            if it exists.
        */
        ControllerFloat* _getAnimController() const { return mAnimController; }

        /// return a sampler local to this TUS instead of the shared global one
        const SamplerPtr& _getLocalSampler();

        TexCoordCalcMethod _deriveTexCoordCalcMethod() const;
private:
        // State
        /// The current animation frame.
        unsigned int mCurrentFrame;

        /// Duration of animation in seconds.
        Real mAnimDuration;

        int mUnorderedAccessMipLevel;

        LayerBlendModeEx mColourBlendMode;
        SceneBlendFactor mColourBlendFallbackSrc;
        SceneBlendFactor mColourBlendFallbackDest;

        LayerBlendModeEx mAlphaBlendMode;
        Real mGamma;
        Real mUMod, mVMod;
        Real mUScale, mVScale;
        Radian mRotate;
        mutable Matrix4 mTexModMatrix;

        /// Content type of texture (normal loaded texture, auto-texture).
        ContentType mContentType;

        mutable bool mTextureLoadFailed;
        mutable bool mRecalcTexMatrix;

        uint8 mTextureCoordSetIndex;

        /// The index of the referenced texture if referencing an MRT in a compositor.
        uint32 mCompositorRefMrtIndex;

        //-----------------------------------------------------------------------------
        // Complex members (those that can't be copied using memcpy) are at the end to 
        // allow for fast copying of the basic members.
        //
        mutable std::vector<TexturePtr> mFramePtrs; // must at least contain a single nullptr
        SamplerPtr mSampler;
        String mName;               ///< Optional name for the TUS.
        EffectMap mEffects;
        /// The data that references the compositor.
        String mCompositorRefName;
        String mCompositorRefTexName;
        //-----------------------------------------------------------------------------

        //-----------------------------------------------------------------------------
        // Pointer members (those that can't be copied using memcpy), and MUST
        // preserving even if assign from others
        //
        Pass* mParent;
        ControllerFloat* mAnimController;
        //-----------------------------------------------------------------------------


        /** Internal method for calculating texture matrix.
        */
        void recalcTextureMatrix(void) const;

        /** Internal method for creating animation controller.
        */
        void createAnimController(void);

        /** Internal method for creating texture effect controller.
        */
        void createEffectController(TextureEffect& effect);

        /** Internal method for ensuring the texture for a given frame is prepared. */
        void ensurePrepared(size_t frame) const;
        /** Internal method for ensuring the texture for a given frame is loaded. */
        void ensureLoaded(size_t frame) const;

        TexturePtr retrieveTexture(const String& name);

        bool checkTexCalcSettings(const TexturePtr& tex) const;
    };

    /** @} */
    /** @} */

} // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif // __TextureUnitState_H__
