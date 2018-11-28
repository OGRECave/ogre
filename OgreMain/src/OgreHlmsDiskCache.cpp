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

#include "OgreHlmsDiskCache.h"
#include "OgreHlmsManager.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreProfiler.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
    #include "iOS/macUtils.h"
#endif

namespace Ogre
{
    static const uint16 c_hlmsDiskCacheVersion = 0u;

    HlmsDiskCache::HlmsDiskCache( HlmsManager *hlmsManager ) :
        mTemplatesOutOfDate( false ),
        mHlmsManager( hlmsManager )
    {
    }
    //-----------------------------------------------------------------------------------
    HlmsDiskCache::~HlmsDiskCache()
    {
        clearCache();
    }
    //-----------------------------------------------------------------------------------
    void HlmsDiskCache::clearCache(void)
    {
        mTemplatesOutOfDate = false;
        memset( mCache.templateHash, 0, sizeof( mCache.templateHash ) );
        mCache.type = 255;
        mCache.sourceCode.clear();
        mCache.pso.clear();
        mShaderProfile.clear();
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    HlmsDiskCache::SourceCode::SourceCode() :
        mergedCache( HlmsPropertyVec(), 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    HlmsDiskCache::SourceCode::SourceCode( const Hlms::ShaderCodeCache &shaderCodeCache ) :
        mergedCache( shaderCodeCache.mergedCache )
    {
        for( size_t i=0; i<NumShaderTypes; ++i )
        {
            if( shaderCodeCache.shaders[i] )
                this->sourceFile[i] = shaderCodeCache.shaders[i]->getSource();
        }
    }
    //-----------------------------------------------------------------------------------
    HlmsDiskCache::Pso::Pso() :
        renderableCache( HlmsPropertyVec(), 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    HlmsDiskCache::Pso::Pso( const Hlms::RenderableCache &srcRenderableCache,
                             const Hlms::PassCache &srcPassCache, const HlmsCache *srcPsoCache ) :
        renderableCache( srcRenderableCache ),
        passProperties( srcPassCache.properties ),
        pso( srcPsoCache->pso ),
        macroblock( *srcPsoCache->pso.macroblock ),
        blendblock( *srcPsoCache->pso.blendblock )
    {
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    void HlmsDiskCache::copyFrom( Hlms *hlms )
    {
        clearCache();

        mCache.type = hlms->getType();
        mShaderProfile = hlms->getShaderProfile();
        hlms->getTemplateChecksum( mCache.templateHash );

        {
            //Copy shaders
            mCache.sourceCode.reserve( hlms->mShaderCodeCache.size() );
            Hlms::ShaderCodeCacheVec::const_iterator itor = hlms->mShaderCodeCache.begin();
            Hlms::ShaderCodeCacheVec::const_iterator end  = hlms->mShaderCodeCache.end();

            while( itor != end )
            {
                SourceCode sourceCode( *itor );
                mCache.sourceCode.push_back( sourceCode );
                ++itor;
            }
        }

        {
            //Copy PSOs
            mCache.pso.reserve( hlms->mShaderCache.size() );
            HlmsCacheVec::const_iterator itor = hlms->mShaderCache.begin();
            HlmsCacheVec::const_iterator end  = hlms->mShaderCache.end();

            while( itor != end )
            {
                const uint32 finalHash = (*itor)->hash;

                const uint32 renderableIdx  = (finalHash >> HlmsBits::RenderableShift) &
                                              (uint32)HlmsBits::RenderableMask;
                const uint32 passIdx        = (finalHash >> HlmsBits::PassShift) &
                                              (uint32)HlmsBits::PassMask;
//                const uint32 inputLayout    = (finalHash >> HlmsBits::InputLayoutShift) &
//                                              (uint32)HlmsBits::InputLayoutMask;
                Pso pso( hlms->mRenderableCache[renderableIdx], hlms->mPassCache[passIdx], *itor );
                mCache.pso.push_back( pso );
                ++itor;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsDiskCache::applyTo( Hlms *hlms )
    {
        LogManager::getSingleton().logMessage( "Applying HlmsDiskCache " +
                                               StringConverter::toString( hlms->getType() ) );

        if( mCache.type != hlms->getType() )
        {
            LogManager::getSingleton().logMessage(
                        "WARNING: The cached Hlms is for type " +
                        StringConverter::toString( mCache.type ) +
                        " but it is being applied to Hlms type: " +
                        StringConverter::toString( hlms->getType() ) +
                        ". HlmsDiskCache won't be applied." );
            return;
        }

        if( mShaderProfile != hlms->getShaderProfile() )
        {
            mTemplatesOutOfDate = true;
            LogManager::getSingleton().logMessage(
                        "INFO: The cached Hlms is for shader profile in '" + mShaderProfile +
                        "' but it does not match the current one '" + hlms->getShaderProfile() +
                        "'. This increases loading times." );
        }

        {
            uint64 currentHash[2];
            hlms->getTemplateChecksum( currentHash );
            if( mCache.templateHash[0] != currentHash[0] ||
                mCache.templateHash[1] != currentHash[1] )
            {
                mTemplatesOutOfDate = true;
                LogManager::getSingleton().logMessage(
                            "WARNING: The cached Hlms is out of date. The templates have changed. "
                            "We will parse the templates again. If you experience crashes or shader "
                            "compiler errors, delete the cache" );
            }
        }

        hlms->clearShaderCache();

        {
            //Compile shaders
            SourceCodeVec::const_iterator itor = mCache.sourceCode.begin();
            SourceCodeVec::const_iterator end  = mCache.sourceCode.end();

            while( itor != end )
            {
                if( !mTemplatesOutOfDate )
                {
                    //Templates haven't changed, send the Hlms-processed shader code for compilation
                    hlms->_compileShaderFromPreprocessedSource( itor->mergedCache, itor->sourceFile );
                }
                else
                {
                    //Templates have changed, they need to be run through the Hlms
                    //preprocessor again before they can be compiled again
                    Hlms::ShaderCodeCache shaderCodeCache( itor->mergedCache.pieces );
                    shaderCodeCache.mergedCache.setProperties = itor->mergedCache.setProperties;
                    hlms->compileShaderCode( shaderCodeCache );
                }
                ++itor;
            }
        }

        {
            hlms->mRenderableCache.reserve( hlms->mRenderableCache.size() + mCache.pso.size() );

            PsoVec::const_iterator itor = mCache.pso.begin();
            PsoVec::const_iterator end  = mCache.pso.end();

            while( itor != end )
            {
                uint32 renderableHash =
                        static_cast<uint32 >(
                            hlms->addRenderableCache( itor->renderableCache.setProperties,
                                                      itor->renderableCache.pieces ) );

                uint32 passHash = 0;
                {
                    assert( hlms->mPassCache.size() <= (uint32)HlmsBits::PassMask &&
                            "Too many passes combinations, we'll overflow "
                            "the bits assigned in the hash!" );

                    Hlms::PassCache passCache;
                    passCache.passPso       = itor->pso.pass;
                    passCache.properties    = itor->passProperties;

                    Hlms::PassCacheVec::iterator it = std::find( hlms->mPassCache.begin(),
                                                                 hlms->mPassCache.end(), passCache );
                    if( it == hlms->mPassCache.end() )
                    {
                        hlms->mPassCache.push_back( passCache );
                        it = hlms->mPassCache.end() - 1u;
                    }

                    passHash = (uint32)(it - hlms->mPassCache.begin()) << (uint32)HlmsBits::PassShift;
                }

                //const uint32 finalHash = renderableHash | passHash;
                //hlms->createShaderCacheEntry( renderableHash, passHash, finalHash, );

                ++itor;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    template <typename T> void write( DataStreamPtr &dataStream, const T &value )
    {
        dataStream->write( &value, sizeof(value) );
    }
    //-----------------------------------------------------------------------------------
    void HlmsDiskCache::save( DataStreamPtr &dataStream, const IdString &hashedString )
    {
        write( dataStream, hashedString.mHash );
#if OGRE_DEBUG_STR_SIZE > 0
        const size_t strLength = strnlen( hashedString.mDebugString, OGRE_DEBUG_STR_SIZE );
        write<uint16>( dataStream, static_cast<uint16>( strLength ) );
        dataStream->write( hashedString.mDebugString, strLength );
#endif
    }
    //-----------------------------------------------------------------------------------
    void HlmsDiskCache::save( DataStreamPtr &dataStream, const String &string )
    {
        write<uint32>( dataStream, static_cast<uint32>( string.size() ) );
        dataStream->write( string.c_str(), string.size() );
    }
    //-----------------------------------------------------------------------------------
    void HlmsDiskCache::save( DataStreamPtr &dataStream, const HlmsPropertyVec &properties )
    {
        write<uint32>( dataStream, static_cast<uint32>( properties.size() ) );

        HlmsPropertyVec::const_iterator itor = properties.begin();
        HlmsPropertyVec::const_iterator end  = properties.end();

        while( itor != end )
        {
            save( dataStream, itor->keyName );
            write( dataStream, itor->value );
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsDiskCache::save( DataStreamPtr &dataStream, const Hlms::RenderableCache &renderableCache )
    {
        //Save the properties
        save( dataStream, renderableCache.setProperties );

        //Save the pieces
        for( size_t i=0; i<NumShaderTypes; ++i )
        {
            write<uint32>( dataStream, static_cast<uint32>( renderableCache.pieces[i].size() ) );

            PiecesMap::const_iterator itor = renderableCache.pieces[i].begin();
            PiecesMap::const_iterator end  = renderableCache.pieces[i].end();

            while( itor != end )
            {
                save( dataStream, itor->first );
                save( dataStream, itor->second );
                ++itor;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsDiskCache::saveTo( DataStreamPtr &dataStream )
    {
        LogManager::getSingleton().logMessage( "Saving HlmsDiskCache to " + dataStream->getName() );

        write<uint16>( dataStream, c_hlmsDiskCacheVersion );
#if OGRE_DEBUG_STR_SIZE > 0
        write<uint16>( dataStream, OGRE_DEBUG_STR_SIZE );
#else
        write<uint16>( dataStream, 0 );
#endif
        write( dataStream, mCache.templateHash );
        write( dataStream, mCache.type );
        save( dataStream, mShaderProfile );

        {
            //Save shaders
            write<uint32>( dataStream, static_cast<uint32>( mCache.sourceCode.size() ) );

            SourceCodeVec::const_iterator itor = mCache.sourceCode.begin();
            SourceCodeVec::const_iterator end  = mCache.sourceCode.end();

            while( itor != end )
            {
                save( dataStream, itor->mergedCache );
                for( size_t i=0; i<NumShaderTypes; ++i )
                    save( dataStream, itor->sourceFile[i] );

                ++itor;
            }
        }

        {
            //Save PSOs
            write<uint32>( dataStream, static_cast<uint32>( mCache.pso.size() ) );

            PsoVec::const_iterator itor = mCache.pso.begin();
            PsoVec::const_iterator end  = mCache.pso.end();

            while( itor != end )
            {
                save( dataStream, itor->renderableCache );
                save( dataStream, itor->passProperties );

                write<uint32>( dataStream, static_cast<uint32>( itor->pso.vertexElements.size() ) );
                VertexElement2VecVec::const_iterator itElem = itor->pso.vertexElements.begin();
                VertexElement2VecVec::const_iterator enElem = itor->pso.vertexElements.end();

                while( itElem != enElem )
                {
                    write<uint32>( dataStream, static_cast<uint32>( itElem->size() ) );
                    VertexElement2Vec::const_iterator itElem2 = itElem->begin();
                    VertexElement2Vec::const_iterator enElem2 = itElem->end();

                    while( itElem2 != enElem2 )
                    {
                        write( dataStream, itElem2->mType );
                        write( dataStream, itElem2->mSemantic );
                        write( dataStream, itElem2->mInstancingStepRate );
                        ++itElem2;
                    }

                    ++itElem;
                }

                write( dataStream, itor->pso.operationType );
                write( dataStream, itor->pso.enablePrimitiveRestart );
                write( dataStream, itor->pso.sampleMask );
                write( dataStream, itor->pso.pass );

                write( dataStream, itor->macroblock.mScissorTestEnabled );
                write( dataStream, itor->macroblock.mDepthCheck );
                write( dataStream, itor->macroblock.mDepthWrite );
                write( dataStream, itor->macroblock.mDepthFunc );
                write( dataStream, itor->macroblock.mDepthBiasConstant );
                write( dataStream, itor->macroblock.mDepthBiasSlopeScale );
                write( dataStream, itor->macroblock.mCullMode );
                write( dataStream, itor->macroblock.mPolygonMode );

                write( dataStream, itor->blendblock.mAlphaToCoverageEnabled );
                write( dataStream, itor->blendblock.mBlendChannelMask );
//                write( dataStream, itor->blendblock.mIsTransparent );
                write( dataStream, itor->blendblock.mSeparateBlend );
                write( dataStream, itor->blendblock.mSourceBlendFactor );
                write( dataStream, itor->blendblock.mDestBlendFactor );
                write( dataStream, itor->blendblock.mSourceBlendFactorAlpha );
                write( dataStream, itor->blendblock.mDestBlendFactorAlpha );
                write( dataStream, itor->blendblock.mBlendOperation );
                write( dataStream, itor->blendblock.mBlendOperationAlpha );

                ++itor;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    template <typename T> void read( DataStreamPtr &dataStream, T &value )
    {
        dataStream->read( &value, sizeof(value) );
    }
    template <typename T> T read( DataStreamPtr &dataStream )
    {
        T value;
        dataStream->read( &value, sizeof(value) );
        return value;
    }
    //-----------------------------------------------------------------------------------
    void HlmsDiskCache::load( DataStreamPtr &dataStream, IdString &hashedString )
    {
        read( dataStream, hashedString.mHash );
#if OGRE_DEBUG_STR_SIZE > 0
        if( mDebugStrSize > 0 )
        {
            const uint16 strLength = read<uint16>( dataStream );
            const uint16 bytesToRead = std::min<uint16>( strLength, OGRE_DEBUG_STR_SIZE - 1u );
            dataStream->read( hashedString.mDebugString, bytesToRead );
            hashedString.mDebugString[bytesToRead] = '\0'; //Force the string to be null-terminated
            dataStream->skip( strLength - bytesToRead );
        }
#else
        if( mDebugStrSize > 0 )
        {
            const uint16 strLength = read<uint16>( dataStream );
            dataStream->skip( strLength );
        }
#endif
    }
    //-----------------------------------------------------------------------------------
    void HlmsDiskCache::load( DataStreamPtr &dataStream, String &string )
    {
        const uint32 stringLength = read<uint32>( dataStream );
        string.resize( stringLength );
        if( stringLength > 0u )
            dataStream->read( &string[0], string.size() );
    }
    //-----------------------------------------------------------------------------------
    void HlmsDiskCache::load( DataStreamPtr &dataStream, HlmsPropertyVec &properties )
    {
        uint32 numEntries = read<uint32>( dataStream );
        properties.clear();
        properties.reserve( numEntries );

        for( size_t j=0; j<numEntries; ++j )
        {
            IdString keyName;
            int32 value;
            load( dataStream, keyName );
            read( dataStream, value );
            properties.push_back( HlmsProperty( keyName, value ) );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsDiskCache::load( DataStreamPtr &dataStream, Hlms::RenderableCache &renderableCache )
    {
        //Load the properties
        load( dataStream, renderableCache.setProperties );

        //Load the pieces
        for( size_t i=0; i<NumShaderTypes; ++i )
        {
            uint32 numEntries = read<uint32>( dataStream );
            //renderableCache.pieces[i].reserve( numEntries );
            renderableCache.pieces[i].clear();

            for( size_t j=0; j<numEntries; ++j )
            {
                IdString key;
                String valueStr;
                load( dataStream, key );
                load( dataStream, valueStr );
                renderableCache.pieces[i][key] = valueStr;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsDiskCache::loadFrom( DataStreamPtr &dataStream )
    {
        LogManager::getSingleton().logMessage( "Loading HlmsDiskCache from " + dataStream->getName() );

        clearCache();

        const uint16 version = read<uint16>( dataStream );
        if( version != c_hlmsDiskCacheVersion )
        {
            LogManager::getSingleton().logMessage( "HlmsDiskCache: Version mismatch. Not loading." );
            return;
        }

        mDebugStrSize = read<uint16>( dataStream );
#if OGRE_DEBUG_STR_SIZE > 0
        if( OGRE_DEBUG_STR_SIZE != mDebugStrSize )
        {
            LogManager::getSingleton().logMessage(
                        "HlmsDiskCache: This cache was built with a OGRE_DEBUG_STR_SIZE (IdString) of "
                        + StringConverter::toString( mDebugStrSize ) +
                        ". It cannot be used. Not loading." );
            return;
        }
#endif

        read( dataStream, mCache.templateHash );
        read( dataStream, mCache.type );
        load( dataStream, mShaderProfile );

        {
            //Load shaders
            uint32 numEntries = read<uint32>( dataStream );
            mCache.sourceCode.reserve( numEntries );

            SourceCode sourceCode;
            for( size_t i=0; i<numEntries; ++i )
            {
                load( dataStream, sourceCode.mergedCache );
                for( size_t j=0; j<NumShaderTypes; ++j )
                    load( dataStream, sourceCode.sourceFile[j] );
                mCache.sourceCode.push_back( sourceCode );
            }
        }

        {
            //Load PSOs
            const uint32 numEntries = read<uint32>( dataStream );
            mCache.pso.reserve( numEntries );

            Pso pso;
            for( size_t i=0; i<numEntries; ++i )
            {
                load( dataStream, pso.renderableCache );
                load( dataStream, pso.passProperties );

                const uint32 numVertexElements = read<uint32>( dataStream );
                pso.pso.vertexElements.reserve( numVertexElements );

                for( size_t j=0; j<numVertexElements; ++j )
                {
                    pso.pso.vertexElements.push_back( VertexElement2Vec() );
                    VertexElement2Vec &vertexElements = pso.pso.vertexElements.back();

                    const uint32 numVertexElements2 = read<uint32>( dataStream );
                    vertexElements.reserve( numVertexElements2 );

                    for( size_t k=0; k<numVertexElements2; ++k )
                    {
                        VertexElementType type          = read<VertexElementType>( dataStream );
                        VertexElementSemantic semantic  = read<VertexElementSemantic>( dataStream );
                        uint32 instancingStepRate       = read<uint32>( dataStream );
                        vertexElements.push_back( VertexElement2( type, semantic ) );
                        vertexElements.back().mInstancingStepRate = instancingStepRate;
                    }
                }

                read( dataStream, pso.pso.operationType );
                read( dataStream, pso.pso.enablePrimitiveRestart );
                read( dataStream, pso.pso.sampleMask );
                read( dataStream, pso.pso.pass );

                read( dataStream, pso.macroblock.mScissorTestEnabled );
                read( dataStream, pso.macroblock.mDepthCheck );
                read( dataStream, pso.macroblock.mDepthWrite );
                read( dataStream, pso.macroblock.mDepthFunc );
                read( dataStream, pso.macroblock.mDepthBiasConstant );
                read( dataStream, pso.macroblock.mDepthBiasSlopeScale );
                read( dataStream, pso.macroblock.mCullMode );
                read( dataStream, pso.macroblock.mPolygonMode );

                read( dataStream, pso.blendblock.mAlphaToCoverageEnabled );
                read( dataStream, pso.blendblock.mBlendChannelMask );
    //          read( dataStream, pso.blendblock.mIsTransparent );
                read( dataStream, pso.blendblock.mSeparateBlend );
                read( dataStream, pso.blendblock.mSourceBlendFactor );
                read( dataStream, pso.blendblock.mDestBlendFactor );
                read( dataStream, pso.blendblock.mSourceBlendFactorAlpha );
                read( dataStream, pso.blendblock.mDestBlendFactorAlpha );
                read( dataStream, pso.blendblock.mBlendOperation );
                read( dataStream, pso.blendblock.mBlendOperationAlpha );

                //We retrieve the Macroblock & Blendblock from HlmsManager and immediately remove them
                //This allows us to create a permanent pointer, while the actual internal pointer is
                //released (i.e. it becomes inactive)
                pso.pso.macroblock = mHlmsManager->getMacroblock( pso.macroblock );
                mHlmsManager->destroyMacroblock( pso.pso.macroblock );

                pso.pso.blendblock = mHlmsManager->getBlendblock( pso.blendblock );
                mHlmsManager->destroyBlendblock( pso.pso.blendblock );

                uint16 inputLayoutId = mHlmsManager->_getInputLayoutId( pso.pso.vertexElements,
                                                                        pso.pso.operationType );

                //Reset these properties because they may be different now
                Hlms::setProperty( pso.renderableCache.setProperties, HlmsPsoProp::Macroblock,
                                   pso.pso.macroblock->mLifetimeId );
                Hlms::setProperty( pso.renderableCache.setProperties, HlmsPsoProp::Blendblock,
                                   pso.pso.blendblock->mLifetimeId );
                Hlms::setProperty( pso.renderableCache.setProperties, HlmsPsoProp::InputLayoutId,
                                   inputLayoutId );

                mCache.pso.push_back( pso );
            }
        }
    }
}
