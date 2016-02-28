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
#include "OgreD3D9VertexDeclaration.h"
#include "OgreD3D9Mappings.h"
#include "OgreException.h"
#include "OgreRoot.h"
#include "OgreD3D9RenderSystem.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    D3D9VertexDeclaration::D3D9VertexDeclaration() : mLastUsedGlobalDeclaration(0), mUsedGlobalDeclaration(false)
    {

    }
    //-----------------------------------------------------------------------
    D3D9VertexDeclaration::~D3D9VertexDeclaration()
    {
       releaseDeclaration();
    }
    //-----------------------------------------------------------------------
    const VertexElement& D3D9VertexDeclaration::addElement(unsigned short source, 
        size_t offset, VertexElementType theType,
        VertexElementSemantic semantic, unsigned short index)
    {
        releaseDeclaration();   
        return VertexDeclaration::addElement(source, offset, theType, semantic, index);
    }
    //-----------------------------------------------------------------------------
    const VertexElement& D3D9VertexDeclaration::insertElement(unsigned short atPosition,
        unsigned short source, size_t offset, VertexElementType theType,
        VertexElementSemantic semantic, unsigned short index)
    {
        releaseDeclaration();   
        return VertexDeclaration::insertElement(atPosition, source, offset, theType, semantic, index);
    }
    //-----------------------------------------------------------------------
    void D3D9VertexDeclaration::removeElement(unsigned short elem_index)
    {
        VertexDeclaration::removeElement(elem_index);
        releaseDeclaration();   
    }
    //-----------------------------------------------------------------------
    void D3D9VertexDeclaration::removeElement(VertexElementSemantic semantic, unsigned short index)
    {
        VertexDeclaration::removeElement(semantic, index);
        releaseDeclaration();   
    }
    //-----------------------------------------------------------------------
    void D3D9VertexDeclaration::removeAllElements(void)
    {
        VertexDeclaration::removeAllElements();
        releaseDeclaration();   
    }
    //-----------------------------------------------------------------------
    void D3D9VertexDeclaration::modifyElement(unsigned short elem_index, 
        unsigned short source, size_t offset, VertexElementType theType,
        VertexElementSemantic semantic, unsigned short index)
    {
        VertexDeclaration::modifyElement(elem_index, source, offset, theType, semantic, index);
        releaseDeclaration();       
    }

     //-----------------------------------------------------------------------
    void D3D9VertexDeclaration::notifyOnDeviceCreate(IDirect3DDevice9* d3d9Device)
    {
        
    }

     //-----------------------------------------------------------------------
    void D3D9VertexDeclaration::notifyOnDeviceDestroy(IDirect3DDevice9* d3d9Device)
    {
        D3D9_DEVICE_ACCESS_CRITICAL_SECTION

        DeviceToDeclarationIterator it = mMapDeviceToDeclaration.find(d3d9Device);

        if (it != mMapDeviceToDeclaration.end())    
        {
            SAFE_RELEASE(it->second);   
            mMapDeviceToDeclaration.erase(it);
        }
    }

    //-----------------------------------------------------------------------
    IDirect3DVertexDeclaration9* D3D9VertexDeclaration::getD3DVertexDeclaration(
            VertexDeclaration * globalDeclaration, bool useGlobalInstancingVertexBufferIsAvailable)
    {
        if (mLastUsedGlobalDeclaration != globalDeclaration ||
            useGlobalInstancingVertexBufferIsAvailable != mUsedGlobalDeclaration )
        {
            releaseDeclaration();
            mLastUsedGlobalDeclaration = globalDeclaration;
            mUsedGlobalDeclaration = useGlobalInstancingVertexBufferIsAvailable;
        }

        IDirect3DDevice9* pCurDevice   = D3D9RenderSystem::getActiveD3D9Device();
        DeviceToDeclarationIterator it = mMapDeviceToDeclaration.find(pCurDevice);
        IDirect3DVertexDeclaration9* lpVertDecl = NULL;

        // Case we have to create the declaration for this device.
        if (it == mMapDeviceToDeclaration.end() || it->second == NULL)
        {
            size_t d3delemsSize = mElementList.size() + 1;
            if(mLastUsedGlobalDeclaration != NULL && mUsedGlobalDeclaration )
            {
                d3delemsSize += globalDeclaration->getElementCount(); 
            }
            D3DVERTEXELEMENT9* d3delems = OGRE_ALLOC_T(D3DVERTEXELEMENT9, d3delemsSize, MEMCATEGORY_RENDERSYS);

            VertexElementList::const_iterator i, iend;
            unsigned int idx;
            size_t maxSource = 0;
            iend = mElementList.end();
            for (idx = 0, i = mElementList.begin(); i != iend; ++i, ++idx)
            {
                const VertexElement & element = *i;
                D3DVERTEXELEMENT9 & dxElement = d3delems[idx];
                convertElement(element, dxElement);
                if(element.getSource() > maxSource)
                {
                    maxSource = element.getSource();
                }
            }

            if(mLastUsedGlobalDeclaration != NULL && mUsedGlobalDeclaration )
            {
                iend = globalDeclaration->getElements().end();
                for (i = globalDeclaration->getElements().begin(); i != iend; ++i, ++idx)
                {
                    const VertexElement & element = *i;
                    D3DVERTEXELEMENT9 & dxElement = d3delems[idx];
                    convertElement(element, dxElement);
                    dxElement.Stream = maxSource + 1;
                }                
            }


            // Add terminator
            d3delems[idx].Stream = 0xff;
            d3delems[idx].Offset = 0;
            d3delems[idx].Type = D3DDECLTYPE_UNUSED;
            d3delems[idx].Method = 0;
            d3delems[idx].Usage = 0;
            d3delems[idx].UsageIndex = 0;

            
            HRESULT hr = pCurDevice->CreateVertexDeclaration(d3delems, &lpVertDecl);

            if (FAILED(hr))
            {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                    "Cannot create D3D9 vertex declaration: " + 
                    Root::getSingleton().getErrorDescription(hr), 
                    "Direct3D9VertexDeclaration::getD3DVertexDeclaration");
            }

            OGRE_FREE(d3delems, MEMCATEGORY_RENDERSYS);

            mMapDeviceToDeclaration[pCurDevice] = lpVertDecl;
        }

        // Declaration already exits.
        else
        {
            lpVertDecl = mMapDeviceToDeclaration[pCurDevice];
        }
        
        return lpVertDecl;
    }
    //-----------------------------------------------------------------------
    void D3D9VertexDeclaration::releaseDeclaration()
    {
        D3D9_DEVICE_ACCESS_CRITICAL_SECTION

        DeviceToDeclarationIterator it = mMapDeviceToDeclaration.begin();

        while (it != mMapDeviceToDeclaration.end())
        {
            SAFE_RELEASE(it->second);
            ++it;
        }   
        mMapDeviceToDeclaration.clear();
    }
    //-----------------------------------------------------------------------
    void D3D9VertexDeclaration::convertElement( const VertexElement & element, D3DVERTEXELEMENT9 & dxElement )
    {
        dxElement.Method = D3DDECLMETHOD_DEFAULT;
        dxElement.Offset = static_cast<WORD>(element.getOffset());
        dxElement.Stream = element.getSource();
        dxElement.Type = D3D9Mappings::get(element.getType());
        dxElement.Usage = D3D9Mappings::get(element.getSemantic());
        // NB force index if colours since D3D uses the same usage for 
        // diffuse & specular
        if (element.getSemantic() == VES_SPECULAR)
        {
            dxElement.UsageIndex = 1;
        }
        else if (element.getSemantic() == VES_DIFFUSE)
        {
            dxElement.UsageIndex = 0;
        }
        else
        {
            dxElement.UsageIndex = static_cast<BYTE>(element.getIndex());
        }
    }
    //-----------------------------------------------------------------------


}

