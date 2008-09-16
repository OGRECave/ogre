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
#ifndef __Compositor_H__
#define __Compositor_H__

#include "OgrePrerequisites.h"
#include "OgreIteratorWrappers.h"
#include "OgreResource.h"

namespace Ogre {
    /** Class representing a Compositor object. Compositors provide the means 
        to flexibly "composite" the final rendering result from multiple scene renders
        and intermediate operations like rendering fullscreen quads. This makes 
        it possible to apply postfilter effects, HDRI postprocessing, and shadow 
        effects to a Viewport.
     */
    class _OgreExport Compositor: public Resource
    {
    public:
        Compositor(ResourceManager* creator, const String& name, ResourceHandle handle,
            const String& group, bool isManual = false, ManualResourceLoader* loader = 0);
        ~Compositor();
        
        /// Data types for internal lists
        typedef std::vector<CompositionTechnique *> Techniques;
        typedef VectorIterator<Techniques> TechniqueIterator;
        
        /** Create a new technique, and return a pointer to it.
        */
        CompositionTechnique *createTechnique();
        
        /** Remove a technique. It will also be destroyed.
        */
        void removeTechnique(size_t idx);
        
        /** Get a technique.
        */
        CompositionTechnique *getTechnique(size_t idx);
        
        /** Get the number of techniques.
        */
        size_t getNumTechniques();
        
        /** Remove all techniques
        */
        void removeAllTechniques();
        
        /** Get an iterator over the Techniques in this compositor. */
        TechniqueIterator getTechniqueIterator(void);
        
        /** Get a supported technique.
        @remarks
            The supported technique list is only available after this compositor has been compiled,
            which typically happens on loading it. Therefore, if this method returns
            an empty list, try calling Compositor::load.
        */
        CompositionTechnique *getSupportedTechnique(size_t idx);
        
        /** Get the number of supported techniques.
        @remarks
            The supported technique list is only available after this compositor has been compiled,
            which typically happens on loading it. Therefore, if this method returns
            an empty list, try calling Compositor::load.
        */
        size_t getNumSupportedTechniques();
        
        /** Gets an iterator over all the Techniques which are supported by the current card. 
        @remarks
            The supported technique list is only available after this compositor has been compiled,
            which typically happens on loading it. Therefore, if this method returns
            an empty list, try calling Compositor::load.
        */
        TechniqueIterator getSupportedTechniqueIterator(void);

		/** Get a pointer to a supported technique for a given scheme. 
		@remarks
			If there is no specific supported technique with this scheme name, 
			then the first supported technique with no specific scheme will be returned.
		@param schemeName The scheme name you are looking for. Blank means to 
			look for techniques with no scheme associated
		*/
		CompositionTechnique *getSupportedTechnique(const String& schemeName = StringUtil::BLANK);

    protected:
        /// @copydoc Resource::loadImpl
        void loadImpl(void);

        /// @copydoc Resource::unloadImpl
        void unloadImpl(void);
        /// @copydoc Resource::calculateSize
        size_t calculateSize(void) const;
        
        /** Check supportedness of techniques.
         */
        void compile();
    private:
        Techniques mTechniques;
        Techniques mSupportedTechniques;
        
        /// Compilation required
        /// This is set if the techniques change and the supportedness of techniques has to be
        /// re-evaluated.
        bool mCompilationRequired;
    };

    /** Specialisation of SharedPtr to allow SharedPtr to be assigned to CompositorPtr 
    @note Has to be a subclass since we need operator=.
    We could templatise this instead of repeating per Resource subclass, 
    except to do so requires a form VC6 does not support i.e.
    ResourceSubclassPtr<T> : public SharedPtr<T>
    */
    class _OgreExport CompositorPtr : public SharedPtr<Compositor> 
    {
    public:
        CompositorPtr() : SharedPtr<Compositor>() {}
        explicit CompositorPtr(Compositor* rep) : SharedPtr<Compositor>(rep) {}
        CompositorPtr(const CompositorPtr& r) : SharedPtr<Compositor>(r) {} 
        CompositorPtr(const ResourcePtr& r) : SharedPtr<Compositor>()
        {
            // lock & copy other mutex pointer
            OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
            {
                OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
                OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
                pRep = static_cast<Compositor*>(r.getPointer());
                pUseCount = r.useCountPointer();
                if (pUseCount)
                {
                    ++(*pUseCount);
                }
            }
        }

        /// Operator used to convert a ResourcePtr to a CompositorPtr
        CompositorPtr& operator=(const ResourcePtr& r)
        {
            if (pRep == static_cast<Compositor*>(r.getPointer()))
                return *this;
            release();
            // lock & copy other mutex pointer
            OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
            {
                OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
                OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
                pRep = static_cast<Compositor*>(r.getPointer());
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
    };
}

#endif
