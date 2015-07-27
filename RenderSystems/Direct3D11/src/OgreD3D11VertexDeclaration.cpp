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
        size_t iNumInputElements = boundVertexProgram->getNumOfVertexInputs();
        
        ShaderToInputDescIterator itElement = mD3delems.find(boundVertexProgram);

        D3D11_INPUT_ELEMENT_DESC* d3delems = NULL;

        if (itElement != mD3delems.end())
        {
            //Fetch previously created input layout
            d3delems = itElement->second;
        }
        else
        {
            d3delems = new D3D11_INPUT_ELEMENT_DESC[iNumInputElements];
            ZeroMemory(d3delems, sizeof(D3D11_INPUT_ELEMENT_DESC) * iNumInputElements);

            for (unsigned int idx = 0, elementIndex = 0; idx < iNumElements; idx++)
            {
                D3D11_SIGNATURE_PARAMETER_DESC inputDesc = boundVertexProgram->getInputParamDesc(idx);
                //If it's a system variable don't bind to vertex input.
                if (inputDesc.SystemValueType != D3D_NAME_UNDEFINED)
                {
                    continue;
                }

                VertexElementList::const_iterator it;
                if (!findElemment(inputDesc, it))
                {
                    StringStream ss;
                    ss << "Could not find a suitable vertex buffer element for '" <<
                        inputDesc.SemanticName << ":" << inputDesc.SemanticIndex << "' in the vertex program '" <<
                        boundVertexProgram->getName() << "'";

                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, ss.str().c_str(),
                        "D3D11VertexDeclaration::getD3DVertexDeclaration");
                }

                VertexBufferBinding::VertexBufferBindingMap::const_iterator foundIter;
				unsigned short vertextBindingSlot = it->getSource();
				foundIter = binding->getBindings().find(vertextBindingSlot);
                if (foundIter == binding->getBindings().end())
                {
					StringStream ss;
					ss << "A reference to vertex buffer slot no. " << vertextBindingSlot << " is found in the vertex deceleration\n\
	         				but could not be found in the  vertex bindings";

					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,ss.str().c_str(),
                        "D3D11VertexDeclaration::getD3DVertexDeclaration");

                }

                fillInputElement(inputDesc, it, foundIter->second, d3delems[elementIndex]);
                elementIndex++;
            }
            
            mD3delems[boundVertexProgram] = d3delems;

        }
        return d3delems;
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

            D3D11_INPUT_ELEMENT_DESC * pVertexDecl = getD3DVertexDeclaration(boundVertexProgram, binding);

            // bad bug tracing. see what will happen next.
            //if (pVertexDecl->Format == DXGI_FORMAT_R16G16_SINT)
            //  pVertexDecl->Format = DXGI_FORMAT_R16G16_FLOAT;
            HRESULT hr = mlpD3DDevice->CreateInputLayout(
                pVertexDecl,
                boundVertexProgram->getNumOfVertexInputs(),
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
    //-----------------------------------------------------------------------
    bool D3D11VertexDeclaration::findElemment(const D3D11_SIGNATURE_PARAMETER_DESC& inputDesc, VertexElementList::const_iterator& it)
    {
        VertexElementList::const_iterator i, iend;
        iend = mElementList.end();

        for (i = mElementList.begin(); i != iend; ++i)
        {
            LPCSTR semanticName = D3D11Mappings::get(i->getSemantic());
            UINT semanticIndex = i->getIndex();
            if (
                strcmp(semanticName, inputDesc.SemanticName) == 0
                && semanticIndex == inputDesc.SemanticIndex
                )
            {
                it = i;
                return true;
                break;
            }
        }

        return false;
    }
    //-----------------------------------------------------------------------
    void D3D11VertexDeclaration::fillInputElement(const D3D11_SIGNATURE_PARAMETER_DESC& inputDesc , 
        const VertexElementList::const_iterator& it, 
        HardwareVertexBufferSharedPtr vertexBuffer,
        D3D11_INPUT_ELEMENT_DESC& element)
    {
        element.SemanticName = inputDesc.SemanticName;
        element.SemanticIndex = inputDesc.SemanticIndex;
        element.Format = D3D11Mappings::get(it->getType());
        element.InputSlot = it->getSource();
        element.AlignedByteOffset = static_cast<WORD>(it->getOffset());
        element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        element.InstanceDataStepRate = 0;


        if (vertexBuffer->getIsInstanceData())
        {
            element.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
            element.InstanceDataStepRate = vertexBuffer->getInstanceDataStepRate();
        }
    
    }

}


