#include "RandomTools.h"
#include "OgreTextureManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreRoot.h"
using namespace Ogre;

static const int NUM_RAND_VALUES = 1024;

TexturePtr RandomTools::generateRandomVelocityTexture()
{
    TexturePtr texPtr = TextureManager::getSingleton().createManual(
        "RandomVelocityTexture",
        RGN_DEFAULT,
        TEX_TYPE_1D, 
        1024, 1, 1, 
        0, 
        PF_FLOAT32_RGBA, HBU_CPU_TO_GPU);

    HardwarePixelBufferSharedPtr pixelBuf = texPtr->getBuffer();

    // Lock the buffer so we can write to it.
    pixelBuf->lock(HardwareBuffer::HBL_DISCARD);
    const PixelBox &pb = pixelBuf->getCurrentLock();
    
    float *randomData = reinterpret_cast<float*>(pb.data);
    // float randomData[NUM_RAND_VALUES * 4];
    for(int i = 0; i < NUM_RAND_VALUES * 4; i++)
    {
        randomData[i] = float( (rand() % 10000) - 5000 );
    }

    // PixelBox pixelBox(1024, 1, 1, PF_FLOAT32_RGBA, &randomData[0]);
    // pixelBuf->blitFromMemory(pixelBox);

    pixelBuf->unlock();

    return texPtr;
}
