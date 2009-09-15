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
#include "OgreStableHeaders.h"

#include "OgreMeshSerializer.h"
#include "OgreMeshFileFormat.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreSkeleton.h"

namespace Ogre {

    String MeshSerializer::msCurrentVersion = "[MeshSerializer_v1.41]";
    const unsigned short HEADER_CHUNK_ID = 0x1000;
    //---------------------------------------------------------------------
    MeshSerializer::MeshSerializer()
		:mListener(0)
    {
        // Set up map
        mImplementations.insert(
            MeshSerializerImplMap::value_type("[MeshSerializer_v1.10]", 
            OGRE_NEW MeshSerializerImpl_v1_1() ) );

        mImplementations.insert(
            MeshSerializerImplMap::value_type("[MeshSerializer_v1.20]", 
            OGRE_NEW MeshSerializerImpl_v1_2() ) );

        mImplementations.insert(
            MeshSerializerImplMap::value_type("[MeshSerializer_v1.30]", 
            OGRE_NEW MeshSerializerImpl_v1_3() ) );

        mImplementations.insert(
            MeshSerializerImplMap::value_type("[MeshSerializer_v1.40]", 
            OGRE_NEW MeshSerializerImpl_v1_4() ) );

        mImplementations.insert(
            MeshSerializerImplMap::value_type(msCurrentVersion, 
            OGRE_NEW MeshSerializerImpl() ) );
    }
    //---------------------------------------------------------------------
    MeshSerializer::~MeshSerializer()
    {
        // delete map
        for (MeshSerializerImplMap::iterator i = mImplementations.begin();
            i != mImplementations.end(); ++i)
        {
            OGRE_DELETE i->second;
        }
        mImplementations.clear();

    }
    //---------------------------------------------------------------------
    void MeshSerializer::exportMesh(const Mesh* pMesh, const String& filename,
		Endian endianMode)
    {
        MeshSerializerImplMap::iterator impl = mImplementations.find(msCurrentVersion);
        if (impl == mImplementations.end())
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Cannot find serializer implementation for "
                "current version " + msCurrentVersion, "MeshSerializer::exportMesh");
        }

        impl->second->exportMesh(pMesh, filename, endianMode);
    }
    //---------------------------------------------------------------------
    void MeshSerializer::importMesh(DataStreamPtr& stream, Mesh* pDest)
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
        MeshSerializerImplMap::iterator impl = mImplementations.find(ver);
        if (impl == mImplementations.end())
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Cannot find serializer implementation for "
                "current version " + ver, "MeshSerializer::importMesh");
        }

        // Call implementation
        impl->second->importMesh(stream, pDest, mListener);
        // Warn on old version of mesh
        if (ver != msCurrentVersion)
        {
            LogManager::getSingleton().logMessage("WARNING: " + pDest->getName() + 
                " is an older format (" + ver + "); you should upgrade it as soon as possible" +
                " using the OgreMeshUpgrade tool.");
        }

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

