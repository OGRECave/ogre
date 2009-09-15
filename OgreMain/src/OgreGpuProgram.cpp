/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2009 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"
#include "OgreGpuProgram.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreGpuProgramManager.h"
#include "OgreVector3.h"
#include "OgreVector4.h"
#include "OgreAutoParamDataSource.h"
#include "OgreLight.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"

namespace Ogre
{
    //-----------------------------------------------------------------------------
    GpuProgram::CmdType GpuProgram::msTypeCmd;
    GpuProgram::CmdSyntax GpuProgram::msSyntaxCmd;
    GpuProgram::CmdSkeletal GpuProgram::msSkeletalCmd;
	GpuProgram::CmdMorph GpuProgram::msMorphCmd;
	GpuProgram::CmdPose GpuProgram::msPoseCmd;
	GpuProgram::CmdVTF GpuProgram::msVTFCmd;
	GpuProgram::CmdManualNamedConstsFile GpuProgram::msManNamedConstsFileCmd;
	GpuProgram::CmdAdjacency GpuProgram::msAdjacencyCmd;
	

    //-----------------------------------------------------------------------------
    GpuProgram::GpuProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader) 
        :Resource(creator, name, handle, group, isManual, loader),
        mType(GPT_VERTEX_PROGRAM), mLoadFromFile(true), mSkeletalAnimation(false),
        mVertexTextureFetch(false), mNeedsAdjacencyInfo(false),
		mCompileError(false), mLoadedManualNamedConstants(false)
    {
		createParameterMappingStructures();
    }
    //-----------------------------------------------------------------------------
    void GpuProgram::setType(GpuProgramType t)
    {
        mType = t;
    }
    //-----------------------------------------------------------------------------
    void GpuProgram::setSyntaxCode(const String& syntax)
    {
        mSyntaxCode = syntax;
    }
    //-----------------------------------------------------------------------------
    void GpuProgram::setSourceFile(const String& filename)
    {
        mFilename = filename;
        mSource.clear();
        mLoadFromFile = true;
		mCompileError = false;
    }
    //-----------------------------------------------------------------------------
    void GpuProgram::setSource(const String& source)
    {
        mSource = source;
        mFilename.clear();
        mLoadFromFile = false;
		mCompileError = false;
    }

    //-----------------------------------------------------------------------------
    void GpuProgram::loadImpl(void)
    {
        if (mLoadFromFile)
        {
            // find & load source code
            DataStreamPtr stream = 
                ResourceGroupManager::getSingleton().openResource(
					mFilename, mGroup, true, this);
            mSource = stream->getAsString();
        }

        // Call polymorphic load
		try 
		{
			loadFromSource();

			if (!mDefaultParams.isNull())
			{
				// Keep a reference to old ones to copy
				GpuProgramParametersSharedPtr savedParams = mDefaultParams;
				// reset params to stop them being referenced in the next create
				mDefaultParams.setNull();

				// Create new params
				mDefaultParams = createParameters();

				// Copy old (matching) values across
				// Don't use copyConstantsFrom since program may be different
				mDefaultParams->copyMatchingNamedConstantsFrom(*savedParams.get());

			}
		}
		catch (const Exception&)
		{
			// will already have been logged
			LogManager::getSingleton().stream()
				<< "Gpu program " << mName << " encountered an error "
				<< "during loading and is thus not supported.";

			mCompileError = true;
		}

    }
    //-----------------------------------------------------------------------------
    bool GpuProgram::isRequiredCapabilitiesSupported(void) const
    {
		const RenderSystemCapabilities* caps = 
			Root::getSingleton().getRenderSystem()->getCapabilities();

        // If skeletal animation is being done, we need support for UBYTE4
        if (isSkeletalAnimationIncluded() && 
            !caps->hasCapability(RSC_VERTEX_FORMAT_UBYTE4))
        {
            return false;
        }

		// Vertex texture fetch required?
		if (isVertexTextureFetchRequired() && 
			!caps->hasCapability(RSC_VERTEX_TEXTURE_FETCH))
		{
			return false;
		}

        return true;
    }
    //-----------------------------------------------------------------------------
    bool GpuProgram::isSupported(void) const
    {
        if (mCompileError || !isRequiredCapabilitiesSupported())
            return false;

        return GpuProgramManager::getSingleton().isSyntaxSupported(mSyntaxCode);
    }
	//---------------------------------------------------------------------
	void GpuProgram::createParameterMappingStructures(bool recreateIfExists) const
	{
		createLogicalParameterMappingStructures(recreateIfExists);
		createNamedParameterMappingStructures(recreateIfExists);
	}
	//---------------------------------------------------------------------
	void GpuProgram::createLogicalParameterMappingStructures(bool recreateIfExists) const
	{
		if (recreateIfExists || mFloatLogicalToPhysical.isNull())
			mFloatLogicalToPhysical = GpuLogicalBufferStructPtr(OGRE_NEW GpuLogicalBufferStruct());
		if (recreateIfExists || mIntLogicalToPhysical.isNull())
			mIntLogicalToPhysical = GpuLogicalBufferStructPtr(OGRE_NEW GpuLogicalBufferStruct());
	}
	//---------------------------------------------------------------------
	void GpuProgram::createNamedParameterMappingStructures(bool recreateIfExists) const
	{
		if (recreateIfExists || mConstantDefs.isNull())
			mConstantDefs = GpuNamedConstantsPtr(OGRE_NEW GpuNamedConstants());
	}
	//---------------------------------------------------------------------
	void GpuProgram::setManualNamedConstantsFile(const String& paramDefFile)
	{
		mManualNamedConstantsFile = paramDefFile;
		mLoadedManualNamedConstants = false;
	}
	//---------------------------------------------------------------------
	void GpuProgram::setManualNamedConstants(const GpuNamedConstants& namedConstants)
	{
		createParameterMappingStructures();
		*mConstantDefs.get() = namedConstants;

		mFloatLogicalToPhysical->bufferSize = mConstantDefs->floatBufferSize;
		mIntLogicalToPhysical->bufferSize = mConstantDefs->intBufferSize;
		mFloatLogicalToPhysical->map.clear();
		mIntLogicalToPhysical->map.clear();
		// need to set up logical mappings too for some rendersystems
		for (GpuConstantDefinitionMap::const_iterator i = mConstantDefs->map.begin();
			i != mConstantDefs->map.end(); ++i)
		{
			const String& name = i->first;
			const GpuConstantDefinition& def = i->second;
			// only consider non-array entries
			if (name.find("[") == String::npos)
			{
				GpuLogicalIndexUseMap::value_type val(def.logicalIndex, 
					GpuLogicalIndexUse(def.physicalIndex, def.arraySize * def.elementSize, def.variability));
				if (def.isFloat())
				{
					mFloatLogicalToPhysical->map.insert(val);
				}
				else
				{
					mIntLogicalToPhysical->map.insert(val);
				}
			}
		}


	}
    //-----------------------------------------------------------------------------
    GpuProgramParametersSharedPtr GpuProgram::createParameters(void)
    {
        // Default implementation simply returns standard parameters.
        GpuProgramParametersSharedPtr ret = 
            GpuProgramManager::getSingleton().createParameters();
		
		
		// optionally load manually supplied named constants
		if (!mManualNamedConstantsFile.empty() && !mLoadedManualNamedConstants)
		{
			try 
			{
				GpuNamedConstants namedConstants;
				DataStreamPtr stream = 
					ResourceGroupManager::getSingleton().openResource(
					mManualNamedConstantsFile, mGroup, true, this);
				namedConstants.load(stream);
				setManualNamedConstants(namedConstants);
			}
			catch(const Exception& e)
			{
				LogManager::getSingleton().stream() <<
					"Unable to load manual named constants for GpuProgram " << mName <<
					": " << e.getDescription();
			}
			mLoadedManualNamedConstants = true;
		}
		
		
		// set up named parameters, if any
		if (!mConstantDefs.isNull() && !mConstantDefs->map.empty())
		{
			ret->_setNamedConstants(mConstantDefs);
		}
		// link shared logical / physical map for low-level use
		ret->_setLogicalIndexes(mFloatLogicalToPhysical, mIntLogicalToPhysical);

        // Copy in default parameters if present
        if (!mDefaultParams.isNull())
            ret->copyConstantsFrom(*(mDefaultParams.get()));
        
        return ret;
    }
    //-----------------------------------------------------------------------------
    GpuProgramParametersSharedPtr GpuProgram::getDefaultParameters(void)
    {
        if (mDefaultParams.isNull())
        {
            mDefaultParams = createParameters();
        }
        return mDefaultParams;
    }
    //-----------------------------------------------------------------------------
    void GpuProgram::setupBaseParamDictionary(void)
    {
        ParamDictionary* dict = getParamDictionary();

        dict->addParameter(
            ParameterDef("type", "'vertex_program', 'geometry_program' or 'fragment_program'",
                PT_STRING), &msTypeCmd);
        dict->addParameter(
            ParameterDef("syntax", "Syntax code, e.g. vs_1_1", PT_STRING), &msSyntaxCmd);
        dict->addParameter(
            ParameterDef("includes_skeletal_animation", 
            "Whether this vertex program includes skeletal animation", PT_BOOL), 
            &msSkeletalCmd);
		dict->addParameter(
			ParameterDef("includes_morph_animation", 
			"Whether this vertex program includes morph animation", PT_BOOL), 
			&msMorphCmd);
		dict->addParameter(
			ParameterDef("includes_pose_animation", 
			"The number of poses this vertex program supports for pose animation", PT_INT), 
			&msPoseCmd);
		dict->addParameter(
			ParameterDef("uses_vertex_texture_fetch", 
			"Whether this vertex program requires vertex texture fetch support.", PT_BOOL), 
			&msVTFCmd);
		dict->addParameter(
			ParameterDef("manual_named_constants", 
			"File containing named parameter mappings for low-level programs.", PT_BOOL), 
			&msManNamedConstsFileCmd);
		dict->addParameter(
			ParameterDef("uses_adjacency_information",
			"Whether this geometry program requires adjacency information from the input primitives.", PT_BOOL),
			&msAdjacencyCmd);
    }

    //-----------------------------------------------------------------------
    const String& GpuProgram::getLanguage(void) const
    {
        static const String language = "asm";

        return language;
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String GpuProgram::CmdType::doGet(const void* target) const
    {
        const GpuProgram* t = static_cast<const GpuProgram*>(target);
        if (t->getType() == GPT_VERTEX_PROGRAM)
        {
            return "vertex_program";
        }
		else if (t->getType() == GPT_GEOMETRY_PROGRAM)
		{
			return "geometry_program";
		}
		else
        {
            return "fragment_program";
        }
    }
    void GpuProgram::CmdType::doSet(void* target, const String& val)
    {
        GpuProgram* t = static_cast<GpuProgram*>(target);
        if (val == "vertex_program")
        {
            t->setType(GPT_VERTEX_PROGRAM);
        }
        else if (val == "geometry_program")
		{
			t->setType(GPT_GEOMETRY_PROGRAM);
		}
		else
        {
            t->setType(GPT_FRAGMENT_PROGRAM);
        }
    }
    //-----------------------------------------------------------------------
    String GpuProgram::CmdSyntax::doGet(const void* target) const
    {
        const GpuProgram* t = static_cast<const GpuProgram*>(target);
        return t->getSyntaxCode();
    }
    void GpuProgram::CmdSyntax::doSet(void* target, const String& val)
    {
        GpuProgram* t = static_cast<GpuProgram*>(target);
        t->setSyntaxCode(val);
    }
    //-----------------------------------------------------------------------
    String GpuProgram::CmdSkeletal::doGet(const void* target) const
    {
        const GpuProgram* t = static_cast<const GpuProgram*>(target);
        return StringConverter::toString(t->isSkeletalAnimationIncluded());
    }
    void GpuProgram::CmdSkeletal::doSet(void* target, const String& val)
    {
        GpuProgram* t = static_cast<GpuProgram*>(target);
        t->setSkeletalAnimationIncluded(StringConverter::parseBool(val));
    }
	//-----------------------------------------------------------------------
	String GpuProgram::CmdMorph::doGet(const void* target) const
	{
		const GpuProgram* t = static_cast<const GpuProgram*>(target);
		return StringConverter::toString(t->isMorphAnimationIncluded());
	}
	void GpuProgram::CmdMorph::doSet(void* target, const String& val)
	{
		GpuProgram* t = static_cast<GpuProgram*>(target);
		t->setMorphAnimationIncluded(StringConverter::parseBool(val));
	}
	//-----------------------------------------------------------------------
	String GpuProgram::CmdPose::doGet(const void* target) const
	{
		const GpuProgram* t = static_cast<const GpuProgram*>(target);
		return StringConverter::toString(t->getNumberOfPosesIncluded());
	}
	void GpuProgram::CmdPose::doSet(void* target, const String& val)
	{
		GpuProgram* t = static_cast<GpuProgram*>(target);
		t->setPoseAnimationIncluded(StringConverter::parseUnsignedInt(val));
	}
	//-----------------------------------------------------------------------
	String GpuProgram::CmdVTF::doGet(const void* target) const
	{
		const GpuProgram* t = static_cast<const GpuProgram*>(target);
		return StringConverter::toString(t->isVertexTextureFetchRequired());
	}
	void GpuProgram::CmdVTF::doSet(void* target, const String& val)
	{
		GpuProgram* t = static_cast<GpuProgram*>(target);
		t->setVertexTextureFetchRequired(StringConverter::parseBool(val));
	}
	//-----------------------------------------------------------------------
	String GpuProgram::CmdManualNamedConstsFile::doGet(const void* target) const
	{
		const GpuProgram* t = static_cast<const GpuProgram*>(target);
		return t->getManualNamedConstantsFile();
	}
	void GpuProgram::CmdManualNamedConstsFile::doSet(void* target, const String& val)
	{
		GpuProgram* t = static_cast<GpuProgram*>(target);
		t->setManualNamedConstantsFile(val);
	}
    //-----------------------------------------------------------------------
	String GpuProgram::CmdAdjacency::doGet(const void* target) const
	{
		const GpuProgram* t = static_cast<const GpuProgram*>(target);
		return StringConverter::toString(t->isAdjacencyInfoRequired());
	}
	void GpuProgram::CmdAdjacency::doSet(void* target, const String& val)
	{
		GpuProgram* t = static_cast<GpuProgram*>(target);
		t->setAdjacencyInfoRequired(StringConverter::parseBool(val));
	}
    //-----------------------------------------------------------------------
    GpuProgramPtr& GpuProgramPtr::operator=(const HighLevelGpuProgramPtr& r)
    {
        // Can assign direct
        if (pRep == r.getPointer())
            return *this;
        release();
		// lock & copy other mutex pointer
        OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
        {
		    OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
		    OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
            pRep = r.getPointer();
            pUseCount = r.useCountPointer();
            if (pUseCount)
            {
                ++(*pUseCount);
            }
        }
		else
		{
			// RHS must be a null pointer
			assert(r.isNull() && "RHS must be null if it has no mutex!");
			setNull();
		}
        return *this;
    }

}
