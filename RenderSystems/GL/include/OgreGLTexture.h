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

#ifndef __GLTEXTURE_H__
#define __GLTEXTURE_H__

#include "OgreGLPrerequisites.h"
#include "OgrePlatform.h"
#include "OgreRenderTexture.h"
#include "OgreTexture.h"
#include "OgreGLSupport.h"
#include "OgreHardwarePixelBuffer.h"

namespace Ogre {

    class _OgrePrivate GLTexture : public Texture
    {
    public:
        // Constructor
        GLTexture(ResourceManager* creator, const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader, 
            GLSupport& support);

        virtual ~GLTexture();      

        void createRenderTexture();
			
		/// @copydoc Texture::getBuffer
		HardwarePixelBufferSharedPtr getBuffer(size_t face, size_t mipmap);

        // Takes the OGRE texture type (1d/2d/3d/cube) and returns the appropriate GL one
        GLenum getGLTextureTarget(void) const;

        GLuint getGLID() const
        {
            return mTextureID;
        }

    protected:
		/// @copydoc Texture::createInternalResourcesImpl
		void createInternalResourcesImpl(void);
        /// @copydoc Resource::prepareImpl
        void prepareImpl(void);
        /// @copydoc Resource::unprepareImpl
        void unprepareImpl(void);
        /// @copydoc Resource::loadImpl
        void loadImpl(void);
        /// @copydoc Resource::freeInternalResourcesImpl
        void freeInternalResourcesImpl(void);

		/** internal method, create GLHardwarePixelBuffers for every face and
			 mipmap level. This method must be called after the GL texture object was created,
			the number of mipmaps was set (GL_TEXTURE_MAX_LEVEL) and glTexImageXD was called to
			actually allocate the buffer
		*/
		void _createSurfaceList();

        /// Used to hold images between calls to prepare and load.
        typedef SharedPtr<vector<Image>::type > LoadedImages;

        /** Vector of images that were pulled from disk by
            prepareLoad but have yet to be pushed into texture memory
            by loadImpl.  Images should be deleted by loadImpl and unprepareImpl.
        */
        LoadedImages mLoadedImages;


    private:
        GLuint mTextureID;
        GLSupport& mGLSupport;
		
		/// Vector of pointers to subsurfaces
		typedef vector<HardwarePixelBufferSharedPtr>::type SurfaceList;
		SurfaceList	mSurfaceList;
    };

    /** Specialisation of SharedPtr to allow SharedPtr to be assigned to GLTexturePtr 
    @note Has to be a subclass since we need operator=.
    We could templatise this instead of repeating per Resource subclass, 
    except to do so requires a form VC6 does not support i.e.
    ResourceSubclassPtr<T> : public SharedPtr<T>
    */
    class _OgrePrivate GLTexturePtr : public SharedPtr<GLTexture> 
    {
    public:
        GLTexturePtr() : SharedPtr<GLTexture>() {}
        explicit GLTexturePtr(GLTexture* rep) : SharedPtr<GLTexture>(rep) {}
        GLTexturePtr(const GLTexturePtr& r) : SharedPtr<GLTexture>(r) {} 
        GLTexturePtr(const ResourcePtr& r) : SharedPtr<GLTexture>()
        {
			// lock & copy other mutex pointer
            OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
            {
			    OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
			    OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
                pRep = static_cast<GLTexture*>(r.getPointer());
                pUseCount = r.useCountPointer();
                if (pUseCount)
                {
                    ++(*pUseCount);
                }
            }
        }
		GLTexturePtr(const TexturePtr& r) : SharedPtr<GLTexture>()
		{
			*this = r;
		}

        /// Operator used to convert a ResourcePtr to a GLTexturePtr
        GLTexturePtr& operator=(const ResourcePtr& r)
        {
            if (pRep == static_cast<GLTexture*>(r.getPointer()))
                return *this;
            release();
			// lock & copy other mutex pointer
            OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
            {
			    OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
			    OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
                pRep = static_cast<GLTexture*>(r.getPointer());
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
        /// Operator used to convert a TexturePtr to a GLTexturePtr
        GLTexturePtr& operator=(const TexturePtr& r)
        {
            if (pRep == static_cast<GLTexture*>(r.getPointer()))
                return *this;
            release();
			// lock & copy other mutex pointer
            OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
            {
			    OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
			    OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
                pRep = static_cast<GLTexture*>(r.getPointer());
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

#endif // __GLTEXTURE_H__
