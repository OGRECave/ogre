
#include "OgrePrerequisites.h"

namespace Demo
{
    class HdrUtils
    {
    public:
        static void init( Ogre::uint8 fsaa );

        static void setSkyColour( const Ogre::ColourValue &colour,
                                  float multiplier );

        /** Modifies the HDR Materials for the new exposure parameters
            By default the HDR implementation will try to auto adjust the
            exposure based on the scene's average luminance.
        @par
            If left unbounded, even the darkest scenes can look well lit
            and the brigthest scenes appear too normal.
        @par
            These parameters are useful to prevent the auto exposure from
            jumping too much from one extreme to the other and provide
            a consistent experience within the same lighting conditions.
            (e.g. you may want to change the params when going from indoors to
            outdoors)
        @par
            The smaller the gap between minAutoExposure & maxAutoExposure, the
            less the auto exposure tries to auto adjust to the scene's lighting
            conditions.
        @param exposure
            Exposure in EV. Valid range is [-inf; inf]
            Low values will make the picture darker.
            Higher values will make the picture brighter.
        @param minAutoExposure
            It's in EV stops. Valid range is [-inf; inf]
            Must be minAutoExposure <= maxAutoExposure
            Controls how much auto exposure darkens a bright scene.

            To prevent that looking at a very bright object makes the rest of
            the scene really dark, use higher values.
        @param maxAutoExposure
            It's in EV stops. Valid range is [-inf; inf]
            Must be minAutoExposure <= maxAutoExposure
            Controls how much auto exposure brightens a dark scene.

            To prevent that looking at a very dark object makes the rest of
            the scene really bright, use lower values.
        */
        static void setExposure( float exposure, float minAutoExposure, float maxAutoExposure );

        /** Controls the bloom intensity.
        @param minThreshold
            Colours darker than minThreshold will not contribute to bloom.
            Higher thresholds reduce the amount of bloom.
            Scale is in lumens / 1024.
        @param fullColourThreshold
            The bloom fades between minThreshold & fullColourThreshold.
            Must be > minThreshold. If it is too close to it, bloom fading
            may be too harsh/sudden.
            Scale is the same as minThreshold.
        */
        static void setBloomThreshold( float minThreshold, float fullColourThreshold );
    };
}
