
#include "OgrePrerequisites.h"

namespace Demo
{
    class MiscUtils
    {
        static int retrievePreprocessorParameter( const Ogre::String &preprocessDefines,
                                                  const Ogre::String &paramName );
    public:
        /**
        @param job
            The compute job to change.
        @param kernelRadius
            The kernel radius. A radius of 8 means there's 17x17 taps (8x2 + 1).
            Changing this parameter will trigger a recompile.
        @param gaussianDeviationFactor
            The std. deviation of a gaussian filter.
        @param K
            A big K (K > 20) means softer, blurrier shadows; but may reveal some artifacts on the
            edge cases ESM does not deal well, while small K (< 8) may hide those artifacts,
            but makes the shadows fat (fatter than normal, still blurry) and very dark.
            Small K combined with large radius (radius > 8) may cause self shadowing artifacts.
            A K larger than 80 may run into floating point precision issues (which means light
            bleeding or weird shadows appearing caused by NaNs).
            Changing this parameter will trigger a recompile.
        */
        static void setGaussianLogFilterParams( Ogre::HlmsComputeJob *job, Ogre::uint8 kernelRadius,
                                                float gaussianDeviationFactor, Ogre::uint16 K );

        /// Adjusts the material (pixel shader variation). See other overload for param description.
        static void setGaussianLogFilterParams( const Ogre::String &materialName, Ogre::uint8 kernelRadius,
                                                float gaussianDeviationFactor, Ogre::uint16 K );
    };
}
