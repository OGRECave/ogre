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
#include "OgreDefaultHardwareBufferManager.h"

#include "OgreAssimpLoader.h"
#include <assimp/postprocess.h>

#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
#include <OgreShaderGenerator.h>
#endif

using namespace Ogre;

namespace
{

void help(void)
{
    std::cout <<
R"HELP(Usage: OgreAssimpConverter [options] sourcefile [destination]

  Converts 3D-formats supported by assimp to native OGRE formats

Available options:
-q                  = Quiet mode, less output
-log filename       = name of the log file (default: 'OgreAssimp.log')
-aniSpeedMod [0..1] = Factor to scale the animation speed (default: 1.0)
-3ds_ani_fix        = Fix for 3ds max, which exports the animation over a
                      longer time frame than the animation actually plays
-max_edge_angle deg = When normals are generated, max angle between
                      two faces to smooth over
sourcefile          = name of file to convert
destination         = optional name of directory to write to. If you don't
                      specify this the converter will use the same
                      directory as the sourcefile.
)HELP";
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
        LogManager::getSingleton().logError("Too many command-line arguments supplied");
        help();
        exit(1);
    }

    if (!source)
    {
        LogManager::getSingleton().logError("Missing source file");
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

    LogManager logMgr;
    // this log catches output from the parseArgs call and routes it to stdout only
    logMgr.createLog("Temporary log", true, true, true);
    AssOptions opts = parseArgs(numargs, args);

    try
    {
        logMgr.setDefaultLog(NULL); // swallow startup messages
        Root root("", "", "");
        // get rid of the temporary log as we use the new log now
        logMgr.destroyLog("Temporary log");

        // use the log specified by the cmdline params
        logMgr.setDefaultLog(logMgr.createLog(opts.logFile, false, true));

        MaterialManager::getSingleton().initialise();

#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
        RTShader::ShaderGenerator::initialize();
#endif

        DefaultHardwareBufferManager bufferManager; // needed because we don't have a rendersystem
        DefaultTextureManager texMgr;

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
        meshSer.exportMesh(mesh, path + basename + ".mesh");

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
#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
        auto& shadergen = RTShader::ShaderGenerator::getSingleton();
        shadergen.setTargetLanguage("glsl"); // must be valid, but otherwise arbitrary
        shadergen.getRenderState(MSN_SHADERGEN)->setLightCountAutoUpdate(false);
        shadergen.validateScheme(MSN_SHADERGEN);
        ms.addListener(shadergen.getMaterialSerializerListener());
#endif
        for (const String& name : exportNames)
            ms.queueForExport(MaterialManager::getSingleton().getByName(name));

        if (!exportNames.empty())
            ms.exportQueued(path + basename + ".material");

        logMgr.setDefaultLog(NULL); // swallow shutdown messages
    }
    catch (Exception& e)
    {
        LogManager::getSingleton().logError(e.getDescription());
        retCode = 1;
    }

    return retCode;
}
