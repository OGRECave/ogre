/*
-----------------------------------------------------------------------------
This source file is part of
                                    _
  ___   __ _ _ __ ___  __ _ ___ ___(_)_ __ ___  _ __
 / _ \ / _` | '__/ _ \/ _` / __/ __| | '_ ` _ \| '_ \
| (_) | (_| | | |  __/ (_| \__ \__ \ | | | | | | |_) |
 \___/ \__, |_|  \___|\__,_|___/___/_|_| |_| |_| .__/
       |___/                                   |_|

For the latest info, see https://bitbucket.org/jacmoe/ogreassimp

Copyright (c) 2011 Jacob 'jacmoe' Moen

Licensed under the MIT license:

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
#include <iostream>

#include "Ogre.h"

#include "OgreMeshSerializer.h"
#include "OgreSkeletonSerializer.h"
#include "OgreDefaultHardwareBufferManager.h"
#include "OgreLodStrategyManager.h"
#include "OgreScriptCompiler.h"

#include "OgreAssimpLoader.h"
#include <assimp/postprocess.h>

using namespace Ogre;

namespace
{
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
DefaultHardwareBufferManager* bufferManager = 0;
MeshManager* meshMgr = 0;
ResourceGroupManager* rgm = 0;
ScriptCompilerManager* scmgr = 0;
DefaultTextureManager* texMgr = 0;

void help(void)
{
    // Print help message
    std::cout << std::endl << "OgreAssimpConverter: Converts data from model formats supported by Assimp" << std::endl;
    std::cout << "to OGRE binary formats (mesh and skeleton) and material script." << std::endl;
    std::cout << std::endl << "Usage: OgreAssimpConverter [options] sourcefile [destination] " << std::endl;
    std::cout << std::endl << "Available options:" << std::endl;
    std::cout << "-q                  = Quiet mode, less output" << std::endl;
    std::cout << "-log filename       = name of the log file (default: 'OgreAssimp.log')" << std::endl;
    std::cout << "-aniSpeedMod value  = Factor to scale the animation speed - (default: '1.0')" << std::endl;
    std::cout << "                      (double between 0 and 1)" << std::endl;
    std::cout << "-3ds_ani_fix        = Fix for the fact that 3ds max exports the animation over a" << std::endl;
    std::cout << "                      longer time frame than the animation actually plays for" << std::endl;
    std::cout << "-max_edge_angle deg = When normals are generated, max angle between two faces to smooth over" << std::endl;
    std::cout << "sourcefile          = name of file to convert" << std::endl;
    std::cout << "destination         = optional name of directory to write to. If you don't" << std::endl;
    std::cout << "                      specify this the converter will use the same directory as the sourcefile."  << std::endl;
    std::cout << std::endl;
}

struct AssOptions
{
    String source;
    String dest;
    String logFile;

    AssimpLoader::Options options;

    AssOptions() { logFile = "OgreAssimp.log"; };
};

AssOptions parseArgs(int numArgs, char** args)
{
    AssOptions opts;

    // ignore program name
    char* source = 0;
    char* dest = 0;

    // Set up options
    UnaryOptionList unOpt;
    BinaryOptionList binOpt;

    unOpt["-q"] = false;
    unOpt["-3ds_ani_fix"] = false;
    binOpt["-log"] = opts.logFile;
    binOpt["-aniName"] = "";
    binOpt["-aniSpeedMod"] = "1.0";
    binOpt["-max_edge_angle"] = "30";

    int startIndex = findCommandLineOpts(numArgs, args, unOpt, binOpt);

    if (unOpt["-q"])
    {
        opts.options.params |= AssimpLoader::LP_QUIET_MODE;
    }
    if (unOpt["-3ds_ani_fix"])
    {
        opts.options.params |= AssimpLoader::LP_CUT_ANIMATION_WHERE_NO_FURTHER_CHANGE;
    }

    opts.options.postProcessSteps = aiProcessPreset_TargetRealtime_Quality;
    opts.logFile = binOpt["-log"];
    StringConverter::parse(binOpt["-aniSpeedMod"], opts.options.animationSpeedModifier);
    opts.options.customAnimationName = binOpt["-aniName"];
    StringConverter::parse(binOpt["-max_edge_angle"], opts.options.maxEdgeAngle);

    // Source / dest
    if (numArgs > startIndex)
        source = args[startIndex];
    if (numArgs > startIndex + 1)
        dest = args[startIndex + 1];
    if (numArgs > startIndex + 2)
    {
        logMgr->logError("Too many command-line arguments supplied");
        help();
        exit(1);
    }

    if (!source)
    {
        logMgr->logError("Missing source file");
        help();
        exit(1);
    }
    opts.source = source;

    if (dest)
    {
        opts.dest = dest;
    }

    if (!unOpt["-q"])
    {
        std::cout << std::endl;
        std::cout << "-- OPTIONS --" << std::endl;

        std::cout << "source file               = " << opts.source << std::endl;
        std::cout << "destination               = " << opts.dest << std::endl;
        std::cout << "animation speed modifier  = " << opts.options.animationSpeedModifier << std::endl;
        std::cout << "log file                  = " << opts.logFile << std::endl;

        std::cout << "-- END OPTIONS --" << std::endl;
        std::cout << std::endl;
    }

    return opts;
}
} // namespace

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

        AssOptions opts = parseArgs(numargs, args);
        // use the log specified by the cmdline params
        logMgr->setDefaultLog(logMgr->createLog(opts.logFile, false, true));
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
        skeletonSerializer = new SkeletonSerializer();
        bufferManager = new DefaultHardwareBufferManager(); // needed because we don't have a rendersystem
        scmgr = new ScriptCompilerManager();
        texMgr = new DefaultTextureManager();

        String basename, ext, path;
        StringUtil::splitFullFilename(opts.source, basename, ext, path);

        MeshPtr mesh = MeshManager::getSingleton().createManual(basename + "." + ext, RGN_DEFAULT);
        SkeletonPtr skeleton;

        AssimpLoader loader;
        loader.load(opts.source, mesh.get(), skeleton, opts.options);

        if (!opts.dest.empty())
        {
            path = opts.dest + "/";
        }

        MeshSerializer meshSer;
        meshSer.exportMesh(mesh.get(), path + basename + ".mesh");

        if (skeleton)
        {
            SkeletonSerializer binSer;
            binSer.exportSkeleton(skeleton.get(), path + skeleton->getName());
        }

        // serialise the materials
        std::set<String> exportNames;
        for (SubMesh* sm : mesh->getSubMeshes())
            exportNames.insert(sm->getMaterialName());

        // queue up the materials for serialise
        MaterialSerializer ms;
        for (const String& name : exportNames)
            ms.queueForExport(MaterialManager::getSingleton().getByName(name));

        if (!exportNames.empty())
            ms.exportQueued(path + basename + ".material");
    }
    catch (Exception& e)
    {
        logMgr->logError(e.getDescription() + " Aborting!");
        retCode = 1;
    }

    delete skeletonSerializer;
    delete meshSerializer;
    delete matMgr;
    delete meshMgr;
    delete skelMgr;
    delete bufferManager;
    delete scmgr;
    delete lodMgr;
    delete mth;
    delete rgm;
    delete logMgr;

    return retCode;
}
