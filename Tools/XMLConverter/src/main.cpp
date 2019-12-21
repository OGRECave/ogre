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
#include "OgreXMLMeshSerializer.h"
#include "OgreMeshSerializer.h"
#include "OgreXMLSkeletonSerializer.h"
#include "OgreSkeletonSerializer.h"
#include "OgreXMLPrerequisites.h"
#include "OgreDefaultHardwareBufferManager.h"
#include "OgreMeshLodGenerator.h"
#include "OgreDistanceLodStrategy.h"
#include "OgreLodStrategyManager.h"
#include <iostream>
#include <sys/stat.h>

using namespace std;
using namespace Ogre;

namespace {

// Some flags were deprecated, because they are provided in OgreMeshUpgrader too!
// Same feature with different implementation! Don't make duplicates!
// Also XMLConverter should only convert between XML format 1:1, while MeshUpgrader should modify it.
struct XmlOptions
{
    String source;
    String dest;
    String sourceExt;
    String destExt;
    String logFile;
    //bool interactiveMode; // Deprecated
    //unsigned short numLods; // Deprecated
    //Real lodValue; // Deprecated
    //String lodStrategy; // Deprecated
    //Real lodPercent; // Deprecated
    //size_t lodFixed; // Deprecated
    size_t nuextremityPoints;
    size_t mergeTexcoordResult;
    size_t mergeTexcoordToDestroy;
    bool usePercent;
    //bool generateEdgeLists; // Deprecated
    //bool generateTangents; // Deprecated
    VertexElementSemantic tangentSemantic;
    bool tangentUseParity;
    bool tangentSplitMirrored;
    bool tangentSplitRotated;
    bool reorganiseBuffers;
    bool optimiseAnimations;
    bool quietMode;
    bool d3d;
    bool gl;
    Serializer::Endian endian;
};

void print_version(void)
{
    // OgreXMLConverter <Name> (1.10.0) unstable
    cout << "OgreXMLConverter " << OGRE_VERSION_NAME << " "
         << "(" << OGRE_VERSION_MAJOR << "." << OGRE_VERSION_MINOR << "." << OGRE_VERSION_PATCH << ")"
         << " " << OGRE_VERSION_SUFFIX << endl;
}

void help(void)
{
    // Print help message
    cout << endl << "OgreXMLConvert: Converts data between XML and OGRE binary formats." << endl;
    cout << "Provided for OGRE by Steve Streeting" << endl << endl;
    cout << "Usage: OgreXMLConverter [options] sourcefile [destfile] " << endl;
    cout << endl << "Available options:" << endl;
    cout << "-v             = Display version information" << endl;
    cout << "-merge [n0,n1] = Merge texcoordn0 with texcoordn1. The , separator must be" << endl;
    cout << "                 present, otherwise only n0 is provided assuming n1 = n0+1;" << endl;
    cout << "                 n0 and n1 must be in the same buffer source & adjacent" << endl;
    cout << "                 to each other for the merge to work." << endl;
    cout << "-o             = DON'T optimise out redundant tracks & keyframes" << endl;
    cout << "-d3d           = Prefer D3D packed colour formats (default on Windows)" << endl;
    cout << "-gl            = Prefer GL packed colour formats (default on non-Windows)" << endl;
    cout << "-E endian      = Set endian mode 'big' 'little' or 'native' (default)" << endl;
    cout << "-x num         = Generate no more than num eXtremes for every submesh (default 0)" << endl;
    cout << "-q             = Quiet mode, less output" << endl;
    cout << "-log filename  = name of the log file (default: 'OgreXMLConverter.log')" << endl;
    cout << "sourcefile     = name of file to convert" << endl;
    cout << "destfile       = optional name of file to write to. If you don't" << endl;
    cout << "                 specify this OGRE works it out through the extension " << endl;
    cout << "                 and the XML contents if the source is XML. For example" << endl;
    cout << "                 test.mesh becomes test.xml, test.xml becomes test.mesh " << endl;
    cout << "                 if the XML document root is <mesh> etc."  << endl;

    cout << endl;
}


XmlOptions parseArgs(int numArgs, char **args)
{
    XmlOptions opts;

    //opts.interactiveMode = false;
    //opts.lodValue = 250000;
    //opts.lodFixed = 0;
    //opts.lodPercent = 20;
    //opts.numLods = 0;
    opts.nuextremityPoints = 0;
    opts.mergeTexcoordResult = 0;
    opts.mergeTexcoordToDestroy = 0;
    opts.usePercent = true;
    //opts.generateEdgeLists = true;
    //opts.generateTangents = false;
    //opts.tangentSemantic = VES_TANGENT;
    //opts.tangentUseParity = false;
    //opts.tangentSplitMirrored = false;
    //opts.tangentSplitRotated = false;
    //opts.reorganiseBuffers = true;
    opts.optimiseAnimations = true;
    opts.quietMode = false;
    opts.endian = Serializer::ENDIAN_NATIVE;

    // ignore program name
    char* source = 0;
    char* dest = 0;

    // Set up options
    UnaryOptionList unOpt;
    BinaryOptionList binOpt;

    //unOpt["-i"] = false;
    //unOpt["-e"] = false;
    unOpt["-r"] = false;
    //unOpt["-t"] = false;
    unOpt["-tm"] = false;
    unOpt["-tr"] = false;
    unOpt["-o"] = false;
    unOpt["-q"] = false;
    unOpt["-d3d"] = false;
    unOpt["-gl"] = false;
    unOpt["-h"] = false;
    unOpt["-v"] = false;
    //binOpt["-l"] = "";
    //binOpt["-s"] = "Distance";
    //binOpt["-p"] = "";
    //binOpt["-f"] = "";
    binOpt["-E"] = "";
    binOpt["-x"] = "";
    binOpt["-log"] = "OgreXMLConverter.log";
    binOpt["-td"] = "";
    binOpt["-ts"] = "";
    binOpt["-merge"] = "0,0";

    int startIndex = findCommandLineOpts(numArgs, args, unOpt, binOpt);
    UnaryOptionList::iterator ui;
    BinaryOptionList::iterator bi;

    ui = unOpt.find("-v");
    if (ui->second)
    {
        print_version();
        exit(0);
    }

    ui = unOpt.find("-h");
    if (ui->second)
    {
        help();
        exit(1);
    }

    ui = unOpt.find("-q");
    if (ui->second)
    {
        opts.quietMode = true;
    }

        ui = unOpt.find("-o");
        if (ui->second)
        {
            opts.optimiseAnimations = false;
        }

        bi = binOpt.find("-merge");
        if (!bi->second.empty())
        {
            String::size_type separator = bi->second.find_first_of( "," );
            if( separator == String::npos )
            {
                //Input format was "-merge 2"
                //Assume we want to merge 2 with 3
                opts.mergeTexcoordResult    = StringConverter::parseInt( bi->second, 0 );
                opts.mergeTexcoordToDestroy = opts.mergeTexcoordResult + 1;
            }
            else if( separator + 1 < bi->second.size() )
            {
                //Input format was "-merge 1,2"
                //We want to merge 1 with 2
                opts.mergeTexcoordResult    = StringConverter::parseInt(
                                                                bi->second.substr( 0, separator ), 0 );
                opts.mergeTexcoordToDestroy = StringConverter::parseInt(
                                                                bi->second.substr( separator+1,
                                                                bi->second.size() ), 1 );
            }
        }
        else
        {
            //Very rare to reach here.
            //Input format was "-merge"
            //Assume we want to merge 0 with 1
            opts.mergeTexcoordResult = 0;
            opts.mergeTexcoordResult = 1;
        }

        bi = binOpt.find("-x");
        if (!bi->second.empty())
        {
            opts.nuextremityPoints = StringConverter::parseInt(bi->second);
        }

        bi = binOpt.find("-log");
        if (!bi->second.empty())
        {
            opts.logFile = bi->second;
        }

        bi = binOpt.find("-E");
        if (!bi->second.empty())
        {
            if (bi->second == "big")
                opts.endian = Serializer::ENDIAN_BIG;
            else if (bi->second == "little")
                opts.endian = Serializer::ENDIAN_LITTLE;
            else 
                opts.endian = Serializer::ENDIAN_NATIVE;
        }

        ui = unOpt.find("-d3d");
        if (ui->second)
        {
            opts.d3d = true;
        }

        ui = unOpt.find("-gl");
        if (ui->second)
        {
            opts.gl = true;
            opts.d3d = false;
        }

    // Source / dest
    if (numArgs > startIndex)
        source = args[startIndex];
    if (numArgs > startIndex+1)
        dest = args[startIndex+1];
    if (numArgs > startIndex+2) {
        cout << "Too many command-line arguments supplied - abort. " << endl;
        help();
        exit(1);
    }

    if (!source)
    {
        cout << "Missing source file - abort. " << endl;
        help();
        exit(1);
    }
    // Work out what kind of conversion this is
    opts.source = source;
    std::vector<String> srcparts = StringUtil::split(opts.source, ".");
    String& ext = srcparts.back();
    StringUtil::toLowerCase(ext);
    opts.sourceExt = ext;

    if (!dest)
    {
        if (opts.sourceExt == "xml")
        {
            // dest is source minus .xml
            opts.dest = opts.source.substr(0, opts.source.size() - 4);
        }
        else
        {
            // dest is source + .xml
            opts.dest = opts.source;
            opts.dest.append(".xml");
        }

    }
    else
    {
        opts.dest = dest;
    }
    std::vector<String> dstparts = StringUtil::split(opts.dest, ".");
    ext = dstparts.back();
    StringUtil::toLowerCase(ext);
    opts.destExt = ext;

    if (!opts.quietMode) 
    {
        cout << endl;
        cout << "-- OPTIONS --" << endl;
        cout << "source file      = " << opts.source << endl;
        cout << "destination file = " << opts.dest << endl;
            cout << "log file         = " << opts.logFile << endl;
        if (opts.nuextremityPoints)
            cout << "Generate extremes per submesh = " << opts.nuextremityPoints << endl;
        cout << " semantic = " << (opts.tangentSemantic == VES_TANGENT? "TANGENT" : "TEXCOORD") << endl;
        cout << " parity = " << opts.tangentUseParity << endl;
        cout << " split mirror = " << opts.tangentSplitMirrored << endl;
        cout << " split rotated = " << opts.tangentSplitRotated << endl;
        
        cout << "-- END OPTIONS --" << endl;
        cout << endl;
    }


    return opts;
}

// Crappy globals
// NB some of these are not directly used, but are required to
//   instantiate the singletons used in the dlls
LogManager* logMgr = 0;
Math* mth = 0;
LodStrategyManager *lodMgr = 0;
MaterialManager* matMgr = 0;
SkeletonManager* skelMgr = 0;
MeshSerializer* meshSerializer = 0;
XMLMeshSerializer* xmlMeshSerializer = 0;
SkeletonSerializer* skeletonSerializer = 0;
XMLSkeletonSerializer* xmlSkeletonSerializer = 0;
DefaultHardwareBufferManager *bufferManager = 0;
MeshManager* meshMgr = 0;
ResourceGroupManager* rgm = 0;


void meshToXML(XmlOptions opts)
{
    std::ifstream ifs;
    ifs.open(opts.source.c_str(), std::ios_base::in | std::ios_base::binary);

    if (ifs.bad())
    {
        cout << "Unable to load file " << opts.source << endl;
        exit(1);
    }

    // pass false for freeOnClose to FileStreamDataStream since ifs is created on stack
    DataStreamPtr stream(new FileStreamDataStream(opts.source, &ifs, false));

    MeshPtr mesh = MeshManager::getSingleton().create("conversion", 
        ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    

    meshSerializer->importMesh(stream, mesh.get());
   
    xmlMeshSerializer->exportMesh(mesh.get(), opts.dest);

    // Clean up the conversion mesh
    MeshManager::getSingleton().remove("conversion",
                                       ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
}

void XMLToBinary(XmlOptions opts)
{
    // Read root element and decide from there what type
    String response;
    pugi::xml_document doc;

    // Some double-parsing here but never mind
    if (!doc.load_file(opts.source.c_str()))
    {
        cout << "Unable to open file " << opts.source << " - fatal error." << endl;
        exit (1);
    }
    pugi::xml_node root = doc.document_element();
    if (!stricmp(root.name(), "mesh"))
    {
        MeshPtr newMesh = MeshManager::getSingleton().createManual("conversion", 
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        VertexElementType colourElementType;
        if (opts.d3d)
            colourElementType = VET_COLOUR_ARGB;
        else
            colourElementType = VET_COLOUR_ABGR;

        xmlMeshSerializer->importMesh(opts.source, colourElementType, newMesh.get());

        // Re-jig the buffers?
        // Make sure animation types are up to date first
        newMesh->_determineAnimationTypes();
        if (opts.reorganiseBuffers)
        {
            logMgr->logMessage("Reorganising vertex buffers to automatic layout...");
            // Shared geometry
            if (newMesh->sharedVertexData)
            {
                // Automatic
                VertexDeclaration* newDcl = 
                    newMesh->sharedVertexData->vertexDeclaration->getAutoOrganisedDeclaration(
                        newMesh->hasSkeleton(), newMesh->hasVertexAnimation(), newMesh->getSharedVertexDataAnimationIncludesNormals());
                if (*newDcl != *(newMesh->sharedVertexData->vertexDeclaration))
                {
                    // Usages don't matter here since we're onlly exporting
                    BufferUsageList bufferUsages;
                    for (size_t u = 0; u <= newDcl->getMaxSource(); ++u)
                        bufferUsages.push_back(HardwareBuffer::HBU_STATIC_WRITE_ONLY);
                    newMesh->sharedVertexData->reorganiseBuffers(newDcl, bufferUsages);
                }
            }
            // Dedicated geometry
            for (size_t i = 0; i < newMesh->getNumSubMeshes(); i++)
            {
                SubMesh* sm = newMesh->getSubMesh(i);
                if (!sm->useSharedVertices)
                {
                    const bool hasVertexAnim = sm->getVertexAnimationType() != Ogre::VAT_NONE;

                    // Automatic
                    VertexDeclaration* newDcl = 
                        sm->vertexData->vertexDeclaration->getAutoOrganisedDeclaration(
                            newMesh->hasSkeleton(), hasVertexAnim, sm->getVertexAnimationIncludesNormals());
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

        if( opts.mergeTexcoordResult != opts.mergeTexcoordToDestroy )
        {
            newMesh->mergeAdjacentTexcoords( uint16(opts.mergeTexcoordResult), uint16(opts.mergeTexcoordToDestroy) );
        }

        if (opts.nuextremityPoints)
        {
            for (size_t i = 0; i < newMesh->getNumSubMeshes(); i++)
            {
                SubMesh* sm = newMesh->getSubMesh(i);
                sm->generateExtremes (opts.nuextremityPoints);
            }
        }

        meshSerializer->exportMesh(newMesh.get(), opts.dest, opts.endian);

        // Clean up the conversion mesh
        MeshManager::getSingleton().remove("conversion",
                                           ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    }
    else if (!stricmp(root.name(), "skeleton"))
    {
        SkeletonPtr newSkel = SkeletonManager::getSingleton().create("conversion", 
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        xmlSkeletonSerializer->importSkeleton(opts.source, newSkel.get());
        if (opts.optimiseAnimations)
        {
            newSkel->optimiseAllAnimations();
        }
        skeletonSerializer->exportSkeleton(newSkel.get(), opts.dest, SKELETON_VERSION_LATEST, opts.endian);

        // Clean up the conversion skeleton
        SkeletonManager::getSingleton().remove("conversion",
                                               ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    }
}

void skeletonToXML(XmlOptions opts)
{

    std::ifstream ifs;
    ifs.open(opts.source.c_str(), std::ios_base::in | std::ios_base::binary);
    if (ifs.bad())
    {
        cout << "Unable to load file " << opts.source << endl;
        exit(1);
    }

    SkeletonPtr skel = SkeletonManager::getSingleton().create("conversion", 
        ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    // pass false for freeOnClose to FileStreamDataStream since ifs is created locally on stack
    DataStreamPtr stream(new FileStreamDataStream(opts.source, &ifs, false));
    skeletonSerializer->importSkeleton(stream, skel.get());
   
    xmlSkeletonSerializer->exportSkeleton(skel.get(), opts.dest);

    // Clean up the conversion skeleton
    SkeletonManager::getSingleton().remove("conversion",
                                           ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
}

struct MaterialCreator : public MeshSerializerListener
{
    void processMaterialName(Mesh *mesh, String *name)
    {
        // create material because we do not load any .material files
        MaterialManager::getSingleton().create(*name, mesh->getGroup());
    }

    void processSkeletonName(Mesh *mesh, String *name) {}
    void processMeshCompleted(Mesh *mesh) {}
};
}

int main(int numargs, char** args)
{
    if (numargs < 2)
    {
        help();
        return -1;
    }

    // Assume success
    int retCode = 0;

    try 
    {
        logMgr = new LogManager();

        // this log catches output from the parseArgs call and routes it to stdout only
        logMgr->createLog("Temporary log", false, true, true); 

        XmlOptions opts = parseArgs(numargs, args);
        // use the log specified by the cmdline params
        logMgr->setDefaultLog(logMgr->createLog(opts.logFile, false, !opts.quietMode)); 
        // get rid of the temporary log as we use the new log now
        logMgr->destroyLog("Temporary log");

        rgm = new ResourceGroupManager();
        mth = new Math();
        lodMgr = new LodStrategyManager();
        meshMgr = new MeshManager();
        matMgr = new MaterialManager();
        matMgr->initialise();
        skelMgr = new SkeletonManager();
        meshSerializer = new MeshSerializer();
        MaterialCreator matCreator;
        meshSerializer->setListener(&matCreator);
        xmlMeshSerializer = new XMLMeshSerializer();
        skeletonSerializer = new SkeletonSerializer();
        xmlSkeletonSerializer = new XMLSkeletonSerializer();
        bufferManager = new DefaultHardwareBufferManager(); // needed because we don't have a rendersystem



        if (opts.sourceExt == "mesh")
        {
            meshToXML(opts);
        }
        else if (opts.sourceExt == "skeleton")
        {
            skeletonToXML(opts);
        }
        else if (opts.sourceExt == "xml")
        {
            XMLToBinary(opts);
        }
        else
        {
            cout << "Unknown input type.\n";
            retCode = 1;
        }

    }
    catch(Exception& e)
    {
        LogManager::getSingleton().logError(e.getDescription());
        retCode = 1;
    }

    Pass::processPendingPassUpdates(); // make sure passes are cleaned up

    delete xmlSkeletonSerializer;
    delete skeletonSerializer;
    delete xmlMeshSerializer;
    delete meshSerializer;
    delete skelMgr;
    delete matMgr;
    delete meshMgr;
    delete bufferManager;
    delete lodMgr;
    delete mth;
    delete rgm;
    delete logMgr;

    return retCode;

}

