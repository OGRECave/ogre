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
#include "OgreD3D9GpuProgram.h"
#include "OgreMatrix4.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreD3D9Mappings.h"
#include "OgreResourceGroupManager.h"
#include "OgreD3D9RenderSystem.h"
#include "OgreGpuProgramManager.h"

namespace Ogre {

    D3D9GpuProgram::CmdColumnMajorMatrices D3D9GpuProgram::msCmdColumnMajorMatrices;
    D3D9GpuProgram::CmdExternalMicrocode D3D9GpuProgram::msCmdExternalMicrocode;

   //-----------------------------------------------------------------------
    String D3D9GpuProgram::CmdColumnMajorMatrices::doGet(const void *target) const
    {
        return StringConverter::toString(static_cast<const D3D9GpuProgram*>(target)->getColumnMajorMatrices());
    }
    void D3D9GpuProgram::CmdColumnMajorMatrices::doSet(void *target, const String& val)
    {
        static_cast<D3D9GpuProgram*>(target)->setColumnMajorMatrices(StringConverter::parseBool(val));
    }
    //-----------------------------------------------------------------------
    String D3D9GpuProgram::CmdExternalMicrocode::doGet(const void *target) const
    {
        //D3D9GpuProgram* program=const_cast<D3D9GpuProgram*>(static_cast<const D3D9GpuProgram*>(target));
        //LPD3DXBUFFER ptr=program->getExternalMicrocode();
        //nothing to do
        return String();
    }
    void D3D9GpuProgram::CmdExternalMicrocode::doSet(void *target, const String& val)
    {
        D3D9GpuProgram* program = const_cast<D3D9GpuProgram*>(static_cast<const D3D9GpuProgram*>(target));
        const void* buffer = val.data();
        program->setExternalMicrocode(buffer, val.size());
    }

    //-----------------------------------------------------------------------------
    D3D9GpuProgram::D3D9GpuProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader) 
        : GpuProgram(creator, name, handle, group, isManual, loader), mExternalMicrocode(NULL), mColumnMajorMatrices(false)
    {           
        if (createParamDictionary("D3D9GpuProgram"))
        {
            setupBaseParamDictionary();

            ParamDictionary* dict = getParamDictionary();
            dict->addParameter(ParameterDef("column_major_matrices", 
                "Whether matrix packing in column-major order.",
                PT_BOOL),&msCmdColumnMajorMatrices);
            dict->addParameter(ParameterDef("external_micro_code", 
                "the cached external micro code data.",
                PT_STRING),&msCmdExternalMicrocode);
        }
    }

    //-----------------------------------------------------------------------------
    D3D9GpuProgram::~D3D9GpuProgram()
    {
        // have to call this here reather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        unload();
    }
    
    //-----------------------------------------------------------------------------
    void D3D9GpuProgram::setExternalMicrocode(const void* pMicrocode, size_t size)
    {
        LPD3DXBUFFER pBuffer=0;
        HRESULT hr=D3DXCreateBuffer(size, &pBuffer);
        if(pBuffer)
        {
            memcpy(pBuffer->GetBufferPointer(), pMicrocode, size);
            this->setExternalMicrocode(pBuffer);
            SAFE_RELEASE(pBuffer);
        }
    }
    //-----------------------------------------------------------------------------
    void D3D9GpuProgram::setExternalMicrocode(ID3DXBuffer* pMicrocode)
    { 
        SAFE_RELEASE(mExternalMicrocode);
        mExternalMicrocode = pMicrocode;
        if(mExternalMicrocode)
        {
            mExternalMicrocode->AddRef();   
        }
    }
    //-----------------------------------------------------------------------------
    LPD3DXBUFFER D3D9GpuProgram::getExternalMicrocode(void)
    {
        return mExternalMicrocode;
    }

    //-----------------------------------------------------------------------------
    void D3D9GpuProgram::loadImpl(void)
    {
        D3D9_DEVICE_ACCESS_CRITICAL_SECTION

        for (uint i = 0; i < D3D9RenderSystem::getResourceCreationDeviceCount(); ++i)
        {
            IDirect3DDevice9* d3d9Device = D3D9RenderSystem::getResourceCreationDevice(i);

            loadImpl(d3d9Device);
        }              
    }

    //-----------------------------------------------------------------------------
    void D3D9GpuProgram::loadImpl(IDirect3DDevice9* d3d9Device)
    {
        D3D9_DEVICE_ACCESS_CRITICAL_SECTION

        if (mExternalMicrocode)
        {
            loadFromMicrocode(d3d9Device, mExternalMicrocode);
        }
        else
        {
            // Normal load-from-source approach
            if (mLoadFromFile)
            {
                // find & load source code
                DataStreamPtr stream = 
                    ResourceGroupManager::getSingleton().openResource(
                    mFilename, mGroup, true, this);
                mSource = stream->getAsString();
            }

            // Call polymorphic load
            loadFromSource(d3d9Device);
        }
    }
    //-----------------------------------------------------------------------------
    void D3D9GpuProgram::loadFromSource(void)
    {
        loadFromSource(NULL);
    }
    //-----------------------------------------------------------------------------
   void D3D9GpuProgram::loadFromSource( IDirect3DDevice9* d3d9Device )
    {
        uint32 hash = _getHash();
        if ( GpuProgramManager::getSingleton().isMicrocodeAvailableInCache(hash) )
        {
            getMicrocodeFromCache( d3d9Device, hash );
        }
        else
        {
            compileMicrocode( d3d9Device );
        }
    }
    //-----------------------------------------------------------------------
    void D3D9GpuProgram::getMicrocodeFromCache( IDirect3DDevice9* d3d9Device, uint32 id )
    {
        GpuProgramManager::Microcode cacheMicrocode = 
            GpuProgramManager::getSingleton().getMicrocodeFromCache(id);
        
        LPD3DXBUFFER microcode;
        HRESULT hr=D3DXCreateBuffer(cacheMicrocode->size(), &microcode); 

        if(microcode)
        {
            memcpy(microcode->GetBufferPointer(), cacheMicrocode->getPtr(), cacheMicrocode->size());
        }
        
        loadFromMicrocode(d3d9Device, microcode);
    }
    //-----------------------------------------------------------------------
    void D3D9GpuProgram::compileMicrocode( IDirect3DDevice9* d3d9Device )
    {
        D3D9_DEVICE_ACCESS_CRITICAL_SECTION


        // Populate compile flags
        DWORD compileFlags = 0;

        // Create the shader
        // Assemble source into microcode
        LPD3DXBUFFER microcode;
        LPD3DXBUFFER errors;
        HRESULT hr = D3DXAssembleShader(
            mSource.c_str(),
            static_cast<UINT>(mSource.length()),
            NULL,               // no #define support
            NULL,               // no #include support
            compileFlags,       // standard compile options
            &microcode,
            &errors);

        if (FAILED(hr))
        {
            String message = "Cannot assemble D3D9 shader " + mName + " Errors:\n" +
                static_cast<const char*>(errors->GetBufferPointer());
            errors->Release();
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, message,
                "D3D9GpuProgram::loadFromSource");
        }
        else if ( GpuProgramManager::getSingleton().getSaveMicrocodesToCache() )
        {
            // create microcode
            GpuProgramManager::Microcode newMicrocode = 
                GpuProgramManager::getSingleton().createMicrocode(microcode->GetBufferSize());

            // save microcode
            memcpy(newMicrocode->getPtr(), microcode->GetBufferPointer(), microcode->GetBufferSize());

            // add to the microcode to the cache
            GpuProgramManager::getSingleton().addMicrocodeToCache(_getHash(), newMicrocode);
        }

        loadFromMicrocode(d3d9Device, microcode);       
        
        SAFE_RELEASE(microcode);
        SAFE_RELEASE(errors);
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr D3D9GpuProgram::createParameters(void)
    {
        // Call superclass
        GpuProgramParametersSharedPtr params = GpuProgram::createParameters();

        // Need to transpose matrices if compiled with column-major matrices
        params->setTransposeMatrices(mColumnMajorMatrices);

        return params;
    }   

    //-----------------------------------------------------------------------------
    IDirect3DVertexShader9* D3D9GpuVertexProgram::getVertexShader( void )
    {
        IDirect3DDevice9* d3d9Device = D3D9RenderSystem::getActiveD3D9Device();

        // Find the shader of this device.
        auto it = mMapDeviceToShader.find(d3d9Device);
        
        // Shader was not found -> load it.
        if (it == mMapDeviceToShader.end())
        {
            loadImpl(d3d9Device);
            it = mMapDeviceToShader.find(d3d9Device);
        }
    
        return static_cast<IDirect3DVertexShader9*>(it->second);
    }

    //-----------------------------------------------------------------------------
    void D3D9GpuProgram::loadFromMicrocode(IDirect3DDevice9* d3d9Device, ID3DXBuffer* microcode)
    {
        auto it = mMapDeviceToShader.find(d3d9Device);

        if (it != mMapDeviceToShader.end())
        {
            if(mType == GPT_VERTEX_PROGRAM)
                static_cast<IDirect3DVertexShader9*>(it->second)->Release();
            else
                static_cast<IDirect3DPixelShader9*>(it->second)->Release();
        }

        if (isSupported())
        {
            // Create the shader
            IUnknown* pShader;
            HRESULT hr;

            if (mType == GPT_VERTEX_PROGRAM)
                hr = d3d9Device->CreateVertexShader(static_cast<DWORD*>(microcode->GetBufferPointer()),
                                                    (IDirect3DVertexShader9**)&pShader);
            else
                hr = d3d9Device->CreatePixelShader(static_cast<DWORD*>(microcode->GetBufferPointer()),
                                                   (IDirect3DPixelShader9**)&pShader);

            if (FAILED(hr))
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                    "Cannot create D3D9 shader " + mName + " from microcode.");
            }

            mMapDeviceToShader[d3d9Device] = pShader;
        }
        else
        {
            LogManager::getSingleton().logMessage(
                "Unsupported D3D9 shader '" + mName + "' was not loaded.");

            mMapDeviceToShader[d3d9Device] = NULL;
        }
    }
    //-----------------------------------------------------------------------------
    void D3D9GpuProgram::unloadImpl(void)
    {
        D3D9_DEVICE_ACCESS_CRITICAL_SECTION

        auto it = mMapDeviceToShader.begin();

        while (it != mMapDeviceToShader.end())
        {
            if(mType == GPT_VERTEX_PROGRAM)
                static_cast<IDirect3DVertexShader9*>(it->second)->Release();
            else
                static_cast<IDirect3DPixelShader9*>(it->second)->Release();
            ++it;
        }
        mMapDeviceToShader.clear();
        SAFE_RELEASE(mExternalMicrocode);
    }
    //-----------------------------------------------------------------------------
    void D3D9GpuProgram::notifyOnDeviceCreate(IDirect3DDevice9* d3d9Device)
    {
        
    }

    //-----------------------------------------------------------------------------
    void D3D9GpuProgram::notifyOnDeviceDestroy(IDirect3DDevice9* d3d9Device)
    {
        D3D9_DEVICE_ACCESS_CRITICAL_SECTION

        // Find the shader of this device.
        auto it = mMapDeviceToShader.find(d3d9Device);

        // Case shader found -> release it and erase from map.
        if (it != mMapDeviceToShader.end())
        {
            if(mType == GPT_VERTEX_PROGRAM)
                static_cast<IDirect3DVertexShader9*>(it->second)->Release();
            else
                static_cast<IDirect3DPixelShader9*>(it->second)->Release();
            mMapDeviceToShader.erase(it);
        }
    }

    //-----------------------------------------------------------------------------
    IDirect3DPixelShader9* D3D9GpuProgram::getPixelShader()
    {
        IDirect3DDevice9* d3d9Device = D3D9RenderSystem::getActiveD3D9Device();

        // Find the shader of this device.
        auto it = mMapDeviceToShader.find(d3d9Device);

        // Shader was not found -> load it.
        if (it == mMapDeviceToShader.end())
        {
            loadImpl(d3d9Device);
            it = mMapDeviceToShader.find(d3d9Device);
        }

        return static_cast<IDirect3DPixelShader9*>(it->second);
    }
    //-----------------------------------------------------------------------------

}

