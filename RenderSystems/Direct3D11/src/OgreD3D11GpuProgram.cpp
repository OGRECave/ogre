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
#include "OgreD3D11GpuProgram.h"
#include "OgreD3D11Mappings.h"
#include "OgreD3D11Device.h"
#include "OgreException.h"

namespace Ogre {
	//-----------------------------------------------------------------------------
	D3D11GpuProgram::D3D11GpuProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
		const String& group, bool isManual, ManualResourceLoader* loader, D3D11Device & device) 
		: GpuProgram(creator, name, handle, group, isManual, loader), 
		mDevice(device)
	{
		if (createParamDictionary("D3D11GpuProgram"))
		{
			setupBaseParamDictionary();
		}
	}
	//-----------------------------------------------------------------------------
	void D3D11GpuProgram::loadImpl(void)
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
		loadFromSource();
	}
	//-----------------------------------------------------------------------------
	void D3D11GpuProgram::loadFromSource(void)
	{
		String message = "AIZ:D3D11 dosn't support assembly shaders. Shader name:" + mName + "\n" ;
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, message,
			"D3D11GpuProgram::loadFromSource");
	}
	//-----------------------------------------------------------------------------
	D3D11GpuVertexProgram::D3D11GpuVertexProgram(ResourceManager* creator, 
		const String& name, ResourceHandle handle, const String& group, 
		bool isManual, ManualResourceLoader* loader, D3D11Device & device) 
		: D3D11GpuProgram(creator, name, handle, group, isManual, loader, device)
		, mVertexShader(NULL)
	{
		mType = GPT_VERTEX_PROGRAM;
	}
	//-----------------------------------------------------------------------------
	D3D11GpuVertexProgram::~D3D11GpuVertexProgram()
	{
		// have to call this here reather than in Resource destructor
		// since calling virtual methods in base destructors causes crash
		unload(); 
	}
	//-----------------------------------------------------------------------------
	void D3D11GpuVertexProgram::loadFromMicrocode(ID3D10Blob *  microcode)
	{
		if (isSupported())
		{
			// Create the shader
			HRESULT hr = mDevice->CreateVertexShader( 
				static_cast<DWORD*>(microcode->GetBufferPointer()), 
				microcode->GetBufferSize(),
				NULL,
				&mVertexShader);

			if (FAILED(hr) || mDevice.isError())
			{
				String errorDescription = mDevice.getErrorDescription(hr);
				OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
					"Cannot create D3D11 vertex shader " + mName + " from microcode\nError Description:" + errorDescription,
					"D3D11GpuVertexProgram::loadFromMicrocode");

			}
		}
		else
		{
			LogManager::getSingleton().logMessage(
				"Unsupported D3D11 vertex shader '" + mName + "' was not loaded.");
		}
	}
	//-----------------------------------------------------------------------------
	void D3D11GpuVertexProgram::unloadImpl(void)
	{
		SAFE_RELEASE(mVertexShader);
	}
	//-----------------------------------------------------------------------------
	ID3D11VertexShader * D3D11GpuVertexProgram::getVertexShader( void ) const
	{
		return mVertexShader;
	}
	//-----------------------------------------------------------------------------
	D3D11GpuFragmentProgram::D3D11GpuFragmentProgram(ResourceManager* creator, 
		const String& name, ResourceHandle handle, const String& group, 
		bool isManual, ManualResourceLoader* loader, D3D11Device & device) 
		: D3D11GpuProgram(creator, name, handle, group, isManual, loader, device)
		, mPixelShader(NULL)
	{
		mType = GPT_FRAGMENT_PROGRAM;
	}
	//-----------------------------------------------------------------------------
	D3D11GpuFragmentProgram::~D3D11GpuFragmentProgram()
	{
		// have to call this here reather than in Resource destructor
		// since calling virtual methods in base destructors causes crash
		unload(); 
	}
	//-----------------------------------------------------------------------------
	void D3D11GpuFragmentProgram::loadFromMicrocode(ID3D10Blob *  microcode)
	{
		if (isSupported())
		{
			// Create the shader
			HRESULT hr = mDevice->CreatePixelShader(
				static_cast<DWORD*>(microcode->GetBufferPointer()), 
				microcode->GetBufferSize(),
				NULL,
				&mPixelShader);

			if (FAILED(hr) || mDevice.isError())
			{
				String errorDescription = mDevice.getErrorDescription(hr);
				OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
					"Cannot create D3D11 pixel shader " + mName + " from microcode.\nError Description:" + errorDescription,
					"D3D11GpuFragmentProgram::loadFromMicrocode");
			}
		}
		else
		{
			LogManager::getSingleton().logMessage(
				"Unsupported D3D11 pixel shader '" + mName + "' was not loaded.");
		}
	}
	//-----------------------------------------------------------------------------
	void D3D11GpuFragmentProgram::unloadImpl(void)
	{
		SAFE_RELEASE(mPixelShader);
	}
	//-----------------------------------------------------------------------------
	ID3D11PixelShader * D3D11GpuFragmentProgram::getPixelShader( void ) const
	{
		return mPixelShader;
	}
	//-----------------------------------------------------------------------------
	D3D11GpuGeometryProgram::D3D11GpuGeometryProgram(ResourceManager* creator, 
		const String& name, ResourceHandle handle, const String& group, 
		bool isManual, ManualResourceLoader* loader, D3D11Device & device) 
		: D3D11GpuProgram(creator, name, handle, group, isManual, loader, device)
		, mGeometryShader(NULL)
	{
		mType = GPT_GEOMETRY_PROGRAM;
	}
	//-----------------------------------------------------------------------------
	D3D11GpuGeometryProgram::~D3D11GpuGeometryProgram()
	{
		// have to call this here reather than in Resource destructor
		// since calling virtual methods in base destructors causes crash
		unload(); 
	}
	//-----------------------------------------------------------------------------
	void D3D11GpuGeometryProgram::loadFromMicrocode(ID3D10Blob *  microcode)
	{
		if (isSupported())
		{
			// Create the shader
			HRESULT hr = mDevice->CreateGeometryShader(
				static_cast<DWORD*>(microcode->GetBufferPointer()), 
				microcode->GetBufferSize(),
				NULL,
				&mGeometryShader);

			if (FAILED(hr) || mDevice.isError())
			{
				String errorDescription = mDevice.getErrorDescription(hr);
				OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
					"Cannot create D3D11 geometry shader " + mName + " from microcode.\nError Description:" + errorDescription,
					"D3D11GpuGeometryProgram::loadFromMicrocode");
			}
		}
		else
		{
			LogManager::getSingleton().logMessage(
				"Unsupported D3D11 geometry shader '" + mName + "' was not loaded.");
		}
	}
	//-----------------------------------------------------------------------------
	void D3D11GpuGeometryProgram::unloadImpl(void)
	{
		SAFE_RELEASE(mGeometryShader);
	}
	//-----------------------------------------------------------------------------
	ID3D11GeometryShader * D3D11GpuGeometryProgram::getGeometryShader( void ) const
	{
		return mGeometryShader;
	}
	//-----------------------------------------------------------------------------
	D3D11GpuDomainProgram::D3D11GpuDomainProgram(ResourceManager* creator, 
		const String& name, ResourceHandle handle, const String& group, 
		bool isManual, ManualResourceLoader* loader, D3D11Device & device) 
		: D3D11GpuProgram(creator, name, handle, group, isManual, loader, device)
		, mDomainShader(NULL)
	{
		mType = GPT_DOMAIN_PROGRAM;
	}
	//-----------------------------------------------------------------------------
	D3D11GpuDomainProgram::~D3D11GpuDomainProgram()
	{
		// have to call this here reather than in Resource destructor
		// since calling virtual methods in base destructors causes crash
		unload(); 
	}
	//-----------------------------------------------------------------------------
	void D3D11GpuDomainProgram::loadFromMicrocode(ID3D10Blob *  microcode)
	{
		if (isSupported())
		{
			// Create the shader
			HRESULT hr = mDevice->CreateDomainShader(
				static_cast<DWORD*>(microcode->GetBufferPointer()), 
				microcode->GetBufferSize(),
				NULL,
				&mDomainShader);

			if (FAILED(hr) || mDevice.isError())
			{
				String errorDescription = mDevice.getErrorDescription(hr);
				OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
					"Cannot create D3D11 domain shader " + mName + " from microcode.\nError Description:" + errorDescription,
					"D3D11GpuFragmentProgram::loadFromMicrocode");
			}
		}
		else
		{
			LogManager::getSingleton().logMessage(
				"Unsupported D3D11 domain shader '" + mName + "' was not loaded.");
		}
	}
	//-----------------------------------------------------------------------------
	void D3D11GpuDomainProgram::unloadImpl(void)
	{
		SAFE_RELEASE(mDomainShader);
	}
	//-----------------------------------------------------------------------------
	ID3D11DomainShader * D3D11GpuDomainProgram::getDomainShader( void ) const
	{
		return mDomainShader;
	}
	//-----------------------------------------------------------------------------
	D3D11GpuHullProgram::D3D11GpuHullProgram(ResourceManager* creator, 
		const String& name, ResourceHandle handle, const String& group, 
		bool isManual, ManualResourceLoader* loader, D3D11Device & device) 
		: D3D11GpuProgram(creator, name, handle, group, isManual, loader, device)
		, mHullShader(NULL)
	{
		mType = GPT_HULL_PROGRAM;
	}
	//-----------------------------------------------------------------------------
	D3D11GpuHullProgram::~D3D11GpuHullProgram()
	{
		// have to call this here reather than in Resource destructor
		// since calling virtual methods in base destructors causes crash
		unload(); 
	}
	//-----------------------------------------------------------------------------
	void D3D11GpuHullProgram::loadFromMicrocode(ID3D10Blob *  microcode)
	{
		if (isSupported())
		{
			// Create the shader
			HRESULT hr = mDevice->CreateHullShader(
				static_cast<DWORD*>(microcode->GetBufferPointer()), 
				microcode->GetBufferSize(),
				NULL,
				&mHullShader);

			if (FAILED(hr) || mDevice.isError())
			{
				String errorDescription = mDevice.getErrorDescription(hr);
				OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
					"Cannot create D3D11 hull shader " + mName + " from microcode.\nError Description:" + errorDescription,
					"D3D11GpuFragmentProgram::loadFromMicrocode");
			}
		}
		else
		{
			LogManager::getSingleton().logMessage(
				"Unsupported D3D11 hull shader '" + mName + "' was not loaded.");
		}
	}
	//-----------------------------------------------------------------------------
	void D3D11GpuHullProgram::unloadImpl(void)
	{
		SAFE_RELEASE(mHullShader);
	}
	//-----------------------------------------------------------------------------
	ID3D11HullShader * D3D11GpuHullProgram::getHullShader( void ) const
	{
		return mHullShader;
	}
	//-----------------------------------------------------------------------------
}

