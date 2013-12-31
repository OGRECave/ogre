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
#ifndef __D3D11PREREQUISITES_H__
#define __D3D11PREREQUISITES_H__



#include "OgrePrerequisites.h"
#include "WIN32/OgreMinGWSupport.h" // extra defines for MinGW to deal with DX SDK

#if OGRE_THREAD_SUPPORT
#define OGRE_LOCK_RECURSIVE_MUTEX(name)   name.lock();
#define OGRE_UNLOCK_RECURSIVE_MUTEX(name) name.unlock();
#else
#define OGRE_LOCK_RECURSIVE_MUTEX(name) 
#define OGRE_UNLOCK_RECURSIVE_MUTEX(name)
#endif


#if OGRE_THREAD_SUPPORT == 1
#define D3D11_DEVICE_ACCESS_LOCK				OGRE_LOCK_RECURSIVE_MUTEX(msDeviceAccessMutex);
#define D3D11_DEVICE_ACCESS_UNLOCK			OGRE_UNLOCK_RECURSIVE_MUTEX(msDeviceAccessMutex);
#define D3D11_DEVICE_ACCESS_CRITICAL_SECTION	OGRE_LOCK_MUTEX(msDeviceAccessMutex)
#else
#define D3D11_DEVICE_ACCESS_LOCK	
#define D3D11_DEVICE_ACCESS_UNLOCK
#define D3D11_DEVICE_ACCESS_CRITICAL_SECTION
#endif

// some D3D commonly used macros
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }


#undef NOMINMAX
#define NOMINMAX // required to stop windows.h screwing up std::min definition
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#include <d3d11.h>
#elif OGRE_PLATFORM == OGRE_PLATFORM_WINRT
#include <d3d11_1.h>
#endif

#if (OGRE_PLATFORM == OGRE_PLATFORM_WINRT && OGRE_WINRT_TARGET_TYPE == PHONE)
#	include <C:\Program Files (x86)\Windows Kits\8.0\Include\um\d3d11shader.h>
#else
#	include <d3d11shader.h>
#	include <D3Dcompiler.h>
#endif
 

namespace Ogre
{
	// typedefs to work with Direct3D 11 or 11.1 as appropriate
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	typedef ID3D11Device			ID3D11DeviceN;
	typedef ID3D11DeviceContext		ID3D11DeviceContextN;
	typedef ID3D11RasterizerState	ID3D11RasterizerStateN;
	typedef IDXGIFactory1			IDXGIFactoryN;
	typedef IDXGIAdapter1			IDXGIAdapterN;
	typedef IDXGIDevice1			IDXGIDeviceN;
	typedef IDXGISwapChain			IDXGISwapChainN;
	typedef DXGI_SWAP_CHAIN_DESC	DXGI_SWAP_CHAIN_DESC_N;
#elif  OGRE_PLATFORM == OGRE_PLATFORM_WINRT
	typedef ID3D11Device1			ID3D11DeviceN;
	typedef ID3D11DeviceContext1	ID3D11DeviceContextN;
	typedef ID3D11RasterizerState1	ID3D11RasterizerStateN;
	typedef IDXGIFactory2			IDXGIFactoryN;
	typedef IDXGIAdapter1			IDXGIAdapterN;			// we don`t need IDXGIAdapter2 functionality
	typedef IDXGIDevice2			IDXGIDeviceN;
	typedef IDXGISwapChain1			IDXGISwapChainN;
	typedef DXGI_SWAP_CHAIN_DESC1	DXGI_SWAP_CHAIN_DESC_N;
#endif

	// Predefine classes
	class D3D11RenderSystem;
	class D3D11RenderWindowBase;
	class D3D11Texture;
	class D3D11TextureManager;
	class D3D11DepthBuffer;
	class D3D11Driver;
	class D3D11DriverList;
	class D3D11VideoMode;
	class D3D11VideoModeList;
	class D3D11GpuProgram;
	class D3D11GpuProgramManager;
	class D3D11HardwareBufferManager;
	class D3D11HardwareIndexBuffer;
	class D3D11HLSLProgramFactory;
	class D3D11HLSLProgram;
	class D3D11VertexDeclaration;
	class D3D11Device;
	class D3D11HardwareBuffer;
	class D3D11HardwarePixelBuffer;

    typedef SharedPtr<D3D11GpuProgram>  D3D11GpuProgramPtr;
    typedef SharedPtr<D3D11HLSLProgram> D3D11HLSLProgramPtr;
    typedef SharedPtr<D3D11Texture>     D3D11TexturePtr;

	//-------------------------------------------
	// Windows setttings
	//-------------------------------------------
#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT) && !defined(OGRE_STATIC_LIB)
#	ifdef OGRED3DENGINEDLL_EXPORTS
#		define _OgreD3D11Export __declspec(dllexport)
#	else
#       if defined( __MINGW32__ )
#           define _OgreD3D11Export
#       else
#    		define _OgreD3D11Export __declspec(dllimport)
#       endif
#	endif
#else
#	define _OgreD3D11Export
#endif	// OGRE_WIN32
}
#endif
