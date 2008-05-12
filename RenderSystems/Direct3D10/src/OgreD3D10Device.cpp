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