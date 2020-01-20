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
        : mlpD3DDevice(device)
    {
    }
    //-----------------------------------------------------------------------
    D3D11VertexDeclaration::~D3D11VertexDeclaration()
    {
    }
    //-----------------------------------------------------------------------
    void D3D11VertexDeclaration::notifyChanged()
    {
        clearCache();
    }
    //-----------------------------------------------------------------------
    void D3D11VertexDeclaration::notifyDeviceLost(D3D11Device* device)
    {
        clearCache();
    }
    //-----------------------------------------------------------------------
    void D3D11VertexDeclaration::notifyDeviceRestored(D3D11Device* device)
    {
    }
    //-----------------------------------------------------------------------
    void D3D11VertexDeclaration::clearCache()
    {
        mD3delems.clear();
        mShaderToILayoutMap.clear();
    }
    //-----------------------------------------------------------------------
    D3D11_INPUT_ELEMENT_DESC * D3D11VertexDeclaration::getD3DVertexDeclaration(D3D11HLSLProgram* boundVertexProgram, VertexBufferBinding* binding)
    {
        // Create D3D elements
        size_t iNumElements = boundVertexProgram->getNumInputs();

        if (mD3delems.find(boundVertexProgram) == mD3delems.end())
        {
            std::vector<D3D11_INPUT_ELEMENT_DESC> D3delems;

            unsigned int idx;
            for (idx = 0; idx < iNumElements; ++idx)
            {
                D3D11_SIGNATURE_PARAMETER_DESC inputDesc = boundVertexProgram->getInputParamDesc(idx);
                VertexElementList::const_iterator i, iend;
                iend = mElementList.end();
                bool found = false;
                for (i = mElementList.begin(); i != iend; ++i)
                {
                    LPCSTR semanticName         = D3D11Mappings::get(i->getSemantic());
                    UINT semanticIndex          = i->getIndex();
                    if(
                        strcmp(semanticName, inputDesc.SemanticName) == 0
                        && semanticIndex == inputDesc.SemanticIndex
                      )
                    {
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                                StringUtil::format("No VertexElement for semantic %s%d in shader %s found",
                                                   inputDesc.SemanticName, inputDesc.SemanticIndex,
                                                   boundVertexProgram->getName().c_str()));
                }

                D3D11_INPUT_ELEMENT_DESC elem = {};
                elem.SemanticName          = inputDesc.SemanticName;
                elem.SemanticIndex         = inputDesc.SemanticIndex;
                elem.Format                = D3D11Mappings::get(i->getType());
                elem.InputSlot             = i->getSource();
                elem.AlignedByteOffset     = static_cast<WORD>(i->getOffset());
                elem.InputSlotClass        = D3D11_INPUT_PER_VERTEX_DATA;
                elem.InstanceDataStepRate  = 0;

                VertexBufferBinding::VertexBufferBindingMap::const_iterator foundIter;
                foundIter = binding->getBindings().find(i->getSource());
                if ( foundIter != binding->getBindings().end() )
                {
                    HardwareVertexBufferSharedPtr bufAtSlot = foundIter->second;
                    if ( bufAtSlot->isInstanceData() )
                    {
                        elem.InputSlotClass        = D3D11_INPUT_PER_INSTANCE_DATA;
                        elem.InstanceDataStepRate  = bufAtSlot->getInstanceDataStepRate();
                    }
                }
                else
                {
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                        "Unable to found a bound vertex for a slot that is used in the vertex declaration." , 
                        "D3D11VertexDeclaration::getD3DVertexDeclaration");

                }
                D3delems.push_back(elem);
            }

            mD3delems[boundVertexProgram].swap(D3delems);

        }

        return mD3delems[boundVertexProgram].data();
    }
    //-----------------------------------------------------------------------
    ID3D11InputLayout*  D3D11VertexDeclaration::getILayoutByShader(D3D11HLSLProgram* boundVertexProgram, VertexBufferBinding* binding)
    {
        ShaderToILayoutMapIterator foundIter = mShaderToILayoutMap.find(boundVertexProgram);

        ComPtr<ID3D11InputLayout> pVertexLayout;

        if (foundIter == mShaderToILayoutMap.end())
        {
            // if not found - create

            DWORD dwShaderFlags = 0;
            const MicroCode &  vSBuf = boundVertexProgram->getMicroCode();

            D3D11_INPUT_ELEMENT_DESC * pVertexDecl=getD3DVertexDeclaration(boundVertexProgram, binding);

            // bad bug tracing. see what will happen next.
            //if (pVertexDecl->Format == DXGI_FORMAT_R16G16_SINT)
            //  pVertexDecl->Format = DXGI_FORMAT_R16G16_FLOAT;
            HRESULT hr = mlpD3DDevice->CreateInputLayout( 
                pVertexDecl, 
                boundVertexProgram->getNumInputs(), 
                &vSBuf[0], 
                vSBuf.size(),
                pVertexLayout.ReleaseAndGetAddressOf() );

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

        return pVertexLayout.Get(); // lifetime is determined by map
    }
    //-----------------------------------------------------------------------
    void D3D11VertexDeclaration::bindToShader(D3D11HLSLProgram* boundVertexProgram, VertexBufferBinding* binding)
    {
        // Set the input layout
        ID3D11InputLayout*  pVertexLayout = getILayoutByShader(boundVertexProgram, binding);
        mlpD3DDevice.GetImmediateContext()->IASetInputLayout( pVertexLayout);
    }   
}


