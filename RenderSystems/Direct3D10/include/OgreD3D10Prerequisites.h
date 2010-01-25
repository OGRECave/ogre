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
#ifndef __D3D10PREREQUISITES_H__
#define __D3D10PREREQUISITES_H__

#include "OgrePrerequisites.h"
#include "WIN32/OgreMinGWSupport.h" // extra defines for MinGW to deal with DX SDK

// Define versions for if DirectX is in use (Win32 only)
#define DIRECT3D_VERSION 0x0900

// some D3D commonly used macros
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }


#if !defined(NOMINMAX) && defined(_MSC_VER)
#	define NOMINMAX // required to stop windows.h messing up std::min
#endif
#include <d3d10.h>
#include <d3dx10.h>
#include <d3d10_1shader.h>


namespace Ogre
{
	// Predefine classes
	class D3D10RenderSystem;
	class D3D10RenderWindow;
	class D3D10Texture;
	class D3D10TextureManager;
	class D3D10Driver;
	class D3D10DriverList;
	class D3D10VideoMode;
	class D3D10VideoModeList;
	class D3D10GpuProgram;
	class D3D10GpuProgramManager;
	class D3D10HardwareBufferManager;
	class D3D10HardwareIndexBuffer;
	class D3D10HLSLProgramFactory;
	class D3D10HLSLProgram;
	class D3D10VertexDeclaration;
	class D3D10Device;
	class D3D10HardwareBuffer;
	class D3D10HardwarePixelBuffer;

	// Should we ask D3D to manage vertex/index buffers automatically?
	// Doing so avoids lost devices, but also has a performance impact
	// which is unacceptably bad when using very large buffers
#define OGRE_D3D_MANAGE_BUFFERS 1

	//-------------------------------------------
	// Windows setttings
	//-------------------------------------------
#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32) && !defined(OGRE_STATIC_LIB)
#	ifdef OGRED3DENGINEDLL_EXPORTS
#		define _OgreD3D10Export __declspec(dllexport)
#	else
#       if defined( __MINGW32__ )
#           define _OgreD3D10Export
#       else
#    		define _OgreD3D10Export __declspec(dllimport)
#       endif
#	endif
#else
#	define _OgreD3D10Export
#endif	// OGRE_WIN32
}
#endif
