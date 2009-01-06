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
