/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

 Copyright (c) 2000-2013 Torus Knot Software Ltd

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
#ifndef __OGRE_POSE_H
#define __OGRE_POSE_H

#include "OgrePrerequisites.h"
#include "OgreString.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreVector3.h"
#include "OgreIteratorWrappers.h"
#include "OgreHeaderPrefix.h"

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
		/// A collection of normals based on the vertex index
		typedef map<size_t, Vector3>::type NormalsMap;
		/// An iterator over the vertex offsets
		typedef MapIterator<NormalsMap> NormalsIterator;
		/// An iterator over the vertex offsets
		typedef ConstMapIterator<NormalsMap> ConstNormalsIterator;
		/// Return whether the pose vertices include normals
		bool getIncludesNormals() const { return !mNormalsMap.empty(); }

		/** Adds an offset to a vertex for this pose. 
		@param index The vertex index
		@param offset The position offset for this pose
		*/
		void addVertex(size_t index, const Vector3& offset);

		/** Adds an offset to a vertex and a new normal for this pose. 
		@param index The vertex index
		@param offset The position offset for this pose
		*/
		void addVertex(size_t index, const Vector3& offset, const Vector3& normal);

		/** Remove a vertex offset. */
		void removeVertex(size_t index);

		/** Clear all vertices. */
		void clearVertices(void);

		/** Gets an iterator over all the vertex offsets. */
		ConstVertexOffsetIterator getVertexOffsetIterator(void) const;
		/** Gets an iterator over all the vertex offsets. */
		VertexOffsetIterator getVertexOffsetIterator(void);
		/** Gets a const reference to the vertex offsets. */
		const VertexOffsetMap& getVertexOffsets(void) const { return mVertexOffsetMap; }

		/** Gets an iterator over all the vertex offsets. */
		ConstNormalsIterator getNormalsIterator(void) const;
		/** Gets an iterator over all the vertex offsets. */
		NormalsIterator getNormalsIterator(void);
		/** Gets a const reference to the vertex offsets. */
		const NormalsMap& getNormals(void) const { return mNormalsMap; }

		/** Get a hardware vertex buffer version of the vertex offsets. */
		const HardwareVertexBufferSharedPtr& _getHardwareVertexBuffer(const VertexData* origData) const;

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
		/// Primary storage, sparse vertex use
		NormalsMap mNormalsMap;
		/// Derived hardware buffer, covers all vertices
		mutable HardwareVertexBufferSharedPtr mBuffer;
	};
	typedef vector<Pose*>::type PoseList;

	/** @} */
	/** @} */

}

#include "OgreHeaderSuffix.h"

#endif
