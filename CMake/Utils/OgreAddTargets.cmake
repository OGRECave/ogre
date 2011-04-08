#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# add a new library target
function(ogre_add_library TARGETNAME LIBTYPE)
  set(_SOURCES ${ARGN})
  if (OGRE_UNITY_BUILD)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    # create Unity compilation units
    set(_FILE_NUM 0)
    set(_FILE_CNT 0)
    set(_FILE_CONTENTS "")
    foreach(_FILE ${ARGN})
      # test if file is more than just a header
      get_filename_component(_EXT ${_FILE} EXT)
      if (_EXT STREQUAL ".cpp")
        set(_FILE_CONTENTS "${_FILE_CONTENTS}\#include \"${_FILE}\"\n")
        math(EXPR _FILE_CNT "${_FILE_CNT}+1")
        if(_FILE_CNT EQUAL OGRE_UNITY_FILES_PER_UNIT)
          set(_FILENAME "${OGRE_BINARY_DIR}/${TARGETNAME}/compile_${TARGETNAME}_${_FILE_NUM}.cpp")
          message(STATUS "Creating Unity compile unit: ${_FILENAME}")
          file(WRITE ${_FILENAME} ${_FILE_CONTENTS})
          math(EXPR _FILE_NUM "${_FILE_NUM}+1")
          set(_FILE_CNT 0)
          set (_FILE_CONTENTS "")
          list(APPEND _SOURCES ${_FILENAME})
          set_source_files_properties(${_FILENAME} PROPERTIES GENERATED TRUE)
        endif()
        # exclude the original source file from the compilation
        set_source_files_properties(${_FILE} PROPERTIES LANGUAGE "" HEADER_FILE_ONLY TRUE)
      endif()
    endforeach()
    # don't forget the last set of files
    set(_FILENAME "${OGRE_BINARY_DIR}/${TARGETNAME}/compile_${TARGETNAME}_${_FILE_NUM}.cpp")
    message(STATUS "Creating Unity compile unit: ${_FILENAME}")
    file(WRITE ${_FILENAME} ${_FILE_CONTENTS})
    list(APPEND _SOURCES ${_FILENAME})
    set_source_files_properties(${_FILENAME} PROPERTIES GENERATED TRUE)
  endif ()
  add_library(${TARGETNAME} ${LIBTYPE} ${_SOURCES})
endfunction(ogre_add_library)
