/*
 -----------------------------------------------------------------------------
 This source file is part of OGRE
 (Object-oriented Graphics Rendering Engine)
 For the latest info, see http://www.ogre3d.org/
 
 Copyright (c) 2000-2014 Torus Knot Software Ltd
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
 */
#include "DefaultSamplesPlugin.h"

#include "OgreComponents.h"

#include "AtomicCounters.h"
#include "BezierPatch.h"
#include "BSP.h"
#ifdef OGRE_BUILD_COMPONENT_BULLET
#include "Bullet.h"
#endif
#include "CameraTrack.h"
#include "CelShading.h"
#include "CharacterSample.h"
#include "Compositor.h"
#include "Compute.h"
#include "CubeMapping.h"
#include "CSMShadows.h"
#include "DeferredShadingDemo.h"
#include "Dot3Bump.h"
#include "DualQuaternion.h"
#include "DynTex.h"
#ifdef OGRE_BUILD_COMPONENT_TERRAIN
#   include "EndlessWorld.h"
#   include "Terrain.h"
#endif
#include "FacialAnimation.h"
#include "Fresnel.h"
#include "Grass.h"
#ifdef HAVE_IMGUI
#include "ImGuiDemo.h"
#endif
#include "Isosurf.h"
#include "Lighting.h"
#include "LightShafts.h"
#ifdef OGRE_BUILD_COMPONENT_MESHLODGENERATOR
#   include "MeshLod.h"
#endif
#include "NewInstancing.h"
#include "OceanDemo.h"
#include "ParticleFX.h"
#include "ParticleGS.h"
#ifdef HAVE_PCZ_PLUGIN
    #include "PCZTestApp.h"
#endif
#include "PBR.h"
#include "RectLight.h"
#include "PNTrianglesTessellation.h"
#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
#   include "ShaderSystem.h"
#   include "ShaderSystemTexturedFog.h"
#   include "ShaderSystemMultiLight.h"
#endif
#include "Shadows.h"
#include "SkeletalAnimation.h"
#include "SkyBox.h"
#include "SkyDome.h"
#include "SkyPlane.h"
#include "Smoke.h"
#include "SphereMapping.h"
#include "SSAO.h"
#include "Tessellation.h"
#include "TextureArray.h"
#include "TextureFX.h"
#include "Transparency.h"
#ifdef OGRE_BUILD_COMPONENT_VOLUME
#   include "VolumeCSG.h"
#   include "VolumeTerrain.h"
#endif
#include "VolumeTex.h"
#include "Water.h"

using namespace Ogre;
using namespace OgreBites;

DefaultSamplesPlugin::DefaultSamplesPlugin() : SamplePlugin("DefaultSamplesPlugin")
{
    addSample(new Sample_AtomicCounters);
    addSample(new Sample_BezierPatch);
#ifdef OGRE_BUILD_COMPONENT_BULLET
    addSample(new Sample_Bullet);
#endif
    addSample(new Sample_CameraTrack);
    addSample(new Sample_Character);
    addSample(new CSMShadows);
#if OGRE_PLATFORM != OGRE_PLATFORM_WINRT
    addSample(new Sample_DynTex);
    addSample(new Sample_FacialAnimation);
    addSample(new Sample_Grass);
    addSample(new Sample_DualQuaternion);
    addSample(new Sample_Isosurf);
#ifdef HAVE_IMGUI
    addSample(new Sample_ImGui);
#endif
    addSample(new Sample_NewInstancing);
    addSample(new Sample_TextureArray);
    addSample(new Sample_Tessellation);
    addSample(new Sample_PNTriangles);
#   ifdef OGRE_BUILD_COMPONENT_VOLUME
    addSample(new Sample_VolumeCSG);
    addSample(new Sample_VolumeTerrain);
#   endif
    addSample(new Sample_VolumeTex);
    addSample(new Sample_Shadows);
    addSample(new Sample_Lighting);
    addSample(new Sample_LightShafts);
#ifdef OGRE_BUILD_COMPONENT_MESHLODGENERATOR
    addSample(new Sample_MeshLod);
#endif
    addSample(new Sample_ParticleFX);
#ifdef HAVE_PCZ_PLUGIN
    addSample(new Sample_PCZTest);
#endif
    addSample(new Sample_ParticleGS);
    addSample(new Sample_Smoke);
#endif // OGRE_PLATFORM_WINRT
    addSample(new Sample_SkeletalAnimation);
    addSample(new Sample_SkyBox);
    addSample(new Sample_SkyDome);
    addSample(new Sample_SkyPlane);
    addSample(new Sample_SphereMapping);
    addSample(new Sample_TextureFX);
    addSample(new Sample_Transparency);

    // the samples below require shaders
    addSample(new Sample_Tessellation);
    addSample(new Sample_PBR);
    addSample(new Sample_RectLight);
#if defined(OGRE_BUILD_COMPONENT_RTSHADERSYSTEM) && OGRE_PLATFORM != OGRE_PLATFORM_WINRT
    addSample(new Sample_ShaderSystem);
    addSample(new Sample_ShaderSystemTexturedFog);
    addSample(new Sample_ShaderSystemMultiLight);
#endif
    addSample(new Sample_BSP);
    addSample(new Sample_CelShading);
    addSample(new Sample_Compositor);
    addSample(new Sample_Compute);
    addSample(new Sample_CubeMapping);
    addSample(new Sample_DeferredShading);
    addSample(new Sample_SSAO);
    addSample(new Sample_Ocean);
    addSample(new Sample_Water);
    addSample(new Sample_Dot3Bump);
    addSample(new Sample_Fresnel);
#ifdef OGRE_BUILD_COMPONENT_TERRAIN
    addSample(new Sample_Terrain);
    addSample(new Sample_EndlessWorld);
#endif
}

DefaultSamplesPlugin::~DefaultSamplesPlugin()
{
    for (auto s : mSamples)
    {
        delete s;
    }
}

#ifndef OGRE_STATIC_LIB
static SamplePlugin* sp;

extern "C" void _OgreSampleExport dllStartPlugin(void);
extern "C" void _OgreSampleExport dllStopPlugin(void);

extern "C" _OgreSampleExport void dllStartPlugin()
{
    sp = new DefaultSamplesPlugin();
    Root::getSingleton().installPlugin(sp);
}

extern "C" _OgreSampleExport void dllStopPlugin()
{
    Root::getSingleton().uninstallPlugin(sp);
    delete sp;
}
#endif
