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
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreStreamSerialiser.h"

namespace Ogre {
    static uint32 CACHE_CHUNK_ID = StreamSerialiser::makeIdentifier("OGPC"); // Ogre Gpu Program cache

    /// default implementation for RenderSystems without assembly shader support
    class UnsupportedGpuProgram : public GpuProgram
    {
    public:
        UnsupportedGpuProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader)
            : GpuProgram(creator, name, handle, group, isManual, loader) { }

        bool isSupported() const { return false; }
        void throwException()
        {
            String message = "assembly shaders are unsupported. Shader name:" + mName + "\n";
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, message);
        }

    protected:
        void loadImpl(void)         { throwException(); }
        void loadFromSource(void)   { throwException(); }
        void unloadImpl(void)       { }
    };

    Resource* GpuProgramManager::createImpl(const String& name, ResourceHandle handle, const String& group,
                                            bool isManual, ManualResourceLoader* loader,
                                            const NameValuePairList* params)
    {
        NameValuePairList::const_iterator paramSyntax, paramType;

        if (!params || (paramSyntax = params->find("syntax")) == params->end() ||
            (paramType = params->find("type")) == params->end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "You must supply 'syntax' and 'type' parameters");
        }

        GpuProgramType gpt;
        if (paramType->second == "vertex_program")
        {
            gpt = GPT_VERTEX_PROGRAM;
        }
        else if (paramType->second == "geometry_program")
        {
            gpt = GPT_GEOMETRY_PROGRAM;
        }
        else
        {
            gpt = GPT_FRAGMENT_PROGRAM;
        }

        return createImpl(name, handle, group, isManual, loader, gpt, paramSyntax->second);
    }

    Resource* GpuProgramManager::createImpl(const String& name, ResourceHandle handle, const String& group,
                                            bool isManual, ManualResourceLoader* loader,
                                            GpuProgramType gptype, const String& syntaxCode)
    {
        return new UnsupportedGpuProgram(this, name, handle, group, isManual, loader);
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
    GpuProgramPtr GpuProgramManager::getByName(const String& name, const String& group, bool preferHighLevelPrograms)
    {
        return static_pointer_cast<GpuProgram>(getResourceByName(name, group, preferHighLevelPrograms));
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

        // subclasses should register with resource group manager
    }
    //---------------------------------------------------------------------------
    GpuProgramManager::~GpuProgramManager()
    {
        // subclasses should unregister with resource group manager
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
    ResourcePtr GpuProgramManager::create(const String& name, const String& group, 
        GpuProgramType gptype, const String& syntaxCode, bool isManual, 
        ManualResourceLoader* loader)
    {
        // Call creation implementation
        ResourcePtr ret = ResourcePtr(
            createImpl(name, getNextHandle(), group, isManual, loader, gptype, syntaxCode));

        addImpl(ret);
        // Tell resource group manager
        if(ret)
            ResourceGroupManager::getSingleton()._notifyResourceCreated(ret);
        return ret;
    }
    //---------------------------------------------------------------------------
    GpuProgramPtr GpuProgramManager::createProgram(const String& name, 
        const String& groupName, const String& filename, 
        GpuProgramType gptype, const String& syntaxCode)
    {
        GpuProgramPtr prg = static_pointer_cast<GpuProgram>(create(name, groupName, gptype, syntaxCode));
        // Set all prarmeters (create does not set, just determines factory)
        prg->setType(gptype);
        prg->setSyntaxCode(syntaxCode);
        prg->setSourceFile(filename);
        return prg;
    }
    //---------------------------------------------------------------------------
    GpuProgramPtr GpuProgramManager::createProgramFromString(const String& name, 
        const String& groupName, const String& code, GpuProgramType gptype, 
        const String& syntaxCode)
    {
        GpuProgramPtr prg = static_pointer_cast<GpuProgram>(create(name, groupName, gptype, syntaxCode));
        // Set all prarmeters (create does not set, just determines factory)
        prg->setType(gptype);
        prg->setSyntaxCode(syntaxCode);
        prg->setSource(code);
        return prg;
    }
    //---------------------------------------------------------------------------
    const GpuProgramManager::SyntaxCodes& GpuProgramManager::getSupportedSyntax(void) const
    {
        // Use the current render system
        RenderSystem* rs = Root::getSingleton().getRenderSystem();

        // Get the supported syntaxed from RenderSystemCapabilities 
        return rs->getCapabilities()->getSupportedShaderProfiles();
    }

    //---------------------------------------------------------------------------
    bool GpuProgramManager::isSyntaxSupported(const String& syntaxCode) const
    {
        // Use the current render system
        RenderSystem* rs = Root::getSingleton().getRenderSystem();

        // Get the supported syntax from RenderSystemCapabilities 
        return rs && rs->getCapabilities()->isShaderProfileSupported(syntaxCode);
    }
    //---------------------------------------------------------------------------
    ResourcePtr GpuProgramManager::getResourceByName(const String& name, const String& group, bool preferHighLevelPrograms)
    {
        if (!preferHighLevelPrograms)
            return ResourceManager::getResourceByName(name, group);
        return getResourceByName(name, group);
    }
    ResourcePtr GpuProgramManager::getResourceByName(const String& name, const String& group)
    {
        // prefer HighLevel Programs
        ResourcePtr ret = HighLevelGpuProgramManager::getSingleton().getResourceByName(name, group);
        return ret ? ret : ResourceManager::getResourceByName(name, group);
    }
    //-----------------------------------------------------------------------------
    GpuProgramParametersSharedPtr GpuProgramManager::createParameters(void)
    {
        return GpuProgramParametersSharedPtr(OGRE_NEW GpuProgramParameters());
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
                "No shared parameter set with name '" + name + "'!", 
                "GpuProgramManager::getSharedParameters");
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
    GpuProgramManager::Microcode GpuProgramManager::createMicrocode( size_t size ) const
    {   
        return Microcode(OGRE_NEW MemoryDataStream(size));  
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
    void GpuProgramManager::saveMicrocodeCache( DataStreamPtr stream ) const
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
        serialiser.writeChunkBegin(CACHE_CHUNK_ID, 2);

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
    void GpuProgramManager::loadMicrocodeCache( DataStreamPtr stream )
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

        if(chunk->id != CACHE_CHUNK_ID || chunk->version != 2)
        {
            LogManager::getSingleton().logWarning("Invalid Microcode Cache");
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

}
