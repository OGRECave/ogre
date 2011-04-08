#include "RandomTools.h"
#include "OgreTextureManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreRoot.h"
using namespace Ogre;

static const int NUM_RAND_VALUES = 1024;

TexturePtr RandomTools::generateRandomVelocityTexture()
{
	// PPP: Temp workaround for DX 11 which does not seem to like usage dynamic
	TextureUsage usage = (Root::getSingletonPtr()->getRenderSystem()->getName()=="Direct3D11 Rendering Subsystem") ? 
		TU_DEFAULT : TU_DYNAMIC;
	TexturePtr texPtr = TextureManager::getSingleton().createManual("RandomVelocityTexture", 
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		TEX_TYPE_1D, 1024, 1, 1, 0, PF_FLOAT32_RGBA, usage);

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
