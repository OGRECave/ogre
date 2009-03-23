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
#ifndef __StaticFaceGroup_H__
#define __StaticFaceGroup_H__

#include "OgrePrerequisites.h"

#include "OgrePlane.h"
#include "OgrePatchSurface.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Scene
	*  @{
	*/
	/** A type of face group, i.e. face list of procedural etc */
    enum FaceGroupType {
        FGT_FACE_LIST,
        FGT_PATCH,
        FGT_UNKNOWN
    };

    /** Collects a group of static i.e. immovable faces together which have common
        properties like the material they use, the plane they lie on.
        @remarks
            Whilst for discrete geometry (i.e. movable objects) groups of faces are
            held in the SubMesh class, for immovable objects like scenery there
            needs to ba little more flexibility in the grouping since the group is
            likely to be a small part of a huge set of geometry. In addition, because
            the faces are unmoving certain optimisations can be performed, e.g.
            precalculating a world-coordinate bounding box and normal.
        @par
            Exactly how this class is used depends on the format of the large
            static geometry used in the level. An example would be the use of this
            class in the BspNode class for indoor levels.
            For flexibility and efficiency, it is not assumed that this class holds
            details of the vertices itself, or in fact that it holds the vertex indices
            itself. Everything is manipulated via pointers so if you want this
            class to point into a block of geometry data it can.
    */
    struct StaticFaceGroup {
        // Type of face group.
        FaceGroupType fType;

        /// Is this a sky surface?
        bool isSky;

        /** Index into a buffer containing vertex definitions. Because we're
            dealing with subsets of large levels this is likely to be part-way
            through a huge vertex buffer. */
        int vertexStart;

        /** The range of vertices in the buffer this facegroup references.
            This is really for copying purposes only, so that we know which
            subset of vertices to copy from our large-level buffer into the rendering buffer.
        */
        int numVertices;

        /** Index into a buffer containing vertex indices. This buffer
            may be individual to this group or shared for memory allocation
            efficiency.The vertex indexes are relative the the mVertexStart pointer,
            not to the start of the large-level buffer, allowing simple reindexing
            when copying data into rendering buffers.
            This is only applicable to FGT_FACE_LIST face group types.
        */
        int elementStart;

        /** The number of vertex indices.
            This is only applicable to FGT_FACE_LIST face group types.
        */
        int numElements;

        /** Handle to material used by this group.
            Note the use of the material handle rather than the material
            name - this is for efficiency since there will be many of these.
        */
        int materialHandle;

        Plane plane;

        /// Patch surface (only applicable when fType = FGT_PATCH)
        PatchSurface* patchSurf;


        _OgreExport friend std::ostream& operator<<(std::ostream& o, const StaticFaceGroup& s)
        {
            o << "StaticFaceGroup(";
            if (s.fType == FGT_FACE_LIST)
            {
                o << "faceList, numVertices=" << s.numVertices << ", vertexStart=" << s.vertexStart;
                o << ", numElements=" << s.numElements << ", elementStart=" << s.elementStart;
                o << ", normal=" << s.plane.normal;
            }
            else if (s.fType == FGT_PATCH)
            {
                o << "bezierPatch, numVertices=" << s.numVertices << ", vertexStart=" << s.vertexStart;
                // TODO
            }

            o << ", materialHandle=" << s.materialHandle;
            o << ")";

            return o;
        }


    };
	/** @} */
	/** @} */

} // namespace

#endif
