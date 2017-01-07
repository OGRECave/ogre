
#include "OgrePrerequisites.h"

#include "OgreGpuProgramParams.h"

namespace Demo
{
    class ScreenSpaceReflections
    {
        Ogre::GpuProgramParametersSharedPtr mPsParams;

    public:
        ScreenSpaceReflections();

        void update( Ogre::Camera *camera );
    };
}
