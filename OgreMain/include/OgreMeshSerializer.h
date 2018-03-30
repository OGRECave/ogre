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

#ifndef __MeshSerializer_H__
#define __MeshSerializer_H__

#include "OgrePrerequisites.h"
#include "OgreSerializer.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
    
    class MeshSerializerListener;
    class MeshVersionData;
    
    /// Mesh compatibility versions
    enum MeshVersion 
    {
        /// Latest version available
        MESH_VERSION_LATEST,
        
        /// OGRE version v1.10+
        MESH_VERSION_1_10,
        /// OGRE version v1.8+
        MESH_VERSION_1_8,
        /// OGRE version v1.7+
        MESH_VERSION_1_7,
        /// OGRE version v1.4+
        MESH_VERSION_1_4,
        /// OGRE version v1.0+
        MESH_VERSION_1_0,
        
        /// Legacy versions, DO NOT USE for writing
        MESH_VERSION_LEGACY
    };

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


        /** Exports a mesh to the file specified, in the latest format
        @remarks
            This method takes an externally created Mesh object, and exports it
            to a .mesh file in the latest format version available.
        @param pMesh Pointer to the Mesh to export
        @param filename The destination filename
        @param endianMode The endian mode of the written file
        */
        void exportMesh(const Mesh* pMesh, const String& filename,
            Endian endianMode = ENDIAN_NATIVE);

        /** Exports a mesh to the file specified, in a specific version format. 
         @remarks
         This method takes an externally created Mesh object, and exports it
         to a .mesh file in the specified format version. Note that picking a
         format version other that the latest will cause some information to be
         lost.
         @param pMesh Pointer to the Mesh to export
         @param filename The destination filename
         @param version Mesh version to write
         @param endianMode The endian mode of the written file
         */
        void exportMesh(const Mesh* pMesh, const String& filename,
                        MeshVersion version,
                        Endian endianMode = ENDIAN_NATIVE);

        /** Exports a mesh to the stream specified, in the latest format. 
        @remarks
         This method takes an externally created Mesh object, and exports it
         to a .mesh file in the latest format version. 
        @param pMesh Pointer to the Mesh to export
        @param stream Writeable stream
        @param endianMode The endian mode of the written file
        */
        void exportMesh(const Mesh* pMesh, DataStreamPtr stream,
            Endian endianMode = ENDIAN_NATIVE);

        /** Exports a mesh to the stream specified, in a specific version format. 
         @remarks
         This method takes an externally created Mesh object, and exports it
         to a .mesh file in the specified format version. Note that picking a
         format version other that the latest will cause some information to be
         lost.
         @param pMesh Pointer to the Mesh to export
         @param stream Writeable stream
         @param version Mesh version to write
         @param endianMode The endian mode of the written file
         */
        void exportMesh(const Mesh* pMesh, DataStreamPtr stream,
                        MeshVersion version,
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
        typedef std::vector<MeshVersionData*> MeshVersionDataList;
        MeshVersionDataList mVersionData;

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
        /// Allows to do changes on mesh after it's completely loaded. For example you can generate LOD levels here.
        virtual void processMeshCompleted(Mesh *mesh) = 0;
    };
    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
