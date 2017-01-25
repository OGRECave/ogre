/*
 * OgreGLTextureCommon.h
 *
 *  Created on: 24.01.2017
 *      Author: pavel
 */

#ifndef RENDERSYSTEMS_GLSUPPORT_INCLUDE_OGREGLTEXTURECOMMON_H_
#define RENDERSYSTEMS_GLSUPPORT_INCLUDE_OGREGLTEXTURECOMMON_H_

#include "OgreGLSupportPrerequisites.h"
#include "OgreTexture.h"
#include "OgreHardwarePixelBuffer.h"

namespace Ogre
{

class _OgreGLExport GLTextureCommon  : public Texture
{
public:
    GLTextureCommon(ResourceManager* creator, const String& name, ResourceHandle handle,
                    const String& group, bool isManual, ManualResourceLoader* loader)
        : Texture(creator, name, handle, group, isManual, loader), mTextureID(0)
    {
    }

    uint getGLID() const { return mTextureID; }

    void getCustomAttribute(const String& name, void* pData);

    void createRenderTexture() {
        // Create the GL texture
        // This already does everything necessary
        createInternalResources();
    }

protected:
    HardwarePixelBufferSharedPtr getBuffer(size_t face, size_t mipmap);

    void prepareImpl(void);

    void unprepareImpl(void)
    {
        mLoadedImages.clear();
    }

    /// Used to hold images between calls to prepare and load.
    typedef vector<Image>::type LoadedImages;

    /** Vector of images that were pulled from disk by
        prepareLoad but have yet to be pushed into texture memory
        by loadImpl.  Images should be deleted by loadImpl and unprepareImpl.
    */
    LoadedImages mLoadedImages;

    void readImage(LoadedImages& imgs, const String& name, const String& ext, bool haveNPOT);

    /// Vector of pointers to subsurfaces
    typedef vector<HardwarePixelBufferSharedPtr>::type SurfaceList;
    SurfaceList mSurfaceList;

    uint mTextureID;
};

} /* namespace Ogre */

#endif /* RENDERSYSTEMS_GLSUPPORT_INCLUDE_OGREGLTEXTURECOMMON_H_ */
