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
        mpMesh = pMesh;
		mColourElementType = colourElementType;
        mXMLDoc = new TiXmlDocument(filename);
        mXMLDoc->LoadFile();

        TiXmlElement* elem;

        TiXmlElement* rootElem = mXMLDoc->RootElement();

        // shared geometry
        elem = rootElem->FirstChildElement("sharedgeometry");
        if (elem)
        {
            const char *claimedVertexCount_ = elem->Attribute("vertexcount");
            if(!claimedVertexCount_ || StringConverter::parseInt(claimedVertexCount_) > 0)
            {
                mpMesh->sharedVertexData = new VertexData();
                readGeometry(elem, mpMesh->sharedVertexData);
            }
        }

        // submeshes
        elem = rootElem->FirstChildElement("submeshes");
        if (elem)
            readSubMeshes(elem);

        // skeleton link
        elem = rootElem->FirstChildElement("skeletonlink");
        if (elem)
            readSkeletonLink(elem);

        // bone assignments
        elem = rootElem->FirstChildElement("boneassignments");
        if (elem)
            readBoneAssignments(elem);

		//Lod
		elem = rootElem->FirstChildElement("levelofdetail");
		if (elem)
			readLodInfo(elem);

		// submesh names
		elem = rootElem->FirstChildElement("submeshnames");
		if (elem)
			readSubMeshNames(elem, mpMesh);

		// submesh extremes
		elem = rootElem->FirstChildElement("extremes");
		if (elem)
			readExtremes(elem, mpMesh);

		// poses
		elem = rootElem->FirstChildElement("poses");
		if (elem)
			readPoses(elem, mpMesh);

		// animations
		elem = rootElem->FirstChildElement("animations");
		if (elem)
			readAnimations(elem, mpMesh);

		delete mXMLDoc;

        LogManager::getSingleton().logMessage("XMLMeshSerializer import successful.");
        
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::exportMesh(const Mesh* pMesh, const String& filename)
    {
        LogManager::getSingleton().logMessage("XMLMeshSerializer writing mesh data to " + filename + "...");
        
        mpMesh = const_cast<Mesh*>(pMesh);

        mXMLDoc = new TiXmlDocument();
        mXMLDoc->InsertEndChild(TiXmlElement("mesh"));

        LogManager::getSingleton().logMessage("Populating DOM...");

            
           
        // Write to DOM
        writeMesh(pMesh);
        LogManager::getSingleton().logMessage("DOM populated, writing XML file..");

        // Write out to a file
        if(! mXMLDoc->SaveFile(filename) )
        {
            LogManager::getSingleton().logMessage("XMLMeshSerializer failed writing the XML file.", LML_CRITICAL);
        }
        else
        {
            LogManager::getSingleton().logMessage("XMLMeshSerializer export successful.");
        }

    
        delete mXMLDoc;


    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeMesh(const Mesh* pMesh)
    {
        TiXmlElement* rootNode = mXMLDoc->RootElement();
        // Write geometry
		if (pMesh->sharedVertexData)
		{
			TiXmlElement* geomNode = 
				rootNode->InsertEndChild(TiXmlElement("sharedgeometry"))->ToElement();
			writeGeometry(geomNode, pMesh->sharedVertexData);
		}

        // Write Submeshes
        TiXmlElement* subMeshesNode = 
            rootNode->InsertEndChild(TiXmlElement("submeshes"))->ToElement();
        for (int i = 0; i < pMesh->getNumSubMeshes(); ++i)
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
            Mesh::BoneAssignmentIterator bi = const_cast<Mesh*>(pMesh)->getBoneAssignmentIterator();
            if (bi.hasMoreElements())
            {
                LogManager::getSingleton().logMessage("Exporting shared geometry bone assignments...");
                TiXmlElement* boneAssignNode = 
                    rootNode->InsertEndChild(TiXmlElement("boneassignments"))->ToElement();

                while (bi.hasMoreElements())
                {
					const VertexBoneAssignment& assign = bi.getNext();
                    writeBoneAssignment(boneAssignNode, &assign);
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
    void XMLMeshSerializer::writeSubMesh(TiXmlElement* mSubMeshesNode, const SubMesh* s)
    {
        TiXmlElement* subMeshNode = 
            mSubMeshesNode->InsertEndChild(TiXmlElement("submesh"))->ToElement();

        size_t numFaces;

        // Material name
        subMeshNode->SetAttribute("material", s->getMaterialName());
        // bool useSharedVertices
        subMeshNode->SetAttribute("usesharedvertices", 
            StringConverter::toString(s->useSharedVertices) );
        // bool use32BitIndexes
		bool use32BitIndexes = (!s->indexData->indexBuffer.isNull() && 
			s->indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT);
        subMeshNode->SetAttribute("use32bitindexes", 
            StringConverter::toString( use32BitIndexes ));

        // Operation type
        switch(s->operationType)
        {
        case RenderOperation::OT_LINE_LIST:
            subMeshNode->SetAttribute("operationtype", "line_list");
            break;
        case RenderOperation::OT_LINE_STRIP:
            subMeshNode->SetAttribute("operationtype", "line_strip");
            break;
        case RenderOperation::OT_POINT_LIST:
            subMeshNode->SetAttribute("operationtype", "point_list");
            break;
        case RenderOperation::OT_TRIANGLE_FAN:
            subMeshNode->SetAttribute("operationtype", "triangle_fan");
            break;
        case RenderOperation::OT_TRIANGLE_LIST:
            subMeshNode->SetAttribute("operationtype", "triangle_list");
            break;
        case RenderOperation::OT_TRIANGLE_STRIP:
            subMeshNode->SetAttribute("operationtype", "triangle_strip");
            break;
        }

        if (s->indexData->indexCount > 0)
        {
            // Faces
            TiXmlElement* facesNode = 
                subMeshNode->InsertEndChild(TiXmlElement("faces"))->ToElement();
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
				OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
					"Unsupported render operation type", 
					__FUNCTION__);
			}
            facesNode->SetAttribute("count", 
                StringConverter::toString(numFaces));
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
                TiXmlElement* faceNode = 
                    facesNode->InsertEndChild(TiXmlElement("face"))->ToElement();
			    if (use32BitIndexes)
			    {
				    faceNode->SetAttribute("v1", StringConverter::toString(*pInt++));
					if(s->operationType == RenderOperation::OT_LINE_LIST)
					{
				        faceNode->SetAttribute("v2", StringConverter::toString(*pInt++));
					}
                    /// Only need all 3 vertex indices if trilist or first face
					else if (s->operationType == RenderOperation::OT_TRIANGLE_LIST || i == 0)
                    {
				        faceNode->SetAttribute("v2", StringConverter::toString(*pInt++));
				        faceNode->SetAttribute("v3", StringConverter::toString(*pInt++));
                    }
			    }
			    else
			    {
				    faceNode->SetAttribute("v1", StringConverter::toString(*pShort++));
 					if(s->operationType == RenderOperation::OT_LINE_LIST)
					{
				        faceNode->SetAttribute("v2", StringConverter::toString(*pShort++));
					}
                    /// Only need all 3 vertex indices if trilist or first face
                    else if (s->operationType == RenderOperation::OT_TRIANGLE_LIST || i == 0)
                    {
				        faceNode->SetAttribute("v2", StringConverter::toString(*pShort++));
				        faceNode->SetAttribute("v3", StringConverter::toString(*pShort++));
                    }
			    }
            }
        }

        // M_GEOMETRY chunk (Optional: present only if useSharedVertices = false)
        if (!s->useSharedVertices)
        {
            TiXmlElement* geomNode = 
                subMeshNode->InsertEndChild(TiXmlElement("geometry"))->ToElement();
            writeGeometry(geomNode, s->vertexData);
        }

        // texture aliases
        writeTextureAliases(subMeshNode, s);

        // Bone assignments
        if (mpMesh->hasSkeleton())
        {
            SubMesh::BoneAssignmentIterator bi = const_cast<SubMesh*>(s)->getBoneAssignmentIterator();
            LogManager::getSingleton().logMessage("Exporting dedicated geometry bone assignments...");

            TiXmlElement* boneAssignNode = 
                subMeshNode->InsertEndChild(TiXmlElement("boneassignments"))->ToElement();
            while (bi.hasMoreElements())
            {
				const VertexBoneAssignment& assign = bi.getNext();
                writeBoneAssignment(boneAssignNode, &assign);
            }
        }
        LogManager::getSingleton().logMessage("Dedicated geometry bone assignments exported.");

    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeGeometry(TiXmlElement* mParentNode, const VertexData* vertexData)
    {
        // Write a vertex buffer per element

        TiXmlElement *vbNode, *vertexNode, *dataNode;

        // Set num verts on parent
        mParentNode->SetAttribute("vertexcount", StringConverter::toString(vertexData->vertexCount));

		VertexDeclaration* decl = vertexData->vertexDeclaration;
		VertexBufferBinding* bind = vertexData->vertexBufferBinding;

		VertexBufferBinding::VertexBufferBindingMap::const_iterator b, bend;
		bend = bind->getBindings().end();
		// Iterate over buffers
		for(b = bind->getBindings().begin(); b != bend; ++b)
		{
			vbNode = mParentNode->InsertEndChild(TiXmlElement("vertexbuffer"))->ToElement();
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
			ARGB* pColour;

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
					vbNode->SetAttribute("positions","true");
                    break;
				case VES_NORMAL:
					vbNode->SetAttribute("normals","true");
                    break;
				case VES_TANGENT:
					vbNode->SetAttribute("tangents","true");
					if (elem.getType() == VET_FLOAT4)
					{
						vbNode->SetAttribute("tangent_dimensions", "4");
					}
					break;
				case VES_BINORMAL:
					vbNode->SetAttribute("binormals","true");
					break;
				case VES_DIFFUSE:
					vbNode->SetAttribute("colours_diffuse","true");
                    break;
				case VES_SPECULAR:
					vbNode->SetAttribute("colours_specular","true");
                    break;
                case VES_TEXTURE_COORDINATES:
					{
						const char *type;
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
						case VET_COLOUR: 
						case VET_COLOUR_ARGB: 
						case VET_COLOUR_ABGR: 
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
						}
						vbNode->SetAttribute(
							"texture_coord_dimensions_" + StringConverter::toString(numTextureCoords), type);
						++numTextureCoords;
					} 
					break;

                default:
                    break;
                }
            }
            if (numTextureCoords > 0)
            {
                vbNode->SetAttribute("texture_coords", 
                    StringConverter::toString(numTextureCoords));
            }

			// For each vertex
			for (size_t v = 0; v < vertexData->vertexCount; ++v)
			{
                vertexNode = 
                    vbNode->InsertEndChild(TiXmlElement("vertex"))->ToElement();
				// Iterate over the elements
				for (i = elems.begin(); i != iend; ++i)
				{
					VertexElement& elem = *i;
					switch(elem.getSemantic())
					{
					case VES_POSITION:
						elem.baseVertexPointerToElement(pVert, &pFloat);
						dataNode = 
							vertexNode->InsertEndChild(TiXmlElement("position"))->ToElement();
						dataNode->SetAttribute("x", StringConverter::toString(pFloat[0]));
						dataNode->SetAttribute("y", StringConverter::toString(pFloat[1]));
						dataNode->SetAttribute("z", StringConverter::toString(pFloat[2]));
						break;
					case VES_NORMAL:
						elem.baseVertexPointerToElement(pVert, &pFloat);
						dataNode = 
							vertexNode->InsertEndChild(TiXmlElement("normal"))->ToElement();
						dataNode->SetAttribute("x", StringConverter::toString(pFloat[0]));
						dataNode->SetAttribute("y", StringConverter::toString(pFloat[1]));
						dataNode->SetAttribute("z", StringConverter::toString(pFloat[2]));
						break;
					case VES_TANGENT:
						elem.baseVertexPointerToElement(pVert, &pFloat);
						dataNode = 
							vertexNode->InsertEndChild(TiXmlElement("tangent"))->ToElement();
						dataNode->SetAttribute("x", StringConverter::toString(pFloat[0]));
						dataNode->SetAttribute("y", StringConverter::toString(pFloat[1]));
						dataNode->SetAttribute("z", StringConverter::toString(pFloat[2]));
						if (elem.getType() == VET_FLOAT4)
						{
							dataNode->SetAttribute("w", StringConverter::toString(pFloat[3]));
						}
						break;
					case VES_BINORMAL:
						elem.baseVertexPointerToElement(pVert, &pFloat);
						dataNode = 
							vertexNode->InsertEndChild(TiXmlElement("binormal"))->ToElement();
						dataNode->SetAttribute("x", StringConverter::toString(pFloat[0]));
						dataNode->SetAttribute("y", StringConverter::toString(pFloat[1]));
						dataNode->SetAttribute("z", StringConverter::toString(pFloat[2]));
						break;
					case VES_DIFFUSE:
						elem.baseVertexPointerToElement(pVert, &pColour);
						dataNode = 
							vertexNode->InsertEndChild(TiXmlElement("colour_diffuse"))->ToElement();
						{
							ARGB rc = *pColour++;
							ColourValue cv;
							cv.b = (rc & 0xFF) / 255.0f;		rc >>= 8;
							cv.g = (rc & 0xFF) / 255.0f;		rc >>= 8;
							cv.r = (rc & 0xFF) / 255.0f;		rc >>= 8;
							cv.a = (rc & 0xFF) / 255.0f;
                            dataNode->SetAttribute("value", StringConverter::toString(cv));
						}
						break;
					case VES_SPECULAR:
						elem.baseVertexPointerToElement(pVert, &pColour);
						dataNode = 
							vertexNode->InsertEndChild(TiXmlElement("colour_specular"))->ToElement();
						{
							ARGB rc = *pColour++;
							ColourValue cv;
							cv.b = (rc & 0xFF) / 255.0f;		rc >>= 8;
							cv.g = (rc & 0xFF) / 255.0f;		rc >>= 8;
							cv.r = (rc & 0xFF) / 255.0f;		rc >>= 8;
							cv.a = (rc & 0xFF) / 255.0f;
							dataNode->SetAttribute("value", StringConverter::toString(cv));
						}
						break;
					case VES_TEXTURE_COORDINATES:
						dataNode = 
							vertexNode->InsertEndChild(TiXmlElement("texcoord"))->ToElement();

						switch(elem.getType())
                        {
                        case VET_FLOAT1:
                            elem.baseVertexPointerToElement(pVert, &pFloat);
    						dataNode->SetAttribute("u", StringConverter::toString(*pFloat++));
                            break;
                        case VET_FLOAT2:
                            elem.baseVertexPointerToElement(pVert, &pFloat);
    						dataNode->SetAttribute("u", StringConverter::toString(*pFloat++));
    						dataNode->SetAttribute("v", StringConverter::toString(*pFloat++));
                            break;
                        case VET_FLOAT3:
                            elem.baseVertexPointerToElement(pVert, &pFloat);
    						dataNode->SetAttribute("u", StringConverter::toString(*pFloat++));
    						dataNode->SetAttribute("v", StringConverter::toString(*pFloat++));
    						dataNode->SetAttribute("w", StringConverter::toString(*pFloat++));
                            break;
                        case VET_FLOAT4:
                            elem.baseVertexPointerToElement(pVert, &pFloat);
    						dataNode->SetAttribute("u", StringConverter::toString(*pFloat++));
    						dataNode->SetAttribute("v", StringConverter::toString(*pFloat++));
    						dataNode->SetAttribute("w", StringConverter::toString(*pFloat++));
    						dataNode->SetAttribute("x", StringConverter::toString(*pFloat++));
                            break;
                        case VET_SHORT1:
                            elem.baseVertexPointerToElement(pVert, &pShort);
    						dataNode->SetAttribute("u", StringConverter::toString(*pShort++ / 65535.0f));
                            break;
                        case VET_SHORT2:
                            elem.baseVertexPointerToElement(pVert, &pShort);
    						dataNode->SetAttribute("u", StringConverter::toString(*pShort++ / 65535.0f));
    						dataNode->SetAttribute("v", StringConverter::toString(*pShort++ / 65535.0f));
                            break;
                        case VET_SHORT3:
                            elem.baseVertexPointerToElement(pVert, &pShort);
    						dataNode->SetAttribute("u", StringConverter::toString(*pShort++ / 65535.0f));
    						dataNode->SetAttribute("v", StringConverter::toString(*pShort++ / 65535.0f));
    						dataNode->SetAttribute("w", StringConverter::toString(*pShort++ / 65535.0f));
                            break;
                        case VET_SHORT4:
                            elem.baseVertexPointerToElement(pVert, &pShort);
    						dataNode->SetAttribute("u", StringConverter::toString(*pShort++ / 65535.0f));
    						dataNode->SetAttribute("v", StringConverter::toString(*pShort++ / 65535.0f));
    						dataNode->SetAttribute("w", StringConverter::toString(*pShort++ / 65535.0f));
    						dataNode->SetAttribute("x", StringConverter::toString(*pShort++ / 65535.0f));
                            break;
                        case VET_COLOUR: case VET_COLOUR_ARGB: case VET_COLOUR_ABGR:
                            elem.baseVertexPointerToElement(pVert, &pColour);
                            {
                                ARGB rc = *pColour++;
                                ColourValue cv;
                                cv.b = (rc & 0xFF) / 255.0f;		rc >>= 8;
                                cv.g = (rc & 0xFF) / 255.0f;		rc >>= 8;
                                cv.r = (rc & 0xFF) / 255.0f;		rc >>= 8;
                                cv.a = (rc & 0xFF) / 255.0f;
                                dataNode->SetAttribute("u", StringConverter::toString(cv));
                            }
                            break;
                        case VET_UBYTE4:
                            elem.baseVertexPointerToElement(pVert, &pChar);
    						dataNode->SetAttribute("u", StringConverter::toString(*pChar++ / 255.0f));
    						dataNode->SetAttribute("v", StringConverter::toString(*pChar++ / 255.0f));
    						dataNode->SetAttribute("w", StringConverter::toString(*pChar++ / 255.0f));
    						dataNode->SetAttribute("x", StringConverter::toString(*pChar++ / 255.0f));
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
    void XMLMeshSerializer::writeSkeletonLink(TiXmlElement* mMeshNode, const String& skelName)
    {

        TiXmlElement* skelNode = 
            mMeshNode->InsertEndChild(TiXmlElement("skeletonlink"))->ToElement();
        skelNode->SetAttribute("name", skelName);
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeBoneAssignment(TiXmlElement* mBoneAssignNode, const VertexBoneAssignment* assign)
    {
        TiXmlElement* assignNode = 
            mBoneAssignNode->InsertEndChild(
            TiXmlElement("vertexboneassignment"))->ToElement();

        assignNode->SetAttribute("vertexindex", 
            StringConverter::toString(assign->vertexIndex));
        assignNode->SetAttribute("boneindex", 
            StringConverter::toString(assign->boneIndex));
        assignNode->SetAttribute("weight",
            StringConverter::toString(assign->weight));


    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeTextureAliases(TiXmlElement* mSubmeshesNode, const SubMesh* subMesh)
    {
        if (!subMesh->hasTextureAliases())
            return; // do nothing

        TiXmlElement* textureAliasesNode = 
            mSubmeshesNode->InsertEndChild(TiXmlElement("textures"))->ToElement();

        // use ogre map iterator
        SubMesh::AliasTextureIterator aliasIterator = subMesh->getAliasTextureIterator();

        while (aliasIterator.hasMoreElements())
        {
            TiXmlElement* aliasTextureNode = 
                textureAliasesNode->InsertEndChild(TiXmlElement("texture"))->ToElement();
            // iterator key is alias and value is texture name
            aliasTextureNode->SetAttribute("alias", aliasIterator.peekNextKey());
            aliasTextureNode->SetAttribute("name", aliasIterator.peekNextValue());
            aliasIterator.moveNext();
        }

    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readSubMeshes(TiXmlElement* mSubmeshesNode)
    {
        LogManager::getSingleton().logMessage("Reading submeshes...");

        for (TiXmlElement* smElem = mSubmeshesNode->FirstChildElement();
            smElem != 0; smElem = smElem->NextSiblingElement())
        {
            // All children should be submeshes 
            SubMesh* sm = mpMesh->createSubMesh();

            const char* mat = smElem->Attribute("material");
            if (mat)
                sm->setMaterialName(mat);

            // Read operation type
            bool readFaces = true;
            const char* optype = smElem->Attribute("operationtype");
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
                    //readFaces = false;
                }
                else if (!strcmp(optype, "point_list"))
                {
                    sm->operationType = RenderOperation::OT_POINT_LIST;
                    readFaces = false;
                }

            }

            const char* tmp = smElem->Attribute("usesharedvertices");
            if (tmp)
                sm->useSharedVertices = StringConverter::parseBool(tmp);
            tmp = smElem->Attribute("use32bitindexes");
            bool use32BitIndexes = false;
            if (tmp)
                use32BitIndexes = StringConverter::parseBool(tmp);
            
            // Faces
            if (readFaces)
            {
                TiXmlElement* faces = smElem->FirstChildElement("faces");
                int actualCount = 0;
                for (TiXmlElement *faceElem = faces->FirstChildElement(); faceElem != 0; faceElem = faceElem->NextSiblingElement())
                {
                        actualCount++;
                }
                const char *claimedCount_ = faces->Attribute("count");
                if (claimedCount_ && StringConverter::parseInt(claimedCount_)!=actualCount)
                {
				    LogManager::getSingleton().stream()
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
						OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "operationType not implemented", 
							__FUNCTION__);
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
					TiXmlElement* faceElem;
					bool firstTri = true;
					for (faceElem = faces->FirstChildElement();
						faceElem != 0; faceElem = faceElem->NextSiblingElement())
					{
						if (use32BitIndexes)
						{
							*pInt++ = StringConverter::parseInt(faceElem->Attribute("v1"));
							if(sm->operationType == RenderOperation::OT_LINE_LIST)
							{
								*pInt++ = StringConverter::parseInt(faceElem->Attribute("v2"));
							}
							// only need all 3 vertices if it's a trilist or first tri
							else if (sm->operationType == RenderOperation::OT_TRIANGLE_LIST || firstTri)
							{
								*pInt++ = StringConverter::parseInt(faceElem->Attribute("v2"));
								*pInt++ = StringConverter::parseInt(faceElem->Attribute("v3"));
							}
						}
						else
						{
							*pShort++ = StringConverter::parseInt(faceElem->Attribute("v1"));
							if(sm->operationType == RenderOperation::OT_LINE_LIST)
							{
								*pShort++ = StringConverter::parseInt(faceElem->Attribute("v2"));
							}
							// only need all 3 vertices if it's a trilist or first tri
							else if (sm->operationType == RenderOperation::OT_TRIANGLE_LIST || firstTri)
							{
								*pShort++ = StringConverter::parseInt(faceElem->Attribute("v2"));
								*pShort++ = StringConverter::parseInt(faceElem->Attribute("v3"));
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
                TiXmlElement* geomNode = smElem->FirstChildElement("geometry");
                if (geomNode)
                {
                    sm->vertexData = new VertexData();
                    readGeometry(geomNode, sm->vertexData);
                }
            }

            // texture aliases
            TiXmlElement* textureAliasesNode = smElem->FirstChildElement("textures");
            if(textureAliasesNode)
                readTextureAliases(textureAliasesNode, sm);

            // Bone assignments
            TiXmlElement* boneAssigns = smElem->FirstChildElement("boneassignments");
            if(boneAssigns)
                readBoneAssignments(boneAssigns, sm);

        }
        LogManager::getSingleton().logMessage("Submeshes done.");
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readGeometry(TiXmlElement* mGeometryNode, VertexData* vertexData)
    {
        LogManager::getSingleton().logMessage("Reading geometry...");
        unsigned char *pVert;
        float *pFloat;
        uint16 *pShort;
        uint8 *pChar;
        ARGB *pCol;

        const char *claimedVertexCount_ = mGeometryNode->Attribute("vertexcount");
        ptrdiff_t claimedVertexCount = 0;
        if (claimedVertexCount_)
        {
                claimedVertexCount =
                        StringConverter::parseInt(claimedVertexCount_);
        }
        // Skip empty 
        if (claimedVertexCount_ && claimedVertexCount <= 0) return;
        

        VertexDeclaration* decl = vertexData->vertexDeclaration;
        VertexBufferBinding* bind = vertexData->vertexBufferBinding;
        unsigned short bufCount = 0;
        unsigned short totalTexCoords = 0; // across all buffers

        // Information for calculating bounds
        Vector3 min = Vector3::ZERO, max = Vector3::UNIT_SCALE, pos = Vector3::ZERO;
        Real maxSquaredRadius = -1;
        bool first = true;

        // Iterate over all children (vertexbuffer entries) 
        for (TiXmlElement* vbElem = mGeometryNode->FirstChildElement();
            vbElem != 0; vbElem = vbElem->NextSiblingElement())
        {
            size_t offset = 0;
            // Skip non-vertexbuffer elems
            if (stricmp(vbElem->Value(), "vertexbuffer")) continue;
           
            const char* attrib = vbElem->Attribute("positions");
            if (attrib && StringConverter::parseBool(attrib))
            {
                // Add element
                decl->addElement(bufCount, offset, VET_FLOAT3, VES_POSITION);
                offset += VertexElement::getTypeSize(VET_FLOAT3);
            }
            attrib = vbElem->Attribute("normals");
            if (attrib && StringConverter::parseBool(attrib))
            {
                // Add element
                decl->addElement(bufCount, offset, VET_FLOAT3, VES_NORMAL);
                offset += VertexElement::getTypeSize(VET_FLOAT3);
            }
			attrib = vbElem->Attribute("tangents");
			if (attrib && StringConverter::parseBool(attrib))
			{
				VertexElementType tangentType = VET_FLOAT3;
				attrib = vbElem->Attribute("tangent_dimensions");
				if (attrib)
				{
					unsigned int dims = StringConverter::parseUnsignedInt(attrib);
					if (dims == 4)
						tangentType = VET_FLOAT4;
				}

				// Add element
				decl->addElement(bufCount, offset, tangentType, VES_TANGENT);
				offset += VertexElement::getTypeSize(tangentType);
			}
			attrib = vbElem->Attribute("binormals");
			if (attrib && StringConverter::parseBool(attrib))
			{
				// Add element
				decl->addElement(bufCount, offset, VET_FLOAT3, VES_BINORMAL);
				offset += VertexElement::getTypeSize(VET_FLOAT3);
			}
            attrib = vbElem->Attribute("colours_diffuse");
            if (attrib && StringConverter::parseBool(attrib))
            {
                // Add element
                decl->addElement(bufCount, offset, mColourElementType, VES_DIFFUSE);
                offset += VertexElement::getTypeSize(mColourElementType);
            }
            attrib = vbElem->Attribute("colours_specular");
            if (attrib && StringConverter::parseBool(attrib))
            {
                // Add element
                decl->addElement(bufCount, offset, mColourElementType, VES_SPECULAR);
                offset += VertexElement::getTypeSize(mColourElementType);
            }
            attrib = vbElem->Attribute("texture_coords");
            if (attrib && StringConverter::parseInt(attrib))
            {
                unsigned short numTexCoords = StringConverter::parseInt(vbElem->Attribute("texture_coords"));
                for (unsigned short tx = 0; tx < numTexCoords; ++tx)
                {
                    // NB set is local to this buffer, but will be translated into a 
                    // global set number across all vertex buffers
					StringUtil::StrStreamType str;
					str << "texture_coord_dimensions_" << tx;
                    attrib = vbElem->Attribute(str.str().c_str());
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
							vtype = VET_COLOUR;
						else if (!::strcmp(attrib,"colour_argb"))
							vtype = VET_COLOUR_ARGB;
						else if (!::strcmp(attrib,"colour_abgr"))
							vtype = VET_COLOUR_ABGR;
						else 
						{
							std::cerr << "ERROR: Did not recognise texture_coord_dimensions value of \""<<attrib<<"\"" << std::endl;
							std::cerr << "Falling back to default of VET_FLOAT2" << std::endl;
						}
					}
					// Add element
					decl->addElement(bufCount, offset, vtype, 
						VES_TEXTURE_COORDINATES, totalTexCoords++);
					offset += VertexElement::getTypeSize(vtype);
                }
            } 

            // calculate how many vertexes there actually are
            int actualVertexCount = 0;
            for (TiXmlElement * vertexElem = vbElem->FirstChildElement(); vertexElem != 0; vertexElem = vertexElem->NextSiblingElement())
            {
                    actualVertexCount++;
            }
            if (claimedVertexCount_ && actualVertexCount!=claimedVertexCount)
            {
				LogManager::getSingleton().stream()
					<< "WARNING: vertex count (" << actualVertexCount 
					<< ") is not as claimed (" << claimedVertexCount_ << ")";
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
            for (TiXmlElement* vertexElem = vbElem->FirstChildElement();
            vertexElem != 0; vertexElem = vertexElem->NextSiblingElement())
            {
                // Now parse the elements, ensure they are all matched
                VertexDeclaration::VertexElementList::const_iterator ielem, ielemend;
                TiXmlElement* xmlElem;
                TiXmlElement* texCoordElem = 0;
                ielemend = elems.end();
                for (ielem = elems.begin(); ielem != ielemend; ++ielem)
                {
                    const VertexElement& elem = *ielem;
                    // Find child for this element
                    switch(elem.getSemantic())
                    {
                    case VES_POSITION:
                        xmlElem = vertexElem->FirstChildElement("position");
                        if (!xmlElem)
                        {
                            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Missing <position> element.",
                                "XMLSerializer::readGeometry");
                        }
                        elem.baseVertexPointerToElement(pVert, &pFloat);

                        *pFloat++ = StringConverter::parseReal(
                            xmlElem->Attribute("x"));
                        *pFloat++ = StringConverter::parseReal(
                            xmlElem->Attribute("y"));
                        *pFloat++ = StringConverter::parseReal(
                            xmlElem->Attribute("z"));

                        pos.x = StringConverter::parseReal(
                            xmlElem->Attribute("x"));
                        pos.y = StringConverter::parseReal(
                            xmlElem->Attribute("y"));
                        pos.z = StringConverter::parseReal(
                            xmlElem->Attribute("z"));
                        
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
                        xmlElem = vertexElem->FirstChildElement("normal");
                        if (!xmlElem)
                        {
                            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Missing <normal> element.",
                                "XMLSerializer::readGeometry");
                        }
                        elem.baseVertexPointerToElement(pVert, &pFloat);

                        *pFloat++ = StringConverter::parseReal(
                            xmlElem->Attribute("x"));
                        *pFloat++ = StringConverter::parseReal(
                            xmlElem->Attribute("y"));
                        *pFloat++ = StringConverter::parseReal(
                            xmlElem->Attribute("z"));
                        break;
					case VES_TANGENT:
						xmlElem = vertexElem->FirstChildElement("tangent");
						if (!xmlElem)
						{
							OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Missing <tangent> element.",
								"XMLSerializer::readGeometry");
						}
						elem.baseVertexPointerToElement(pVert, &pFloat);

						*pFloat++ = StringConverter::parseReal(
							xmlElem->Attribute("x"));
						*pFloat++ = StringConverter::parseReal(
							xmlElem->Attribute("y"));
						*pFloat++ = StringConverter::parseReal(
							xmlElem->Attribute("z"));
						if (elem.getType() == VET_FLOAT4)
						{
							*pFloat++ = StringConverter::parseReal(
								xmlElem->Attribute("w"));
						}
						break;
					case VES_BINORMAL:
						xmlElem = vertexElem->FirstChildElement("binormal");
						if (!xmlElem)
						{
							OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Missing <binormal> element.",
								"XMLSerializer::readGeometry");
						}
						elem.baseVertexPointerToElement(pVert, &pFloat);

						*pFloat++ = StringConverter::parseReal(
							xmlElem->Attribute("x"));
						*pFloat++ = StringConverter::parseReal(
							xmlElem->Attribute("y"));
						*pFloat++ = StringConverter::parseReal(
							xmlElem->Attribute("z"));
						break;
                    case VES_DIFFUSE:
                        xmlElem = vertexElem->FirstChildElement("colour_diffuse");
                        if (!xmlElem)
                        {
                            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Missing <colour_diffuse> element.",
                                "XMLSerializer::readGeometry");
                        }
                        elem.baseVertexPointerToElement(pVert, &pCol);
						{
							ColourValue cv;
							cv = StringConverter::parseColourValue(
								xmlElem->Attribute("value"));
							*pCol++ = VertexElement::convertColourValue(cv, mColourElementType);
						}
                        break;
                    case VES_SPECULAR:
                        xmlElem = vertexElem->FirstChildElement("colour_specular");
                        if (!xmlElem)
                        {
                            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Missing <colour_specular> element.",
                                "XMLSerializer::readGeometry");
                        }
                        elem.baseVertexPointerToElement(pVert, &pCol);
						{
							ColourValue cv;
							cv = StringConverter::parseColourValue(
								xmlElem->Attribute("value"));
							*pCol++ = VertexElement::convertColourValue(cv, mColourElementType);
						}
                        break;
                    case VES_TEXTURE_COORDINATES:
                        if (!texCoordElem)
                        {
                            // Get first texcoord
                            xmlElem = vertexElem->FirstChildElement("texcoord");
                        }
                        else
                        {
                            // Get next texcoord
                            xmlElem = texCoordElem->NextSiblingElement("texcoord");
                        }
                        if (!xmlElem)
                        {
                            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Missing <texcoord> element.",
                                "XMLSerializer::readGeometry");
                        }
						// Record the latest texture coord entry
						texCoordElem = xmlElem;

                        if (!xmlElem->Attribute("u"))
                            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'u' attribute not found.", "XMLMeshSerializer::readGeometry");
                        
                        // depending on type, pack appropriately, can process colour channels separately which is a bonus
						switch (elem.getType()) 
						{
						case VET_FLOAT1:
							elem.baseVertexPointerToElement(pVert, &pFloat);
							*pFloat++ = StringConverter::parseReal(xmlElem->Attribute("u"));
							break;

						case VET_FLOAT2:
							if (!xmlElem->Attribute("v"))
								OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'v' attribute not found.", "XMLMeshSerializer::readGeometry");
							elem.baseVertexPointerToElement(pVert, &pFloat);
							*pFloat++ = StringConverter::parseReal(xmlElem->Attribute("u"));
							*pFloat++ = StringConverter::parseReal(xmlElem->Attribute("v"));
							break;

						case VET_FLOAT3:
							if (!xmlElem->Attribute("v"))
								OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'v' attribute not found.", "XMLMeshSerializer::readGeometry");
							if (!xmlElem->Attribute("w"))
								OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'w' attribute not found.", "XMLMeshSerializer::readGeometry");
							elem.baseVertexPointerToElement(pVert, &pFloat);
							*pFloat++ = StringConverter::parseReal(xmlElem->Attribute("u"));
							*pFloat++ = StringConverter::parseReal(xmlElem->Attribute("v"));
							*pFloat++ = StringConverter::parseReal(xmlElem->Attribute("w"));
							break;

						case VET_FLOAT4:
							if (!xmlElem->Attribute("v"))
								OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'v' attribute not found.", "XMLMeshSerializer::readGeometry");
							if (!xmlElem->Attribute("w"))
								OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'w' attribute not found.", "XMLMeshSerializer::readGeometry");
							if (!xmlElem->Attribute("x"))
								OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'x' attribute not found.", "XMLMeshSerializer::readGeometry");
							elem.baseVertexPointerToElement(pVert, &pFloat);
							*pFloat++ = StringConverter::parseReal(xmlElem->Attribute("u"));
							*pFloat++ = StringConverter::parseReal(xmlElem->Attribute("v"));
							*pFloat++ = StringConverter::parseReal(xmlElem->Attribute("w"));
							*pFloat++ = StringConverter::parseReal(xmlElem->Attribute("x"));
							break;

						case VET_SHORT1:
							elem.baseVertexPointerToElement(pVert, &pShort);
							*pShort++ = static_cast<uint16>(65535.0f * StringConverter::parseReal(xmlElem->Attribute("u")));
							break;

						case VET_SHORT2:
							if (!xmlElem->Attribute("v"))
								OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'v' attribute not found.", "XMLMeshSerializer::readGeometry");
							elem.baseVertexPointerToElement(pVert, &pShort);
							*pShort++ = static_cast<uint16>(65535.0f * StringConverter::parseReal(xmlElem->Attribute("u")));
							*pShort++ = static_cast<uint16>(65535.0f * StringConverter::parseReal(xmlElem->Attribute("v")));
							break;

						case VET_SHORT3:
							if (!xmlElem->Attribute("v"))
								OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'v' attribute not found.", "XMLMeshSerializer::readGeometry");
							if (!xmlElem->Attribute("w"))
								OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'w' attribute not found.", "XMLMeshSerializer::readGeometry");
							elem.baseVertexPointerToElement(pVert, &pShort);
							*pShort++ = static_cast<uint16>(65535.0f * StringConverter::parseReal(xmlElem->Attribute("u")));
							*pShort++ = static_cast<uint16>(65535.0f * StringConverter::parseReal(xmlElem->Attribute("v")));
							*pShort++ = static_cast<uint16>(65535.0f * StringConverter::parseReal(xmlElem->Attribute("w")));
							break;

						case VET_SHORT4:
							if (!xmlElem->Attribute("v"))
								OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'v' attribute not found.", "XMLMeshSerializer::readGeometry");
							if (!xmlElem->Attribute("w"))
								OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'w' attribute not found.", "XMLMeshSerializer::readGeometry");
							if (!xmlElem->Attribute("x"))
								OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'x' attribute not found.", "XMLMeshSerializer::readGeometry");
							elem.baseVertexPointerToElement(pVert, &pShort);
							*pShort++ = static_cast<uint16>(65535.0f * StringConverter::parseReal(xmlElem->Attribute("u")));
							*pShort++ = static_cast<uint16>(65535.0f * StringConverter::parseReal(xmlElem->Attribute("v")));
							*pShort++ = static_cast<uint16>(65535.0f * StringConverter::parseReal(xmlElem->Attribute("w")));
							*pShort++ = static_cast<uint16>(65535.0f * StringConverter::parseReal(xmlElem->Attribute("x")));
							break;

						case VET_UBYTE4:
							if (!xmlElem->Attribute("v"))
								OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'v' attribute not found.", "XMLMeshSerializer::readGeometry");
							if (!xmlElem->Attribute("w"))
								OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'w' attribute not found.", "XMLMeshSerializer::readGeometry");
							if (!xmlElem->Attribute("x"))
								OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Texcoord 'x' attribute not found.", "XMLMeshSerializer::readGeometry");
							elem.baseVertexPointerToElement(pVert, &pChar);
							// round off instead of just truncating -- avoids magnifying rounding errors
							*pChar++ = static_cast<uint8>(0.5f + 255.0f * StringConverter::parseReal(xmlElem->Attribute("u")));
							*pChar++ = static_cast<uint8>(0.5f + 255.0f * StringConverter::parseReal(xmlElem->Attribute("v")));
							*pChar++ = static_cast<uint8>(0.5f + 255.0f * StringConverter::parseReal(xmlElem->Attribute("w")));
							*pChar++ = static_cast<uint8>(0.5f + 255.0f * StringConverter::parseReal(xmlElem->Attribute("x")));
							break;

						case VET_COLOUR: 
							{
								elem.baseVertexPointerToElement(pVert, &pCol);
								ColourValue cv = StringConverter::parseColourValue(xmlElem->Attribute("u"));
								*pCol++ = VertexElement::convertColourValue(cv, mColourElementType);
							}
							break;

						case VET_COLOUR_ARGB:
						case VET_COLOUR_ABGR: 
							{
								elem.baseVertexPointerToElement(pVert, &pCol);
								ColourValue cv = StringConverter::parseColourValue(xmlElem->Attribute("u"));
								*pCol++ = VertexElement::convertColourValue(cv, elem.getType());
							}
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
        const AxisAlignedBox& currBox = mpMesh->getBounds();
        Real currRadius = mpMesh->getBoundingSphereRadius();
        if (currBox.isNull())
        {
	    //do not pad the bounding box
            mpMesh->_setBounds(AxisAlignedBox(min, max), false);
            mpMesh->_setBoundingSphereRadius(Math::Sqrt(maxSquaredRadius));
        }
        else
        {
            AxisAlignedBox newBox(min, max);
            newBox.merge(currBox);
	    //do not pad the bounding box
            mpMesh->_setBounds(newBox, false);
            mpMesh->_setBoundingSphereRadius(std::max(Math::Sqrt(maxSquaredRadius), currRadius));
        }
        

        LogManager::getSingleton().logMessage("Geometry done...");
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readSkeletonLink(TiXmlElement* mSkelNode)
    {
        mpMesh->setSkeletonName(mSkelNode->Attribute("name"));
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readBoneAssignments(TiXmlElement* mBoneAssignmentsNode)
    {
        LogManager::getSingleton().logMessage("Reading bone assignments...");

        // Iterate over all children (vertexboneassignment entries)
        for (TiXmlElement* elem = mBoneAssignmentsNode->FirstChildElement();
        elem != 0; elem = elem->NextSiblingElement())
        {
            VertexBoneAssignment vba;
            vba.vertexIndex = StringConverter::parseInt(
                elem->Attribute("vertexindex"));
            vba.boneIndex = StringConverter::parseInt(
                elem->Attribute("boneindex"));
            vba.weight= StringConverter::parseReal(
                elem->Attribute("weight"));

            mpMesh->addBoneAssignment(vba);
        }

        LogManager::getSingleton().logMessage("Bone assignments done.");
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readTextureAliases(TiXmlElement* mTextureAliasesNode, SubMesh* subMesh)
    {
        LogManager::getSingleton().logMessage("Reading sub mesh texture aliases...");

        // Iterate over all children (texture entries)
        for (TiXmlElement* elem = mTextureAliasesNode->FirstChildElement();
             elem != 0; elem = elem->NextSiblingElement())
        {
            // pass alias and texture name to submesh
            // read attribute "alias"
            String alias = elem->Attribute("alias");
            // read attribute "name"
            String name = elem->Attribute("name");

            subMesh->addTextureAlias(alias, name);
        }

        LogManager::getSingleton().logMessage("Texture aliases done.");
    }
    //---------------------------------------------------------------------
	void XMLMeshSerializer::readSubMeshNames(TiXmlElement* mMeshNamesNode, Mesh *sm)
	{
		LogManager::getSingleton().logMessage("Reading mesh names...");

		// Iterate over all children (vertexboneassignment entries)
		for (TiXmlElement* elem = mMeshNamesNode->FirstChildElement();
			elem != 0; elem = elem->NextSiblingElement())
		{
			String meshName = elem->Attribute("name");
			int index = StringConverter::parseInt(elem->Attribute("index"));

			sm->nameSubMesh(meshName, index);
		}

		LogManager::getSingleton().logMessage("Mesh names done.");
	}
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readBoneAssignments(TiXmlElement* mBoneAssignmentsNode, SubMesh* sm)
    {
        LogManager::getSingleton().logMessage("Reading bone assignments...");
        // Iterate over all children (vertexboneassignment entries)
        for (TiXmlElement* elem = mBoneAssignmentsNode->FirstChildElement();
        elem != 0; elem = elem->NextSiblingElement())
        {
            VertexBoneAssignment vba;
            vba.vertexIndex = StringConverter::parseInt(
                elem->Attribute("vertexindex"));
            vba.boneIndex = StringConverter::parseInt(
                elem->Attribute("boneindex"));
            vba.weight= StringConverter::parseReal(
                elem->Attribute("weight"));

            sm->addBoneAssignment(vba);
        }
        LogManager::getSingleton().logMessage("Bone assignments done.");
    }
    //---------------------------------------------------------------------
	void XMLMeshSerializer::writeLodInfo(TiXmlElement* mMeshNode, const Mesh* pMesh)
	{
        TiXmlElement* lodNode = 
            mMeshNode->InsertEndChild(TiXmlElement("levelofdetail"))->ToElement();

        const LodStrategy *strategy = pMesh->getLodStrategy();
		unsigned short numLvls = pMesh->getNumLodLevels();
		bool manual = pMesh->isLodManual();
        lodNode->SetAttribute("strategy", strategy->getName());
		lodNode->SetAttribute("numlevels", StringConverter::toString(numLvls));
		lodNode->SetAttribute("manual", StringConverter::toString(manual));

		// Iterate from level 1, not 0 (full detail)
		for (unsigned short i = 1; i < numLvls; ++i)
		{
			const MeshLodUsage& usage = pMesh->getLodLevel(i);
			if (manual)
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
    void XMLMeshSerializer::writeSubMeshNames(TiXmlElement* mMeshNode, const Mesh* m)
    {
        const Mesh::SubMeshNameMap& nameMap = m->getSubMeshNameMap();
        if (nameMap.empty())
            return; // do nothing

        TiXmlElement* namesNode = 
            mMeshNode->InsertEndChild(TiXmlElement("submeshnames"))->ToElement();
        Mesh::SubMeshNameMap::const_iterator i, iend;
        iend = nameMap.end();
        for (i = nameMap.begin(); i != iend; ++i)
        {
            TiXmlElement* subNameNode = 
                namesNode->InsertEndChild(TiXmlElement("submeshname"))->ToElement();

            subNameNode->SetAttribute("name", i->first);
            subNameNode->SetAttribute("index", 
                StringConverter::toString(i->second));
        }

    }
    //---------------------------------------------------------------------
	void XMLMeshSerializer::writeLodUsageManual(TiXmlElement* usageNode, 
		unsigned short levelNum, const MeshLodUsage& usage)
	{
		TiXmlElement* manualNode = 
			usageNode->InsertEndChild(TiXmlElement("lodmanual"))->ToElement();

		manualNode->SetAttribute("value", 
            StringConverter::toString(usage.userValue));
		manualNode->SetAttribute("meshname", usage.manualName);

	}
    //---------------------------------------------------------------------
	void XMLMeshSerializer::writeLodUsageGenerated(TiXmlElement* usageNode, 
		unsigned short levelNum,  const MeshLodUsage& usage, 
		const Mesh* pMesh)
	{
		TiXmlElement* generatedNode = 
			usageNode->InsertEndChild(TiXmlElement("lodgenerated"))->ToElement();
		generatedNode->SetAttribute("value", 
			StringConverter::toString(usage.userValue));

		// Iterate over submeshes at this level
		unsigned short numsubs = pMesh->getNumSubMeshes();

		for (unsigned short subi = 0; subi < numsubs; ++subi)
		{
			TiXmlElement* subNode = 
				generatedNode->InsertEndChild(TiXmlElement("lodfacelist"))->ToElement();
			SubMesh* sub = pMesh->getSubMesh(subi);
			subNode->SetAttribute("submeshindex", StringConverter::toString(subi));
			// NB level - 1 because SubMeshes don't store the first index in geometry
		    IndexData* facedata = sub->mLodFaceList[levelNum - 1];
			subNode->SetAttribute("numfaces", StringConverter::toString(facedata->indexCount / 3));

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
				}
				else
				{
					pShort = static_cast<unsigned short*>(
						ibuf->lock(HardwareBuffer::HBL_READ_ONLY)); 
				}
				
				for (size_t f = 0; f < facedata->indexCount; f += 3)
				{
					TiXmlElement* faceNode = 
						subNode->InsertEndChild(TiXmlElement("face"))->ToElement();
					if (use32BitIndexes)
					{
						faceNode->SetAttribute("v1", StringConverter::toString(*pInt++));
						faceNode->SetAttribute("v2", StringConverter::toString(*pInt++));
						faceNode->SetAttribute("v3", StringConverter::toString(*pInt++));
					}
					else
					{
						faceNode->SetAttribute("v1", StringConverter::toString(*pShort++));
						faceNode->SetAttribute("v2", StringConverter::toString(*pShort++));
						faceNode->SetAttribute("v3", StringConverter::toString(*pShort++));
					}

				}
			}

		}

	}
    //---------------------------------------------------------------------
	void XMLMeshSerializer::writeExtremes(TiXmlElement* mMeshNode, const Mesh* m)
	{
		TiXmlElement* extremesNode = NULL;
		int idx = 0;
		for (Mesh::SubMeshIterator i = ((Mesh &)*m).getSubMeshIterator ();
			 i.hasMoreElements (); i.moveNext (), ++idx)
		{
			SubMesh *sm = i.peekNext ();
			if (sm->extremityPoints.empty())
				continue; // do nothing

			if (!extremesNode)
				extremesNode = mMeshNode->InsertEndChild(TiXmlElement("extremes"))->ToElement();

			TiXmlElement* submeshNode =
				extremesNode->InsertEndChild(TiXmlElement("submesh_extremes"))->ToElement();

			submeshNode->SetAttribute("index",  StringConverter::toString(idx));

			for (vector<Vector3>::type::const_iterator v = sm->extremityPoints.begin ();
				 v != sm->extremityPoints.end (); ++v)
			{
				TiXmlElement* vert = submeshNode->InsertEndChild(
					TiXmlElement("position"))->ToElement();
				vert->SetAttribute("x", StringConverter::toString(v->x));
				vert->SetAttribute("y", StringConverter::toString(v->y));
				vert->SetAttribute("z", StringConverter::toString(v->z));
			}
		}
	}
	//---------------------------------------------------------------------
	void XMLMeshSerializer::readLodInfo(TiXmlElement*  lodNode)
	{
		
        LogManager::getSingleton().logMessage("Parsing LOD information...");

        const char* val = lodNode->Attribute("strategy");
        // This attribute is optional to maintain backwards compatibility
        if (val)
        {
            String strategyName = val;
            LodStrategy *strategy = LodStrategyManager::getSingleton().getStrategy(strategyName);
            mpMesh->setLodStrategy(strategy);
        }

		val = lodNode->Attribute("numlevels");
		unsigned short numLevels = static_cast<unsigned short>(
			StringConverter::parseUnsignedInt(val));

		val = lodNode->Attribute("manual");
		bool manual = StringConverter::parseBool(val);

		// Set up the basic structures
		mpMesh->_setLodInfo(numLevels, manual);

		// Parse the detail, start from 1 (the first sub-level of detail)
		unsigned short i = 1;
		TiXmlElement* usageElem;
		if (manual)
		{
			usageElem = lodNode->FirstChildElement("lodmanual");
		}
		else
		{
			usageElem = lodNode->FirstChildElement("lodgenerated");
		}
		while (usageElem)
		{
			if (manual)
			{
				readLodUsageManual(usageElem, i);
				usageElem = usageElem->NextSiblingElement();
			}
			else
			{
				readLodUsageGenerated(usageElem, i);
				usageElem = usageElem->NextSiblingElement();
			}
			++i;
		}
		
        LogManager::getSingleton().logMessage("LOD information done.");
		
	}
    //---------------------------------------------------------------------
	void XMLMeshSerializer::readLodUsageManual(TiXmlElement* manualNode, unsigned short index)
	{
		MeshLodUsage usage;
		const char* val = manualNode->Attribute("value");

        // If value attribute not found check for old name
        if (!val)
        {
            val = manualNode->Attribute("fromdepthsquared");
            if (val)
                LogManager::getSingleton().logMessage("WARNING: 'fromdepthsquared' attribute has been renamed to 'value'.");
			// user values are non-squared
			usage.userValue = Math::Sqrt(StringConverter::parseReal(val));
        }
		else
		{
			usage.userValue = StringConverter::parseReal(val);
		}
		usage.value = mpMesh->getLodStrategy()->transformUserValue(usage.userValue);
		usage.manualName = manualNode->Attribute("meshname");
        usage.edgeData = NULL;

		mpMesh->_setLodUsage(index, usage);
	}
    //---------------------------------------------------------------------
	void XMLMeshSerializer::readLodUsageGenerated(TiXmlElement* genNode, unsigned short index)
	{
		MeshLodUsage usage;
		const char* val = genNode->Attribute("value");

        // If value attribute not found check for old name
        if (!val)
        {
            val = genNode->Attribute("fromdepthsquared");
            if (val)
                LogManager::getSingleton().logMessage("WARNING: 'fromdepthsquared' attribute has been renamed to 'value'.");
			// user values are non-squared
			usage.userValue = Math::Sqrt(StringConverter::parseReal(val));
		}
		else
		{
			usage.userValue = StringConverter::parseReal(val);
		}
		usage.value = mpMesh->getLodStrategy()->transformUserValue(usage.userValue);
		usage.manualMesh.setNull();
		usage.manualName = "";
        usage.edgeData = NULL;

		mpMesh->_setLodUsage(index, usage);

		// Read submesh face lists
		TiXmlElement* faceListElem = genNode->FirstChildElement("lodfacelist");
		HardwareIndexBufferSharedPtr ibuf;
		while (faceListElem)
		{
			val = faceListElem->Attribute("submeshindex");
			unsigned short subidx = StringConverter::parseUnsignedInt(val);
			val = faceListElem->Attribute("numfaces");
			unsigned short numFaces = StringConverter::parseUnsignedInt(val);
			if (numFaces)
			{
				// use of 32bit indexes depends on submesh
				HardwareIndexBuffer::IndexType itype = 
					mpMesh->getSubMesh(subidx)->indexData->indexBuffer->getType();
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
				TiXmlElement* faceElem = faceListElem->FirstChildElement("face");
				for (unsigned int face = 0; face < numFaces; ++face, faceElem = faceElem->NextSiblingElement())
				{
					if (use32bitindexes)
					{
						val = faceElem->Attribute("v1");
						*pInt++ = StringConverter::parseUnsignedInt(val);
						val = faceElem->Attribute("v2");
						*pInt++ = StringConverter::parseUnsignedInt(val);
						val = faceElem->Attribute("v3");
						*pInt++ = StringConverter::parseUnsignedInt(val);
					}
					else
					{
						val = faceElem->Attribute("v1");
						*pShort++ = StringConverter::parseUnsignedInt(val);
						val = faceElem->Attribute("v2");
						*pShort++ = StringConverter::parseUnsignedInt(val);
						val = faceElem->Attribute("v3");
						*pShort++ = StringConverter::parseUnsignedInt(val);
					}

				}

				ibuf->unlock();
			}
			IndexData* facedata = new IndexData(); // will be deleted by SubMesh
			facedata->indexCount = numFaces * 3;
            facedata->indexStart = 0;
            facedata->indexBuffer = ibuf;
			mpMesh->_setSubMeshLodFaceList(subidx, index, facedata);

			faceListElem = faceListElem->NextSiblingElement();
		}
        
	}
	//-----------------------------------------------------------------------------
	void XMLMeshSerializer::readExtremes(TiXmlElement* extremesNode, Mesh *m)
	{
		LogManager::getSingleton().logMessage("Reading extremes...");

		// Iterate over all children (submesh_extreme list)
		for (TiXmlElement* elem = extremesNode->FirstChildElement();
			 elem != 0; elem = elem->NextSiblingElement())
		{
			int index = StringConverter::parseInt(elem->Attribute("index"));

			SubMesh *sm = m->getSubMesh(index);
			sm->extremityPoints.clear ();
			for (TiXmlElement* vert = elem->FirstChildElement();
				 vert != 0; vert = vert->NextSiblingElement())
			{
				Vector3 v;
				v.x = StringConverter::parseReal(vert->Attribute("x"));
				v.y = StringConverter::parseReal(vert->Attribute("y"));
				v.z = StringConverter::parseReal(vert->Attribute("z"));
				sm->extremityPoints.push_back (v);
			}
		}

		LogManager::getSingleton().logMessage("Extremes done.");
	}
	//-----------------------------------------------------------------------------
	void XMLMeshSerializer::readPoses(TiXmlElement* posesNode, Mesh *m)
	{
		TiXmlElement* poseNode = posesNode->FirstChildElement("pose");

		while (poseNode)
		{
			const char* target = poseNode->Attribute("target");
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
				const char* val = poseNode->Attribute("index");
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
			const char* val = poseNode->Attribute("name");
			if (val)
				name = val;
			Pose* pose = m->createPose(targetID, name);

			TiXmlElement* poseOffsetNode = poseNode->FirstChildElement("poseoffset");
			while (poseOffsetNode)
			{
				uint index = StringConverter::parseUnsignedInt(
					poseOffsetNode->Attribute("index"));
				Vector3 offset;
				offset.x = StringConverter::parseReal(poseOffsetNode->Attribute("x"));
				offset.y = StringConverter::parseReal(poseOffsetNode->Attribute("y"));
				offset.z = StringConverter::parseReal(poseOffsetNode->Attribute("z"));

				pose->addVertex(index, offset);

				poseOffsetNode = poseOffsetNode->NextSiblingElement();
			}

			poseNode = poseNode->NextSiblingElement();

		}


	}
	//-----------------------------------------------------------------------------
	void XMLMeshSerializer::readAnimations(TiXmlElement* mAnimationsNode, Mesh *pMesh)
	{
		TiXmlElement* animElem = mAnimationsNode->FirstChildElement("animation");
		while (animElem)
		{
			String name = animElem->Attribute("name");
			const char* charLen = animElem->Attribute("length");
			Real len = StringConverter::parseReal(charLen);

			Animation* anim = pMesh->createAnimation(name, len);

			TiXmlElement* tracksNode = animElem->FirstChildElement("tracks");
			if (tracksNode)
			{
				readTracks(tracksNode, pMesh, anim);
			}

			animElem = animElem->NextSiblingElement();

		}


	}
	//-----------------------------------------------------------------------------
	void XMLMeshSerializer::readTracks(TiXmlElement* tracksNode, Mesh *m, Animation* anim)
	{
		TiXmlElement* trackNode = tracksNode->FirstChildElement("track");
		while (trackNode)
		{
			String target = trackNode->Attribute("target");
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
				const char* val = trackNode->Attribute("index");
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
			String strAnimType = trackNode->Attribute("type");
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

			TiXmlElement* keyframesNode = trackNode->FirstChildElement("keyframes");
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

			trackNode = trackNode->NextSiblingElement();
		}
	}
	//-----------------------------------------------------------------------------
	void XMLMeshSerializer::readMorphKeyFrames(TiXmlElement* keyframesNode, 
		VertexAnimationTrack* track, size_t vertexCount)
	{
		TiXmlElement* keyNode = keyframesNode->FirstChildElement("keyframe");
		while (keyNode)
		{
			const char* val = keyNode->Attribute("time");
			if (!val)
			{
				OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
					"Required attribute 'time' missing on keyframe", 
					"XMLMeshSerializer::readKeyFrames");
			}
			Real time = StringConverter::parseReal(val);

			VertexMorphKeyFrame* kf = track->createVertexMorphKeyFrame(time);

			// create a vertex buffer
			HardwareVertexBufferSharedPtr vbuf = 
				HardwareBufferManager::getSingleton().createVertexBuffer(
				VertexElement::getTypeSize(VET_FLOAT3), vertexCount, 
				HardwareBuffer::HBU_STATIC, true);

			float* pFloat = static_cast<float*>(
				vbuf->lock(HardwareBuffer::HBL_DISCARD));


			TiXmlElement* posNode = keyNode->FirstChildElement("position");
			for (size_t v = 0; v < vertexCount; ++v)
			{
				if (!posNode)
				{
					OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
						"Not enough 'position' elements under keyframe", 
						"XMLMeshSerializer::readKeyFrames");

				}

				*pFloat++ = StringConverter::parseReal(
					posNode->Attribute("x"));
				*pFloat++ = StringConverter::parseReal(
					posNode->Attribute("y"));
				*pFloat++ = StringConverter::parseReal(
					posNode->Attribute("z"));


				posNode = posNode->NextSiblingElement("position");
			}

			vbuf->unlock();

			kf->setVertexBuffer(vbuf);
				

			keyNode = keyNode->NextSiblingElement();
		}


	}
	//-----------------------------------------------------------------------------
	void XMLMeshSerializer::readPoseKeyFrames(TiXmlElement* keyframesNode, VertexAnimationTrack* track)
	{
		TiXmlElement* keyNode = keyframesNode->FirstChildElement("keyframe");
		while (keyNode)
		{
			const char* val = keyNode->Attribute("time");
			if (!val)
			{
				OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
					"Required attribute 'time' missing on keyframe", 
					"XMLMeshSerializer::readKeyFrames");
			}
			Real time = StringConverter::parseReal(val);

			VertexPoseKeyFrame* kf = track->createVertexPoseKeyFrame(time);

			// Read all pose references
			TiXmlElement* poseRefNode = keyNode->FirstChildElement("poseref");
			while (poseRefNode)
			{
				const char* val = poseRefNode->Attribute("poseindex");
				if (!val)
				{
					OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
						"Required attribute 'poseindex' missing on poseref", 
						"XMLMeshSerializer::readPoseKeyFrames");
				}
				unsigned short poseIndex = StringConverter::parseUnsignedInt(val);
				Real influence = 1.0f;
				val = poseRefNode->Attribute("influence");
				if (val)
				{
					influence = StringConverter::parseReal(val);
				}

				kf->addPoseReference(poseIndex, influence);

				poseRefNode = poseRefNode->NextSiblingElement();
			}

			keyNode = keyNode->NextSiblingElement();
		}

	}
	//-----------------------------------------------------------------------------
	void XMLMeshSerializer::writePoses(TiXmlElement* meshNode, const Mesh* m)
	{
		if (m->getPoseCount() == 0)
			return;

		TiXmlElement* posesNode = 
			meshNode->InsertEndChild(TiXmlElement("poses"))->ToElement();

		Mesh::ConstPoseIterator poseIt = m->getPoseIterator();
		while (poseIt.hasMoreElements())
		{
			const Pose* pose = poseIt.getNext();
			TiXmlElement* poseNode = 
				posesNode->InsertEndChild(TiXmlElement("pose"))->ToElement();
			unsigned short target = pose->getTarget();
			if (target == 0)
			{
				// Main mesh
				poseNode->SetAttribute("target", "mesh");
			}
			else
			{
				// Submesh - rebase index
				poseNode->SetAttribute("target", "submesh");
				poseNode->SetAttribute("index", 
					StringConverter::toString(target - 1));
			}
			poseNode->SetAttribute("name", pose->getName());

			Pose::ConstVertexOffsetIterator vit = pose->getVertexOffsetIterator();
			while (vit.hasMoreElements())
			{
				TiXmlElement* poseOffsetElement = poseNode->InsertEndChild(
					TiXmlElement("poseoffset"))->ToElement();

				poseOffsetElement->SetAttribute("index", 
					StringConverter::toString(vit.peekNextKey()));

				Vector3 offset = vit.getNext();
				poseOffsetElement->SetAttribute("x", StringConverter::toString(offset.x));
				poseOffsetElement->SetAttribute("y", StringConverter::toString(offset.y));
				poseOffsetElement->SetAttribute("z", StringConverter::toString(offset.z));


			}

		}

	}
	//-----------------------------------------------------------------------------
	void XMLMeshSerializer::writeAnimations(TiXmlElement* meshNode, const Mesh* m)
	{
		// Skip if no animation
		if (!m->hasVertexAnimation())
			return;

		TiXmlElement* animationsNode = 
			meshNode->InsertEndChild(TiXmlElement("animations"))->ToElement();

		for (unsigned short a = 0; a < m->getNumAnimations(); ++a)
		{
			Animation* anim = m->getAnimation(a);

			TiXmlElement* animNode = 
				animationsNode->InsertEndChild(TiXmlElement("animation"))->ToElement();
			animNode->SetAttribute("name", anim->getName());
			animNode->SetAttribute("length", 
				StringConverter::toString(anim->getLength()));

			TiXmlElement* tracksNode = 
				animNode->InsertEndChild(TiXmlElement("tracks"))->ToElement();
			Animation::VertexTrackIterator iter = anim->getVertexTrackIterator();
			while(iter.hasMoreElements())
			{
				const VertexAnimationTrack* track = iter.getNext();
				TiXmlElement* trackNode = 
					tracksNode->InsertEndChild(TiXmlElement("track"))->ToElement();

				unsigned short targetID = track->getHandle();
				if (targetID == 0)
				{
					trackNode->SetAttribute("target", "mesh");
				}
				else
				{
					trackNode->SetAttribute("target", "submesh");
					trackNode->SetAttribute("index", 
						StringConverter::toString(targetID-1));
				}

				if (track->getAnimationType() == VAT_MORPH)
				{
					trackNode->SetAttribute("type", "morph");
					writeMorphKeyFrames(trackNode, track);
				}
				else
				{
					trackNode->SetAttribute("type", "pose");
					writePoseKeyFrames(trackNode, track);
				}


			}
		}

		
	}
	//-----------------------------------------------------------------------------
	void XMLMeshSerializer::writeMorphKeyFrames(TiXmlElement* trackNode, const VertexAnimationTrack* track)
	{
		TiXmlElement* keyframesNode = 
			trackNode->InsertEndChild(TiXmlElement("keyframes"))->ToElement();

		size_t vertexCount = track->getAssociatedVertexData()->vertexCount;

		for (unsigned short k = 0; k < track->getNumKeyFrames(); ++k)
		{
			VertexMorphKeyFrame* kf = track->getVertexMorphKeyFrame(k);
			TiXmlElement* keyNode = 
				keyframesNode->InsertEndChild(TiXmlElement("keyframe"))->ToElement();
			keyNode->SetAttribute("time", 
				StringConverter::toString(kf->getTime()));

			HardwareVertexBufferSharedPtr vbuf = kf->getVertexBuffer();
			float* pFloat = static_cast<float*>(
				vbuf->lock(HardwareBuffer::HBL_READ_ONLY));

			for (size_t v = 0; v < vertexCount; ++v)
			{
				TiXmlElement* posNode = 
					keyNode->InsertEndChild(TiXmlElement("position"))->ToElement();
				posNode->SetAttribute("x", StringConverter::toString(*pFloat++));
				posNode->SetAttribute("y", StringConverter::toString(*pFloat++));
				posNode->SetAttribute("z", StringConverter::toString(*pFloat++));
			}

		}
	}
	//-----------------------------------------------------------------------------
	void XMLMeshSerializer::writePoseKeyFrames(TiXmlElement* trackNode, const VertexAnimationTrack* track)
	{
		TiXmlElement* keyframesNode = 
			trackNode->InsertEndChild(TiXmlElement("keyframes"))->ToElement();

		for (unsigned short k = 0; k < track->getNumKeyFrames(); ++k)
		{
			VertexPoseKeyFrame* kf = track->getVertexPoseKeyFrame(k);
			TiXmlElement* keyNode = 
				keyframesNode->InsertEndChild(TiXmlElement("keyframe"))->ToElement();
			keyNode->SetAttribute("time", 
				StringConverter::toString(kf->getTime()));

			VertexPoseKeyFrame::PoseRefIterator poseIt = kf->getPoseReferenceIterator();
			while (poseIt.hasMoreElements())
			{
				const VertexPoseKeyFrame::PoseRef& poseRef = poseIt.getNext();
				TiXmlElement* poseRefNode = 
					keyNode->InsertEndChild(TiXmlElement("poseref"))->ToElement();

				poseRefNode->SetAttribute("poseindex", 
					StringConverter::toString(poseRef.poseIndex));
				poseRefNode->SetAttribute("influence", 
					StringConverter::toString(poseRef.influence));

			}

		}


	}




}

