/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreD3D9GpuProgram.h"
#include "OgreMatrix4.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreD3D9Mappings.h"
#include "OgreResourceGroupManager.h"
#include "OgreD3D9RenderSystem.h"

namespace Ogre {

    //-----------------------------------------------------------------------------
    D3D9GpuProgram::D3D9GpuProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader) 
        : GpuProgram(creator, name, handle, group, isManual, loader), mpExternalMicrocode(NULL)
    {			
        if (createParamDictionary("D3D9GpuProgram"))
        {
            setupBaseParamDictionary();
        }
    }

	//-----------------------------------------------------------------------------
	D3D9GpuProgram::~D3D9GpuProgram()
	{

	}

	//-----------------------------------------------------------------------------
    void D3D9GpuProgram::loadImpl(void)
    {
		for (uint i = 0; i < D3D9RenderSystem::getResourceCreationDeviceCount(); ++i)
		{
			IDirect3DDevice9* d3d9Device = D3D9RenderSystem::getResourceCreationDevice(i);

			loadImpl(d3d9Device);
		}		       
    }

	//-----------------------------------------------------------------------------
	void D3D9GpuProgram::loadImpl(IDirect3DDevice9* d3d9Device)
	{
		if (mpExternalMicrocode)
		{
			loadFromMicrocode(d3d9Device, mpExternalMicrocode);
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
		for (uint i = 0; i < D3D9RenderSystem::getResourceCreationDeviceCount(); ++i)
		{
			IDirect3DDevice9* d3d9Device = D3D9RenderSystem::getResourceCreationDevice(i);

			loadFromSource(d3d9Device);
		}
    }

	//-----------------------------------------------------------------------------
	void D3D9GpuProgram::loadFromSource(IDirect3DDevice9* d3d9Device)
	{
		// Create the shader
		// Assemble source into microcode
		LPD3DXBUFFER microcode;
		LPD3DXBUFFER errors;
		HRESULT hr = D3DXAssembleShader(
			mSource.c_str(),
			static_cast<UINT>(mSource.length()),
			NULL,               // no #define support
			NULL,               // no #include support
			0,                  // standard compile options
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
	
		loadFromMicrocode(d3d9Device, microcode);		

		SAFE_RELEASE(microcode);
		SAFE_RELEASE(errors);
	}
	
	//-----------------------------------------------------------------------------
    D3D9GpuVertexProgram::D3D9GpuVertexProgram(ResourceManager* creator, 
        const String& name, ResourceHandle handle, const String& group, 
        bool isManual, ManualResourceLoader* loader) 
        : D3D9GpuProgram(creator, name, handle, group, isManual, loader)       
    {
        mType = GPT_VERTEX_PROGRAM;		
    }
	//-----------------------------------------------------------------------------
	D3D9GpuVertexProgram::~D3D9GpuVertexProgram()
	{
		// have to call this here reather than in Resource destructor
		// since calling virtual methods in base destructors causes crash
		unload(); 		
	}
	//-----------------------------------------------------------------------------
    void D3D9GpuVertexProgram::loadFromMicrocode(IDirect3DDevice9* d3d9Device, ID3DXBuffer* microcode)
    {		 
		DeviceToVertexShaderIterator it = mMapDeviceToVertexShader.find(d3d9Device);

		if (it != mMapDeviceToVertexShader.end())
			SAFE_RELEASE(it->second);

		if (isSupported())
		{
			// Create the shader
			IDirect3DVertexShader9* pVertexShader;
			HRESULT hr;
			
			hr = d3d9Device->CreateVertexShader( 
				static_cast<DWORD*>(microcode->GetBufferPointer()), 
				&pVertexShader);

			if (FAILED(hr))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Cannot create D3D9 vertex shader " + mName + " from microcode",
					"D3D9GpuVertexProgram::loadFromMicrocode");
	            
			}

			mMapDeviceToVertexShader[d3d9Device] = pVertexShader;
		}
		else
		{
			LogManager::getSingleton().logMessage(
				"Unsupported D3D9 vertex shader '" + mName + "' was not loaded.");

			mMapDeviceToVertexShader[d3d9Device] = NULL;
		}
    }
	//-----------------------------------------------------------------------------
    void D3D9GpuVertexProgram::unloadImpl(void)
    {
        DeviceToVertexShaderIterator it = mMapDeviceToVertexShader.begin();

		while (it != mMapDeviceToVertexShader.end())
		{
			SAFE_RELEASE(it->second);
			++it;
		}
		mMapDeviceToVertexShader.clear();		
    }

	//-----------------------------------------------------------------------------
	void D3D9GpuVertexProgram::notifyOnDeviceCreate(IDirect3DDevice9* d3d9Device)
	{
			
	}

	//-----------------------------------------------------------------------------
	void D3D9GpuVertexProgram::notifyOnDeviceDestroy(IDirect3DDevice9* d3d9Device)
	{
		DeviceToVertexShaderIterator it;

		// Find the shader of this device.
		it = mMapDeviceToVertexShader.find(d3d9Device);

		// Case shader found -> release it and erase from map.
		if (it != mMapDeviceToVertexShader.end())
		{
			SAFE_RELEASE(it->second);
			mMapDeviceToVertexShader.erase(it);
		}
	}

	//-----------------------------------------------------------------------------
	IDirect3DVertexShader9* D3D9GpuVertexProgram::getVertexShader( void )
	{
		IDirect3DDevice9* d3d9Device = D3D9RenderSystem::getActiveD3D9Device();
		DeviceToVertexShaderIterator it;

		// Find the shader of this device.
		it = mMapDeviceToVertexShader.find(d3d9Device);
		
		// Shader was not found -> load it.
		if (it == mMapDeviceToVertexShader.end())		
		{
			loadImpl(d3d9Device);		
			it = mMapDeviceToVertexShader.find(d3d9Device);
		}
	
		return it->second;
	}

	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
    D3D9GpuFragmentProgram::D3D9GpuFragmentProgram(ResourceManager* creator, 
        const String& name, ResourceHandle handle, const String& group, 
        bool isManual, ManualResourceLoader* loader) 
        : D3D9GpuProgram(creator, name, handle, group, isManual, loader)       
    {
        mType = GPT_FRAGMENT_PROGRAM;
    }
	//-----------------------------------------------------------------------------
	D3D9GpuFragmentProgram::~D3D9GpuFragmentProgram()
	{
		// have to call this here reather than in Resource destructor
		// since calling virtual methods in base destructors causes crash
		unload(); 
	}
	//-----------------------------------------------------------------------------
    void D3D9GpuFragmentProgram::loadFromMicrocode(IDirect3DDevice9* d3d9Device, ID3DXBuffer* microcode)
    {
		DeviceToPixelShaderIterator it = mMapDeviceToPixelShader.find(d3d9Device);

		if (it != mMapDeviceToPixelShader.end())
			SAFE_RELEASE(it->second);

		if (isSupported())
		{
			// Create the shader
			IDirect3DPixelShader9* pPixelShader;
			HRESULT hr;

			hr = d3d9Device->CreatePixelShader(
				static_cast<DWORD*>(microcode->GetBufferPointer()), 
				&pPixelShader);

			if (FAILED(hr))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Cannot create D3D9 pixel shader " + mName + " from microcode.",
					"D3D9GpuFragmentProgram::loadFromMicrocode");
	            
			}

			mMapDeviceToPixelShader[d3d9Device] = pPixelShader;
		}
		else
		{
			LogManager::getSingleton().logMessage(
				"Unsupported D3D9 pixel shader '" + mName + "' was not loaded.");

			mMapDeviceToPixelShader[d3d9Device] = NULL;
		}
    }
	//-----------------------------------------------------------------------------
    void D3D9GpuFragmentProgram::unloadImpl(void)
    {
		DeviceToPixelShaderIterator it = mMapDeviceToPixelShader.begin();

		while (it != mMapDeviceToPixelShader.end())
		{
			SAFE_RELEASE(it->second);
			++it;
		}
		mMapDeviceToPixelShader.clear();		
    }
	//-----------------------------------------------------------------------------
	void D3D9GpuFragmentProgram::notifyOnDeviceCreate(IDirect3DDevice9* d3d9Device)
	{
		

	}

	//-----------------------------------------------------------------------------
	void D3D9GpuFragmentProgram::notifyOnDeviceDestroy(IDirect3DDevice9* d3d9Device)
	{
		DeviceToPixelShaderIterator it;

		// Find the shader of this device.
		it = mMapDeviceToPixelShader.find(d3d9Device);

		// Case shader found -> release it and erase from map.
		if (it != mMapDeviceToPixelShader.end())
		{
			SAFE_RELEASE(it->second);
			mMapDeviceToPixelShader.erase(it);
		}
	}

	//-----------------------------------------------------------------------------
	IDirect3DPixelShader9* D3D9GpuFragmentProgram::getPixelShader( void )
	{
		IDirect3DDevice9* d3d9Device = D3D9RenderSystem::getActiveD3D9Device();
		DeviceToPixelShaderIterator it;

		// Find the shader of this device.
		it = mMapDeviceToPixelShader.find(d3d9Device);

		// Shader was not found -> load it.
		if (it == mMapDeviceToPixelShader.end())		
		{
			loadImpl(d3d9Device);			
			it = mMapDeviceToPixelShader.find(d3d9Device);
		}

		return it->second;
	}
	//-----------------------------------------------------------------------------

}

