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
    const char* D3D11Device::sDebugLevelUndefined = "Undefined";
    const char* D3D11Device::sDebugLevelNone = "No information queue exceptions";
    const char* D3D11Device::sDebugLevelMessage = "Message";
    const char* D3D11Device::sDebugLevelInfo = "Info";
    const char* D3D11Device::sDebugLevelWarning = "Warning";
    const char* D3D11Device::sDebugLevelError = "Error";
    const char* D3D11Device::sDebugLevelCorruption = "Corruption";
    //---------------------------------------------------------------------
    D3D11Device::D3D11Device()
        : mD3D11Device(NULL)
        , mImmediateContext(NULL)
        , mClassLinkage(NULL)
        , mInfoQueue(NULL)
        , mExceptionsErrorLevel(D3D11Device::DEL_WARNING)
        , mStorageErrorLevel(D3D11Device::DEL_WARNING)
        , mInfoQueueDirty(true)
#if OGRE_D3D11_PROFILING
        , mPerf(NULL)
#endif
    {
    }
    //---------------------------------------------------------------------
    D3D11Device::~D3D11Device()
    {
        ReleaseAll();
    }
    //---------------------------------------------------------------------
    void D3D11Device::ReleaseAll()
    {
        // Clear state
        if (mImmediateContext)
        {
            mImmediateContext->Flush();
            mImmediateContext->ClearState();
        }
#if OGRE_D3D11_PROFILING
        SAFE_RELEASE(mPerf);
#endif
        SAFE_RELEASE(mInfoQueue);
        SAFE_RELEASE(mClassLinkage);
        SAFE_RELEASE(mImmediateContext);
        SAFE_RELEASE(mD3D11Device);

        mInfoQueueDirty = true;
    }

    //---------------------------------------------------------------------
    void D3D11Device::generateSeverity(const eDebugErrorLevel severityLevel, SevrityList& severityList)
    {
        switch (severityLevel)
        {

#ifdef _WIN32_WINNT_WIN8
        case DEL_MESSAGE:
            severityList.push_back(D3D11_MESSAGE_SEVERITY_MESSAGE);
#endif
        case DEL_INFO:
            severityList.push_back(D3D11_MESSAGE_SEVERITY_INFO);
        case DEL_WARNING:
            severityList.push_back(D3D11_MESSAGE_SEVERITY_WARNING);
        case DEL_ERROR:
            severityList.push_back(D3D11_MESSAGE_SEVERITY_ERROR);
        case DEL_CORRUPTION:
            severityList.push_back(D3D11_MESSAGE_SEVERITY_CORRUPTION);
        case DEL_NO_EXCEPTION:
        default:
            break;
        }
    }
    //---------------------------------------------------------------------    
    void D3D11Device::TransferOwnership(ID3D11DeviceN* d3d11device)
    {
        assert(mD3D11Device != d3d11device);
        ReleaseAll();
        
        if (d3d11device)
        {
            HRESULT hr = S_OK;

            mD3D11Device = d3d11device;

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            mD3D11Device->GetImmediateContext(&mImmediateContext);
#elif OGRE_PLATFORM == OGRE_PLATFORM_WINRT
            mD3D11Device->GetImmediateContext1(&mImmediateContext);
#endif

#if OGRE_D3D11_PROFILING
            hr = mImmediateContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (LPVOID*)&mPerf);
            if (!mPerf->GetStatus())
                SAFE_RELEASE(mPerf);
#endif
            refreshInfoQueue();
        }
    }
    //---------------------------------------------------------------------
    void D3D11Device::refreshInfoQueue()
    {
        if (mInfoQueueDirty == true && mD3D11Device != NULL)
        {
            HRESULT hr = mD3D11Device->QueryInterface(__uuidof(ID3D11InfoQueue), (LPVOID*)&mInfoQueue);
            if (SUCCEEDED(hr))
            {
                mInfoQueue->ClearStoredMessages();
                mInfoQueue->ClearRetrievalFilter();
                mInfoQueue->ClearStorageFilter();

                D3D11_INFO_QUEUE_FILTER filter;
                ZeroMemory(&filter, sizeof(D3D11_INFO_QUEUE_FILTER));
                std::vector<D3D11_MESSAGE_SEVERITY> severityList;

                generateSeverity(mStorageErrorLevel, severityList);
                if (severityList.size() > 0)
                {
                    filter.AllowList.NumSeverities = severityList.size();
                    filter.AllowList.pSeverityList = &severityList[0];
                }
                mInfoQueue->AddStorageFilterEntries(&filter);

                severityList.clear();

                generateSeverity(mExceptionsErrorLevel, severityList);
                if (severityList.size() > 0)
                {
                    filter.AllowList.NumSeverities = severityList.size();
                    filter.AllowList.pSeverityList = &severityList[0];
                }
                mInfoQueue->AddRetrievalFilterEntries(&filter);
            }

            // If feature level is higher than 11, create class linkage
            if (mD3D11Device->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0)
            {
                hr = mD3D11Device->CreateClassLinkage(&mClassLinkage);
            }
            mInfoQueueDirty = false;
        }
    }
    //---------------------------------------------------------------------
    void D3D11Device::setExceptionsErrorLevel(const String& errorLevel)
    {
        setExceptionsErrorLevel(parseErrorLevel(errorLevel));
    }
    //---------------------------------------------------------------------
    void D3D11Device::setExceptionsErrorLevel(const eDebugErrorLevel errorLevel)
    {
        eDebugErrorLevel requestedErrorLevel = errorLevel;

#if OGRE_DEBUG_MODE == 1 && FORCE_REPORT_ERRORS_ON_DEBUG == 1
        // A debug build should always and report errors.
        if (requestedErrorLevel < DEL_ERROR)
            requestedErrorLevel = DEL_ERROR;
#endif        

        if (mStorageErrorLevel != requestedErrorLevel && errorLevel != DEL_UNDEFINED)
        {
            mStorageErrorLevel = requestedErrorLevel;
            markInfoQueueDirty();
        }
    }
    //---------------------------------------------------------------------
    void D3D11Device::markInfoQueueDirty()
    {
        mInfoQueueDirty = true;
        refreshInfoQueue();
    }
    //---------------------------------------------------------------------
    void D3D11Device::setStorageErrorLevel(const eDebugErrorLevel errorLevel)
    {
       if (mStorageErrorLevel != errorLevel && errorLevel != DEL_UNDEFINED)
        {
            mStorageErrorLevel = errorLevel;
            markInfoQueueDirty();
        }
    }
    //---------------------------------------------------------------------
    void D3D11Device::setStorageErrorLevel(const String& errorLevel)
    {
        setStorageErrorLevel(parseErrorLevel(errorLevel));
    }

    //---------------------------------------------------------------------
    const D3D11Device::eDebugErrorLevel D3D11Device::getExceptionsErrorLevel()
    {
        return mExceptionsErrorLevel;
    }
    const String D3D11Device::getStorageErrorLevelAsString()
    {
        return errorLevelToString(mStorageErrorLevel);
    }
    const String D3D11Device::getExceptionErrorLevelAsString()
    {
        return errorLevelToString(mExceptionsErrorLevel);
    }
    //---------------------------------------------------------------------
    D3D11Device::eDebugErrorLevel D3D11Device::getDebugErrorLevelFromSeverity(const D3D11_MESSAGE_SEVERITY severity) const
    {
        switch (severity)
        {
#ifdef _WIN32_WINNT_WIN8
        case D3D11_MESSAGE_SEVERITY_MESSAGE:
            return DEL_MESSAGE;
#endif
        case D3D11_MESSAGE_SEVERITY_INFO:
            return DEL_INFO;
        case D3D11_MESSAGE_SEVERITY_WARNING:
            return DEL_WARNING;
        case D3D11_MESSAGE_SEVERITY_ERROR:
            return DEL_ERROR;
        case D3D11_MESSAGE_SEVERITY_CORRUPTION:
            return DEL_CORRUPTION;
        default:
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "bad or currupted 'serverity' parameter",
                "D3D11Device::getExceptionLevelFromSeverity");
        }
    }
    //---------------------------------------------------------------------
    String D3D11Device::errorLevelToString(const D3D11Device::eDebugErrorLevel errorLevel)
    {
        switch (errorLevel)
        {
        case DEL_UNDEFINED:
            return sDebugLevelUndefined;
        case DEL_NO_EXCEPTION:
            return sDebugLevelNone;
        case DEL_CORRUPTION:
            return sDebugLevelCorruption;
        case DEL_ERROR:
            return sDebugLevelError;
        case DEL_WARNING:
            return sDebugLevelWarning;
        case DEL_INFO:
            return sDebugLevelInfo;
        case DEL_MESSAGE:
            return sDebugLevelMessage;
        default:
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "bad or currupted 'serverity' parameter",
                "D3D11Device::getExceptionLevelFromSeverity");
        }
  
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    D3D11Device::eDebugErrorLevel D3D11Device::parseErrorLevel(const String& errorLevel)
    {
        if (sDebugLevelNone == errorLevel)
            return DEL_NO_EXCEPTION;
        else if (sDebugLevelCorruption == errorLevel)
            return DEL_CORRUPTION;
        else if (sDebugLevelError == errorLevel)
            return DEL_ERROR;
        else if (sDebugLevelWarning == errorLevel)
            return DEL_WARNING;
        else if (sDebugLevelInfo == errorLevel)
            return DEL_INFO;
#ifdef _WIN32_WINNT_WIN8
        else if (sDebugLevelMessage == errorLevel)
            return  DEL_MESSAGE;
#endif
        else
            return DEL_UNDEFINED;
    }
    //---------------------------------------------------------------------
    String D3D11Device::getErrorDescription(const HRESULT lastResult /* = NO_ERROR */) const
    {
        if (!mD3D11Device)
        {
            return "NULL device";
        }

        if (DEL_NO_EXCEPTION == mExceptionsErrorLevel)
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
            UINT64 numStoredMessages = mInfoQueue->GetNumStoredMessagesAllowedByRetrievalFilter();
            for (UINT64 i = 0; i < numStoredMessages; i++)
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
    bool D3D11Device::_getErrorsFromQueue() const
    {
        if (mInfoQueue != NULL)
        {
            UINT64 numStoredMessages = mInfoQueue->GetNumStoredMessagesAllowedByRetrievalFilter();

            if (numStoredMessages > 0)
            {
                // If there are any messages in the retrieval queue and mExceptionsErrorLevel is
                // set to the highest level, then every message is treated as an error.
                // and there is no need to iterate over the stored messages.

                if (mExceptionsErrorLevel ==
#ifdef _WIN32_WINNT_WIN8
                    DEL_MESSAGE
#else
                    DEL_INFO
#endif
                    )
                    {
                        return true;
                    }
            }

            for (UINT64 i = 0; i < numStoredMessages; i++)
            {
                // Get the size of the message
                SIZE_T messageLength = 0;
                mInfoQueue->GetMessage(i, NULL, &messageLength);
                // Allocate space and get the message
                D3D11_MESSAGE * pMessage = (D3D11_MESSAGE*)malloc(messageLength);
                mInfoQueue->GetMessage(i, pMessage, &messageLength);
                eDebugErrorLevel exceptionLevel = getDebugErrorLevelFromSeverity(pMessage->Severity);
                free(pMessage);
                if (exceptionLevel <= mExceptionsErrorLevel)
                {
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
    //---------------------------------------------------------------------    
    void D3D11Device::clearStoredErrorMessages() const
    {
        if (mD3D11Device && DEL_NO_EXCEPTION != mExceptionsErrorLevel)
        {
            if (mInfoQueue != NULL)
            {
                mInfoQueue->ClearStoredMessages();
            }
        }
    }

}