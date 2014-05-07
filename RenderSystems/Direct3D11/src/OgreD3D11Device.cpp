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
#include "OgreD3D11Device.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	D3D11Device::eExceptionsErrorLevel D3D11Device::mExceptionsErrorLevel = D3D11Device::D3D_NO_EXCEPTION;
	//---------------------------------------------------------------------
	D3D11Device::D3D11Device( ID3D11DeviceN * D3D11device )
		: mD3D11Device(D3D11device)
		, mImmediateContext(0)
		, mInfoQueue(NULL)
		, mClassLinkage(0)
	{
        if (D3D11device)
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		    D3D11device->GetImmediateContext(&mImmediateContext);
#elif OGRE_PLATFORM == OGRE_PLATFORM_WINRT
		    D3D11device->GetImmediateContext1(&mImmediateContext);
#endif
            HRESULT hr = mD3D11Device->QueryInterface(__uuidof(ID3D11InfoQueue), (LPVOID*)&mInfoQueue);

		    if (SUCCEEDED(hr))
		    {
			    mInfoQueue->ClearStoredMessages();
			    mInfoQueue->ClearRetrievalFilter();
			    mInfoQueue->ClearStorageFilter();

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

			    mInfoQueue->AddStorageFilterEntries(&filter);
			    mInfoQueue->AddRetrievalFilterEntries(&filter);
		    }

			// If feature level is 11, create class linkage
			SAFE_RELEASE(mClassLinkage)
			if (mD3D11Device->GetFeatureLevel() == D3D_FEATURE_LEVEL_11_0)
			{
				HRESULT hr = mD3D11Device->CreateClassLinkage(&mClassLinkage);
			}
        }	
	}
	//---------------------------------------------------------------------
	D3D11Device::D3D11Device() : mD3D11Device(0), mImmediateContext(0), mClassLinkage(0)
	{
	}
	//---------------------------------------------------------------------
	D3D11Device::~D3D11Device()
	{
	}
	//---------------------------------------------------------------------
	ID3D11DeviceN * D3D11Device::operator=( ID3D11DeviceN * D3D11device )
	{
		mD3D11Device = D3D11device; 
		if (D3D11device)
		{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			D3D11device->GetImmediateContext(&mImmediateContext);
#elif OGRE_PLATFORM == OGRE_PLATFORM_WINRT
		    D3D11device->GetImmediateContext1(&mImmediateContext);
#endif			
			
			// If feature level is 11, create class linkage
			SAFE_RELEASE(mClassLinkage);
			if (mD3D11Device->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0)
			{
				HRESULT hr = mD3D11Device->CreateClassLinkage(&mClassLinkage);
			}
			
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
			res.append("invalid parameters were passed.\n");
			break;
		default:
			{
			char tmp[64];
			sprintf(tmp, "hr = 0x%08X\n", lastResult);
			res.append(tmp);
			}
		}

		if (mInfoQueue != NULL)
		{
			UINT64 numStoredMessages = mInfoQueue->GetNumStoredMessages();
			for (UINT64 i = 0 ; i < numStoredMessages ; i++ )
			{
				// Get the size of the message
				SIZE_T messageLength = 0;
				mInfoQueue->GetMessage(i, NULL, &messageLength);
				// Allocate space and get the message
				D3D11_MESSAGE * pMessage = (D3D11_MESSAGE*)malloc(messageLength);
				mInfoQueue->GetMessage(i, pMessage, &messageLength);
				res = res + pMessage->pDescription + "\n";
				free(pMessage);
			}
		}

		return res;
	}
	//---------------------------------------------------------------------
	void D3D11Device::release()
	{
		// Clear state
		if (mImmediateContext)
		{
			mImmediateContext->Flush();
			mImmediateContext->ClearState();
		}
        SAFE_RELEASE(mInfoQueue);
		SAFE_RELEASE(mD3D11Device);
        SAFE_RELEASE(mImmediateContext);
		SAFE_RELEASE(mClassLinkage);
	}
	//---------------------------------------------------------------------
	ID3D11DeviceN * D3D11Device::get()
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
    const bool D3D11Device::_getErrorsFromQueue() const
    {
        if (mInfoQueue != NULL)
        {
            UINT64 numStoredMessages = mInfoQueue->GetNumStoredMessages();

            if (D3D_INFO == mExceptionsErrorLevel && numStoredMessages > 0)
            {
                // if D3D_INFO we don't need to loop if the numStoredMessages > 0
                return true;
            }
            for (UINT64 i = 0 ; i < numStoredMessages ; i++ )
            {
                // Get the size of the message
                SIZE_T messageLength = 0;
                mInfoQueue->GetMessage(i, NULL, &messageLength);
                // Allocate space and get the message
                D3D11_MESSAGE * pMessage = (D3D11_MESSAGE*)malloc(messageLength);
                mInfoQueue->GetMessage(i, pMessage, &messageLength);

                bool res = false;
                switch(pMessage->Severity)
                {
                case D3D11_MESSAGE_SEVERITY_CORRUPTION:
                    if (D3D_CORRUPTION == mExceptionsErrorLevel)
                    {
                        res = true;
                    }
                    break;
                case D3D11_MESSAGE_SEVERITY_ERROR:
                    switch(mExceptionsErrorLevel)
                    {
                    case D3D_INFO:
                    case D3D_WARNING:
                    case D3D_ERROR:
                        res = true;
                    }
                    break;
                case D3D11_MESSAGE_SEVERITY_WARNING:
                    switch(mExceptionsErrorLevel)
                    {
                    case D3D_INFO:
                    case D3D_WARNING:
                        res = true;
                    }
                    break;
                }

                free(pMessage);
                if (res)
                {
                    // we don't need to loop anymore...
                    return true;
                }

            }

            clearStoredErrorMessages();

            return false;

        }
        else
        {
            return false;
        }
    }

    const void D3D11Device::clearStoredErrorMessages() const
    {
        if (mD3D11Device && D3D_NO_EXCEPTION != mExceptionsErrorLevel)
        {
            if (mInfoQueue != NULL)
            {
                mInfoQueue->ClearStoredMessages();
            }
        }
    }

	//---------------------------------------------------------------------
}