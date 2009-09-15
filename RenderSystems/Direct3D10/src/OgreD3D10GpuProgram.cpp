/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#include "OgreD3D10GpuProgram.h"
#include "OgreD3D10Mappings.h"
#include "OgreD3D10Device.h"
#include "OgreException.h"

namespace Ogre {
	//-----------------------------------------------------------------------------
	D3D10GpuProgram::D3D10GpuProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
		const String& group, bool isManual, ManualResourceLoader* loader, D3D10Device & device) 
		: GpuProgram(creator, name, handle, group, isManual, loader), 
		mDevice(device), mpExternalMicrocode(NULL)
	{
		if (createParamDictionary("D3D10GpuProgram"))
		{
			setupBaseParamDictionary();
		}
	}
	//-----------------------------------------------------------------------------
	void D3D10GpuProgram::loadImpl(void)
	{
		if (mpExternalMicrocode)
		{
			loadFromMicrocode(mpExternalMicrocode);
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
			loadFromSource();
		}

	}
	//-----------------------------------------------------------------------------
	void D3D10GpuProgram::loadFromSource(void)
	{
		String message = "AIZ:D3D10 dosn't support assembly shaders. Shader name:" + mName + "\n" ;
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, message,
			"D3D10GpuProgram::loadFromSource");

		/*  // Create the shader
		// Assemble source into microcode
		ID3D10Blob *  microcode;
		ID3D10Blob *  errors;
		HRESULT hr = D3DX10CompileFromMemory(
		mSource.c_str(),
		static_cast<UINT>(mSource.length()),
		mFilename.c_str(),
		NULL,               // no #define support
		NULL,               // no #include support
		0,                  // standard compile options
		&microcode,
		&errors);

		if (FAILED(hr))
		{
		String message = "Cannot assemble D3D10 shader " + mName + " Errors:\n" +
		static_cast<const char*>(errors->GetBufferPointer());
		errors->Release();
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, message,
		"D3D10GpuProgram::loadFromSource");

		}

		loadFromMicrocode(microcode);

		SAFE_RELEASE(microcode);
		SAFE_RELEASE(errors);
		*/
	}
	//-----------------------------------------------------------------------------
	void D3D10GpuProgram::setExternalMicrocode( ID3D10Blob * pMicrocode )
	{
		mpExternalMicrocode = pMicrocode;
	}
	//-----------------------------------------------------------------------------
	ID3D10Blob * D3D10GpuProgram::getExternalMicrocode( void )
	{
		return mpExternalMicrocode;
	}
	//-----------------------------------------------------------------------------
	D3D10GpuVertexProgram::D3D10GpuVertexProgram(ResourceManager* creator, 
		const String& name, ResourceHandle handle, const String& group, 
		bool isManual, ManualResourceLoader* loader, D3D10Device & device) 
		: D3D10GpuProgram(creator, name, handle, group, isManual, loader, device)
		, mpVertexShader(NULL)
	{
		mType = GPT_VERTEX_PROGRAM;
	}
	//-----------------------------------------------------------------------------
	D3D10GpuVertexProgram::~D3D10GpuVertexProgram()
	{
		// have to call this here reather than in Resource destructor
		// since calling virtual methods in base destructors causes crash
		unload(); 
	}
	//-----------------------------------------------------------------------------
	void D3D10GpuVertexProgram::loadFromMicrocode(ID3D10Blob *  microcode)
	{
		if (isSupported())
		{
			// Create the shader
			HRESULT hr = mDevice->CreateVertexShader( 
				static_cast<DWORD*>(microcode->GetBufferPointer()), 
				microcode->GetBufferSize(),
				&mpVertexShader);

			if (FAILED(hr) || mDevice.isError())
			{
				String errorDescription = mDevice.getErrorDescription(hr);
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Cannot create D3D10 vertex shader " + mName + " from microcode\nError Description:" + errorDescription,
					"D3D10GpuVertexProgram::loadFromMicrocode");

			}
		}
		else
		{
			LogManager::getSingleton().logMessage(
				"Unsupported D3D10 vertex shader '" + mName + "' was not loaded.");
		}
	}
	//-----------------------------------------------------------------------------
	void D3D10GpuVertexProgram::unloadImpl(void)
	{
		SAFE_RELEASE(mpVertexShader);
	}
	//-----------------------------------------------------------------------------
	ID3D10VertexShader * D3D10GpuVertexProgram::getVertexShader( void ) const
	{
		return mpVertexShader;
	}
	//-----------------------------------------------------------------------------
	D3D10GpuFragmentProgram::D3D10GpuFragmentProgram(ResourceManager* creator, 
		const String& name, ResourceHandle handle, const String& group, 
		bool isManual, ManualResourceLoader* loader, D3D10Device & device) 
		: D3D10GpuProgram(creator, name, handle, group, isManual, loader, device)
		, mpPixelShader(NULL)
	{
		mType = GPT_FRAGMENT_PROGRAM;
	}
	//-----------------------------------------------------------------------------
	D3D10GpuFragmentProgram::~D3D10GpuFragmentProgram()
	{
		// have to call this here reather than in Resource destructor
		// since calling virtual methods in base destructors causes crash
		unload(); 
	}
	//-----------------------------------------------------------------------------
	void D3D10GpuFragmentProgram::loadFromMicrocode(ID3D10Blob *  microcode)
	{
		if (isSupported())
		{
			// Create the shader
			HRESULT hr = mDevice->CreatePixelShader(
				static_cast<DWORD*>(microcode->GetBufferPointer()), 
				microcode->GetBufferSize(),
				&mpPixelShader);

			if (FAILED(hr) || mDevice.isError())
			{
				String errorDescription = mDevice.getErrorDescription(hr);
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Cannot create D3D10 pixel shader " + mName + " from microcode.\nError Description:" + errorDescription,
					"D3D10GpuFragmentProgram::loadFromMicrocode");
			}
		}
		else
		{
			LogManager::getSingleton().logMessage(
				"Unsupported D3D10 pixel shader '" + mName + "' was not loaded.");
		}
	}
	//-----------------------------------------------------------------------------
	void D3D10GpuFragmentProgram::unloadImpl(void)
	{
		SAFE_RELEASE(mpPixelShader);
	}
	//-----------------------------------------------------------------------------
	ID3D10PixelShader * D3D10GpuFragmentProgram::getPixelShader( void ) const
	{
		return mpPixelShader;
	}
	//-----------------------------------------------------------------------------

}

