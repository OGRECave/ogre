/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#include "OgreRenderQueue.h"

namespace Ogre {

    uint SimpleRenderable::msGenNameCount = 0;

    SimpleRenderable::SimpleRenderable() : SimpleRenderable(BLANKSTRING)
    {
        // Generate name
        mName = StringUtil::format("SimpleRenderable%d", msGenNameCount++);
    }

    SimpleRenderable::SimpleRenderable(const String& name)
    : MovableObject(name)
    , mTransform(Affine3::IDENTITY)
    , mMaterial(MaterialManager::getSingleton().getDefaultMaterial())
    , mCamera(NULL)
    {
    }

    void SimpleRenderable::setMaterial( const MaterialPtr& mat )
    {
        mMaterial = mat;
        // Won't load twice anyway
        mMaterial->load();
    }

    const MaterialPtr& SimpleRenderable::getMaterial(void) const
    {
        return mMaterial;
    }

    void SimpleRenderable::getRenderOperation(RenderOperation& op)
    {
        op = mRenderOp;
    }

    void SimpleRenderable::setRenderOperation( const RenderOperation& rend )
    {
        mRenderOp = rend;
    }

    void SimpleRenderable::setTransform( const Affine3& xform )
    {
        mTransform = xform;
    }

    void SimpleRenderable::getWorldTransforms( Matrix4* xform ) const
    {
        *xform = mParentNode->_getFullTransform() * mTransform;
    }

    void SimpleRenderable::_notifyCurrentCamera(Camera* cam)
    {
        MovableObject::_notifyCurrentCamera(cam);

        mCamera = cam;
    }

    void SimpleRenderable::setBoundingBox( const AxisAlignedBox& box )
    {
        mBox = box;
    }

    const AxisAlignedBox& SimpleRenderable::getBoundingBox(void) const
    {
        return mBox;
    }

    void SimpleRenderable::_updateRenderQueue(RenderQueue* queue)
    {
        queue->addRenderable( this, mRenderQueueID, mRenderQueuePriority);
    }

    void SimpleRenderable::visitRenderables(Renderable::Visitor* visitor, 
        bool debugRenderables)
    {
        visitor->visit(this, 0, false);
    }

    //-----------------------------------------------------------------------
    const String& SimpleRenderable::getMovableType(void) const
    {
        static String movType = "SimpleRenderable";
        return movType;
    }
    //-----------------------------------------------------------------------
    const LightList& SimpleRenderable::getLights(void) const
    {
        // Use movable query lights
        return queryLights();
    }

}
