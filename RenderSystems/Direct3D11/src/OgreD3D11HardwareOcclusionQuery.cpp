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
#include "OgreD3D11HardwareOcclusionQuery.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreException.h"
#include "OgreD3D11Device.h"
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
    D3D11HardwareOcclusionQuery::D3D11HardwareOcclusionQuery( D3D11Device & device ) :
        mDevice(device)
	{ 
		D3D11_QUERY_DESC queryDesc;
		queryDesc.Query = D3D11_QUERY_OCCLUSION;
		// create the occlusion query
		const HRESULT hr = mDevice->CreateQuery(&queryDesc, &mpQuery);

		if (FAILED(hr) || mDevice.isError()) 
		{	
			String errorDescription = mDevice.getErrorDescription(hr);
			OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
				"Cannot allocate a Hardware query. This video card doesn't supports it, sorry.\nError Description:" + errorDescription, 
				"D3D11HardwareOcclusionQuery::D3D11HardwareOcclusionQuery");
		}
	}

	/**
	* Object destructor
	*/
	D3D11HardwareOcclusionQuery::~D3D11HardwareOcclusionQuery() 
	{ 
		SAFE_RELEASE(mpQuery); 
	}

	//------------------------------------------------------------------
	// Occlusion query functions (see base class documentation for this)
	//--
	void D3D11HardwareOcclusionQuery::beginOcclusionQuery() 
	{	    	
		mDevice.GetImmediateContext()->Begin(mpQuery);//Issue(D3DISSUE_BEGIN); 
        mIsQueryResultStillOutstanding = true;
        mPixelCount = 0;
	}

	void D3D11HardwareOcclusionQuery::endOcclusionQuery() 
	{ 
		mDevice.GetImmediateContext()->End(mpQuery);//Issue(D3DISSUE_END); 
	}

	//------------------------------------------------------------------
	bool D3D11HardwareOcclusionQuery::pullOcclusionQuery( unsigned int* NumOfFragments ) 
	{
        // in case you didn't check if query arrived and want the result now.
        if (mIsQueryResultStillOutstanding)
        {
            // Loop until the data becomes available
            DWORD pixels;
            const size_t dataSize = sizeof( DWORD );
			while (1)
            {
                const HRESULT hr = mDevice.GetImmediateContext()->GetData(mpQuery, (void *)&pixels, dataSize, 0);//D3DGETDATA_FLUSH

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
					D3D11_QUERY_DESC queryDesc;
					queryDesc.Query = D3D11_QUERY_OCCLUSION;
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
    bool D3D11HardwareOcclusionQuery::isStillOutstanding(void)
    {       
        // in case you already asked for this query
        if (!mIsQueryResultStillOutstanding)
            return false;

        DWORD pixels;
        const HRESULT hr = mDevice.GetImmediateContext()->GetData(mpQuery, (void *) &pixels, sizeof( DWORD ), 0);

        if (hr  == S_FALSE)
            return true;

       /* if (hr == D3DERR_DEVICELOST)
        {
            mPixelCount = 100000;
            D3D11_QUERY_DESC queryDesc;
			queryDesc.Query = D3D11_QUERY_OCCLUSION;
			mDevice->CreateQuery(&queryDesc, &mpQuery);
        }
		*/
        mPixelCount = pixels;
        mIsQueryResultStillOutstanding = false;
        return false;
    
    } 
}
