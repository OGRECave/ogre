// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include "OgreGLRenderToVertexBufferCommon.h"

#include "OgreHardwareBufferManager.h"

namespace Ogre
{

void GLRenderToVertexBufferCommon::reallocateBuffer(uint8 index)
{
    assert(index < 2);

    // Transform feedback buffer must be at least as large as the
    // number of output primitives.
    mVertexBuffers[index] = HardwareBufferManager::getSingleton().createVertexBuffer(
        mVertexData->vertexDeclaration->getVertexSize(0), mMaxVertexCount, HBU_GPU_ONLY);
}

String GLRenderToVertexBufferCommon::getSemanticVaryingName(VertexElementSemantic semantic,
                                                                     unsigned short index)
{
    switch (semantic)
    {
    case VES_POSITION:
        return "xfb_position";
    case VES_NORMAL:
        return "xfb_normal";
    case VES_TEXTURE_COORDINATES:
        return StringUtil::format("xfb_uv%d", index);
    case VES_DIFFUSE:
        return "xfb_colour";
    case VES_SPECULAR:
        return "xfb_colour2";
    default:
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unsupported VertexElementSemantic");
    }
}

uint32 GLRenderToVertexBufferCommon::getVertexCountPerPrimitive(RenderOperation::OperationType operationType)
{
    // We can only get points, lines or triangles since they are the only
    // legal R2VB output primitive types.
    switch (operationType)
    {
    case RenderOperation::OT_POINT_LIST:
        return 1;
    case RenderOperation::OT_LINE_LIST:
        return 2;
    default:
    case RenderOperation::OT_TRIANGLE_LIST:
        return 3;
    }
}

} /* namespace Ogre */
