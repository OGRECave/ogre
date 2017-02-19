
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

        /** SSR algorithm works by "voxelizing" the scene in screen space; and these voxels have a size
            in view space. That means SSR is sensitive to how big these voxels are and the unit of
            measure you're working in. This function lets you control these parameters.
        @param equivalentMetersInCurrentUnit
            By default, parameters of SSR are calibrated for meters, but different values are needed
            if you work e.g. in millimeters or in imperial units.
            This setting controls how much is a meter in your unit in measurement.
            For example if your project is in millimeters, call setupUnitOfMeasurement( 1000 );
            If your project is in yards, call setupUnitOfMeasurement( 1.09361 );
            If your project is in km, call setupUnitOfMeasurement( 0.001 );
            The rest of the values should be fed in meters, we will perform the conversion
            internally.
        @param zThickness
            SSR algorithm works by "voxelizing" the scene in screen space. A ray will reflect if it
            hits the voxel defined by corners [X, Y, Z] and [X+1, Y+1, Z+zThickness] in screen space.
            It will miss the reflection and continue ray marching if it doesn't hit the voxel.
            A small value is good for reflecting close or small objects. A large value is good for
            reflecting distant or big objects, while "missing"/"skipping" small/ close objects.
            Indoor you'll want small values. Outdoors you'll want large values.
            See also zThicknessBias which lets you start with small values but adapt to large ones too.
            This value is always in meters.
        @param zThicknessBiasAmount
            Bias in meters to add to zThickness based on distance to camera.
            As the ray marches further away from the camera, it's likely we will want to reflect big
            and distant objects. This value lets you increase the zThickness
            as objects are away from camera which results in higher quality reflections for far away
            objects.
            Example:
            camera  -------> | zThickn |
                    ---------->   |  < zThickness + bias >  |
                    -------------------->   |  <     zThickness + bias'   >  |
            A value of 0 disables the bias (constant zThickness).
        @param zThicknessBiasStart
            Distance in meters away from camera since we should begin applying zThicknessBiasAmount.
        @param zThicknessBiasEnd
            Distance in meters away from camera at which we apply maximum zThicknessBiasAmount.
        @param maxDistance
            How far should we ray march before giving up if no reflection was found.
            Value is in meters. See also maxSteps
        @param reprojectionMaxDistanceError
            Our SSR algorithm uses the colour buffer from the previous frame to perform reflection by
            reprojecting from current frame to previous frame.
            This can cause artifacts if the framerate is too low or the scene/camera is changing fast.
            We compare depth difference as an heuristic to see if we reprojected correctly.
            This value controls the threshold on how much error we allow before considering we cannot
            reflect a particular pixel because the real data has probably been lost between the
            previous and current frame.
            Value is in meters.
        @param pixelStride
            How many pixels we move per ray march. Increasing this value improves performance
            but degrades quality.
            Value is in pixels. You may want to adapt this value based on resolution.
        @param maxSteps
            How many pixels we traverse before giving up if no reflection was found.
            Low values cause noticeable "sudden termination" of reflections, but limit worst case
            scenario (improving performance).
            Value is in pixels. You may want to adapt this value based on resolution.
        */
        static void setupSSRValues( double equivalentMetersInCurrentUnit,
                                    double zThickness = 0.25,
                                    double zThicknessBiasAmount = 2.0,
                                    double zThicknessBiasStart = 10.0,
                                    double zThicknessBiasEnd = 100.0,
                                    float maxDistance = 10000.0f,
                                    float reprojectionMaxDistanceError = 2.0f,
                                    Ogre::uint16 pixelStride = 4,
                                    Ogre::uint16 maxSteps = 200 );
    };
}
