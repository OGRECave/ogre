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

#ifndef __Bone_H__
#define __Bone_H__

#include "OgrePrerequisites.h"
#include "OgreNode.h"


namespace Ogre 
{
    /** A bone in a skeleton.
    @remarks
        See Skeleton for more information about the principles behind skeletal animation.
        This class is a node in the joint hierarchy. Mesh vertices also have assignments
        to bones to define how they move in relation to the skeleton.
    */
    class _OgreExport Bone : public Node
    {
    public:
        /** Constructor, not to be used directly (use Bone::createChild or Skeleton::createBone) */
        Bone(unsigned short handle, Skeleton* creator);
        /** Constructor, not to be used directly (use Bone::createChild or Skeleton::createBone) */
        Bone(const String& name, unsigned short handle, Skeleton* creator);
        ~Bone();

        /** Creates a new Bone as a child of this bone.
        @remarks
            This method creates a new bone which will inherit the transforms of this
            bone, with the handle specified.
            @param 
                handle The numeric handle to give the new bone; must be unique within the Skeleton.
            @param
                translate Initial translation offset of child relative to parent
            @param
                rotate Initial rotation relative to parent
        */
        Bone* createChild(unsigned short handle, 
            const Vector3& translate = Vector3::ZERO, const Quaternion& rotate = Quaternion::IDENTITY);


        /** Gets the numeric handle for this bone (unique within the skeleton). */
        unsigned short getHandle(void) const;

        /** Sets the current position / orientation to be the 'binding pose' ie the layout in which 
            bones were originally bound to a mesh.
        */
        void setBindingPose(void);

        /** Resets the position and orientation of this Bone to the original binding position.
        @remarks
            Bones are bound to the mesh in a binding pose. They are then modified from this
            position during animation. This method returns the bone to it's original position and
            orientation.
        */
        void reset(void);

        /** Sets whether or not this bone is manually controlled. 
        @remarks
            Manually controlled bones can be altered by the application at runtime, 
            and their positions will not be reset by the animation routines. Note 
            that you should also make sure that there are no AnimationTrack objects
            referencing this bone, or if there are, you should disable them using
            pAnimation->destroyTrack(pBone->getHandle());
        */
        void setManuallyControlled(bool manuallyControlled);

        /** Getter for mManuallyControlled Flag */
        bool isManuallyControlled() const;

        
        /** Gets the transform which takes bone space to current from the binding pose. 
        @remarks
            Internal use only.
        */
        void _getOffsetTransform(Matrix4& m) const;

		/** Gets the inverted binding pose scale. */
		const Vector3& _getBindingPoseInverseScale(void) const { return mBindDerivedInverseScale; }
		/** Gets the inverted binding pose position. */
		const Vector3& _getBindingPoseInversePosition(void) const { return mBindDerivedInversePosition; }
		/** Gets the inverted binding pose orientation. */
		const Quaternion& _getBindingPoseInverseOrientation(void) const { return mBindDerivedInverseOrientation; }

		/// @see Node::needUpdate
		void needUpdate(bool forceParentUpdate = false);


    protected:
        /// The numeric handle of this bone
        unsigned short mHandle;

        /** Bones set as manuallyControlled are not reseted in Skeleton::reset() */
        bool mManuallyControlled;

        /** See Node. */
        Node* createChildImpl(void);
        /** See Node. */
        Node* createChildImpl(const String& name);

        /// Pointer back to creator, for child creation (not smart ptr so child does not preserve parent)
        Skeleton* mCreator;

        /// The inversed derived scale of the bone in the binding pose
        Vector3 mBindDerivedInverseScale;
        /// The inversed derived orientation of the bone in the binding pose
        Quaternion mBindDerivedInverseOrientation;
        /// The inversed derived position of the bone in the binding pose
        Vector3 mBindDerivedInversePosition;
    };


}

#endif
