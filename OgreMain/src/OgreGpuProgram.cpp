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
#include "OgreGpuProgram.h"
#include "OgreGpuProgramManager.h"
#include "OgreRenderSystemCapabilities.h"

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
    GpuProgram::CmdComputeGroupDims GpuProgram::msComputeGroupDimsCmd;
    

    GpuLogicalBufferStructPtr GpuProgram::mBoolLogicalToPhysical;

    //-----------------------------------------------------------------------------
    GpuProgram::GpuProgram(ResourceManager* creator, const String& name, ResourceHandle handle, const String& group,
                           bool isManual, ManualResourceLoader* loader)
        : Resource(creator, name, handle, group, isManual, loader), mType(GPT_VERTEX_PROGRAM), mLoadFromFile(true),
          mSkeletalAnimation(false), mMorphAnimation(false), mVertexTextureFetch(false), mNeedsAdjacencyInfo(false),
          mCompileError(false), mPoseAnimation(0), mLoadedManualNamedConstants(false)
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

    uint32 GpuProgram::_getHash(uint32 seed) const
    {
        // include filename as same source can be used with different defines & entry points
        uint32 hash = FastHash(mName.c_str(), mName.size(), seed);
        return FastHash(mSource.c_str(), mSource.size(), hash);
    }

    size_t GpuProgram::calculateSize(void) const
    {
        size_t memSize = sizeof(*this);
        memSize += mManualNamedConstantsFile.size() * sizeof(char);
        memSize += mFilename.size() * sizeof(char);
        memSize += mSource.size() * sizeof(char);
        memSize += mSyntaxCode.size() * sizeof(char);

        size_t paramsSize = 0;
        if(mDefaultParams)
            paramsSize += mDefaultParams->calculateSize();
        if(mFloatLogicalToPhysical)
            paramsSize += mFloatLogicalToPhysical->bufferSize;
        if(mDoubleLogicalToPhysical)
            paramsSize += mDoubleLogicalToPhysical->bufferSize;
        if(mIntLogicalToPhysical)
            paramsSize += mIntLogicalToPhysical->bufferSize;
        if(mConstantDefs)
            paramsSize += mConstantDefs->calculateSize();

        return memSize + paramsSize;
    }
    //-----------------------------------------------------------------------------
    void GpuProgram::prepareImpl()
    {
        if (!mLoadFromFile)
            return;

        mSource = ResourceGroupManager::getSingleton().openResource(mFilename, mGroup, this)->getAsString();
    }

    void GpuProgram::safePrepare()
    {
        try
        {
            prepare();
        }
        catch (const RuntimeAssertionException&)
        {
            throw;
        }
        catch (const Exception& e)
        {
            // will already have been logged
            LogManager::getSingleton().stream(LML_CRITICAL)
                << "Program '" << mName << "' is not supported: " << e.getDescription();

            mCompileError = true;
        }
    }

    void GpuProgram::loadImpl(void)
    {
        if(mCompileError)
            return;

        // Call polymorphic load
        try 
        {
            loadFromSource();
        }
        catch (const RuntimeAssertionException&)
        {
            throw;
        }
        catch (const Exception& e)
        {
            // will already have been logged
            LogManager::getSingleton().stream(LML_CRITICAL)
                << "Program '" << mName << "' is not supported: " << e.getDescription();

            mCompileError = true;
        }
    }
    void GpuProgram::postLoadImpl()
    {
        if (!mDefaultParams || mCompileError)
            return;

        // Keep a reference to old ones to copy
        GpuProgramParametersSharedPtr savedParams = mDefaultParams;
        // reset params to stop them being referenced in the next create
        mDefaultParams.reset();

        // Create new params
        mDefaultParams = createParameters();

        // Copy old (matching) values across
        // Don't use copyConstantsFrom since program may be different
        mDefaultParams->copyMatchingNamedConstantsFrom(*savedParams.get());
    }

    //-----------------------------------------------------------------------------
    bool GpuProgram::isRequiredCapabilitiesSupported(void) const
    {
        const RenderSystemCapabilities* caps = 
            Root::getSingleton().getRenderSystem()->getCapabilities();

        // Basic support check
        if ((getType() == GPT_GEOMETRY_PROGRAM && !caps->hasCapability(RSC_GEOMETRY_PROGRAM)) ||
            (getType() == GPT_DOMAIN_PROGRAM && !caps->hasCapability(RSC_TESSELLATION_DOMAIN_PROGRAM)) ||
            (getType() == GPT_HULL_PROGRAM && !caps->hasCapability(RSC_TESSELLATION_HULL_PROGRAM)) ||
            (getType() == GPT_COMPUTE_PROGRAM && !caps->hasCapability(RSC_COMPUTE_PROGRAM)))
        {
            return false;
        }

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
        if (recreateIfExists || !mFloatLogicalToPhysical)
            mFloatLogicalToPhysical = GpuLogicalBufferStructPtr(OGRE_NEW GpuLogicalBufferStruct());
        if (recreateIfExists || !mIntLogicalToPhysical)
            mIntLogicalToPhysical = GpuLogicalBufferStructPtr(OGRE_NEW GpuLogicalBufferStruct());
    }
    //---------------------------------------------------------------------
    void GpuProgram::createNamedParameterMappingStructures(bool recreateIfExists) const
    {
        if (recreateIfExists || !mConstantDefs)
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
            if (name.find('[') == String::npos)
            {
                GpuLogicalIndexUseMap::value_type val(def.logicalIndex, 
                    GpuLogicalIndexUse(def.physicalIndex, def.arraySize * def.elementSize, def.variability));
                if (def.isFloat())
                {
                    mFloatLogicalToPhysical->map.emplace(val);
                }
                else
                {
                    mIntLogicalToPhysical->map.emplace(val);
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
                    mManualNamedConstantsFile, mGroup, this);
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
        if (mConstantDefs && !mConstantDefs->map.empty())
        {
            ret->_setNamedConstants(mConstantDefs);
        }
        // link shared logical / physical map for low-level use
        ret->_setLogicalIndexes(mFloatLogicalToPhysical, mDoubleLogicalToPhysical,
                                mIntLogicalToPhysical);

        // Copy in default parameters if present
        if (mDefaultParams)
            ret->copyConstantsFrom(*(mDefaultParams.get()));
        
        return ret;
    }
    //-----------------------------------------------------------------------------
    GpuProgramParametersSharedPtr GpuProgram::getDefaultParameters(void)
    {
        if (!mDefaultParams)
        {
            mDefaultParams = createParameters();
        }
        return mDefaultParams;
    }
    //-----------------------------------------------------------------------------
    const String GpuProgram::getProgramTypeName(GpuProgramType programType)
    {
        switch (programType)
        {
        case GPT_VERTEX_PROGRAM:
            return "vertex";
        case GPT_GEOMETRY_PROGRAM:
            return "geometry";
        case GPT_FRAGMENT_PROGRAM:
            return "fragment";
        case GPT_DOMAIN_PROGRAM:
            return "domain";
        case GPT_HULL_PROGRAM:
            return "hull";
        case GPT_COMPUTE_PROGRAM:
            return "compute";
        default:
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Unexpected GPU program type",
                "GpuProgram::GetName");
        }
    }
    //-----------------------------------------------------------------------------
    void GpuProgram::setupBaseParamDictionary(void)
    {
        ParamDictionary* dict = getParamDictionary();

        dict->addParameter(
            ParameterDef("type", "'vertex_program', 'geometry_program', 'fragment_program', 'hull_program', 'domain_program', 'compute_program'",
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
        dict->addParameter(
            ParameterDef("compute_group_dimensions",
                         "The number of process groups created by this compute program.", PT_VECTOR3),
            &msComputeGroupDimsCmd);
            
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
        else if (t->getType() == GPT_DOMAIN_PROGRAM)
        {
            return "domain_program";
        }
        else if (t->getType() == GPT_HULL_PROGRAM)
        {
            return "hull_program";
        }
        else if (t->getType() == GPT_COMPUTE_PROGRAM)
        {
            return "compute_program";
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
        else if (val == "domain_program")
        {
            t->setType(GPT_DOMAIN_PROGRAM);
        }
        else if (val == "hull_program")
        {
            t->setType(GPT_HULL_PROGRAM);
        }
        else if (val == "compute_program")
        {
            t->setType(GPT_COMPUTE_PROGRAM);
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
        t->setPoseAnimationIncluded((ushort)StringConverter::parseUnsignedInt(val));
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
        LogManager::getSingleton().logWarning("'uses_adjacency_information' is deprecated. "
        "Set the respective RenderOperation::OpertionType instead.");
        GpuProgram* t = static_cast<GpuProgram*>(target);
        t->mNeedsAdjacencyInfo = StringConverter::parseBool(val);
    }
    //-----------------------------------------------------------------------
    String GpuProgram::CmdComputeGroupDims::doGet(const void* target) const
    {
        const GpuProgram* t = static_cast<const GpuProgram*>(target);
        return StringConverter::toString(t->getComputeGroupDimensions());
    }
    void GpuProgram::CmdComputeGroupDims::doSet(void* target, const String& val)
    {
        GpuProgram* t = static_cast<GpuProgram*>(target);
        t->setComputeGroupDimensions(StringConverter::parseVector3(val));
    }
}

