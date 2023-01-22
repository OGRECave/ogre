// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#ifndef RENDERSYSTEMS_GLSUPPORT_OGREGLRENDERTOVERTEXBUFFERCOMMON_H_
#define RENDERSYSTEMS_GLSUPPORT_OGREGLRENDERTOVERTEXBUFFERCOMMON_H_

#include "OgreRenderToVertexBuffer.h"

namespace Ogre {

class GLRenderToVertexBufferCommon : public RenderToVertexBuffer
{
protected:
    void reallocateBuffer(uint8 index);
    static String getSemanticVaryingName(VertexElementSemantic semantic, unsigned short index);
    static uint32 getVertexCountPerPrimitive(RenderOperation::OperationType operationType);
};

} /* namespace Ogre */

#endif /* RENDERSYSTEMS_GLSUPPORT_OGREGLRENDERTOVERTEXBUFFERCOMMON_H_ */
