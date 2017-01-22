
#include "OgrePrerequisites.h"

#include "OgreGpuProgramParams.h"

namespace Demo
{
    class ScreenSpaceReflections
    {
        Ogre::GpuProgramParametersSharedPtr mPsParams[2];

    public:
        ScreenSpaceReflections( const Ogre::TexturePtr &globalCubemap );

        void update( Ogre::Camera *camera );
    };
}
