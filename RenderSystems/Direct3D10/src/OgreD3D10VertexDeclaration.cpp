/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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
-----------------------------------------------------------------------------
*/
#include "OgreD3D10VertexDeclaration.h"
#include "OgreD3D10Mappings.h"
#include "OgreD3D10HLSLProgram.h"
#include "OgreD3D10Device.h"


namespace Ogre {

	//-----------------------------------------------------------------------
	D3D10VertexDeclaration::D3D10VertexDeclaration(D3D10Device &  device) 
		: 
	mlpD3DDevice(device)
	, mNeedsRebuild(true)
	, mD3delems(NULL)
	{

	}
	//-----------------------------------------------------------------------
	D3D10VertexDeclaration::~D3D10VertexDeclaration()
	{
		ShaderToILayoutMapIterator iter = mShaderToILayoutMap.begin();
		ShaderToILayoutMapIterator iterE = mShaderToILayoutMap.end();

		for ( ; iter != iterE ; iter++)
		{
			iter->second->Release();
		}

		if (mD3delems)
		{
			SAFE_DELETE_ARRAY(mD3delems);
		}


	}
	//-----------------------------------------------------------------------
	const VertexElement& D3D10VertexDeclaration::addElement(unsigned short source, 
		size_t offset, VertexElementType theType,
		VertexElementSemantic semantic, unsigned short index)
	{
		mNeedsRebuild = true;
		return VertexDeclaration::addElement(source, offset, theType, semantic, index);
	}
	//-----------------------------------------------------------------------------
	const VertexElement& D3D10VertexDeclaration::insertElement(unsigned short atPosition,
		unsigned short source, size_t offset, VertexElementType theType,
		VertexElementSemantic semantic, unsigned short index)
	{
		mNeedsRebuild = true;
		return VertexDeclaration::insertElement(atPosition, source, offset, theType, semantic, index);
	}
	//-----------------------------------------------------------------------
	void D3D10VertexDeclaration::removeElement(unsigned short elem_index)
	{
		VertexDeclaration::removeElement(elem_index);
		mNeedsRebuild = true;
	}
	//-----------------------------------------------------------------------
	void D3D10VertexDeclaration::removeElement(VertexElementSemantic semantic, unsigned short index)
	{
		VertexDeclaration::removeElement(semantic, index);
		mNeedsRebuild = true;
	}
	//-----------------------------------------------------------------------
	void D3D10VertexDeclaration::removeAllElements(void)
	{
		VertexDeclaration::removeAllElements();
		mNeedsRebuild = true;
	}
	//-----------------------------------------------------------------------
	void D3D10VertexDeclaration::modifyElement(unsigned short elem_index, 
		unsigned short source, size_t offset, VertexElementType theType,
		VertexElementSemantic semantic, unsigned short index)
	{
		VertexDeclaration::modifyElement(elem_index, source, offset, theType, semantic, index);
		mNeedsRebuild = true;
	}
	//-----------------------------------------------------------------------
	D3D10_INPUT_ELEMENT_DESC * D3D10VertexDeclaration::getD3DVertexDeclaration(void)
	{
		// Create D3D elements
		size_t iNumElements = mElementList.size();

		//SAFE_DELETE_ARRAY(mD3delems);

		if (!mD3delems)
		{
			D3D10_INPUT_ELEMENT_DESC*  D3delems = new D3D10_INPUT_ELEMENT_DESC[iNumElements];

			VertexElementList::const_iterator i, iend;
			unsigned int idx;
			iend = mElementList.end();
			for (idx = 0, i = mElementList.begin(); i != iend; ++i, ++idx)
			{
				D3delems[idx].SemanticName			= D3D10Mappings::get(i->getSemantic(),i->getIndex());
				D3delems[idx].SemanticIndex		= i->getIndex();
				D3delems[idx].Format				= D3D10Mappings::get(i->getType());
				D3delems[idx].InputSlot			= i->getSource();
				D3delems[idx].AlignedByteOffset	= static_cast<WORD>(i->getOffset());
				D3delems[idx].InputSlotClass		= D3D10_INPUT_PER_VERTEX_DATA;
				D3delems[idx].InstanceDataStepRate	= 0;

			}

			mD3delems = D3delems;

		}

		return mD3delems;
	}
	//-----------------------------------------------------------------------
	ID3D10InputLayout*  D3D10VertexDeclaration::getILayoutByShader(D3D10HLSLProgram* boundVertexProgram)
	{
		ShaderToILayoutMapIterator foundIter = mShaderToILayoutMap.find(boundVertexProgram);

		ID3D10InputLayout*  pVertexLayout = 0; 

		if (foundIter == mShaderToILayoutMap.end())
		{
			// if not found - create

			DWORD dwShaderFlags = 0;
			ID3D10Blob* pVSBuf = boundVertexProgram->getMicroCode();
			D3D10_INPUT_ELEMENT_DESC * pVertexDecl=getD3DVertexDeclaration();
			HRESULT hr = mlpD3DDevice->CreateInputLayout( 
				pVertexDecl, 
				(UINT)getElementCount(), 
				pVSBuf->GetBufferPointer(), 
				pVSBuf->GetBufferSize(),
				&pVertexLayout );

			if (FAILED(hr)|| mlpD3DDevice.isError())
			{
				String errorDescription = mlpD3DDevice.getErrorDescription();

				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set D3D10 vertex declaration"+errorDescription , 
					"D3D10VertexDeclaration::getILayoutByShader");
			}

			mShaderToILayoutMap[boundVertexProgram] = pVertexLayout;

		}
		else
		{
			pVertexLayout = foundIter->second;
		}

		return pVertexLayout;
	}
	//-----------------------------------------------------------------------
	void D3D10VertexDeclaration::bindToShader(D3D10HLSLProgram* boundVertexProgram)
	{
		ID3D10InputLayout*  pVertexLayout = getILayoutByShader(boundVertexProgram);

		// Set the input layout
		mlpD3DDevice->IASetInputLayout( pVertexLayout );
	}	
	//-----------------------------------------------------------------------
	const VertexBufferDeclaration & D3D10VertexDeclaration::getVertexBufferDeclaration()
	{
		if (mVertexBufferDeclaration.getVertexBufferElementList().empty())
		{
			VertexBufferElementList newList;
			unsigned short res = 0;
			for (unsigned short i = 0 ; i < getElementCount() ; i++)
			{
				VertexBufferElement newVertexBufferElement;
				newVertexBufferElement.setVertexElementIndex(getElement(i)->getIndex());
				newVertexBufferElement.setVertexElementSemantic(getElement(i)->getSemantic());
				newVertexBufferElement.setVertexElementType(getElement(i)->getType());
				newList.push_back(newVertexBufferElement);
			}

			mVertexBufferDeclaration.setVertexBufferElementList(newList);

		}

		return mVertexBufferDeclaration;
	}
}


