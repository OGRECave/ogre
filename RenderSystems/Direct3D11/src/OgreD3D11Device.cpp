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
#include "OgreD3D11Device.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	D3D11Device::eExceptionsErrorLevel D3D11Device::mExceptionsErrorLevel = D3D11Device::D3D_NO_EXCEPTION;
	//---------------------------------------------------------------------
	D3D11Device::D3D11Device( ID3D11Device * D3D11device ) : mD3D11Device(D3D11device)
	{
		D3D11device->GetImmediateContext(&mImmediateContext);

		ID3D11InfoQueue * pInfoQueue = NULL; 
		HRESULT hr = mD3D11Device->QueryInterface(__uuidof(ID3D11InfoQueue), (LPVOID*)&pInfoQueue);

		if (SUCCEEDED(hr))
		{
			pInfoQueue->ClearStoredMessages();
			pInfoQueue->ClearRetrievalFilter();
			pInfoQueue->ClearStorageFilter();

			D3D11_INFO_QUEUE_FILTER filter;
			ZeroMemory(&filter, sizeof(D3D11_INFO_QUEUE_FILTER));
			std::vector<D3D11_MESSAGE_SEVERITY> severityList;

			switch(mExceptionsErrorLevel)
			{
			case D3D_NO_EXCEPTION:
				severityList.push_back(D3D11_MESSAGE_SEVERITY_CORRUPTION);
			case D3D_CORRUPTION:
				severityList.push_back(D3D11_MESSAGE_SEVERITY_ERROR);
			case D3D_ERROR:
				severityList.push_back(D3D11_MESSAGE_SEVERITY_WARNING);
			case D3D_WARNING:
			case D3D_INFO:
				severityList.push_back(D3D11_MESSAGE_SEVERITY_INFO);
			default: 
				break;
			}


			if (severityList.size() > 0)
			{
				filter.DenyList.NumSeverities = severityList.size();
				filter.DenyList.pSeverityList = &severityList[0];
			}

			pInfoQueue->AddStorageFilterEntries(&filter);
			pInfoQueue->AddRetrievalFilterEntries(&filter);
		}

	}
	//---------------------------------------------------------------------
	D3D11Device::D3D11Device() : mD3D11Device(0), mImmediateContext(0)
	{
	}
	//---------------------------------------------------------------------
	ID3D11Device * D3D11Device::operator=( ID3D11Device * D3D11device )
	{
		mD3D11Device = D3D11device; 
		if (D3D11device)
		{
			D3D11device->GetImmediateContext(&mImmediateContext);
		}
		return mD3D11Device;
	}
	//---------------------------------------------------------------------
	const bool D3D11Device::isNull()
	{
		return mD3D11Device == 0;
	}
	//---------------------------------------------------------------------
	const String D3D11Device::getErrorDescription(const HRESULT lastResult /* = NO_ERROR */) const
	{
		if (!mD3D11Device)
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

		ID3D11InfoQueue * pInfoQueue = NULL; 
		HRESULT hr = mD3D11Device->QueryInterface(__uuidof(ID3D11InfoQueue), (LPVOID*)&pInfoQueue);

		if (SUCCEEDED(hr))
		{
			UINT64 numStoredMessages = pInfoQueue->GetNumStoredMessages();
			for (UINT64 i = 0 ; i < numStoredMessages ; i++ )
			{
				// Get the size of the message
				SIZE_T messageLength = 0;
				hr = pInfoQueue->GetMessage(i, NULL, &messageLength);
				// Allocate space and get the message
				D3D11_MESSAGE * pMessage = (D3D11_MESSAGE*)malloc(messageLength);
				hr = pInfoQueue->GetMessage(i, pMessage, &messageLength);
				res = res + pMessage->pDescription + "\n";
				free(pMessage);
			}
		}

		return res;
	}
	//---------------------------------------------------------------------
	void D3D11Device::release()
	{
		SAFE_RELEASE(mImmediateContext);
		SAFE_RELEASE(mD3D11Device);
	}
	//---------------------------------------------------------------------
	ID3D11Device * D3D11Device::get()
	{
		return mD3D11Device;
	}
	//---------------------------------------------------------------------
	void D3D11Device::setExceptionsErrorLevel( const eExceptionsErrorLevel exceptionsErrorLevel )
	{
		mExceptionsErrorLevel = exceptionsErrorLevel;
	}
	//---------------------------------------------------------------------
	const D3D11Device::eExceptionsErrorLevel D3D11Device::getExceptionsErrorLevel()
	{
		return mExceptionsErrorLevel;
	}
	//---------------------------------------------------------------------
}