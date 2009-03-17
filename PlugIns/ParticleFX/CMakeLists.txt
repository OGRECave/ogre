# Configure BSP SceneManager build

set (HEADER_FILES
  include/OgreAreaEmitter.h
  include/OgreBoxEmitterFactory.h
  include/OgreBoxEmitter.h
  include/OgreColourFaderAffector2.h
  include/OgreColourFaderAffectorFactory2.h
  include/OgreColourFaderAffectorFactory.h
  include/OgreColourFaderAffector.h
  include/OgreColourImageAffectorFactory.h
  include/OgreColourImageAffector.h
  include/OgreColourInterpolatorAffectorFactory.h
  include/OgreColourInterpolatorAffector.h
  include/OgreCylinderEmitterFactory.h
  include/OgreCylinderEmitter.h
  include/OgreDeflectorPlaneAffectorFactory.h
  include/OgreDeflectorPlaneAffector.h
  include/OgreDirectionRandomiserAffectorFactory.h
  include/OgreDirectionRandomiserAffector.h
  include/OgreEllipsoidEmitterFactory.h
  include/OgreEllipsoidEmitter.h
  include/OgreHollowEllipsoidEmitterFactory.h
  include/OgreHollowEllipsoidEmitter.h
  include/OgreLinearForceAffectorFactory.h
  include/OgreLinearForceAffector.h
  include/OgreParticleFXPlugin.h
  include/OgreParticleFXPrerequisites.h
  include/OgrePointEmitterFactory.h
  include/OgrePointEmitter.h
  include/OgreRingEmitterFactory.h
  include/OgreRingEmitter.h
  include/OgreRotationAffectorFactory.h
  include/OgreRotationAffector.h
  include/OgreScaleAffectorFactory.h
  include/OgreScaleAffector.h
)

set (SOURCE_FILES
  src/OgreAreaEmitter.cpp
  src/OgreBoxEmitter.cpp
  src/OgreColourFaderAffector2.cpp
  src/OgreColourFaderAffector.cpp
  src/OgreColourImageAffector.cpp
  src/OgreColourInterpolatorAffector.cpp
  src/OgreCylinderEmitter.cpp
  src/OgreDeflectorPlaneAffector.cpp
  src/OgreDirectionRandomiserAffector.cpp
  src/OgreEllipsoidEmitter.cpp
  src/OgreHollowEllipsoidEmitter.cpp
  src/OgreLinearForceAffector.cpp
  src/OgreParticleFX.cpp
  src/OgreParticleFXPlugin.cpp
  src/OgrePointEmitter.cpp
  src/OgreRingEmitter.cpp
  src/OgreRotationAffector.cpp
  src/OgreScaleAffector.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_definitions(-D_USRDLL)

add_library(Plugin_ParticleFX ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(Plugin_ParticleFX OgreMain)
if (NOT OGRE_STATIC)
  set_target_properties(Plugin_ParticleFX PROPERTIES
    COMPILE_DEFINITIONS OGRE_PARTICLEFXPLUGIN_EXPORTS
  ) 
endif ()

ogre_config_plugin(Plugin_ParticleFX)
