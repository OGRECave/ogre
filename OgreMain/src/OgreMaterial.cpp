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
#include "OgreStableHeaders.h"

#include "OgreMaterial.h"
#include "OgreLodStrategyManager.h"
#include "OgreLodStrategy.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    Material::Material(ResourceManager* creator, const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader)
        :Resource(creator, name, handle, group, false, NULL),
         mReceiveShadows(true),
         mTransparencyCastsShadows(false),
         mCompilationRequired(true)
    {
        // Override isManual, not applicable for Material (we always want to call loadImpl)
        if(isManual)
        {
            LogManager::getSingleton().logWarning("Material " + name +
                " was requested with isManual=true, but this is not applicable " 
                "for materials; the flag has been reset to false");
        }

        // Initialise to default strategy
        mLodStrategy = LodStrategyManager::getSingleton().getDefaultStrategy();

        mLodValues.push_back(0.0f);

        applyDefaults();

        /* For consistency with StringInterface, but we don't add any parameters here
        That's because the Resource implementation of StringInterface is to
        list all the options that need to be set before loading, of which 
        we have none as such. Full details can be set through scripts.
        */ 
        createParamDictionary("Material");
    }
    //-----------------------------------------------------------------------
    Material::~Material()
    {
        removeAllTechniques();
        // have to call this here reather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        unload(); 
    }
    //-----------------------------------------------------------------------
    Material& Material::operator=(const Material& rhs)
    {
        if (this == &rhs)
            return *this;

        Resource::operator=(rhs);
        mReceiveShadows = rhs.mReceiveShadows;
        mTransparencyCastsShadows = rhs.mTransparencyCastsShadows;

        // Copy Techniques
        this->removeAllTechniques();
        for(auto *t : rhs.mTechniques)
        {
            Technique* tech = this->createTechnique();
            *tech = *t;
            if (t->isSupported())
            {
                insertSupportedTechnique(tech);
            }
        }

        // Also copy LOD information
        mUserLodValues = rhs.mUserLodValues;
        mLodValues = rhs.mLodValues;
        mLodStrategy = rhs.mLodStrategy;
        mCompilationRequired = rhs.mCompilationRequired;
        // illumination passes are not compiled right away so
        // mIsLoaded state should still be the same as the original material
        assert(isLoaded() == rhs.isLoaded());

        return *this;
    }


    //-----------------------------------------------------------------------
    void Material::prepareImpl(void)
    {
        // compile if required
        if (mCompilationRequired)
            compile();

        // Load all supported techniques
        for (auto *t : mSupportedTechniques)
        {
            t->_prepare();
        }
    }
    //-----------------------------------------------------------------------
    void Material::unprepareImpl(void)
    {
        // Load all supported techniques
        for (auto *t : mSupportedTechniques)
        {
            t->_unprepare();
        }
    }
    //-----------------------------------------------------------------------
    void Material::loadImpl(void)
    {
        // Load all supported techniques
        for (auto *t : mSupportedTechniques)
        {
            t->_load();
        }
    }
    //-----------------------------------------------------------------------
    void Material::unloadImpl(void)
    {
        // Unload all supported techniques
        for (auto *t : mSupportedTechniques)
        {
            t->_unload();
        }

        mCompilationRequired = true;
    }
    //-----------------------------------------------------------------------
    size_t Material::calculateSize(void) const
    {
        size_t memSize = sizeof(*this) + Resource::calculateSize();

        // Tally up techniques
        for (auto t : mTechniques)
        {
            memSize += t->calculateSize();
        }

        memSize += mUnsupportedReasons.size() * sizeof(char);

        return memSize;
    }
    //-----------------------------------------------------------------------
    MaterialPtr Material::clone(const String& newName, const String& newGroup) const
    {
        MaterialPtr newMat =
            MaterialManager::getSingleton().create(newName, newGroup.empty() ? mGroup : newGroup);

        if(!newMat) // interception by collision handler
            return newMat;

        // Keep handle (see below, copy overrides everything)
        ResourceHandle newHandle = newMat->getHandle();
        // Assign values from this
        *newMat = *this;
        // Restore new group if required, will have been overridden by operator
        if (!newGroup.empty())
        {
            newMat->mGroup = newGroup;
        }
        
        // Correct the name & handle, they get copied too
        newMat->mName = newName;
        newMat->mHandle = newHandle;

        //if we're cloning from a loaded material, notify the creator or otherwise size won't be right
        if (newMat->getLoadingState() == LOADSTATE_LOADED)
        {
            // Notify manager
            if (mCreator)
                mCreator->_notifyResourceLoaded(newMat.get());
        }

        return newMat;
    }
    //-----------------------------------------------------------------------
    void Material::copyDetailsTo(MaterialPtr& mat) const
    {
        // Keep handle (see below, copy overrides everything)
        ResourceHandle savedHandle = mat->mHandle;
        String savedName = mat->mName;
        String savedGroup = mat->mGroup;
        // Assign values from this
        *mat = *this;
        // Correct the name & handle, they get copied too
        mat->mName = savedName;
        mat->mHandle = savedHandle;
        mat->mGroup = savedGroup;
    }
    //-----------------------------------------------------------------------
    void Material::applyDefaults(void)
    {
        MaterialPtr defaults = MaterialManager::getSingleton().getDefaultSettings();

        if (defaults)
        {
            // save name & handle
            String savedName = mName;
            String savedGroup = mGroup;
            ResourceHandle savedHandle = mHandle;
            *this = *defaults;
            // restore name & handle
            mName = savedName;
            mHandle = savedHandle;
            mGroup = savedGroup;
        }
        mCompilationRequired = true;

    }
    //-----------------------------------------------------------------------
    Technique* Material::createTechnique(void)
    {
        Technique *t = OGRE_NEW Technique(this);
        mTechniques.push_back(t);
        mCompilationRequired = true;
        return t;
    }
    //-----------------------------------------------------------------------
    Technique* Material::getTechnique(const String& name) const
    {
        Technique* foundTechnique = 0;
        // iterate through techniques to find a match
        for (auto *t : mTechniques)
        {
            if (t->getName() == name)
            {
                foundTechnique = t;
                break;
            }
        }

        return foundTechnique;
    }
    //-----------------------------------------------------------------------
    unsigned short Material::getNumLodLevels(unsigned short schemeIndex) const
    {
        // Safety check - empty list?
        if (mBestTechniquesBySchemeList.empty())
            return 0;

        BestTechniquesBySchemeList::const_iterator i = 
            mBestTechniquesBySchemeList.find(schemeIndex);
        if (i == mBestTechniquesBySchemeList.end())
        {
            // get the first item, will be 0 (the default) if default
            // scheme techniques exist, otherwise the earliest defined
            i = mBestTechniquesBySchemeList.begin();
        }

        return static_cast<unsigned short>(i->second.size());
    }
    //-----------------------------------------------------------------------
    unsigned short Material::getNumLodLevels(const String& schemeName) const
    {
        return getNumLodLevels(
            MaterialManager::getSingleton()._getSchemeIndex(schemeName));
    }
    //-----------------------------------------------------------------------
    void Material::insertSupportedTechnique(Technique* t)
    {
        mSupportedTechniques.push_back(t);
        // get scheme
        unsigned short schemeIndex = t->_getSchemeIndex();

        // Insert won't replace if supported technique for this scheme/lod is
        // already there, which is what we want
        mBestTechniquesBySchemeList[schemeIndex].emplace(t->getLodIndex(), t);

    }
    //-----------------------------------------------------------------------------
    Technique* Material::getBestTechnique(unsigned short lodIndex, const Renderable* rend)
    {
        if (mSupportedTechniques.empty())
        {
            return NULL;
        }
        else
        {
            Technique* ret = 0;
            MaterialManager& matMgr = MaterialManager::getSingleton();
            // get scheme
            auto si = mBestTechniquesBySchemeList.find(matMgr._getActiveSchemeIndex());
            // scheme not found?
            if (si == mBestTechniquesBySchemeList.end())
            {
                // listener specified alternative technique available?
                ret = matMgr._arbitrateMissingTechniqueForActiveScheme(this, lodIndex, rend);
                if (ret)
                    return ret;

                OgreAssert(!mBestTechniquesBySchemeList.empty(), "handleSchemeNotFound() must not remove techniques");
                // Nope, use default
                // get the first item, will be 0 (the default) if default
                // scheme techniques exist, otherwise the earliest defined
                si = mBestTechniquesBySchemeList.begin();
            }

            // get LOD
            auto li = si->second.find(lodIndex);
            // LOD not found? 
            if (li == si->second.end())
            {
                // Use the next LOD level up
                for (auto rli = si->second.rbegin(); rli != si->second.rend(); ++rli)
                {
                    if (rli->second->getLodIndex() < lodIndex)
                    {
                        ret = rli->second;
                        break;
                    }

                }
                if (!ret)
                {
                    // shouldn't ever hit this really, unless user defines no LOD 0
                    // pick the first LOD we have (must be at least one to have a scheme entry)
                    ret = si->second.begin()->second;
                }

            }
            else
            {
                // LOD found
                ret = li->second;
            }

            return ret;

        }
    }
    //-----------------------------------------------------------------------
    void Material::removeTechnique(unsigned short index)
    {
        assert (index < mTechniques.size() && "Index out of bounds.");
        Techniques::iterator i = mTechniques.begin() + index;
        OGRE_DELETE(*i);
        mTechniques.erase(i);
        clearBestTechniqueList();
    }
    //-----------------------------------------------------------------------
    void Material::removeAllTechniques(void)
    {
        for (auto *t : mTechniques)
        {
            OGRE_DELETE t;
        }
        mTechniques.clear();
        clearBestTechniqueList();
    }
    //-----------------------------------------------------------------------
    Material::TechniqueIterator Material::getTechniqueIterator(void) 
    {
        return TechniqueIterator(mTechniques.begin(), mTechniques.end());
    }
    //-----------------------------------------------------------------------
    Material::TechniqueIterator Material::getSupportedTechniqueIterator(void)
    {
        return TechniqueIterator(mSupportedTechniques.begin(), mSupportedTechniques.end());
    }
    //-----------------------------------------------------------------------
    bool Material::isTransparent(void) const
    {
        // Check each technique
        for (auto *t : mTechniques)
        {
            if (t->isTransparent())
                return true;
        }
        return false;
    }
    //-----------------------------------------------------------------------
    void Material::compile(bool autoManageTextureUnits)
    {
        // Compile each technique, then add it to the list of supported techniques
        clearBestTechniqueList();
        mUnsupportedReasons.clear();

        size_t techNo = 0;
        for (auto *t : mTechniques)
        {
            String compileMessages = t->_compile(autoManageTextureUnits);
            if (t->isSupported())
            {
                insertSupportedTechnique(t);
            }
            else
            {
                // Log informational
                StringStream str;
                str << "Material " << mName << " Technique " << techNo;
                if (!t->getName().empty())
                    str << "(" << t->getName() << ")";
                str << " is not supported. " << compileMessages;
                LogManager::getSingleton().logMessage(str.str(), LML_TRIVIAL);
                mUnsupportedReasons += compileMessages;
            }
            ++techNo;
        }

        mCompilationRequired = false;

        // Did we find any?
        if (mSupportedTechniques.empty())
        {
            LogManager::getSingleton().stream(LML_WARNING)
                << "Warning: material " << mName << " has no supportable "
                << "Techniques and will be blank. Explanation: \n" << mUnsupportedReasons;
        }
    }
    //-----------------------------------------------------------------------
    void Material::clearBestTechniqueList(void)
    {
        mSupportedTechniques.clear();
        mBestTechniquesBySchemeList.clear();
        mCompilationRequired = true;
    }
    //-----------------------------------------------------------------------
    #define ALL_TECHNIQUES(fncall) for(auto t : mTechniques) t->fncall
    void Material::setPointSize(Real ps) { ALL_TECHNIQUES(setPointSize(ps)); }
    //-----------------------------------------------------------------------
    void Material::setAmbient(float red, float green, float blue) { setAmbient(ColourValue(red, green, blue)); }
    //-----------------------------------------------------------------------
    void Material::setAmbient(const ColourValue& ambient) { ALL_TECHNIQUES(setAmbient(ambient)); }
    //-----------------------------------------------------------------------
    void Material::setDiffuse(float red, float green, float blue, float alpha)
    {
        ALL_TECHNIQUES(setDiffuse(red, green, blue, alpha));
    }
    //-----------------------------------------------------------------------
    void Material::setDiffuse(const ColourValue& diffuse) { setDiffuse(diffuse.r, diffuse.g, diffuse.b, diffuse.a); }
    //-----------------------------------------------------------------------
    void Material::setSpecular(float red, float green, float blue, float alpha)
    {
        ALL_TECHNIQUES(setSpecular(red, green, blue, alpha));
    }
    //-----------------------------------------------------------------------
    void Material::setSpecular(const ColourValue& specular)
    {
        setSpecular(specular.r, specular.g, specular.b, specular.a);
    }
    //-----------------------------------------------------------------------
    void Material::setShininess(Real val) { ALL_TECHNIQUES(setShininess(val)); }
    //-----------------------------------------------------------------------
    void Material::setSelfIllumination(float red, float green, float blue)
    {
        setSelfIllumination(ColourValue(red, green, blue));
    }
    //-----------------------------------------------------------------------
    void Material::setSelfIllumination(const ColourValue& selfIllum) { ALL_TECHNIQUES(setSelfIllumination(selfIllum)); }
    //-----------------------------------------------------------------------
    void Material::setDepthCheckEnabled(bool enabled) { ALL_TECHNIQUES(setDepthCheckEnabled(enabled)); }
    //-----------------------------------------------------------------------
    void Material::setDepthWriteEnabled(bool enabled) { ALL_TECHNIQUES(setDepthWriteEnabled(enabled)); }
    //-----------------------------------------------------------------------
    void Material::setDepthFunction(CompareFunction func) { ALL_TECHNIQUES(setDepthFunction(func)); }
    //-----------------------------------------------------------------------
    void Material::setColourWriteEnabled(bool enabled) { ALL_TECHNIQUES(setColourWriteEnabled(enabled)); }
    //-----------------------------------------------------------------------
    void Material::setColourWriteEnabled(bool red, bool green, bool blue, bool alpha)
    {
        ALL_TECHNIQUES(setColourWriteEnabled(red, green, blue, alpha));
    }
    //-----------------------------------------------------------------------
    void Material::setCullingMode(CullingMode mode) { ALL_TECHNIQUES(setCullingMode(mode)); }
    //-----------------------------------------------------------------------
    void Material::setManualCullingMode(ManualCullingMode mode) { ALL_TECHNIQUES(setManualCullingMode(mode)); }
    //-----------------------------------------------------------------------
    void Material::setLightingEnabled(bool enabled) { ALL_TECHNIQUES(setLightingEnabled(enabled)); }
    //-----------------------------------------------------------------------
    void Material::setShadingMode(ShadeOptions mode) { ALL_TECHNIQUES(setShadingMode(mode)); }
    //-----------------------------------------------------------------------
    void Material::setFog(bool overrideScene, FogMode mode, const ColourValue& colour, Real expDensity,
                          Real linearStart, Real linearEnd)
    {
        ALL_TECHNIQUES(setFog(overrideScene, mode, colour, expDensity, linearStart, linearEnd));
    }
    //-----------------------------------------------------------------------
    void Material::setDepthBias(float constantBias, float slopeScaleBias)
    {
        ALL_TECHNIQUES(setDepthBias(constantBias, slopeScaleBias));
    }
    //-----------------------------------------------------------------------
    void Material::setTextureFiltering(TextureFilterOptions filterType)
    {
        ALL_TECHNIQUES(setTextureFiltering(filterType));
    }
    // --------------------------------------------------------------------
    void Material::setTextureAnisotropy(int maxAniso) { ALL_TECHNIQUES(setTextureAnisotropy(maxAniso)); }
    // --------------------------------------------------------------------
    void Material::setSceneBlending(const SceneBlendType sbt) { ALL_TECHNIQUES(setSceneBlending(sbt)); }
    // --------------------------------------------------------------------
    void Material::setSeparateSceneBlending(const SceneBlendType sbt, const SceneBlendType sbta)
    {
        ALL_TECHNIQUES(setSeparateSceneBlending(sbt, sbta));
    }
    // --------------------------------------------------------------------
    void Material::setSceneBlending(const SceneBlendFactor sourceFactor, const SceneBlendFactor destFactor)
    {
        ALL_TECHNIQUES(setSceneBlending(sourceFactor, destFactor));
    }
    // --------------------------------------------------------------------
    void Material::setSeparateSceneBlending( const SceneBlendFactor sourceFactor, const SceneBlendFactor destFactor, const SceneBlendFactor sourceFactorAlpha, const SceneBlendFactor destFactorAlpha)
    {
        ALL_TECHNIQUES(setSeparateSceneBlending(sourceFactor, destFactor, sourceFactorAlpha, destFactorAlpha));
    }
    #undef ALL_TECHNIQUES
    // --------------------------------------------------------------------
    void Material::_notifyNeedsRecompile(void)
    {
        mCompilationRequired = true;
        // Also need to unload to ensure we loaded any new items
        if (isLoaded()) // needed to stop this being called in 'loading' state
            unload();
    }
    // --------------------------------------------------------------------
    void Material::setLodLevels(const LodValueList& lodValues)
    {
        // Square the distances for the internal list
        // First, clear and add single zero entry
        mLodValues.clear();
        mUserLodValues.clear();
        mUserLodValues.push_back(0);
        if (mLodStrategy)
            mLodValues.push_back(mLodStrategy->getBaseValue());
        for (auto& v : lodValues)
        {
            mUserLodValues.push_back(v);
            if (mLodStrategy)
                mLodValues.push_back(mLodStrategy->transformUserValue(v));
        }
    }
    // --------------------------------------------------------------------
    ushort Material::getLodIndex(Real value) const
    {
        return mLodStrategy->getIndex(value, mLodValues);
    }
    // --------------------------------------------------------------------
    Material::LodValueIterator Material::getLodValueIterator(void) const
    {
        return LodValueIterator(mLodValues.begin(), mLodValues.end());
    }
    // --------------------------------------------------------------------
    Material::LodValueIterator Material::getUserLodValueIterator(void) const
    {
        return LodValueIterator(mUserLodValues.begin(), mUserLodValues.end());
    }
    //---------------------------------------------------------------------
    const LodStrategy *Material::getLodStrategy() const
    {
        return mLodStrategy;
    }
    //---------------------------------------------------------------------
    void Material::setLodStrategy(LodStrategy *lodStrategy)
    {
        mLodStrategy = lodStrategy;

        assert(mLodValues.size());
        mLodValues[0] = mLodStrategy->getBaseValue();

        // Re-transform all user LOD values (starting at index 1, no need to transform base value)
        for (size_t i = 1; i < mUserLodValues.size(); ++i)
            mLodValues[i] = mLodStrategy->transformUserValue(mUserLodValues[i]);
    }
    //---------------------------------------------------------------------
}
