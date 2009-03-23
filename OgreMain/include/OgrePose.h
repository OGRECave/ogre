/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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
#ifndef __OGRE_POSE_H
#define __OGRE_POSE_H

#include "OgrePrerequisites.h"
#include "OgreString.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreVector3.h"
#include "OgreIteratorWrappers.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Animation
	*  @{
	*/
	/** A pose is a linked set of vertex offsets applying to one set of vertex
		data. 
	@remarks
		The target index referred to by the pose has a meaning set by the user
		of this class; but for example when used by Mesh it refers to either the
		Mesh shared geometry (0) or a SubMesh dedicated geometry (1+).
		Pose instances can be referred to by keyframes in VertexAnimationTrack in
		order to animate based on blending poses together.
	*/
	class _OgreExport Pose : public AnimationAlloc
	{
	public:
		/** Constructor
			@param target The target vertexdata index (0 for shared, 1+ for 
				dedicated at the submesh index + 1)
			@param name Optional name
		*/
		Pose(ushort target, const String& name = StringUtil::BLANK);
		virtual ~Pose();
		/// Return the name of the pose (may be blank)
		const String& getName(void) const { return mName; }
		/// Return the target geometry index of the pose
		ushort getTarget(void) const { return mTarget; }
		/// A collection of vertex offsets based on the vertex index
		typedef map<size_t, Vector3>::type VertexOffsetMap;
		/// An iterator over the vertex offsets
		typedef MapIterator<VertexOffsetMap> VertexOffsetIterator;
		/// An iterator over the vertex offsets
		typedef ConstMapIterator<VertexOffsetMap> ConstVertexOffsetIterator;

		/** Adds an offset to a vertex for this pose. 
		@param index The vertex index
		@param offset The position offset for this pose
		*/
		void addVertex(size_t index, const Vector3& offset);

		/** Remove a vertex offset. */
		void removeVertex(size_t index);

		/** Clear all vertex offsets. */
		void clearVertexOffsets(void);

		/** Gets an iterator over all the vertex offsets. */
		ConstVertexOffsetIterator getVertexOffsetIterator(void) const;
		/** Gets an iterator over all the vertex offsets. */
		VertexOffsetIterator getVertexOffsetIterator(void);
		/** Gets a const reference to the vertex offsets. */
		const VertexOffsetMap& getVertexOffsets(void) const { return mVertexOffsetMap; }

		/** Get a hardware vertex buffer version of the vertex offsets. */
		const HardwareVertexBufferSharedPtr& _getHardwareVertexBuffer(size_t numVertices) const;

		/** Clone this pose and create another one configured exactly the same
			way (only really useful for cloning holders of this class).
		*/
		Pose* clone(void) const;
	protected:
		/// Target geometry index
		ushort mTarget;
		/// Optional name
		String mName;
		/// Primary storage, sparse vertex use
		VertexOffsetMap mVertexOffsetMap;
		/// Derived hardware buffer, covers all vertices
		mutable HardwareVertexBufferSharedPtr mBuffer;
	};
	typedef vector<Pose*>::type PoseList;

	/** @} */
	/** @} */

}

#endif
