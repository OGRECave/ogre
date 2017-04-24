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

#include "OgreHlmsCompute.h"
#include "OgreHlmsComputeJob.h"

#include "OgreHighLevelGpuProgramManager.h"
#include "OgreHighLevelGpuProgram.h"

#include "OgreSceneManager.h"
#include "Compositor/OgreCompositorShadowNode.h"

#include "CommandBuffer/OgreCommandBuffer.h"

#include "Vao/OgreConstBufferPacked.h"
#include "Vao/OgreTexBufferPacked.h"
#include "Vao/OgreUavBufferPacked.h"

#include "OgreLogManager.h"

#include "Hash/MurmurHash3.h"

#if OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_32
        #define OGRE_HASH128_FUNC MurmurHash3_x86_128
#else
        #define OGRE_HASH128_FUNC MurmurHash3_x64_128
#endif

namespace Ogre
{
    const IdString ComputeProperty::ThreadsPerGroupX    = IdString( "threads_per_group_x" );
    const IdString ComputeProperty::ThreadsPerGroupY    = IdString( "threads_per_group_y" );
    const IdString ComputeProperty::ThreadsPerGroupZ    = IdString( "threads_per_group_z" );
    const IdString ComputeProperty::NumThreadGroupsX    = IdString( "num_thread_groups_x" );
    const IdString ComputeProperty::NumThreadGroupsY    = IdString( "num_thread_groups_y" );
    const IdString ComputeProperty::NumThreadGroupsZ    = IdString( "num_thread_groups_z" );

    const IdString ComputeProperty::NumTextureSlots     = IdString( "num_texture_slots" );
    const IdString ComputeProperty::MaxTextureSlot      = IdString( "max_texture_slot" );
    const char *ComputeProperty::Texture                = "texture";

    const IdString ComputeProperty::NumUavSlots         = IdString( "num_uav_slots" );
    const IdString ComputeProperty::MaxUavSlot          = IdString( "max_uav_slot" );
    const char *ComputeProperty::Uav                    = "uav";

    //Must be sorted from best to worst
    const String BestD3DComputeShaderTargets[3] =
    {
        "cs_5_0", "cs_4_1", "cs_4_0"
    };

    HlmsCompute::HlmsCompute( AutoParamDataSource *autoParamDataSource ) :
        Hlms( HLMS_COMPUTE, "compute", 0, 0 ),
        mAutoParamDataSource( autoParamDataSource ),
        mComputeShaderTarget( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    HlmsCompute::~HlmsCompute()
    {
        destroyAllComputeJobs();
    }
    //-----------------------------------------------------------------------------------
    void HlmsCompute::_changeRenderSystem( RenderSystem *newRs )
    {
        Hlms::_changeRenderSystem( newRs );

        if( mRenderSystem )
        {
            //Prefer glsl over glsles
            const String shaderProfiles[3] = { "hlsl", "glsles", "glsl" };
            const RenderSystemCapabilities *capabilities = mRenderSystem->getCapabilities();

            for( size_t i=0; i<3; ++i )
            {
                if( capabilities->isShaderProfileSupported( shaderProfiles[i] ) )
                    mShaderProfile = shaderProfiles[i];
            }

            if( mShaderProfile == "hlsl" )
            {
                for( size_t j=0; j<3 && !mComputeShaderTarget; ++j )
                {
                    if( capabilities->isShaderProfileSupported( BestD3DComputeShaderTargets[j] ) )
                        mComputeShaderTarget = &BestD3DComputeShaderTargets[j];
                }
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsCompute::processPieces( const StringVector &pieceFiles )
    {
        ResourceGroupManager &resourceGroupMgr = ResourceGroupManager::getSingleton();

        StringVector::const_iterator itor = pieceFiles.begin();
        StringVector::const_iterator end  = pieceFiles.end();

        while( itor != end )
        {
            String filename = *itor;

            //If it has an explicit extension, only open it if it matches the current
            //render system's. If it doesn't, then we add it ourselves.
            String::size_type pos = filename.find_last_of( '.' );
            if( pos == String::npos ||
                (filename.compare( pos + 1, String::npos, mShaderFileExt ) != 0 &&
                 filename.compare( pos + 1, String::npos, "metal" ) != 0 &&
                 filename.compare( pos + 1, String::npos, "glsl" ) != 0 &&
                 filename.compare( pos + 1, String::npos, "glsles" ) != 0 &&
                 filename.compare( pos + 1, String::npos, "hlsl" ) != 0) )
            {
                filename += mShaderFileExt;
            }

            DataStreamPtr inFile = resourceGroupMgr.openResource( filename );

            String inString;
            String outString;

            inString.resize(inFile->size());
            inFile->read(&inString[0], inFile->size());

            this->parseMath(inString, outString);
            while( outString.find( "@foreach" ) != String::npos )
            {
                this->parseForEach(outString, inString);
                inString.swap( outString );
            }
            this->parseProperties(outString, inString);
            this->parseUndefPieces(inString, outString);
            this->collectPieces(outString, inString);
            this->parseCounter(inString, outString);

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    HlmsComputePso HlmsCompute::compileShader( HlmsComputeJob *job, uint32 finalHash )
    {
        //Assumes mSetProperties is already set
        //mSetProperties.clear();
        {
            //Add RenderSystem-specific properties
            IdStringVec::const_iterator itor = mRsSpecificExtensions.begin();
            IdStringVec::const_iterator end  = mRsSpecificExtensions.end();

            while( itor != end )
                setProperty( *itor++, 1 );
        }

        GpuProgramPtr shader;
        //Generate the shader

        //Collect pieces
        mPieces.clear();

        //Start with the pieces sent by the user
        mPieces = job->mPieces;

        const String sourceFilename = job->mSourceFilename + mShaderFileExt;

        ResourceGroupManager &resourceGroupMgr = ResourceGroupManager::getSingleton();
        DataStreamPtr inFile = resourceGroupMgr.openResource( sourceFilename );

        if( mShaderProfile == "glsl" ) //TODO: String comparision
            setProperty( HlmsBaseProp::GL3Plus, 330 );

        setProperty( HlmsBaseProp::HighQuality, mHighQuality );

        //Piece files
        processPieces( job->mIncludedPieceFiles );

        String inString;
        String outString;

        inString.resize( inFile->size() );
        inFile->read( &inString[0], inFile->size() );

        bool syntaxError = false;

        syntaxError |= this->parseMath( inString, outString );
        while( !syntaxError && outString.find( "@foreach" ) != String::npos )
        {
            syntaxError |= this->parseForEach( outString, inString );
            inString.swap( outString );
        }
        syntaxError |= this->parseProperties( outString, inString );
        syntaxError |= this->parseUndefPieces( inString, outString );
        while( !syntaxError  && (outString.find( "@piece" ) != String::npos ||
                                 outString.find( "@insertpiece" ) != String::npos) )
        {
            syntaxError |= this->collectPieces( outString, inString );
            syntaxError |= this->insertPieces( inString, outString );
        }
        syntaxError |= this->parseCounter( outString, inString );

        outString.swap( inString );

        if( syntaxError )
        {
            LogManager::getSingleton().logMessage( "There were HLMS syntax errors while parsing "
                                                   + StringConverter::toString( finalHash ) +
                                                   job->mSourceFilename + mShaderFileExt );
        }

        String debugFilenameOutput;

        if( mDebugOutput )
        {
            debugFilenameOutput = mOutputPath + "./" +
                                    StringConverter::toString( finalHash ) +
                                    job->mSourceFilename + mShaderFileExt;
            std::ofstream outFile( debugFilenameOutput.c_str(),
                                   std::ios::out | std::ios::binary );
            outFile.write( &outString[0], outString.size() );
        }

        //Don't create and compile if template requested not to
        if( !getProperty( HlmsBaseProp::DisableStage ) )
        {
            //Very similar to what the GpuProgramManager does with its microcode cache,
            //but we **need** to know if two Compute Shaders share the same source code.
            Hash hashVal;
            OGRE_HASH128_FUNC( outString.c_str(), outString.size(), IdString::Seed, &hashVal );

            CompiledShaderMap::const_iterator itor = mCompiledShaderCache.find( hashVal );
            if( itor != mCompiledShaderCache.end() )
            {
                shader = itor->second;
            }
            else
            {
                HighLevelGpuProgramManager *gpuProgramManager =
                        HighLevelGpuProgramManager::getSingletonPtr();

                HighLevelGpuProgramPtr gp = gpuProgramManager->createProgram(
                            StringConverter::toString( finalHash ) + job->mSourceFilename,
                            ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME,
                            mShaderProfile, GPT_COMPUTE_PROGRAM );
                gp->setSource( outString, debugFilenameOutput );

                if( mComputeShaderTarget )
                {
                    //D3D-specific
                    gp->setParameter( "target", *mComputeShaderTarget );
                    gp->setParameter( "entry_point", "main" );
                }

                gp->setSkeletalAnimationIncluded( getProperty( HlmsBaseProp::Skeleton ) != 0 );
                gp->setMorphAnimationIncluded( false );
                gp->setPoseAnimationIncluded( getProperty( HlmsBaseProp::Pose ) );
                gp->setVertexTextureFetchRequired( false );

                gp->load();

                shader = gp;

                mCompiledShaderCache[hashVal] = shader;
            }
        }

        //Reset the disable flag.
        setProperty( HlmsBaseProp::DisableStage, 0 );

        HlmsComputePso pso;
        pso.initialize();
        pso.computeShader = shader;
        pso.computeParams = shader->createParameters();
        pso.mThreadsPerGroup[0] = getProperty( ComputeProperty::ThreadsPerGroupX );
        pso.mThreadsPerGroup[1] = getProperty( ComputeProperty::ThreadsPerGroupY );
        pso.mThreadsPerGroup[2] = getProperty( ComputeProperty::ThreadsPerGroupZ );
        pso.mNumThreadGroups[0] = getProperty( ComputeProperty::NumThreadGroupsX );
        pso.mNumThreadGroups[1] = getProperty( ComputeProperty::NumThreadGroupsY );
        pso.mNumThreadGroups[2] = getProperty( ComputeProperty::NumThreadGroupsZ );

        if( pso.mThreadsPerGroup[0] * pso.mThreadsPerGroup[1] * pso.mThreadsPerGroup[2] == 0u ||
            pso.mNumThreadGroups[0] * pso.mNumThreadGroups[1] * pso.mNumThreadGroups[2] == 0u )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "Shader or C++ must set threads_per_group_x, threads_per_group_y & "
                         "threads_per_group_z and num_thread_groups_x through num_thread_groups_z."
                         " Otherwise we can't run on Metal. Use @pset( threads_per_group_x, 512 );"
                         " or read the value using @value( threads_per_group_x ) if you've already"
                         " set it from C++ or the JSON material", "HlmsCompute::compileShader" );
        }

        ShaderParams *shaderParams = job->_getShaderParams( "default" );
        if( shaderParams )
            shaderParams->updateParameters( pso.computeParams, true );

        shaderParams = job->_getShaderParams( mShaderProfile );
        if( shaderParams )
            shaderParams->updateParameters( pso.computeParams, true );

        mRenderSystem->_hlmsComputePipelineStateObjectCreated( &pso );

        return pso;
    }
    //-----------------------------------------------------------------------------------
    void HlmsCompute::destroyComputeJob( IdString name )
    {
        HlmsComputeJobMap::iterator itor = mComputeJobs.find( name );

        if( itor != mComputeJobs.end() )
        {
            OGRE_DELETE itor->second.computeJob;
            mComputeJobs.erase( itor );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsCompute::destroyAllComputeJobs()
    {
        HlmsComputeJobMap::const_iterator itor = mComputeJobs.begin();
        HlmsComputeJobMap::const_iterator end  = mComputeJobs.end();

        while( itor != end )
        {
            OGRE_DELETE itor->second.computeJob;
            ++itor;
        }

        mComputeJobs.clear();
    }
    //-----------------------------------------------------------------------------------
    void HlmsCompute::clearShaderCache(void)
    {
        ComputePsoCacheVec::iterator itor = mComputeShaderCache.begin();
        ComputePsoCacheVec::iterator end  = mComputeShaderCache.end();

        while( itor != end )
        {
            mRenderSystem->_hlmsComputePipelineStateObjectDestroyed( &itor->pso );
            itor->job->mPsoCacheHash = -1;
            ++itor;
        }

        Hlms::clearShaderCache();
        mCompiledShaderCache.clear();
        mComputeShaderCache.clear();
    }
    //-----------------------------------------------------------------------------------
    void HlmsCompute::dispatch( HlmsComputeJob *job, SceneManager *sceneManager, Camera *camera )
    {
        job->_calculateNumThreadGroupsBasedOnSetting();

        if( job->mPsoCacheHash >= mComputeShaderCache.size() )
        {
            //Potentially needs to recompile.
            job->_updateAutoProperties();

            ComputePsoCache psoCache;
            psoCache.job = job;
            //To perform the search, temporarily borrow the properties to avoid an allocation & a copy.
            psoCache.setProperties.swap( job->mSetProperties );
            ComputePsoCacheVec::const_iterator itor = std::find( mComputeShaderCache.begin(),
                                                                 mComputeShaderCache.end(),
                                                                 psoCache );
            if( itor == mComputeShaderCache.end() )
            {
                //Needs to recompile.

                //Return back the borrowed properties and make
                //a hard copy for starting the compilation.
                psoCache.setProperties.swap( job->mSetProperties );
                this->mSetProperties = job->mSetProperties;

                //Compile and add the PSO to the cache.
                psoCache.pso = compileShader( job, mComputeShaderCache.size() );

                ShaderParams *shaderParams = job->_getShaderParams( "default" );
                if( shaderParams )
                    psoCache.paramsUpdateCounter = shaderParams->getUpdateCounter();
                if( shaderParams )
                    psoCache.paramsProfileUpdateCounter = shaderParams->getUpdateCounter();

                mComputeShaderCache.push_back( psoCache );

                //The PSO in the cache doesn't have the properties. Make a hard copy.
                //We can use this->mSetProperties as it may have been modified during
                //compilerShader by the template.
                mComputeShaderCache.back().setProperties = job->mSetProperties;

                job->mPsoCacheHash = mComputeShaderCache.size() - 1u;
            }
            else
            {
                //It was already in the cache. Return back the borrowed
                //properties and set the proper index to the cache.
                psoCache.setProperties.swap( job->mSetProperties );
                job->mPsoCacheHash = itor - mComputeShaderCache.begin();
            }
        }

        ComputePsoCache &psoCache = mComputeShaderCache[job->mPsoCacheHash];

        {
            //Update dirty parameters, if necessary
            ShaderParams *shaderParams = job->_getShaderParams( "default" );
            if( shaderParams && psoCache.paramsUpdateCounter != shaderParams->getUpdateCounter() )
            {
                shaderParams->updateParameters( psoCache.pso.computeParams, false );
                psoCache.paramsUpdateCounter = shaderParams->getUpdateCounter();
            }

            shaderParams = job->_getShaderParams( mShaderProfile );
            if( shaderParams && psoCache.paramsProfileUpdateCounter != shaderParams->getUpdateCounter() )
            {
                shaderParams->updateParameters( psoCache.pso.computeParams, false );
                psoCache.paramsProfileUpdateCounter = shaderParams->getUpdateCounter();
            }
        }

        mRenderSystem->_setComputePso( &psoCache.pso );

        HlmsComputeJob::ConstBufferSlotVec::const_iterator itConst =
                job->mConstBuffers.begin();
        HlmsComputeJob::ConstBufferSlotVec::const_iterator enConst =
                job->mConstBuffers.end();

        while( itConst != enConst )
        {
            itConst->buffer->bindBufferCS( itConst->slotIdx );
            ++itConst;
        }

        uint32 slotIdx = 0u;
        HlmsComputeJob::TextureSlotVec::const_iterator itTex = job->mTextureSlots.begin();
        HlmsComputeJob::TextureSlotVec::const_iterator enTex = job->mTextureSlots.end();

        while( itTex != enTex )
        {
            if( itTex->buffer )
            {
                static_cast<TexBufferPacked*>( itTex->buffer )->bindBufferCS(
                            slotIdx, itTex->offset, itTex->sizeBytes );
            }
            else
            {
                mRenderSystem->_setTextureCS( slotIdx, !itTex->texture.isNull(), itTex->texture.get() );
                if( itTex->samplerblock )
                    mRenderSystem->_setHlmsSamplerblockCS( slotIdx, itTex->samplerblock );
            }

            ++slotIdx;
            ++itTex;
        }

        slotIdx = 0u;
        HlmsComputeJob::TextureSlotVec::const_iterator itUav = job->mUavSlots.begin();
        HlmsComputeJob::TextureSlotVec::const_iterator enUav = job->mUavSlots.end();

        while( itUav != enUav )
        {
            if( itUav->buffer )
            {
                static_cast<UavBufferPacked*>( itUav->buffer )->bindBufferCS(
                            slotIdx, itUav->offset, itUav->sizeBytes );
            }
            else
            {
                mRenderSystem->_bindTextureUavCS( slotIdx, itUav->texture.get(),
                                                  itUav->access, itUav->mipmapLevel,
                                                  itUav->textureArrayIndex, itUav->pixelFormat );
            }

            ++slotIdx;
            ++itUav;
        }

        mAutoParamDataSource->setCurrentJob( job );
        mAutoParamDataSource->setCurrentCamera( camera );
        mAutoParamDataSource->setCurrentSceneManager( sceneManager );
        //mAutoParamDataSource->setCurrentShadowNode( shadowNode );
        //mAutoParamDataSource->setCurrentViewport( sceneManager->getCurrentViewport() );

        GpuProgramParametersSharedPtr csParams = psoCache.pso.computeParams;
        csParams->_updateAutoParams( mAutoParamDataSource, GPV_ALL );
        mRenderSystem->bindGpuProgramParameters( GPT_COMPUTE_PROGRAM, csParams, GPV_ALL );

        mRenderSystem->_dispatch( psoCache.pso );
    }
    //----------------------------------------------------------------------------------
    HlmsDatablock* HlmsCompute::createDatablockImpl( IdString datablockName,
                                                     const HlmsMacroblock *macroblock,
                                                     const HlmsBlendblock *blendblock,
                                                     const HlmsParamVec &paramVec )
    {
        return 0;
    }
    //----------------------------------------------------------------------------------
    void HlmsCompute::reloadFrom( Archive *newDataFolder, ArchiveVec *libraryFolders )
    {
        Hlms::reloadFrom( newDataFolder, libraryFolders );

        HlmsComputeJobMap::const_iterator itor = mComputeJobs.begin();
        HlmsComputeJobMap::const_iterator end  = mComputeJobs.end();

        while( itor != end )
        {
            HlmsComputeJob *job = itor->second.computeJob;
            map<IdString, ShaderParams>::type::iterator it = job->mShaderParams.begin();
            map<IdString, ShaderParams>::type::iterator en = job->mShaderParams.end();

            while( it != en )
            {
                ShaderParams &shaderParams = it->second;
                ShaderParams::ParamVec::iterator itParam = shaderParams.mParams.begin();
                ShaderParams::ParamVec::iterator enParam = shaderParams.mParams.end();

                while( itParam != enParam )
                {
                    itParam->isDirty = true;
                    ++itParam;
                }

                shaderParams.setDirty();
                ++it;
            }

            ++itor;
        }
    }
    //----------------------------------------------------------------------------------
    HlmsComputeJob* HlmsCompute::createComputeJob( IdString datablockName, const String &refName,
                                                   const String &sourceFilename,
                                                   const StringVector &includedPieceFiles )
    {
        HlmsComputeJob *retVal = OGRE_NEW HlmsComputeJob( datablockName, this,
                                                          sourceFilename, includedPieceFiles );
        mComputeJobs[datablockName] = ComputeJobEntry( retVal, refName );

        return retVal;
    }
    //----------------------------------------------------------------------------------
    HlmsComputeJob* HlmsCompute::findComputeJob( IdString datablockName ) const
    {
        HlmsComputeJob *retVal = findComputeJobNoThrow( datablockName );

        if( !retVal )
        {
            OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND,
                         "Compute Job with name " + datablockName.getFriendlyText() + " not found",
                         "HlmsCompute::findComputeJob" );
        }

        return retVal;
    }
    //----------------------------------------------------------------------------------
    HlmsComputeJob* HlmsCompute::findComputeJobNoThrow( IdString datablockName ) const
    {
        HlmsComputeJob *retVal = 0;

        HlmsComputeJobMap::const_iterator itor = mComputeJobs.find( datablockName );
        if( itor != mComputeJobs.end() )
            retVal = itor->second.computeJob;

        return retVal;
    }
    //----------------------------------------------------------------------------------
    const String* HlmsCompute::getJobNameStr( IdString name ) const
    {
        String const *retVal = 0;
        HlmsComputeJobMap::const_iterator itor = mComputeJobs.find( name );
        if( itor != mComputeJobs.end() )
            retVal = &itor->second.name;

        return retVal;
    }
    //----------------------------------------------------------------------------------
    HlmsDatablock* HlmsCompute::createDefaultDatablock(void)
    {
        return 0;
    }
    //----------------------------------------------------------------------------------
    uint32 HlmsCompute::fillBuffersFor( const HlmsCache *cache, const QueuedRenderable &queuedRenderable,
                                   bool casterPass, uint32 lastCacheHash,
                                   uint32 lastTextureHash )
    {
        OGRE_EXCEPT( Exception::ERR_INVALID_CALL, "This is a Compute Hlms",
                     "HlmsCompute::fillBuffersFor" );
    }
    uint32 HlmsCompute::fillBuffersForV1( const HlmsCache *cache,
                                     const QueuedRenderable &queuedRenderable,
                                     bool casterPass, uint32 lastCacheHash,
                                     CommandBuffer *commandBuffer )
    {
        OGRE_EXCEPT( Exception::ERR_INVALID_CALL, "This is a Compute Hlms",
                     "HlmsCompute::fillBuffersForV1" );
    }
    uint32 HlmsCompute::fillBuffersForV2( const HlmsCache *cache,
                                     const QueuedRenderable &queuedRenderable,
                                     bool casterPass, uint32 lastCacheHash,
                                     CommandBuffer *commandBuffer )
    {
        OGRE_EXCEPT( Exception::ERR_INVALID_CALL, "This is a Compute Hlms",
                     "HlmsCompute::fillBuffersForV2" );
    }
}

#undef OGRE_HASH128_FUNC
