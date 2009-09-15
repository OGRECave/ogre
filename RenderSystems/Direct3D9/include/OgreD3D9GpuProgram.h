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
		typedef map<IDirect3DDevice9*, IDirect3DVertexShader9*>::type   DeviceToVertexShaderMap;
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
		typedef map<IDirect3DDevice9*, IDirect3DPixelShader9*>::type	DeviceToPixelShaderMap;
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
