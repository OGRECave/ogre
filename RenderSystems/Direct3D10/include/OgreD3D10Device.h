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
#ifndef __D3D10DEVICE_H__
#define __D3D10DEVICE_H__


#include "OgreD3D10Prerequisites.h"

namespace Ogre
{
	class D3D10Device
	{
	private:
		ID3D10Device * mD3D10Device;

	public:
		D3D10Device();
		D3D10Device(ID3D10Device * device);
		
		inline ID3D10Device * operator->() const
		{
			assert(mD3D10Device); 
			if (D3D_NO_EXCEPTION != mExceptionsErrorLevel)
			{
				clearStoredErrorMessages();
			}
			return mD3D10Device;
		}

		inline const void clearStoredErrorMessages(  ) const
		{
			if (mD3D10Device && D3D_NO_EXCEPTION != mExceptionsErrorLevel)
			{
				ID3D10InfoQueue * pInfoQueue = NULL; 
				HRESULT hr = mD3D10Device->QueryInterface(__uuidof(ID3D10InfoQueue), (LPVOID*)&pInfoQueue);
				if (SUCCEEDED(hr))
				{
					pInfoQueue->ClearStoredMessages();
				}
			}
		}

		ID3D10Device * operator=(ID3D10Device * device);
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

		const bool _getErrorsFromQueue() const
		{
			ID3D10InfoQueue * pInfoQueue = NULL; 
			HRESULT hr = mD3D10Device->QueryInterface(__uuidof(ID3D10InfoQueue), (LPVOID*)&pInfoQueue);
			if (SUCCEEDED(hr))
			{
				unsigned int numStoredMessages = pInfoQueue->GetNumStoredMessages();

				if (D3D_INFO == mExceptionsErrorLevel && numStoredMessages > 0)
				{
					// if D3D_INFO we don't need to loop if the numStoredMessages > 0
					return true;
				}
				for (unsigned int i = 0 ; i < numStoredMessages ; i++ )
				{
					// Get the size of the message
					SIZE_T messageLength = 0;
					hr = pInfoQueue->GetMessage(i, NULL, &messageLength);
					// Allocate space and get the message
					D3D10_MESSAGE * pMessage = (D3D10_MESSAGE*)malloc(messageLength);
					hr = pInfoQueue->GetMessage(i, pMessage, &messageLength);

					bool res = false;
					switch(pMessage->Severity)
					{
					case D3D10_MESSAGE_SEVERITY_CORRUPTION:
						if (D3D_CORRUPTION == mExceptionsErrorLevel)
						{
							res = true;
						}
						break;
					case D3D10_MESSAGE_SEVERITY_ERROR:
						switch(mExceptionsErrorLevel)
						{
						case D3D_CORRUPTION :
						case D3D_ERROR:
							res = true;
						}
						break;
					case D3D10_MESSAGE_SEVERITY_WARNING:
						switch(mExceptionsErrorLevel)
						{
						case D3D_CORRUPTION :
						case D3D_ERROR:
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

				return false;

			}
			else
			{
				return false;
			}
		}
		void release();
		ID3D10Device * get();

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
