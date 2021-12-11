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
#include "OgreLodStrategyManager.h"
#include <iostream>

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
    size_t nuextremityPoints;
    size_t mergeTexcoordResult;
    size_t mergeTexcoordToDestroy;
    bool optimiseAnimations;
    bool quietMode;
    VertexElementType colourElementType;
    Serializer::Endian endian;
};

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
    cout << "-d3d           = Use packed argb colour format (default on Windows)" << endl;
    cout << "-gl            = Use packed abgr colour format (default on non-Windows)" << endl;
    cout << "-byte          = Use ubyte4 colour format (default since 1.13)" << endl;
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

    opts.nuextremityPoints = 0;
    opts.mergeTexcoordResult = 0;
    opts.mergeTexcoordToDestroy = 0;
    opts.optimiseAnimations = true;
    opts.quietMode = false;
    opts.endian = Serializer::ENDIAN_NATIVE;
    opts.colourElementType = VET_COLOUR;

    // ignore program name
    char* source = 0;
    char* dest = 0;

    // Set up options
    UnaryOptionList unOpt;
    BinaryOptionList binOpt;

    unOpt["-o"] = false;
    unOpt["-q"] = false;
    unOpt["-d3d"] = false;
    unOpt["-gl"] = false;
    unOpt["-byte"] = false;
    unOpt["-h"] = false;
    unOpt["-v"] = false;
    binOpt["-E"] = "";
    binOpt["-x"] = "";
    binOpt["-log"] = "OgreXMLConverter.log";
    binOpt["-merge"] = "0,0";

    int startIndex = findCommandLineOpts(numArgs, args, unOpt, binOpt);

    if (unOpt["-v"])
    {
        print_version();
        exit(0);
    }
    if (unOpt["-h"])
    {
        help();
        exit(1);
    }
    if (unOpt["-q"])
    {
        opts.quietMode = true;
    }
    if (unOpt["-o"])
    {
        opts.optimiseAnimations = false;
    }

    auto bi = binOpt.find("-merge");
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

    opts.logFile = binOpt["-log"];

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

    if (unOpt["-d3d"])
    {
        opts.colourElementType = VET_COLOUR_ARGB;
    }

    if (unOpt["-gl"])
    {
        opts.colourElementType = VET_COLOUR_ABGR;
    }

    if (unOpt["-byte"])
    {
        opts.colourElementType = VET_UBYTE4_NORM;
    }

    // Source / dest
    if (numArgs > startIndex)
        source = args[startIndex];
    if (numArgs > startIndex+1)
        dest = args[startIndex+1];
    if (numArgs > startIndex+2) {
        logMgr->logError("Too many command-line arguments supplied");
        exit(1);
    }

    if (!source)
    {
        logMgr->logError("Missing source file");
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
        
        cout << "-- END OPTIONS --" << endl;
        cout << endl;
    }


    return opts;
}

void meshToXML(XmlOptions opts)
{
    std::ifstream ifs;
    ifs.open(opts.source.c_str(), std::ios_base::in | std::ios_base::binary);

    if (!ifs.good())
    {
        logMgr->logError("Unable to load file " + opts.source);
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
        logMgr->logError("Unable to load file " + opts.source);
        exit (1);
    }
    pugi::xml_node root = doc.document_element();
    if (StringUtil::startsWith("mesh", root.name()))
    {
        MeshPtr newMesh = MeshManager::getSingleton().createManual("conversion", 
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

        xmlMeshSerializer->importMesh(opts.source, opts.colourElementType, newMesh.get());

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
    else if (StringUtil::startsWith("skeleton", root.name()))
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
    if (!ifs.good())
    {
        logMgr->logError("Unable to load file " + opts.source);
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

struct MeshResourceCreator : public MeshSerializerListener
{
    void processMaterialName(Mesh *mesh, String *name)
    {
        if (name->empty())
        {
            LogManager::getSingleton().logWarning("one of the SubMeshes is using an empty material name. "
                                                  "See https://ogrecave.github.io/ogre/api/latest/_mesh-_tools.html#autotoc_md32");
            // here, we explicitly want to allow fixing that
            return;
        }

        // create material because we do not load any .material files
        MaterialManager::getSingleton().createOrRetrieve(*name, mesh->getGroup());
    }

    void processSkeletonName(Mesh *mesh, String *name)
    {
        if (name->empty())
        {
            LogManager::getSingleton().logWarning("the mesh is using an empty skeleton name.");
            // here, we explicitly want to allow fixing that
            return;
        }

        // create skeleton because we do not load any .skeleton files
        SkeletonManager::getSingleton().createOrRetrieve(*name, mesh->getGroup(), true);
    }
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
        MeshResourceCreator resCreator;
        meshSerializer->setListener(&resCreator);
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
            logMgr->logError("Unknown input type: " + opts.sourceExt);
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

