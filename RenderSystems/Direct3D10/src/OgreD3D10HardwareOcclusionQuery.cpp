/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#include "OgreD3D10HardwareOcclusionQuery.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreException.h"
#include "OgreD3D10Device.h"
namespace Ogre {

	/**
	* This is a class that is the DirectX9 implementation of 
	* hardware occlusion testing.
	*
	* @author Lee Sandberg
	*
	* Updated on 12/7/2004 by Chris McGuirk
	* Updated on 4/8/2005 by Tuan Kuranes email: tuan.kuranes@free.fr
	*/

	/**
	* Default object constructor
	*/
    D3D10HardwareOcclusionQuery::D3D10HardwareOcclusionQuery( D3D10Device & device ) :
        mDevice(device)
	{ 
		D3D10_QUERY_DESC queryDesc;
		queryDesc.Query = D3D10_QUERY_OCCLUSION;
		// create the occlusion query
		const HRESULT hr = mDevice->CreateQuery(&queryDesc, &mpQuery);

		if (FAILED(hr) || mDevice.isError()) 
		{	
			String errorDescription = mDevice.getErrorDescription(hr);
			OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
				"Cannot allocate a Hardware query. This video card doesn't supports it, sorry.\nError Description:" + errorDescription, 
				"D3D10HardwareOcclusionQuery::D3D10HardwareOcclusionQuery");
		}
	}

	/**
	* Object destructor
	*/
	D3D10HardwareOcclusionQuery::~D3D10HardwareOcclusionQuery() 
	{ 
		SAFE_RELEASE(mpQuery); 
	}

	//------------------------------------------------------------------
	// Occlusion query functions (see base class documentation for this)
	//--
	void D3D10HardwareOcclusionQuery::beginOcclusionQuery() 
	{	    	
		mpQuery->Begin();//Issue(D3DISSUE_BEGIN); 
        mIsQueryResultStillOutstanding = true;
        mPixelCount = 0;
	}

	void D3D10HardwareOcclusionQuery::endOcclusionQuery() 
	{ 
		mpQuery->End();//Issue(D3DISSUE_END); 
	}

	//------------------------------------------------------------------
	bool D3D10HardwareOcclusionQuery::pullOcclusionQuery( unsigned int* NumOfFragments ) 
	{
        // in case you didn't check if query arrived and want the result now.
        if (mIsQueryResultStillOutstanding)
        {
            // Loop until the data becomes available
            DWORD pixels;
            const size_t dataSize = sizeof( DWORD );
			while (1)
            {
                const HRESULT hr = mpQuery->GetData((void *)&pixels, dataSize, 0);//D3DGETDATA_FLUSH

                if  (hr == S_FALSE)
                    continue;
                if  (hr == S_OK)
                {
                    mPixelCount = pixels;
                    *NumOfFragments = pixels;
                    break;
                }
				//in directx10 the device will never be lost
               /* if (hr == D3DERR_DEVICELOST)
                {
                    *NumOfFragments = 100000;
                    mPixelCount = 100000;
					D3D10_QUERY_DESC queryDesc;
					queryDesc.Query = D3D10_QUERY_OCCLUSION;
					mDevice->CreateQuery(%queryDesc, &mpQuery);
                    break;
                }
				*/
            } 
            mIsQueryResultStillOutstanding = false;
        }
        else
        {
            // we already stored result from last frames.
            *NumOfFragments = mPixelCount;
        }		
		return true;
	}
    //------------------------------------------------------------------
    bool D3D10HardwareOcclusionQuery::isStillOutstanding(void)
    {       
        // in case you already asked for this query
        if (!mIsQueryResultStillOutstanding)
            return false;

        DWORD pixels;
        const HRESULT hr = mpQuery->GetData( (void *) &pixels, sizeof( DWORD ), 0);

        if (hr  == S_FALSE)
            return true;

       /* if (hr == D3DERR_DEVICELOST)
        {
            mPixelCount = 100000;
            D3D10_QUERY_DESC queryDesc;
			queryDesc.Query = D3D10_QUERY_OCCLUSION;
			mDevice->CreateQuery(&queryDesc, &mpQuery);
        }
		*/
        mPixelCount = pixels;
        mIsQueryResultStillOutstanding = false;
        return false;
    
    } 
}
