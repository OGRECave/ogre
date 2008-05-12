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

namespace Ogre {

    //-----------------------------------------------------------------------------
    D3D9GpuProgram::D3D9GpuProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader, LPDIRECT3DDEVICE9 pDev) 
        : GpuProgram(creator, name, handle, group, isManual, loader), 
        mpDevice(pDev), mpExternalMicrocode(NULL)
    {
        if (createParamDictionary("D3D9GpuProgram"))
        {
            setupBaseParamDictionary();
        }
    }
	//-----------------------------------------------------------------------------
    void D3D9GpuProgram::loadImpl(void)
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
    void D3D9GpuProgram::loadFromSource(void)
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

        loadFromMicrocode(microcode);

        SAFE_RELEASE(microcode);
        SAFE_RELEASE(errors);
    }
	//-----------------------------------------------------------------------------
    D3D9GpuVertexProgram::D3D9GpuVertexProgram(ResourceManager* creator, 
        const String& name, ResourceHandle handle, const String& group, 
        bool isManual, ManualResourceLoader* loader, LPDIRECT3DDEVICE9 pDev) 
        : D3D9GpuProgram(creator, name, handle, group, isManual, loader, pDev)
        , mpVertexShader(NULL)
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
    void D3D9GpuVertexProgram::loadFromMicrocode(LPD3DXBUFFER microcode)
    {
		if (isSupported())
		{
			// Create the shader
			HRESULT hr = mpDevice->CreateVertexShader( 
				static_cast<DWORD*>(microcode->GetBufferPointer()), 
				&mpVertexShader);

			if (FAILED(hr))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Cannot create D3D9 vertex shader " + mName + " from microcode",
					"D3D9GpuVertexProgram::loadFromMicrocode");
	            
			}
		}
		else
		{
			LogManager::getSingleton().logMessage(
				"Unsupported D3D9 vertex shader '" + mName + "' was not loaded.");
		}
    }
	//-----------------------------------------------------------------------------
    void D3D9GpuVertexProgram::unloadImpl(void)
    {
        SAFE_RELEASE(mpVertexShader);
    }
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
    D3D9GpuFragmentProgram::D3D9GpuFragmentProgram(ResourceManager* creator, 
        const String& name, ResourceHandle handle, const String& group, 
        bool isManual, ManualResourceLoader* loader, LPDIRECT3DDEVICE9 pDev) 
        : D3D9GpuProgram(creator, name, handle, group, isManual, loader, pDev)
        , mpPixelShader(NULL)
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
    void D3D9GpuFragmentProgram::loadFromMicrocode(LPD3DXBUFFER microcode)
    {
		if (isSupported())
		{
			// Create the shader
			HRESULT hr = mpDevice->CreatePixelShader(
				static_cast<DWORD*>(microcode->GetBufferPointer()), 
				&mpPixelShader);

			if (FAILED(hr))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Cannot create D3D9 pixel shader " + mName + " from microcode.",
					"D3D9GpuFragmentProgram::loadFromMicrocode");
	            
			}
		}
		else
		{
			LogManager::getSingleton().logMessage(
				"Unsupported D3D9 pixel shader '" + mName + "' was not loaded.");
		}
    }
	//-----------------------------------------------------------------------------
    void D3D9GpuFragmentProgram::unloadImpl(void)
    {
        SAFE_RELEASE(mpPixelShader);
    }
	//-----------------------------------------------------------------------------

}

