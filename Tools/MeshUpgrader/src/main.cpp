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


#include "Ogre.h"
#include "OgreMeshSerializer.h"
#include "OgreSkeletonSerializer.h"
#include "OgreDefaultHardwareBufferManager.h"
#include "OgreMeshLodGenerator.h"
#include "OgreDistanceLodStrategy.h"
#include "OgreLodStrategyManager.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgrePixelCountLodStrategy.h"
#include "OgreLodConfig.h"

#include <iostream>
#include <sys/stat.h>

using namespace std;
using namespace Ogre;

namespace {

void help(void)
{
    // Print help message
    cout << endl << "OgreMeshUpgrader: Upgrades or downgrades .mesh file versions." << endl;
    cout << "Provided for OGRE by Steve Streeting 2004-2014" << endl << endl;
    cout << "Usage: OgreMeshUpgrader [opts] sourcefile [destfile] " << endl;
    cout << "-i             = Interactive mode, prompt for options" << endl;
    cout << "-autogen       = Generate autoconfigured LOD. No more LOD options needed!" << endl;
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
    cout << "-d3d       = Convert to argb colour format" << endl;
    cout << "-gl        = Convert to abgr colour format" << endl;
    cout << "-byte      = Convert to byte colour format" << endl;
    cout << "-srcd3d    = Interpret ambiguous colours as D3D style" << endl;
    cout << "-srcgl     = Interpret ambiguous colours as GL style" << endl;
    cout << "-E endian  = Set endian mode 'big' 'little' or 'native' (default)" << endl;
    cout << "-b         = Recalculate bounding box (static meshes only)" << endl;
    cout << "-V version = Specify OGRE version format to write instead of latest" << endl;
    cout << "             Options are: 1.10, 1.8, 1.7, 1.4, 1.0" << endl;
    cout << "sourcefile = name of file to convert" << endl;
    cout << "destfile   = optional name of file to write to. If you don't" << endl;
    cout << "             specify this OGRE overwrites the existing file." << endl;

    cout << endl;
}

struct UpgradeOptions {
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
    bool lodAutoconfigure;
    unsigned short numLods;
    Real lodDist;
    Real lodPercent;
    size_t lodFixed;
    bool usePercent;
    Serializer::Endian endian;
    bool recalcBounds;
    MeshVersion targetVersion;

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

    opts.lodAutoconfigure = false;
    opts.lodDist = 500;
    opts.lodFixed = 0;
    opts.lodPercent = 20;
    opts.numLods = 0;
    opts.usePercent = true;
    opts.recalcBounds = false;
    opts.targetVersion = MESH_VERSION_LATEST;

    opts.suppressEdgeLists = unOpts["-e"];
    opts.generateTangents = unOpts["-t"];
    opts.tangentSplitMirrored = unOpts["-tm"];
    opts.tangentSplitRotated = unOpts["-tr"];
    opts.lodAutoconfigure = unOpts["-autogen"];
    opts.interactive = unOpts["-i"];
    opts.dontReorganise = unOpts["-r"];

    if (unOpts["-d3d"]) {
        opts.destColourFormatSet = true;
        opts.destColourFormat = VET_COLOUR_ARGB;
    }
    if (unOpts["-gl"]) {
        opts.destColourFormatSet = true;
        opts.destColourFormat = VET_COLOUR_ABGR;
    }
    if (unOpts["-byte"]) {
        opts.destColourFormatSet = true;
        opts.destColourFormat = VET_UBYTE4_NORM;
    }
    if (unOpts["-srcd3d"]) {
        opts.srcColourFormatSet = true;
        opts.srcColourFormat = VET_COLOUR_ARGB;
    }
    if (unOpts["-srcgl"]) {
        opts.srcColourFormatSet = true;
        opts.srcColourFormat = VET_COLOUR_ABGR;
    }
    if (unOpts["-b"]) {
        opts.recalcBounds = true;
    }

    BinaryOptionList::iterator bi = binOpts.find("-l");
    if (!bi->second.empty()) {
        opts.numLods = StringConverter::parseInt(bi->second);
    }

    bi = binOpts.find("-d");
    if (!bi->second.empty()) {
        opts.lodDist = StringConverter::parseReal(bi->second);
    }

    bi = binOpts.find("-p");
    if (!bi->second.empty()) {
        opts.lodPercent = StringConverter::parseReal(bi->second);
        opts.usePercent = true;
    }


    bi = binOpts.find("-f");
    if (!bi->second.empty()) {
        opts.lodFixed = StringConverter::parseInt(bi->second);
        opts.usePercent = false;
    }

    bi = binOpts.find("-E");
    if (!bi->second.empty()) {
        if (bi->second == "big") {
            opts.endian = Serializer::ENDIAN_BIG;
        } else if (bi->second == "little") {
            opts.endian = Serializer::ENDIAN_LITTLE;
        } else {
            opts.endian = Serializer::ENDIAN_NATIVE;
    }
    }
    bi = binOpts.find("-td");
    if (!bi->second.empty()) {
        if (bi->second == "uvw") {
            opts.tangentSemantic = VES_TEXTURE_COORDINATES;
        } else { // if (bi->second == "tangent"), or anything else
            opts.tangentSemantic = VES_TANGENT;
    }
    }
    bi = binOpts.find("-ts");
    if (!bi->second.empty()) {
        if (bi->second == "4") {
            opts.tangentUseParity = true;
    }
    }
    
    bi = binOpts.find("-V");
    if (!bi->second.empty()) {
        if (bi->second == "1.10") {
            opts.targetVersion = MESH_VERSION_1_10;
        } else if (bi->second == "1.8") {
            opts.targetVersion = MESH_VERSION_1_8;
        } else if (bi->second == "1.7") {
            opts.targetVersion = MESH_VERSION_1_7;
        } else if (bi->second == "1.4") {
            opts.targetVersion = MESH_VERSION_1_4;
        } else if (bi->second == "1.0") {
            opts.targetVersion = MESH_VERSION_1_0;
        } else {
            logMgr->logError("Unrecognised target mesh version '" + bi->second + "'");
        }
    }
    
}

String describeSemantic(VertexElementSemantic sem)
{
    switch (sem) {
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
    }
    return "";
}
void displayVertexBuffers(VertexDeclaration::VertexElementList& elemList)
{
    // Iterate per buffer
    unsigned short currentBuffer = 999;
    unsigned short elemNum = 0;
    VertexDeclaration::VertexElementList::iterator i, iend;
    iend = elemList.end();
    for (i = elemList.begin(); i != iend; ++i) {
        if (i->getSource() != currentBuffer) {
            currentBuffer = i->getSource();
            cout << "> Buffer " << currentBuffer << ":" << endl;
        }
        cout << "   - Element " << elemNum++ << ": " << describeSemantic(i->getSemantic());
        if (i->getSemantic() == VES_TEXTURE_COORDINATES) {
            cout << " (index " << i->getIndex() << ")"; 
        }
        cout << endl;

    }
}
// Sort routine for VertexElement
bool vertexElementLess(const VertexElement& e1, const VertexElement& e2)
{
    // Sort by source first
    if (e1.getSource() < e2.getSource()) {
        return true;
    } else if (e1.getSource() == e2.getSource()) {
        // Use ordering of semantics to sort
        if (e1.getSemantic() < e2.getSemantic()) {
            return true;
        } else if (e1.getSemantic() == e2.getSemantic()) {
            // Use index to sort
            if (e1.getIndex() < e2.getIndex()) {
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
    for (i = origElems.begin(); i != iend; ++i) {
        elemList->push_back(*i);
    }
    elemList->sort(VertexDeclaration::vertexElementLess);
}
// Utility function to allow the user to modify the layout of vertex buffers.
void reorganiseVertexBuffers(const String& desc, Mesh& mesh, SubMesh* sm, VertexData* vertexData)
{
    cout << endl << desc << ":- " << endl;
    // Copy elements into a list
    VertexDeclaration::VertexElementList elemList;
    copyElems(vertexData->vertexDeclaration, &elemList);

    bool finish = false;
    bool anyChanges = false;
    while (!finish) {
        displayVertexBuffers(elemList);
        cout << endl;

        cout << "Options: (a)utomatic" << endl;
        cout << "         (m)ove element" << endl;
        cout << "         (d)elete element" << endl;
        cout << "         (r)eset" << endl;
        cout << "         (f)inish" << endl;
        String response = "";
        while (response.empty()) {
            cin >> response;
            StringUtil::toLowerCase(response);

            if (response == "m") {
                String moveResp;
                cout << "Which element do you want to move (type number): ";
                cin >> moveResp;
                if (!moveResp.empty()) {
                    int eindex = StringConverter::parseInt(moveResp);
                    VertexDeclaration::VertexElementList::iterator movei = elemList.begin();
                    std::advance(movei, eindex);
                    cout << endl << "Move element " << eindex << "(" + describeSemantic(movei->getSemantic()) <<
                    ") to which buffer: ";
                    cin >> moveResp;
                    if (!moveResp.empty()) {
                        int bindex = StringConverter::parseInt(moveResp);
                        // Move (note offset will be wrong)
                        *movei = VertexElement(bindex, 0, movei->getType(),
                            movei->getSemantic(), movei->getIndex());
                        elemList.sort(vertexElementLess);
                        anyChanges = true;
                                
                    }
                }
            } else if (response == "a") {
                // Automatic
                VertexDeclaration* newDcl = 
                    vertexData->vertexDeclaration->getAutoOrganisedDeclaration(
                        mesh.hasSkeleton(), mesh.hasVertexAnimation(), 
                        sm ? sm->getVertexAnimationIncludesNormals() : mesh.getSharedVertexDataAnimationIncludesNormals());
                copyElems(newDcl, &elemList);
                HardwareBufferManager::getSingleton().destroyVertexDeclaration(newDcl);
                anyChanges = true;

            } else if (response == "d") {
                String moveResp;
                cout << "Which element do you want to delete (type number): ";
                cin >> moveResp;
                if (!moveResp.empty()) {
                    int eindex = StringConverter::parseInt(moveResp);
                    VertexDeclaration::VertexElementList::iterator movei = elemList.begin();
                    std::advance(movei, eindex);
                    cout << std::endl << "Delete element " << eindex << "(" + describeSemantic(movei->getSemantic()) << ")?: ";
                    cin >> moveResp;
                    StringUtil::toLowerCase(moveResp);
                    if (moveResp == "y") {
                        elemList.erase(movei);
                        anyChanges = true;
                    }
                }
            } else if (response == "r") {
                // reset
                copyElems(vertexData->vertexDeclaration, &elemList);
                anyChanges = false;
            } else if (response == "f") {
                // finish
                finish = true;
            } else {
                std::cout << "Wrong answer!\n";
                response = "";
            }
            
        }
    }

    if (anyChanges) {
        String response;
        while (response.empty()) {
            displayVertexBuffers(elemList);
            cout << "Really reorganise the vertex buffers this way? ";
            cin >> response;
            StringUtil::toLowerCase(response);
            if (response == "y") {
                VertexDeclaration* newDecl = HardwareBufferManager::getSingleton().createVertexDeclaration();
                VertexDeclaration::VertexElementList::iterator i, iend;
                iend = elemList.end();
                unsigned short currentBuffer = 999;
                size_t offset = 0;
                for (i = elemList.begin(); i != iend; ++i) {
                    // Calc offsets since reorg changes them
                    if (i->getSource() != currentBuffer) {
                        offset = 0;
                        currentBuffer = i->getSource();
                    }
                    offset += newDecl
                                  ->addElement(currentBuffer, offset, i->getType(), i->getSemantic(),
                                               i->getIndex())
                                  .getSize();
                }
                // Usages don't matter here since we're onlly exporting
                BufferUsageList bufferUsages;
                for (size_t u = 0; u <= newDecl->getMaxSource(); ++u) {
                    bufferUsages.push_back(HardwareBuffer::HBU_STATIC_WRITE_ONLY);
                }
                vertexData->reorganiseBuffers(newDecl, bufferUsages);
            } else if (response == "n") {
                // do nothing
            } else {
                std::cout << "Wrong answer!\n";
                response = "";
            }
        }
        
    }
        

}
// Utility function to allow the user to modify the layout of vertex buffers.
void reorganiseVertexBuffers(Mesh& mesh)
{
    // Make sure animation types up to date
    mesh._determineAnimationTypes();

    if (mesh.sharedVertexData) {
        if (opts.interactive) {
            reorganiseVertexBuffers("Shared Geometry", mesh, 0, mesh.sharedVertexData);
        } else {
            // Automatic
            VertexDeclaration* newDcl = 
                mesh.sharedVertexData->vertexDeclaration->getAutoOrganisedDeclaration(
                mesh.hasSkeleton(), mesh.hasVertexAnimation(), mesh.getSharedVertexDataAnimationIncludesNormals());
            if (*newDcl != *(mesh.sharedVertexData->vertexDeclaration)) {
                // Usages don't matter here since we're onlly exporting
                BufferUsageList bufferUsages;
                for (size_t u = 0; u <= newDcl->getMaxSource(); ++u) {
                    bufferUsages.push_back(HardwareBuffer::HBU_STATIC_WRITE_ONLY);
                }
                mesh.sharedVertexData->reorganiseBuffers(newDcl, bufferUsages);
            }

        }
    }

    for (size_t i = 0; i < mesh.getNumSubMeshes(); i++)
    {
        SubMesh* sm = mesh.getSubMesh(i);
        if (!sm->useSharedVertices) {
            if (opts.interactive) {
                StringStream str;
                str << "SubMesh " << i;
                reorganiseVertexBuffers(str.str(), mesh, sm, sm->vertexData);
            } else {
                const bool hasVertexAnim = sm->getVertexAnimationType() != Ogre::VAT_NONE;

                // Automatic
                VertexDeclaration* newDcl = 
                    sm->vertexData->vertexDeclaration->getAutoOrganisedDeclaration(
                    mesh.hasSkeleton(), hasVertexAnim, sm->getVertexAnimationIncludesNormals() );
                if (*newDcl != *(sm->vertexData->vertexDeclaration)) {
                    // Usages don't matter here since we're onlly exporting
                    BufferUsageList bufferUsages;
                    for (size_t u = 0; u <= newDcl->getMaxSource(); ++u) {
                        bufferUsages.push_back(HardwareBuffer::HBU_STATIC_WRITE_ONLY);
                    }
                    sm->vertexData->reorganiseBuffers(newDcl, bufferUsages);
                }
                
            }
        }
    }
}


void vertexBufferReorg(Mesh& mesh)
{
    String response;

    if (opts.interactive) {

        // Check to see whether we would like to reorganise vertex buffers
        std::cout << "\nWould you like to reorganise the vertex buffers for this mesh? ";
        while (response.empty()) {
            cin >> response;
            StringUtil::toLowerCase(response);
            if (response == "y") {
                reorganiseVertexBuffers(mesh);
            } else if (response == "n") {
                // Do nothing
            } else {
                std::cout << "Wrong answer!\n";
                response = "";
            }
        }
    } else if (!opts.dontReorganise) {
                reorganiseVertexBuffers(mesh);
            }

}

void recalcBounds(const VertexData* vdata, AxisAlignedBox& aabb, Real& radius)
{
    const VertexElement* posElem = 
        vdata->vertexDeclaration->findElementBySemantic(VES_POSITION);
    
    const HardwareVertexBufferSharedPtr buf = vdata->vertexBufferBinding->getBuffer(
        posElem->getSource());
    void* pBase = buf->lock(HardwareBuffer::HBL_READ_ONLY);

    for (size_t v = 0; v < vdata->vertexCount; ++v) {
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

    if (mesh->sharedVertexData) {
        recalcBounds(mesh->sharedVertexData, aabb, radius);
    }
    for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i) {
        SubMesh* sm = mesh->getSubMesh(i);
        if (!sm->useSharedVertices) {
            recalcBounds(sm->vertexData, aabb, radius);
    }
    }

    mesh->_setBounds(aabb, false);
    mesh->_setBoundingSphereRadius(radius);
}

void printLodConfig(const LodConfig& lodConfig)
{
    cout << "\n\nLOD config summary:";
    cout << "\n  lodConfig.strategy=" << lodConfig.strategy->getName();
    String reductionMethod("Unknown");
    if (lodConfig.levels[0].reductionMethod == LodLevel::VRM_PROPORTIONAL) {
        reductionMethod = "VRM_PROPORTIONAL";
    } else if (lodConfig.levels[0].reductionMethod == LodLevel::VRM_CONSTANT) {
        reductionMethod = "VRM_PROPORTIONAL";
    } else if (lodConfig.levels[0].reductionMethod == LodLevel::VRM_COLLAPSE_COST) {
        reductionMethod = "VRM_COLLAPSE_COST";
    }
    String distQuantity;
    if (lodConfig.strategy == PixelCountLodStrategy::getSingletonPtr()) {
        distQuantity = "px";
    }
    for (unsigned short i = 0; i < lodConfig.levels.size(); i++) {
        const LodLevel& lodLevel = lodConfig.levels[i];
        cout << "\n  lodConfig.levels[" << i << "].distance=" << lodLevel.distance << distQuantity;
        cout << "\n  lodConfig.levels[" << i << "].reductionMethod=" <<
        (lodLevel.manualMeshName.empty() ? reductionMethod : "N/A");
        cout << "\n  lodConfig.levels[" << i << "].reductionValue=" <<
        (lodLevel.manualMeshName.empty() ? StringConverter::toString(lodLevel.reductionValue) : "N/A");
        cout << "\n  lodConfig.levels[" << i << "].manualMeshName=" <<
        (lodLevel.manualMeshName.empty() ? "N/A" : lodLevel.manualMeshName);
    }
}
size_t getUniqueVertexCount(MeshPtr mesh)
{

    // The vertex buffer contains the same vertex position multiple times.
    // To get the count of the vertices, which has unique positions, we can use progressive mesh.
    // It is constructing a mesh grid at the beginning, so if we reduce 0%, we will get the unique vertex count.
    LodConfig lodConfig(mesh, PixelCountLodStrategy::getSingletonPtr());
    lodConfig.advanced.useBackgroundQueue = false; // Non-threaded
    lodConfig.createGeneratedLodLevel(0.0f, 0.0f);
    MeshLodGenerator().generateLodLevels(lodConfig);
    return lodConfig.levels[0].outUniqueVertexCount;
}
void buildLod(MeshPtr& mesh)
{
    String response;

    // Prompt for LOD generation?
    bool genLod = (opts.numLods != 0 || opts.interactive || opts.lodAutoconfigure);
    bool askLodDtls = opts.interactive;
    if (genLod) { // otherwise only ask if not specified on command line
        if (mesh->getNumLodLevels() > 1) {
            do {
                std::cout << "\nMesh already contains level-of-detail information.\n"
                             "Do you want to: (u)se it, (r)eplace it, or (d)rop it? ";
                cin >> response;
                StringUtil::toLowerCase(response);
                if (response == "u") {
                    genLod = false;
                    askLodDtls = false;
                } else if (response == "d") {
                    mesh->removeLodLevels();
                    genLod = false;
                    askLodDtls = false;
                } else if (response == "r") {
                    genLod = true;
                } else {
                    std::cout << "Wrong answer!\n";
                    response = "";
                }
            } while (response == "");
        } else if (askLodDtls) {
            do {
                std::cout << "\nWould you like to generate level-of-detail information? (y/n) ";
                cin >> response;
                StringUtil::toLowerCase(response);
                if (response == "n") {
                    genLod = false;
                } else if (response == "y") {
                    genLod = true;
                } else {
                    std::cout << "Wrong answer!\n";
                    response = "";
                }
            } while (response == "");
        }
    }

    if (!genLod) {
        return;
    }

    int numLod;
    LodConfig lodConfig;
    lodConfig.mesh = mesh;
    lodConfig.strategy = DistanceLodBoxStrategy::getSingletonPtr();
    if (askLodDtls) {
        do {
            std::cout <<
            "\nDo you want to (m)anually configure or (a)utoconfigure it?\nautoconfigure=no more questions asked!(m/a) ";
            cin >> response;
            StringUtil::toLowerCase(response);
            if (response == "a") {
                opts.lodAutoconfigure = true;
            } else if (response == "m") {
                opts.lodAutoconfigure = false;
            } else {
                std::cout << "Wrong answer!\n";
                response = "";
            }
        } while (response == "");
        if (!opts.lodAutoconfigure) {
            do {
                std::cout << "\nDo you want to use (p)ixels or (d)istance to determine where the LOD activates? (p/d) ";
                cin >> response;
                StringUtil::toLowerCase(response);
                if (response == "p") {
                    lodConfig.strategy = PixelCountLodStrategy::getSingletonPtr();
                } else if (response == "d") {
                    lodConfig.strategy = DistanceLodBoxStrategy::getSingletonPtr();
                } else {
                    std::cout << "Wrong answer!\n";
                    response = "";
                }
            } while (response == "");
            LodLevel lodLevel;
            size_t vertexCount = 0;
            do {
                cout <<
                "\nWhat unit of reduction would you like to use(fixed=constant vertex number; proportional=percentage):" <<
                "\n(f)ixed or (p)roportional? ";
                cin >> response;
                StringUtil::toLowerCase(response);
                if (response == "f") {
                    lodLevel.reductionMethod = LodLevel::VRM_CONSTANT;
                    vertexCount = getUniqueVertexCount(mesh);
                } else if (response == "p") {
                    lodLevel.reductionMethod = LodLevel::VRM_PROPORTIONAL;
                } else {
                    std::cout << "Wrong answer!\n";
                    response = "";
                }
            } while (response == "");

            numLod = 0;
            while (numLod < 1 || numLod > 99) {
                cout << "\nHow many extra LOD levels would you like to generate? (1-99)";
                cin >> response;
                numLod = StringConverter::parseInt(response);
            }
            Real minReduction = 0.0;
            Real minDistance = lodConfig.strategy->getBaseValue();
            for (int iLod = 0; iLod < numLod; ++iLod) {
                do {
                    cout << "\nShould LOD" << (iLod + 1) << " be a (m)anual or (g)enerated LOD level? (m/g)";
                    cin >> response;
                    StringUtil::toLowerCase(response);
                    if (response == "m") {
                        while (lodLevel.manualMeshName.empty()) {
                            cout << "\nEnter the mesh name to show for LOD" << (iLod + 1) << ". (e.g. Sinbad_LOD1.mesh)";
                            cout << "\nIt should have the same animation states and it should not have LOD levels!\n";
                            cin >> lodLevel.manualMeshName;
                        }
                    } else if (response == "g") {
                        if (lodLevel.reductionMethod == LodLevel::VRM_PROPORTIONAL) {
                            cout << "\nWhat percentage of remaining vertices should be removed at LOD" <<
                            (iLod + 1) << "? (e.g. 40 = remove 40% of vertices)";
                            cout << "\nIt should be between " << minReduction << "%-100%\n";
                        } else {
                            cout << "\nHow many vertices should be removed at LOD" <<
                            (iLod + 1) << "? (e.g. 50 = remove 50 vertices)";
                            cout << "\nIt should be between " << (int) minReduction << "-" << vertexCount << " vertices\n";
                        }
                        while (1) {
                            cin >> response;
                            lodLevel.reductionValue = StringConverter::parseReal(response);
                            if (lodLevel.reductionMethod == LodLevel::VRM_PROPORTIONAL) {
                                if (lodLevel.reductionValue > minReduction && lodLevel.reductionValue <= 100.0) {
                                    minReduction = lodLevel.reductionValue;
                                    lodLevel.reductionValue *= 0.01;
                                    if (minReduction == 100.0) {
                                        cout << "\nCan't insert more LOD levels!";
                                        iLod = numLod;
                                    }
                                    break;
                                } else {
                                    cout << "\nWrong answer! It should be between " << minReduction << "%-100%.";
                                }
                            } else {
                                if ((int) lodLevel.reductionValue > (int) minReduction && (int) lodLevel.reductionValue <= (int)vertexCount) {
                                    minReduction = lodLevel.reductionValue;
                                    break;
                                } else {
                                    cout << "\nIt should be between " << (int) minReduction << "-" << vertexCount << " vertices.";
                                }
                            }
                        }

                    } else {
                        std::cout << "Wrong answer!\n";
                        response = "";
                    }
                } while (response == "");

                if (lodConfig.strategy == DistanceLodBoxStrategy::getSingletonPtr()) {
                    cout << "\nEnter the distance for LOD" <<
                    (iLod + 1) << " to activate. (distance of camera and mesh in Ogre units)";
                    cout << "\nIt should be more then last Lod level: " << minDistance << "\n";
                } else {
                    cout << "\nEnter the pixel count for LOD" <<
                    (iLod + 1) << " to activate. (pixels of the bounding sphere in the render output)";
                    cout << "\nIt should be LESS then last Lod level: " << minDistance << "\n";
                }
                while (1) {
                    cin >> response;
                    lodLevel.distance = StringConverter::parseReal(response);
                    if (lodConfig.strategy == DistanceLodBoxStrategy::getSingletonPtr()) {
                        if (lodLevel.distance > minDistance) {
                            break;
                        } else {
                            cout << "\nWrong answer! It should be more than " << minDistance << ".";
                        }
                    } else {
                        if (lodLevel.distance < minDistance && lodLevel.distance >= 0) {
                            break;
                        } else {
                            cout << "\nWrong answer! It should be less than " << minDistance << "px.";
                        }
                    }
                }
                minDistance = lodLevel.distance;
                cout << "\nLOD" << (iLod + 1) << " level summary:";
                cout << "\n    lodLevel.distance=" << lodLevel.distance;
                String reductionMethod =
                    ((lodLevel.reductionMethod == LodLevel::VRM_PROPORTIONAL) ? "VRM_PROPORTIONAL" : "VRM_CONSTANT");
                cout << "\n    lodLevel.reductionMethod=" << (lodLevel.manualMeshName.empty() ? reductionMethod : "N/A");
                cout << "\n    lodLevel.reductionValue=" <<
                (lodLevel.manualMeshName.empty() ? StringConverter::toString(lodLevel.reductionValue) : "N/A");
                cout << "\n    lodLevel.manualMeshName=" << (lodLevel.manualMeshName.empty() ? "N/A" : lodLevel.manualMeshName);
                lodConfig.levels.push_back(lodLevel);
            }
        }
    } else {
        // not interactive: read parameters from console
        numLod = opts.numLods;
        LodLevel lodLevel;
        lodLevel.distance = 0.0;
        for (unsigned short iLod = 0; iLod < numLod; ++iLod) {

            lodLevel.reductionMethod = opts.usePercent ?
                                       LodLevel::VRM_PROPORTIONAL : LodLevel::VRM_CONSTANT;
            if (opts.usePercent) {
                lodLevel.reductionValue += opts.lodPercent * 0.01f;
            } else {
                lodLevel.reductionValue += (Ogre::Real)opts.lodFixed;
            }

            lodLevel.distance += opts.lodDist;
            lodConfig.levels.push_back(lodLevel);
        }
    }

    // ensure we use correct bounds
    recalcBounds(mesh.get());

    MeshLodGenerator gen;
    if (opts.lodAutoconfigure) {
        // In this case we ignore other settings
        gen.getAutoconfig(mesh, lodConfig);
    }
    printLodConfig(lodConfig);


    cout << "\n\nGenerating LOD levels...";
    gen.generateLodLevels(lodConfig);
    cout << "success\n";
}

void checkColour(VertexData* vdata, bool& hasColour, bool& hasAmbiguousColour,
                 VertexElementType& originalType)
{
    const VertexDeclaration::VertexElementList& elemList = vdata->vertexDeclaration->getElements();
    for (VertexDeclaration::VertexElementList::const_iterator i = elemList.begin();
         i != elemList.end(); ++i) {
        const VertexElement& elem = *i;
        switch (elem.getType()) {
        case VET_COLOUR:
            hasAmbiguousColour = true;
            OGRE_FALLTHROUGH;
        case VET_COLOUR_ABGR:
        case VET_COLOUR_ARGB:
            hasColour = true;
            originalType = elem.getType();

        default:
            // do nothing
            ;
        }
    }

}

void resolveColourAmbiguities(Mesh* mesh)
{
    // Check what we're dealing with
    bool hasColour = false;
    bool hasAmbiguousColour = false;
    VertexElementType originalType = VET_FLOAT1;
    if (mesh->sharedVertexData) {
        checkColour(mesh->sharedVertexData, hasColour, hasAmbiguousColour, originalType);
    }
    for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i) {
        SubMesh* sm = mesh->getSubMesh(i);
        if (sm->useSharedVertices == false) {
            checkColour(sm->vertexData, hasColour, hasAmbiguousColour, originalType);
        }
    }

    String response;
    if (hasAmbiguousColour) {
        if (opts.srcColourFormatSet) {
            originalType = opts.srcColourFormat;
        } else {
            // unknown input colour, have to ask
            std::cout   << "\nYour mesh has vertex colours but I don't know whether they were generated\n"
                        << "using GL or D3D ordering. Please indicate which was used when the mesh was\n"
                        << "created (type 'gl' or 'd3d').\n";
            while (response.empty()) {
                cin >> response;
                StringUtil::toLowerCase(response);
                if (response == "d3d") {
                    originalType = VET_COLOUR_ARGB;
                } else if (response == "gl") {
                    originalType = VET_COLOUR_ABGR;
                } else {
                    std::cout << "Wrong answer!\n";
                    response = "";
                }
            }
        }
    }

    // Ask what format we want to save in
    VertexElementType desiredType = VET_FLOAT1;
    if (hasColour) {
        if (opts.destColourFormatSet) {
            desiredType = opts.destColourFormat;
        } else {
            if (opts.interactive) {

                response = "";
                std::cout   << "\nYour mesh has vertex colours, which can be stored in one of two layouts,\n"
                            << "each of which will be slightly faster to load in a different render system.\n"
                            << "Do you want to prefer Direct3D (d3d) or OpenGL (gl)?\n";
                while (response.empty()) {
                    cin >> response;
                    StringUtil::toLowerCase(response);
                    if (response == "d3d") {
                        desiredType = VET_COLOUR_ARGB;
                    } else if (response == "gl") {
                        desiredType = VET_COLOUR_ABGR;
                    } else {
                        std::cout << "Wrong answer!\n";
                        response = "";
                    }
                }
            } else {
                // 'do no harm'
                return;
            }
        }

    }

    if (mesh->sharedVertexData && hasColour) {
        mesh->sharedVertexData->convertPackedColour(originalType, desiredType);
    }
    for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i) {
        SubMesh* sm = mesh->getSubMesh(i);
        if (sm->useSharedVertices == false && hasColour) {
            sm->vertexData->convertPackedColour(originalType, desiredType);
        }
    }


}

struct MaterialCreator : public MeshSerializerListener
{
    void processMaterialName(Mesh *mesh, String *name)
    {
        // create material because we do not load any .material files
        MaterialManager::getSingleton().createOrRetrieve(*name, mesh->getGroup());
    }

    void processSkeletonName(Mesh *mesh, String *name) {}
    void processMeshCompleted(Mesh *mesh) {}
};
}

int main(int numargs, char** args)
{
    if (numargs < 2) {
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
        MaterialCreator matCreator;
        meshSerializer->setListener(&matCreator);
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
        unOptList["-byte"] = false;
        unOptList["-srcgl"] = false;
        unOptList["-srcd3d"] = false;
        unOptList["-autogen"] = false;
        unOptList["-b"] = false;
        binOptList["-l"] = "";
        binOptList["-d"] = "";
        binOptList["-p"] = "";
        binOptList["-f"] = "";
        binOptList["-E"] = "";
        binOptList["-td"] = "";
        binOptList["-ts"] = "";
        binOptList["-V"] = "";

        int startIdx = findCommandLineOpts(numargs, args, unOptList, binOptList);
        parseOpts(unOptList, binOptList);

        String source(args[startIdx]);


        // Load the mesh
        struct stat tagStat;

        FILE* pFile = fopen( source.c_str(), "rb" );
        if (!pFile) {
            OGRE_EXCEPT(Exception::ERR_FILE_NOT_FOUND, 
                "File " + source + " not found.", "OgreMeshUpgrade");
        }
        stat( source.c_str(), &tagStat );
        MemoryDataStream* memstream = new MemoryDataStream(source, tagStat.st_size, true);
        size_t result = fread( (void*)memstream->getPtr(), 1, tagStat.st_size, pFile );
        if (result != size_t(tagStat.st_size))
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                "Unexpected error while reading file " + source, "OgreMeshUpgrade");
        fclose( pFile );

        MeshPtr meshPtr = MeshManager::getSingleton().createManual("conversion",
                                                                   ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        Mesh* mesh = meshPtr.get();

        DataStreamPtr stream(memstream);
        meshSerializer->importMesh(stream, mesh);

        // Write out the converted mesh
        String dest;
        if (numargs == startIdx + 2) {
            dest = args[startIdx + 1];
        } else {
            dest = source;
        }

        String response;

        vertexBufferReorg(*mesh);

        // Deal with VET_COLOUR ambiguities
        resolveColourAmbiguities(mesh);
        
        buildLod(meshPtr);

        if (opts.interactive) {
            do {
                std::cout << "\nWould you like to (b)uild/(r)emove/(k)eep Edge lists? (b/r/k) ";
                cin >> response;
                StringUtil::toLowerCase(response);
                if (response == "k") {
                    // Do nothing
                } else if (response == "b") {
                    cout << "\nGenerating edge lists...";
                    mesh->buildEdgeList();
                    cout << "success\n";
                } else if (response == "r") {
                    mesh->freeEdgeList();
                } else {
                    std::cout << "Wrong answer!\n";
                    response = "";
                }
            } while (response == "");
        } else {
        // Make sure we generate edge lists, provided they are not deliberately disabled
            if (!opts.suppressEdgeLists) {
                cout << "\nGenerating edge lists...";
                mesh->buildEdgeList();
                cout << "success\n";
            } else {
                mesh->freeEdgeList();
        }
        }
        if (opts.interactive) {
            do {
                std::cout << "\nWould you like to (g)enerate/(k)eep tangent buffer? (g/k) ";
                cin >> response;
                StringUtil::toLowerCase(response);
                if (response == "k") {
                    opts.generateTangents = false;
                } else if (response == "g") {
                    opts.generateTangents = true;
                } else {
                    std::cout << "Wrong answer!\n";
                    response = "";
                }
            } while (response == "");
        }
        // Generate tangents?
        if (opts.generateTangents) {
            unsigned short srcTex, destTex;
            bool existing = mesh->suggestTangentVectorBuildParams(opts.tangentSemantic, srcTex, destTex);
            if (existing) {
                if (opts.interactive) {
                    do {
                    std::cout << "\nThis mesh appears to already have a set of tangents, " <<
                        "which would suggest tangent vectors have already been calculated. Do you really " <<
                        "want to generate new tangent vectors (may duplicate)? (y/n) ";
                        cin >> response;
                        StringUtil::toLowerCase(response);
                        if (response == "y") {
                            // Do nothing
                        } else if (response == "n") {
                            opts.generateTangents = false;
                        } else {
                            std::cout << "Wrong answer!\n";
                            response = "";
                        }

                    } while (response == "");
                } else {
                    // safe
                    opts.generateTangents = false;
                }

            }
            if (opts.generateTangents) {
                cout << "\nGenerating tangent vectors....";
                mesh->buildTangentVectors(opts.tangentSemantic, srcTex, destTex,
                    opts.tangentSplitMirrored, opts.tangentSplitRotated, 
                    opts.tangentUseParity);
                cout << "success" << std::endl;
            }
        }


        if (opts.recalcBounds) {
            recalcBounds(mesh);
        }

        meshSerializer->exportMesh(mesh, dest, opts.targetVersion, opts.endian);
    
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
