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
#ifndef __D3D9GpuProgram_H_
#define __D3D9GpuProgram_H_

// Precompiler options
#include "OgreD3D9Prerequisites.h"
#include "OgreGpuProgram.h"
#include "OgreD3D9Resource.h"

namespace Ogre {

    /** Direct3D implementation of a few things common to low-level vertex & fragment programs. */
    class D3D9GpuProgram : public GpuProgram, public D3D9Resource
    {   	
    public:
        D3D9GpuProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader);
        ~D3D9GpuProgram();

        /** Tells the program to load from some externally created microcode instead of a file or source. 
        @remarks
            It is the callers responsibility to delete the microcode buffer.
        */ 
        void setExternalMicrocode(ID3DXBuffer* pMicrocode) { mpExternalMicrocode = pMicrocode; }
        /** Gets the external microcode buffer, if any. */
        LPD3DXBUFFER getExternalMicrocode(void) { return mpExternalMicrocode; }
    protected:
        /** @copydoc Resource::loadImpl */
        void loadImpl(void);
		/** Loads this program to specified device */
		void loadImpl(IDirect3DDevice9* d3d9Device);
        /** Overridden from GpuProgram */
        void loadFromSource(void);
		/** Loads this program from source to specified device */
		void loadFromSource(IDirect3DDevice9* d3d9Device);        
		/** Loads this program from microcode, must be overridden by subclasses. */
        virtual void loadFromMicrocode(IDirect3DDevice9* d3d9Device, ID3DXBuffer* microcode) = 0;

	protected:    
		ID3DXBuffer* mpExternalMicrocode; // microcode from elsewhere, we do NOT delete this ourselves	

    };

    /** Direct3D implementation of low-level vertex programs. */
    class D3D9GpuVertexProgram : public D3D9GpuProgram
    {  
    public:
        D3D9GpuVertexProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader);
		~D3D9GpuVertexProgram();
        
		/// Gets the vertex shader
        IDirect3DVertexShader9* getVertexShader(void);

		// Called immediately after the Direct3D device has been created.
		virtual void notifyOnDeviceCreate(IDirect3DDevice9* d3d9Device);

		// Called before the Direct3D device is going to be destroyed.
		virtual void notifyOnDeviceDestroy(IDirect3DDevice9* d3d9Device);

    protected:
        /** @copydoc Resource::unloadImpl */
        void unloadImpl(void);
        void loadFromMicrocode(IDirect3DDevice9* d3d9Device, ID3DXBuffer* microcode);

	protected:
		typedef std::map<IDirect3DDevice9*, IDirect3DVertexShader9*>	DeviceToVertexShaderMap;
		typedef DeviceToVertexShaderMap::iterator						DeviceToVertexShaderIterator;
	
		DeviceToVertexShaderMap		mMapDeviceToVertexShader;	
    };

    /** Direct3D implementation of low-level fragment programs. */
    class D3D9GpuFragmentProgram : public D3D9GpuProgram
    {  
    public:
        D3D9GpuFragmentProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader);
		~D3D9GpuFragmentProgram();
        /// Gets the pixel shader
        IDirect3DPixelShader9* getPixelShader(void);

		// Called immediately after the Direct3D device has been created.
		virtual void notifyOnDeviceCreate(IDirect3DDevice9* d3d9Device);

		// Called before the Direct3D device is going to be destroyed.
		virtual void notifyOnDeviceDestroy(IDirect3DDevice9* d3d9Device);

    protected:
        /** @copydoc Resource::unloadImpl */
        void unloadImpl(void);
        void loadFromMicrocode(IDirect3DDevice9* d3d9Device, ID3DXBuffer* microcode);

	protected:
		typedef std::map<IDirect3DDevice9*, IDirect3DPixelShader9*>		DeviceToPixelShaderMap;
		typedef DeviceToPixelShaderMap::iterator						DeviceToPixelShaderIterator;

		DeviceToPixelShaderMap		mMapDeviceToPixelShader;			
    };
    /** Specialisation of SharedPtr to allow SharedPtr to be assigned to D3D9GpuProgramPtr 
    @note Has to be a subclass since we need operator=.
    We could templatise this instead of repeating per Resource subclass, 
    except to do so requires a form VC6 does not support i.e.
    ResourceSubclassPtr<T> : public SharedPtr<T>
    */
    class _OgreExport D3D9GpuProgramPtr : public SharedPtr<D3D9GpuProgram> 
    {
    public:
        D3D9GpuProgramPtr() : SharedPtr<D3D9GpuProgram>() {}
        explicit D3D9GpuProgramPtr(D3D9GpuProgram* rep) : SharedPtr<D3D9GpuProgram>(rep) {}
        D3D9GpuProgramPtr(const D3D9GpuProgramPtr& r) : SharedPtr<D3D9GpuProgram>(r) {} 
        D3D9GpuProgramPtr(const ResourcePtr& r) : SharedPtr<D3D9GpuProgram>()
        {
			// lock & copy other mutex pointer
			OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
			OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
            pRep = static_cast<D3D9GpuProgram*>(r.getPointer());
            pUseCount = r.useCountPointer();
            if (pUseCount)
            {
                ++(*pUseCount);
            }
        }

        /// Operator used to convert a ResourcePtr to a D3D9GpuProgramPtr
        D3D9GpuProgramPtr& operator=(const ResourcePtr& r)
        {
            if (pRep == static_cast<D3D9GpuProgram*>(r.getPointer()))
                return *this;
            release();
			// lock & copy other mutex pointer
			OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
			OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
            pRep = static_cast<D3D9GpuProgram*>(r.getPointer());
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
