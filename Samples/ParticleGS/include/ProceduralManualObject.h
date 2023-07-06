/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

  You may use this sample code for anything you like, it is not covered by the
  same license as the rest of the engine.
  -----------------------------------------------------------------------------
*/

#ifndef __PROCEDURAL_MANUAL_OBJECT_H__
#define __PROCEDURAL_MANUAL_OBJECT_H__

#include "OgreManualObject.h"
#include "OgreSimpleRenderable.h"
#include "OgreRenderToVertexBuffer.h"

namespace Ogre
{
    class ProceduralManualObject : public SimpleRenderable
    {
    public:
        ProceduralManualObject() {}
        virtual ~ProceduralManualObject() {}

        void setRenderToVertexBuffer(const RenderToVertexBufferSharedPtr& r2vbObject)
        { mR2vbObject = r2vbObject; }
        const RenderToVertexBufferSharedPtr& getRenderToVertexBuffer()
        { return mR2vbObject; }

        void _updateRenderQueue(RenderQueue* queue) override;
        void getRenderOperation(RenderOperation& op) override { mR2vbObject->getRenderOperation(op); }

        // Delegate to the manual object.
        Real getBoundingRadius(void) const override { return 0; }
        Real getSquaredViewDepth(const Camera* cam) const override { return 0; }

    protected:
        RenderToVertexBufferSharedPtr mR2vbObject;
    };

}
#endif
