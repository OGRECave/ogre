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


#include "OgreXMLMeshSerializer.h"
#include "OgreSubMesh.h"
#include "OgreLogManager.h"
#include "OgreSkeleton.h"
#include "OgreStringConverter.h"
#include "OgreHardwareBufferManager.h"
#include "OgreException.h"
#include "OgreAnimation.h"
#include "OgreAnimationTrack.h"
#include "OgreKeyFrame.h"
#include "OgreLodStrategyManager.h"
#include "OgreLodStrategy.h"
#include "OgreMaterialManager.h"
#include "OgreSkeletonManager.h"
#include <cstddef>

namespace Ogre {

    //---------------------------------------------------------------------
    XMLMeshSerializer::XMLMeshSerializer()
    {
    }
    //---------------------------------------------------------------------
    XMLMeshSerializer::~XMLMeshSerializer()
    {
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::importMesh(const String& filename, 
        VertexElementType colourElementType, Mesh* pMesh)
    {
        LogManager::getSingleton().logMessage("XMLMeshSerializer reading mesh data from " + filename + "...");
        mMesh = pMesh;
        mColourElementType = colourElementType;
        pugi::xml_document mXMLDoc;
        mXMLDoc.load_file(filename.c_str());

        pugi::xml_node elem;

        pugi::xml_node rootElem = mXMLDoc.document_element();

        // shared geometry
        elem = rootElem.child("sharedgeometry");
        if (elem)
        {
            if(StringConverter::parseInt(elem.attribute("vertexcount").value()) > 0)
            {
                mMesh->sharedVertexData = new VertexData();
                readGeometry(elem, mMesh->sharedVertexData);
            }
        }

        // submeshes
        elem = rootElem.child("submeshes");
        if (elem)
            readSubMeshes(elem);

        // skeleton link
        elem = rootElem.child("skeletonlink");
        if (elem)
            readSkeletonLink(elem);

        // bone assignments
        elem = rootElem.child("boneassignments");
        if (elem)
            readBoneAssignments(elem);

        //Lod
        elem = rootElem.child("levelofdetail");
        if (elem)
            readLodInfo(elem);

        // submesh names
        elem = rootElem.child("submeshnames");
        if (elem)
            readSubMeshNames(elem, mMesh);

        // submesh extremes
        elem = rootElem.child("extremes");
        if (elem)
            readExtremes(elem, mMesh);

        // poses
        elem = rootElem.child("poses");
        if (elem)
            readPoses(elem, mMesh);

        // animations
        elem = rootElem.child("animations");
        if (elem)
            readAnimations(elem, mMesh);

        LogManager::getSingleton().logMessage("XMLMeshSerializer import successful.");
        
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::exportMesh(const Mesh* pMesh, const String& filename)
    {
        LogManager::getSingleton().logMessage("XMLMeshSerializer writing mesh data to " + filename + "...");
        
        mMesh = const_cast<Mesh*>(pMesh);

        pugi::xml_document mXMLDoc;
        pugi::xml_node rootNode = mXMLDoc.append_child("mesh");

        LogManager::getSingleton().logMessage("Populating DOM...");

            
           
        // Write to DOM
        writeMesh(pMesh, rootNode);
        LogManager::getSingleton().logMessage("DOM populated, writing XML file..");

        // Write out to a file
        if(! mXMLDoc.save_file(filename.c_str()) )
        {
            LogManager::getSingleton().logMessage("XMLMeshSerializer failed writing the XML file.", LML_CRITICAL);
        }
        else
        {
            LogManager::getSingleton().logMessage("XMLMeshSerializer export successful.");
        }
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeMesh(const Mesh* pMesh, pugi::xml_node& rootNode)
    {
        // Write geometry
        if (pMesh->sharedVertexData)
        {
            pugi::xml_node geomNode = rootNode.append_child("sharedgeometry");
            writeGeometry(geomNode, pMesh->sharedVertexData);
        }

        // Write Submeshes
        pugi::xml_node subMeshesNode = rootNode.append_child("submeshes");
        for (size_t i = 0; i < pMesh->getNumSubMeshes(); ++i)
        {
            LogManager::getSingleton().logMessage("Writing submesh...");
            writeSubMesh(subMeshesNode, pMesh->getSubMesh(i));
            LogManager::getSingleton().logMessage("Submesh exported.");
        }

        // Write skeleton info if required
        if (pMesh->hasSkeleton())
        {
            LogManager::getSingleton().logMessage("Exporting skeleton link...");
            // Write skeleton link
            writeSkeletonLink(rootNode, pMesh->getSkeletonName());
            LogManager::getSingleton().logMessage("Skeleton link exported.");

            // Write bone assignments
            const auto& boneAssigns = pMesh->getBoneAssignments();
            if (!boneAssigns.empty())
            {
                LogManager::getSingleton().logMessage("Exporting shared geometry bone assignments...");
                pugi::xml_node boneAssignNode = rootNode.append_child("boneassignments");

                for (const auto& e : boneAssigns)
                {
                    writeBoneAssignment(boneAssignNode, &e.second);
                }

                LogManager::getSingleton().logMessage("Shared geometry bone assignments exported.");
            }
        }
        if (pMesh->getNumLodLevels() > 1)
        {
            LogManager::getSingleton().logMessage("Exporting LOD information...");
            writeLodInfo(rootNode, pMesh);
            LogManager::getSingleton().logMessage("LOD information exported.");
        }
        // Write submesh names
        writeSubMeshNames(rootNode, pMesh);
        // Write poses
        writePoses(rootNode, pMesh);
        // Write animations
        writeAnimations(rootNode, pMesh);
        // Write extremes
        writeExtremes(rootNode, pMesh);
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeSubMesh(pugi::xml_node& mSubMeshesNode, const SubMesh* s)
    {
        pugi::xml_node subMeshNode = mSubMeshesNode.append_child("submesh");

        size_t numFaces;

        // Material name
        subMeshNode.append_attribute("material") = s->getMaterialName().c_str();
        // bool useSharedVertices
        subMeshNode.append_attribute("usesharedvertices") =
            StringConverter::toString(s->useSharedVertices).c_str();
        // bool use32BitIndexes
        bool use32BitIndexes = (s->indexData->indexBuffer && 
            s->indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT);
        subMeshNode.append_attribute("use32bitindexes") =
            StringConverter::toString( use32BitIndexes ).c_str();

        // Operation type
        switch(s->operationType)
        {
        case RenderOperation::OT_LINE_LIST:
            subMeshNode.append_attribute("operationtype") = "line_list";
            break;
        case RenderOperation::OT_LINE_STRIP:
            subMeshNode.append_attribute("operationtype") = "line_strip";
            break;
        case RenderOperation::OT_POINT_LIST:
            subMeshNode.append_attribute("operationtype") = "point_list";
            break;
        case RenderOperation::OT_TRIANGLE_FAN:
            subMeshNode.append_attribute("operationtype") = "triangle_fan";
            break;
        case RenderOperation::OT_TRIANGLE_LIST:
            subMeshNode.append_attribute("operationtype") = "triangle_list";
            break;
        case RenderOperation::OT_TRIANGLE_STRIP:
            subMeshNode.append_attribute("operationtype") = "triangle_strip";
            break;
        case RenderOperation::OT_TRIANGLE_LIST_ADJ:
            subMeshNode.append_attribute("operationtype") = "triangle_list_adj";
            break;
        case RenderOperation::OT_TRIANGLE_STRIP_ADJ:
            subMeshNode.append_attribute("operationtype") = "triangle_strip_adj";
            break;
        case RenderOperation::OT_LINE_LIST_ADJ:
            subMeshNode.append_attribute("operationtype") = "line_list_adj";
            break;
        case RenderOperation::OT_LINE_STRIP_ADJ:
            subMeshNode.append_attribute("operationtype") = "line_strip_adj";
            break;
        default:
            OgreAssert(false, "Patch control point operations not supported");
            break;
        }

        if (s->indexData->indexCount > 0)
        {
            // Faces
            pugi::xml_node facesNode =
                subMeshNode.append_child("faces");
            switch(s->operationType)
            {
            case RenderOperation::OT_TRIANGLE_LIST:
                // tri list
                numFaces = s->indexData->indexCount / 3;

                break;
            case RenderOperation::OT_LINE_LIST:
                numFaces = s->indexData->indexCount / 2;

                break;
            case RenderOperation::OT_TRIANGLE_FAN:
            case RenderOperation::OT_TRIANGLE_STRIP:
                // triangle fan or triangle strip
                numFaces = s->indexData->indexCount - 2;

                break;
            default:
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Unsupported render operation type");
            }
            facesNode.append_attribute("count") = StringConverter::toString(numFaces).c_str();
            // Write each face in turn
            size_t i;
            unsigned int* pInt = 0;
            unsigned short* pShort = 0;
            HardwareIndexBufferSharedPtr ibuf = s->indexData->indexBuffer;
            if (use32BitIndexes)
            {
                pInt = static_cast<unsigned int*>(
                    ibuf->lock(HardwareBuffer::HBL_READ_ONLY)); 
            }
            else
            {
                pShort = static_cast<unsigned short*>(
                    ibuf->lock(HardwareBuffer::HBL_READ_ONLY)); 
            }
            for (i = 0; i < numFaces; ++i)
            {
                pugi::xml_node faceNode = facesNode.append_child("face");
                if (use32BitIndexes)
                {
                    faceNode.append_attribute("v1") = StringConverter::toString(*pInt++).c_str();
                    if(s->operationType == RenderOperation::OT_LINE_LIST)
                    {
                        faceNode.append_attribute("v2") = StringConverter::toString(*pInt++).c_str();
                    }
                    /// Only need all 3 vertex indices if trilist or first face
                    else if (s->operationType == RenderOperation::OT_TRIANGLE_LIST || i == 0)
                    {
                        faceNode.append_attribute("v2") = StringConverter::toString(*pInt++).c_str();
                        faceNode.append_attribute("v3") = StringConverter::toString(*pInt++).c_str();
                    }
                }
                else
                {
                    faceNode.append_attribute("v1") = StringConverter::toString(*pShort++).c_str();
                    if(s->operationType == RenderOperation::OT_LINE_LIST)
                    {
                        faceNode.append_attribute("v2") = StringConverter::toString(*pShort++).c_str();
                    }
                    /// Only need all 3 vertex indices if trilist or first face
                    else if (s->operationType == RenderOperation::OT_TRIANGLE_LIST || i == 0)
                    {
                        faceNode.append_attribute("v2") = StringConverter::toString(*pShort++).c_str();
                        faceNode.append_attribute("v3") = StringConverter::toString(*pShort++).c_str();
                    }
                }
            }
            ibuf->unlock();
        }

        // M_GEOMETRY chunk (Optional: present only if useSharedVertices = false)
        if (!s->useSharedVertices)
        {
            pugi::xml_node geomNode = subMeshNode.append_child("geometry");
            writeGeometry(geomNode, s->vertexData);
        }

        // texture aliases
        writeTextureAliases(subMeshNode, s);

        // Bone assignments
        if (mMesh->hasSkeleton())
        {
            LogManager::getSingleton().logMessage("Exporting dedicated geometry bone assignments...");

            pugi::xml_node boneAssignNode = subMeshNode.append_child("boneassignments");
            for (const auto& e : s->getBoneAssignments())
            {
                writeBoneAssignment(boneAssignNode, &e.second);
            }
        }
        LogManager::getSingleton().logMessage("Dedicated geometry bone assignments exported.");

    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeGeometry(pugi::xml_node& mParentNode, const VertexData* vertexData)
    {
        // Write a vertex buffer per element

        pugi::xml_node vbNode, vertexNode, dataNode;

        // Set num verts on parent
        mParentNode.append_attribute("vertexcount") = StringConverter::toString(vertexData->vertexCount).c_str();

        VertexDeclaration* decl = vertexData->vertexDeclaration;
        VertexBufferBinding* bind = vertexData->vertexBufferBinding;

        VertexBufferBinding::VertexBufferBindingMap::const_iterator b, bend;
        bend = bind->getBindings().end();
        // Iterate over buffers
        for(b = bind->getBindings().begin(); b != bend; ++b)
        {
            vbNode = mParentNode.append_child("vertexbuffer");
            const HardwareVertexBufferSharedPtr vbuf = b->second;
            unsigned short bufferIdx = b->first;
            // Get all the elements that relate to this buffer          
            VertexDeclaration::VertexElementList elems = decl->findElementsBySource(bufferIdx);
            VertexDeclaration::VertexElementList::iterator i, iend;
            iend = elems.end();

            // Set up the data access for this buffer (lock read-only)
            unsigned char* pVert;
            float* pFloat;
            uint16* pShort;
            uint8* pChar;
            ABGR* pColour;

            pVert = static_cast<unsigned char*>(
                vbuf->lock(HardwareBuffer::HBL_READ_ONLY));

            // Skim over the elements to set up the general data
            unsigned short numTextureCoords = 0;
            for (i = elems.begin(); i != iend; ++i)
            {
                VertexElement& elem = *i;
                switch(elem.getSemantic())
                {
                case VES_POSITION:
                    vbNode.append_attribute("positions") = "true";
                    break;
                case VES_NORMAL:
                    vbNode.append_attribute("normals") = "true";
                    break;
                case VES_TANGENT:
                    vbNode.append_attribute("tangents") = "true";
                    if (elem.getType() == VET_FLOAT4)
                    {
                        vbNode.append_attribute("tangent_dimensions") = "4";
                    }
                    break;
                case VES_BINORMAL:
                    vbNode.append_attribute("binormals") = "true";
                    break;
                case VES_DIFFUSE:
                    vbNode.append_attribute("colours_diffuse") = "true";
                    break;
                case VES_SPECULAR:
                    vbNode.append_attribute("colours_specular") = "true";
                    break;
                case VES_TEXTURE_COORDINATES:
                    {
                        const char *type = "float2";
                        switch (elem.getType()) 
                        {
                        case VET_FLOAT1: 
                            type = "float1"; 
                            break;
                        case VET_FLOAT2: 
                            type = "float2"; 
                            break;
                        case VET_FLOAT3: 
                            type = "float3"; 
                            break;
                        case VET_FLOAT4: 
                            type = "float4"; 
                            break;
                        case VET_UBYTE4_NORM:
                            type = "colour"; 
                            break;
                        case VET_SHORT1: 
                            type = "short1"; 
                            break;
                        case VET_SHORT2: 
                            type = "short2"; 
                            break;
                        case VET_SHORT3: 
                            type = "short3"; 
                            break;
                        case VET_SHORT4: 
                            type = "short4"; 
                            break;
                        case VET_UBYTE4: 
                            type = "ubyte4"; 
                            break;
                        default:
                            OgreAssert(false, "Unsupported VET");
                            break;
                        }
                        vbNode.append_attribute(
                            ("texture_coord_dimensions_" + StringConverter::toString(numTextureCoords)).c_str()) = type;
                        ++numTextureCoords;
                    } 
                    break;

                default:
                    break;
                }
            }
            if (numTextureCoords > 0)
            {
                vbNode.append_attribute("texture_coords") =
                    StringConverter::toString(numTextureCoords).c_str();
            }

            // For each vertex
            for (size_t v = 0; v < vertexData->vertexCount; ++v)
            {
                vertexNode = vbNode.append_child("vertex");
                // Iterate over the elements
                for (i = elems.begin(); i != iend; ++i)
                {
                    VertexElement& elem = *i;
                    switch(elem.getSemantic())
                    {
                    case VES_POSITION:
                        elem.baseVertexPointerToElement(pVert, &pFloat);
                        dataNode = vertexNode.append_child("position");
                        dataNode.append_attribute("x") = StringConverter::toString(pFloat[0]).c_str();
                        dataNode.append_attribute("y") = StringConverter::toString(pFloat[1]).c_str();
                        dataNode.append_attribute("z") = StringConverter::toString(pFloat[2]).c_str();
                        break;
                    case VES_NORMAL:
                        elem.baseVertexPointerToElement(pVert, &pFloat);
                        dataNode = vertexNode.append_child("normal");
                        dataNode.append_attribute("x") = StringConverter::toString(pFloat[0]).c_str();
                        dataNode.append_attribute("y") = StringConverter::toString(pFloat[1]).c_str();
                        dataNode.append_attribute("z") = StringConverter::toString(pFloat[2]).c_str();
                        break;
                    case VES_TANGENT:
                        elem.baseVertexPointerToElement(pVert, &pFloat);
                        dataNode = vertexNode.append_child("tangent");
                        dataNode.append_attribute("x") = StringConverter::toString(pFloat[0]).c_str();
                        dataNode.append_attribute("y") = StringConverter::toString(pFloat[1]).c_str();
                        dataNode.append_attribute("z") = StringConverter::toString(pFloat[2]).c_str();
                        if (elem.getType() == VET_FLOAT4)
                        {
                            dataNode.append_attribute("w") = StringConverter::toString(pFloat[3]).c_str();
                        }
                        break;
                    case VES_BINORMAL:
                        elem.baseVertexPointerToElement(pVert, &pFloat);
                        dataNode = vertexNode.append_child("binormal");
                        dataNode.append_attribute("x") = StringConverter::toString(pFloat[0]).c_str();
                        dataNode.append_attribute("y") = StringConverter::toString(pFloat[1]).c_str();
                        dataNode.append_attribute("z") = StringConverter::toString(pFloat[2]).c_str();
                        break;
                    case VES_DIFFUSE:
                        elem.baseVertexPointerToElement(pVert, &pColour);
                        dataNode = vertexNode.append_child("colour_diffuse");
                        {
                            ColourValue cv;
                            elem.getType() == VET_COLOUR_ABGR ? cv.setAsABGR(*pColour) : cv.setAsARGB(*pColour);
                            dataNode.append_attribute("value") = StringConverter::toString(cv).c_str();
                        }
                        break;
                    case VES_SPECULAR:
                        elem.baseVertexPointerToElement(pVert, &pColour);
                        dataNode = vertexNode.append_child("colour_specular");
                        {
                            ColourValue cv;
                            elem.getType() == VET_COLOUR_ABGR ? cv.setAsABGR(*pColour) : cv.setAsARGB(*pColour);
                            dataNode.append_attribute("value") = StringConverter::toString(cv).c_str();
                        }
                        break;
                    case VES_TEXTURE_COORDINATES:
                        dataNode = vertexNode.append_child("texcoord");

                        switch(elem.getType())
                        {
                        case VET_FLOAT1:
                            elem.baseVertexPointerToElement(pVert, &pFloat);
                            dataNode.append_attribute("u") = StringConverter::toString(*pFloat++).c_str();
                            break;
                        case VET_FLOAT2:
                            elem.baseVertexPointerToElement(pVert, &pFloat);
                            dataNode.append_attribute("u") = StringConverter::toString(*pFloat++).c_str();
                            dataNode.append_attribute("v") = StringConverter::toString(*pFloat++).c_str();
                            break;
                        case VET_FLOAT3:
                            elem.baseVertexPointerToElement(pVert, &pFloat);
                            dataNode.append_attribute("u") = StringConverter::toString(*pFloat++).c_str();
                            dataNode.append_attribute("v") = StringConverter::toString(*pFloat++).c_str();
                            dataNode.append_attribute("w") = StringConverter::toString(*pFloat++).c_str();
                            break;
                        case VET_FLOAT4:
                            elem.baseVertexPointerToElement(pVert, &pFloat);
                            dataNode.append_attribute("u") = StringConverter::toString(*pFloat++).c_str();
                            dataNode.append_attribute("v") = StringConverter::toString(*pFloat++).c_str();
                            dataNode.append_attribute("w") = StringConverter::toString(*pFloat++).c_str();
                            dataNode.append_attribute("x") = StringConverter::toString(*pFloat++).c_str();
                            break;
                        case VET_SHORT1:
                            elem.baseVertexPointerToElement(pVert, &pShort);
                            dataNode.append_attribute("u") =  StringConverter::toString(*pShort++ / 65535.0f).c_str();
                            break;
                        case VET_SHORT2:
                            elem.baseVertexPointerToElement(pVert, &pShort);
                            dataNode.append_attribute("u") =  StringConverter::toString(*pShort++ / 65535.0f).c_str();
                            dataNode.append_attribute("v") =  StringConverter::toString(*pShort++ / 65535.0f).c_str();
                            break;
                        case VET_SHORT3:
                            elem.baseVertexPointerToElement(pVert, &pShort);
                            dataNode.append_attribute("u") =  StringConverter::toString(*pShort++ / 65535.0f).c_str();
                            dataNode.append_attribute("v") =  StringConverter::toString(*pShort++ / 65535.0f).c_str();
                            dataNode.append_attribute("w") =  StringConverter::toString(*pShort++ / 65535.0f).c_str();
                            break;
                        case VET_SHORT4:
                            elem.baseVertexPointerToElement(pVert, &pShort);
                            dataNode.append_attribute("u") =  StringConverter::toString(*pShort++ / 65535.0f).c_str();
                            dataNode.append_attribute("v") =  StringConverter::toString(*pShort++ / 65535.0f).c_str();
                            dataNode.append_attribute("w") =  StringConverter::toString(*pShort++ / 65535.0f).c_str();
                            dataNode.append_attribute("x") =  StringConverter::toString(*pShort++ / 65535.0f).c_str();
                            break;
                        case VET_UBYTE4_NORM:
                            elem.baseVertexPointerToElement(pVert, &pColour);
                            {
                                ColourValue cv;
                                elem.getType() == VET_COLOUR_ARGB ? cv.setAsARGB(*pColour) : cv.setAsABGR(*pColour);
                                dataNode.append_attribute("u") = StringConverter::toString(cv).c_str();
                            }
                            break;
                        case VET_UBYTE4:
                            elem.baseVertexPointerToElement(pVert, &pChar);
                            dataNode.append_attribute("u") =  StringConverter::toString(*pChar++ / 255.0f).c_str();
                            dataNode.append_attribute("v") =  StringConverter::toString(*pChar++ / 255.0f).c_str();
                            dataNode.append_attribute("w") =  StringConverter::toString(*pChar++ / 255.0f).c_str();
                            dataNode.append_attribute("x") =  StringConverter::toString(*pChar++ / 255.0f).c_str();
                            break;
                        default:
                            OgreAssert(false, "Unsupported VET");
                            break;
                        }
                        break;
                    default:
                        break;

                    }
                }
                pVert += vbuf->getVertexSize();
            }
            vbuf->unlock();
        }

    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeSkeletonLink(pugi::xml_node& mMeshNode, const String& skelName)
    {

        pugi::xml_node skelNode = mMeshNode.append_child("skeletonlink");
        skelNode.append_attribute("name") = skelName.c_str();
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeBoneAssignment(pugi::xml_node& mBoneAssignNode, const VertexBoneAssignment* assign)
    {
        pugi::xml_node assignNode = mBoneAssignNode.append_child("vertexboneassignment");

        assignNode.append_attribute("vertexindex") =
            StringConverter::toString(assign->vertexIndex).c_str();
        assignNode.append_attribute("boneindex") =
            StringConverter::toString(assign->boneIndex).c_str();
        assignNode.append_attribute("weight" ) =
            StringConverter::toString(assign->weight).c_str();


    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeTextureAliases(pugi::xml_node& mSubmeshesNode, const SubMesh* subMesh)
    {
        if (!subMesh->hasTextureAliases())
            return; // do nothing

        pugi::xml_node textureAliasesNode = mSubmeshesNode.append_child("textures");

        // use ogre map iterator
        SubMesh::AliasTextureIterator aliasIterator = subMesh->getAliasTextureIterator();

        while (aliasIterator.hasMoreElements())
        {
            pugi::xml_node aliasTextureNode = textureAliasesNode.append_child("texture");
            // iterator key is alias and value is texture name
            aliasTextureNode.append_attribute("alias") = aliasIterator.peekNextKey().c_str();
            aliasTextureNode.append_attribute("name") = aliasIterator.peekNextValue().c_str();
            aliasIterator.moveNext();
        }

    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readSubMeshes(pugi::xml_node& mSubmeshesNode)
    {
        LogManager::getSingleton().logMessage("Reading submeshes...");
        assert(mMesh->getNumSubMeshes() == 0);
        for (pugi::xml_node& smElem : mSubmeshesNode.children())
        {
            // All children should be submeshes 
            SubMesh* sm = mMesh->createSubMesh();

            const char* mat = smElem.attribute("material").as_string(NULL);
            if (mat && mat[0] != '\0')
            {
                // we do not load any materials - so create a dummy here to just store the name
                sm->setMaterial(MaterialManager::getSingleton().create(mat, RGN_DEFAULT));
            }
            else
            {
                LogManager::getSingleton().logError(
                    "empty material name encountered. This violates the specs and can lead to crashes.");
            }

            // Read operation type
            bool readFaces = true;
            const char* optype = smElem.attribute("operationtype").as_string(NULL);
            if (optype)
            {
                if (!strcmp(optype, "triangle_list"))
                {
                    sm->operationType = RenderOperation::OT_TRIANGLE_LIST;
                }
                else if (!strcmp(optype, "triangle_fan"))
                {
                    sm->operationType = RenderOperation::OT_TRIANGLE_FAN;
                }
                else if (!strcmp(optype, "triangle_strip"))
                {
                    sm->operationType = RenderOperation::OT_TRIANGLE_STRIP;
                }
                else if (!strcmp(optype, "line_strip"))
                {
                    sm->operationType = RenderOperation::OT_LINE_STRIP;
                    readFaces = false;
                }
                else if (!strcmp(optype, "line_list"))
                {
                    sm->operationType = RenderOperation::OT_LINE_LIST;
                    readFaces = false;
                }
                else if (!strcmp(optype, "point_list"))
                {
                    sm->operationType = RenderOperation::OT_POINT_LIST;
                    readFaces = false;
                }
                else if (!strcmp(optype, "triangle_list_adj"))
                {
                    sm->operationType = RenderOperation::OT_TRIANGLE_LIST_ADJ;
                }
                else if (!strcmp(optype, "triangle_strip_adj"))
                {
                    sm->operationType = RenderOperation::OT_TRIANGLE_STRIP_ADJ;
                }
                else if (!strcmp(optype, "line_strip_adj"))
                {
                    sm->operationType = RenderOperation::OT_LINE_STRIP_ADJ;
                    readFaces = false;
                }
                else if (!strcmp(optype, "line_list_adj"))
                {
                    sm->operationType = RenderOperation::OT_LINE_LIST_ADJ;
                    readFaces = false;
                }
            }

            sm->useSharedVertices = StringConverter::parseBool(smElem.attribute("usesharedvertices").value());
            bool use32BitIndexes = StringConverter::parseBool(smElem.attribute("use32bitindexes").value());
            
            // Faces
            if (readFaces)
            {
                pugi::xml_node faces = smElem.child("faces");
                int actualCount = std::distance(faces.begin(), faces.end());
                const char *claimedCount_ = faces.attribute("count").value();
                if (StringConverter::parseInt(claimedCount_)!=actualCount)
                {
                    LogManager::getSingleton().stream(LML_WARNING)
                        << "WARNING: face count (" << actualCount << ") " <<
                        "is not as claimed (" << claimedCount_ << ")";
                }


                if (actualCount > 0)
                {
                    // Faces
                    switch(sm->operationType)
                    {
                    case RenderOperation::OT_TRIANGLE_LIST:
                        // tri list
                        sm->indexData->indexCount = actualCount * 3;

                        break;
                    case RenderOperation::OT_LINE_LIST:
                        sm->indexData->indexCount = actualCount * 2;

                        break;
                    case RenderOperation::OT_TRIANGLE_FAN:
                    case RenderOperation::OT_TRIANGLE_STRIP:
                        // triangle fan or triangle strip
                        sm->indexData->indexCount = actualCount + 2;

                        break;
                    default:
                        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                                    "operationType not implemented");
                    }

                    // Allocate space
                    HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().
                        createIndexBuffer(
                            use32BitIndexes? HardwareIndexBuffer::IT_32BIT : HardwareIndexBuffer::IT_16BIT, 
                            sm->indexData->indexCount, 
                            HardwareBuffer::HBU_DYNAMIC,
                            false);
                    sm->indexData->indexBuffer = ibuf;
                    unsigned int *pInt = 0;
                    unsigned short *pShort = 0;
                    if (use32BitIndexes)
                    {
                        pInt = static_cast<unsigned int*>(
                            ibuf->lock(HardwareBuffer::HBL_DISCARD));
                    }
                    else
                    {
                        pShort = static_cast<unsigned short*>(
                            ibuf->lock(HardwareBuffer::HBL_DISCARD));
                    }

                    bool firstTri = true;
                    for (auto faceElem : faces.children())
                    {
                        if (use32BitIndexes)
                        {
                            *pInt++ = StringConverter::parseInt(faceElem.attribute("v1").value());
                            if(sm->operationType == RenderOperation::OT_LINE_LIST)
                            {
                                *pInt++ = StringConverter::parseInt(faceElem.attribute("v2").value());
                            }
                            // only need all 3 vertices if it's a trilist or first tri
                            else if (sm->operationType == RenderOperation::OT_TRIANGLE_LIST || firstTri)
                            {
                                *pInt++ = StringConverter::parseInt(faceElem.attribute("v2").value());
                                *pInt++ = StringConverter::parseInt(faceElem.attribute("v3").value());
                            }
                        }
                        else
                        {
                            *pShort++ = StringConverter::parseInt(faceElem.attribute("v1").value());
                            if(sm->operationType == RenderOperation::OT_LINE_LIST)
                            {
                                *pShort++ = StringConverter::parseInt(faceElem.attribute("v2").value());
                            }
                            // only need all 3 vertices if it's a trilist or first tri
                            else if (sm->operationType == RenderOperation::OT_TRIANGLE_LIST || firstTri)
                            {
                                *pShort++ = StringConverter::parseInt(faceElem.attribute("v2").value());
                                *pShort++ = StringConverter::parseInt(faceElem.attribute("v3").value());
                            }
                        }
                        firstTri = false;
                    }
                    ibuf->unlock();
                }
            }

            // Geometry
            if (!sm->useSharedVertices)
            {
                pugi::xml_node geomNode = smElem.child("geometry");
                if (geomNode)
                {
                    sm->vertexData = new VertexData();
                    readGeometry(geomNode, sm->vertexData);
                }
            }

            // texture aliases
            pugi::xml_node textureAliasesNode = smElem.child("textures");
            if(textureAliasesNode)
                readTextureAliases(textureAliasesNode, sm);

            // Bone assignments
            pugi::xml_node boneAssigns = smElem.child("boneassignments");
            if(boneAssigns)
                readBoneAssignments(boneAssigns, sm);

        }
        LogManager::getSingleton().logMessage("Submeshes done.");
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readGeometry(pugi::xml_node& mGeometryNode, VertexData* vertexData)
    {
        LogManager::getSingleton().logMessage("Reading geometry...");
        unsigned char *pVert;
        float *pFloat;
        uint16 *pShort;
        uint8 *pChar;
        ARGB *pCol;

        ptrdiff_t claimedVertexCount =
            StringConverter::parseInt(mGeometryNode.attribute("vertexcount").value());

        // Skip empty 
        if (claimedVertexCount <= 0) return;
        

        VertexDeclaration* decl = vertexData->vertexDeclaration;
        VertexBufferBinding* bind = vertexData->vertexBufferBinding;
        unsigned short bufCount = 0;
        unsigned short totalTexCoords = 0; // across all buffers

        // Information for calculating bounds
        Vector3 min = Vector3::ZERO, max = Vector3::UNIT_SCALE, pos = Vector3::ZERO;
        Real maxSquaredRadius = -1;
        bool first = true;

        // Iterate over all children (vertexbuffer entries) 
        for (pugi::xml_node& vbElem : mGeometryNode.children("vertexbuffer"))
        {
            size_t offset = 0;
            if (StringConverter::parseBool(vbElem.attribute("positions").value()))
            {
                offset += decl->addElement(bufCount, offset, VET_FLOAT3, VES_POSITION).getSize();
            }
            if (StringConverter::parseBool(vbElem.attribute("normals").value()))
            {
                offset += decl->addElement(bufCount, offset, VET_FLOAT3, VES_NORMAL).getSize();
            }
            if (StringConverter::parseBool(vbElem.attribute("tangents").value()))
            {
                VertexElementType tangentType = VET_FLOAT3;
                unsigned int dims = StringConverter::parseUnsignedInt(vbElem.attribute("tangent_dimensions").value());
                if (dims == 4)
                    tangentType = VET_FLOAT4;

                offset += decl->addElement(bufCount, offset, tangentType, VES_TANGENT).getSize();
            }
            if (StringConverter::parseBool(vbElem.attribute("binormals").value()))
            {
                offset += decl->addElement(bufCount, offset, VET_FLOAT3, VES_BINORMAL).getSize();
            }
            if (StringConverter::parseBool(vbElem.attribute("colours_diffuse").value()))
            {
                offset += decl->addElement(bufCount, offset, mColourElementType, VES_DIFFUSE).getSize();
            }
            if (StringConverter::parseBool(vbElem.attribute("colours_specular").value()))
            {
                // Add element
                offset += decl->addElement(bufCount, offset, mColourElementType, VES_SPECULAR).getSize();
            }
            if (StringConverter::parseInt(vbElem.attribute("texture_coords").value()))
            {
                unsigned short numTexCoords = StringConverter::parseInt(vbElem.attribute("texture_coords").value());
                for (unsigned short tx = 0; tx < numTexCoords; ++tx)
                {
                    // NB set is local to this buffer, but will be translated into a 
                    // global set number across all vertex buffers
                    StringStream str;
                    str << "texture_coord_dimensions_" << tx;
                    auto attrib = vbElem.attribute(str.str().c_str()).as_string(NULL);
                    VertexElementType vtype = VET_FLOAT2; // Default
                    if (attrib)
                    {
                        if (!::strcmp(attrib,"1")) 
                            vtype = VET_FLOAT1;
                        else if (!::strcmp(attrib,"2"))
                            vtype = VET_FLOAT2;
                        else if (!::strcmp(attrib,"3"))
                            vtype = VET_FLOAT3;
                        else if (!::strcmp(attrib,"4"))
                            vtype = VET_FLOAT4;
                        else if (!::strcmp(attrib,"float1"))
                            vtype = VET_FLOAT1;
                        else if (!::strcmp(attrib,"float2"))
                            vtype = VET_FLOAT2;
                        else if (!::strcmp(attrib,"float3"))
                            vtype = VET_FLOAT3;
                        else if (!::strcmp(attrib,"float4"))
                            vtype = VET_FLOAT4;
                        else if (!::strcmp(attrib,"short1"))
                            vtype = VET_SHORT1;
                        else if (!::strcmp(attrib,"short2"))
                            vtype = VET_SHORT2;
                        else if (!::strcmp(attrib,"short3"))
                            vtype = VET_SHORT3;
                        else if (!::strcmp(attrib,"short4"))
                            vtype = VET_SHORT4;
                        else if (!::strcmp(attrib,"ubyte4"))
                            vtype = VET_UBYTE4;
                        else if (!::strcmp(attrib,"colour"))
                            vtype = VET_UBYTE4_NORM;
                        else if (!::strcmp(attrib,"colour_argb"))
                            vtype = _DETAIL_SWAP_RB;
                        else if (!::strcmp(attrib,"colour_abgr"))
                            vtype = VET_UBYTE4_NORM;
                        else 
                        {
                            auto err = LogManager::getSingleton().stream(LML_CRITICAL);
                            err << "Did not recognise texture_coord_dimensions value of \""<<attrib<<"\"\n";
                            err << "Falling back to default of VET_FLOAT2\n";
                        }
                    }
                    offset += decl->addElement(bufCount, offset, vtype,
                        VES_TEXTURE_COORDINATES, totalTexCoords++).getSize();
                }
            } 

            // calculate how many vertexes there actually are
            int actualVertexCount = std::distance(vbElem.begin(), vbElem.end());
            if (actualVertexCount!=claimedVertexCount)
            {
                LogManager::getSingleton().stream(LML_WARNING)
                    << "WARNING: vertex count (" << actualVertexCount 
                    << ") is not as claimed (" << claimedVertexCount << ")";
            }

            vertexData->vertexCount = actualVertexCount;
            // Now create the vertex buffer
            HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton().
                createVertexBuffer(offset, vertexData->vertexCount, 
                    HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
            // Bind it
            bind->setBinding(bufCount, vbuf);
            // Lock it
            pVert = static_cast<unsigned char*>(
                vbuf->lock(HardwareBuffer::HBL_DISCARD));

            // Get the element list for this buffer alone
            VertexDeclaration::VertexElementList elems = decl->findElementsBySource(bufCount);
            // Now the buffer is set up, parse all the vertices
            for (pugi::xml_node& vertexElem : vbElem.children())
            {
                // Now parse the elements, ensure they are all matched
                VertexDeclaration::VertexElementList::const_iterator ielem, ielemend;
                pugi::xml_node xmlElem;
                pugi::xml_node texCoordElem;
                ielemend = elems.end();
                for (ielem = elems.begin(); ielem != ielemend; ++ielem)
                {
                    const VertexElement& elem = *ielem;
                    // Find child for this element
                    switch(elem.getSemantic())
                    {
                    case VES_POSITION:
                        xmlElem = vertexElem.child("position");
                        if (!xmlElem)
                        {
                            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Missing <position> element.",
                                "XMLSerializer::readGeometry");
                        }
                        elem.baseVertexPointerToElement(pVert, &pFloat);

                        *pFloat++ = StringConverter::parseReal(xmlElem.attribute("x").value());
                        *pFloat++ = StringConverter::parseReal(xmlElem.attribute("y").value());
                        *pFloat++ = StringConverter::parseReal(xmlElem.attribute("z").value());

                        pos.x = StringConverter::parseReal(xmlElem.attribute("x").value());
                        pos.y = StringConverter::parseReal(xmlElem.attribute("y").value());
                        pos.z = StringConverter::parseReal(xmlElem.attribute("z").value());
                        
                        if (first)
                        {
                            min = max = pos;
                            maxSquaredRadius = pos.squaredLength();
                            first = false;
                        }
                        else
                        {
                            min.makeFloor(pos);
                            max.makeCeil(pos);
                            maxSquaredRadius = std::max(pos.squaredLength(), maxSquaredRadius);
                        }
                        break;
                    case VES_NORMAL:
                        xmlElem = vertexElem.child("normal");
                        if (!xmlElem)
                        {
                            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Missing <normal> element.",
                                "XMLSerializer::readGeometry");
                        }
                        elem.baseVertexPointerToElement(pVert, &pFloat);

                        *pFloat++ = StringConverter::parseReal(xmlElem.attribute("x").value());
                        *pFloat++ = StringConverter::parseReal(xmlElem.attribute("y").value());
                        *pFloat++ = StringConverter::parseReal(xmlElem.attribute("z").value());
                        break;
                    case VES_TANGENT:
                        xmlElem = vertexElem.child("tangent");
                        if (!xmlElem)
                        {
                            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Missing <tangent> element.",
                                "XMLSerializer::readGeometry");
                        }
                        elem.baseVertexPointerToElement(pVert, &pFloat);

                        *pFloat++ = StringConverter::parseReal(xmlElem.attribute("x").value());
                        *pFloat++ = StringConverter::parseReal(xmlElem.attribute("y").value());
                        *pFloat++ = StringConverter::parseReal(xmlElem.attribute("z").value());
                        if (elem.getType() == VET_FLOAT4)
                        {
                            *pFloat++ = StringConverter::parseReal(xmlElem.attribute("w").value());
                        }
                        break;
                    case VES_BINORMAL:
                        xmlElem = vertexElem.child("binormal");
                        if (!xmlElem)
                        {
                            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Missing <binormal> element.",
                                "XMLSerializer::readGeometry");
                        }
                        elem.baseVertexPointerToElement(pVert, &pFloat);

                        *pFloat++ = StringConverter::parseReal(xmlElem.attribute("x").value());
                        *pFloat++ = StringConverter::parseReal(xmlElem.attribute("y").value());
                        *pFloat++ = StringConverter::parseReal(xmlElem.attribute("z").value());
                        break;
                    case VES_DIFFUSE:
                        xmlElem = vertexElem.child("colour_diffuse");
                        if (!xmlElem)
                        {
                            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Missing <colour_diffuse> element.",
                                "XMLSerializer::readGeometry");
                        }
                        elem.baseVertexPointerToElement(pVert, &pCol);
                        {
                            ColourValue cv;
                            cv = StringConverter::parseColourValue(xmlElem.attribute("value").value());
                            *pCol++ = VertexElement::convertColourValue(cv, mColourElementType);
                        }
                        break;
                    case VES_SPECULAR:
                        xmlElem = vertexElem.child("colour_specular");
                        if (!xmlElem)
                        {
                            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Missing <colour_specular> element.",
                                "XMLSerializer::readGeometry");
                        }
                        elem.baseVertexPointerToElement(pVert, &pCol);
                        {
                            ColourValue cv;
                            cv = StringConverter::parseColourValue(xmlElem.attribute("value").value());
                            *pCol++ = VertexElement::convertColourValue(cv, mColourElementType);
                        }
                        break;
                    case VES_TEXTURE_COORDINATES:
                        if (!texCoordElem)
                        {
                            // Get first texcoord
                            xmlElem = vertexElem.child("texcoord");
                        }
                        else
                        {
                            // Get next texcoord
                            xmlElem = texCoordElem.next_sibling("texcoord");
                        }
                        if (!xmlElem)
                        {
                            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Missing <texcoord> element.",
                                "XMLSerializer::readGeometry");
                        }
                        // Record the latest texture coord entry
                        texCoordElem = xmlElem;

                        if (!xmlElem.attribute("u"))
                            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'u' attribute not found.", "XMLMeshSerializer::readGeometry");
                        
                        // depending on type, pack appropriately, can process colour channels separately which is a bonus
                        switch (elem.getType()) 
                        {
                        case VET_FLOAT1:
                            elem.baseVertexPointerToElement(pVert, &pFloat);
                            *pFloat++ = StringConverter::parseReal(xmlElem.attribute("u").value());
                            break;

                        case VET_FLOAT2:
                            if (!xmlElem.attribute("v"))
                                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'v' attribute not found.", "XMLMeshSerializer::readGeometry");
                            elem.baseVertexPointerToElement(pVert, &pFloat);
                            *pFloat++ = StringConverter::parseReal(xmlElem.attribute("u").value());
                            *pFloat++ = StringConverter::parseReal(xmlElem.attribute("v").value());
                            break;

                        case VET_FLOAT3:
                            if (!xmlElem.attribute("v"))
                                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'v' attribute not found.", "XMLMeshSerializer::readGeometry");
                            if (!xmlElem.attribute("w"))
                                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'w' attribute not found.", "XMLMeshSerializer::readGeometry");
                            elem.baseVertexPointerToElement(pVert, &pFloat);
                            *pFloat++ = StringConverter::parseReal(xmlElem.attribute("u").value());
                            *pFloat++ = StringConverter::parseReal(xmlElem.attribute("v").value());
                            *pFloat++ = StringConverter::parseReal(xmlElem.attribute("w").value());
                            break;

                        case VET_FLOAT4:
                            if (!xmlElem.attribute("v"))
                                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'v' attribute not found.", "XMLMeshSerializer::readGeometry");
                            if (!xmlElem.attribute("w"))
                                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'w' attribute not found.", "XMLMeshSerializer::readGeometry");
                            if (!xmlElem.attribute("x"))
                                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'x' attribute not found.", "XMLMeshSerializer::readGeometry");
                            elem.baseVertexPointerToElement(pVert, &pFloat);
                            *pFloat++ = StringConverter::parseReal(xmlElem.attribute("u").value());
                            *pFloat++ = StringConverter::parseReal(xmlElem.attribute("v").value());
                            *pFloat++ = StringConverter::parseReal(xmlElem.attribute("w").value());
                            *pFloat++ = StringConverter::parseReal(xmlElem.attribute("x").value());
                            break;

                        case VET_SHORT1:
                            elem.baseVertexPointerToElement(pVert, &pShort);
                            *pShort++ = static_cast<uint16>(65535.0f * StringConverter::parseReal(xmlElem.attribute("u").value()));
                            break;

                        case VET_SHORT2:
                            if (!xmlElem.attribute("v"))
                                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'v' attribute not found.", "XMLMeshSerializer::readGeometry");
                            elem.baseVertexPointerToElement(pVert, &pShort);
                            *pShort++ = static_cast<uint16>(65535.0f * StringConverter::parseReal(xmlElem.attribute("u").value()));
                            *pShort++ = static_cast<uint16>(65535.0f * StringConverter::parseReal(xmlElem.attribute("v").value()));
                            break;

                        case VET_SHORT3:
                            if (!xmlElem.attribute("v"))
                                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'v' attribute not found.", "XMLMeshSerializer::readGeometry");
                            if (!xmlElem.attribute("w"))
                                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'w' attribute not found.", "XMLMeshSerializer::readGeometry");
                            elem.baseVertexPointerToElement(pVert, &pShort);
                            *pShort++ = static_cast<uint16>(65535.0f * StringConverter::parseReal(xmlElem.attribute("u").value()));
                            *pShort++ = static_cast<uint16>(65535.0f * StringConverter::parseReal(xmlElem.attribute("v").value()));
                            *pShort++ = static_cast<uint16>(65535.0f * StringConverter::parseReal(xmlElem.attribute("w").value()));
                            break;

                        case VET_SHORT4:
                            if (!xmlElem.attribute("v"))
                                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'v' attribute not found.", "XMLMeshSerializer::readGeometry");
                            if (!xmlElem.attribute("w"))
                                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'w' attribute not found.", "XMLMeshSerializer::readGeometry");
                            if (!xmlElem.attribute("x"))
                                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'x' attribute not found.", "XMLMeshSerializer::readGeometry");
                            elem.baseVertexPointerToElement(pVert, &pShort);
                            *pShort++ = static_cast<uint16>(65535.0f * StringConverter::parseReal(xmlElem.attribute("u").value()));
                            *pShort++ = static_cast<uint16>(65535.0f * StringConverter::parseReal(xmlElem.attribute("v").value()));
                            *pShort++ = static_cast<uint16>(65535.0f * StringConverter::parseReal(xmlElem.attribute("w").value()));
                            *pShort++ = static_cast<uint16>(65535.0f * StringConverter::parseReal(xmlElem.attribute("x").value()));
                            break;

                        case VET_UBYTE4:
                            if (!xmlElem.attribute("v"))
                                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'v' attribute not found.", "XMLMeshSerializer::readGeometry");
                            if (!xmlElem.attribute("w"))
                                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'w' attribute not found.", "XMLMeshSerializer::readGeometry");
                            if (!xmlElem.attribute("x"))
                                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'x' attribute not found.", "XMLMeshSerializer::readGeometry");
                            elem.baseVertexPointerToElement(pVert, &pChar);
                            // round off instead of just truncating -- avoids magnifying rounding errors
                            *pChar++ = static_cast<uint8>(0.5f + 255.0f * StringConverter::parseReal(xmlElem.attribute("u").value()));
                            *pChar++ = static_cast<uint8>(0.5f + 255.0f * StringConverter::parseReal(xmlElem.attribute("v").value()));
                            *pChar++ = static_cast<uint8>(0.5f + 255.0f * StringConverter::parseReal(xmlElem.attribute("w").value()));
                            *pChar++ = static_cast<uint8>(0.5f + 255.0f * StringConverter::parseReal(xmlElem.attribute("x").value()));
                            break;

                        case VET_UBYTE4_NORM:
                            {
                                elem.baseVertexPointerToElement(pVert, &pCol);
                                ColourValue cv = StringConverter::parseColourValue(xmlElem.attribute("u").value());
                                *pCol++ = cv.getAsABGR();
                            }
                            break;
                        default:
                            OgreAssert(false, "Unsupported VET");
                            break;
                        }

                        break;
                    default:
                        break;
                    }
                } // semantic
                pVert += vbuf->getVertexSize();
            } // vertex
            bufCount++;
            vbuf->unlock();
        } // vertexbuffer

        // Set bounds
        const AxisAlignedBox& currBox = mMesh->getBounds();
        Real currRadius = mMesh->getBoundingSphereRadius();
        if (currBox.isNull())
        {
        //do not pad the bounding box
            mMesh->_setBounds(AxisAlignedBox(min, max), false);
            mMesh->_setBoundingSphereRadius(Math::Sqrt(maxSquaredRadius));
        }
        else
        {
            AxisAlignedBox newBox(min, max);
            newBox.merge(currBox);
        //do not pad the bounding box
            mMesh->_setBounds(newBox, false);
            mMesh->_setBoundingSphereRadius(std::max(Math::Sqrt(maxSquaredRadius), currRadius));
        }
        

        LogManager::getSingleton().logMessage("Geometry done...");
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readSkeletonLink(pugi::xml_node& mSkelNode)
    {
        String name = mSkelNode.attribute("name").value();
        // create dummy, because we do not load external resources
        auto skel = SkeletonManager::getSingleton().create(name, mMesh->getGroup());
        mMesh->_notifySkeleton(skel);
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readBoneAssignments(pugi::xml_node& mBoneAssignmentsNode)
    {
        LogManager::getSingleton().logMessage("Reading bone assignments...");

        // Iterate over all children (vertexboneassignment entries)
        for (pugi::xml_node& elem : mBoneAssignmentsNode.children())
        {
            VertexBoneAssignment vba;
            vba.vertexIndex = StringConverter::parseInt(elem.attribute("vertexindex").value());
            vba.boneIndex = StringConverter::parseInt(elem.attribute("boneindex").value());
            vba.weight = StringConverter::parseReal(elem.attribute("weight").value());

            mMesh->addBoneAssignment(vba);
        }

        LogManager::getSingleton().logMessage("Bone assignments done.");
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readTextureAliases(pugi::xml_node& mTextureAliasesNode, SubMesh* subMesh)
    {
        LogManager::getSingleton().logMessage("Reading sub mesh texture aliases...");

        // Iterate over all children (texture entries)
        for (pugi::xml_node& elem : mTextureAliasesNode.children())
        {
            // pass alias and texture name to submesh
            // read attribute "alias"
            String alias = elem.attribute("alias").value();
            // read attribute "name"
            String name = elem.attribute("name").value();

            subMesh->addTextureAlias(alias, name);
        }

        LogManager::getSingleton().logMessage("Texture aliases done.");
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readSubMeshNames(pugi::xml_node& mMeshNamesNode, Mesh *sm)
    {
        LogManager::getSingleton().logMessage("Reading mesh names...");

        // Iterate over all children (vertexboneassignment entries)
        for (pugi::xml_node& elem : mMeshNamesNode.children())
        {
            String meshName = elem.attribute("name").value();
            int index = StringConverter::parseInt(elem.attribute("index").value());

            sm->nameSubMesh(meshName, index);
        }

        LogManager::getSingleton().logMessage("Mesh names done.");
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readBoneAssignments(pugi::xml_node& mBoneAssignmentsNode, SubMesh* sm)
    {
        LogManager::getSingleton().logMessage("Reading bone assignments...");
        // Iterate over all children (vertexboneassignment entries)
        for (pugi::xml_node& elem : mBoneAssignmentsNode.children())
        {
            VertexBoneAssignment vba;
            vba.vertexIndex = StringConverter::parseInt(elem.attribute("vertexindex").value());
            vba.boneIndex = StringConverter::parseInt(elem.attribute("boneindex").value());
            vba.weight = StringConverter::parseReal(elem.attribute("weight").value());

            sm->addBoneAssignment(vba);
        }
        LogManager::getSingleton().logMessage("Bone assignments done.");
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeLodInfo(pugi::xml_node& mMeshNode, const Mesh* pMesh)
    {
        pugi::xml_node lodNode = mMeshNode.append_child("levelofdetail");

        const LodStrategy *strategy = pMesh->getLodStrategy();
        unsigned short numLvls = pMesh->getNumLodLevels();
        bool manual = pMesh->hasManualLodLevel();
        lodNode.append_attribute("strategy") = strategy->getName().c_str();
        lodNode.append_attribute("numlevels") = StringConverter::toString(numLvls).c_str();
        lodNode.append_attribute("manual") = StringConverter::toString(manual).c_str();

        // Iterate from level 1, not 0 (full detail)
        for (unsigned short i = 1; i < numLvls; ++i)
        {
            const MeshLodUsage& usage = pMesh->getLodLevel(i);
            if (pMesh->_isManualLodLevel(i))
            {
                writeLodUsageManual(lodNode, i, usage);
            }
            else
            {
                writeLodUsageGenerated(lodNode, i, usage, pMesh);
            }
        }

    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeSubMeshNames(pugi::xml_node& mMeshNode, const Mesh* m)
    {
        const Mesh::SubMeshNameMap& nameMap = m->getSubMeshNameMap();
        if (nameMap.empty())
            return; // do nothing

        pugi::xml_node namesNode = mMeshNode.append_child("submeshnames");
        Mesh::SubMeshNameMap::const_iterator i, iend;
        iend = nameMap.end();
        for (i = nameMap.begin(); i != iend; ++i)
        {
            pugi::xml_node subNameNode = namesNode.append_child("submeshname");

            subNameNode.append_attribute("name") = i->first.c_str();
            subNameNode.append_attribute("index") =
                StringConverter::toString(i->second).c_str();
        }

    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeLodUsageManual(pugi::xml_node& usageNode,
        unsigned short levelNum, const MeshLodUsage& usage)
    {
        pugi::xml_node manualNode = usageNode.append_child("lodmanual");

        manualNode.append_attribute("value") =
            StringConverter::toString(usage.userValue).c_str();
        manualNode.append_attribute("meshname") = usage.manualName.c_str();

    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeLodUsageGenerated(pugi::xml_node& usageNode,
        unsigned short levelNum,  const MeshLodUsage& usage, 
        const Mesh* pMesh)
    {
        pugi::xml_node generatedNode = usageNode.append_child("lodgenerated");
        generatedNode.append_attribute("value") =
            StringConverter::toString(usage.userValue).c_str();

        // Iterate over submeshes at this level
        size_t numsubs = pMesh->getNumSubMeshes();

        for (size_t subi = 0; subi < numsubs; ++subi)
        {
            pugi::xml_node subNode = generatedNode.append_child("lodfacelist");
            SubMesh* sub = pMesh->getSubMesh(subi);
            subNode.append_attribute("submeshindex") = StringConverter::toString(subi).c_str();
            // NB level - 1 because SubMeshes don't store the first index in geometry
            IndexData* facedata = sub->mLodFaceList[levelNum - 1];
            subNode.append_attribute("numfaces") =  StringConverter::toString(facedata->indexCount / 3).c_str();

            if (facedata->indexCount > 0)
            {
                // Write each face in turn
                bool use32BitIndexes = (facedata->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT);

                // Write each face in turn
                unsigned int* pInt = 0;
                unsigned short* pShort = 0;
                HardwareIndexBufferSharedPtr ibuf = facedata->indexBuffer;
                if (use32BitIndexes)
                {
                    pInt = static_cast<unsigned int*>(
                        ibuf->lock(HardwareBuffer::HBL_READ_ONLY)); 
                    pInt += facedata->indexStart;
                }
                else
                {
                    pShort = static_cast<unsigned short*>(
                        ibuf->lock(HardwareBuffer::HBL_READ_ONLY)); 
                    pShort += facedata->indexStart;
                }
                
                for (size_t f = 0; f < facedata->indexCount; f += 3)
                {
                    pugi::xml_node faceNode =
                        subNode.append_child("face");
                    if (use32BitIndexes)
                    {
                        faceNode.append_attribute("v1") = StringConverter::toString(*pInt++).c_str();
                        faceNode.append_attribute("v2") = StringConverter::toString(*pInt++).c_str();
                        faceNode.append_attribute("v3") = StringConverter::toString(*pInt++).c_str();
                    }
                    else
                    {
                        faceNode.append_attribute("v1") = StringConverter::toString(*pShort++).c_str();
                        faceNode.append_attribute("v2") = StringConverter::toString(*pShort++).c_str();
                        faceNode.append_attribute("v3") = StringConverter::toString(*pShort++).c_str();
                    }

                }

                ibuf->unlock();
            }

        }

    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeExtremes(pugi::xml_node& mMeshNode, const Mesh* m)
    {
        pugi::xml_node extremesNode;
        size_t submeshCount = m->getNumSubMeshes();
        for (size_t idx = 0; idx < submeshCount; ++idx)
        {
            SubMesh *sm = m->getSubMesh(idx);
            if (sm->extremityPoints.empty())
                continue; // do nothing

            if (!extremesNode)
                extremesNode = mMeshNode.append_child("extremes");

            pugi::xml_node submeshNode = extremesNode.append_child("submesh_extremes");

            submeshNode.append_attribute("index") =   StringConverter::toString(idx).c_str();

            for (std::vector<Vector3>::const_iterator v = sm->extremityPoints.begin ();
                 v != sm->extremityPoints.end (); ++v)
            {
                pugi::xml_node vert = submeshNode.append_child("position");
                vert.append_attribute("x") = StringConverter::toString(v->x).c_str();
                vert.append_attribute("y") = StringConverter::toString(v->y).c_str();
                vert.append_attribute("z") = StringConverter::toString(v->z).c_str();
            }
        }
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readLodInfo(pugi::xml_node&  lodNode)
    {
        
        LogManager::getSingleton().logMessage("Parsing LOD information...");

        const char* val = lodNode.attribute("strategy").as_string(NULL);
        // This attribute is optional to maintain backwards compatibility
        if (val)
        {
            String strategyName = val;
            LodStrategy *strategy = LodStrategyManager::getSingleton().getStrategy(strategyName);
            mMesh->setLodStrategy(strategy);
        }

        val = lodNode.attribute("numlevels").value();
        unsigned short numLevels = static_cast<unsigned short>(
            StringConverter::parseUnsignedInt(val));

        val = lodNode.attribute("manual").value();
        StringConverter::parseBool(val);

        // Set up the basic structures
        mMesh->_setLodInfo(numLevels);

        // Parse the detail, start from 1 (the first sub-level of detail)
        unsigned short i = 1;
        for (auto usageElem : lodNode.children())
        {
            if (usageElem.name() == String("lodmanual"))
            {
                readLodUsageManual(usageElem, i);
            }
            else if (usageElem.name() == String("lodgenerated"))
            {
                readLodUsageGenerated(usageElem, i);
            }
            ++i;
        }
        
        LogManager::getSingleton().logMessage("LOD information done.");
        
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readLodUsageManual(pugi::xml_node& manualNode, unsigned short index)
    {
        MeshLodUsage usage;
        const char* val = manualNode.attribute("value").as_string(NULL);

        // If value attribute not found check for old name
        if (!val)
        {
            val = manualNode.attribute("fromdepthsquared").as_string(NULL);
            if (val)
                LogManager::getSingleton().logWarning("'fromdepthsquared' attribute has been renamed to 'value'.");
            // user values are non-squared
            usage.userValue = Math::Sqrt(StringConverter::parseReal(val));
        }
        else
        {
            usage.userValue = StringConverter::parseReal(val);
        }
        usage.value = mMesh->getLodStrategy()->transformUserValue(usage.userValue);
        usage.manualName = manualNode.attribute("meshname").value();
        usage.edgeData = NULL;

        // Generate for mixed
        size_t numSubs, i;
        numSubs = mMesh->getNumSubMeshes();
        for (i = 0; i < numSubs; ++i)
        {
            SubMesh* sm = mMesh->getSubMesh(i);
            sm->mLodFaceList[index - 1] = OGRE_NEW IndexData();
        }
        mMesh->_setLodUsage(index, usage);
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readLodUsageGenerated(pugi::xml_node& genNode, unsigned short index)
    {
        MeshLodUsage usage;
        const char* val = genNode.attribute("value").as_string(NULL);

        // If value attribute not found check for old name
        if (!val)
        {
            val = genNode.attribute("fromdepthsquared").value();
            if (val)
                LogManager::getSingleton().logWarning("'fromdepthsquared' attribute has been renamed to 'value'.");
            // user values are non-squared
            usage.userValue = Math::Sqrt(StringConverter::parseReal(val));
        }
        else
        {
            usage.userValue = StringConverter::parseReal(val);
        }
        usage.value = mMesh->getLodStrategy()->transformUserValue(usage.userValue);
        usage.manualMesh.reset();
        usage.manualName = "";
        usage.edgeData = NULL;

        mMesh->_setLodUsage(index, usage);

        // Read submesh face lists

        HardwareIndexBufferSharedPtr ibuf;
        for (pugi::xml_node faceListElem : genNode.children("lodfacelist"))
        {
            val = faceListElem.attribute("submeshindex").value();
            unsigned short subidx = StringConverter::parseUnsignedInt(val);
            val = faceListElem.attribute("numfaces").value();
            unsigned short numFaces = StringConverter::parseUnsignedInt(val);
            if (numFaces)
            {
                // use of 32bit indexes depends on submesh
                HardwareIndexBuffer::IndexType itype = 
                    mMesh->getSubMesh(subidx)->indexData->indexBuffer->getType();
                bool use32bitindexes = (itype == HardwareIndexBuffer::IT_32BIT);

                // Assign memory: this will be deleted by the submesh 
                ibuf = HardwareBufferManager::getSingleton().
                    createIndexBuffer(
                        itype, numFaces * 3, HardwareBuffer::HBU_STATIC_WRITE_ONLY);

                unsigned short *pShort = 0;
                unsigned int *pInt = 0;
                if (use32bitindexes)
                {
                    pInt = static_cast<unsigned int*>(
                        ibuf->lock(HardwareBuffer::HBL_DISCARD));
                }
                else
                {
                    pShort = static_cast<unsigned short*>(
                        ibuf->lock(HardwareBuffer::HBL_DISCARD));
                }
                pugi::xml_node faceElem = faceListElem.child("face");
                for (unsigned int face = 0; face < numFaces; ++face, faceElem = faceElem.next_sibling())
                {
                    if (use32bitindexes)
                    {
                        val = faceElem.attribute("v1").value();
                        *pInt++ = StringConverter::parseUnsignedInt(val);
                        val = faceElem.attribute("v2").value();
                        *pInt++ = StringConverter::parseUnsignedInt(val);
                        val = faceElem.attribute("v3").value();
                        *pInt++ = StringConverter::parseUnsignedInt(val);
                    }
                    else
                    {
                        val = faceElem.attribute("v1").value();
                        *pShort++ = StringConverter::parseUnsignedInt(val);
                        val = faceElem.attribute("v2").value();
                        *pShort++ = StringConverter::parseUnsignedInt(val);
                        val = faceElem.attribute("v3").value();
                        *pShort++ = StringConverter::parseUnsignedInt(val);
                    }

                }

                ibuf->unlock();
            }
            IndexData* facedata = new IndexData(); // will be deleted by SubMesh
            facedata->indexCount = numFaces * 3;
            facedata->indexStart = 0;
            facedata->indexBuffer = ibuf;
            mMesh->_setSubMeshLodFaceList(subidx, index, facedata);
        }
        
    }
    //-----------------------------------------------------------------------------
    void XMLMeshSerializer::readExtremes(pugi::xml_node& extremesNode, Mesh *m)
    {
        LogManager::getSingleton().logMessage("Reading extremes...");

        // Iterate over all children (submesh_extreme list)
        for (pugi::xml_node& elem : extremesNode.children())
        {
            int index = StringConverter::parseInt(elem.attribute("index").value());

            SubMesh *sm = m->getSubMesh(index);
            sm->extremityPoints.clear ();
            for (pugi::xml_node& vert : elem.children())
            {
                Vector3 v;
                v.x = StringConverter::parseReal(vert.attribute("x").value());
                v.y = StringConverter::parseReal(vert.attribute("y").value());
                v.z = StringConverter::parseReal(vert.attribute("z").value());
                sm->extremityPoints.push_back (v);
            }
        }

        LogManager::getSingleton().logMessage("Extremes done.");
    }
    //-----------------------------------------------------------------------------
    void XMLMeshSerializer::readPoses(pugi::xml_node& posesNode, Mesh *m)
    {


        for (pugi::xml_node poseNode : posesNode.children("pose"))
        {
            const char* target = poseNode.attribute("target").as_string(NULL);
            if (!target)
            {
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                    "Required attribute 'target' missing on pose", 
                    "XMLMeshSerializer::readPoses");
            }
            unsigned short targetID;
            if(String(target) == "mesh")
            {
                targetID = 0;
            }
            else
            {
                // submesh, get index
                const char* val = poseNode.attribute("index").as_string(NULL);
                if (!val)
                {
                    OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                        "Required attribute 'index' missing on pose", 
                        "XMLMeshSerializer::readPoses");
                }
                unsigned short submeshIndex = static_cast<unsigned short>(
                    StringConverter::parseUnsignedInt(val));

                targetID = submeshIndex + 1;
            }

            String name;
            const char* val = poseNode.attribute("name").as_string(NULL);
            if (val)
                name = val;
            Pose* pose = m->createPose(targetID, name);

            for (pugi::xml_node poseOffsetNode : poseNode.children("poseoffset"))
            {
                uint index = StringConverter::parseUnsignedInt(poseOffsetNode.attribute("index").value());
                Vector3 offset;
                offset.x = StringConverter::parseReal(poseOffsetNode.attribute("x").value());
                offset.y = StringConverter::parseReal(poseOffsetNode.attribute("y").value());
                offset.z = StringConverter::parseReal(poseOffsetNode.attribute("z").value());

                if (poseOffsetNode.attribute("nx").value() &&
                    poseOffsetNode.attribute("ny").value() &&
                    poseOffsetNode.attribute("nz").value())
                {
                    Vector3 normal;
                    normal.x = StringConverter::parseReal(poseOffsetNode.attribute("nx").value());
                    normal.y = StringConverter::parseReal(poseOffsetNode.attribute("ny").value());
                    normal.z = StringConverter::parseReal(poseOffsetNode.attribute("nz").value());
                    pose->addVertex(index, offset, normal);
                    
                }
                else 
                {
                    pose->addVertex(index, offset);
                }
            }
        }


    }
    //-----------------------------------------------------------------------------
    void XMLMeshSerializer::readAnimations(pugi::xml_node& mAnimationsNode, Mesh *pMesh)
    {
        for (pugi::xml_node animElem : mAnimationsNode.children("animation"))
        {
            String name = animElem.attribute("name").value();
            Real len = StringConverter::parseReal(animElem.attribute("length").value());

            Animation* anim = pMesh->createAnimation(name, len);

            pugi::xml_node baseInfoNode = animElem.child("baseinfo");
            if (baseInfoNode)
            {
                String baseName = baseInfoNode.attribute("baseanimationname").value();
                Real baseTime = StringConverter::parseReal(baseInfoNode.attribute("basekeyframetime").value());
                anim->setUseBaseKeyFrame(true, baseTime, baseName);
            }
            
            pugi::xml_node tracksNode = animElem.child("tracks");
            if (tracksNode)
            {
                readTracks(tracksNode, pMesh, anim);
            }
        }


    }
    //-----------------------------------------------------------------------------
    void XMLMeshSerializer::readTracks(pugi::xml_node& tracksNode, Mesh *m, Animation* anim)
    {
        for (pugi::xml_node trackNode : tracksNode.children("track"))
        {
            String target = trackNode.attribute("target").value();
            unsigned short targetID;
            VertexData* vertexData = 0;
            if(target == "mesh")
            {
                targetID = 0;
                vertexData = m->sharedVertexData;
            }
            else
            {
                // submesh, get index
                const char* val = trackNode.attribute("index").as_string(NULL);
                if (!val)
                {
                    OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                        "Required attribute 'index' missing on submesh track", 
                        "XMLMeshSerializer::readTracks");
                }
                unsigned short submeshIndex = static_cast<unsigned short>(
                    StringConverter::parseUnsignedInt(val));

                targetID = submeshIndex + 1;
                vertexData = m->getSubMesh(submeshIndex)->vertexData;

            }

            if (!vertexData)
            {
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                    "Track cannot be created for " + target + " since VertexData "
                    "does not exist at the specified index", 
                    "XMLMeshSerializer::readTracks");
            }

            // Get type
            VertexAnimationType animType = VAT_NONE;
            String strAnimType = trackNode.attribute("type").value();
            if (strAnimType == "morph")
            {
                animType = VAT_MORPH;
            }
            else if (strAnimType == "pose")
            {
                animType = VAT_POSE;
            }
            else
            {
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                    "Unrecognised animation track type '" + strAnimType + "'",
                    "XMLMeshSerializer::readTracks");
            }

            // Create track
            VertexAnimationTrack* track = 
                anim->createVertexTrack(targetID, vertexData, animType);

            pugi::xml_node keyframesNode = trackNode.child("keyframes");
            if (keyframesNode)
            {
                if (track->getAnimationType() == VAT_MORPH)
                {
                    readMorphKeyFrames(keyframesNode, track, vertexData->vertexCount);
                }
                else // VAT_POSE
                {
                    readPoseKeyFrames(keyframesNode, track);
                }
            }
        }
    }
    //-----------------------------------------------------------------------------
    void XMLMeshSerializer::readMorphKeyFrames(pugi::xml_node& keyframesNode,
        VertexAnimationTrack* track, size_t vertexCount)
    {
        for (pugi::xml_node keyNode : keyframesNode.children("keyframe"))
        {
            const char* val = keyNode.attribute("time").as_string(NULL);
            if (!val)
            {
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                    "Required attribute 'time' missing on keyframe", 
                    "XMLMeshSerializer::readKeyFrames");
            }
            Real time = StringConverter::parseReal(val);

            VertexMorphKeyFrame* kf = track->createVertexMorphKeyFrame(time);
            
            bool includesNormals = keyNode.child("normal");

            size_t vertexSize = sizeof(float) * (includesNormals ? 6 : 3);
            // create a vertex buffer
            HardwareVertexBufferSharedPtr vbuf = 
                HardwareBufferManager::getSingleton().createVertexBuffer(
                vertexSize, vertexCount, 
                HardwareBuffer::HBU_STATIC, true);

            float* pFloat = static_cast<float*>(
                vbuf->lock(HardwareBuffer::HBL_DISCARD));


            pugi::xml_node posNode = keyNode.child("position");
            pugi::xml_node normNode = keyNode.child("normal");
            for (size_t v = 0; v < vertexCount; ++v)
            {
                if (!posNode)
                {
                    OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                        "Not enough 'position' elements under keyframe", 
                        "XMLMeshSerializer::readKeyFrames");

                }

                *pFloat++ = StringConverter::parseReal(posNode.attribute("x").value());
                *pFloat++ = StringConverter::parseReal(posNode.attribute("y").value());
                *pFloat++ = StringConverter::parseReal(posNode.attribute("z").value());
                    
                if (includesNormals)
                {
                    if (!normNode)
                    {
                        OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                            "Not enough 'normal' elements under keyframe", 
                            "XMLMeshSerializer::readKeyFrames");

                    }
                    
                    *pFloat++ = StringConverter::parseReal(normNode.attribute("x").value());
                    *pFloat++ = StringConverter::parseReal(normNode.attribute("y").value());
                    *pFloat++ = StringConverter::parseReal(normNode.attribute("z").value());
                    normNode = normNode.next_sibling("normal");
                }


                posNode = posNode.next_sibling("position");
            }

            vbuf->unlock();

            kf->setVertexBuffer(vbuf);
        }


    }
    //-----------------------------------------------------------------------------
    void XMLMeshSerializer::readPoseKeyFrames(pugi::xml_node& keyframesNode, VertexAnimationTrack* track)
    {
        for (pugi::xml_node keyNode : keyframesNode.children("keyframe"))
        {
            const char* val = keyNode.attribute("time").as_string(NULL);
            if (!val)
            {
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                    "Required attribute 'time' missing on keyframe", 
                    "XMLMeshSerializer::readKeyFrames");
            }
            Real time = StringConverter::parseReal(val);

            VertexPoseKeyFrame* kf = track->createVertexPoseKeyFrame(time);

            // Read all pose references
            for (pugi::xml_node poseRefNode : keyNode.children("poseref"))
            {
                const char* attr = poseRefNode.attribute("poseindex").as_string(NULL);
                if (!attr)
                {
                    OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                        "Required attribute 'poseindex' missing on poseref", 
                        "XMLMeshSerializer::readPoseKeyFrames");
                }
                unsigned short poseIndex = StringConverter::parseUnsignedInt(attr);
                Real influence = 1.0f;
                attr = poseRefNode.attribute("influence").as_string(NULL);
                if (attr)
                {
                    influence = StringConverter::parseReal(attr);
                }

                kf->addPoseReference(poseIndex, influence);
            }
        }

    }
    //-----------------------------------------------------------------------------
    void XMLMeshSerializer::writePoses(pugi::xml_node& meshNode, const Mesh* m)
    {
        if (m->getPoseList().empty())
            return;

        pugi::xml_node posesNode = meshNode.append_child("poses");

        PoseList::const_iterator it;
        for( it = m->getPoseList().begin(); it != m->getPoseList().end(); ++it)
        {
            const Pose* pose = *it;
            pugi::xml_node poseNode = posesNode.append_child("pose");
            unsigned short target = pose->getTarget();
            if (target == 0)
            {
                // Main mesh
                poseNode.append_attribute("target") = "mesh";
            }
            else
            {
                // Submesh - rebase index
                poseNode.append_attribute("target") = "submesh";
                poseNode.append_attribute("index") =
                    StringConverter::toString(target - 1).c_str();
            }
            poseNode.append_attribute("name") = pose->getName().c_str();
            
            bool includesNormals = !pose->getNormals().empty();
            auto nit = pose->getNormals().begin();
            for (const auto& vit : pose->getVertexOffsets())
            {
                pugi::xml_node poseOffsetElement = poseNode.append_child("poseoffset");

                poseOffsetElement.append_attribute("index") =
                    StringConverter::toString(vit.first).c_str();

                const Vector3& offset = vit.second;
                poseOffsetElement.append_attribute("x") = StringConverter::toString(offset.x).c_str();
                poseOffsetElement.append_attribute("y") = StringConverter::toString(offset.y).c_str();
                poseOffsetElement.append_attribute("z") = StringConverter::toString(offset.z).c_str();
                
                if (includesNormals)
                {
                    const Vector3& normal = nit->second;
                    poseOffsetElement.append_attribute("nx") = StringConverter::toString(normal.x).c_str();
                    poseOffsetElement.append_attribute("ny") = StringConverter::toString(normal.y).c_str();
                    poseOffsetElement.append_attribute("nz") = StringConverter::toString(normal.z).c_str();
                    nit++;
                }
                
                


            }

        }

    }
    //-----------------------------------------------------------------------------
    void XMLMeshSerializer::writeAnimations(pugi::xml_node& meshNode, const Mesh* m)
    {
        // Skip if no animation
        if (!m->hasVertexAnimation())
            return;

        pugi::xml_node animationsNode = meshNode.append_child("animations");

        for (unsigned short a = 0; a < m->getNumAnimations(); ++a)
        {
            Animation* anim = m->getAnimation(a);

            pugi::xml_node animNode =
                animationsNode.append_child("animation");
            animNode.append_attribute("name") = anim->getName().c_str();
            animNode.append_attribute("length") =
                StringConverter::toString(anim->getLength()).c_str();

            // Optional base keyframe information
            if (anim->getUseBaseKeyFrame())
            {
                pugi::xml_node baseInfoNode =
                animNode.append_child("baseinfo");
                baseInfoNode.append_attribute("baseanimationname") = anim->getBaseKeyFrameAnimationName().c_str();
                baseInfoNode.append_attribute("basekeyframetime") = StringConverter::toString(anim->getBaseKeyFrameTime()).c_str();
            }
            
            pugi::xml_node tracksNode = animNode.append_child("tracks");
            for (const auto& trackIt : anim->_getVertexTrackList())
            {
                const VertexAnimationTrack* track = trackIt.second;
                pugi::xml_node trackNode = tracksNode.append_child("track");

                unsigned short targetID = trackIt.first;
                if (targetID == 0)
                {
                    trackNode.append_attribute("target") = "mesh";
                }
                else
                {
                    trackNode.append_attribute("target") = "submesh";
                    trackNode.append_attribute("index") =
                        StringConverter::toString(targetID-1).c_str();
                }

                if (track->getAnimationType() == VAT_MORPH)
                {
                    trackNode.append_attribute("type") = "morph";
                    writeMorphKeyFrames(trackNode, track);
                }
                else
                {
                    trackNode.append_attribute("type") = "pose";
                    writePoseKeyFrames(trackNode, track);
                }


            }
        }

        
    }
    //-----------------------------------------------------------------------------
    void XMLMeshSerializer::writeMorphKeyFrames(pugi::xml_node& trackNode, const VertexAnimationTrack* track)
    {
        pugi::xml_node keyframesNode = trackNode.append_child("keyframes");

        size_t vertexCount = track->getAssociatedVertexData()->vertexCount;

        for (unsigned short k = 0; k < track->getNumKeyFrames(); ++k)
        {
            VertexMorphKeyFrame* kf = track->getVertexMorphKeyFrame(k);
            pugi::xml_node keyNode = keyframesNode.append_child("keyframe");
            keyNode.append_attribute("time") =
                StringConverter::toString(kf->getTime()).c_str();

            HardwareVertexBufferSharedPtr vbuf = kf->getVertexBuffer();
            
            bool includesNormals = vbuf->getVertexSize() > (sizeof(float) * 3);
            
            float* pFloat = static_cast<float*>(
                vbuf->lock(HardwareBuffer::HBL_READ_ONLY));

            for (size_t v = 0; v < vertexCount; ++v)
            {
                pugi::xml_node posNode = keyNode.append_child("position");
                posNode.append_attribute("x") = StringConverter::toString(*pFloat++).c_str();
                posNode.append_attribute("y") = StringConverter::toString(*pFloat++).c_str();
                posNode.append_attribute("z") = StringConverter::toString(*pFloat++).c_str();
                
                if (includesNormals)
                {
                    pugi::xml_node normNode = keyNode.append_child("normal");
                    normNode.append_attribute("x") = StringConverter::toString(*pFloat++).c_str();
                    normNode.append_attribute("y") = StringConverter::toString(*pFloat++).c_str();
                    normNode.append_attribute("z") = StringConverter::toString(*pFloat++).c_str();
                }
            }

        }
    }
    //-----------------------------------------------------------------------------
    void XMLMeshSerializer::writePoseKeyFrames(pugi::xml_node& trackNode, const VertexAnimationTrack* track)
    {
        pugi::xml_node keyframesNode = trackNode.append_child("keyframes");

        for (unsigned short k = 0; k < track->getNumKeyFrames(); ++k)
        {
            VertexPoseKeyFrame* kf = track->getVertexPoseKeyFrame(k);
            pugi::xml_node keyNode = keyframesNode.append_child("keyframe");
            keyNode.append_attribute("time") =
                StringConverter::toString(kf->getTime()).c_str();

            VertexPoseKeyFrame::PoseRefList::const_iterator poseIt =
                kf->getPoseReferences().begin();
            for (;poseIt != kf->getPoseReferences().end(); ++poseIt)
            {
                const VertexPoseKeyFrame::PoseRef& poseRef = *poseIt;
                pugi::xml_node poseRefNode = keyNode.append_child("poseref");

                poseRefNode.append_attribute("poseindex") =
                    StringConverter::toString(poseRef.poseIndex).c_str();
                poseRefNode.append_attribute("influence") =
                    StringConverter::toString(poseRef.influence).c_str();

            }

        }


    }




}

