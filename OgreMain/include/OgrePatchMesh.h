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
#ifndef __PatchMesh_H__
#define __PatchMesh_H__

#include "OgrePrerequisites.h"
#include "OgreMesh.h"
#include "OgrePatchSurface.h"

namespace Ogre {

    /** Patch specialisation of Mesh. 
    @remarks
        Instances of this class should be created by calling MeshManager::createBezierPatch.
    */
    class _OgreExport PatchMesh : public Mesh
    {
    protected:
        /// Internal surface definition
        PatchSurface mSurface;
        /// Vertex declaration, cloned from the input
        VertexDeclaration* mDeclaration;
    public:
        /// Constructor
        PatchMesh(ResourceManager* creator, const String& name, ResourceHandle handle,
            const String& group);

        /// Define the patch, as defined in MeshManager::createBezierPatch
        void define(void* controlPointBuffer, 
            VertexDeclaration *declaration, size_t width, size_t height,
            size_t uMaxSubdivisionLevel = PatchSurface::AUTO_LEVEL, 
            size_t vMaxSubdivisionLevel = PatchSurface::AUTO_LEVEL,
            PatchSurface::VisibleSide visibleSide = PatchSurface::VS_FRONT,
            HardwareBuffer::Usage vbUsage = HardwareBuffer::HBU_STATIC_WRITE_ONLY, 
            HardwareBuffer::Usage ibUsage = HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY,
            bool vbUseShadow = false, bool ibUseShadow = false);

        /* Sets the current subdivision level as a proportion of full detail.
        @param factor Subdivision factor as a value from 0 (control points only) to 1 (maximum
            subdivision). */
        void setSubdivision(Real factor);
    protected:
        /// Overridden from Resource
        void loadImpl(void);
		/// Overridden from Resource - do nothing (no disk caching)
		void prepareImpl(void) {}

    };
    /** Specialisation of SharedPtr to allow SharedPtr to be assigned to PatchMeshPtr 
    @note Has to be a subclass since we need operator=.
    We could templatise this instead of repeating per Resource subclass, 
    except to do so requires a form VC6 does not support i.e.
    ResourceSubclassPtr<T> : public SharedPtr<T>
    */
    class _OgreExport PatchMeshPtr : public SharedPtr<PatchMesh> 
    {
    public:
        PatchMeshPtr() : SharedPtr<PatchMesh>() {}
        explicit PatchMeshPtr(PatchMesh* rep) : SharedPtr<PatchMesh>(rep) {}
        PatchMeshPtr(const PatchMeshPtr& r) : SharedPtr<PatchMesh>(r) {} 
        PatchMeshPtr(const ResourcePtr& r) : SharedPtr<PatchMesh>()
        {
			// lock & copy other mutex pointer
            OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
            {
			    OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
			    OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
                pRep = static_cast<PatchMesh*>(r.getPointer());
                pUseCount = r.useCountPointer();
                if (pUseCount)
                {
                    ++(*pUseCount);
                }
            }
        }

        /// Operator used to convert a ResourcePtr to a PatchMeshPtr
        PatchMeshPtr& operator=(const ResourcePtr& r)
        {
            if (pRep == static_cast<PatchMesh*>(r.getPointer()))
                return *this;
            release();

            OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
            {
			    // lock & copy other mutex pointer
			    OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
			    OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
                pRep = static_cast<PatchMesh*>(r.getPointer());
                pUseCount = r.useCountPointer();
                if (pUseCount)
                {
                    ++(*pUseCount);
                }
            }
			else
			{
				// RHS must be a null pointer
				assert(r.isNull() && "RHS must be null if it has no mutex!");
				setNull();
			}
            return *this;
        }
        /// Operator used to convert a MeshPtr to a PatchMeshPtr
        PatchMeshPtr& operator=(const MeshPtr& r)
        {
            if (pRep == static_cast<PatchMesh*>(r.getPointer()))
                return *this;
            release();
			// lock & copy other mutex pointer
            OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
            {
			    OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
			    OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
                pRep = static_cast<PatchMesh*>(r.getPointer());
                pUseCount = r.useCountPointer();
                if (pUseCount)
                {
                    ++(*pUseCount);
                }
            }
            return *this;
        }
    };

}

#endif
