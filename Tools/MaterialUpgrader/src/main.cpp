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
#include "OldMaterialReader.h"
#include "OgreMaterialManager.h"
#include "OgreMaterialSerializer.h"

#include <iostream>
#include <sys/stat.h>

using namespace std;

void help(void)
{
    // Print help message
    cout << endl << "OgreMaterialUpgrader: Upgrades .material files to the latest version." << endl;
    cout << "Provided for OGRE by Steve Streeting 2003" << endl << endl;
    cout << "Usage: OgreMaterialUpgrade sourcefile [destfile] " << endl;
    cout << "sourcefile = name of file to convert" << endl;
    cout << "destfile   = optional name of file to write to. If you don't" << endl;
    cout << "             specify this OGRE overwrites the existing file." << endl;

    cout << endl;
}


using namespace Ogre;

// Crappy globals
// NB some of these are not directly used, but are required to
//   instantiate the singletons used in the dlls
LogManager* logMgr;
Math* mth;
ResourceGroupManager* resGrpMgr;
MaterialManager* matMgr;

int main(int numargs, char** args)
{
    if (numargs < 2)
    {
        help();
        return -1;
    }

    logMgr = new LogManager();
	logMgr->createLog("OgreMaterialUpgrade.log", true);
    mth = new Math();
    resGrpMgr = new ResourceGroupManager();
    matMgr = new MaterialManager();
    matMgr->initialise();

    String source(args[1]);

    // Load the material
    struct stat tagStat;

    FILE* pFile = fopen( source.c_str(), "r" );
    if (!pFile)
    {
        OGRE_EXCEPT(Exception::ERR_FILE_NOT_FOUND, 
            "File " + source + " not found.", "OgreMaterialUpgrade");
    }
    stat( source.c_str(), &tagStat );
    MemoryDataStream* memStream = new MemoryDataStream(source, tagStat.st_size, true);
    fread( (void*)memStream->getPtr(), tagStat.st_size, 1, pFile );
    fclose( pFile );

    // Read script, note this will create potentially many Materials and load them
    // into the MaterialManager
    OldMaterialReader reader;
    DataStreamPtr stream(memStream);
    reader.parseScript(stream);

    // Write out the converted mesh
    String dest;
    if (numargs == 3)
    {
        dest = args[2];
    }
    else
    {
        dest = source;
    }


    MaterialSerializer serializer;
    ResourceManager::ResourceMapIterator it = matMgr->getResourceIterator();
    while (it.hasMoreElements())
    {
        MaterialPtr m = it.getNext();
        // Skip builtin materials
        if (m->getName() == "BaseWhite" || m->getName() == "BaseWhiteNoLighting")
            continue;
        serializer.queueForExport(m);
    }
    serializer.exportQueued(dest);
    
    delete matMgr;
    delete resGrpMgr;
    delete mth;
    delete logMgr;

    return 0;

}

