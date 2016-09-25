
#include "OgrePrerequisites.h"

namespace Demo
{
    class MeshUtils
    {
    public:
        /** Opens a v1 mesh with the given name and imports it to a v2 mesh
            using the same name.
            The v1 mesh will be unloaded.
        @param meshName
            Name of the mesh to open.
        @param groupName
            Group of the mesh it resides in.
        */
        static void importV1Mesh( const Ogre::String &meshName, const Ogre::String &groupName );
    };
}
