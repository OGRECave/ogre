/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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

#include "OgreTechnique.h"
#include "OgreMaterial.h"
#include "OgrePass.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreGpuProgramManager.h"
#include "OgreMaterialManager.h"


namespace Ogre {
    //-----------------------------------------------------------------------------
    Technique::Technique(Material* parent)
        : mParent(parent), mIsSupported(false), mIlluminationPassesCompilationPhase(IPS_NOT_COMPILED), mLodIndex(0), mSchemeIndex(0)
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
        Passes::const_iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            memSize += (*i)->calculateSize();
        }
        return memSize;
    }
    //-----------------------------------------------------------------------------
    String Technique::_compile(bool autoManageTextureUnits)
    {
		StringUtil::StrStreamType errors;

		mIsSupported = checkGPURules(errors);
		if (mIsSupported)
		{
			mIsSupported = checkHardwareSupport(autoManageTextureUnits, errors);
		}

        // Compile for categorised illumination on demand
        clearIlluminationPasses();
        mIlluminationPassesCompilationPhase = IPS_NOT_COMPILED;

		return errors.str();

    }
	//---------------------------------------------------------------------
	bool Technique::checkHardwareSupport(bool autoManageTextureUnits, StringUtil::StrStreamType& compileErrors)
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
			// Check for advanced blending operation support
			if((currPass->getSceneBlendingOperation() != SBO_ADD || currPass->getSceneBlendingOperationAlpha() != SBO_ADD) && 
				!caps->hasCapability(RSC_ADVANCED_BLEND_OPERATIONS))
			{
				return false;		
			}
			// Check texture unit requirements
			size_t numTexUnitsRequested = currPass->getNumTextureUnitStates();
			// Don't trust getNumTextureUnits for programmable
			if(!currPass->hasFragmentProgram())
			{
#if defined(OGRE_PRETEND_TEXTURE_UNITS) && OGRE_PRETEND_TEXTURE_UNITS > 0
				if (numTexUnits > OGRE_PRETEND_TEXTURE_UNITS)
					numTexUnits = OGRE_PRETEND_TEXTURE_UNITS;
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
			}
			if (currPass->hasComputeProgram())
			{
				// Check fragment program version
				if (!currPass->getComputeProgram()->isSupported())
				{
					// Can't do this one
					compileErrors << "Pass " << passNum << 
						": Compute program " << currPass->getComputeProgram()->getName()
						<< " cannot be used - ";
					if (currPass->getComputeProgram()->hasCompileError())
						compileErrors << "compile error.";
					else
						compileErrors << "not supported.";

					compileErrors << std::endl;
					return false;
				}
			}
			if (currPass->hasVertexProgram())
			{
				// Check vertex program version
				if (!currPass->getVertexProgram()->isSupported() )
				{
					// Can't do this one
					compileErrors << "Pass " << passNum << 
						": Vertex program " << currPass->getVertexProgram()->getName()
						<< " cannot be used - ";
					if (currPass->getVertexProgram()->hasCompileError())
						compileErrors << "compile error.";
					else
						compileErrors << "not supported.";

					compileErrors << std::endl;
					return false;
				}
			}
			if (currPass->hasTessellationHullProgram())
			{
				// Check tessellation control program version
				if (!currPass->getTessellationHullProgram()->isSupported() )
				{
					// Can't do this one
					compileErrors << "Pass " << passNum << 
						": Tessellation Hull program " << currPass->getTessellationHullProgram()->getName()
						<< " cannot be used - ";
					if (currPass->getTessellationHullProgram()->hasCompileError())
						compileErrors << "compile error.";
					else
						compileErrors << "not supported.";

					compileErrors << std::endl;
					return false;
				}
			}
			if (currPass->hasTessellationDomainProgram())
			{
				// Check tessellation control program version
				if (!currPass->getTessellationDomainProgram()->isSupported() )
				{
					// Can't do this one
					compileErrors << "Pass " << passNum << 
						": Tessellation Domain program " << currPass->getTessellationDomainProgram()->getName()
						<< " cannot be used - ";
					if (currPass->getTessellationDomainProgram()->hasCompileError())
						compileErrors << "compile error.";
					else
						compileErrors << "not supported.";

					compileErrors << std::endl;
					return false;
				}
			}
			if (currPass->hasGeometryProgram())
			{
				// Check geometry program version
				if (!currPass->getGeometryProgram()->isSupported() )
				{
					// Can't do this one
					compileErrors << "Pass " << passNum << 
						": Geometry program " << currPass->getGeometryProgram()->getName()
						<< " cannot be used - ";
					if (currPass->getGeometryProgram()->hasCompileError())
						compileErrors << "compile error.";
					else
						compileErrors << "not supported.";

					compileErrors << std::endl;
					return false;
				}
			}
			if (currPass->hasFragmentProgram())
			{
				// Check fragment program version
				if (!currPass->getFragmentProgram()->isSupported())
				{
					// Can't do this one
					compileErrors << "Pass " << passNum << 
						": Fragment program " << currPass->getFragmentProgram()->getName()
						<< " cannot be used - ";
					if (currPass->getFragmentProgram()->hasCompileError())
						compileErrors << "compile error.";
					else
						compileErrors << "not supported.";

					compileErrors << std::endl;
					return false;
				}
			}
			else
			{
				// Check a few fixed-function options in texture layers
				Pass::TextureUnitStateIterator texi = currPass->getTextureUnitStateIterator();
				size_t texUnit = 0;
				while (texi.hasMoreElements())
				{
					TextureUnitState* tex = texi.getNext();
					// Any Cube textures? NB we make the assumption that any
					// card capable of running fragment programs can support
					// cubic textures, which has to be true, surely?
					if (tex->is3D() && !caps->hasCapability(RSC_CUBEMAPPING))
					{
						// Fail
						compileErrors << "Pass " << passNum << 
							" Tex " << texUnit <<
							": Cube maps not supported by current environment."
							<< std::endl;
						return false;
					}
					// Any 3D textures? NB we make the assumption that any
					// card capable of running fragment programs can support
					// 3D textures, which has to be true, surely?
					if (((tex->getTextureType() == TEX_TYPE_3D) || (tex->getTextureType() == TEX_TYPE_2D_ARRAY)) && 
                         !caps->hasCapability(RSC_TEXTURE_3D))
					{
						// Fail
						compileErrors << "Pass " << passNum << 
							" Tex " << texUnit <<
							": Volume textures not supported by current environment."
							<< std::endl;
						return false;
					}
					// Any Dot3 blending?
					if (tex->getColourBlendMode().operation == LBX_DOTPRODUCT &&
						!caps->hasCapability(RSC_DOT3))
					{
						// Fail
						compileErrors << "Pass " << passNum << 
							" Tex " << texUnit <<
							": DOT3 blending not supported by current environment."
							<< std::endl;
						return false;
					}
					++texUnit;
				}

				// We're ok on operations, now we need to check # texture units
				if (!currPass->hasFragmentProgram())
				{
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
						std::copy_backward(i, (mPasses.end()-1), mPasses.end());
						*i = currPass;
						// Adjust pass index
						currPass->_notifyIndex(passNum);
					}
				}
			}

		}
		// If we got this far, we're ok
		return true;
	}
	//---------------------------------------------------------------------
	bool Technique::checkGPURules(StringUtil::StrStreamType& errors)
	{
		const RenderSystemCapabilities* caps =
			Root::getSingleton().getRenderSystem()->getCapabilities();

		StringUtil::StrStreamType includeRules;
		bool includeRulesPresent = false;
		bool includeRuleMatched = false;

		// Check vendors first
		for (GPUVendorRuleList::const_iterator i = mGPUVendorRules.begin();
			i != mGPUVendorRules.end(); ++i)
		{
			if (i->includeOrExclude == INCLUDE)
			{
				includeRulesPresent = true;
				includeRules << caps->vendorToString(i->vendor) << " ";
				if (i->vendor == caps->getVendor())
					includeRuleMatched = true;
			}
			else // EXCLUDE
			{
				if (i->vendor == caps->getVendor())
				{
					errors << "Excluded GPU vendor: " << caps->vendorToString(i->vendor)
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
		includeRules.str(StringUtil::BLANK);
		includeRulesPresent = false;
		includeRuleMatched = false;

		for (GPUDeviceNameRuleList::const_iterator i = mGPUDeviceNameRules.begin();
			i != mGPUDeviceNameRules.end(); ++i)
		{
			if (i->includeOrExclude == INCLUDE)
			{
				includeRulesPresent = true;
				includeRules << i->devicePattern << " ";
				if (StringUtil::match(caps->getDeviceName(), i->devicePattern, i->caseSensitive))
					includeRuleMatched = true;
			}
			else // EXCLUDE
			{
				if (StringUtil::match(caps->getDeviceName(), i->devicePattern, i->caseSensitive))
				{
					errors << "Excluded GPU device: " << i->devicePattern
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
    Pass* Technique::getPass(unsigned short index)
    {
		assert(index < mPasses.size() && "Index out of bounds");
		return mPasses[index];
    }
    //-----------------------------------------------------------------------------
    Pass* Technique::getPass(const String& name)
    {
        Passes::iterator i    = mPasses.begin();
        Passes::iterator iend = mPasses.end();
        Pass* foundPass = 0;

        // iterate through techniques to find a match
        while (i != iend)
        {
            if ( (*i)->getName() == name )
            {
                foundPass = (*i);
                break;
            }
            ++i;
        }

        return foundPass;
    }
    //-----------------------------------------------------------------------------
    unsigned short Technique::getNumPasses(void) const
    {
		return static_cast<unsigned short>(mPasses.size());
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
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->queueForDeletion();
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
		Passes::const_iterator i, iend;
		iend = rhs.mPasses.end();
		for (i = rhs.mPasses.begin(); i != iend; ++i)
		{
			Pass* p = OGRE_NEW Pass(this, (*i)->getIndex(), *(*i));
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
		Passes::iterator i, iend;
		iend = mPasses.end();
		for (i = mPasses.begin(); i != iend; ++i)
		{
			(*i)->_prepare();
		}

		IlluminationPassList::iterator il, ilend;
		ilend = mIlluminationPasses.end();
		for (il = mIlluminationPasses.begin(); il != ilend; ++il)
		{
			if((*il)->pass != (*il)->originalPass)
			    (*il)->pass->_prepare();
		}
    }
    //-----------------------------------------------------------------------------
    void Technique::_unprepare(void)
    {
		// Unload each pass
		Passes::iterator i, iend;
		iend = mPasses.end();
		for (i = mPasses.begin(); i != iend; ++i)
		{
			(*i)->_unprepare();
		}
    }
    //-----------------------------------------------------------------------------
    void Technique::_load(void)
    {
		assert (mIsSupported && "This technique is not supported");
		// Load each pass
		Passes::iterator i, iend;
		iend = mPasses.end();
		for (i = mPasses.begin(); i != iend; ++i)
		{
			(*i)->_load();
		}

		IlluminationPassList::iterator il, ilend;
		ilend = mIlluminationPasses.end();
		for (il = mIlluminationPasses.begin(); il != ilend; ++il)
		{
			if((*il)->pass != (*il)->originalPass)
			    (*il)->pass->_load();
		}

		if (!mShadowCasterMaterial.isNull())
		{
			mShadowCasterMaterial->load();
		}
		else if (!mShadowCasterMaterialName.empty())
		{
			// in case we could not get material as it wasn't yet parsed/existent at that time.
			mShadowCasterMaterial = MaterialManager::getSingleton().getByName(mShadowCasterMaterialName);
            if (!mShadowCasterMaterial.isNull())
			    mShadowCasterMaterial->load();
		}
		if (!mShadowReceiverMaterial.isNull())
		{
			mShadowReceiverMaterial->load();
		}
		else if (!mShadowReceiverMaterialName.empty())
		{
			// in case we could not get material as it wasn't yet parsed/existent at that time.
			mShadowReceiverMaterial = MaterialManager::getSingleton().getByName(mShadowReceiverMaterialName);
            if (!mShadowReceiverMaterial.isNull())
			    mShadowReceiverMaterial->load();
		}
    }
    //-----------------------------------------------------------------------------
    void Technique::_unload(void)
    {
		// Unload each pass
		Passes::iterator i, iend;
		iend = mPasses.end();
		for (i = mPasses.begin(); i != iend; ++i)
		{
			(*i)->_unload();
		}	
    }
    //-----------------------------------------------------------------------------
    bool Technique::isLoaded(void) const
    {
        // Only supported technique will be loaded
        return mParent->isLoaded() && mIsSupported;
    }
    //-----------------------------------------------------------------------
    void Technique::setPointSize(Real ps)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setPointSize(ps);
        }

    }
    //-----------------------------------------------------------------------
    void Technique::setAmbient(Real red, Real green, Real blue)
    {
		setAmbient(ColourValue(red, green, blue));
        

    }
    //-----------------------------------------------------------------------
    void Technique::setAmbient(const ColourValue& ambient)
    {
		Passes::iterator i, iend;
		iend = mPasses.end();
		for (i = mPasses.begin(); i != iend; ++i)
		{
			(*i)->setAmbient(ambient);
		}
    }
    //-----------------------------------------------------------------------
    void Technique::setDiffuse(Real red, Real green, Real blue, Real alpha)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setDiffuse(red, green, blue, alpha);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setDiffuse(const ColourValue& diffuse)
    {
        setDiffuse(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
    }
    //-----------------------------------------------------------------------
    void Technique::setSpecular(Real red, Real green, Real blue, Real alpha)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setSpecular(red, green, blue, alpha);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setSpecular(const ColourValue& specular)
    {
        setSpecular(specular.r, specular.g, specular.b, specular.a);
    }
    //-----------------------------------------------------------------------
    void Technique::setShininess(Real val)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setShininess(val);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setSelfIllumination(Real red, Real green, Real blue)
    {
        setSelfIllumination(ColourValue(red, green, blue));
    }
    //-----------------------------------------------------------------------
    void Technique::setSelfIllumination(const ColourValue& selfIllum)
    {
		Passes::iterator i, iend;
		iend = mPasses.end();
		for (i = mPasses.begin(); i != iend; ++i)
		{
			(*i)->setSelfIllumination(selfIllum);
		}
    }
    //-----------------------------------------------------------------------
    void Technique::setDepthCheckEnabled(bool enabled)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setDepthCheckEnabled(enabled);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setDepthWriteEnabled(bool enabled)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setDepthWriteEnabled(enabled);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setDepthFunction( CompareFunction func )
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setDepthFunction(func);
        }
    }
    //-----------------------------------------------------------------------
	void Technique::setColourWriteEnabled(bool enabled)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setColourWriteEnabled(enabled);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setCullingMode( CullingMode mode )
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setCullingMode(mode);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setManualCullingMode( ManualCullingMode mode )
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setManualCullingMode(mode);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setLightingEnabled(bool enabled)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setLightingEnabled(enabled);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setShadingMode( ShadeOptions mode )
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setShadingMode(mode);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setFog(bool overrideScene, FogMode mode, const ColourValue& colour,
        Real expDensity, Real linearStart, Real linearEnd)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setFog(overrideScene, mode, colour, expDensity, linearStart, linearEnd);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setDepthBias(float constantBias, float slopeScaleBias)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setDepthBias(constantBias, slopeScaleBias);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setTextureFiltering(TextureFilterOptions filterType)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setTextureFiltering(filterType);
        }
    }
    // --------------------------------------------------------------------
    void Technique::setTextureAnisotropy(unsigned int maxAniso)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setTextureAnisotropy(maxAniso);
        }
    }
    // --------------------------------------------------------------------
    void Technique::setSceneBlending( const SceneBlendType sbt )
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setSceneBlending(sbt);
        }
    }
    // --------------------------------------------------------------------
    void Technique::setSeparateSceneBlending( const SceneBlendType sbt, const SceneBlendType sbta )
	{
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setSeparateSceneBlending(sbt, sbta);
        }
	}
    // --------------------------------------------------------------------
    void Technique::setSceneBlending( const SceneBlendFactor sourceFactor,
        const SceneBlendFactor destFactor)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setSceneBlending(sourceFactor, destFactor);
        }
    }
    // --------------------------------------------------------------------
    void Technique::setSeparateSceneBlending( const SceneBlendFactor sourceFactor, const SceneBlendFactor destFactor, const SceneBlendFactor sourceFactorAlpha, const SceneBlendFactor destFactorAlpha)
	{
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setSeparateSceneBlending(sourceFactor, destFactor, sourceFactorAlpha, destFactorAlpha);
        }
	}

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
		Passes::iterator i, ibegin, iend;
		ibegin = mPasses.begin();
		iend = mPasses.end();

		for (i = ibegin; i != iend; ++i)
		{
			if ((*i)->getIlluminationStage() == IS_UNKNOWN)
				return false;
		}

		// ok, all manually controlled, so just use that
		for (i = ibegin; i != iend; ++i)
		{
			IlluminationPass* iPass = OGRE_NEW IlluminationPass();
			iPass->destroyOnShutdown = false;
			iPass->originalPass = iPass->pass = *i;
			iPass->stage = (*i)->getIlluminationStage();
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
								Pass::TextureUnitStateIterator tusi = newPass->getTextureUnitStateIterator();
								while (tusi.hasMoreElements())
								{
									TextureUnitState* tus = tusi.getNext();
									tus->setColourOperationEx(LBX_SOURCE1, LBS_CURRENT);
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
								Pass::TextureUnitStateIterator tusi = newPass->getTextureUnitStateIterator();
								while (tusi.hasMoreElements())
								{
									TextureUnitState* tus = tusi.getNext();
									tus->setColourOperationEx(LBX_SOURCE1, LBS_CURRENT);
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
        IlluminationPassList::iterator i, iend;
        iend = mIlluminationPasses.end();
        for (i = mIlluminationPasses.begin(); i != iend; ++i)
        {
            if ((*i)->destroyOnShutdown)
            {
                (*i)->pass->queueForDeletion();
            }
            OGRE_DELETE *i;
        }
        mIlluminationPasses.clear();
    }
    //-----------------------------------------------------------------------
    const Technique::IlluminationPassIterator
    Technique::getIlluminationPassIterator(void)
    {
        IlluminationPassesState targetState = IPS_COMPILED;
        if (mIlluminationPassesCompilationPhase != targetState)
        {
            // prevents parent->_notifyNeedsRecompile() call during compile
            mIlluminationPassesCompilationPhase = IPS_COMPILE_DISABLED;
            // Splitting the passes into illumination passes
            _compileIlluminationPasses();
            // Mark that illumination passes compilation finished
            mIlluminationPassesCompilationPhase = targetState;
        }

        return IlluminationPassIterator(mIlluminationPasses.begin(),
            mIlluminationPasses.end());
    }
    //-----------------------------------------------------------------------
	const String& Technique::getResourceGroup(void) const
	{
		return mParent->getGroup();
	}

    //-----------------------------------------------------------------------
    bool Technique::applyTextureAliases(const AliasTextureNamePairList& aliasList, const bool apply) const
    {
        // iterate through passes and apply texture alias
        Passes::const_iterator i, iend;
        iend = mPasses.end();
        bool testResult = false;

        for(i = mPasses.begin(); i != iend; ++i)
        {
            if ((*i)->applyTextureAliases(aliasList, apply))
                testResult = true;
        }

        return testResult;
    }
	//-----------------------------------------------------------------------
	Ogre::MaterialPtr  Technique::getShadowCasterMaterial() const 
	{ 
		return mShadowCasterMaterial; 
	}
	//-----------------------------------------------------------------------
	void  Technique::setShadowCasterMaterial(Ogre::MaterialPtr val) 
	{ 
		if (val.isNull())
		{
			mShadowCasterMaterial.setNull();
			mShadowCasterMaterialName.clear();
		}
		else
		{
			mShadowCasterMaterial = val; 
			mShadowCasterMaterialName = val->getName();
		}
	}
	//-----------------------------------------------------------------------
	void  Technique::setShadowCasterMaterial(const Ogre::String &name) 
	{ 
		mShadowCasterMaterialName = name;
		mShadowCasterMaterial = MaterialManager::getSingleton().getByName(name);
	}
	//-----------------------------------------------------------------------
	Ogre::MaterialPtr  Technique::getShadowReceiverMaterial() const 
	{ 
		return mShadowReceiverMaterial; 
	}
	//-----------------------------------------------------------------------
	void  Technique::setShadowReceiverMaterial(Ogre::MaterialPtr val) 
	{ 
		if (val.isNull())
		{
			mShadowReceiverMaterial.setNull();
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
