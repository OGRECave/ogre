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
#include "OgreD3D10Device.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	D3D10Device::eExceptionsErrorLevel D3D10Device::mExceptionsErrorLevel = D3D10Device::D3D_NO_EXCEPTION;
	//---------------------------------------------------------------------
	D3D10Device::D3D10Device( ID3D10Device * D3D10device ) : mD3D10Device(D3D10device)
	{

	}
	//---------------------------------------------------------------------
	D3D10Device::D3D10Device() : mD3D10Device(0)
	{
	}
	//---------------------------------------------------------------------
	ID3D10Device * D3D10Device::operator=( ID3D10Device * D3D10device )
	{
		mD3D10Device = D3D10device; 
		return mD3D10Device;
	}
	//---------------------------------------------------------------------
	const bool D3D10Device::isNull()
	{
		return mD3D10Device == 0;
	}
	//---------------------------------------------------------------------
	const String D3D10Device::getErrorDescription(const HRESULT lastResult /* = NO_ERROR */) const
	{
		if (!mD3D10Device)
		{
			return "NULL device";
		}

		if (D3D_NO_EXCEPTION == mExceptionsErrorLevel)
		{
			return "infoQ exceptions are turned off";
		}

		String res;

		switch (lastResult)
		{
		case NO_ERROR:
			break;
		case E_INVALIDARG:
			res = res + "invalid parameters were passed.\n";
			break;
		default:
			assert(false); // unknown HRESULT
		}

		ID3D10InfoQueue * pInfoQueue = NULL; 
		HRESULT hr = mD3D10Device->QueryInterface(__uuidof(ID3D10InfoQueue), (LPVOID*)&pInfoQueue);

		if (SUCCEEDED(hr))
		{
			unsigned int numStoredMessages = pInfoQueue->GetNumStoredMessages();
			for (unsigned int i = 0 ; i < numStoredMessages ; i++ )
			{
				// Get the size of the message
				SIZE_T messageLength = 0;
				hr = pInfoQueue->GetMessage(i, NULL, &messageLength);
				// Allocate space and get the message
				D3D10_MESSAGE * pMessage = (D3D10_MESSAGE*)malloc(messageLength);
				hr = pInfoQueue->GetMessage(i, pMessage, &messageLength);
				res = res + pMessage->pDescription + "\n";
				free(pMessage);
			}
		}

		return res;
	}
	//---------------------------------------------------------------------
	void D3D10Device::release()
	{
		SAFE_RELEASE(mD3D10Device);
	}
	//---------------------------------------------------------------------
	ID3D10Device * D3D10Device::get()
	{
		return mD3D10Device;
	}
	//---------------------------------------------------------------------
	void D3D10Device::setExceptionsErrorLevel( const eExceptionsErrorLevel exceptionsErrorLevel )
	{
		mExceptionsErrorLevel = exceptionsErrorLevel;
	}
	//---------------------------------------------------------------------
	const D3D10Device::eExceptionsErrorLevel D3D10Device::getExceptionsErrorLevel()
	{
		return mExceptionsErrorLevel;
	}
	//---------------------------------------------------------------------
}