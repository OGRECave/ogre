
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
                                Ogre::RenderSystem *renderSystem );

        void update( Ogre::Camera *camera );

        /** Setups SSR materials and compositor nodes according to settings.
            Must be called BEFORE setting up the workspace. Do not call it
            while a workspace is using the SSR nodes.
        @remarks
            Take note it's a static function, so it can be called anytime
        @param useMsaa
            Whether the final render target we'll be rendering to uses MSAA.
            Note that MSAA solves geometric aliasing, but it won't fix shader
            aliasing (i.e. in this case the aliasing caused by our reflections)
        @param useHq
            True to use high quality. Quite slower, but much higher quality with lower
            aliasing artifacts. A must have if high fidelity is what you are after.
            False to use low quality if you don't mind the aliasing and love the speed
            bump. When false, we run the raytracing at half resolution and try to reconstruct
            (interpolate) the raytraced data which is the reason of the quality/speed trade off.
        @param compositorManager
        */
        static void setupSSR( bool useMsaa, bool useHq, Ogre::CompositorManager2 *compositorManager );
    };
}
