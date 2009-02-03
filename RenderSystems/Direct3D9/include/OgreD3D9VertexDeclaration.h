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
#ifndef __D3D9VERTEXDECLARATION_H__
#define __D3D9VERTEXDECLARATION_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreD3D9Resource.h"

namespace Ogre { 

    /** Specialisation of VertexDeclaration for D3D9 */
    class D3D9VertexDeclaration : public VertexDeclaration, public D3D9Resource
    {
    
    public:
        D3D9VertexDeclaration();
        ~D3D9VertexDeclaration();
        
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

		// Called immediately after the Direct3D device has been created.
		virtual void notifyOnDeviceCreate(IDirect3DDevice9* d3d9Device);

		// Called before the Direct3D device is going to be destroyed.
		virtual void notifyOnDeviceDestroy(IDirect3DDevice9* d3d9Device);

        /** Gets the D3D9-specific vertex declaration. */
        IDirect3DVertexDeclaration9* getD3DVertexDeclaration(void);

	protected:
		void	releaseDeclaration();


	protected:        
		typedef map<IDirect3DDevice9*, IDirect3DVertexDeclaration9*>::type	DeviceToDeclarationMap;
		typedef DeviceToDeclarationMap::iterator							DeviceToDeclarationIterator;

		DeviceToDeclarationMap		mMapDeviceToDeclaration;
    };

}

#endif
