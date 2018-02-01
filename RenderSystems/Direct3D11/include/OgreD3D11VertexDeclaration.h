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
#ifndef __D3D11VERTEXDECLARATION_H__
#define __D3D11VERTEXDECLARATION_H__

#include "OgreD3D11Prerequisites.h"
#include "OgreD3D11DeviceResource.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreHighLevelGpuProgramManager.h"

namespace Ogre { 

    /** Specialisation of VertexDeclaration for D3D11 */
    class _OgreD3D11Export D3D11VertexDeclaration
        : public VertexDeclaration
        , protected D3D11DeviceResource
    {
    protected:
        D3D11Device & mlpD3DDevice;

        typedef std::map<D3D11HLSLProgram*, ComPtr<ID3D11InputLayout>> ShaderToILayoutMap;
        typedef ShaderToILayoutMap::iterator ShaderToILayoutMapIterator;
        typedef std::map<D3D11HLSLProgram*, std::vector<D3D11_INPUT_ELEMENT_DESC>> ShaderToInputDesc;
        typedef ShaderToInputDesc::iterator ShaderToInputDescIterator;

        ShaderToInputDesc  mD3delems;

        ShaderToILayoutMap mShaderToILayoutMap;

        /** Gets the D3D11-specific vertex declaration. */
        ID3D11InputLayout   *  getILayoutByShader(D3D11HLSLProgram* boundVertexProgram, VertexBufferBinding* binding);
        D3D11_INPUT_ELEMENT_DESC * getD3DVertexDeclaration(D3D11HLSLProgram* boundVertexProgram, VertexBufferBinding* binding);
        void clearCache();

        void notifyDeviceLost(D3D11Device* device);
        void notifyDeviceRestored(D3D11Device* device);

        void notifyChanged();
    public:
        D3D11VertexDeclaration(D3D11Device &  device);
        ~D3D11VertexDeclaration();

        void bindToShader(D3D11HLSLProgram* boundVertexProgram, VertexBufferBinding* binding);
    };

}

#endif
