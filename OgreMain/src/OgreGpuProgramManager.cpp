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
#include "OgreGpuProgramManager.h"

#include <memory>
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreUnifiedHighLevelGpuProgram.h"
#include "OgreStreamSerialiser.h"

namespace Ogre {
namespace {
    uint32 CACHE_CHUNK_ID = StreamSerialiser::makeIdentifier("OGPC"); // Ogre Gpu Program cache

    String sNullLang = "null";
    class NullProgram : public GpuProgram
    {
    protected:
        /** Internal load implementation, must be implemented by subclasses.
        */
        void loadFromSource(void) override {}
        void unloadImpl() override {}

    public:
        NullProgram(ResourceManager* creator,
            const String& name, ResourceHandle handle, const String& group,
            bool isManual, ManualResourceLoader* loader)
            : GpuProgram(creator, name, handle, group, isManual, loader){}
        ~NullProgram() {}
        /// Overridden from GpuProgram - never supported
        bool isSupported(void) const override { return false; }
        /// Overridden from GpuProgram
        const String& getLanguage(void) const override { return sNullLang; }
        size_t calculateSize(void) const override { return 0; }

        /// Overridden from StringInterface
        bool setParameter(const String& name, const String& value)
        {
            // always silently ignore all parameters so as not to report errors on
            // unsupported platforms
            return true;
        }

    };
    class NullProgramFactory : public HighLevelGpuProgramFactory
    {
    public:
        NullProgramFactory() {}
        ~NullProgramFactory() {}
        /// Get the name of the language this factory creates programs for
        const String& getLanguage(void) const override
        {
            return sNullLang;
        }
        GpuProgram* create(ResourceManager* creator,
            const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader) override
        {
            return OGRE_NEW NullProgram(creator, name, handle, group, isManual, loader);
        }
    };
}

    Resource* GpuProgramManager::createImpl(const String& name, ResourceHandle handle, const String& group,
                                            bool isManual, ManualResourceLoader* loader,
                                            const NameValuePairList* params)
    {
        OgreAssert(params, "params cannot be null");

        auto langIt = params->find("language");
        auto typeIt = params->find("type");

        if(langIt == params->end())
            langIt = params->find("syntax");

        if (langIt == params->end() || typeIt == params->end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "You must supply 'language' or 'syntax' and 'type' parameters");
        }

        // "syntax" and "type" will be applied by ResourceManager::createResource
        return getFactory(langIt->second)->create(this, name, handle, group, isManual, loader);
    }

    //-----------------------------------------------------------------------
    template<> GpuProgramManager* Singleton<GpuProgramManager>::msSingleton = 0;
    GpuProgramManager* GpuProgramManager::getSingletonPtr(void)
    {
        return msSingleton;
    }
    GpuProgramManager& GpuProgramManager::getSingleton(void)
    {  
        assert( msSingleton );  return ( *msSingleton );  
    }
    //-----------------------------------------------------------------------
    GpuProgramPtr GpuProgramManager::getByName(const String& name, const String& group) const
    {
        return static_pointer_cast<GpuProgram>(getResourceByName(name, group));
    }
    //---------------------------------------------------------------------------
    GpuProgramManager::GpuProgramManager()
    {
        // Loading order
        mLoadOrder = 50.0f;
        // Resource type
        mResourceType = "GpuProgram";
        mSaveMicrocodesToCache = false;
        mCacheDirty = false;

        mNullFactory = std::make_unique<NullProgramFactory>();
        addFactory(mNullFactory.get());
        mUnifiedFactory = std::make_unique<UnifiedHighLevelGpuProgramFactory>();
        addFactory(mUnifiedFactory.get());

        ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);
    }
    //---------------------------------------------------------------------------
    GpuProgramManager::~GpuProgramManager()
    {
        ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);
    }
    //---------------------------------------------------------------------------
    GpuProgramPtr GpuProgramManager::load(const String& name,
        const String& groupName, const String& filename, 
        GpuProgramType gptype, const String& syntaxCode)
    {
        GpuProgramPtr prg;
        {
            OGRE_LOCK_AUTO_MUTEX;
            prg = getByName(name, groupName);
            if (!prg)
            {
                prg = createProgram(name, groupName, filename, gptype, syntaxCode);
            }

        }
        prg->load();
        return prg;
    }
    //---------------------------------------------------------------------------
    GpuProgramPtr GpuProgramManager::loadFromString(const String& name, 
        const String& groupName, const String& code, 
        GpuProgramType gptype, const String& syntaxCode)
    {
        GpuProgramPtr prg;
        {
                    OGRE_LOCK_AUTO_MUTEX;
            prg = getByName(name, groupName);
            if (!prg)
            {
                prg = createProgramFromString(name, groupName, code, gptype, syntaxCode);
            }

        }
        prg->load();
        return prg;
    }
    //---------------------------------------------------------------------------
    GpuProgramPtr GpuProgramManager::create(const String& name, const String& group, GpuProgramType gptype,
                                            const String& syntaxCode, bool isManual,
                                            ManualResourceLoader* loader)
    {
        auto prg = getFactory(syntaxCode)->create(this, name, getNextHandle(), group, isManual, loader);
        prg->setType(gptype);
        prg->setSyntaxCode(syntaxCode);

        ResourcePtr ret(prg);
        addImpl(ret);
        // Tell resource group manager
        if(ret)
            ResourceGroupManager::getSingleton()._notifyResourceCreated(ret);
        return static_pointer_cast<GpuProgram>(ret);
    }
    //---------------------------------------------------------------------------
    GpuProgramPtr GpuProgramManager::createProgram(const String& name, const String& groupName,
                                                   const String& filename, GpuProgramType gptype,
                                                   const String& syntaxCode)
    {
        GpuProgramPtr prg = createProgram(name, groupName, syntaxCode, gptype);
        prg->setSourceFile(filename);
        return prg;
    }
    //---------------------------------------------------------------------------
    GpuProgramPtr GpuProgramManager::createProgramFromString(const String& name, 
        const String& groupName, const String& code, GpuProgramType gptype, 
        const String& syntaxCode)
    {
        GpuProgramPtr prg = createProgram(name, groupName, syntaxCode, gptype);
        prg->setSource(code);
        return prg;
    }
    //---------------------------------------------------------------------------
    const GpuProgramManager::SyntaxCodes& GpuProgramManager::getSupportedSyntax(void)
    {
        // Use the current render system
        RenderSystem* rs = Root::getSingleton().getRenderSystem();

        // Get the supported syntaxed from RenderSystemCapabilities 
        return rs->getCapabilities()->getSupportedShaderProfiles();
    }

    //---------------------------------------------------------------------------
    bool GpuProgramManager::isSyntaxSupported(const String& syntaxCode)
    {
        // Use the current render system
        RenderSystem* rs = Root::getSingleton().getRenderSystem();

        // Get the supported syntax from RenderSystemCapabilities 
        return rs && rs->getCapabilities()->isShaderProfileSupported(syntaxCode);
    }
    //---------------------------------------------------------------------
    GpuSharedParametersPtr GpuProgramManager::createSharedParameters(const String& name)
    {
        if (mSharedParametersMap.find(name) != mSharedParametersMap.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "The shared parameter set '" + name + "' already exists!", 
                "GpuProgramManager::createSharedParameters");
        }
        GpuSharedParametersPtr ret(OGRE_NEW GpuSharedParameters(name));
        mSharedParametersMap[name] = ret;
        return ret;
    }
    //---------------------------------------------------------------------
    GpuSharedParametersPtr GpuProgramManager::getSharedParameters(const String& name) const
    {
        SharedParametersMap::const_iterator i = mSharedParametersMap.find(name);
        if (i == mSharedParametersMap.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "referenced shared_params '" + name + "' not found");
        }
        return i->second;
    }
    //---------------------------------------------------------------------
    const GpuProgramManager::SharedParametersMap& 
    GpuProgramManager::getAvailableSharedParameters() const
    {
        return mSharedParametersMap;
    }
    //---------------------------------------------------------------------
    bool GpuProgramManager::getSaveMicrocodesToCache() const
    {
        return mSaveMicrocodesToCache;
    }
    //---------------------------------------------------------------------
    bool GpuProgramManager::canGetCompiledShaderBuffer()
    {
        // Use the current render system
        RenderSystem* rs = Root::getSingleton().getRenderSystem();

        // Check if the supported  
        return rs->getCapabilities()->hasCapability(RSC_CAN_GET_COMPILED_SHADER_BUFFER);
    }
    //---------------------------------------------------------------------
    void GpuProgramManager::setSaveMicrocodesToCache( const bool val )
    {
        // Check that saving shader microcode is supported
        if(!canGetCompiledShaderBuffer())
            mSaveMicrocodesToCache = false;
        else
            mSaveMicrocodesToCache = val;
    }
    //---------------------------------------------------------------------
    bool GpuProgramManager::isCacheDirty( void ) const
    {
        return mCacheDirty;     
    }
    //---------------------------------------------------------------------
    String GpuProgramManager::addRenderSystemToName( const String & name )
    {
        // Use the current render system
        RenderSystem* rs = Root::getSingleton().getRenderSystem();

        return rs->getName() + "_" + name;
    }
    //---------------------------------------------------------------------
    bool GpuProgramManager::isMicrocodeAvailableInCache( uint32 id ) const
    {
        return mMicrocodeCache.find(id) != mMicrocodeCache.end();
    }
    //---------------------------------------------------------------------
    const GpuProgramManager::Microcode & GpuProgramManager::getMicrocodeFromCache( uint32 id ) const
    {
        return mMicrocodeCache.find(id)->second;
    }
    //---------------------------------------------------------------------
    void GpuProgramManager::addMicrocodeToCache( uint32 id, const GpuProgramManager::Microcode & microcode )
    {   
        auto foundIter = mMicrocodeCache.find(id);
        if ( foundIter == mMicrocodeCache.end() )
        {
            mMicrocodeCache.insert(make_pair(id, microcode));
            // if cache is modified, mark it as dirty.
            mCacheDirty = true;
        }
        else
        {
            foundIter->second = microcode;

        }       
    }
    //---------------------------------------------------------------------
    void GpuProgramManager::removeMicrocodeFromCache( uint32 id )
    {
        auto foundIter = mMicrocodeCache.find(id);

        if (foundIter != mMicrocodeCache.end())
        {
            mMicrocodeCache.erase( foundIter );
            mCacheDirty = true;
        }
    }
    //---------------------------------------------------------------------
    void GpuProgramManager::saveMicrocodeCache( const DataStreamPtr& stream ) const
    {
        if (!mCacheDirty)
            return; 

        if (!stream->isWriteable())
        {
            OGRE_EXCEPT(Exception::ERR_CANNOT_WRITE_TO_FILE,
                "Unable to write to stream " + stream->getName(),
                "GpuProgramManager::saveMicrocodeCache");
        }
        
        StreamSerialiser serialiser(stream);
        serialiser.writeChunkBegin(CACHE_CHUNK_ID, 3);

        // write the size of the array
        uint32 sizeOfArray = static_cast<uint32>(mMicrocodeCache.size());
        serialiser.write(&sizeOfArray);
        
        // loop the array and save it
        for ( const auto& entry : mMicrocodeCache )
        {
            // saves the id of the shader
            serialiser.write(&entry.first);

            // saves the microcode
            const Microcode & microcodeOfShader = entry.second;
            uint32 microcodeLength = static_cast<uint32>(microcodeOfShader->size());
            serialiser.write(&microcodeLength);
            serialiser.writeData(microcodeOfShader->getPtr(), 1, microcodeLength);
        }

        serialiser.writeChunkEnd(CACHE_CHUNK_ID);
    }
    //---------------------------------------------------------------------
    void GpuProgramManager::loadMicrocodeCache( const DataStreamPtr& stream )
    {
        mMicrocodeCache.clear();

        StreamSerialiser serialiser(stream);
        const StreamSerialiser::Chunk* chunk;

        try
        {
            chunk = serialiser.readChunkBegin();
        }
        catch (const InvalidStateException& e)
        {
            LogManager::getSingleton().logWarning("Could not load Microcode Cache: " +
                                                  e.getDescription());
            return;
        }

        if(chunk->id != CACHE_CHUNK_ID || chunk->version != 3)
        {
            LogManager::getSingleton().logWarning("Invalid Microcode Cache");
            serialiser.readChunkEnd(CACHE_CHUNK_ID);
            return;
        }
        // write the size of the array
        uint32 sizeOfArray = 0;
        serialiser.read(&sizeOfArray);
        
        // loop the array and load it
        for ( uint32 i = 0 ; i < sizeOfArray ; i++ )
        {
            // loads the id of the shader
            uint32 id;
            serialiser.read(&id);

            // loads the microcode
            uint32 microcodeLength = 0;
            serialiser.read(&microcodeLength);

            Microcode microcodeOfShader(OGRE_NEW MemoryDataStream(microcodeLength));
            microcodeOfShader->seek(0);
            serialiser.readData(microcodeOfShader->getPtr(), 1, microcodeLength);

            mMicrocodeCache.insert(make_pair(id, microcodeOfShader));
        }
        serialiser.readChunkEnd(CACHE_CHUNK_ID);

        // if cache is not modified, mark it as clean.
        mCacheDirty = false;
        
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------------
    void GpuProgramManager::addFactory(GpuProgramFactory* factory)
    {
        // deliberately allow later plugins to override earlier ones
        mFactories[factory->getLanguage()] = factory;
    }
    //---------------------------------------------------------------------------
    void GpuProgramManager::removeFactory(GpuProgramFactory* factory)
    {
        // Remove only if equal to registered one, since it might overridden
        // by other plugins
        FactoryMap::iterator it = mFactories.find(factory->getLanguage());
        if (it != mFactories.end() && it->second == factory)
        {
            mFactories.erase(it);
        }
    }
    //---------------------------------------------------------------------------
    GpuProgramFactory* GpuProgramManager::getFactory(const String& language)
    {
        FactoryMap::iterator i = mFactories.find(language);

        if (i == mFactories.end())
        {
            // use the null factory to create programs that will never be supported
            i = mFactories.find(sNullLang);
        }
        return i->second;
    }
    //---------------------------------------------------------------------
    bool GpuProgramManager::isLanguageSupported(const String& lang) const
    {
        return mFactories.find(lang) != mFactories.end();
    }
}
