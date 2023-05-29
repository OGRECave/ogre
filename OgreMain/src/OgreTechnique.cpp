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


namespace Ogre {
    //-----------------------------------------------------------------------------
    Technique::Technique(Material* parent)
        : mParent(parent), mIlluminationPassesCompilationPhase(IPS_NOT_COMPILED), mIsSupported(false), mLodIndex(0), mSchemeIndex(0)
    {
        // See above, defaults to unsupported until examined
    }
    //-----------------------------------------------------------------------------
    Technique::Technique(Material* parent, const Technique& oth)
        : mParent(parent), mLodIndex(0), mSchemeIndex(0)
    {
        // Copy using operator=
        *this = oth;
    }
    //-----------------------------------------------------------------------------
    Technique::~Technique()
    {
        removeAllPasses();
        clearIlluminationPasses();
    }
    //-----------------------------------------------------------------------------
    bool Technique::isSupported(void) const
    {
        return mIsSupported;
    }
    //-----------------------------------------------------------------------------
    size_t Technique::calculateSize(void) const
    {
        size_t memSize = 0;

        // Tally up passes
        for (auto *p : mPasses)
        {
            memSize += p->calculateSize();
        }
        return memSize;
    }
    //-----------------------------------------------------------------------------
    String Technique::_compile(bool autoManageTextureUnits)
    {
        StringStream errors;

        if(!Root::getSingleton().getRenderSystem()) {
            errors << "NULL RenderSystem";
        } else {
            mIsSupported = checkGPURules(errors) && checkHardwareSupport(autoManageTextureUnits, errors);
        }

        // Compile for categorised illumination on demand
        clearIlluminationPasses();
        mIlluminationPassesCompilationPhase = IPS_NOT_COMPILED;

        return errors.str();

    }
    //---------------------------------------------------------------------
    bool Technique::checkHardwareSupport(bool autoManageTextureUnits, StringStream& compileErrors)
    {
        // Go through each pass, checking requirements
        Passes::iterator i;
        unsigned short passNum = 0;
        const RenderSystemCapabilities* caps =
            Root::getSingleton().getRenderSystem()->getCapabilities();
        unsigned short numTexUnits = caps->getNumTextureUnits();
        for (i = mPasses.begin(); i != mPasses.end(); ++i, ++passNum)
        {
            Pass* currPass = *i;
            // Adjust pass index
            currPass->_notifyIndex(passNum);

            // Check texture unit requirements
            size_t numTexUnitsRequested = currPass->getNumTextureUnitStates();
            // Don't trust getNumTextureUnits for programmable
            if(!currPass->hasFragmentProgram())
            {
#if defined(OGRE_PRETEND_TEXTURE_UNITS) && OGRE_PRETEND_TEXTURE_UNITS > 0
                numTexUnits = std::min(numTexUnits, OGRE_PRETEND_TEXTURE_UNITS);
#endif
                if (numTexUnitsRequested > numTexUnits)
                {
                    if (!autoManageTextureUnits)
                    {
                        // The user disabled auto pass split
                        compileErrors << "Pass " << passNum <<
                            ": Too many texture units for the current hardware and no splitting allowed."
                            << std::endl;
                        return false;
                    }
                    else if (currPass->hasVertexProgram())
                    {
                        // Can't do this one, and can't split a programmable pass
                        compileErrors << "Pass " << passNum <<
                            ": Too many texture units for the current hardware and "
                            "cannot split programmable passes."
                            << std::endl;
                        return false;
                    }
                }

                // Check a few fixed-function options in texture layers
                size_t texUnit = 0;
                for(const TextureUnitState* tex : currPass->getTextureUnitStates())
                {
                    const char* err = 0;
                    if ((tex->getTextureType() == TEX_TYPE_3D) && !caps->hasCapability(RSC_TEXTURE_3D))
                        err = "Volume";

                    if ((tex->getTextureType() == TEX_TYPE_2D_ARRAY) && !caps->hasCapability(RSC_TEXTURE_2D_ARRAY))
                        err = "Array";

                    if (err)
                    {
                        // Fail
                        compileErrors << "Pass " << passNum << " Tex " << texUnit << ": " << err
                                      << " textures not supported by RenderSystem";
                        return false;
                    }
                    ++texUnit;
                }

                // We're ok on operations, now we need to check # texture units

                    // Keep splitting this pass so long as units requested > gpu units
                while (numTexUnitsRequested > numTexUnits)
                {
                    // chop this pass into many passes
                    currPass = currPass->_split(numTexUnits);
                    numTexUnitsRequested = currPass->getNumTextureUnitStates();
                    // Advance pass number
                    ++passNum;
                    // Reset iterator
                    i = mPasses.begin() + passNum;
                    // Move the new pass to the right place (will have been created
                    // at the end, may be other passes in between)
                    assert(mPasses.back() == currPass);
                    std::copy_backward(i, (mPasses.end() - 1), mPasses.end());
                    *i = currPass;
                    // Adjust pass index
                    currPass->_notifyIndex(passNum);
                }
            }



            //Check compilation errors for all program types.
            for (int t = 0; t < 6; t++)
            {
                GpuProgramType programType = GpuProgramType(t);
                if (currPass->hasGpuProgram(programType))
                {
                    GpuProgramPtr program = currPass->getGpuProgram(programType);
                    if (!program->isSupported())
                    {
                        compileErrors << "Pass " << passNum <<
                            ": " << GpuProgram::getProgramTypeName(programType) + " program " << program->getName()
                            << " cannot be used - ";
                        if (program->hasCompileError() && program->getSource().empty())
                            compileErrors << "resource not found.";
                        else if (program->hasCompileError())
                            compileErrors << "compile error.";
                        else
                            compileErrors << "not supported.";

                        compileErrors << std::endl;
                        return false;
                    }
                }
            }
        }
        // If we got this far, we're ok
        return true;
    }
    //---------------------------------------------------------------------
    bool Technique::checkGPURules(StringStream& errors)
    {
        const RenderSystemCapabilities* caps =
            Root::getSingleton().getRenderSystem()->getCapabilities();

        StringStream includeRules;
        bool includeRulesPresent = false;
        bool includeRuleMatched = false;

        // Check vendors first
        for (auto& r : mGPUVendorRules)
        {
            if (r.includeOrExclude == INCLUDE)
            {
                includeRulesPresent = true;
                includeRules << caps->vendorToString(r.vendor) << " ";
                if (r.vendor == caps->getVendor())
                    includeRuleMatched = true;
            }
            else // EXCLUDE
            {
                if (r.vendor == caps->getVendor())
                {
                    errors << "Excluded GPU vendor: " << caps->vendorToString(r.vendor)
                        << std::endl;
                    return false;
                }

            }
        }

        if (includeRulesPresent && !includeRuleMatched)
        {
            errors << "Failed to match GPU vendor: " << includeRules.str( )
                << std::endl;
            return false;
        }

        // now check device names
        includeRules.str(BLANKSTRING);
        includeRulesPresent = false;
        includeRuleMatched = false;

        for (auto& r : mGPUDeviceNameRules)
        {
            if (r.includeOrExclude == INCLUDE)
            {
                includeRulesPresent = true;
                includeRules << r.devicePattern << " ";
                if (StringUtil::match(caps->getDeviceName(), r.devicePattern, r.caseSensitive))
                    includeRuleMatched = true;
            }
            else // EXCLUDE
            {
                if (StringUtil::match(caps->getDeviceName(), r.devicePattern, r.caseSensitive))
                {
                    errors << "Excluded GPU device: " << r.devicePattern
                        << std::endl;
                    return false;
                }

            }
        }

        if (includeRulesPresent && !includeRuleMatched)
        {
            errors << "Failed to match GPU device: " << includeRules.str( )
                << std::endl;
            return false;
        }

        // passed
        return true;
    }
    //-----------------------------------------------------------------------------
    Pass* Technique::createPass(void)
    {
        Pass* newPass = OGRE_NEW Pass(this, static_cast<unsigned short>(mPasses.size()));
        mPasses.push_back(newPass);
        return newPass;
    }
    //-----------------------------------------------------------------------------
    Pass* Technique::getPass(const String& name) const
    {
        // iterate through techniques to find a match
        for (Pass *p : mPasses) {
            if (p->getName() == name )
                return p;
        }

        return (Pass *)0;
    }
    //-----------------------------------------------------------------------------
    void Technique::removePass(unsigned short index)
    {
        assert(index < mPasses.size() && "Index out of bounds");
        Passes::iterator i = mPasses.begin() + index;
        (*i)->queueForDeletion();
        i = mPasses.erase(i);
        // Adjust passes index
        for (; i != mPasses.end(); ++i, ++index)
        {
            (*i)->_notifyIndex(index);
        }
    }
    //-----------------------------------------------------------------------------
    void Technique::removeAllPasses(void)
    {
        for (auto *p : mPasses)
        {
            p->queueForDeletion();
        }
        mPasses.clear();
    }

    //-----------------------------------------------------------------------------
    bool Technique::movePass(const unsigned short sourceIndex, const unsigned short destinationIndex)
    {
        bool moveSuccessful = false;

        // don't move the pass if source == destination
        if (sourceIndex == destinationIndex) return true;

        if( (sourceIndex < mPasses.size()) && (destinationIndex < mPasses.size()))
        {
            Passes::iterator i = mPasses.begin() + sourceIndex;
            //Passes::iterator DestinationIterator = mPasses.begin() + destinationIndex;

            Pass* pass = (*i);
            mPasses.erase(i);

            i = mPasses.begin() + destinationIndex;

            mPasses.insert(i, pass);

            // Adjust passes index
            unsigned short beginIndex, endIndex;
            if (destinationIndex > sourceIndex)
            {
                beginIndex = sourceIndex;
                endIndex = destinationIndex;
            }
            else
            {
                beginIndex = destinationIndex;
                endIndex = sourceIndex;
            }
            for (unsigned short index = beginIndex; index <= endIndex; ++index)
            {
                mPasses[index]->_notifyIndex(index);
            }
            moveSuccessful = true;
        }

        return moveSuccessful;
    }

    //-----------------------------------------------------------------------------
    const Technique::PassIterator Technique::getPassIterator(void)
    {
        return PassIterator(mPasses.begin(), mPasses.end());
    }
    //-----------------------------------------------------------------------------
    Technique& Technique::operator=(const Technique& rhs)
    {
        if (this == &rhs)
            return *this;

        mName = rhs.mName;
        this->mIsSupported = rhs.mIsSupported;
        this->mLodIndex = rhs.mLodIndex;
        this->mSchemeIndex = rhs.mSchemeIndex;
        this->mShadowCasterMaterial = rhs.mShadowCasterMaterial;
        this->mShadowCasterMaterialName = rhs.mShadowCasterMaterialName;
        this->mShadowReceiverMaterial = rhs.mShadowReceiverMaterial;
        this->mShadowReceiverMaterialName = rhs.mShadowReceiverMaterialName;
        this->mGPUVendorRules = rhs.mGPUVendorRules;
        this->mGPUDeviceNameRules = rhs.mGPUDeviceNameRules;

        // copy passes
        removeAllPasses();
        for (auto *rp : rhs.mPasses)
        {
            Pass* p = OGRE_NEW Pass(this, rp->getIndex(), *(rp));
            mPasses.push_back(p);
        }
        // Compile for categorised illumination on demand
        clearIlluminationPasses();
        mIlluminationPassesCompilationPhase = IPS_NOT_COMPILED;
        return *this;
    }
    //-----------------------------------------------------------------------------
    bool Technique::isTransparent(void) const
    {
        if (mPasses.empty())
        {
            return false;
        }
        else
        {
            // Base decision on the transparency of the first pass
            return mPasses[0]->isTransparent();
        }
    }
    //-----------------------------------------------------------------------------
    bool Technique::isTransparentSortingEnabled(void) const
    {
        if (mPasses.empty())
        {
            return true;
        }
        else
        {
            // Base decision on the transparency of the first pass
            return mPasses[0]->getTransparentSortingEnabled();
        }
    }
    //-----------------------------------------------------------------------------
    bool Technique::isTransparentSortingForced(void) const
    {
        if (mPasses.empty())
        {
            return false;
        }
        else
        {
            // Base decision on the first pass
            return mPasses[0]->getTransparentSortingForced();
        }
    }
    //-----------------------------------------------------------------------------
    bool Technique::isDepthWriteEnabled(void) const
    {
        if (mPasses.empty())
        {
            return false;
        }
        else
        {
            // Base decision on the depth settings of the first pass
            return mPasses[0]->getDepthWriteEnabled();
        }
    }
    //-----------------------------------------------------------------------------
    bool Technique::isDepthCheckEnabled(void) const
    {
        if (mPasses.empty())
        {
            return false;
        }
        else
        {
            // Base decision on the depth settings of the first pass
            return mPasses[0]->getDepthCheckEnabled();
        }
    }
    //-----------------------------------------------------------------------------
    bool Technique::hasColourWriteDisabled(void) const
    {
        if (mPasses.empty())
        {
            return true;
        }
        else
        {
            // Base decision on the colour write settings of the first pass
            return !mPasses[0]->getColourWriteEnabled();
        }
    }
    //-----------------------------------------------------------------------------
    void Technique::_prepare(void)
    {
        assert (mIsSupported && "This technique is not supported");
        // Load each pass
        for (auto *p : mPasses)
        {
            p->_prepare();
        }

        for (auto *i : mIlluminationPasses)
        {
            if(i->pass != i->originalPass)
                i->pass->_prepare();
        }
    }
    //-----------------------------------------------------------------------------
    void Technique::_unprepare(void)
    {
        // Unload each pass
        for (auto *p : mPasses)
        {
            p->_unprepare();
        }
    }
    //-----------------------------------------------------------------------------
    void Technique::_load(void)
    {
        // Load each pass
        for (auto *p : mPasses)
        {
            p->_load();
        }

        for (auto *i : mIlluminationPasses)
        {
            if(i->pass != i->originalPass)
                i->pass->_load();
        }

        if (!mShadowCasterMaterial && !mShadowCasterMaterialName.empty())
        {
            // in case we could not get material as it wasn't yet parsed/existent at that time.
            mShadowCasterMaterial = MaterialManager::getSingleton().getByName(mShadowCasterMaterialName);
        }

        if (mShadowCasterMaterial && mShadowCasterMaterial.get() != getParent())
        {
            mShadowCasterMaterial->load();
        }

        if(!mShadowReceiverMaterial && !mShadowReceiverMaterialName.empty())
        {
            // in case we could not get material as it wasn't yet parsed/existent at that time.
            mShadowReceiverMaterial = MaterialManager::getSingleton().getByName(mShadowReceiverMaterialName);
        }

        if (mShadowReceiverMaterial && mShadowReceiverMaterial.get() != getParent())
        {
            mShadowReceiverMaterial->load();
        }
    }
    //-----------------------------------------------------------------------------
    void Technique::_unload(void)
    {
        // Unload each pass
        for (auto *p : mPasses)
        {
            p->_unload();
        }
    }
    //-----------------------------------------------------------------------------
    bool Technique::isLoaded(void) const
    {
        // Only supported technique will be loaded
        return mParent->isLoaded() && mIsSupported;
    }
    //-----------------------------------------------------------------------
    #define ALL_PASSES(fncall) for(auto p : mPasses) p->fncall
    void Technique::setPointSize(Real ps) { ALL_PASSES(setPointSize(ps)); }
    //-----------------------------------------------------------------------
    void Technique::setAmbient(float red, float green, float blue) { setAmbient(ColourValue(red, green, blue)); }
    //-----------------------------------------------------------------------
    void Technique::setAmbient(const ColourValue& ambient) { ALL_PASSES(setAmbient(ambient)); }
    //-----------------------------------------------------------------------
    void Technique::setDiffuse(float red, float green, float blue, float alpha)
    {
        ALL_PASSES(setDiffuse(red, green, blue, alpha));
    }
    //-----------------------------------------------------------------------
    void Technique::setDiffuse(const ColourValue& diffuse) { setDiffuse(diffuse.r, diffuse.g, diffuse.b, diffuse.a); }
    //-----------------------------------------------------------------------
    void Technique::setSpecular(float red, float green, float blue, float alpha)
    {
        ALL_PASSES(setSpecular(red, green, blue, alpha));
    }
    //-----------------------------------------------------------------------
    void Technique::setSpecular(const ColourValue& specular)
    {
        setSpecular(specular.r, specular.g, specular.b, specular.a);
    }
    //-----------------------------------------------------------------------
    void Technique::setShininess(Real val) { ALL_PASSES(setShininess(val)); }
    //-----------------------------------------------------------------------
    void Technique::setSelfIllumination(float red, float green, float blue)
    {
        setSelfIllumination(ColourValue(red, green, blue));
    }
    //-----------------------------------------------------------------------
    void Technique::setSelfIllumination(const ColourValue& selfIllum) { ALL_PASSES(setSelfIllumination(selfIllum)); }
    //-----------------------------------------------------------------------
    void Technique::setDepthCheckEnabled(bool enabled) { ALL_PASSES(setDepthCheckEnabled(enabled)); }
    //-----------------------------------------------------------------------
    void Technique::setDepthWriteEnabled(bool enabled) { ALL_PASSES(setDepthWriteEnabled(enabled)); }
    //-----------------------------------------------------------------------
    void Technique::setDepthFunction(CompareFunction func) { ALL_PASSES(setDepthFunction(func)); }
    //-----------------------------------------------------------------------
    void Technique::setColourWriteEnabled(bool enabled) { ALL_PASSES(setColourWriteEnabled(enabled)); }
    //-----------------------------------------------------------------------
    void Technique::setColourWriteEnabled(bool red, bool green, bool blue, bool alpha)
    {
        ALL_PASSES(setColourWriteEnabled(red, green, blue, alpha));
    }
    //-----------------------------------------------------------------------
    void Technique::setCullingMode(CullingMode mode) { ALL_PASSES(setCullingMode(mode)); }
    //-----------------------------------------------------------------------
    void Technique::setManualCullingMode(ManualCullingMode mode) { ALL_PASSES(setManualCullingMode(mode)); }
    //-----------------------------------------------------------------------
    void Technique::setLightingEnabled(bool enabled) { ALL_PASSES(setLightingEnabled(enabled)); }
    //-----------------------------------------------------------------------
    void Technique::setShadingMode(ShadeOptions mode) { ALL_PASSES(setShadingMode(mode)); }
    //-----------------------------------------------------------------------
    void Technique::setFog(bool overrideScene, FogMode mode, const ColourValue& colour, Real expDensity,
                           Real linearStart, Real linearEnd)
    {
        ALL_PASSES(setFog(overrideScene, mode, colour, expDensity, linearStart, linearEnd));
    }
    //-----------------------------------------------------------------------
    void Technique::setDepthBias(float constantBias, float slopeScaleBias)
    {
        ALL_PASSES(setDepthBias(constantBias, slopeScaleBias));
    }
    //-----------------------------------------------------------------------
    void Technique::setTextureFiltering(TextureFilterOptions filterType)
    {
        ALL_PASSES(setTextureFiltering(filterType));
    }
    // --------------------------------------------------------------------
    void Technique::setTextureAnisotropy(unsigned int maxAniso) { ALL_PASSES(setTextureAnisotropy(maxAniso)); }
    // --------------------------------------------------------------------
    void Technique::setSceneBlending(const SceneBlendType sbt) { ALL_PASSES(setSceneBlending(sbt)); }
    // --------------------------------------------------------------------
    void Technique::setSeparateSceneBlending(const SceneBlendType sbt, const SceneBlendType sbta)
    {
        ALL_PASSES(setSeparateSceneBlending(sbt, sbta));
    }
    // --------------------------------------------------------------------
    void Technique::setSceneBlending(const SceneBlendFactor sourceFactor, const SceneBlendFactor destFactor)
    {
        ALL_PASSES(setSceneBlending(sourceFactor, destFactor));
    }
    // --------------------------------------------------------------------
    void Technique::setSeparateSceneBlending( const SceneBlendFactor sourceFactor, const SceneBlendFactor destFactor, const SceneBlendFactor sourceFactorAlpha, const SceneBlendFactor destFactorAlpha)
    {
        ALL_PASSES(setSeparateSceneBlending(sourceFactor, destFactor, sourceFactorAlpha, destFactorAlpha));
    }
    #undef ALL_PASSES

    // --------------------------------------------------------------------
    void Technique::setName(const String& name)
    {
        mName = name;
    }


    //-----------------------------------------------------------------------
    void Technique::_notifyNeedsRecompile(void)
    {
        // Disable require to recompile when splitting illumination passes
        if (mIlluminationPassesCompilationPhase != IPS_COMPILE_DISABLED)
        {
            mParent->_notifyNeedsRecompile();
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setLodIndex(unsigned short index)
    {
        mLodIndex = index;
        _notifyNeedsRecompile();
    }
    //-----------------------------------------------------------------------
    void Technique::setSchemeName(const String& schemeName)
    {
        mSchemeIndex = MaterialManager::getSingleton()._getSchemeIndex(schemeName);
        _notifyNeedsRecompile();
    }
    //-----------------------------------------------------------------------
    const String& Technique::getSchemeName(void) const
    {
        return MaterialManager::getSingleton()._getSchemeName(mSchemeIndex);
    }
    //-----------------------------------------------------------------------
    unsigned short Technique::_getSchemeIndex(void) const
    {
        return mSchemeIndex;
    }
    //---------------------------------------------------------------------
    bool Technique::checkManuallyOrganisedIlluminationPasses()
    {
        // first check whether all passes have manually assigned illumination
        for (auto *p : mPasses)
        {
            if (p->getIlluminationStage() == IS_UNKNOWN)
                return false;
        }

        // ok, all manually controlled, so just use that
        for (auto *p : mPasses)
        {
            IlluminationPass* iPass = OGRE_NEW IlluminationPass();
            iPass->destroyOnShutdown = false;
            iPass->originalPass = iPass->pass = p;
            iPass->stage = p->getIlluminationStage();
            mIlluminationPasses.push_back(iPass);
        }

        return true;
    }
    //-----------------------------------------------------------------------
    void Technique::_compileIlluminationPasses(void)
    {
        clearIlluminationPasses();

        if (!checkManuallyOrganisedIlluminationPasses())
        {
            // Build based on our own heuristics

            Passes::iterator i, iend;
            iend = mPasses.end();
            i = mPasses.begin();

            IlluminationStage iStage = IS_AMBIENT;

            bool haveAmbient = false;
            while (i != iend)
            {
                IlluminationPass* iPass;
                Pass* p = *i;
                switch(iStage)
                {
                case IS_AMBIENT:
                    // Keep looking for ambient only
                    if (p->isAmbientOnly())
                    {
                        // Add this pass wholesale
                        iPass = OGRE_NEW IlluminationPass();
                        iPass->destroyOnShutdown = false;
                        iPass->originalPass = iPass->pass = p;
                        iPass->stage = iStage;
                        mIlluminationPasses.push_back(iPass);
                        haveAmbient = true;
                        // progress to next pass
                        ++i;
                    }
                    else
                    {
                        // Split off any ambient part
                        if (p->getAmbient() != ColourValue::Black ||
                            p->getSelfIllumination() != ColourValue::Black ||
                            p->getAlphaRejectFunction() != CMPF_ALWAYS_PASS)
                        {
                            // Copy existing pass
                            Pass* newPass = OGRE_NEW Pass(this, p->getIndex(), *p);
                            if (newPass->getAlphaRejectFunction() != CMPF_ALWAYS_PASS)
                            {
                                // Alpha rejection passes must retain their transparency, so
                                // we allow the texture units, but override the colour functions
                                for(auto *s : newPass->getTextureUnitStates())
                                {
                                    s->setColourOperationEx(LBX_SOURCE1, LBS_CURRENT);
                                }
                            }
                            else
                            {
                                // Remove any texture units
                                newPass->removeAllTextureUnitStates();
                            }
                            // Remove any fragment program
                            if (newPass->hasFragmentProgram())
                                newPass->setFragmentProgram("");
                            // We have to leave vertex program alone (if any) and
                            // just trust that the author is using light bindings, which
                            // we will ensure there are none in the ambient pass
                            newPass->setDiffuse(0, 0, 0, newPass->getDiffuse().a);  // Preserving alpha
                            newPass->setSpecular(ColourValue::Black);

                            // Calculate hash value for new pass, because we are compiling
                            // illumination passes on demand, which will loss hash calculate
                            // before it add to render queue first time.
                            newPass->_recalculateHash();

                            iPass = OGRE_NEW IlluminationPass();
                            iPass->destroyOnShutdown = true;
                            iPass->originalPass = p;
                            iPass->pass = newPass;
                            iPass->stage = iStage;

                            mIlluminationPasses.push_back(iPass);
                            haveAmbient = true;

                        }

                        if (!haveAmbient)
                        {
                            // Make up a new basic pass
                            Pass* newPass = OGRE_NEW Pass(this, p->getIndex());
                            newPass->setAmbient(ColourValue::Black);
                            newPass->setDiffuse(ColourValue::Black);

                            // Calculate hash value for new pass, because we are compiling
                            // illumination passes on demand, which will loss hash calculate
                            // before it add to render queue first time.
                            newPass->_recalculateHash();

                            iPass = OGRE_NEW IlluminationPass();
                            iPass->destroyOnShutdown = true;
                            iPass->originalPass = p;
                            iPass->pass = newPass;
                            iPass->stage = iStage;
                            mIlluminationPasses.push_back(iPass);
                            haveAmbient = true;
                        }
                        // This means we're done with ambients, progress to per-light
                        iStage = IS_PER_LIGHT;
                    }
                    break;
                case IS_PER_LIGHT:
                    if (p->getIteratePerLight())
                    {
                        // If this is per-light already, use it directly
                        iPass = OGRE_NEW IlluminationPass();
                        iPass->destroyOnShutdown = false;
                        iPass->originalPass = iPass->pass = p;
                        iPass->stage = iStage;
                        mIlluminationPasses.push_back(iPass);
                        // progress to next pass
                        ++i;
                    }
                    else
                    {
                        // Split off per-light details (can only be done for one)
                        if (p->getLightingEnabled() &&
                            (p->getDiffuse() != ColourValue::Black ||
                            p->getSpecular() != ColourValue::Black))
                        {
                            // Copy existing pass
                            Pass* newPass = OGRE_NEW Pass(this, p->getIndex(), *p);
                            if (newPass->getAlphaRejectFunction() != CMPF_ALWAYS_PASS)
                            {
                                // Alpha rejection passes must retain their transparency, so
                                // we allow the texture units, but override the colour functions
                                for(auto *s : newPass->getTextureUnitStates())
                                {
                                    s->setColourOperationEx(LBX_SOURCE1, LBS_CURRENT);
                                }
                            }
                            else
                            {
                                // remove texture units
                                newPass->removeAllTextureUnitStates();
                            }
                            // remove fragment programs
                            if (newPass->hasFragmentProgram())
                                newPass->setFragmentProgram("");
                            // Cannot remove vertex program, have to assume that
                            // it will process diffuse lights, ambient will be turned off
                            newPass->setAmbient(ColourValue::Black);
                            newPass->setSelfIllumination(ColourValue::Black);
                            // must be additive
                            newPass->setSceneBlending(SBF_ONE, SBF_ONE);

                            // Calculate hash value for new pass, because we are compiling
                            // illumination passes on demand, which will loss hash calculate
                            // before it add to render queue first time.
                            newPass->_recalculateHash();

                            iPass = OGRE_NEW IlluminationPass();
                            iPass->destroyOnShutdown = true;
                            iPass->originalPass = p;
                            iPass->pass = newPass;
                            iPass->stage = iStage;

                            mIlluminationPasses.push_back(iPass);

                        }
                        // This means the end of per-light passes
                        iStage = IS_DECAL;
                    }
                    break;
                case IS_DECAL:
                    // We just want a 'lighting off' pass to finish off
                    // and only if there are texture units
                    if (p->getNumTextureUnitStates() > 0)
                    {
                        if (!p->getLightingEnabled())
                        {
                            // we assume this pass already combines as required with the scene
                            iPass = OGRE_NEW IlluminationPass();
                            iPass->destroyOnShutdown = false;
                            iPass->originalPass = iPass->pass = p;
                            iPass->stage = iStage;
                            mIlluminationPasses.push_back(iPass);
                        }
                        else
                        {
                            // Copy the pass and tweak away the lighting parts
                            Pass* newPass = OGRE_NEW Pass(this, p->getIndex(), *p);
                            newPass->setAmbient(ColourValue::Black);
                            newPass->setDiffuse(0, 0, 0, newPass->getDiffuse().a);  // Preserving alpha
                            newPass->setSpecular(ColourValue::Black);
                            newPass->setSelfIllumination(ColourValue::Black);
                            newPass->setLightingEnabled(false);
                            newPass->setIteratePerLight(false, false);
                            // modulate
                            newPass->setSceneBlending(SBF_DEST_COLOUR, SBF_ZERO);

                            // Calculate hash value for new pass, because we are compiling
                            // illumination passes on demand, which will loss hash calculate
                            // before it add to render queue first time.
                            newPass->_recalculateHash();

                            // NB there is nothing we can do about vertex & fragment
                            // programs here, so people will just have to make their
                            // programs friendly-like if they want to use this technique
                            iPass = OGRE_NEW IlluminationPass();
                            iPass->destroyOnShutdown = true;
                            iPass->originalPass = p;
                            iPass->pass = newPass;
                            iPass->stage = iStage;
                            mIlluminationPasses.push_back(iPass);

                        }
                    }
                    ++i; // always increment on decal, since nothing more to do with this pass

                    break;
                case IS_UNKNOWN:
                    break;
                }
            }
        }

    }
    //-----------------------------------------------------------------------
    void Technique::clearIlluminationPasses(void)
    {
        if(MaterialManager::getSingletonPtr())
            MaterialManager::getSingleton()._notifyBeforeIlluminationPassesCleared(this);

        for (auto *i : mIlluminationPasses)
        {
            if (i->destroyOnShutdown)
            {
                i->pass->queueForDeletion();
            }
            OGRE_DELETE i;
        }
        mIlluminationPasses.clear();
    }
    //-----------------------------------------------------------------------
    const IlluminationPassList&
    Technique::getIlluminationPasses(void)
    {
        IlluminationPassesState targetState = IPS_COMPILED;
        if(mIlluminationPassesCompilationPhase != targetState
        && mIlluminationPassesCompilationPhase != IPS_COMPILE_DISABLED)
        {
            // prevents parent->_notifyNeedsRecompile() call during compile
            mIlluminationPassesCompilationPhase = IPS_COMPILE_DISABLED;
            // Splitting the passes into illumination passes
            _compileIlluminationPasses();
            // Post notification, so that technique owner can post-process created passes
            if(MaterialManager::getSingletonPtr())
                MaterialManager::getSingleton()._notifyAfterIlluminationPassesCreated(this);
            // Mark that illumination passes compilation finished
            mIlluminationPassesCompilationPhase = targetState;
        }

        return mIlluminationPasses;
    }
    //-----------------------------------------------------------------------
    const String& Technique::getResourceGroup(void) const
    {
        return mParent->getGroup();
    }
    //-----------------------------------------------------------------------
    Ogre::MaterialPtr  Technique::getShadowCasterMaterial() const
    {
        return mShadowCasterMaterial;
    }
    //-----------------------------------------------------------------------
    void  Technique::setShadowCasterMaterial(Ogre::MaterialPtr val)
    {
        if (!val)
        {
            mShadowCasterMaterial.reset();
            mShadowCasterMaterialName.clear();
        }
        else
        {
            // shadow caster material should never receive shadows
            val->setReceiveShadows(false); // should we warn if this is not set?
            mShadowCasterMaterial = val;
            mShadowCasterMaterialName = val->getName();
        }
    }
    //-----------------------------------------------------------------------
    void  Technique::setShadowCasterMaterial(const Ogre::String &name)
    {
        setShadowCasterMaterial(MaterialManager::getSingleton().getByName(name));
        // remember the name, even if it is not created yet
        mShadowCasterMaterialName = name;
    }
    //-----------------------------------------------------------------------
    Ogre::MaterialPtr  Technique::getShadowReceiverMaterial() const
    {
        return mShadowReceiverMaterial;
    }
    //-----------------------------------------------------------------------
    void  Technique::setShadowReceiverMaterial(Ogre::MaterialPtr val)
    {
        if (!val)
        {
            mShadowReceiverMaterial.reset();
            mShadowReceiverMaterialName.clear();
        }
        else
        {
            mShadowReceiverMaterial = val;
            mShadowReceiverMaterialName = val->getName();
        }
    }
    //-----------------------------------------------------------------------
    void  Technique::setShadowReceiverMaterial(const Ogre::String &name)
    {
        mShadowReceiverMaterialName = name;
        mShadowReceiverMaterial = MaterialManager::getSingleton().getByName(name);
    }
    //---------------------------------------------------------------------
    void Technique::addGPUVendorRule(GPUVendor vendor, Technique::IncludeOrExclude includeOrExclude)
    {
        addGPUVendorRule(GPUVendorRule(vendor, includeOrExclude));
    }
    //---------------------------------------------------------------------
    void Technique::addGPUVendorRule(const Technique::GPUVendorRule& rule)
    {
        // remove duplicates
        removeGPUVendorRule(rule.vendor);
        mGPUVendorRules.push_back(rule);
    }
    //---------------------------------------------------------------------
    void Technique::removeGPUVendorRule(GPUVendor vendor)
    {
        for (GPUVendorRuleList::iterator i = mGPUVendorRules.begin(); i != mGPUVendorRules.end(); )
        {
            if (i->vendor == vendor)
                i = mGPUVendorRules.erase(i);
            else
                ++i;
        }
    }
    //---------------------------------------------------------------------
    Technique::GPUVendorRuleIterator Technique::getGPUVendorRuleIterator() const
    {
        return GPUVendorRuleIterator(mGPUVendorRules.begin(), mGPUVendorRules.end());
    }
    //---------------------------------------------------------------------
    void Technique::addGPUDeviceNameRule(const String& devicePattern,
        Technique::IncludeOrExclude includeOrExclude, bool caseSensitive)
    {
        addGPUDeviceNameRule(GPUDeviceNameRule(devicePattern, includeOrExclude, caseSensitive));
    }
    //---------------------------------------------------------------------
    void Technique::addGPUDeviceNameRule(const Technique::GPUDeviceNameRule& rule)
    {
        // remove duplicates
        removeGPUDeviceNameRule(rule.devicePattern);
        mGPUDeviceNameRules.push_back(rule);
    }
    //---------------------------------------------------------------------
    void Technique::removeGPUDeviceNameRule(const String& devicePattern)
    {
        for (GPUDeviceNameRuleList::iterator i = mGPUDeviceNameRules.begin(); i != mGPUDeviceNameRules.end(); )
        {
            if (i->devicePattern == devicePattern)
                i = mGPUDeviceNameRules.erase(i);
            else
                ++i;
        }
    }
    //---------------------------------------------------------------------
    Technique::GPUDeviceNameRuleIterator Technique::getGPUDeviceNameRuleIterator() const
    {
        return GPUDeviceNameRuleIterator(mGPUDeviceNameRules.begin(), mGPUDeviceNameRules.end());
    }
    //---------------------------------------------------------------------
}
