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
#ifndef __D3D10VERTEXDECLARATION_H__
#define __D3D10VERTEXDECLARATION_H__

#include "OgreD3D10Prerequisites.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreFixedFuncState.h"

namespace Ogre { 

	/** Specialisation of VertexDeclaration for D3D10 */
	class D3D10VertexDeclaration : public VertexDeclaration
	{
	protected:
		D3D10Device & mlpD3DDevice;

		bool mNeedsRebuild;

		typedef map<D3D10HLSLProgram*, ID3D10InputLayout*>::type ShaderToILayoutMap;
		typedef ShaderToILayoutMap::iterator ShaderToILayoutMapIterator;

		D3D10_INPUT_ELEMENT_DESC*  mD3delems;

		ShaderToILayoutMap mShaderToILayoutMap;

		VertexBufferDeclaration mVertexBufferDeclaration;

		/** Gets the D3D10-specific vertex declaration. */

		ID3D10InputLayout	*  getILayoutByShader(D3D10HLSLProgram* boundVertexProgram);
	public:
		D3D10VertexDeclaration(D3D10Device &  device);
		~D3D10VertexDeclaration();

		/** See VertexDeclaration */
		const VertexElement& addElement(unsigned short source, size_t offset, VertexElementType theType,
			VertexElementSemantic semantic, unsigned short index = 0);

		/** See VertexDeclaration */
		const VertexElement& insertElement(unsigned short atPosition,
			unsigned short source, size_t offset, VertexElementType theType,
			VertexElementSemantic semantic, unsigned short index = 0);

		/** See VertexDeclaration */
		void removeElement(unsigned short elem_index);

		/** See VertexDeclaration */
		void removeElement(VertexElementSemantic semantic, unsigned short index = 0);

		/** See VertexDeclaration */
		void removeAllElements(void);


		/** See VertexDeclaration */
		void modifyElement(unsigned short elem_index, unsigned short source, size_t offset, VertexElementType theType,
			VertexElementSemantic semantic, unsigned short index = 0);


		D3D10_INPUT_ELEMENT_DESC * getD3DVertexDeclaration(void);
		void bindToShader(D3D10HLSLProgram* boundVertexProgram);

		const VertexBufferDeclaration & getVertexBufferDeclaration();

	};

}

#endif
