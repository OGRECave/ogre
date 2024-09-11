/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#include "OgreD3D9HardwareOcclusionQuery.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreException.h"
#include "OgreD3D9RenderSystem.h"

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
    D3D9HardwareOcclusionQuery::D3D9HardwareOcclusionQuery()
    { 
        
    }

    /**
    * Object destructor
    */
    D3D9HardwareOcclusionQuery::~D3D9HardwareOcclusionQuery() 
    { 
        DeviceToQueryIterator it = mMapDeviceToQuery.begin();

        while (it != mMapDeviceToQuery.end())
        {
            SAFE_RELEASE(it->second);
            ++it;
        }   
        mMapDeviceToQuery.clear();
    }

    //------------------------------------------------------------------
    // Occlusion query functions (see base class documentation for this)
    //--
    void D3D9HardwareOcclusionQuery::beginOcclusionQuery() 
    {   
        IDirect3DDevice9* pCurDevice  = D3D9RenderSystem::getActiveD3D9Device();                
        DeviceToQueryIterator it      = mMapDeviceToQuery.find(pCurDevice);

        // No resource exits for current device -> create it.
        if (it == mMapDeviceToQuery.end() || it->second == NULL)        
            createQuery(pCurDevice);            
        

        // Grab the query of the current device.
        IDirect3DQuery9* pOccQuery = mMapDeviceToQuery[pCurDevice];

        
        if (pOccQuery != NULL)
        {
            pOccQuery->Issue(D3DISSUE_BEGIN); 
            mIsQueryResultStillOutstanding = true;
        }       
    }

    void D3D9HardwareOcclusionQuery::endOcclusionQuery() 
    { 
        IDirect3DDevice9* pCurDevice  = D3D9RenderSystem::getActiveD3D9Device();                
        DeviceToQueryIterator it      = mMapDeviceToQuery.find(pCurDevice);
        
        if (it == mMapDeviceToQuery.end())
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                "End occlusion called without matching begin call !!", 
                "D3D9HardwareOcclusionQuery::endOcclusionQuery" );
        }

        IDirect3DQuery9* pOccQuery = mMapDeviceToQuery[pCurDevice];

        if (pOccQuery != NULL)
            pOccQuery->Issue(D3DISSUE_END); 
    }   

    //------------------------------------------------------------------
    bool D3D9HardwareOcclusionQuery::pullOcclusionQuery( unsigned int* NumOfFragments ) 
    {
        IDirect3DDevice9* pCurDevice = D3D9RenderSystem::getActiveD3D9Device();
        DeviceToQueryIterator it     = mMapDeviceToQuery.find(pCurDevice);

        if (it == mMapDeviceToQuery.end())      
            return false;

        if (it->second == NULL)
            return false;

        // in case you didn't check if query arrived and want the result now.
        if (mIsQueryResultStillOutstanding)
        {
            // Loop until the data becomes available
            DWORD pixels;
            const size_t dataSize = sizeof( DWORD );
            while (1)
            {
                const HRESULT hr = it->second->GetData((void *)&pixels, dataSize, D3DGETDATA_FLUSH);

                if  (hr == S_FALSE)
                    continue;
                if  (hr == S_OK)
                {
                    mPixelCount = pixels;
                    *NumOfFragments = pixels;
                    break;
                }
                if (hr == D3DERR_DEVICELOST)
                {
                    *NumOfFragments = 0;
                    SAFE_RELEASE(it->second);
                    break;
                }
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
    bool D3D9HardwareOcclusionQuery::isStillOutstanding(void)
    {       
        // in case you already asked for this query
        if (!mIsQueryResultStillOutstanding)
            return false;

        IDirect3DDevice9* pCurDevice = D3D9RenderSystem::getActiveD3D9Device();
        DeviceToQueryIterator it     = mMapDeviceToQuery.find(pCurDevice);

        if (it == mMapDeviceToQuery.end())      
            return false;

        if (it->second == NULL)
            return false;


        DWORD pixels;
        const HRESULT hr = it->second->GetData( (void *) &pixels, sizeof( DWORD ), 0);

        if (hr  == S_FALSE)
            return true;

        if (hr == D3DERR_DEVICELOST)
        {
            mPixelCount = 100000;  
            SAFE_RELEASE(it->second);
        }

        mPixelCount = pixels;
        mIsQueryResultStillOutstanding = false;
        return false;
    }

    //------------------------------------------------------------------
    void D3D9HardwareOcclusionQuery::notifyOnDeviceCreate(IDirect3DDevice9* d3d9Device)
    {
        
    }

    //------------------------------------------------------------------
    void D3D9HardwareOcclusionQuery::notifyOnDeviceDestroy(IDirect3DDevice9* d3d9Device)
    {       
        releaseQuery(d3d9Device);
    }

    //------------------------------------------------------------------
    void D3D9HardwareOcclusionQuery::notifyOnDeviceLost(IDirect3DDevice9* d3d9Device)
    {       
        releaseQuery(d3d9Device);       
    }

    //------------------------------------------------------------------
    void D3D9HardwareOcclusionQuery::notifyOnDeviceReset(IDirect3DDevice9* d3d9Device)
    {       
        
    }

    //------------------------------------------------------------------
    void D3D9HardwareOcclusionQuery::createQuery(IDirect3DDevice9* d3d9Device)
    {
        HRESULT hr;

        // Check if query supported.
        hr = d3d9Device->CreateQuery(D3DQUERYTYPE_OCCLUSION, NULL);
        if (FAILED(hr))
        {
            mMapDeviceToQuery[d3d9Device] = NULL;
            return;
        }

        // create the occlusion query.
        IDirect3DQuery9*  pCurQuery;
        hr = d3d9Device->CreateQuery(D3DQUERYTYPE_OCCLUSION, &pCurQuery);
                
        mMapDeviceToQuery[d3d9Device] = pCurQuery;  
    }

    //------------------------------------------------------------------
    void D3D9HardwareOcclusionQuery::releaseQuery(IDirect3DDevice9* d3d9Device)
    {
        DeviceToQueryIterator it     = mMapDeviceToQuery.find(d3d9Device);

        // Remove from query resource map.
        if (it != mMapDeviceToQuery.end())      
        {
            SAFE_RELEASE(it->second);
            mMapDeviceToQuery.erase(it);
        }
    }
}
