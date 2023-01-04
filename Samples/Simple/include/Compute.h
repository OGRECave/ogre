/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

  You may use this sample code for anything you like, it is not covered by the
  same license as the rest of the engine.
  -----------------------------------------------------------------------------
*/

#ifndef __Compute_H__
#define __Compute_H__

#include "SdkSample.h"
#include "SamplePlugin.h"

namespace OgreBites {
using namespace Ogre;
class _OgreSampleClassExport Sample_Compute : public SdkSample
{
    Entity* mOgreEnt;

 public:
    Sample_Compute();
    void testCapabilities(const RenderSystemCapabilities* caps) override;
    void setupContent(void) override;
};
}
#endif  // end _CompositorDemo_H_
