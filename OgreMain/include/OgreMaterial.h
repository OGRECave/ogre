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
#include "OgreIteratorWrappers.h"
#include "OgreCommon.h"
#include "OgreColourValue.h"
#include "OgreBlendMode.h"
#include "OgreHeaderPrefix.h"
#include "OgreSharedPtr.h"

namespace Ogre {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Materials
    *  @{
    */
    /** Class encapsulates rendering properties of an object.
    @remarks
    Ogre's material class encapsulates ALL aspects of the visual appearance,
    of an object. It also includes other flags which 
    might not be traditionally thought of as material properties such as 
    culling modes and depth buffer settings, but these affect the 
    appearance of the rendered object and are convenient to attach to the 
    material since it keeps all the settings in one place. This is 
    different to Direct3D which treats a material as just the colour 
    components (diffuse, specular) and not texture maps etc. An Ogre 
    Material can be thought of as equivalent to a 'Shader'.
    @par
    A Material can be rendered in multiple different ways depending on the
    hardware available. You may configure a Material to use high-complexity
    fragment shaders, but these won't work on every card; therefore a Technique
    is an approach to creating the visual effect you are looking for. You are advised
    to create fallback techniques with lower hardware requirements if you decide to
    use advanced features. In addition, you also might want lower-detail techniques
    for distant geometry.
    @par
    Each technique can be made up of multiple passes. A fixed-function pass
    may combine multiple texture layers using multitexturing, but Ogre can 
    break that into multiple passes automatically if the active card cannot
    handle that many simultaneous textures. Programmable passes, however, cannot
    be split down automatically, so if the active graphics card cannot handle the
    technique which contains these passes, OGRE will try to find another technique
    which the card can do. If, at the end of the day, the card cannot handle any of the
    techniques which are listed for the material, the engine will render the 
    geometry plain white, which should alert you to the problem.
    @par
    Ogre comes configured with a number of default settings for a newly 
    created material. These can be changed if you wish by retrieving the 
    default material settings through 
    SceneManager::getDefaultMaterialSettings. Any changes you make to the 
    Material returned from this method will apply to any materials created 
    from this point onward.
    */
    class _OgreExport Material : public Resource
    {
        friend class SceneManager;
        friend class MaterialManager;

    public:
        /// distance list used to specify LOD
        typedef FastArray<Real> LodValueArray;
        typedef ConstVectorIterator<LodValueArray> LodValueIterator;
    protected:


        /** Internal method which sets the material up from the default settings.
        */
        void applyDefaults(void);

        typedef vector<Technique*>::type Techniques;
        /// All techniques, supported and unsupported
        Techniques mTechniques;
        /// Supported techniques of any sort
        Techniques mSupportedTechniques;
        typedef map<unsigned short, Technique*>::type LodTechniques;
        typedef map<unsigned short, LodTechniques*>::type BestTechniquesBySchemeList;
        /** Map of scheme -> list of LOD techniques. 
            Current scheme is set on MaterialManager,
            and can be set per Viewport for auto activation.
        */
        BestTechniquesBySchemeList mBestTechniquesBySchemeList;

        LodValueArray mUserLodValues;
        LodValueArray mLodValues;
        bool mReceiveShadows;
        bool mTransparencyCastsShadows;
        /// Does this material require compilation?
        bool mCompilationRequired;
        /// Text description of why any techniques are not supported
        String mUnsupportedReasons;

        /** Insert a supported technique into the local collections. */
        void insertSupportedTechnique(Technique* t);

        /** Clear the best technique list.
        */
        void clearBestTechniqueList(void);

        /** Overridden from Resource.
        */
        void prepareImpl(void);

        /** Overridden from Resource.
        */
        void unprepareImpl(void);

        /** Overridden from Resource.
        */
        void loadImpl(void);

        /** Unloads the material, frees resources etc.
        @see
        Resource
        */
        void unloadImpl(void);
        /// @copydoc Resource::calculateSize
        size_t calculateSize(void) const;
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
        @remarks
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
        @remarks
        This method allows a material to cast a shadow, even if it is transparent.
        By default, transparent materials neither cast nor receive shadows. Shadows
        will not be cast on any objects unless the scene is set up to support shadows 
        (@see SceneManager::setShadowTechnique), and not all techniques cast
        shadows on all objects.
        */
        void setTransparencyCastsShadows(bool enabled) { mTransparencyCastsShadows = enabled; }
        /** Returns whether or not objects using this material be classified as opaque to the shadow caster system. */
        bool getTransparencyCastsShadows(void) const { return mTransparencyCastsShadows; }

        /** Creates a new Technique for this Material.
        @remarks
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
        Technique* getTechnique(unsigned short index);
        /** searches for the named technique.
            Return 0 if technique with name is not found
        */
        Technique* getTechnique(const String& name);
        /** Retrieves the number of techniques. */
        unsigned short getNumTechniques(void) const;
        /** Removes the technique at the given index. */        
        void removeTechnique(unsigned short index);     
        /** Removes all the techniques in this Material. */
        void removeAllTechniques(void);
        typedef VectorIterator<Techniques> TechniqueIterator;
        /** Get an iterator over the Techniques in this Material. */
        TechniqueIterator getTechniqueIterator(void);
        /** Gets an iterator over all the Techniques which are supported by the current card. 
        @remarks
            The supported technique list is only available after this material has been compiled,
            which typically happens on loading the material. Therefore, if this method returns
            an empty list, try calling Material::load.
        */
        TechniqueIterator getSupportedTechniqueIterator(void);
        
        /** Gets the indexed supported technique. */
        Technique* getSupportedTechnique(unsigned short index);
        /** Retrieves the number of supported techniques. */
        unsigned short getNumSupportedTechniques(void) const;
        /** Gets a string explaining why any techniques are not supported. */
        const String& getUnsupportedTechniquesExplanation() const { return mUnsupportedReasons; }

        /** Gets the number of levels-of-detail this material has in the 
            given scheme, based on Technique::setLodIndex. 
        @remarks
            Note that this will not be up to date until the material has been compiled.
        */
        unsigned short getNumLodLevels(unsigned short schemeIndex) const;
        /** Gets the number of levels-of-detail this material has in the 
            given scheme, based on Technique::setLodIndex. 
        @remarks
            Note that this will not be up to date until the material has been compiled.
        */
        unsigned short getNumLodLevels(const String& schemeName) const;

        /** Gets the best supported technique. 
        @remarks
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


        /** Creates a new copy of this material with the same settings but a new name.
        @param newName The name for the cloned material
        @param changeGroup If true, the resource group of the clone is changed
        @param newGroup Only required if changeGroup is true; the new group to assign
        */
        MaterialPtr clone(const String& newName, bool changeGroup = false, 
            const String& newGroup = BLANKSTRING) const;

        /** Copies the details of this material into another, preserving the target's handle and name
        (unlike operator=) but copying everything else.
        @param mat Weak reference to material which will receive this material's settings.
        */
        void copyDetailsTo(MaterialPtr& mat) const;

        /** 'Compiles' this Material.
        @remarks
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

        // -------------------------------------------------------------------------------
        // The following methods are to make migration from previous versions simpler
        // and to make code easier to write when dealing with simple materials
        // They set the properties which have been moved to Pass for all Techniques and all Passes

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
        void setAmbient(Real red, Real green, Real blue);

        /** Sets the ambient colour reflectance properties for every Pass in every Technique.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setAmbient
        */
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
        void setDiffuse(Real red, Real green, Real blue, Real alpha);

        /** Sets the diffuse colour reflectance properties of every Pass in every Technique.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setDiffuse
        */
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
        void setSpecular(Real red, Real green, Real blue, Real alpha);

        /** Sets the specular colour reflectance properties of every Pass in every Technique.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setSpecular
        */
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
        void setSelfIllumination(Real red, Real green, Real blue);

        /** Sets the amount of self-illumination of every Pass in every Technique.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setSelfIllumination
        */
        void setSelfIllumination(const ColourValue& selfIllum);

        /** Sets the macroblock for all passes
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setMacroblock
        */
        void setMacroblock( const HlmsMacroblock &macroblock );

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
            Real expDensity = 0.001, Real linearStart = 0.0, Real linearEnd = 1.0 );

        /** Set samplerblock for every texture unit in every Technique and Pass
        @note
            This property has been moved to the TextureUnitState class, which is accessible via the 
            Technique and Pass. For simplicity, this method allows you to set these properties for 
            every current TeextureUnitState, If you need more precision, retrieve the Technique, 
            Pass and TextureUnitState instances and set the property there.
        @see TextureUnitState::setSamplerblock
        */
        void setSamplerblock( const HlmsSamplerblock &samplerblock );

        /** Sets the blendbock to every pass
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setBlendblock
        */
        void setBlendblock( const HlmsBlendblock &blendblock );

        /** Tells the material that it needs recompilation. */
        void _notifyNeedsRecompile(void);

        /** Sets the distance at which level-of-detail (LOD) levels come into effect.
        @remarks
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
        void setLodLevels(const LodValueArray& lodValues);

        const LodValueArray* _getLodValues(void) const                      { return &mLodValues; }

        /** Gets an iterator over the list of values transformed by the LodStrategy at which each LOD comes into effect. 
        @remarks
            Note that the iterator returned from this method is not totally analogous to 
            the one passed in by calling setLodLevels - the list includes a zero
            entry at the start (since the highest LOD starts at value 0). Also, the
            values returned are after being transformed by LodStrategy::transformUserValue.
        */
        LodValueIterator getLodValueIterator(void) const;

        /** Gets an iterator over the user-defined list of values which are internally transfomed by the LodStrategy. 
        @remarks
            Note that the iterator returned from this method is not totally analogous to 
            the one passed in by calling setLodLevels - the list includes a zero
            entry at the start (since the highest LOD starts at value 0). Also, the
            values returned are after being transformed by LodStrategy::transformUserValue.
        */
        LodValueIterator getUserLodValueIterator(void) const;

        /** @copydoc Resource::touch
        */
        void touch(void) 
        { 
            if (mCompilationRequired) 
                compile();
            // call superclass
            Resource::touch();
        }
        
        /** Applies texture names to Texture Unit State with matching texture name aliases.
            All techniques, passes, and Texture Unit States within the material are checked.
            If matching texture aliases are found then true is returned.

        @param
            aliasList is a map container of texture alias, texture name pairs
        @param
            apply set true to apply the texture aliases else just test to see if texture alias matches are found.
        @return
            True if matching texture aliases were found in the material.
        */
        bool applyTextureAliases(const AliasTextureNamePairList& aliasList, const bool apply = true) const;

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
