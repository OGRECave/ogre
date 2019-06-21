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

#ifndef __XMLMeshSerializer_H__
#define __XMLMeshSerializer_H__

#include "OgreXMLPrerequisites.h"
#include "OgreMesh.h"


namespace Ogre {

    /** Class for serializing a Mesh to/from XML.
    @remarks
        This class behaves the same way as MeshSerializer in the main project,
        but is here to allow conversions to / from XML. This class is 
        deliberately not included in the main project because <UL>
        <LI>Dependence on Xerces would unnecessarily bloat the main library</LI>
        <LI>Runtime use of XML is discouraged because of the parsing overhead</LI></UL>
        This class gives people the option of saving out a Mesh as XML for examination
        and possible editing. It can then be converted back to the native format
        for maximum runtime efficiency.
    */
    class XMLMeshSerializer
    {
    public:

        XMLMeshSerializer();
        virtual ~XMLMeshSerializer();
        /** Imports a Mesh from the given XML file.
        @param filename The name of the file to import, expected to be in XML format.
        @param colourElementType The vertex element to use for packed colours
        @param pMesh The pre-created Mesh object to be populated.
        */
        void importMesh(const String& filename, VertexElementType colourElementType, Mesh* pMesh);

        /** Exports a mesh to the named XML file. */
        void exportMesh(const Mesh* pMesh, const String& filename);

    protected:
        // State for import
        Mesh* mMesh;
        VertexElementType mColourElementType;

        // Internal methods
        void writeMesh(const Mesh* pMesh, pugi::xml_node& rootNode);
        void writeSubMesh(pugi::xml_node& mSubmeshesNode, const SubMesh* s);
        void writeGeometry(pugi::xml_node& mParentNode, const VertexData* pData);
        void writeSkeletonLink(pugi::xml_node& mMeshNode, const String& skelName);
        void writeBoneAssignment(pugi::xml_node& mBoneAssignNode, const VertexBoneAssignment* assign);
        void writeTextureAliases(pugi::xml_node& mSubmeshesNode, const SubMesh* s);
        void writeLodInfo(pugi::xml_node& mMeshNode, const Mesh* pMesh);
        void writeLodUsageManual(pugi::xml_node& usageNode, unsigned short levelNum,
            const MeshLodUsage& usage);
        void writeLodUsageGenerated(pugi::xml_node& usageNode, unsigned short levelNum,
            const MeshLodUsage& usage, const Mesh* pMesh);
        void writeSubMeshNames(pugi::xml_node& mMeshNode, const Mesh* m);
        void writePoses(pugi::xml_node& meshNode, const Mesh* m);
        void writeAnimations(pugi::xml_node& meshNode, const Mesh* m);
        void writeMorphKeyFrames(pugi::xml_node& trackNode, const VertexAnimationTrack* track);
        void writePoseKeyFrames(pugi::xml_node& trackNode, const VertexAnimationTrack* track);
        void writeExtremes(pugi::xml_node& mMeshNode, const Mesh* m);

        void readSubMeshes(pugi::xml_node& mSubmeshesNode);
        void readGeometry(pugi::xml_node& mGeometryNode, VertexData* pData);
        void readSkeletonLink(pugi::xml_node& mSkelNode);
        void readBoneAssignments(pugi::xml_node& mBoneAssignmentsNode);
        void readBoneAssignments(pugi::xml_node& mBoneAssignmentsNode, SubMesh* sm);
        void readTextureAliases(pugi::xml_node& mTextureAliasesNode, SubMesh* sm);
        void readLodInfo(pugi::xml_node&  lodNode);
        void readLodUsageManual(pugi::xml_node& manualNode, unsigned short index);
        void readLodUsageGenerated(pugi::xml_node& genNode, unsigned short index);
        void readSubMeshNames(pugi::xml_node& mMeshNamesNode, Mesh* sm);
        void readPoses(pugi::xml_node& posesNode, Mesh *m);
        void readAnimations(pugi::xml_node& mAnimationsNode, Mesh *m);
        void readTracks(pugi::xml_node& tracksNode, Mesh *m, Animation* anim);
        void readMorphKeyFrames(pugi::xml_node& keyframesNode, VertexAnimationTrack* track,
            size_t vertexCount);
        void readPoseKeyFrames(pugi::xml_node& keyframesNode, VertexAnimationTrack* track);
        void readExtremes(pugi::xml_node& extremesNode, Mesh *m);
    };
}

#endif
