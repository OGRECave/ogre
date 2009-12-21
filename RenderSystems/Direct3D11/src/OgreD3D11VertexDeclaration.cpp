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
#include "OgreD3D11VertexDeclaration.h"
#include "OgreD3D11Mappings.h"
#include "OgreD3D11HLSLProgram.h"
#include "OgreD3D11Device.h"


namespace Ogre {

	//-----------------------------------------------------------------------
	D3D11VertexDeclaration::D3D11VertexDeclaration(D3D11Device &  device) 
		: 
	mlpD3DDevice(device)
	, mNeedsRebuild(true)
	, mD3delems(NULL)
	{

	}
	//-----------------------------------------------------------------------
	D3D11VertexDeclaration::~D3D11VertexDeclaration()
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
	const VertexElement& D3D11VertexDeclaration::addElement(unsigned short source, 
		size_t offset, VertexElementType theType,
		VertexElementSemantic semantic, unsigned short index)
	{
		mNeedsRebuild = true;
		return VertexDeclaration::addElement(source, offset, theType, semantic, index);
	}
	//-----------------------------------------------------------------------------
	const VertexElement& D3D11VertexDeclaration::insertElement(unsigned short atPosition,
		unsigned short source, size_t offset, VertexElementType theType,
		VertexElementSemantic semantic, unsigned short index)
	{
		mNeedsRebuild = true;
		return VertexDeclaration::insertElement(atPosition, source, offset, theType, semantic, index);
	}
	//-----------------------------------------------------------------------
	void D3D11VertexDeclaration::removeElement(unsigned short elem_index)
	{
		VertexDeclaration::removeElement(elem_index);
		mNeedsRebuild = true;
	}
	//-----------------------------------------------------------------------
	void D3D11VertexDeclaration::removeElement(VertexElementSemantic semantic, unsigned short index)
	{
		VertexDeclaration::removeElement(semantic, index);
		mNeedsRebuild = true;
	}
	//-----------------------------------------------------------------------
	void D3D11VertexDeclaration::removeAllElements(void)
	{
		VertexDeclaration::removeAllElements();
		mNeedsRebuild = true;
	}
	//-----------------------------------------------------------------------
	void D3D11VertexDeclaration::modifyElement(unsigned short elem_index, 
		unsigned short source, size_t offset, VertexElementType theType,
		VertexElementSemantic semantic, unsigned short index)
	{
		VertexDeclaration::modifyElement(elem_index, source, offset, theType, semantic, index);
		mNeedsRebuild = true;
	}
	//-----------------------------------------------------------------------
	D3D11_INPUT_ELEMENT_DESC * D3D11VertexDeclaration::getD3DVertexDeclaration(void)
	{
		// Create D3D elements
		size_t iNumElements = mElementList.size();

		//SAFE_DELETE_ARRAY(mD3delems);

		if (!mD3delems)
		{
			D3D11_INPUT_ELEMENT_DESC*  D3delems = new D3D11_INPUT_ELEMENT_DESC[iNumElements];

			VertexElementList::const_iterator i, iend;
			unsigned int idx;
			iend = mElementList.end();
			for (idx = 0, i = mElementList.begin(); i != iend; ++i, ++idx)
			{
				D3delems[idx].SemanticName			= D3D11Mappings::get(i->getSemantic(),i->getIndex());
				D3delems[idx].SemanticIndex		= i->getIndex();
				D3delems[idx].Format				= D3D11Mappings::get(i->getType());
				D3delems[idx].InputSlot			= i->getSource();
				D3delems[idx].AlignedByteOffset	= static_cast<WORD>(i->getOffset());
				D3delems[idx].InputSlotClass		= D3D11_INPUT_PER_VERTEX_DATA;
				D3delems[idx].InstanceDataStepRate	= 0;

			}

			mD3delems = D3delems;

		}

		return mD3delems;
	}
	//-----------------------------------------------------------------------
	ID3D11InputLayout*  D3D11VertexDeclaration::getILayoutByShader(D3D11HLSLProgram* boundVertexProgram)
	{
		ShaderToILayoutMapIterator foundIter = mShaderToILayoutMap.find(boundVertexProgram);

		ID3D11InputLayout*  pVertexLayout = 0; 

		if (foundIter == mShaderToILayoutMap.end())
		{
			// if not found - create

			DWORD dwShaderFlags = 0;
			ID3D10Blob* pVSBuf = boundVertexProgram->getMicroCode();
			D3D11_INPUT_ELEMENT_DESC * pVertexDecl=getD3DVertexDeclaration();
			HRESULT hr = mlpD3DDevice->CreateInputLayout( 
				pVertexDecl, 
				(UINT)getElementCount(), 
				pVSBuf->GetBufferPointer(), 
				pVSBuf->GetBufferSize(),
				&pVertexLayout );

			if (FAILED(hr)|| mlpD3DDevice.isError())
			{
				String errorDescription = mlpD3DDevice.getErrorDescription();

				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set D3D11 vertex declaration"+errorDescription , 
					"D3D11VertexDeclaration::getILayoutByShader");
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
	void D3D11VertexDeclaration::bindToShader(D3D11HLSLProgram* boundVertexProgram)
	{
		ID3D11InputLayout*  pVertexLayout = getILayoutByShader(boundVertexProgram);

		// Set the input layout
		mlpD3DDevice.GetImmediateContext()->IASetInputLayout( pVertexLayout );
	}	
	//-----------------------------------------------------------------------
	const VertexBufferDeclaration & D3D11VertexDeclaration::getVertexBufferDeclaration()
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


