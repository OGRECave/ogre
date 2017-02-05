
#include "OgrePrerequisites.h"

#include "OgreGpuProgramParams.h"
#include "OgreMatrix4.h"

namespace Demo
{
    class ScreenSpaceReflections
    {
        Ogre::GpuProgramParametersSharedPtr mPsParams[2];

        Ogre::Matrix4   mLastUvSpaceViewProjMatrix;
        Ogre::Real      mRsDepthRange;

    public:
        ScreenSpaceReflections( const Ogre::TexturePtr &globalCubemap,
                                Ogre::RenderSystem *renderSystem, bool useMsaa );

        void update( Ogre::Camera *camera );
    };
}
