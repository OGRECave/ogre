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
#ifndef _Material_H__
#define _Material_H__

#include "OgrePrerequisites.h"

#include "OgreResource.h"
#include "OgreCommon.h"
#include "OgreColourValue.h"
#include "OgreBlendMode.h"
#include "OgreHeaderPrefix.h"
#include "OgreSharedPtr.h"

namespace Ogre {

    // Forward declaration
    class LodStrategy;
    template <typename T> class ConstVectorIterator;
    template <typename T> class VectorIterator;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Materials
    *  @{
    */
    /** Class encapsulates rendering properties of an object.

    %Ogre's material class encapsulates *all* aspects of the visual appearance,
    of an object. It also includes other flags which 
    might not be traditionally thought of as material properties such as 
    culling modes and depth buffer settings, but these affect the 
    appearance of the rendered object and are convenient to attach to the 
    material since it keeps all the settings in one place. This is 
    different to Direct3D which treats a material as just the colour 
    components (diffuse, specular) and not texture maps etc. An Ogre 
    Material can be thought of as equivalent to a 'Shader'.

    A Material can be rendered in multiple different ways depending on the
    hardware available. You may configure a Material to use high-complexity
    fragment shaders, but these won't work on every card; therefore a Technique
    is an approach to creating the visual effect you are looking for. You are advised
    to create fallback techniques with lower hardware requirements if you decide to
    use advanced features. In addition, you also might want lower-detail techniques
    for distant geometry.

    Each Technique can be made up of multiple passes. A fixed-function Pass
    may combine multiple texture layers using multitexturing, but Ogre can 
    break that into multiple passes automatically if the active card cannot
    handle that many simultaneous textures. Programmable passes, however, cannot
    be split down automatically, so if the active graphics card cannot handle the
    technique which contains these passes, OGRE will try to find another technique
    which the card can do. If, at the end of the day, the card cannot handle any of the
    techniques which are listed for the material, the engine will render the 
    geometry plain white, which should alert you to the problem.

    %Ogre comes configured with a number of default settings for a newly 
    created material. These can be changed if you wish by retrieving the 
    default material settings through 
    MaterialManager::getDefaultSettings. Any changes you make to the
    Material returned from this method will apply to any materials created 
    from this point onward.
    */
    class _OgreExport Material final : public Resource
    {
        friend class SceneManager;
        friend class MaterialManager;

    public:
        /// distance list used to specify LOD
        typedef std::vector<Real> LodValueList;
        typedef ConstVectorIterator<LodValueList> LodValueIterator;
        typedef std::vector<Technique*> Techniques;
    private:


        /** Internal method which sets the material up from the default settings.
        */
        void applyDefaults(void);

        /// All techniques, supported and unsupported
        Techniques mTechniques;
        /// Supported techniques of any sort
        Techniques mSupportedTechniques;
        typedef std::map<unsigned short, Technique*> LodTechniques;
        typedef std::map<unsigned short, LodTechniques> BestTechniquesBySchemeList;
        /** Map of scheme -> list of LOD techniques. 
            Current scheme is set on MaterialManager,
            and can be set per Viewport for auto activation.
        */
        BestTechniquesBySchemeList mBestTechniquesBySchemeList;

        LodValueList mUserLodValues;
        LodValueList mLodValues;
        const LodStrategy *mLodStrategy;
        /// Text description of why any techniques are not supported
        String mUnsupportedReasons;
        bool mReceiveShadows;
        bool mTransparencyCastsShadows;
        /// Does this material require compilation?
        bool mCompilationRequired;

        /** Insert a supported technique into the local collections. */
        void insertSupportedTechnique(Technique* t);

        /** Clear the best technique list.
        */
        void clearBestTechniqueList(void);

        void prepareImpl(void) override;
        void unprepareImpl(void) override;
        void loadImpl(void) override;

        /** Unloads the material, frees resources etc.
        @see
        Resource
        */
        void unloadImpl(void) override;
        /// @copydoc Resource::calculateSize
        size_t calculateSize(void) const override;
    public:

        /** Constructor - use resource manager's create method rather than this.
        */
        Material(ResourceManager* creator, const String& name, ResourceHandle handle,
            const String& group, bool isManual = false, ManualResourceLoader* loader = 0);

        ~Material();
        /** Assignment operator to allow easy copying between materials.
        */
        Material& operator=( const Material& rhs );

        /** Determines if the material has any transparency with the rest of the scene (derived from 
            whether any Techniques say they involve transparency).
        */
        bool isTransparent(void) const;

        /** Sets whether objects using this material will receive shadows.

            This method allows a material to opt out of receiving shadows, if
            it would otherwise do so. Shadows will not be cast on any objects
            unless the scene is set up to support shadows 
            (@see SceneManager::setShadowTechnique), and not all techniques cast
            shadows on all objects. In any case, if you have a need to prevent
            shadows being received by material, this is the method you call to
            do it.
        @note 
            Transparent materials never receive shadows despite this setting. 
            The default is to receive shadows.
        */
        void setReceiveShadows(bool enabled) { mReceiveShadows = enabled; }
        /** Returns whether or not objects using this material will receive shadows. */
        bool getReceiveShadows(void) const { return mReceiveShadows; }

        /** Sets whether objects using this material be classified as opaque to the shadow caster system.

        This method allows a material to cast a shadow, even if it is transparent.
        By default, transparent materials neither cast nor receive shadows. Shadows
        will not be cast on any objects unless the scene is set up to support shadows 
        (@see SceneManager::setShadowTechnique), and not all techniques cast
        shadows on all objects.
        */
        void setTransparencyCastsShadows(bool enabled) { mTransparencyCastsShadows = enabled; }
        /** Returns whether or not objects using this material be classified as opaque to the shadow caster system. */
        bool getTransparencyCastsShadows(void) const { return mTransparencyCastsShadows; }

        typedef VectorIterator<Techniques> TechniqueIterator;
        /// @name Techniques
        /// @{
        /** Creates a new Technique for this Material.

            A Technique is a single way of rendering geometry in order to achieve the effect
            you are intending in a material. There are many reason why you would want more than
            one - the main one being to handle variable graphics card abilities; you might have
            one technique which is impressive but only runs on 4th-generation graphics cards, 
            for example. In this case you will want to create at least one fallback Technique.
            OGRE will work out which Techniques a card can support and pick the best one.
        @par
            If multiple Techniques are available, the order in which they are created is 
            important - the engine will consider lower-indexed Techniques to be preferable
            to higher-indexed Techniques, ie when asked for the 'best' technique it will
            return the first one in the technique list which is supported by the hardware.
        */
        Technique* createTechnique(void);
        /** Gets the indexed technique. */
        Technique* getTechnique(size_t index) const { return mTechniques.at(index); }
        /** searches for the named technique.
            Return 0 if technique with name is not found
        */
        Technique* getTechnique(const String& name) const;
        /** Retrieves the number of techniques.  */
        size_t getNumTechniques(void) const { return mTechniques.size(); }
        /** Removes the technique at the given index. */        
        void removeTechnique(unsigned short index);     
        /** Removes all the techniques in this Material. */
        void removeAllTechniques(void);
        /** Get an iterator over the Techniques in this Material.
         * @deprecated use getTechniques() */
        OGRE_DEPRECATED TechniqueIterator getTechniqueIterator(void);

        /** Get the Techniques in this Material. */
        const Techniques& getTechniques(void) const {
            return mTechniques;
        }

        /** Gets all the Techniques which are supported by the current card.

            The supported technique list is only available after this material has been compiled,
            which typically happens on loading the material. Therefore, if this method returns
            an empty list, try calling Material::load.
        */
        const Techniques& getSupportedTechniques(void) const {
            return mSupportedTechniques;
        }

        /// @deprecated use getSupportedTechniques()
        OGRE_DEPRECATED TechniqueIterator getSupportedTechniqueIterator(void);
        
        /** Gets the indexed supported technique. */
        Technique* getSupportedTechnique(size_t index) const { return mSupportedTechniques.at(index); }
        /** Retrieves the number of supported techniques. */
        size_t getNumSupportedTechniques(void) const { return mSupportedTechniques.size(); }
        /** Gets a string explaining why any techniques are not supported. */
        const String& getUnsupportedTechniquesExplanation() const { return mUnsupportedReasons; }

        /** Gets the best supported technique. 

            This method returns the lowest-index supported Technique in this material
            (since lower-indexed Techniques are considered to be better than higher-indexed
            ones).
        @par
            The best supported technique is only available after this material has been compiled,
            which typically happens on loading the material. Therefore, if this method returns
            NULL, try calling Material::load.
        @param lodIndex The material LOD index to use
        @param rend Optional parameter specifying the Renderable that is requesting
            this technique. Only used if no valid technique for the active material 
            scheme is found, at which point it is passed to 
            MaterialManager::Listener::handleSchemeNotFound as information.
        */
        Technique* getBestTechnique(unsigned short lodIndex = 0, const Renderable* rend = 0);
        /// @}

        /** Creates a new copy of this material with the same settings but a new name.
        @param newName The name for the cloned material
        @param newGroup
            Optional name of the new group to assign the clone to;
            if you leave this blank, the clone will be assigned to the same
            group as this Material.
        */
        MaterialPtr clone(const String& newName, const String& newGroup = BLANKSTRING) const;

        // needed because of deprecated variant below
        MaterialPtr clone(const String& newName, const char* newGroup) const { return clone(newName, String(newGroup)); }

        /// @deprecated use clone(const String&, const String&)
        OGRE_DEPRECATED MaterialPtr clone(const String& newName, bool changeGroup,
                                          const String& newGroup = BLANKSTRING) const
        {
            return clone(newName, newGroup);
        }

        /** Copies the details of this material into another, preserving the target's handle and name
        (unlike operator=) but copying everything else.
        @param mat Weak reference to material which will receive this material's settings.
        */
        void copyDetailsTo(MaterialPtr& mat) const;

        /** 'Compiles' this Material.

            Compiling a material involves determining which Techniques are supported on the
            card on which OGRE is currently running, and for fixed-function Passes within those
            Techniques, splitting the passes down where they contain more TextureUnitState 
            instances than the current card has texture units.
        @par
            This process is automatically done when the Material is loaded, but may be
            repeated if you make some procedural changes.
        @param
            autoManageTextureUnits If true, when a fixed function pass has too many TextureUnitState
                entries than the card has texture units, the Pass in question will be split into
                more than one Pass in order to emulate the Pass. If you set this to false and
                this situation arises, an Exception will be thrown.
        */
        void compile(bool autoManageTextureUnits = true);

        /** @name Forwarded Pass Properties

            The following methods are to make migration from previous versions simpler
            and to make code easier to write when dealing with simple materials
            They set the properties which have been moved to Pass for all Techniques and all Passes
        */

        /// @{
        /** Sets the point size properties for every Pass in every Technique.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setPointSize
        */
        void setPointSize(Real ps);

        /** Sets the ambient colour reflectance properties for every Pass in every Technique.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setAmbient
        */
        void setAmbient(float red, float green, float blue);

        /// @overload
        void setAmbient(const ColourValue& ambient);

        /** Sets the diffuse colour reflectance properties of every Pass in every Technique.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setDiffuse
        */
        void setDiffuse(float red, float green, float blue, float alpha);

        /// @overload
        void setDiffuse(const ColourValue& diffuse);

        /** Sets the specular colour reflectance properties of every Pass in every Technique.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setSpecular
        */
        void setSpecular(float red, float green, float blue, float alpha);

        /// @overload
        void setSpecular(const ColourValue& specular);

        /** Sets the shininess properties of every Pass in every Technique.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setShininess
        */
        void setShininess(Real val);

        /** Sets the amount of self-illumination of every Pass in every Technique.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setSelfIllumination
        */
        void setSelfIllumination(float red, float green, float blue);

        /// @overload
        void setSelfIllumination(const ColourValue& selfIllum);

        /** Sets whether or not each Pass renders with depth-buffer checking on or not.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setDepthCheckEnabled
        */
        void setDepthCheckEnabled(bool enabled);

        /** Sets whether or not each Pass renders with depth-buffer writing on or not.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setDepthWriteEnabled
        */
        void setDepthWriteEnabled(bool enabled);

        /** Sets the function used to compare depth values when depth checking is on.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setDepthFunction
        */
        void setDepthFunction( CompareFunction func );

        /** Sets whether or not colour buffer writing is enabled for each Pass.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setColourWriteEnabled
        */
        void setColourWriteEnabled(bool enabled);

        /** Sets which colour buffer channels are enabled for writing for each Pass.
         @see Pass::setColourWriteEnabled
         */
        void setColourWriteEnabled(bool red, bool green, bool blue, bool alpha);

        /** Sets the culling mode for each pass  based on the 'vertex winding'.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setCullingMode
        */
        void setCullingMode( CullingMode mode );

        /** Sets the manual culling mode, performed by CPU rather than hardware.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setManualCullingMode
        */
        void setManualCullingMode( ManualCullingMode mode );

        /** Sets whether or not dynamic lighting is enabled for every Pass.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setLightingEnabled
        */
        void setLightingEnabled(bool enabled);

        /** Sets the type of light shading required
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setShadingMode
        */
        void setShadingMode( ShadeOptions mode );

        /** Sets the fogging mode applied to each pass.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setFog
        */
        void setFog(
            bool overrideScene,
            FogMode mode = FOG_NONE,
            const ColourValue& colour = ColourValue::White,
            Real expDensity = 0.001f, Real linearStart = 0.0f, Real linearEnd = 1.0f );

        /** Sets the depth bias to be used for each Pass.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setDepthBias
        */
        void setDepthBias(float constantBias, float slopeScaleBias);

        /** Set texture filtering for every texture unit in every Technique and Pass
        @note
            This property has been moved to the TextureUnitState class, which is accessible via the 
            Technique and Pass. For simplicity, this method allows you to set these properties for 
            every current TeextureUnitState, If you need more precision, retrieve the Technique, 
            Pass and TextureUnitState instances and set the property there.
        @see TextureUnitState::setTextureFiltering
        */
        void setTextureFiltering(TextureFilterOptions filterType);
        /** Sets the anisotropy level to be used for all textures.
        @note
            This property has been moved to the TextureUnitState class, which is accessible via the 
            Technique and Pass. For simplicity, this method allows you to set these properties for 
            every current TeextureUnitState, If you need more precision, retrieve the Technique, 
            Pass and TextureUnitState instances and set the property there.
        @see TextureUnitState::setTextureAnisotropy
        */
        void setTextureAnisotropy(int maxAniso);

        /** Sets the kind of blending every pass has with the existing contents of the scene.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setSceneBlending
        */
        void setSceneBlending( const SceneBlendType sbt );

        /** Sets the kind of blending every pass has with the existing contents of the scene, using individual factors for color and alpha channels
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setSeparateSceneBlending
        */
        void setSeparateSceneBlending( const SceneBlendType sbt, const SceneBlendType sbta );

        /** Allows very fine control of blending every Pass with the existing contents of the scene.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setSceneBlending
        */
        void setSceneBlending( const SceneBlendFactor sourceFactor, const SceneBlendFactor destFactor);

        /** Allows very fine control of blending every Pass with the existing contents of the scene, using individual factors for color and alpha channels
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setSeparateSceneBlending
        */
        void setSeparateSceneBlending( const SceneBlendFactor sourceFactor, const SceneBlendFactor destFactor, const SceneBlendFactor sourceFactorAlpha, const SceneBlendFactor destFactorAlpha);
        /// @}

        /** Tells the material that it needs recompilation. */
        void _notifyNeedsRecompile(void);

        /// @name Level of Detail
        /// @{
        /** Gets the number of levels-of-detail this material has in the
            given scheme, based on Technique::setLodIndex.

            Note that this will not be up to date until the material has been compiled.
        */
        unsigned short getNumLodLevels(unsigned short schemeIndex) const;
        /** Gets the number of levels-of-detail this material has in the
            given scheme, based on Technique::setLodIndex.

            Note that this will not be up to date until the material has been compiled.
        */
        unsigned short getNumLodLevels(const String& schemeName) const;
        /** Sets the distance at which level-of-detail (LOD) levels come into effect.

            You should only use this if you have assigned LOD indexes to the Technique
            instances attached to this Material. If you have done so, you should call this
            method to determine the distance at which the lowe levels of detail kick in.
            The decision about what distance is actually used is a combination of this
            and the LOD bias applied to both the current Camera and the current Entity.
        @param lodValues A vector of Reals which indicate the LOD value at which to 
            switch to lower details. They are listed in LOD index order, starting at index
            1 (ie the first level down from the highest level 0, which automatically applies
            from a value of 0). These are 'user values', before being potentially 
            transformed by the strategy, so for the distance strategy this is an
            unsquared distance for example.
        */
        void setLodLevels(const LodValueList& lodValues);

        /** Gets the list of values transformed by the LodStrategy at which each LOD comes into effect.

            Note that the iterator returned from this method is not totally analogous to 
            the one passed in by calling setLodLevels - the list includes a zero
            entry at the start (since the highest LOD starts at value 0). Also, the
            values returned are after being transformed by LodStrategy::transformUserValue.
        */
        const LodValueList& getLodValues(void) const {
            return mLodValues;
        }

        /// @deprecated use getLodValues()
        OGRE_DEPRECATED LodValueIterator getLodValueIterator(void) const;

        /** Gets the user-defined list of values which are internally transformed by the LodStrategy.

            Note that the iterator returned from this method is not totally analogous to 
            the one passed in by calling setLodLevels - the list includes a zero
            entry at the start (since the highest LOD starts at value 0). Also, the
            values returned are after being transformed by LodStrategy::transformUserValue.
        */
        const LodValueList& getUserLodValues(void) const {
            return mUserLodValues;
        }

        /// @deprecated use getUserLodValues()
        OGRE_DEPRECATED LodValueIterator getUserLodValueIterator(void) const;

        /** Gets the LOD index to use at the given value. 
        @note The value passed in is the 'transformed' value. If you are dealing with
        an original source value (e.g. distance), use LodStrategy::transformUserValue
        to turn this into a lookup value.
        */
        ushort getLodIndex(Real value) const;

        /** Get LOD strategy used by this material. */
        const LodStrategy *getLodStrategy() const;
        /** Set the LOD strategy used by this material. */
        void setLodStrategy(LodStrategy *lodStrategy);
        /// @}

        void touch(void) override
        { 
            if (mCompilationRequired) 
                compile();
            // call superclass
            Resource::touch();
        }

        /** Gets the compilation status of the material.
        @return True if the material needs recompilation.
        */
        bool getCompilationRequired() const
        {
            return mCompilationRequired;
        }


    };
    /** @} */
    /** @} */

} //namespace 

#include "OgreHeaderSuffix.h"

#endif
