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
	{

	}
	//-----------------------------------------------------------------------
	D3D11VertexDeclaration::~D3D11VertexDeclaration()
	{
        {
		    ShaderToILayoutMapIterator iter = mShaderToILayoutMap.begin();
		    ShaderToILayoutMapIterator iterE = mShaderToILayoutMap.end();

		    for ( ; iter != iterE ; iter++)
		    {
			    iter->second->Release();
		    }
        }

        {
            ShaderToInputDescIterator iter = mD3delems.begin();
            ShaderToInputDescIterator iterE = mD3delems.end();
            for( ; iter != iterE ; iter++ )
            {
                SAFE_DELETE_ARRAY(iter->second);

            }
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
	D3D11_INPUT_ELEMENT_DESC * D3D11VertexDeclaration::getD3DVertexDeclaration(D3D11HLSLProgram* boundVertexProgram, VertexBufferBinding* binding)
	{
		// Create D3D elements
		size_t iNumElements = boundVertexProgram->getNumInputs();

		if (mD3delems.find(boundVertexProgram) == mD3delems.end())
		{
			D3D11_INPUT_ELEMENT_DESC*  D3delems = new D3D11_INPUT_ELEMENT_DESC[iNumElements];
            ZeroMemory(D3delems, sizeof(D3D11_INPUT_ELEMENT_DESC) * iNumElements);

			unsigned int idx;
       		for (idx = 0; idx < iNumElements; ++idx)
			{
                D3D11_SIGNATURE_PARAMETER_DESC inputDesc = boundVertexProgram->getInputParamDesc(idx);
       			VertexElementList::const_iterator i, iend;
			    iend = mElementList.end();
                bool found = false;
		        for (i = mElementList.begin(); i != iend; ++i)
                {
                    LPCSTR semanticName			= D3D11Mappings::get(i->getSemantic());
                    UINT semanticIndex			= i->getIndex();
                    if(
                        strcmp(semanticName, inputDesc.SemanticName) == 0
                        && semanticIndex == inputDesc.SemanticIndex
                      )
                    {
                        found = true;
                        break;
                    }
                }

                if(!found)
                {
                    // find by pos
                    i = mElementList.begin();
                    for (unsigned int count = 0; count < idx && i != iend; count++, ++i)
                    {
                    }
                    if (i != iend)
                    {
                        found = true;
                    }
                }

                if(!found)
                {
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set D3D11 vertex declaration" , 
                        						"D3D11VertexDeclaration::getILayoutByShader");
                }

				D3delems[idx].SemanticName			= inputDesc.SemanticName;
				D3delems[idx].SemanticIndex			= inputDesc.SemanticIndex;
				D3delems[idx].Format				= D3D11Mappings::get(i->getType());
				D3delems[idx].InputSlot				= i->getSource();
				D3delems[idx].AlignedByteOffset		= static_cast<WORD>(i->getOffset());
				D3delems[idx].InputSlotClass		= D3D11_INPUT_PER_VERTEX_DATA;
				D3delems[idx].InstanceDataStepRate	= 0;

				VertexBufferBinding::VertexBufferBindingMap::const_iterator foundIter;
				foundIter = binding->getBindings().find(i->getSource());
				if ( foundIter != binding->getBindings().end() )
				{
					HardwareVertexBufferSharedPtr bufAtSlot = foundIter->second;
					if ( bufAtSlot->isInstanceData() )
					{
						D3delems[idx].InputSlotClass		= D3D11_INPUT_PER_INSTANCE_DATA;
						D3delems[idx].InstanceDataStepRate	= bufAtSlot->getInstanceDataStepRate();
					}
				}
				else
				{
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
						"Unable to found a bound vertex for a slot that is used in the vertex declaration." , 
						"D3D11VertexDeclaration::getD3DVertexDeclaration");

				}				
			}

			mD3delems[boundVertexProgram] = D3delems;

		}

		return mD3delems[boundVertexProgram];
	}
	//-----------------------------------------------------------------------
	ID3D11InputLayout*  D3D11VertexDeclaration::getILayoutByShader(D3D11HLSLProgram* boundVertexProgram, VertexBufferBinding* binding)
	{
		ShaderToILayoutMapIterator foundIter = mShaderToILayoutMap.find(boundVertexProgram);

		ID3D11InputLayout*  pVertexLayout = 0; 

		if (foundIter == mShaderToILayoutMap.end())
		{
			// if not found - create

			DWORD dwShaderFlags = 0;
			const MicroCode &  vSBuf = boundVertexProgram->getMicroCode();

			D3D11_INPUT_ELEMENT_DESC * pVertexDecl=getD3DVertexDeclaration(boundVertexProgram, binding);
			HRESULT hr = mlpD3DDevice->CreateInputLayout( 
				pVertexDecl, 
				boundVertexProgram->getNumInputs(), 
				&vSBuf[0], 
				vSBuf.size(),
				&pVertexLayout );

			if (FAILED(hr)|| mlpD3DDevice.isError())
			{
				String errorDescription = mlpD3DDevice.getErrorDescription(hr);
                errorDescription += "\nBound shader name: " + boundVertexProgram->getName();

				OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
					"Unable to set D3D11 vertex declaration" + errorDescription,
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
	void D3D11VertexDeclaration::bindToShader(D3D11HLSLProgram* boundVertexProgram, VertexBufferBinding* binding)
	{
		ID3D11InputLayout*  pVertexLayout = getILayoutByShader(boundVertexProgram, binding);

		// Set the input layout
		mlpD3DDevice.GetImmediateContext()->IASetInputLayout( pVertexLayout);
	}	
}


