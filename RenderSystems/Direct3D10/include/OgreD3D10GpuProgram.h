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
#ifndef __D3D10GpuProgram_H_
#define __D3D10GpuProgram_H_

// Precompiler options
#include "OgreD3D10Prerequisites.h"
#include "OgreGpuProgram.h"

namespace Ogre {

	/** Direct3D implementation of a few things common to low-level vertex & fragment programs. */
	class D3D10GpuProgram : public GpuProgram
	{
	protected:
		D3D10Device & mDevice;
		ID3D10Blob *  mpExternalMicrocode; // microcode from elsewhere, we do NOT delete this ourselves
	public:
		D3D10GpuProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
			const String& group, bool isManual, ManualResourceLoader* loader, D3D10Device & device);


		/** Tells the program to load from some externally created microcode instead of a file or source. 
		@remarks
		It is the callers responsibility to delete the microcode buffer.
		*/ 
		void setExternalMicrocode(ID3D10Blob *  pMicrocode);
		/** Gets the external microcode buffer, if any. */
		ID3D10Blob *  getExternalMicrocode(void);
	protected:
		/** @copydoc Resource::loadImpl */
		void loadImpl(void);
		/** Overridden from GpuProgram */
		void loadFromSource(void);
		/** Internal method to load from microcode, must be overridden by subclasses. */
		virtual void loadFromMicrocode(ID3D10Blob *  microcode) = 0;


	};

	/** Direct3D implementation of low-level vertex programs. */
	class D3D10GpuVertexProgram : public D3D10GpuProgram
	{
	protected:
		ID3D10VertexShader * mpVertexShader;
	public:
		D3D10GpuVertexProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
			const String& group, bool isManual, ManualResourceLoader* loader, D3D10Device & device);
		~D3D10GpuVertexProgram();
		/// Gets the vertex shader
		ID3D10VertexShader * getVertexShader(void) const;
	protected:
		/** @copydoc Resource::unloadImpl */
		void unloadImpl(void);
		void loadFromMicrocode(ID3D10Blob *  microcode);
	};

	/** Direct3D implementation of low-level fragment programs. */
	class D3D10GpuFragmentProgram : public D3D10GpuProgram
	{
	protected:
		ID3D10PixelShader * mpPixelShader;
	public:
		D3D10GpuFragmentProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
			const String& group, bool isManual, ManualResourceLoader* loader, D3D10Device & device);
		~D3D10GpuFragmentProgram();
		/// Gets the pixel shader
		ID3D10PixelShader * getPixelShader(void) const;
	protected:
		/** @copydoc Resource::unloadImpl */
		void unloadImpl(void);
		void loadFromMicrocode(ID3D10Blob *  microcode);
	};
	/** Specialisation of SharedPtr to allow SharedPtr to be assigned to D3D10GpuProgramPtr 
	@note Has to be a subclass since we need operator=.
	We could templatise this instead of repeating per Resource subclass, 
	except to do so requires a form VC6 does not support i.e.
	ResourceSubclassPtr<T> : public SharedPtr<T>
	*/
	class _OgreExport D3D10GpuProgramPtr : public SharedPtr<D3D10GpuProgram> 
	{
	public:
		D3D10GpuProgramPtr() : SharedPtr<D3D10GpuProgram>() {}
		explicit D3D10GpuProgramPtr(D3D10GpuProgram* rep) : SharedPtr<D3D10GpuProgram>(rep) {}
		D3D10GpuProgramPtr(const D3D10GpuProgramPtr& r) : SharedPtr<D3D10GpuProgram>(r) {} 
		D3D10GpuProgramPtr(const ResourcePtr& r) : SharedPtr<D3D10GpuProgram>()
		{
			// lock & copy other mutex pointer
			OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
				OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
				pRep = static_cast<D3D10GpuProgram*>(r.getPointer());
			pUseCount = r.useCountPointer();
			if (pUseCount)
			{
				++(*pUseCount);
			}
		}

		/// Operator used to convert a ResourcePtr to a D3D10GpuProgramPtr
		D3D10GpuProgramPtr& operator=(const ResourcePtr& r)
		{
			if (pRep == static_cast<D3D10GpuProgram*>(r.getPointer()))
				return *this;
			release();
			// lock & copy other mutex pointer
			OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
				OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
				pRep = static_cast<D3D10GpuProgram*>(r.getPointer());
			pUseCount = r.useCountPointer();
			if (pUseCount)
			{
				++(*pUseCount);
			}
			return *this;
		}
	};

}


#endif
