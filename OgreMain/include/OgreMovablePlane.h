/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
// Original free version by:
// Magic Software, Inc.
// http://www.geometrictools.com/
// Copyright (c) 2000, All Rights Reserved

#ifndef __MovablePlane_H__
#define __MovablePlane_H__

#include "OgrePrerequisites.h"

#include "OgrePlane.h"
#include "OgreMovableObject.h"

namespace Ogre {


	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Scene
	*  @{
	*/
	/** Definition of a Plane that may be attached to a node, and the derived
        details of it retrieved simply.
    @remarks
        This plane is not here for rendering purposes, it's to allow you to attach
        planes to the scene in order to have them move and follow nodes on their
        own, which is useful if you're using the plane for some kind of calculation,
        e.g. reflection.
    */
    class _OgreExport MovablePlane : public Plane, public MovableObject
    {
    protected:
        mutable Plane mDerivedPlane;
        mutable Vector3 mLastTranslate;
        mutable Quaternion mLastRotate;
        AxisAlignedBox mNullBB;
        mutable bool mDirty;
        static String msMovableType;
    public:

        MovablePlane(const String& name);
        MovablePlane (const Plane& rhs);
        /** Construct a plane through a normal, and a distance to move the plane along the normal.*/
        MovablePlane (const Vector3& rkNormal, Real fConstant);
        MovablePlane (const Vector3& rkNormal, const Vector3& rkPoint);
        MovablePlane (const Vector3& rkPoint0, const Vector3& rkPoint1,
            const Vector3& rkPoint2);
        ~MovablePlane() {}
        /// Overridden from MovableObject
        void _notifyCurrentCamera(Camera*) { /* don't care */ }
        /// Overridden from MovableObject
        const AxisAlignedBox& getBoundingBox(void) const { return mNullBB; }
        /// Overridden from MovableObject
        Real getBoundingRadius(void) const { return Math::POS_INFINITY; }
        /// Overridden from MovableObject
        void _updateRenderQueue(RenderQueue*) { /* do nothing */}
        /// Overridden from MovableObject
        const String& getMovableType(void) const;
        /// Get the derived plane as transformed by its parent node. 
        const Plane& _getDerivedPlane(void) const;
		/// @copydoc MovableObject::visitRenderables
		void visitRenderables(Renderable::Visitor* visitor, 
			bool debugRenderables = false) {/* do nothing */}

    };
	/** @} */
	/** @} */
}
#endif
