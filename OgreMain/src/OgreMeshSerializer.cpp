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
#include "OgreStableHeaders.h"
#include "OgreMeshSerializerImpl.h"

namespace Ogre {

    class _OgrePrivate MeshVersionData : public SerializerAlloc
    {
    public:
        MeshVersion version;
        String versionString;
        MeshSerializerImpl* impl;

        MeshVersionData(MeshVersion _ver, const String& _string, MeshSerializerImpl* _impl)
        : version(_ver), versionString(_string), impl(_impl) {}

        ~MeshVersionData() { OGRE_DELETE impl; }

    };

    const unsigned short HEADER_CHUNK_ID = 0x1000;
    //---------------------------------------------------------------------
    MeshSerializer::MeshSerializer()
        :mListener(0)
    {
        // Init implementations
        // String identifiers have not always been 100% unified with OGRE version
        
        // Note MUST be added in reverse order so latest is first in the list

        // This one is a little ugly, 1.10 is used for version 1.1 legacy meshes.
        // So bump up to 1.100
        mVersionData.push_back(OGRE_NEW MeshVersionData(
            MESH_VERSION_1_10, "[MeshSerializer_v1.100]", 
            OGRE_NEW MeshSerializerImpl()));

        mVersionData.push_back(OGRE_NEW MeshVersionData(
            MESH_VERSION_1_8, "[MeshSerializer_v1.8]", 
            OGRE_NEW MeshSerializerImpl_v1_8()));

        mVersionData.push_back(OGRE_NEW MeshVersionData(
            MESH_VERSION_1_7, "[MeshSerializer_v1.41]", 
            OGRE_NEW MeshSerializerImpl_v1_41()));

        mVersionData.push_back(OGRE_NEW MeshVersionData(
            MESH_VERSION_1_4, "[MeshSerializer_v1.40]", 
            OGRE_NEW MeshSerializerImpl_v1_4()));

        mVersionData.push_back(OGRE_NEW MeshVersionData(
            MESH_VERSION_1_0, "[MeshSerializer_v1.30]", 
            OGRE_NEW MeshSerializerImpl_v1_3()));
        mVersionData.push_back(OGRE_NEW MeshVersionData(
            MESH_VERSION_LEGACY, "[MeshSerializer_v1.20]", 
            OGRE_NEW MeshSerializerImpl_v1_2()));

        mVersionData.push_back(OGRE_NEW MeshVersionData(
            MESH_VERSION_LEGACY, "[MeshSerializer_v1.10]", 
            OGRE_NEW MeshSerializerImpl_v1_1()));
        
    }
    //---------------------------------------------------------------------
    MeshSerializer::~MeshSerializer()
    {
        // delete map
        for (MeshVersionDataList::iterator i = mVersionData.begin();
            i != mVersionData.end(); ++i)
        {
            OGRE_DELETE *i;
        }
        mVersionData.clear();

    }
    //---------------------------------------------------------------------
    void MeshSerializer::exportMesh(const Mesh* pMesh, const String& filename,
        Endian endianMode)
    {
        DataStreamPtr stream = _openFileStream(filename, std::ios::binary | std::ios::out);

        exportMesh(pMesh, stream, endianMode);

        stream->close();
    }
    //---------------------------------------------------------------------
    void MeshSerializer::exportMesh(const Mesh* pMesh, const String& filename,
                                    MeshVersion version, Endian endianMode)
    {
        DataStreamPtr stream = _openFileStream(filename, std::ios::binary | std::ios::out);
        
        exportMesh(pMesh, stream, version, endianMode);
        
        stream->close();
    }
    //---------------------------------------------------------------------
    void MeshSerializer::exportMesh(const Mesh* pMesh, DataStreamPtr stream,
        Endian endianMode)
    {
        exportMesh(pMesh, stream, MESH_VERSION_LATEST, endianMode);
    }
    //---------------------------------------------------------------------
    void MeshSerializer::exportMesh(const Mesh* pMesh, DataStreamPtr stream,
                                    MeshVersion version, Endian endianMode)
    {
        if (version == MESH_VERSION_LEGACY)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                        "You may not supply a legacy version number (pre v1.0) for writing meshes.",
                        "MeshSerializer::exportMesh");
        
        MeshSerializerImpl* impl = 0;
        if (version == MESH_VERSION_LATEST)
            impl = mVersionData[0]->impl;
        else 
        {
            for (MeshVersionDataList::iterator i = mVersionData.begin(); 
                 i != mVersionData.end(); ++i)
            {
                if (version == (*i)->version)
                {
                    impl = (*i)->impl;
                    break;
                }
            }
        }
        
        if (!impl)
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Cannot find serializer implementation for "
                    "specified version", "MeshSerializer::exportMesh");

                    
        impl->exportMesh(pMesh, stream, endianMode);
    }
    //---------------------------------------------------------------------
    void MeshSerializer::importMesh(const DataStreamPtr& stream, Mesh* pDest)
    {
        determineEndianness(stream);

        // Read header and determine the version
        unsigned short headerID;
        
        // Read header ID
        readShorts(stream, &headerID, 1);
        
        if (headerID != HEADER_CHUNK_ID)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "File header not found",
                "MeshSerializer::importMesh");
        }
        // Read version
        String ver = readString(stream);
        // Jump back to start
        stream->seek(0);

        // Find the implementation to use
        MeshSerializerImpl* impl = 0;
        for (MeshVersionDataList::iterator i = mVersionData.begin(); 
             i != mVersionData.end(); ++i)
        {
            if ((*i)->versionString == ver)
            {
                impl = (*i)->impl;
                break;
            }
        }           
        if (!impl)
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Cannot find serializer implementation for "
                        "mesh version " + ver, "MeshSerializer::importMesh");
        
        // Call implementation
        impl->importMesh(stream, pDest, mListener);
        // Warn on old version of mesh
        if (ver != mVersionData[0]->versionString)
        {
            LogManager::getSingleton().logWarning(pDest->getName() + " uses an old format " + ver +
                                                  "; upgrade with the OgreMeshUpgrader tool");
        }

        if(mListener)
            mListener->processMeshCompleted(pDest);
    }
    //---------------------------------------------------------------------
    void MeshSerializer::setListener(Ogre::MeshSerializerListener *listener)
    {
        mListener = listener;
    }
    //-------------------------------------------------------------------------
    MeshSerializerListener *MeshSerializer::getListener()
    {
        return mListener;
    }
}

