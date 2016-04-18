
#include "Utils/MeshUtils.h"

#include "OgreMeshManager.h"
#include "OgreMesh.h"
#include "OgreMeshManager2.h"
#include "OgreMesh2.h"

namespace Demo
{
    void MeshUtils::importV1Mesh( const Ogre::String &meshName, const Ogre::String &groupName )
    {
        Ogre::v1::MeshPtr v1Mesh;
        Ogre::MeshPtr v2Mesh;

        v1Mesh = Ogre::v1::MeshManager::getSingleton().load( meshName, groupName,
                    Ogre::v1::HardwareBuffer::HBU_STATIC, Ogre::v1::HardwareBuffer::HBU_STATIC );

        //Create a v2 mesh to import to, with the same name (arbitrary).
        v2Mesh = Ogre::MeshManager::getSingleton().createManual( meshName, groupName );

        v2Mesh->importV1( v1Mesh.get(), true, true, true );

        //Free memory
        v1Mesh->unload();
        Ogre::v1::MeshManager::getSingleton().remove( meshName );
    }
    //-----------------------------------------------------------------------------------
}
