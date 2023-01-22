/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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

#include "OgreStableHeaders.h"
#include "OgreRenderToVertexBuffer.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    RenderToVertexBuffer::RenderToVertexBuffer() :
        mOperationType(RenderOperation::OT_TRIANGLE_LIST),
        mResetsEveryUpdate(false),
        mResetRequested(true),
        mSourceRenderable(0),
        mMaxVertexCount(1000)
    {
        mVertexData.reset(new VertexData);
    }
    RenderToVertexBuffer::~RenderToVertexBuffer() = default; // ensure unique_ptr destructors are in cpp
    //-----------------------------------------------------------------------
    VertexDeclaration* RenderToVertexBuffer::getVertexDeclaration()
    {
        //TODO : Mark dirty?
        return mVertexData->vertexDeclaration;
    }
    //-----------------------------------------------------------------------
    void RenderToVertexBuffer::setRenderToBufferMaterialName(const String& materialName)
    {
        mMaterial = MaterialManager::getSingleton().getByName(materialName);

        if (!mMaterial)
            OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND, "Could not find material " + materialName,
                         "RenderToVertexBuffer::setRenderToBufferMaterialName" );

        /* Ensure that the new material was loaded (will not load again if
           already loaded anyway)
        */
        mMaterial->load();
    }
    void RenderToVertexBuffer::getRenderOperation(RenderOperation& op)
    {
        op.operationType = mOperationType;
        op.useIndexes = false;
        op.vertexData = mVertexData.get();
    }
    Pass* RenderToVertexBuffer::derivePass(SceneManager* sceneMgr)
    {
        // Single pass only for now.
        Ogre::Pass* r2vbPass = mMaterial->getBestTechnique()->getPass(0);
        // Set pass before binding buffers to activate the GPU programs.
        sceneMgr->_setPass(r2vbPass);

        r2vbPass->_updateAutoParams(sceneMgr->_getAutoParamDataSource(), GPV_GLOBAL);

        // Bind shader parameters.
        RenderSystem* targetRenderSystem = Root::getSingleton().getRenderSystem();
        if (r2vbPass->hasVertexProgram())
        {
            targetRenderSystem->bindGpuProgramParameters(GPT_VERTEX_PROGRAM,
                                                         r2vbPass->getVertexProgramParameters(), GPV_ALL);
        }
        if (r2vbPass->hasGeometryProgram())
        {
            targetRenderSystem->bindGpuProgramParameters(GPT_GEOMETRY_PROGRAM,
                                                         r2vbPass->getGeometryProgramParameters(), GPV_ALL);
        }
        //TODO add tessellation stages

        return r2vbPass;
    }
}
