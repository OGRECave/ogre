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

#ifndef __MeshSerializer_H__
#define __MeshSerializer_H__

#include "OgrePrerequisites.h"
#include "OgreMeshSerializerImpl.h"
#include "OgreSerializer.h"

namespace Ogre {
	
	class MeshSerializerListener;

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Resources
	*  @{
	*/
	/** Class for serialising mesh data to/from an OGRE .mesh file.
    @remarks
        This class allows exporters to write OGRE .mesh files easily, and allows the
        OGRE engine to import .mesh files into instantiated OGRE Meshes.
        Note that a .mesh file can include not only the Mesh, but also definitions of
        any Materials it uses (although this is optional, the .mesh can rely on the
        Material being loaded from another source, especially useful if you want to
        take advantage of OGRE's advanced Material properties which may not be available
        in your modeller).
    @par
        To export a Mesh:<OL>
        <LI>Use the MaterialManager methods to create any dependent Material objects, if you want
            to export them with the Mesh.</LI>
        <LI>Create a Mesh object and populate it using it's methods.</LI>
        <LI>Call the exportMesh method</LI>
        </OL>
    @par
        It's important to realise that this exporter uses OGRE terminology. In this context,
        'Mesh' means a top-level mesh structure which can actually contain many SubMeshes, each
        of which has only one Material. Modelling packages may refer to these differently, for
        example in Milkshape, it says 'Model' instead of 'Mesh' and 'Mesh' instead of 'SubMesh', 
        but the theory is the same.
    */
    class _OgreExport MeshSerializer : public Serializer
    {
    public:
        MeshSerializer();
        virtual ~MeshSerializer();


        /** Exports a mesh to the file specified. 
        @remarks
            This method takes an externally created Mesh object, and exports both it
            and optionally the Materials it uses to a .mesh file.
        @param pMesh Pointer to the Mesh to export
        @param filename The destination filename
		@param endianMode The endian mode of the written file
        */
        void exportMesh(const Mesh* pMesh, const String& filename,
			Endian endianMode = ENDIAN_NATIVE);

        /** Imports Mesh and (optionally) Material data from a .mesh file DataStream.
        @remarks
            This method imports data from a DataStream opened from a .mesh file and places it's
            contents into the Mesh object which is passed in. 
        @param stream The DataStream holding the .mesh data. Must be initialised (pos at the start of the buffer).
        @param pDest Pointer to the Mesh object which will receive the data. Should be blank already.
        */
        void importMesh(DataStreamPtr& stream, Mesh* pDest);

		/// Sets the listener for this serializer
		void setListener(MeshSerializerListener *listener);
		/// Returns the current listener
		MeshSerializerListener *getListener();
    protected:
        static String msCurrentVersion;

        typedef map<String, MeshSerializerImpl* >::type MeshSerializerImplMap;
        MeshSerializerImplMap mImplementations;

		MeshSerializerListener *mListener;

    };

	/** 
	 @remarks
		This class allows users to hook into the mesh loading process and
		modify references within the mesh as they are loading. Material and
		skeletal references can be processed using this interface which allows
		finer control over resources.
	*/
	class MeshSerializerListener
	{
	public:
		virtual ~MeshSerializerListener() {}
		/// Called to override the loading of the given named material
		virtual void processMaterialName(Mesh *mesh, String *name) = 0;
		/// Called to override the reference to a skeleton
		virtual void processSkeletonName(Mesh *mesh, String *name) = 0;
	};
	/** @} */
	/** @} */
}


#endif
