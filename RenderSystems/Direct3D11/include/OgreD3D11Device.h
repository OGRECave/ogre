/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2012 Torus Knot Software Ltd

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
	class D3D11Device
	{
	private:
		ID3D11Device * mD3D11Device;
		ID3D11DeviceContext * mImmediateContext;
        ID3D11InfoQueue * mInfoQueue; 

		struct ThreadInfo
		{
			ID3D11DeviceContext* mContext;
			void* mEventHandle;

			ThreadInfo(ID3D11DeviceContext* context)
				: mContext(context)
				, mEventHandle(0)
			{
				mEventHandle = CreateEvent(0, false, false, "ThreadContextEvent");
			}

			~ThreadInfo()
			{
				mContext->Release();
				mContext = 0;

				CloseHandle(mEventHandle);
			}
		};

		D3D11Device();
    public:


		D3D11Device(ID3D11Device * device);

		~D3D11Device();

		inline ID3D11DeviceContext * GetImmediateContext()
		{
			return mImmediateContext;
		}
		
		inline ID3D11Device * operator->() const
		{
			assert(mD3D11Device); 
			if (D3D_NO_EXCEPTION != mExceptionsErrorLevel)
			{
				clearStoredErrorMessages();
			}
			return mD3D11Device;
		}

		const void clearStoredErrorMessages(  ) const;

		ID3D11Device * operator=(ID3D11Device * device);
		const bool isNull();
		const String getErrorDescription(const HRESULT hr = NO_ERROR) const;

		inline const bool isError(  ) const
		{
			if (D3D_NO_EXCEPTION == mExceptionsErrorLevel)
			{
				return  false;
			}

			return _getErrorsFromQueue();
		}

		const bool _getErrorsFromQueue() const;
		void release();
		ID3D11Device * get();

		enum eExceptionsErrorLevel
		{
			D3D_NO_EXCEPTION,
			D3D_CORRUPTION,
			D3D_ERROR,
			D3D_WARNING,
			D3D_INFO,
		};

		static eExceptionsErrorLevel mExceptionsErrorLevel;
		static void setExceptionsErrorLevel(const eExceptionsErrorLevel exceptionsErrorLevel);
		static const eExceptionsErrorLevel getExceptionsErrorLevel();


	};
}
#endif
