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
#include "OgreComPtr.h"       // too much resource leaks were caused without it by throwing constructors

#include "OgreException.h"

#ifdef OGRE_EXCEPT_EX
#undef OGRE_EXCEPT_EX
#endif

#define OGRE_EXCEPT_EX(code, num, desc, src) throw Ogre::D3D11RenderingAPIException(num, desc, src, __FILE__, __LINE__)

#define OGRE_CHECK_DX_ERROR(dxcall) \
{ \
    HRESULT hr = dxcall; \
    if (FAILED(hr) || mDevice.isError()) \
    { \
        String desc = mDevice.getErrorDescription(hr); \
        throw Ogre::D3D11RenderingAPIException(hr, desc, __FUNCTION__, __FILE__, __LINE__); \
    } \
}

// some D3D commonly used macros
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

#if defined(_WIN32_WINNT_WIN8) // Win8 SDK required to compile, will work on Windows 8 and Platform Update for Windows 7
#define OGRE_D3D11_PROFILING OGRE_PROFILING
#endif

#undef NOMINMAX
#define NOMINMAX // required to stop windows.h screwing up std::min definition
#if OGRE_PLATFORM == OGRE_PLATFORM_WINRT || OGRE_D3D11_PROFILING
#include <d3d11_1.h>
#else
#include <d3d11.h>
#endif

#if __OGRE_WINRT_PHONE_80
#   include <C:\Program Files (x86)\Windows Kits\8.0\Include\um\d3d11shader.h>
#else
#   include <d3d11shader.h>
#   include <d3dcompiler.h>
#endif
 

namespace Ogre
{
    // typedefs to work with Direct3D 11 or 11.1 as appropriate
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    typedef ID3D11Device            ID3D11DeviceN;
    typedef ID3D11DeviceContext     ID3D11DeviceContextN;
    typedef ID3D11RasterizerState   ID3D11RasterizerStateN;
    typedef IDXGIFactory1           IDXGIFactoryN;
    typedef IDXGIAdapter1           IDXGIAdapterN;
    typedef IDXGIDevice1            IDXGIDeviceN;
    typedef IDXGISwapChain          IDXGISwapChainN;
    typedef DXGI_SWAP_CHAIN_DESC    DXGI_SWAP_CHAIN_DESC_N;
#elif  OGRE_PLATFORM == OGRE_PLATFORM_WINRT
    typedef ID3D11Device1           ID3D11DeviceN;
    typedef ID3D11DeviceContext1    ID3D11DeviceContextN;
    typedef ID3D11RasterizerState1  ID3D11RasterizerStateN;
    typedef IDXGIFactory2           IDXGIFactoryN;
    typedef IDXGIAdapter1           IDXGIAdapterN;          // we don`t need IDXGIAdapter2 functionality
    typedef IDXGIDevice2            IDXGIDeviceN;
    typedef IDXGISwapChain1         IDXGISwapChainN;
    typedef DXGI_SWAP_CHAIN_DESC1   DXGI_SWAP_CHAIN_DESC_N;
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
    class GpuProgramManager;
    class D3D11HardwareBufferManager;
    class D3D11HardwareIndexBuffer;
    class D3D11HLSLProgramFactory;
    class D3D11HLSLProgram;
    class D3D11VertexDeclaration;
    class D3D11Device;
    class D3D11HardwareBuffer;
    class D3D11HardwarePixelBuffer;
    class D3D11RenderTarget;

    typedef SharedPtr<D3D11HLSLProgram> D3D11HLSLProgramPtr;
    typedef SharedPtr<D3D11Texture>     D3D11TexturePtr;

    //-------------------------------------------
    // Windows setttings
    //-------------------------------------------
#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT) && !defined(OGRE_STATIC_LIB)
#   ifdef OGRED3DENGINEDLL_EXPORTS
#       define _OgreD3D11Export __declspec(dllexport)
#   else
#       if defined( __MINGW32__ )
#           define _OgreD3D11Export
#       else
#           define _OgreD3D11Export __declspec(dllimport)
#       endif
#   endif
#else
#   define _OgreD3D11Export
#endif  // OGRE_WIN32

    class _OgreD3D11Export D3D11RenderingAPIException : public RenderingAPIException
    {
        int hresult;
    public:
        D3D11RenderingAPIException(int hr, const String& inDescription, const String& inSource, const char* inFile, long inLine)
            : RenderingAPIException(hr, inDescription, inSource, inFile, inLine), hresult(hr) {}

        int getHResult() const { return hresult; }

        const String& getFullDescription(void) const {
            StringStream ss;
            ss << RenderingAPIException::getFullDescription() << " HRESULT=0x" << std::hex << hresult;
            fullDesc = ss.str();
            return fullDesc;
        }
    };
}
#endif
