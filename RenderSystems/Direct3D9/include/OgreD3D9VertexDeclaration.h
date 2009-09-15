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
