// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.

#ifndef RENDERSYSTEMS_GLSUPPORT_INCLUDE_OGREGLRENDERTARGET_H_
#define RENDERSYSTEMS_GLSUPPORT_INCLUDE_OGREGLRENDERTARGET_H_

#include "OgreGLSupportPrerequisites.h"

namespace Ogre
{
class GLContext;
class GLFrameBufferObjectCommon;

class _OgrePrivate GLRenderTarget
{
public:
    virtual ~GLRenderTarget() {}

    /** Get the GL context that needs to be active to render to this target
     */
    virtual GLContext* getContext() const = 0;
    virtual GLFrameBufferObjectCommon* getFBO() { return NULL; }
};
} // namespace Ogre

#endif /* RENDERSYSTEMS_GLSUPPORT_INCLUDE_OGREGLRENDERTARGET_H_ */
