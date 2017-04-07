
#include "OgrePrerequisites.h"

namespace Demo
{
	class MiscUtils
	{
	public:
		static void setGaussianFilterParams( Ogre::HlmsComputeJob *job, Ogre::uint8 kernelRadius,
											 float gaussianDeviationFactor );
	};
}
