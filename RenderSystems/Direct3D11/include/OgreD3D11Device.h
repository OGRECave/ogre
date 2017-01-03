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
#ifndef __D3D11DEVICE_H__
#define __D3D11DEVICE_H__


#include "OgreD3D11Prerequisites.h"

namespace Ogre
{
    class _OgreD3D11Export D3D11Device
    {
    private:
        ComPtr<ID3D11DeviceN>           mD3D11Device;
        ComPtr<ID3D11DeviceContextN>    mImmediateContext;
        ComPtr<ID3D11ClassLinkage>      mClassLinkage;
        ComPtr<ID3D11InfoQueue>         mInfoQueue;
        LARGE_INTEGER                   mDriverVersion;
        ComPtr<IDXGIFactoryN>           mDXGIFactory;
#if OGRE_D3D11_PROFILING
        ComPtr<ID3DUserDefinedAnnotation> mPerf;
#endif

        D3D11Device(const D3D11Device& device); /* intentionally not implemented */
        const D3D11Device& operator=(D3D11Device& device); /* intentionally not implemented */

    public:
        D3D11Device();
        ~D3D11Device();

        void ReleaseAll();
        void TransferOwnership(ID3D11DeviceN* device);
        bool IsDeviceLost();

        bool isNull()                                { return !mD3D11Device; }
        ID3D11DeviceN* get()                         { return mD3D11Device.Get(); }
        ID3D11DeviceContextN* GetImmediateContext()  { return mImmediateContext.Get(); }
        ID3D11ClassLinkage* GetClassLinkage()        { return mClassLinkage.Get(); }
        IDXGIFactoryN* GetDXGIFactory()              { return mDXGIFactory.Get(); }
        LARGE_INTEGER GetDriverVersion()             { return mDriverVersion; }
#if OGRE_D3D11_PROFILING
        ID3DUserDefinedAnnotation* GetProfiler()     { return mPerf.Get(); }
#endif
        
        ID3D11DeviceN* operator->() const
        {
            assert(mD3D11Device); 
            if (D3D_NO_EXCEPTION != mExceptionsErrorLevel)
            {
                clearStoredErrorMessages();
            }
            return mD3D11Device.Get();
        }

        void throwIfFailed(HRESULT hr, const char* desc, const char* src);
        void throwIfFailed(const char* desc, const char* src) { throwIfFailed(NO_ERROR, desc, src); }

        String getErrorDescription(const HRESULT hr = NO_ERROR) const;
        void clearStoredErrorMessages() const;
        bool _getErrorsFromQueue() const;

        bool isError() const                         { return (D3D_NO_EXCEPTION == mExceptionsErrorLevel) ? false : _getErrorsFromQueue(); }

        enum eExceptionsErrorLevel
        {
            D3D_NO_EXCEPTION,
            D3D_CORRUPTION,
            D3D_ERROR,
            D3D_WARNING,
            D3D_INFO,
        };

        static eExceptionsErrorLevel mExceptionsErrorLevel;
        static const eExceptionsErrorLevel getExceptionsErrorLevel();
        static void setExceptionsErrorLevel(const eExceptionsErrorLevel exceptionsErrorLevel);
        static void setExceptionsErrorLevel(const Ogre::String& exceptionsErrorLevel);

        static D3D_FEATURE_LEVEL parseFeatureLevel(const Ogre::String& value, D3D_FEATURE_LEVEL fallback);
        static D3D_DRIVER_TYPE parseDriverType(const Ogre::String& value, D3D_DRIVER_TYPE fallback = D3D_DRIVER_TYPE_HARDWARE);
    };
}
#endif
