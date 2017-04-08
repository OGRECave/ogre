
#include "OgrePrerequisites.h"

namespace Demo
{
    class MiscUtils
    {
        static int retrievePreprocessorParameter( const Ogre::String &preprocessDefines,
                                                  const Ogre::String &paramName );
    public:
        static void setGaussianFilterParams( Ogre::HlmsComputeJob *job, Ogre::uint8 kernelRadius,
                                             float gaussianDeviationFactor );
        static void setGaussianFilterParams( const Ogre::String &materialName, Ogre::uint8 kernelRadius,
                                             float gaussianDeviationFactor );
    };
}
