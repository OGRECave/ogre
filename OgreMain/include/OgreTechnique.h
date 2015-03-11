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
#ifndef __Technique_H__
#define __Technique_H__

#include "OgrePrerequisites.h"
#include "OgreCommon.h"
#include "OgrePass.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreUserObjectBindings.h"

namespace Ogre {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Materials
    *  @{
    */
    /** Class representing an approach to rendering this particular Material. 
    @remarks
        Ogre will attempt to use the best technique supported by the active hardware, 
        unless you specifically request a lower detail technique (say for distant
        rendering).
    */
    class _OgreExport Technique : public TechniqueAlloc
    {
    protected:
        typedef vector<Pass*>::type Passes;
        /// List of primary passes
        Passes mPasses;
        // Raw pointer since we don't want child to stop parent's destruction
        Material* mParent;
        bool mIsSupported;
        /// LOD level
        unsigned short mLodIndex;
        /** Scheme index, derived from scheme name but the names are held on
            MaterialManager, for speed an index is used here.
        */
        unsigned short mSchemeIndex;
        /// Optional name for the technique
        String mName;

        /** When casting shadow, if not using default Ogre shadow casting material, or 
        * nor using fixed function casting, mShadowCasterMaterial let you customize per material
        * shadow caster behavior
        */
        MaterialPtr mShadowCasterMaterial;
        /** When casting shadow, if not using default Ogre shadow casting material, or 
        * nor using fixed function casting, mShadowCasterMaterial let you customize per material
        * shadow caster behavior.There only material name is stored so that it can be loaded once all file parsed in a resource group.
        */
        String mShadowCasterMaterialName;

        // User objects binding.
        UserObjectBindings  mUserObjectBindings;

    public:
        /** Directive used to manually control technique support based on the
            inclusion or exclusion of some factor.
        */
        enum IncludeOrExclude
        {
            /// Inclusive - only support if present
            INCLUDE = 0,
            /// Exclusive - do not support if present
            EXCLUDE = 1
        };
        /// Rule controlling whether technique is deemed supported based on GPU vendor
        struct GPUVendorRule
        {
            GPUVendor vendor;
            IncludeOrExclude includeOrExclude;
            GPUVendorRule()
                : vendor(GPU_UNKNOWN), includeOrExclude(EXCLUDE) {}
            GPUVendorRule(GPUVendor v, IncludeOrExclude ie)
                : vendor(v), includeOrExclude(ie) {}
        };
        /// Rule controlling whether technique is deemed supported based on GPU device name
        struct GPUDeviceNameRule
        {
            String devicePattern;
            IncludeOrExclude includeOrExclude;
            bool caseSensitive;
            GPUDeviceNameRule()
                : includeOrExclude(EXCLUDE), caseSensitive(false) {}
            GPUDeviceNameRule(const String& pattern, IncludeOrExclude ie, bool caseSen)
                : devicePattern(pattern), includeOrExclude(ie), caseSensitive(caseSen) {}
        };
        typedef vector<GPUVendorRule>::type GPUVendorRuleList;
        typedef vector<GPUDeviceNameRule>::type GPUDeviceNameRuleList;
    protected:
        GPUVendorRuleList mGPUVendorRules;
        GPUDeviceNameRuleList mGPUDeviceNameRules;
    public:
        /// Constructor
        Technique(Material* parent);
        /// Copy constructor
        Technique(Material* parent, const Technique& oth);
        ~Technique();
        /** Indicates if this technique is supported by the current graphics card.
        @remarks
            This will only be correct after the Technique has been compiled, which is
            usually done from Material::compile.
        */
        bool isSupported(void) const;
        /** Internal compilation method; see Material::compile. 
        @return Any information explaining problems with the compile.
        */
        String _compile(bool autoManageTextureUnits);
        /// Internal method for checking GPU vendor / device rules
        bool checkGPURules(StringStream& errors);
        /// Internal method for checking hardware support
        bool checkHardwareSupport(bool autoManageTextureUnits, StringStream& compileErrors);
        size_t calculateSize(void) const;

        /** Creates a new Pass for this Technique.
        @remarks
            A Pass is a single rendering pass, i.e. a single draw of the given material.
            Note that if you create a pass without a fragment program, during compilation of the
            material the pass may be split into multiple passes if the graphics card cannot
            handle the number of texture units requested. For passes with fragment programs, however, 
            the number of passes you create will never be altered, so you have to make sure 
            that you create an alternative fallback Technique for if a card does not have 
            enough facilities for what you're asking for.
        */
        Pass* createPass(void);
        /** Retrieves the Pass with the given index. */
        Pass* getPass(unsigned short index);
        /** Retrieves the Pass matching name.
            Returns 0 if name match is not found.
        */
        Pass* getPass(const String& name);
        /** Retrieves the number of passes. */
        unsigned short getNumPasses(void) const;
        /** Removes the Pass with the given index. */
        void removePass(unsigned short index);
        /** Removes all Passes from this Technique. */
        void removeAllPasses(void);
        /** Move a pass from source index to destination index.
            If successful then returns true.
        */
        bool movePass(const unsigned short sourceIndex, const unsigned short destinationIndex);
        typedef VectorIterator<Passes> PassIterator;
        /** Gets an iterator over the passes in this Technique. */
        const PassIterator getPassIterator(void);
        /// Gets the parent Material
        Material* getParent(void) const { return mParent; }

        /** Overloaded operator to copy on Technique to another. */
        Technique& operator=(const Technique& rhs);

        /// Gets the resource group of the ultimate parent Material
        const String& getResourceGroup(void) const;

        /** Returns true if this Technique involves transparency. 
        @remarks
            This basically boils down to whether the first pass
            has a scene blending factor. Even if the other passes 
            do not, the base colour, including parts of the original 
            scene, may be used for blending, therefore we have to treat
            the whole Technique as transparent.
        */
        bool isTransparent(void) const;

        /** Internal prepare method, derived from call to Material::prepare. */
        void _prepare(void);
        /** Internal unprepare method, derived from call to Material::unprepare. */
        void _unprepare(void);
        /** Internal load method, derived from call to Material::load. */
        void _load(void);
        /** Internal unload method, derived from call to Material::unload. */
        void _unload(void);

        /// Is this loaded?
        bool isLoaded(void) const;

        /** Tells the technique that it needs recompilation. */
        void _notifyNeedsRecompile(void);

        /** return this material specific  shadow casting specific material
        */
        Ogre::MaterialPtr getShadowCasterMaterial() const;
        /** set this material specific  shadow casting specific material
        */
        void setShadowCasterMaterial(Ogre::MaterialPtr val);
        /** set this material specific  shadow casting specific material
        */
        void setShadowCasterMaterial(const Ogre::String &name);

        // -------------------------------------------------------------------------------
        // The following methods are to make migration from previous versions simpler
        // and to make code easier to write when dealing with simple materials
        // They set the properties which have been moved to Pass for all Techniques and all Passes

        /** Sets the point size properties for every Pass in this Technique.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setPointSize
        */
        void setPointSize(Real ps);

        /** Sets the ambient colour reflectance properties for every Pass in every Technique.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setAmbient
        */
        void setAmbient(Real red, Real green, Real blue);

        /** Sets the ambient colour reflectance properties for every Pass in every Technique.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setAmbient
        */
        void setAmbient(const ColourValue& ambient);

        /** Sets the diffuse colour reflectance properties of every Pass in every Technique.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setDiffuse
        */
        void setDiffuse(Real red, Real green, Real blue, Real alpha);

        /** Sets the diffuse colour reflectance properties of every Pass in every Technique.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setDiffuse
        */
        void setDiffuse(const ColourValue& diffuse);

        /** Sets the specular colour reflectance properties of every Pass in every Technique.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setSpecular
        */
        void setSpecular(Real red, Real green, Real blue, Real alpha);

        /** Sets the specular colour reflectance properties of every Pass in every Technique.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setSpecular
        */
        void setSpecular(const ColourValue& specular);

        /** Sets the shininess properties of every Pass in every Technique.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setShininess
        */
        void setShininess(Real val);

        /** Sets the amount of self-illumination of every Pass in every Technique.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setSelfIllumination
        */
        void setSelfIllumination(Real red, Real green, Real blue);

        /** Sets the amount of self-illumination of every Pass in every Technique.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setSelfIllumination
        */
        void setSelfIllumination(const ColourValue& selfIllum);

        /** Sets the type of light shading required
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setShadingMode
        */
        void setShadingMode( ShadeOptions mode );

        /** Sets the fogging mode applied to each pass.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setFog
        */
        void setFog(
            bool overrideScene,
            FogMode mode = FOG_NONE,
            const ColourValue& colour = ColourValue::White,
            Real expDensity = 0.001, Real linearStart = 0.0, Real linearEnd = 1.0 );

        /** Set samplerblock for every texture unit in every Pass
        @note
            This property actually exists on the TextureUnitState class
            For simplicity, this method allows you to set these properties for 
            every current TeextureUnitState, If you need more precision, retrieve the  
            Pass and TextureUnitState instances and set the property there.
        @see TextureUnitState::setSamplerblock
        */
        void setSamplerblock( const HlmsSamplerblock &samplerblock );

        /** Sets the macroblock every pass has with the existing contents of the scene.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows
            you to set these properties for every current Pass within this Technique. If
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setBlendblock
        */
        void setMacroblock( const HlmsMacroblock &macroblock );

        /** Sets the blendblock every pass has with the existing contents of the scene.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setBlendblock
        */
        void setBlendblock( const HlmsBlendblock &blendblock );

        /** Assigns a level-of-detail (LOD) index to this Technique.
        @remarks
            As noted previously, as well as providing fallback support for various
            graphics cards, multiple Technique objects can also be used to implement
            material LOD, where the detail of the material diminishes with distance to 
            save rendering power.
        @par
            By default, all Techniques have a LOD index of 0, which means they are the highest
            level of detail. Increasing LOD indexes are lower levels of detail. You can 
            assign more than one Technique to the same LOD index, meaning that the best 
            Technique that is supported at that LOD index is used. 
        @par
            You should not leave gaps in the LOD sequence; Ogre will allow you to do this
            and will continue to function as if the LODs were sequential, but it will 
            confuse matters.
        */
        void setLodIndex(unsigned short index);
        /** Gets the level-of-detail index assigned to this Technique. */
        unsigned short getLodIndex(void) const { return mLodIndex; }

        /** Set the 'scheme name' for this technique. 
        @remarks
            Material schemes are used to control top-level switching from one
            set of techniques to another. For example, you might use this to 
            define 'high', 'medium' and 'low' complexity levels on materials
            to allow a user to pick a performance / quality ratio. Another
            possibility is that you have a fully HDR-enabled pipeline for top
            machines, rendering all objects using unclamped shaders, and a 
            simpler pipeline for others; this can be implemented using 
            schemes.
        @par
            Every technique belongs to a scheme - if you don't specify one, the
            Technique belongs to the scheme called 'Default', which is also the
            scheme used to render by default. The active scheme is set one of
            two ways - either by calling Viewport::setMaterialScheme, or
            by manually calling MaterialManager::setActiveScheme.
        */
        void setSchemeName(const String& schemeName);
        /** Returns the scheme to which this technique is assigned.
            @see Technique::setSchemeName
        */
        const String& getSchemeName(void) const;
        
        /// Internal method for getting the scheme index
        unsigned short _getSchemeIndex(void) const;
            
        /** Is depth writing going to occur on this technique? */
        bool isDepthWriteEnabled(void) const;

        /** Is depth checking going to occur on this technique? */
        bool isDepthCheckEnabled(void) const;

        /** Exists colour writing disabled pass on this technique? */
        bool hasColourWriteDisabled(void) const;

        /** Set the name of the technique.
        @remarks
        The use of technique name is optional.  Its useful in material scripts where a material could inherit
        from another material and only want to modify a particular technique.
        */
        void setName(const String& name);
        /// Gets the name of the technique
        const String& getName(void) const { return mName; }

        /** Applies texture names to Texture Unit State with matching texture name aliases.
            All passes, and Texture Unit States within the technique are checked.
            If matching texture aliases are found then true is returned.

        @param
            aliasList is a map container of texture alias, texture name pairs
        @param
            apply set true to apply the texture aliases else just test to see if texture alias matches are found.
        @return
            True if matching texture aliases were found in the Technique.
        */
        bool applyTextureAliases(const AliasTextureNamePairList& aliasList, const bool apply = true) const;


        /** Add a rule which manually influences the support for this technique based
            on a GPU vendor.
        @remarks
            You can use this facility to manually control whether a technique is
            considered supported, based on a GPU vendor. You can add inclusive
            or exclusive rules, and you can add as many of each as you like. If
            at least one inclusive rule is added, a technique is considered 
            unsupported if it does not match any of those inclusive rules. If exclusive rules are
            added, the technique is considered unsupported if it matches any of
            those inclusive rules. 
        @note
            Any rule for the same vendor will be removed before adding this one.
        @param vendor The GPU vendor
        @param includeOrExclude Whether this is an inclusive or exclusive rule
        */
        void addGPUVendorRule(GPUVendor vendor, IncludeOrExclude includeOrExclude);
        /** Add a rule which manually influences the support for this technique based
            on a GPU vendor.
        @remarks
            You can use this facility to manually control whether a technique is
            considered supported, based on a GPU vendor. You can add inclusive
            or exclusive rules, and you can add as many of each as you like. If
            at least one inclusive rule is added, a technique is considered 
            unsupported if it does not match any of those inclusive rules. If exclusive rules are
            added, the technique is considered unsupported if it matches any of
            those inclusive rules. 
        @note
            Any rule for the same vendor will be removed before adding this one.
        */
        void addGPUVendorRule(const GPUVendorRule& rule);
        /** Removes a matching vendor rule.
        @see addGPUVendorRule
        */
        void removeGPUVendorRule(GPUVendor vendor);
        typedef ConstVectorIterator<GPUVendorRuleList> GPUVendorRuleIterator;
        /// Get an iterator over the currently registered vendor rules.
        GPUVendorRuleIterator getGPUVendorRuleIterator() const;

        /** Add a rule which manually influences the support for this technique based
            on a pattern that matches a GPU device name (e.g. '*8800*').
        @remarks
            You can use this facility to manually control whether a technique is
            considered supported, based on a GPU device name pattern. You can add inclusive
            or exclusive rules, and you can add as many of each as you like. If
            at least one inclusive rule is added, a technique is considered 
            unsupported if it does not match any of those inclusive rules. If exclusive rules are
            added, the technique is considered unsupported if it matches any of
            those inclusive rules. The pattern you supply can include wildcard
            characters ('*') if you only want to match part of the device name.
        @note
            Any rule for the same device pattern will be removed before adding this one.
        @param devicePattern The GPU vendor
        @param includeOrExclude Whether this is an inclusive or exclusive rule
        @param caseSensitive Whether the match is case sensitive or not
        */
        void addGPUDeviceNameRule(const String& devicePattern, IncludeOrExclude includeOrExclude, bool caseSensitive = false);
        /** Add a rule which manually influences the support for this technique based
            on a pattern that matches a GPU device name (e.g. '*8800*').
        @remarks
            You can use this facility to manually control whether a technique is
            considered supported, based on a GPU device name pattern. You can add inclusive
            or exclusive rules, and you can add as many of each as you like. If
            at least one inclusive rule is added, a technique is considered 
            unsupported if it does not match any of those inclusive rules. If exclusive rules are
            added, the technique is considered unsupported if it matches any of
            those inclusive rules. The pattern you supply can include wildcard
            characters ('*') if you only want to match part of the device name.
        @note
            Any rule for the same device pattern will be removed before adding this one.
        */
        void addGPUDeviceNameRule(const GPUDeviceNameRule& rule);
        /** Removes a matching device name rule.
        @see addGPUDeviceNameRule
        */
        void removeGPUDeviceNameRule(const String& devicePattern);
        typedef ConstVectorIterator<GPUDeviceNameRuleList> GPUDeviceNameRuleIterator;
        /// Get an iterator over the currently registered device name rules.
        GPUDeviceNameRuleIterator getGPUDeviceNameRuleIterator() const;

        /** Return an instance of user objects binding associated with this class.
        You can use it to associate one or more custom objects with this class instance.
        @see UserObjectBindings::setUserAny.
        */
        UserObjectBindings& getUserObjectBindings() { return mUserObjectBindings; }

        /** Return an instance of user objects binding associated with this class.
        You can use it to associate one or more custom objects with this class instance.
        @see UserObjectBindings::setUserAny.        
        */
        const UserObjectBindings& getUserObjectBindings() const { return mUserObjectBindings; }

    };

    /** @} */
    /** @} */

}
#endif
