#include "RandomTools.h"
#include "OgreTextureManager.h"
#include "OgreHardwarePixelBuffer.h"
using namespace Ogre;

static const int NUM_RAND_VALUES = 1024;

TexturePtr RandomTools::generateRandomVelocityTexture()
{
	TexturePtr texPtr = TextureManager::getSingleton().createManual("RandomVelocityTexture", 
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		TEX_TYPE_1D, 1024, 1, 1, 1, PF_FLOAT32_RGBA, TU_DYNAMIC);

	HardwarePixelBufferSharedPtr pixelBuf = texPtr->getBuffer();

	float randomData[NUM_RAND_VALUES*4];
	for(int i=0; i<NUM_RAND_VALUES*4; i++)
    {
        randomData[i] = float( (rand()%10000) - 5000 );
    }

	PixelBox pixelBox(1024, 1, 1, PF_FLOAT32_RGBA, &randomData[0]);
	pixelBuf->blitFromMemory(pixelBox);
	return texPtr;
}
