#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

############################################################
# OgreMath library
############################################################

PROJECT(OgreMath)

include(PrecompiledHeader)

# define header and source files for the library
set (HEADER_FILES
  include/Default/asm_math.h
  include/Default/OgreAxisAlignedBox.h
  include/Default/OgreBitwise.h
  include/Default/OgreConvexBody.h
  include/Default/OgreDualQuaternion.h
  include/Default/OgreEdgeListBuilder.h
  include/Default/OgreFrustum.h
  include/Default/OgreMath.h
  include/Default/OgreMatrix3.h
  include/Default/OgreMatrix4.h
  include/Default/OgreNumerics.h
  include/Default/OgreOptimisedUtil.h
  include/Default/OgrePlane.h
  include/Default/OgrePlaneBoundedVolume.h
  include/Default/OgrePolygon.h
  include/Default/OgreQuaternion.h
  include/Default/OgreRay.h
  include/Default/OgreRotationalSpline.h
  include/Default/OgreSIMDHelper.h
  include/Default/OgreSimpleSpline.h
  include/Default/OgreTangentSpaceCalc.h
  include/Default/OgreVector2.h
  include/Default/OgreVector3.h
  include/Default/OgreVector4.h
)

set (SOURCE_FILES
  src/Default/OgreAxisAlignedBox.cpp
  src/Default/OgreConvexBody.cpp
  src/Default/OgreDualQuaternion.cpp
  src/Default/OgreEdgeListBuilder.cpp
  src/Default/OgreFrustum.cpp
  src/Default/OgreMath.cpp
  src/Default/OgreMatrix3.cpp
  src/Default/OgreMatrix4.cpp
  src/Default/OgreNumerics.cpp
  src/Default/OgreOptimisedUtil.cpp
  src/Default/OgreOptimisedUtilGeneral.cpp
  src/SSE/OgreOptimisedUtilSSE.cpp
  src/Default/OgrePlane.cpp
  src/Default/OgrePolygon.cpp
  src/Default/OgreQuaternion.cpp
  src/Default/OgreRotationSpline.cpp
  src/Default/OgreSimpleSpline.cpp
  src/Default/OgreTangentSpaceCalc.cpp
  src/Default/OgreVector2.cpp
  src/Default/OgreVector3.cpp
  src/Default/OgreVector4.cpp
)

# setup OgreMath target

include_directories("include/Default" "include/SSE")
 
# Always build as static
ogre_add_library(OgreMath STATIC ${HEADER_FILES} ${SOURCE_FILES} ${PLATFORM_HEADER_FILES} ${PLATFORM_SOURCE_FILES})
set_target_properties(OgreMath PROPERTIES VERSION ${OGRE_SOVERSION})

# install 
ogre_config_component(OgreMath)

install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/Math
)
