
#include "OgrePrerequisites.h"

#include "OgreGpuProgramParams.h"
#include "OgreMatrix4.h"

namespace Demo
{
    class ScreenSpaceReflections
    {
        Ogre::GpuProgramParametersSharedPtr mPsParams[2];

        Ogre::Matrix4   mLastUvSpaceViewProjMatrix;

    public:
        ScreenSpaceReflections( const Ogre::TexturePtr &globalCubemap );

        void update( Ogre::Camera *camera );
    };
}
