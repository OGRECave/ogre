#ifndef __Sample_ParticleGS_H__
#define __Sample_ParticleGS_H__


#include "ProceduralManualObject.h"
#include "OgreRenderToVertexBuffer.h"
#include "RandomTools.h"
#include "SamplePlugin.h"
#include "SdkSample.h"

// #define LOG_GENERATED_BUFFER
namespace OgreBites {
using namespace Ogre;

class _OgreSampleClassExport Sample_ParticleGS : public SdkSample
{
 public:
    Sample_ParticleGS();

 protected:

    void createProceduralParticleSystem();
    void testCapabilities(const RenderSystemCapabilities* caps) override;
    void setupContent(void) override;
    void cleanupContent() override;
#ifdef LOG_GENERATED_BUFFER
    bool frameEnded(const FrameEvent& evt);
#endif
    ProceduralManualObject* mParticleSystem;
};
}
#endif
