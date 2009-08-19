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
#include "OgreD3D9VertexDeclaration.h"
#include "OgreD3D9Mappings.h"
#include "OgreException.h"
#include "OgreRoot.h"
#include "OgreD3D9RenderSystem.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    D3D9VertexDeclaration::D3D9VertexDeclaration()         
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
    IDirect3DVertexDeclaration9* D3D9VertexDeclaration::getD3DVertexDeclaration(void)
    {
		IDirect3DDevice9* pCurDevice   = D3D9RenderSystem::getActiveD3D9Device();
		DeviceToDeclarationIterator it = mMapDeviceToDeclaration.find(pCurDevice);
		IDirect3DVertexDeclaration9* lpVertDecl = NULL;

		// Case we have to create the declaration for this device.
		if (it == mMapDeviceToDeclaration.end() || it->second == NULL)
		{
			D3DVERTEXELEMENT9* d3delems = new D3DVERTEXELEMENT9[mElementList.size() + 1];

			VertexElementList::const_iterator i, iend;
			unsigned int idx;
			iend = mElementList.end();
			for (idx = 0, i = mElementList.begin(); i != iend; ++i, ++idx)
			{
				d3delems[idx].Method = D3DDECLMETHOD_DEFAULT;
				d3delems[idx].Offset = static_cast<WORD>(i->getOffset());
				d3delems[idx].Stream = i->getSource();
				d3delems[idx].Type = D3D9Mappings::get(i->getType());
				d3delems[idx].Usage = D3D9Mappings::get(i->getSemantic());
				// NB force index if colours since D3D uses the same usage for 
				// diffuse & specular
				if (i->getSemantic() == VES_SPECULAR)
				{
					d3delems[idx].UsageIndex = 1;
				}
				else if (i->getSemantic() == VES_DIFFUSE)
				{
					d3delems[idx].UsageIndex = 0;
				}
				else
				{
					d3delems[idx].UsageIndex = i->getIndex();
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

			delete [] d3delems;

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


}

