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
#include "OgreSkeletonSerializer.h"
#include "OgreDefaultHardwareBufferManager.h"
#include "OgreMeshLodGenerator.h"
#include "OgreDistanceLodStrategy.h"
#include "OgreLodStrategyManager.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgrePixelCountLodStrategy.h"
#include "OgreLodConfig.h"
#include "OgreRoot.h"

#include "OgreMeshManager2.h"
#include "OgreMesh2.h"

#include "UpgradeOptions.h"

#ifdef OGRE_STATIC_LIB
#include "OgreNULLRenderSystem.h"
#endif

#include "XML/tinyxml.h"
#include "XML/OgreXMLMeshSerializer.h"
#include "XML/OgreXMLSkeletonSerializer.h"

#include <iostream>
#include <sys/stat.h>

using namespace std;
using namespace Ogre;

void help(void)
{
    // OgreMeshTool <Name> (2.1.0) unstable
    cout << "OgreMeshTool " << OGRE_VERSION_NAME << " "
         << "(" << OGRE_VERSION_MAJOR << "." << OGRE_VERSION_MINOR << "." << OGRE_VERSION_PATCH << ")"
         << " " << OGRE_VERSION_SUFFIX << endl;

    // Print help message
    cout << endl << "* Converts to and from XML" << endl;
    cout << "* Processes meshes" << endl;
    cout << "* Converts between v2 and v1 mesh formats" << endl;
    cout << "* Upgrades or downgrades .mesh file versions from either v2 and v1 mesh formats" << endl;
    cout << "Provided for OGRE by Steve Streeting 2004-2017" << endl << endl;
    cout << "Usage: OgreMeshTool [opts] sourcefile [destfile] " << endl;
    cout << "-i             = Interactive mode, prompt for options. Implies -U" << endl;
    cout << "-autogen       = Generate autoconfigured LOD. No more LOD options needed!. Implies -U" << endl;
    cout << "-l lodlevels   = number of LOD levels" << endl;
    cout << "-d loddist     = distance increment to reduce LOD" << endl;
    cout << "-p lodpercent  = Percentage triangle reduction amount per LOD" << endl;
    cout << "-f lodnumtris  = Fixed vertex reduction per LOD" << endl;
    cout << "-e             = DON'T generate edge lists (for stencil shadows)" << endl;
    cout << "-t             = Generate tangents (for normal mapping). Implies -U" << endl;
    cout << "-td [uvw|tangent]" << endl;
    cout << "           = Tangent vertex semantic destination (default tangent)" << endl;
    cout << "-ts [3|4]      = Tangent size (3 or 4 components, 4 includes parity, default 3)" << endl;
    cout << "-tm            = Split tangent vertices at UV mirror points" << endl;
    cout << "-tr            = Split tangent vertices where basis is rotated > 90 degrees" << endl;
    cout << "-r         = DON'T reorganise buffers to recommended format" << endl;
    cout << "-o         = DON'T optimise out redundant tracks & keyframes" << endl;
    cout << "-d3d       = Convert to D3D colour formats" << endl;
    cout << "-gl        = Convert to GL colour formats" << endl;
    cout << "-srcd3d    = Interpret ambiguous colours as D3D style" << endl;
    cout << "-srcgl     = Interpret ambiguous colours as GL style" << endl;
    cout << "-E endian  = Set endian mode 'big' 'little' or 'native' (default)" << endl;
    cout << "-b         = Recalculate bounding box (static meshes only)" << endl;
    cout << "-V version = Specify OGRE version format to write instead of latest" << endl;
    cout << "             Options are: 2.1, 1.10, 1.8, 1.7, 1.4, 1.0" << endl;
    cout << "-v2          Export the mesh as a v2 object. Keeps the original format otherwise." << endl;
    cout << "             Use this format if you load the mesh by the SceneManager::createItem() method." << endl;
    cout << "-v1          Export the mesh as a v1 object. Keeps the original format otherwise." << endl;
    cout << "             Use this if you load the mesh by the SceneManager::createEntity() method or if you import from v1 to v2 at runtime." << endl;
    cout << "-O puqs    = Optimize vertex buffers for shaders." << endl;
    cout << "             p converts POSITION to 16-bit floats" << endl;
    cout << "             q converts normal tangent and bitangent (28-36 bytes) to QTangents (8 bytes)." << endl;
    cout << "             u converts UVs to 16-bit floats." << endl;
    cout << "             s make shadow mapping passes have their own optimized buffers. Overrides existing ones if any." << endl;
    cout << "             S strips the buffers for shadow mapping (consumes less space and memory)." << endl;
    cout << "-U         = Performs the opposite of -O puq: Converts 16-bit half to to float and " << endl;
    cout << "             converts QTangents to Normal + Tangent + Reflection. Needed by many" << endl;
    cout << "             other options that have to read from position, normals or UVs." << endl;
    cout << "             '-o puq' can be used to optimize the buffers again right before saving to disk." << endl;
    cout << "sourcefile = name of file to convert" << endl;
    cout << "destfile   = optional name of file to write to. If you don't" << endl;
    cout << "             specify this OGRE overwrites the existing file." << endl;
    cout << endl;
    cout << "Recommended params for modern DESKTOP (w/ normal mapping):" << endl;
    cout << "   OgreMeshTool -e -t -ts 4 -O puqs sourcefile [destfile]" << endl;
    cout << "Recommended params for GLES2 (w/ normal mapping):" << endl;
    cout << "   OgreMeshTool -e -t -ts 4 -O qs sourcefile [destfile]" << endl;
    cout << "Recommended params for modern DESKTOP (w/out normal mapping):" << endl;
    cout << "   OgreMeshTool -e -O puqs sourcefile [destfile]" << endl;
    cout << "Recommended params for GLES2 (w/out normal mapping):" << endl;
    cout << "   OgreMeshTool -e -O qs sourcefile [destfile]" << endl;

    cout << endl;
}

// Crappy globals
v1::MeshSerializer* meshSerializer = 0;
v1::SkeletonSerializer* skeletonSerializer = 0;
LogManager *logManager = 0;
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
    opts.dontOptimiseAnimations = false;
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
    opts.targetVersion  = v1::MESH_VERSION_LATEST;
    opts.targetVersionV2= MESH_VERSION_LATEST;
    opts.exportAsV1     = false;
    opts.exportAsV2     = false;
    opts.unoptimizeBuffer = false;
    opts.optimizeBuffer = false;
    opts.halfPos        = false;
    opts.halfTexCoords  = false;
    opts.qTangents      = false;
    opts.optimizeForShadowMapping = false;
    opts.stripShadowMapping = false;


    UnaryOptionList::iterator ui = unOpts.find("-e");
    opts.suppressEdgeLists = ui->second;
    ui = unOpts.find("-t");
    opts.generateTangents = ui->second;
    ui = unOpts.find("-tm");
    opts.tangentSplitMirrored = ui->second;
    ui = unOpts.find("-tr");
    opts.tangentSplitRotated = ui->second;

    ui = unOpts.find("-autogen");
    opts.lodAutoconfigure = ui->second;

    ui = unOpts.find("-i");
    opts.interactive = ui->second;
    ui = unOpts.find("-r");
    opts.dontReorganise = ui->second;
    ui = unOpts.find("-o");
    opts.dontOptimiseAnimations = ui->second;
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
    ui = unOpts.find("-v1");
    if (ui->second)
    {
        opts.exportAsV1 = true;
        opts.exportAsV2 = false;
    }
    ui = unOpts.find("-v2");
    if (ui->second)
    {
        opts.exportAsV1 = false;
        opts.exportAsV2 = true;
    }
    ui = unOpts.find("-U");
    if (ui->second)
    {
        opts.unoptimizeBuffer = true;
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
        {
            opts.endian = Serializer::ENDIAN_BIG;
        }
        else if (bi->second == "little")
        {
            opts.endian = Serializer::ENDIAN_LITTLE;
        }
        else
        {
            opts.endian = Serializer::ENDIAN_NATIVE;
        }
    }
    bi = binOpts.find("-td");
    if (!bi->second.empty())
    {
        if (bi->second == "uvw")
        {
            opts.tangentSemantic = VES_TEXTURE_COORDINATES;
        }
        else     // if (bi->second == "tangent"), or anything else
        {
            opts.tangentSemantic = VES_TANGENT;
        }
    }
    bi = binOpts.find("-ts");
    if (!bi->second.empty())
    {
        if (bi->second == "4")
        {
            opts.tangentUseParity = true;
        }
    }

    bi = binOpts.find("-V");
    if (!bi->second.empty())
    {
        if (bi->second == "2.1")
        {
            opts.targetVersion  = v1::MESH_VERSION_2_1;
            opts.targetVersionV2= MESH_VERSION_2_1;
        }

        if( !opts.exportAsV2 )
        {
            if (bi->second == "1.10")
            {
                opts.targetVersion = v1::MESH_VERSION_1_10;
            }
            else if (bi->second == "1.8")
            {
                opts.targetVersion = v1::MESH_VERSION_1_8;
            }
            else if (bi->second == "1.7")
            {
                opts.targetVersion = v1::MESH_VERSION_1_7;
            }
            else if (bi->second == "1.4")
            {
                opts.targetVersion = v1::MESH_VERSION_1_4;
            }
            else if (bi->second == "1.0")
            {
                opts.targetVersion = v1::MESH_VERSION_1_0;
            }
        }

        if( opts.targetVersion == v1::MESH_VERSION_LATEST && !opts.exportAsV2 )
        {
            LogManager::getSingleton().getDefaultLog()->stream() << "Unrecognised target mesh version '" << bi->second << "'";
        }
        else if( opts.targetVersionV2 == MESH_VERSION_LATEST && opts.exportAsV2 )
        {
            LogManager::getSingleton().getDefaultLog()->stream() << "Unrecognised target mesh version '" <<
                                                                    bi->second <<
                                                                    "' or version can't be used with -v2 argument";
        }
    }

    ui = unOpts.find("-O");
    opts.optimizeBuffer = ui->second;

    bi = binOpts.find("-O");
    if( !bi->second.empty() )
    {
        if( bi->second.find( 'p' ) != String::npos )
            opts.halfPos = true;
        if( bi->second.find( 'u' ) != String::npos )
            opts.halfTexCoords = true;
        if( bi->second.find( 'q' ) != String::npos )
            opts.qTangents = true;
        if( bi->second.find( 's' ) != String::npos )
            opts.optimizeForShadowMapping = true;
        if( bi->second.find( 'S' ) != String::npos )
        {
            opts.optimizeForShadowMapping = true;
            opts.stripShadowMapping = true;
        }
    }

    if( opts.interactive || opts.numLods || opts.lodAutoconfigure || opts.generateTangents )
        opts.unoptimizeBuffer = true;
}

// Utility function to allow the user to modify the layout of vertex buffers.
void vertexBufferReorg(v1::Mesh& mesh);
void buildEdgeLists( v1::MeshPtr &mesh );
void generateTangents( v1::MeshPtr &mesh );
void recalcBounds( v1::MeshPtr &v1Mesh, MeshPtr &v2Mesh );

void printLodConfig(const LodConfig& lodConfig)
{
    cout << "\n\nLOD config summary:";
    cout << "\n  lodConfig.strategy=" << lodConfig.strategy->getName();
    String reductionMethod("Unknown");
    if (lodConfig.levels[0].reductionMethod == LodLevel::VRM_PROPORTIONAL)
    {
        reductionMethod = "VRM_PROPORTIONAL";
    }
    else if (lodConfig.levels[0].reductionMethod == LodLevel::VRM_CONSTANT)
    {
        reductionMethod = "VRM_PROPORTIONAL";
    }
    else if (lodConfig.levels[0].reductionMethod == LodLevel::VRM_COLLAPSE_COST)
    {
        reductionMethod = "VRM_COLLAPSE_COST";
    }
    String distQuantity;
    if (lodConfig.strategy == PixelCountLodStrategy::getSingletonPtr())
    {
        distQuantity = "px";
    }
    for (unsigned short i = 0; i < lodConfig.levels.size(); i++)
    {
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
size_t getUniqueVertexCount(v1::MeshPtr mesh)
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
void buildLod(v1::MeshPtr& mesh)
{
    String response;

    // Prompt for LOD generation?
    bool genLod = (opts.numLods != 0 || opts.interactive || opts.lodAutoconfigure);
    bool askLodDtls = opts.interactive;
    if (genLod)
    {
        if( mesh.isNull() )
        {
            cout << "LOD Generation only works on v1 meshes at the moment." << endl;
            cout << "Export it as -v1, run the command again, and re-export it to -v2" << endl;
            return;
        }

        // otherwise only ask if not specified on command line
        if (mesh->getNumLodLevels() > 1)
        {
            do
            {
                std::cout << "\nMesh already contains level-of-detail information.\n"
                          "Do you want to: (u)se it, (r)eplace it, or (d)rop it? ";
                cin >> response;
                StringUtil::toLowerCase(response);
                if (response == "u")
                {
                    genLod = false;
                    askLodDtls = false;
                }
                else if (response == "d")
                {
                    mesh->removeLodLevels();
                    genLod = false;
                    askLodDtls = false;
                }
                else if (response == "r")
                {
                    genLod = true;
                }
                else
                {
                    std::cout << "Wrong answer!\n";
                    response = "";
                }
            }
            while( response == "" );
        }
        else if (askLodDtls)
        {
            do
            {
                std::cout << "\nWould you like to generate level-of-detail information? (y/n) ";
                cin >> response;
                StringUtil::toLowerCase(response);
                if (response == "n")
                {
                    genLod = false;
                }
                else if (response == "y")
                {
                    genLod = true;
                }
                else
                {
                    std::cout << "Wrong answer!\n";
                    response = "";
                }
            }
            while( response == "" );
        }
    }

    if (!genLod)
        return;

    int numLod;
    LodConfig lodConfig;
    lodConfig.mesh = mesh;
    lodConfig.strategy = DistanceLodStrategy::getSingletonPtr();
    if (askLodDtls)
    {
        do
        {
            std::cout <<
                      "\nDo you want to (m)anually configure or (a)utoconfigure it?\nautoconfigure=no more questions asked!(m/a) ";
            cin >> response;
            StringUtil::toLowerCase(response);
            if (response == "a")
            {
                opts.lodAutoconfigure = true;
            }
            else if (response == "m")
            {
                opts.lodAutoconfigure = false;
            }
            else
            {
                std::cout << "Wrong answer!\n";
                response = "";
            }
        }
        while (response == "");
        if (!opts.lodAutoconfigure)
        {
            do
            {
                std::cout << "\nDo you want to use (p)ixels or (d)istance to determine where the LOD activates? (p/d) ";
                cin >> response;
                StringUtil::toLowerCase(response);
                if (response == "p")
                {
                    lodConfig.strategy = PixelCountLodStrategy::getSingletonPtr();
                }
                else if (response == "d")
                {
                    lodConfig.strategy = DistanceLodStrategy::getSingletonPtr();
                }
                else
                {
                    std::cout << "Wrong answer!\n";
                    response = "";
                }
            }
            while (response == "");
            LodLevel lodLevel;
            size_t vertexCount = 0;
            do
            {
                cout <<
                     "\nWhat unit of reduction would you like to use(fixed=constant vertex number; proportional=percentage):" <<
                     "\n(f)ixed or (p)roportional? ";
                cin >> response;
                StringUtil::toLowerCase(response);
                if (response == "f")
                {
                    lodLevel.reductionMethod = LodLevel::VRM_CONSTANT;
                    vertexCount = getUniqueVertexCount(mesh);
                }
                else if (response == "p")
                {
                    lodLevel.reductionMethod = LodLevel::VRM_PROPORTIONAL;
                }
                else
                {
                    std::cout << "Wrong answer!\n";
                    response = "";
                }
            }
            while (response == "");

            numLod = 0;
            while (numLod < 1 || numLod > 99)
            {
                cout << "\nHow many extra LOD levels would you like to generate? (1-99)";
                cin >> response;
                numLod = StringConverter::parseInt(response);
            }
            Real minReduction = 0.0;
            Real minDistance = lodConfig.strategy->getBaseValue();
            for (int iLod = 0; iLod < numLod; ++iLod)
            {
                do
                {
                    cout << "\nShould LOD" << (iLod + 1) << " be a (m)anual or (g)enerated LOD level? (m/g)";
                    cin >> response;
                    StringUtil::toLowerCase(response);
                    if (response == "m")
                    {
                        while (lodLevel.manualMeshName.empty())
                        {
                            cout << "\nEnter the mesh name to show for LOD" << (iLod + 1) << ". (e.g. Sinbad_LOD1.mesh)";
                            cout << "\nIt should have the same animation states and it should not have LOD levels!\n";
                            cin >> lodLevel.manualMeshName;
                        }
                    }
                    else if (response == "g")
                    {
                        if (lodLevel.reductionMethod == LodLevel::VRM_PROPORTIONAL)
                        {
                            cout << "\nWhat percentage of remaining vertices should be removed at LOD" <<
                                 (iLod + 1) << "? (e.g. 40 = remove 40% of vertices)";
                            cout << "\nIt should be between " << minReduction << "%-100%\n";
                        }
                        else
                        {
                            cout << "\nHow many vertices should be removed at LOD" <<
                                 (iLod + 1) << "? (e.g. 50 = remove 50 vertices)";
                            cout << "\nIt should be between " << (int) minReduction << "-" << vertexCount << " vertices\n";
                        }
                        while (1)
                        {
                            cin >> response;
                            lodLevel.reductionValue = StringConverter::parseReal(response);
                            if (lodLevel.reductionMethod == LodLevel::VRM_PROPORTIONAL)
                            {
                                if (lodLevel.reductionValue > minReduction && lodLevel.reductionValue <= 100.0)
                                {
                                    minReduction = lodLevel.reductionValue;
                                    lodLevel.reductionValue *= 0.01;
                                    if (minReduction == 100.0)
                                    {
                                        cout << "\nCan't insert more LOD levels!";
                                        iLod = numLod;
                                    }
                                    break;
                                }
                                else
                                {
                                    cout << "\nWrong answer! It should be between " << minReduction << "%-100%.";
                                }
                            }
                            else
                            {
                                if ((int) lodLevel.reductionValue > (int) minReduction && (int) lodLevel.reductionValue <= (int)vertexCount)
                                {
                                    minReduction = lodLevel.reductionValue;
                                    break;
                                }
                                else
                                {
                                    cout << "\nIt should be between " << (int) minReduction << "-" << vertexCount << " vertices.";
                                }
                            }
                        }

                    }
                    else
                    {
                        std::cout << "Wrong answer!\n";
                        response = "";
                    }
                }
                while (response == "");

                if (lodConfig.strategy == DistanceLodStrategy::getSingletonPtr())
                {
                    cout << "\nEnter the distance for LOD" <<
                         (iLod + 1) << " to activate. (distance of camera and mesh in Ogre units)";
                    cout << "\nIt should be more then last Lod level: " << minDistance << "\n";
                }
                else
                {
                    cout << "\nEnter the pixel count for LOD" <<
                         (iLod + 1) << " to activate. (pixels of the bounding sphere in the render output)";
                    cout << "\nIt should be LESS then last Lod level: " << minDistance << "\n";
                }
                while (1)
                {
                    cin >> response;
                    lodLevel.distance = StringConverter::parseReal(response);
                    if (lodConfig.strategy == DistanceLodStrategy::getSingletonPtr())
                    {
                        if (lodLevel.distance > minDistance)
                        {
                            break;
                        }
                        else
                        {
                            cout << "\nWrong answer! It should be more than " << minDistance << ".";
                        }
                    }
                    else
                    {
                        if (lodLevel.distance < minDistance && lodLevel.distance >= 0)
                        {
                            break;
                        }
                        else
                        {
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
    }
    else
    {
        // not interactive: read parameters from console
        numLod = opts.numLods;
        LodLevel lodLevel;
        lodLevel.distance = 0.0;
        for (unsigned short iLod = 0; iLod < numLod; ++iLod)
        {

            lodLevel.reductionMethod = opts.usePercent ?
                                       LodLevel::VRM_PROPORTIONAL : LodLevel::VRM_CONSTANT;
            if (opts.usePercent)
            {
                lodLevel.reductionValue += opts.lodPercent * 0.01f;
            }
            else
            {
                lodLevel.reductionValue += (Ogre::Real)opts.lodFixed;
            }

            lodLevel.distance += opts.lodDist;
            lodConfig.levels.push_back(lodLevel);
        }
    }

    {
        // ensure we use correct bounds
        MeshPtr nullv2;
        recalcBounds( mesh, nullv2 );
    }

    MeshLodGenerator gen;
    if (opts.lodAutoconfigure)
    {
        // In this case we ignore other settings
        gen.getAutoconfig(mesh, lodConfig);
    }
    printLodConfig(lodConfig);


    cout << "\n\nGenerating LOD levels...";
    gen.generateLodLevels(lodConfig);
    cout << "success\n";
}

void checkColour(v1::VertexData* vdata, bool& hasColour, bool& hasAmbiguousColour,
                 VertexElementType& originalType)
{
    const v1::VertexDeclaration::VertexElementList& elemList = vdata->vertexDeclaration->getElements();
    for (v1::VertexDeclaration::VertexElementList::const_iterator i = elemList.begin();
            i != elemList.end(); ++i)
    {
        const v1::VertexElement& elem = *i;
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
        }
    }

}

void resolveColourAmbiguities(v1::Mesh* mesh)
{
    // Check what we're dealing with
    bool hasColour = false;
    bool hasAmbiguousColour = false;
    VertexElementType originalType = VET_FLOAT1;
    if (mesh->sharedVertexData[VpNormal])
    {
        checkColour(mesh->sharedVertexData[VpNormal], hasColour, hasAmbiguousColour, originalType);
    }
    for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        v1::SubMesh* sm = mesh->getSubMesh(i);
        if (sm->useSharedVertices == false)
        {
            checkColour(sm->vertexData[VpNormal], hasColour, hasAmbiguousColour, originalType);
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
            std::cout   << "\nYour mesh has vertex colours but I don't know whether they were generated\n"
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
                    std::cout << "Wrong answer!\n";
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
                std::cout   << "\nYour mesh has vertex colours, which can be stored in one of two layouts,\n"
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
                        std::cout << "Wrong answer!\n";
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

    if (mesh->sharedVertexData[VpNormal] && hasColour)
    {
        mesh->sharedVertexData[VpNormal]->convertPackedColour(originalType, desiredType);
    }
    for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        v1::SubMesh* sm = mesh->getSubMesh(i);
        if (sm->useSharedVertices == false && hasColour)
        {
            sm->vertexData[VpNormal]->convertPackedColour(originalType, desiredType);
        }
    }
}

DataStreamPtr openFile( String source )
{
    struct stat tagStat;

    FILE* pFile = fopen( source.c_str(), "rb" );
    if (!pFile)
    {
        OGRE_EXCEPT(Exception::ERR_FILE_NOT_FOUND,
                    "File " + source + " not found.", "OgreMeshTool");
    }
    stat( source.c_str(), &tagStat );
    MemoryDataStream* memstream = new MemoryDataStream(source, tagStat.st_size, true);
    size_t result = fread( (void*)memstream->getPtr(), 1, tagStat.st_size, pFile );
    if (result != tagStat.st_size)
    {
        OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                    "Unexpected error while reading file " + source, "OgreMeshTool");
    }
    fclose( pFile );

    return DataStreamPtr( memstream );
}

/** Loads a mesh to either meshPtr or v2MeshPtr. Both may be empty if we just loaded
    an XML skeleton (in which case we just save it)
@param source [in]
@param meshPtr [out]
@param v2MeshPtr [out]
@param meshSerializer2 [in]
@return
    True on success.
    False on failure.
*/
bool loadMesh( const String &source, v1::MeshPtr &v1MeshPtr, MeshPtr &v2MeshPtr,
               v1::SkeletonPtr &v1Skeleton,
               Ogre::MeshSerializer &meshSerializer2, v1::XMLMeshSerializer &xmlMeshSerializer,
               v1::XMLSkeletonSerializer &xmlSkeletonSerializer )
{
    bool retVal = false;

    const String::size_type extPos = source.find_last_of( '.' );
    const String sourceExt( source.substr( extPos + 1, source.size() ) );

    if( sourceExt == "mesh" )
    {
        DataStreamPtr stream( openFile( source ) );

        try
        {
            cout << "Trying to read " << source << " as a v1 mesh..." << endl;
            v1MeshPtr = v1::MeshManager::getSingleton().createManual(
                        "conversionSrcV1", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
            meshSerializer->importMesh( stream, v1MeshPtr.get() );
            retVal = true;
            cout << "Success!" << endl;
        }
        catch( Exception & )
        {
            cout << "Failed." << endl;
            if( v1MeshPtr )
                v1::MeshManager::getSingleton().remove( v1MeshPtr );
            v1MeshPtr.setNull();
        }

        if( !retVal )
        {
            try
            {
                cout << "Trying to read " << source << " as a v2 mesh..." << endl;
                v2MeshPtr = MeshManager::getSingleton().createManual(
                            "conversionSrcV2", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
                stream->seek( 0 );
                meshSerializer2.importMesh( stream, v2MeshPtr.get() );
                retVal = true;
                cout << "Success!" << endl;
            }
            catch( Exception & )
            {
                cout << "Failed." << endl;
                v2MeshPtr.setNull();
            }
        }
    }
    else if( sourceExt == "skeleton" )
    {
        DataStreamPtr stream( openFile( source ) );

        cout << "Trying to read " << source << " as a v1 skeleton..." << endl;
        v1Skeleton = v1::OldSkeletonManager::getSingleton().create(
                        "conversion", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
        skeletonSerializer->importSkeleton( stream, v1Skeleton.get() );
        retVal = true;
        cout << "Success!" << endl;
    }
    else if( sourceExt == "xml" )
    {
        // Read root element and decide from there what type
        TiXmlDocument* doc = new TiXmlDocument( source );
        // Some double-parsing here but never mind
        if( !doc->LoadFile() )
        {
            cout << "Unable to open file " << source << " - fatal error." << endl;
        }
        else
        {
            TiXmlElement* root = doc->RootElement();
            if( !stricmp(root->Value(), "mesh") )
            {
                v1MeshPtr = v1::MeshManager::getSingleton().createManual(
                            "conversionSrcXML", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );

                VertexElementType colourElementType = VET_COLOUR_ABGR;
                if( opts.destColourFormatSet )
                    colourElementType = opts.destColourFormat;

                xmlMeshSerializer.importMesh( source, colourElementType, v1MeshPtr.get() );

                // Re-jig the buffers?
                // Make sure animation types are up to date first
                v1MeshPtr->_determineAnimationTypes();

                retVal = true;
            }
            else if( !stricmp(root->Value(), "skeleton") )
            {
                v1Skeleton = v1::OldSkeletonManager::getSingleton().create(
                            "conversion", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
                xmlSkeletonSerializer.importSkeleton( source, v1Skeleton.get() );

                retVal = true;
            }
        }

        delete doc;
    }
    else
    {
        cout << "Couldn't identify extension of filename '" << source << "'" << endl;
    }

    return retVal;
}

void saveMesh( const String &destination, v1::MeshPtr &v1Mesh, MeshPtr &v2Mesh,
               v1::SkeletonPtr &v1Skeleton,
               Ogre::MeshSerializer &meshSerializer2, v1::XMLMeshSerializer &xmlMeshSerializer,
               v1::XMLSkeletonSerializer &xmlSkeletonSerializer )
{
    const String::size_type extPos = destination.find_last_of( '.' );
    const String dstExt( destination.substr( extPos + 1, destination.size() ) );

    if( dstExt == "mesh" )
    {
        if( (!opts.exportAsV2 && !v1Mesh.isNull()) || opts.exportAsV1 )
        {
            if( opts.exportAsV1 && v1Mesh.isNull() )
            {
                v1Mesh = v1::MeshManager::getSingleton().createManual( "conversionDstV1",
                                                                       ResourceGroupManager::
                                                                       DEFAULT_RESOURCE_GROUP_NAME );
                v1Mesh->importV2( v2Mesh.get() );

                if( v1Mesh->hasSkeleton() )
                    vertexBufferReorg( *v1Mesh.get() );
            }

            cout << "Saving as a v1 mesh..." << endl;
            meshSerializer->exportMesh( v1Mesh.get(), destination, opts.targetVersion, opts.endian );
        }
        else
        {
            if( v2Mesh.isNull() )
            {
                v2Mesh = MeshManager::getSingleton().createManual( "v2Mesh",
                                                                   ResourceGroupManager::
                                                                   DEFAULT_RESOURCE_GROUP_NAME );
            }

            if( !v1Mesh.isNull() )
                v2Mesh->importV1( v1Mesh.get(), false, false, false );

            cout << "Saving as a v2 mesh..." << endl;
            meshSerializer2.exportMesh( v2Mesh.get(), destination, opts.targetVersionV2, opts.endian );
        }

        if( !v1Skeleton.isNull() )
        {
            skeletonSerializer->exportSkeleton( v1Skeleton.get(), destination + ".skeleton",
                                                v1::SKELETON_VERSION_LATEST, opts.endian );
        }
    }
    else if( dstExt == "skeleton" )
    {
        if( !v1Skeleton.isNull() )
        {
            skeletonSerializer->exportSkeleton( v1Skeleton.get(), destination,
                                                v1::SKELETON_VERSION_LATEST, opts.endian );
        }
    }
    else if( dstExt == "xml" )
    {
        if( v1Mesh.isNull() && !v2Mesh.isNull() )
        {
            v1Mesh = v1::MeshManager::getSingleton().createManual(
                                    "conversionDstXML",
                         ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
            v1Mesh->importV2( v2Mesh.get() );
        }

        if( !v1Mesh.isNull() )
        {
            cout << "Saving as a XML mesh..." << endl;
            xmlMeshSerializer.exportMesh( v1Mesh.get(), destination );
        }

        if( !v1Skeleton.isNull() )
        {
            cout << "Saving as a XML skeleton..." << endl;
            xmlSkeletonSerializer.exportSkeleton( v1Skeleton.get(), destination );
        }
    }
}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    WCHAR gWorkingDir[MAX_PATH];
#endif

void setWorkingDirectory()
{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    GetCurrentDirectoryW( MAX_PATH, gWorkingDir );

    HMODULE hModule = GetModuleHandleW(NULL);
    WCHAR path[MAX_PATH];
    GetModuleFileNameW( hModule, path, MAX_PATH );

    std::wstring pathName( path );
    std::wstring::size_type charPos = pathName.find_last_of( L"/\\" );

    if( charPos != std::wstring::npos )
        pathName = pathName.substr( 0, charPos );

    SetCurrentDirectoryW( pathName.c_str() );
#else
    //This ought to work in Unix, but I didn't test it, otherwise try setenv()
    //chdir( fullAppPath.GetPath().mb_str() );
#endif
    //Most Ogre materials assume floating point to use radix point, not comma.
    //Prevent awfull number truncation in non-US systems
    //(I love standarization...)
    setlocale( LC_NUMERIC, "C" );
}

void restoreWorkingDir()
{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    SetCurrentDirectoryW( gWorkingDir );
#else
    //This ought to work in Unix, but I didn't test it, otherwise try setenv()
    //chdir( fullAppPath.GetPath().mb_str() );
#endif
}

int main(int numargs, char** args)
{
    Root *root = 0;

    if (numargs < 2)
    {
        help();
        return -1;
    }

    int retCode = 0;
    try
    {
        Ogre::String pluginsPath;
        // only use plugins.cfg if not static
#ifndef OGRE_STATIC_LIB
#if OGRE_DEBUG_MODE
        pluginsPath = "plugins_tools_d.cfg";
#else
        pluginsPath = "plugins_tools.cfg";
#endif
#endif
        logManager = OGRE_NEW LogManager();
        logManager->createLog( "OgreMeshTool.log", true, true );
        LogManager::getSingleton().getDefaultLog()->setLogDetail( LL_LOW );
        setWorkingDirectory();
        root = OGRE_NEW Root( pluginsPath, "", "OgreMeshTool.log" ) ;
        restoreWorkingDir();

#ifdef OGRE_STATIC_LIB
        root->addRenderSystem(new Ogre::NULLRenderSystem());
#endif

        root->setRenderSystem( root->getRenderSystemByName( "NULL Rendering Subsystem" ) );
        root->initialise( true );
        LogManager::getSingleton().getDefaultLog()->setLogDetail( LL_NORMAL );

        meshSerializer = new v1::MeshSerializer();
        skeletonSerializer = new v1::SkeletonSerializer();

        Ogre::MeshSerializer meshSerializer2( root->getRenderSystem()->getVaoManager() );
        v1::XMLMeshSerializer xmlMeshSerializer;
        v1::XMLSkeletonSerializer xmlSkeletonSerializer;

        // don't pad during upgrade
        v1::MeshManager::getSingleton().setBoundsPaddingFactor(0.0f);
        MeshManager::getSingleton().setBoundsPaddingFactor(0.0f);


        UnaryOptionList unOptList;
        BinaryOptionList binOptList;

        unOptList["-i"] = false;
        unOptList["-e"] = false;
        unOptList["-t"] = false;
        unOptList["-tm"] = false;
        unOptList["-tr"] = false;
        unOptList["-r"] = false;
        unOptList["-o"] = false;
        unOptList["-gl"] = false;
        unOptList["-d3d"] = false;
        unOptList["-srcgl"] = false;
        unOptList["-srcd3d"] = false;
        unOptList["-autogen"] = false;
        unOptList["-b"] = false;
        unOptList["-O"] = false;
        unOptList["-U"] = false;
        unOptList["-v1"]= false;
        unOptList["-v2"]= false;
        binOptList["-l"] = "";
        binOptList["-d"] = "";
        binOptList["-p"] = "";
        binOptList["-f"] = "";
        binOptList["-E"] = "";
        binOptList["-td"] = "";
        binOptList["-ts"] = "";
        binOptList["-V"] = "";
        binOptList["-O"] = "";

        int startIdx = findCommandLineOpts(numargs, args, unOptList, binOptList);
        parseOpts(unOptList, binOptList);

        String source(args[startIdx]);

        // Load the mesh
        v1::MeshPtr v1Mesh;
        v1::SkeletonPtr v1Skeleton;
        MeshPtr v2Mesh;

        if( !loadMesh( source, v1Mesh, v2Mesh, v1Skeleton, meshSerializer2,
                       xmlMeshSerializer, xmlSkeletonSerializer ) )
        {
            OGRE_EXCEPT( Exception::ERR_FILE_NOT_FOUND, "Could not open '" + source + "'", "main" );
        }

        if( opts.unoptimizeBuffer )
        {
            if( !v1Mesh.isNull() )
            {
                if( v1Mesh->sharedVertexData[VpNormal] )
                {
                    cout << "v1 Mesh has shared geometry. 'Unsharing' them..." << endl;
                    v1::MeshManager::unshareVertices( v1Mesh.get() );
                    cout << "Unshare operation successful" << endl;
                }
                v1Mesh->dearrangeToInefficient();
            }

            if( !v2Mesh.isNull() )
                v2Mesh->dearrangeToInefficient();
        }

        v1::Mesh* mesh = v1Mesh.get();

        // Write out the converted mesh
        String dest;
        if (numargs == startIdx + 2)
        {
            dest = args[startIdx + 1];
        }
        else
        {
            const String::size_type extPos = source.find_last_of( '.' );
            const String sourceExt( source.substr( extPos + 1, source.size() ) );

            if( sourceExt == "xml" )
            {
                // dest is source minus .xml
                dest = source.substr( 0, source.size() - 4 );
            }
            else
            {
                dest = source;
            }
        }

        {
            const String::size_type extPos = dest.find_last_of( '.' );
            const String dstExt( dest.substr( extPos + 1, dest.size() ) );
            if( dstExt == "xml" )
            {
                if( opts.optimizeBuffer )
                {
                    cout << "-O is ignored when exporting to XML" << endl;
                }
                opts.optimizeBuffer = false;
            }
        }

        if( !v1Mesh.isNull() )
        {
            vertexBufferReorg(*mesh);

            // Deal with VET_COLOUR ambiguities
            resolveColourAmbiguities(mesh);
        }

        buildLod( v1Mesh );
        buildEdgeLists( v1Mesh );
        generateTangents( v1Mesh );

        if( opts.optimizeBuffer )
        {
            if( !v1Mesh.isNull() )
                mesh->arrangeEfficient( opts.halfPos, opts.halfTexCoords, opts.qTangents );
            if( !v2Mesh.isNull() )
                v2Mesh->arrangeEfficient( opts.halfPos, opts.halfTexCoords, opts.qTangents );
        }

        if (opts.recalcBounds)
        {
            recalcBounds( v1Mesh, v2Mesh );
        }

        if( opts.optimizeForShadowMapping )
        {
            if( !v1Mesh.isNull() )
            {
                mesh->_updateCompiledBoneAssignments();
                v1::Mesh::msOptimizeForShadowMapping = !opts.stripShadowMapping;
                mesh->prepareForShadowMapping( false );
                v1::Mesh::msOptimizeForShadowMapping = false;
            }

            if( !v2Mesh.isNull() )
            {
                Mesh::msOptimizeForShadowMapping = !opts.stripShadowMapping;
                v2Mesh->prepareForShadowMapping( false );
                Mesh::msOptimizeForShadowMapping = false;
            }
        }

        if( !opts.dontOptimiseAnimations && !v1Skeleton.isNull() )
        {
            v1Skeleton->optimiseAllAnimations();
        }

        saveMesh( dest, v1Mesh, v2Mesh, v1Skeleton, meshSerializer2,
                  xmlMeshSerializer, xmlSkeletonSerializer );
    }
    catch (Exception& e)
    {
        cout << "Exception caught: " << e.getDescription() << std::endl;
        retCode = 1;
    }

    LogManager::getSingleton().getDefaultLog()->setLogDetail( LL_LOW );

    delete skeletonSerializer;
    delete meshSerializer;

    OGRE_DELETE root;
    root = 0;

    delete logManager;
    logManager = 0;

    return retCode;

}
