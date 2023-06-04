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
#include "OgreDefaultHardwareBufferManager.h"
#include "OgreMeshLodGenerator.h"
#include "OgreDistanceLodStrategy.h"
#include "OgreLodStrategyManager.h"
#include "OgrePixelCountLodStrategy.h"
#include "OgreLodConfig.h"

#include <iostream>
#include <sys/stat.h>

using namespace std;
using namespace Ogre;

namespace {

void help(void)
{
    cout <<
R"HELP(Usage: OgreMeshUpgrader [opts] sourcefile [destfile]

  Upgrades or downgrades .mesh file versions.

-pack          = Pack normals and tangents as int_10_10_10_2
-autogen       = Generate autoconfigured LOD. No LOD options needed
-l lodlevels   = number of LOD levels
-d loddist     = distance increment to reduce LOD
-p lodpercent  = Percentage triangle reduction amount per LOD
-f lodnumtris  = Fixed vertex reduction per LOD
-el            = generate edge lists (for stencil shadows)
-t             = Generate tangents (for normal mapping)
-ts [3|4]      = Tangent size (4 includes parity, default: 3)
-tm            = Split tangent vertices at UV mirror points
-tr            = Split tangent vertices where basis is rotated > 90 degrees
-r             = DON'T reorganise buffers to recommended format
-E endian      = Set endian mode 'big' 'little' or 'native' (default)
-b             = Recalculate bounding box (static meshes only)
-V version     = Specify OGRE version format to write instead of latest
                 Options are: 1.10, 1.8, 1.7, 1.4, 1.0
-log filename  = name of the log file (default: 'OgreMeshUpgrader.log')
sourcefile     = name of file to convert
destfile       = optional name of file to write to. If you don't
                 specify this OGRE overwrites the existing file.
)HELP";
}

struct UpgradeOptions {
    bool generateEdgeLists;
    bool generateTangents;
    bool tangentUseParity;
    bool tangentSplitMirrored;
    bool tangentSplitRotated;
    bool dontReorganise;
    bool lodAutoconfigure;
    bool packNormalsTangents;
    unsigned short numLods;
    Real lodDist;
    Real lodPercent;
    size_t lodFixed;
    bool usePercent;
    Serializer::Endian endian;
    bool recalcBounds;
    MeshVersion targetVersion;
    String logFile;
};

UpgradeOptions parseOpts(UnaryOptionList& unOpts, BinaryOptionList& binOpts)
{
    UpgradeOptions opts;

    // Defaults
    opts.generateEdgeLists = false;
    opts.generateTangents = false;
    opts.tangentUseParity = false;
    opts.tangentSplitMirrored = false;
    opts.tangentSplitRotated = false;
    opts.dontReorganise = false;
    opts.endian = Serializer::ENDIAN_NATIVE;

    opts.packNormalsTangents = false;

    opts.lodAutoconfigure = false;
    opts.lodDist = 500;
    opts.lodFixed = 0;
    opts.lodPercent = 20;
    opts.numLods = 0;
    opts.usePercent = true;
    opts.recalcBounds = false;
    opts.targetVersion = MESH_VERSION_LATEST;

    opts.generateEdgeLists = unOpts["-el"];
    opts.generateTangents = unOpts["-t"];
    opts.tangentSplitMirrored = unOpts["-tm"];
    opts.tangentSplitRotated = unOpts["-tr"];
    opts.lodAutoconfigure = unOpts["-autogen"];
    opts.dontReorganise = unOpts["-r"];
    opts.packNormalsTangents = unOpts["-pack"];

    // Unary options (true/false options that don't take a parameter)
    if (unOpts["-b"]) {
        opts.recalcBounds = true;
    }

    // Binary options (options that take a parameter)
    BinaryOptionList::iterator bi = binOpts.find("-l");
    if (!bi->second.empty()) {
        opts.numLods = StringConverter::parseInt(bi->second);
    }

    bi = binOpts.find("-log");
    if (!bi->second.empty()) {
        opts.logFile = binOpts["-log"];
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
            LogManager::getSingleton().logError("Unrecognised target mesh version '" + bi->second + "'");
        }
    }

    return opts;
}

// Utility function to allow the user to modify the layout of vertex buffers.
void reorganiseVertexBuffers(const UpgradeOptions& opts, Mesh& mesh)
{
    // Make sure animation types up to date
    mesh._determineAnimationTypes();

    if (mesh.sharedVertexData) {
        // Automatic
        VertexDeclaration* newDcl =
            mesh.sharedVertexData->vertexDeclaration->getAutoOrganisedDeclaration(
            mesh.hasSkeleton(), mesh.hasVertexAnimation(), mesh.getSharedVertexDataAnimationIncludesNormals());
        if (*newDcl != *(mesh.sharedVertexData->vertexDeclaration)) {
            mesh.sharedVertexData->reorganiseBuffers(newDcl);
        }
    }

    for (size_t i = 0; i < mesh.getNumSubMeshes(); i++)
    {
        SubMesh* sm = mesh.getSubMesh(i);
        if (!sm->useSharedVertices) {
            const bool hasVertexAnim = sm->getVertexAnimationType() != Ogre::VAT_NONE;

            // Automatic
            VertexDeclaration* newDcl =
                sm->vertexData->vertexDeclaration->getAutoOrganisedDeclaration(
                mesh.hasSkeleton(), hasVertexAnim, sm->getVertexAnimationIncludesNormals() );
            if (*newDcl != *(sm->vertexData->vertexDeclaration)) {
                sm->vertexData->reorganiseBuffers(newDcl);
            }
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
    auto logMgr = LogManager::getSingletonPtr();
    logMgr->logMessage("LOD config summary:");
    logMgr->logMessage(" - lodConfig.strategy=" + lodConfig.strategy->getName());
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
        logMgr->logMessage(" - lodConfig.levels[" + StringConverter::toString(i) + "].distance=" +
                           StringConverter::toString(lodLevel.distance) + distQuantity);
        logMgr->logMessage(" - lodConfig.levels[" + StringConverter::toString(i) + "].reductionMethod=" +
                           (lodLevel.manualMeshName.empty() ? reductionMethod : "N/A"));
        logMgr->logMessage(" - lodConfig.levels[" + StringConverter::toString(i) + "].reductionValue=" +
                           (lodLevel.manualMeshName.empty() ? StringConverter::toString(lodLevel.reductionValue) : "N/A"));
        logMgr->logMessage(" - lodConfig.levels[" + StringConverter::toString(i) + "].manualMeshName=" +
                           (lodLevel.manualMeshName.empty() ? "N/A" : lodLevel.manualMeshName));
    }
}

void buildLod(UpgradeOptions& opts, MeshPtr& mesh)
{
    String response;

    // Prompt for LOD generation?
    bool genLod = (opts.numLods != 0 || opts.lodAutoconfigure);
    if (!genLod) {
        return;
    }

    int numLod;
    LodConfig lodConfig;
    lodConfig.mesh = mesh;
    lodConfig.strategy = DistanceLodBoxStrategy::getSingletonPtr();

    // not interactive: read parameters from console
    numLod = opts.numLods;
    LodLevel lodLevel = {};
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

    // ensure we use correct bounds
    recalcBounds(mesh.get());

    MeshLodGenerator gen;
    if (opts.lodAutoconfigure) {
        // In this case we ignore other settings
        gen.getAutoconfig(mesh, lodConfig);
    }
    printLodConfig(lodConfig);

    LogManager::getSingleton().logMessage("Generating LOD levels...");
    gen.generateLodLevels(lodConfig);
    LogManager::getSingleton().logMessage("Generating LOD levels... success");
}

void checkColour(VertexData* vdata, bool& hasColour, bool& hasAmbiguousColour,
                 VertexElementType& originalType)
{
    const VertexDeclaration::VertexElementList& elemList = vdata->vertexDeclaration->getElements();
    for (const auto & elem : elemList) {
        switch (elem.getType()) {
        case _DETAIL_SWAP_RB:
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

    if(!hasColour)
        return;

    VertexElementType desiredType = VET_UBYTE4_NORM;
    if (mesh->sharedVertexData) {
        mesh->sharedVertexData->convertPackedColour(originalType, desiredType);
    }
    for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i) {
        SubMesh* sm = mesh->getSubMesh(i);
        if (!sm->useSharedVertices) {
            sm->vertexData->convertPackedColour(originalType, desiredType);
        }
    }
}

struct MeshResourceCreator : public MeshSerializerListener
{
    void processMaterialName(Mesh *mesh, String *name) override
    {
		if(name->empty()) {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "The provided mesh file has an empty material name. See https://ogrecave.github.io/ogre/api/latest/_mesh-_tools.html#autotoc_md32");
		}
        else {
            // create material because we do not load any .material files
            MaterialManager::getSingleton().createOrRetrieve(*name, mesh->getGroup());
        }
    }

    void processSkeletonName(Mesh *mesh, String *name) override
    {
        if (name->empty())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "The provided mesh file uses an empty skeleton name");
        }

        // create skeleton because we do not load any .skeleton files
        SkeletonManager::getSingleton().createOrRetrieve(*name, mesh->getGroup(), true);
    }
    void processMeshCompleted(Mesh *mesh) override {}
};
}

int main(int numargs, char** args)
{
    if (numargs < 2) {
        help();
        return -1;
    }

    int retCode = 0;

    LogManager logMgr;
    // this log catches output from the parseArgs call and routes it to stdout only
    logMgr.createLog("Temporary log", true, true, true);

    try
    {
        UnaryOptionList unOptList;
        BinaryOptionList binOptList;

        unOptList["-byte"] = true; // this is the only option now, dont error if specified
        unOptList["-e"] = true; // this is default, dont error if specified

        unOptList["-el"] = false;
        unOptList["-t"] = false;
        unOptList["-tm"] = false;
        unOptList["-tr"] = false;
        unOptList["-r"] = false;
        unOptList["-autogen"] = false;
        unOptList["-pack"] = false;
        unOptList["-b"] = false;
        binOptList["-l"] = "";
        binOptList["-d"] = "";
        binOptList["-p"] = "";
        binOptList["-f"] = "";
        binOptList["-E"] = "";
        binOptList["-ts"] = "";
        binOptList["-V"] = "";
        binOptList["-log"] = "OgreMeshUpgrader.log";

        int startIdx = findCommandLineOpts(numargs, args, unOptList, binOptList);
        auto opts = parseOpts(unOptList, binOptList);

        logMgr.setDefaultLog(NULL); // swallow startup messages
        DefaultHardwareBufferManager bufferManager; // needed because we don't have a rendersystem
        Root root("", "", "");
        // get rid of the temporary log as we use the new log now
        logMgr.destroyLog("Temporary log");

        // use the log specified by the cmdline params
        logMgr.setDefaultLog(logMgr.createLog(opts.logFile, true, true));

        String source(args[startIdx]);

        MaterialManager::getSingleton().initialise();
        MeshSerializer meshSerializer;
        MeshResourceCreator resCreator;
        meshSerializer.setListener(&resCreator);
        SkeletonSerializer skeletonSerializer;
        // don't pad during upgrade
        MeshManager::getSingleton().setBoundsPaddingFactor(0.0f);

        // Load the mesh
        struct stat tagStat;

        FILE* pFile = fopen( source.c_str(), "rb" );
        if (!pFile) {
            OGRE_EXCEPT(Exception::ERR_FILE_NOT_FOUND,
                "File " + source + " not found.", "OgreMeshUpgrader");
        }
        stat( source.c_str(), &tagStat );
        MemoryDataStream* memstream = new MemoryDataStream(source, tagStat.st_size, true);
        size_t result = fread( (void*)memstream->getPtr(), 1, tagStat.st_size, pFile );
        if (result != size_t(tagStat.st_size))
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                "Unexpected error while reading file " + source, "OgreMeshUpgrader");
        fclose( pFile );

        MeshPtr meshPtr = MeshManager::getSingleton().createManual("TmpConversionMesh", RGN_DEFAULT);
        Mesh* mesh = meshPtr.get();

        DataStreamPtr stream(memstream);
        meshSerializer.importMesh(stream, mesh);

        // Write out the converted mesh
        String dest;
        if (numargs == startIdx + 2) {
            dest = args[startIdx + 1];
        } else {
            dest = source;
        }

        String response;

        reorganiseVertexBuffers(opts, *mesh);

        // Deal with VET_COLOUR ambiguities
        resolveColourAmbiguities(mesh);

        buildLod(opts, meshPtr);

        // Make sure we generate edge lists, provided they are not deliberately disabled
        if (opts.generateEdgeLists) {
            logMgr.logMessage("Generating edge lists...");
            mesh->buildEdgeList();
            logMgr.logMessage("Generating edge lists... success");
        } else {
            mesh->freeEdgeList();
        }
        // Generate tangents?
        if (opts.generateTangents) {
            unsigned short srcTex;
            bool existing = mesh->suggestTangentVectorBuildParams(srcTex);
            if (existing) {
                // safe
                opts.generateTangents = false;
            }
            if (opts.generateTangents) {
                logMgr.logMessage("Generating tangent vectors...");
                mesh->buildTangentVectors(srcTex, opts.tangentSplitMirrored, opts.tangentSplitRotated,
                                          opts.tangentUseParity);
                logMgr.logMessage("Generating tangent vectors... success");
            }
        }

        if(opts.packNormalsTangents)
        {
            mesh->_convertVertexElement(VES_NORMAL, VET_INT_10_10_10_2_NORM);
            mesh->_convertVertexElement(VES_TANGENT, VET_INT_10_10_10_2_NORM);
        }

        if (opts.recalcBounds) {
            recalcBounds(mesh);
        }

        meshSerializer.exportMesh(mesh, dest, opts.targetVersion, opts.endian);

        logMgr.setDefaultLog(NULL); // swallow shutdown messages
    }
    catch (Exception& e)
    {
        LogManager::getSingleton().logError(e.getDescription());
        retCode = 1;
    }

    return retCode;
}
