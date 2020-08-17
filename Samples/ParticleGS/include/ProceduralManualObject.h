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

        void setRenderToVertexBuffer(RenderToVertexBufferSharedPtr r2vbObject)
        { mR2vbObject = r2vbObject; }
        const RenderToVertexBufferSharedPtr& getRenderToVertexBuffer()
        { return mR2vbObject; }

        void setManualObject(ManualObject* manualObject);
        ManualObject* getManualObject() const { return mManualObject; }

        /** @copydoc SimpleRenderable::_updateRenderQueue. */
        void _updateRenderQueue(RenderQueue* queue);
        /** @copydoc SimpleRenderable::getMovableType. */
        const String& getMovableType(void) const;
        /** @copydoc SimpleRenderable::getRenderOperation. */
        void getRenderOperation(RenderOperation& op);

        // Delegate to the manual object.
        Real getBoundingRadius(void) const
        { return mManualObject->getBoundingRadius(); }
        Real getSquaredViewDepth(const Ogre::Camera* cam) const
        { return mManualObject->getSections()[0]->getSquaredViewDepth(cam); }

    protected:
        ManualObject* mManualObject;
        RenderToVertexBufferSharedPtr mR2vbObject;
    };

    class ProceduralManualObjectFactory : public MovableObjectFactory
    {
    public:
        ProceduralManualObjectFactory() {}
        ~ProceduralManualObjectFactory() {}

        static String FACTORY_TYPE_NAME;

        const String& getType(void) const;
        void destroyInstance( MovableObject* obj);

    protected:
        MovableObject* createInstanceImpl(const String& name, const NameValuePairList* params);
    };

}
#endif
