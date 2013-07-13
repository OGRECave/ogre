#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------


include(OgreAddTargets)

# Configure Samples build
  add_subdirectory(BezierPatch)
  add_subdirectory(CameraTrack)
  add_subdirectory(Character)
  add_subdirectory(Compositor)
  add_subdirectory(DeferredShading)
  add_subdirectory(DualQuaternion)
  add_subdirectory(DynTex)
  add_subdirectory(EndlessWorld)
  add_subdirectory(FacialAnimation)
  add_subdirectory(Fresnel)
  add_subdirectory(Grass)
  add_subdirectory(Instancing)
  add_subdirectory(Lighting)
  add_subdirectory(MeshLod)
  add_subdirectory(NewInstancing)
  add_subdirectory(OceanDemo)
  add_subdirectory(ParticleFX)
  add_subdirectory(PNTrianglesTessellation)
  add_subdirectory(ShaderSystem)
  add_subdirectory(ShaderSystemMultiLight)
  add_subdirectory(ShaderSystemTexturedFog)
  add_subdirectory(Shadows)
  add_subdirectory(SkeletalAnimation)
  add_subdirectory(SkyBox)
  add_subdirectory(SkyDome)
  add_subdirectory(SkyPlane)
  add_subdirectory(Smoke)
  add_subdirectory(SphereMapping)
  add_subdirectory(Terrain)
  add_subdirectory(TextureFX)
  add_subdirectory(TextureArray)
  add_subdirectory(Transparency)
  add_subdirectory(VolumeTex)
  add_subdirectory(VolumeCSG)
  add_subdirectory(VolumeTerrain)
  add_subdirectory(Water)

if (NOT OGRE_BUILD_RENDERSYSTEM_GLES AND NOT OGRE_BUILD_RENDERSYSTEM_GLES2)
  add_subdirectory(BSP)
  add_subdirectory(Isosurf)
  add_subdirectory(ParticleGS)
  add_subdirectory(SSAO)
  add_subdirectory(Tesselation)
endif ()

if (NOT OGRE_BUILD_RENDERSYSTEM_GLES OR OGRE_BUILD_RENDERSYSTEM_GLES2)
  add_subdirectory(CelShading)
  add_subdirectory(CubeMapping)
  add_subdirectory(Dot3Bump)
endif ()

  # Add browser last
  add_subdirectory(Browser)
