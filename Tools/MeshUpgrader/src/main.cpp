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


#include "Ogre.h"
#include "OgreMeshSerializer.h"
#include "OgreSkeletonSerializer.h"
#include "OgreDefaultHardwareBufferManager.h"
#include "OgreHardwareVertexBuffer.h"

#include <iostream>
#include <sys/stat.h>

using namespace std;
using namespace Ogre;

void help(void)
{
    // Print help message
    cout << endl << "OgreMeshUpgrader: Upgrades .mesh files to the latest version." << endl;
    cout << "Provided for OGRE by Steve Streeting 2004" << endl << endl;
    cout << "Usage: OgreMeshUpgrader [-e] sourcefile [destfile] " << endl;
	cout << "-i             = Interactive mode, prompt for options" << endl;
	cout << "-l lodlevels   = number of LOD levels" << endl;
	cout << "-d loddist     = distance increment to reduce LOD" << endl;
	cout << "-p lodpercent  = Percentage triangle reduction amount per LOD" << endl;
	cout << "-f lodnumtris  = Fixed vertex reduction per LOD" << endl;
    cout << "-e         = DON'T generate edge lists (for stencil shadows)" << endl;
    cout << "-t         = Generate tangents (for normal mapping)" << endl;
	cout << "-td [uvw|tangent]" << endl;
	cout << "           = Tangent vertex semantic destination (default tangent)" << endl;
	cout << "-ts [3|4]      = Tangent size (3 or 4 components, 4 includes parity, default 3)" << endl;
	cout << "-tm            = Split tangent vertices at UV mirror points" << endl;
	cout << "-tr            = Split tangent vertices where basis is rotated > 90 degrees" << endl;
	cout << "-r         = DON'T reorganise buffers to recommended format" << endl;
	cout << "-d3d       = Convert to D3D colour formats" << endl;
	cout << "-gl        = Convert to GL colour formats" << endl;
	cout << "-srcd3d    = Interpret ambiguous colours as D3D style" << endl;
	cout << "-srcgl     = Interpret ambiguous colours as GL style" << endl;
	cout << "-E endian  = Set endian mode 'big' 'little' or 'native' (default)" << endl;
	cout << "-b         = Recalculate bounding box (static meshes only)" << endl;
    cout << "sourcefile = name of file to convert" << endl;
    cout << "destfile   = optional name of file to write to. If you don't" << endl;
    cout << "             specify this OGRE overwrites the existing file." << endl;

    cout << endl;
}

struct UpgradeOptions
{
	bool interactive;
	bool suppressEdgeLists;
	bool generateTangents;
	VertexElementSemantic tangentSemantic;
	bool tangentUseParity;
	bool tangentSplitMirrored;
	bool tangentSplitRotated;
	bool dontReorganise;
	bool destColourFormatSet;
	VertexElementType destColourFormat;
	bool srcColourFormatSet;
	VertexElementType srcColourFormat;
	unsigned short numLods;
	Real lodDist;
	Real lodPercent;
	size_t lodFixed;
	bool usePercent;
	Serializer::Endian endian;
	bool recalcBounds;

};


// Crappy globals
// NB some of these are not directly used, but are required to
//   instantiate the singletons used in the dlls
LogManager* logMgr = 0;
Math* mth = 0;
LodStrategyManager* lodMgr = 0;
MaterialManager* matMgr = 0;
SkeletonManager* skelMgr = 0;
MeshSerializer* meshSerializer = 0;
SkeletonSerializer* skeletonSerializer = 0;
DefaultHardwareBufferManager *bufferManager = 0;
ResourceGroupManager* rgm = 0;
MeshManager* meshMgr = 0;
UpgradeOptions opts;

void parseOpts(UnaryOptionList& unOpts, BinaryOptionList& binOpts)
{
	opts.interactive = false;
	opts.suppressEdgeLists = false;
	opts.generateTangents = false;
	opts.tangentSemantic = VES_TANGENT;
	opts.tangentUseParity = false;
	opts.tangentSplitMirrored = false;
	opts.tangentSplitRotated = false;
	opts.dontReorganise = false;
	opts.endian = Serializer::ENDIAN_NATIVE;
	opts.destColourFormatSet = false;
	opts.srcColourFormatSet = false;

	opts.lodDist = 500;
	opts.lodFixed = 0;
	opts.lodPercent = 20;
	opts.numLods = 0;
	opts.usePercent = true;
	opts.recalcBounds = false;


	UnaryOptionList::iterator ui = unOpts.find("-e");
	opts.suppressEdgeLists = ui->second;
	ui = unOpts.find("-t");
	opts.generateTangents = ui->second;
	ui = unOpts.find("-tm");
	opts.tangentSplitMirrored = ui->second;
	ui = unOpts.find("-tr");
	opts.tangentSplitRotated = ui->second;

	ui = unOpts.find("-i");
	opts.interactive = ui->second;
	ui = unOpts.find("-r");
	opts.dontReorganise = ui->second;
	ui = unOpts.find("-d3d");
	if (ui->second)
	{
		opts.destColourFormatSet = true;
		opts.destColourFormat = VET_COLOUR_ARGB;
	}
	ui = unOpts.find("-gl");
	if (ui->second)
	{
		opts.destColourFormatSet = true;
		opts.destColourFormat = VET_COLOUR_ABGR;
	}
	ui = unOpts.find("-srcd3d");
	if (ui->second)
	{
		opts.srcColourFormatSet = true;
		opts.srcColourFormat = VET_COLOUR_ARGB;
	}
	ui = unOpts.find("-srcgl");
	if (ui->second)
	{
		opts.srcColourFormatSet = true;
		opts.srcColourFormat = VET_COLOUR_ABGR;
	}
	ui = unOpts.find("-b");
	if (ui->second)
	{
		opts.recalcBounds = true;
	}


	BinaryOptionList::iterator bi = binOpts.find("-l");
	if (!bi->second.empty())
	{
		opts.numLods = StringConverter::parseInt(bi->second);
	}

	bi = binOpts.find("-d");
	if (!bi->second.empty())
	{
		opts.lodDist = StringConverter::parseReal(bi->second);
	}

	bi = binOpts.find("-p");
	if (!bi->second.empty())
	{
		opts.lodPercent = StringConverter::parseReal(bi->second);
		opts.usePercent = true;
	}


	bi = binOpts.find("-f");
	if (!bi->second.empty())
	{
		opts.lodFixed = StringConverter::parseInt(bi->second);
		opts.usePercent = false;
	}

	bi = binOpts.find("-E");
	if (!bi->second.empty())
	{
	    if (bi->second == "big")
            opts.endian = Serializer::ENDIAN_BIG;
	    else if (bi->second == "little")
            opts.endian = Serializer::ENDIAN_LITTLE;
	    else 
            opts.endian = Serializer::ENDIAN_NATIVE;
    }
	bi = binOpts.find("-td");
	if (!bi->second.empty())
	{
		if (bi->second == "uvw")
			opts.tangentSemantic = VES_TEXTURE_COORDINATES;
		else // if (bi->second == "tangent"), or anything else
			opts.tangentSemantic = VES_TANGENT;
	}
	bi = binOpts.find("-ts");
	if (!bi->second.empty())
	{
		if (bi->second == "4")
			opts.tangentUseParity = true;
	}
}

String describeSemantic(VertexElementSemantic sem)
{
	switch (sem)
	{
	case VES_POSITION:
		return "Positions";
	case VES_NORMAL:
		return "Normals";
	case VES_BLEND_WEIGHTS:
		return "Blend Weights";
	case VES_BLEND_INDICES:
		return "Blend Indices";
	case VES_DIFFUSE:
		return "Diffuse";
	case VES_SPECULAR:
		return "Specular";
	case VES_TEXTURE_COORDINATES:
		return "Texture coordinates";
	case VES_BINORMAL:
		return "Binormals";
	case VES_TANGENT:
		return "Tangents";
	};
    return "";
}
void displayVertexBuffers(VertexDeclaration::VertexElementList& elemList)
{
	// Iterate per buffer
	unsigned short currentBuffer = 999;
	unsigned short elemNum = 0;
	VertexDeclaration::VertexElementList::iterator i, iend;
	iend = elemList.end();
	for (i = elemList.begin(); i != iend; ++i)
	{
		if (i->getSource() != currentBuffer)
		{
			currentBuffer = i->getSource();
			cout << "> Buffer " << currentBuffer << ":" << endl;
		}
		cout << "   - Element " << elemNum++ << ": " << describeSemantic(i->getSemantic());
		if (i->getSemantic() == VES_TEXTURE_COORDINATES)
		{
			cout << " (index " << i->getIndex() << ")"; 
		}
		cout << endl;

	}
}
// Sort routine for VertexElement
bool vertexElementLess(const VertexElement& e1, const VertexElement& e2)
{
	// Sort by source first
	if (e1.getSource() < e2.getSource())
	{
		return true;
	}
	else if (e1.getSource() == e2.getSource())
	{
		// Use ordering of semantics to sort
		if (e1.getSemantic() < e2.getSemantic())
		{
			return true;
		}
		else if (e1.getSemantic() == e2.getSemantic())
		{
			// Use index to sort
			if (e1.getIndex() < e2.getIndex())
			{
				return true;
			}
		}
	}
	return false;
}
void copyElems(VertexDeclaration* decl, VertexDeclaration::VertexElementList* elemList)
{
	
	elemList->clear();
	const VertexDeclaration::VertexElementList& origElems = decl->getElements();
    VertexDeclaration::VertexElementList::const_iterator i, iend;
	iend = origElems.end();
	for (i = origElems.begin(); i != iend; ++i)
	{
		elemList->push_back(*i);
	}
	elemList->sort(VertexDeclaration::vertexElementLess);
}
// Utility function to allow the user to modify the layout of vertex buffers.
void reorganiseVertexBuffers(const String& desc, Mesh& mesh, VertexData* vertexData)
{
	cout << endl << desc << ":- " << endl;
	// Copy elements into a list
	VertexDeclaration::VertexElementList elemList;
	copyElems(vertexData->vertexDeclaration, &elemList);

	bool finish = false;
	bool anyChanges = false;
	while (!finish)
	{
		displayVertexBuffers(elemList);
		cout << endl;

		cout << "Options: (a)utomatic" << endl;
        cout << "         (m)ove element" << endl;
		cout << "         (d)elete element" << endl;
		cout << "         (r)eset" << endl;
		cout << "         (f)inish" << endl;
		String response = "";
		while (response.empty())
		{
			cin >> response;
			StringUtil::toLowerCase(response);

			if (response == "m")
			{
				String moveResp;
				cout << "Which element do you want to move (type number): ";
				cin >> moveResp;
				if (!moveResp.empty())
				{
					int eindex = StringConverter::parseInt(moveResp);
					VertexDeclaration::VertexElementList::iterator movei = elemList.begin();
					std::advance(movei, eindex);
					cout << endl << "Move element " << eindex << "(" + describeSemantic(movei->getSemantic()) << ") to which buffer: ";
					cin >> moveResp;
					if (!moveResp.empty())
					{
						int bindex = StringConverter::parseInt(moveResp);
						// Move (note offset will be wrong)
						*movei = VertexElement(bindex, 0, movei->getType(),
							movei->getSemantic(), movei->getIndex());
						elemList.sort(vertexElementLess);
                        anyChanges = true;
								
					}
				}
			}
            else if (response == "a")
            {
                // Automatic
                VertexDeclaration* newDcl = 
                    vertexData->vertexDeclaration->getAutoOrganisedDeclaration(
                        mesh.hasSkeleton(), mesh.hasVertexAnimation());
                copyElems(newDcl, &elemList);
                HardwareBufferManager::getSingleton().destroyVertexDeclaration(newDcl);
                anyChanges = true;

            }
			else if (response == "d")
			{
				String moveResp;
				cout << "Which element do you want to delete (type number): ";
				cin >> moveResp;
				if (!moveResp.empty())
				{
					int eindex = StringConverter::parseInt(moveResp);
					VertexDeclaration::VertexElementList::iterator movei = elemList.begin();
					std::advance(movei, eindex);
                    cout << std::endl << "Delete element " << eindex << "(" + describeSemantic(movei->getSemantic()) << ")?: ";
					cin >> moveResp;
					StringUtil::toLowerCase(moveResp);
					if (moveResp == "y")
					{
						elemList.erase(movei);
                        anyChanges = true;
					}
				}
			}
			else if (response == "r")
			{
				// reset
				copyElems(vertexData->vertexDeclaration, &elemList);
				anyChanges = false;
			}
			else if (response == "f")
			{
				// finish
				finish = true;
			}
			else
			{
				response == "";
			}
			
		}
	}

	if (anyChanges)
	{
		String response;
		while (response.empty())
		{
			displayVertexBuffers(elemList);
			cout << "Really reorganise the vertex buffers this way?";
			cin >> response;
			StringUtil::toLowerCase(response);
			if (response == "y")
			{
				VertexDeclaration* newDecl = HardwareBufferManager::getSingleton().createVertexDeclaration();
				VertexDeclaration::VertexElementList::iterator i, iend;
				iend = elemList.end();
				unsigned short currentBuffer = 999;
				size_t offset = 0;
				for (i = elemList.begin(); i != iend; ++i)
				{
					// Calc offsets since reorg changes them
					if (i->getSource() != currentBuffer)
					{
						offset = 0;
						currentBuffer = i->getSource();
					}
					newDecl->addElement(
						currentBuffer,
						offset,
						i->getType(),
						i->getSemantic(),
						i->getIndex());

					offset += VertexElement::getTypeSize(i->getType());
					
				}
                // Usages don't matter here since we're onlly exporting
                BufferUsageList bufferUsages;
                for (size_t u = 0; u <= newDecl->getMaxSource(); ++u)
                    bufferUsages.push_back(HardwareBuffer::HBU_STATIC_WRITE_ONLY);
				vertexData->reorganiseBuffers(newDecl, bufferUsages);
			}
			else if (response == "n")
			{
				// do nothing
			}
			else
			{
				response = "";
			}
        }
		
	}
		

	
}
// Utility function to allow the user to modify the layout of vertex buffers.
void reorganiseVertexBuffers(Mesh& mesh)
{
	if (mesh.sharedVertexData)
	{
		if (opts.interactive)
			reorganiseVertexBuffers("Shared Geometry", mesh, mesh.sharedVertexData);
		else
		{
			// Automatic
			VertexDeclaration* newDcl = 
				mesh.sharedVertexData->vertexDeclaration->getAutoOrganisedDeclaration(
				mesh.hasSkeleton(), mesh.hasVertexAnimation());
			if (*newDcl != *(mesh.sharedVertexData->vertexDeclaration))
			{
				// Usages don't matter here since we're onlly exporting
				BufferUsageList bufferUsages;
				for (size_t u = 0; u <= newDcl->getMaxSource(); ++u)
					bufferUsages.push_back(HardwareBuffer::HBU_STATIC_WRITE_ONLY);
				mesh.sharedVertexData->reorganiseBuffers(newDcl, bufferUsages);
			}

		}
	}

    Mesh::SubMeshIterator smIt = mesh.getSubMeshIterator();
	unsigned short idx = 0;
	while (smIt.hasMoreElements())
	{
		SubMesh* sm = smIt.getNext();
		if (!sm->useSharedVertices)
		{
			if (opts.interactive)
			{
				StringUtil::StrStreamType str;
				str << "SubMesh " << idx++; 
				reorganiseVertexBuffers(str.str(), mesh, sm->vertexData);
			}
			else
			{
				// Automatic
				VertexDeclaration* newDcl = 
					sm->vertexData->vertexDeclaration->getAutoOrganisedDeclaration(
					mesh.hasSkeleton(), mesh.hasVertexAnimation());
				if (*newDcl != *(sm->vertexData->vertexDeclaration))
				{
					// Usages don't matter here since we're onlly exporting
					BufferUsageList bufferUsages;
					for (size_t u = 0; u <= newDcl->getMaxSource(); ++u)
						bufferUsages.push_back(HardwareBuffer::HBU_STATIC_WRITE_ONLY);
					sm->vertexData->reorganiseBuffers(newDcl, bufferUsages);
				}
				
			}
		}
	}
}


void vertexBufferReorg(Mesh& mesh)
{
	String response;

	if (opts.interactive)
	{

		// Check to see whether we would like to reorganise vertex buffers
		std::cout << "\nWould you like to reorganise the vertex buffers for this mesh?";
		while (response.empty())
		{
			cin >> response;
			StringUtil::toLowerCase(response);
			if (response == "y")
			{
				reorganiseVertexBuffers(mesh);
			}
			else if (response == "n")
			{
				// Do nothing
			}
			else
			{
				response = "";
			}
		}
	}
	else if (!opts.dontReorganise)
	{
		reorganiseVertexBuffers(mesh);
	}

}

void buildLod(Mesh* mesh)
{	
	String response;

	// Prompt for LOD generation?
	bool genLod = false;
	bool askLodDtls = false;
	if (!opts.interactive) // derive from params if in batch mode
	{
		askLodDtls = false;
		if (opts.numLods == 0)
		{
			genLod = false;
		}
		else
		{
			genLod = true;
		}
	}
	else if(opts.numLods == 0) // otherwise only ask if not specified on command line
	{
		if (mesh->getNumLodLevels() > 1)
		{
			std::cout << "\nXML already contains level-of detail information.\n"
				"Do you want to: (u)se it, (r)eplace it, or (d)rop it?";
			while (response == "")
			{
				cin >> response;
				StringUtil::toLowerCase(response);
				if (response == "u")
				{
					// Do nothing
				}
				else if (response == "d")
				{
					mesh->removeLodLevels();
				}
				else if (response == "r")
				{
					genLod = true;
					askLodDtls = true;

				}
				else
				{
					response = "";
				}
			}// while response == ""
		}
		else // no existing LOD
		{
			std::cout << "\nWould you like to generate LOD information? (y/n)";
			while (response == "")
			{
				cin >> response;
				StringUtil::toLowerCase(response);
				if (response == "n")
				{
					// Do nothing
				}
				else if (response == "y")
				{
					genLod = true;
					askLodDtls = true;
				}
			}
		}
	}

	if (genLod)
	{
		unsigned short numLod;
		ProgressiveMesh::VertexReductionQuota quota;
		Real reduction;
		Mesh::LodValueList distanceList;

		if (askLodDtls)
		{
			cout << "\nHow many extra LOD levels would you like to generate?";
			cin >> numLod;

			cout << "\nWhat unit of reduction would you like to use:" <<
				"\n(f)ixed or (p)roportional?";
			cin >> response;
			StringUtil::toLowerCase(response);
			if (response == "f")
			{
				quota = ProgressiveMesh::VRQ_CONSTANT;
				cout << "\nHow many vertices should be removed at each LOD?";
			}
			else
			{
				quota = ProgressiveMesh::VRQ_PROPORTIONAL;
				cout << "\nWhat percentage of remaining vertices should be removed "
					"\at each LOD (e.g. 50)?";
			}
			cin >> reduction;
			if (quota == ProgressiveMesh::VRQ_PROPORTIONAL)
			{
				// Percentage -> parametric
				reduction = reduction * 0.01f;
			}

			cout << "\nEnter the distance for each LOD to come into effect.";

			Real distance;
			for (unsigned short iLod = 0; iLod < numLod; ++iLod)
			{
				cout << "\nLOD Level " << (iLod+1) << ":";
				cin >> distance;
				distanceList.push_back(distance);
			}
		}
		else
		{
			numLod = opts.numLods;
			quota = opts.usePercent? 
				ProgressiveMesh::VRQ_PROPORTIONAL : ProgressiveMesh::VRQ_CONSTANT;
			if (opts.usePercent)
			{
				reduction = opts.lodPercent * 0.01f;
			}
			else
			{
				reduction = opts.lodFixed;
			}
			Real currDist = 0;
			for (unsigned short iLod = 0; iLod < numLod; ++iLod)
			{
				currDist += opts.lodDist;
                Real currDistSq = Ogre::Math::Sqr(currDist);
				distanceList.push_back(currDistSq);
			}

		}

		mesh->generateLodLevels(distanceList, quota, reduction);
	}

}

void checkColour(VertexData* vdata, bool &hasColour, bool &hasAmbiguousColour,
	VertexElementType& originalType)
{
	const VertexDeclaration::VertexElementList& elemList = vdata->vertexDeclaration->getElements();
	for (VertexDeclaration::VertexElementList::const_iterator i = elemList.begin();
		i != elemList.end(); ++i)
	{
		const VertexElement& elem = *i;
		switch (elem.getType())
		{
		case VET_COLOUR:
			hasAmbiguousColour = true;
		case VET_COLOUR_ABGR:
		case VET_COLOUR_ARGB:
			hasColour = true;
			originalType = elem.getType();
		default:
			// do nothing
			;
		};
	}

}

void resolveColourAmbiguities(Mesh* mesh)
{
	// Check what we're dealing with 
	bool hasColour = false;
	bool hasAmbiguousColour = false;
	VertexElementType originalType = VET_FLOAT1;
	if (mesh->sharedVertexData)
	{
		checkColour(mesh->sharedVertexData, hasColour, hasAmbiguousColour, originalType);
	}
	for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
	{
		SubMesh* sm = mesh->getSubMesh(i);
		if (sm->useSharedVertices == false)
		{
			checkColour(sm->vertexData, hasColour, hasAmbiguousColour, originalType);
		}
	}

	String response;
	if (hasAmbiguousColour)
	{
		if (opts.srcColourFormatSet)
		{
			originalType = opts.srcColourFormat;
		}
		else
		{
			// unknown input colour, have to ask
			std::cout << "\nYour mesh has vertex colours but I don't know whether they were generated\n"
				<< "using GL or D3D ordering. Please indicate which was used when the mesh was\n"
				<< "created (type 'gl' or 'd3d').\n";
			while (response.empty())
			{
				cin >> response;
				StringUtil::toLowerCase(response);
				if (response == "d3d")
				{
					originalType = VET_COLOUR_ARGB;
				}
				else if (response == "gl")
				{
					originalType = VET_COLOUR_ABGR;
				}
				else
				{
					response = "";
				}
			}
		}
	}

	// Ask what format we want to save in
	VertexElementType desiredType = VET_FLOAT1;
	if (hasColour)
	{
		if (opts.destColourFormatSet)
		{
			desiredType = opts.destColourFormat;
		}
		else
		{
			if (opts.interactive)
			{

				response = "";
				std::cout << "\nYour mesh has vertex colours, which can be stored in one of two layouts,\n"
					<< "each of which will be slightly faster to load in a different render system.\n"
					<< "Do you want to prefer Direct3D (d3d) or OpenGL (gl)?\n";
				while (response.empty())
				{
					cin >> response;
					StringUtil::toLowerCase(response);
					if (response == "d3d")
					{
						desiredType = VET_COLOUR_ARGB;
					}
					else if (response == "gl")
					{
						desiredType = VET_COLOUR_ABGR;
					}
					else
					{
						response = "";
					}
				}
			}
			else
			{
				// 'do no harm'
				return;
			}
		}

	}

	if (mesh->sharedVertexData && hasColour)
	{
		mesh->sharedVertexData->convertPackedColour(originalType, desiredType);
	}
	for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
	{
		SubMesh* sm = mesh->getSubMesh(i);
		if (sm->useSharedVertices == false && hasColour)
		{
			sm->vertexData->convertPackedColour(originalType, desiredType);
		}
	}


}

void recalcBounds(const VertexData* vdata, AxisAlignedBox& aabb, Real& radius)
{
	const VertexElement* posElem = 
		vdata->vertexDeclaration->findElementBySemantic(VES_POSITION);
	
	const HardwareVertexBufferSharedPtr buf = vdata->vertexBufferBinding->getBuffer(
		posElem->getSource());
	void* pBase = buf->lock(HardwareBuffer::HBL_READ_ONLY);

	for (size_t v = 0; v < vdata->vertexCount; ++v)
	{
		float* pFloat;
		posElem->baseVertexPointerToElement(pBase, &pFloat);
		
		Vector3 pos(pFloat[0], pFloat[1], pFloat[2]);
		aabb.merge(pos);
		radius = std::max(radius, pos.length());

		pBase = static_cast<void*>(static_cast<char*>(pBase) + buf->getVertexSize());

	}

	buf->unlock();

}

void recalcBounds(Mesh* mesh)
{
	AxisAlignedBox aabb;
	Real radius = 0.0f;

	if (mesh->sharedVertexData)
		recalcBounds(mesh->sharedVertexData, aabb, radius);
	for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
	{
		SubMesh* sm = mesh->getSubMesh(i);
		if (!sm->useSharedVertices)
			recalcBounds(sm->vertexData, aabb, radius);
	}

	mesh->_setBounds(aabb, false);
	mesh->_setBoundingSphereRadius(radius);
}

int main(int numargs, char** args)
{
    if (numargs < 2)
    {
        help();
        return -1;
    }

	int retCode = 0;
	try 
	{
		logMgr = new LogManager();
		logMgr->createLog("OgreMeshUpgrade.log", true);
		rgm = new ResourceGroupManager();
		mth = new Math();
		lodMgr = new LodStrategyManager();
		matMgr = new MaterialManager();
		matMgr->initialise();
		skelMgr = new SkeletonManager();
		meshSerializer = new MeshSerializer();
		skeletonSerializer = new SkeletonSerializer();
		bufferManager = new DefaultHardwareBufferManager(); // needed because we don't have a rendersystem
		meshMgr = new MeshManager();
		// don't pad during upgrade
		meshMgr->setBoundsPaddingFactor(0.0f);

	    
		UnaryOptionList unOptList;
		BinaryOptionList binOptList;

		unOptList["-i"] = false;
		unOptList["-e"] = false;
		unOptList["-t"] = false;
		unOptList["-tm"] = false;
		unOptList["-tr"] = false;
		unOptList["-r"] = false;
		unOptList["-gl"] = false;
		unOptList["-d3d"] = false;
		unOptList["-srcgl"] = false;
		unOptList["-srcd3d"] = false;
		unOptList["-b"] = false;
		binOptList["-l"] = "";
		binOptList["-d"] = "";
		binOptList["-p"] = "";
		binOptList["-f"] = "";
		binOptList["-E"] = "";
		binOptList["-td"] = "";
		binOptList["-ts"] = "";

		int startIdx = findCommandLineOpts(numargs, args, unOptList, binOptList);
		parseOpts(unOptList, binOptList);

		String source(args[startIdx]);


		// Load the mesh
		struct stat tagStat;

		FILE* pFile = fopen( source.c_str(), "rb" );
		if (!pFile)
		{
			OGRE_EXCEPT(Exception::ERR_FILE_NOT_FOUND, 
				"File " + source + " not found.", "OgreMeshUpgrade");
		}
		stat( source.c_str(), &tagStat );
		MemoryDataStream* memstream = new MemoryDataStream(source, tagStat.st_size, true);
		fread( (void*)memstream->getPtr(), tagStat.st_size, 1, pFile );
		fclose( pFile );

		Mesh mesh(meshMgr, "conversion", 0, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		DataStreamPtr stream(memstream);
		meshSerializer->importMesh(stream, &mesh);

		// Write out the converted mesh
		String dest;
		if (numargs == startIdx + 2)
		{
			dest = args[startIdx + 1];
		}
		else
		{
			dest = source;
		}

		String response;

		vertexBufferReorg(mesh);

		// Deal with VET_COLOUR ambiguities
		resolveColourAmbiguities(&mesh);
		
		buildLod(&mesh);

		// Make sure we generate edge lists, provided they are not deliberately disabled
		if (!opts.suppressEdgeLists)
		{
			cout << "\nGenerating edge lists.." << std::endl;
			mesh.buildEdgeList();
		}
		else
		{
			mesh.freeEdgeList();
		}

		// Generate tangents?
		if (opts.generateTangents)
		{
			unsigned short srcTex, destTex;
			bool existing = mesh.suggestTangentVectorBuildParams(opts.tangentSemantic, srcTex, destTex);
			if (existing)
			{
				if (opts.interactive)
				{
					std::cout << "\nThis mesh appears to already have a set of tangents, " <<
						"which would suggest tangent vectors have already been calculated. Do you really " <<
						"want to generate new tangent vectors (may duplicate)? (y/n)";
					while (response == "")
					{
						cin >> response;
						StringUtil::toLowerCase(response);
						if (response == "y")
						{
							// Do nothing
						}
						else if (response == "n")
						{
							opts.generateTangents = false;
						}
						else
						{
							response = "";
						}
					}
				}
				else
				{
					// safe
					opts.generateTangents = false;
				}

			}
			if (opts.generateTangents)
			{
				cout << "Generating tangent vectors...." << std::endl;
				mesh.buildTangentVectors(opts.tangentSemantic, srcTex, destTex, 
					opts.tangentSplitMirrored, opts.tangentSplitRotated, 
					opts.tangentUseParity);
			}
		}


		if (opts.recalcBounds)
			recalcBounds(&mesh);

		meshSerializer->exportMesh(&mesh, dest, opts.endian);
    
	}
	catch (Exception& e)
	{
		cout << "Exception caught: " << e.getDescription();
		retCode = 1;
	}


    delete meshMgr;
    delete skeletonSerializer;
    delete meshSerializer;
    delete skelMgr;
    delete matMgr;
	delete lodMgr;
    delete mth;
    delete rgm;
    delete logMgr;

    return retCode;

}

