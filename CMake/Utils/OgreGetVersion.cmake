#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

include(PreprocessorUtils)

macro(ogre_get_version HEADER)
  file(READ ${HEADER} TEMP_VAR_CONTENTS)
  get_preprocessor_entry(TEMP_VAR_CONTENTS OGRE_VERSION_MAJOR OGRE_VERSION_MAJOR)
  get_preprocessor_entry(TEMP_VAR_CONTENTS OGRE_VERSION_MINOR OGRE_VERSION_MINOR)
  get_preprocessor_entry(TEMP_VAR_CONTENTS OGRE_VERSION_PATCH OGRE_VERSION_PATCH)
  get_preprocessor_entry(TEMP_VAR_CONTENTS OGRE_VERSION_NAME OGRE_VERSION_NAME)
  get_preprocessor_entry(TEMP_VAR_CONTENTS OGRE_VERSION_SUFFIX OGRE_VERSION_SUFFIX)
  set(OGRE_VERSION "${OGRE_VERSION_MAJOR}.${OGRE_VERSION_MINOR}.${OGRE_VERSION_PATCH}${OGRE_VERSION_SUFFIX}")
  set(OGRE_SOVERSION "${OGRE_VERSION_MAJOR}.${OGRE_VERSION_MINOR}.${OGRE_VERSION_PATCH}")
  set(OGRE_VERSION_DASH_SEPARATED "${OGRE_VERSION_MAJOR}-${OGRE_VERSION_MINOR}-${OGRE_VERSION_PATCH}${OGRE_VERSION_SUFFIX}")

endmacro()
