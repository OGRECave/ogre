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
#ifndef __SimpleRenderable_H__
#define __SimpleRenderable_H__

#include "OgrePrerequisites.h"

#include "OgreMovableObject.h"
#include "OgreRenderable.h"
#include "OgreAxisAlignedBox.h"
#include "OgreRenderOperation.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */
    /** Simple implementation of MovableObject and Renderable for single-part custom objects. 
    @see ManualObject for a simpler interface with more flexibility
    */
    class _OgreExport SimpleRenderable : public MovableObject, public Renderable
    {
        bool getCastsShadows(void) const override { return getCastShadows(); }
    protected:
        RenderOperation mRenderOp;

        Affine3 mTransform;
        AxisAlignedBox mBox;

        MaterialPtr mMaterial;

        /// The scene manager for the current frame.
        SceneManager *mParentSceneManager;

        /// The camera for the current frame.
        Camera *mCamera;

        /// Static member used to automatically generate names for SimpleRendaerable objects.
        static uint msGenNameCount;

    public:
        /// Constructor
        SimpleRenderable();

        /// Named constructor
        SimpleRenderable(const String& name);

        virtual void setMaterial(const MaterialPtr& mat);
        virtual const MaterialPtr& getMaterial(void) const;

        virtual void setRenderOperation( const RenderOperation& rend );
        virtual void getRenderOperation(RenderOperation& op);

        void setTransform( const Affine3& xform );
        virtual void getWorldTransforms( Matrix4* xform ) const;


        virtual void _notifyCurrentCamera(Camera* cam);

        void setBoundingBox( const AxisAlignedBox& box );
        virtual const AxisAlignedBox& getBoundingBox(void) const;

        virtual void _updateRenderQueue(RenderQueue* queue);

        void visitRenderables(Renderable::Visitor* visitor,
            bool debugRenderables = false) override;
        virtual const String& getMovableType(void) const override;
        const LightList& getLights(void) const override;

    };
    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif

