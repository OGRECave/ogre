#ifndef __Sample_Isosurf_H__
#define __Sample_Isosurf_H__

#include "SdkSample.h"
#include "SamplePlugin.h"
#include "ProceduralTools.h"

namespace OgreBites {
using namespace Ogre;

class _OgreSampleClassExport Sample_Isosurf : public SdkSample
{
    Entity* tetrahedra;
    MeshPtr mTetrahedraMesh;

 public:
    Sample_Isosurf();
    void testCapabilities(const RenderSystemCapabilities* caps) override;
    void setupContent(void) override;
    void cleanupContent() override;
    bool frameRenderingQueued(const FrameEvent& evt) override;
};
}

#endif
