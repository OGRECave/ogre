/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
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
}
#endif
