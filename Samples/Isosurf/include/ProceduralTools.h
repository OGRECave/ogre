#ifndef __PROCEDURAL_TOOLS_H__
#define __PROCEDURAL_TOOLS_H__

#include "OgrePrerequisites.h"
#include "OgreMesh.h"

//A class containing utility methods to generate procedural content required by the demo
class ProceduralTools
{
public:
    static Ogre::MeshPtr generateTetrahedra();
};

#endif
