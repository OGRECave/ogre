##################################################################
# Support macro to use a precompiled header
# Usage:
#   use_precompiled_header(TARGET HEADER_FILE SRC_FILE)
#
# PREC will contain the dependency that needs to be linked
# to the project.
##################################################################

macro(use_precompiled_header TARGET HEADER_FILE SRC_FILE)
  get_filename_component(HEADER ${HEADER_FILE} NAME)

  if (MSVC)
    add_definitions(/Yu"${HEADER}")
    set_source_files_properties(${SRC_FILE}
      PROPERTIES COMPILE_FLAGS /Yc"${HEADER}"
    )
    
  elseif (CMAKE_COMPILER_IS_GNUCXX)
    # disabled because it seems to increase compile time
    ## this is some serious hack... we definitely need native 
    ## support in CMake for this!
    ## we will generate the precompiled header via a workaround
    ## first give the header a new name with the proper extension
    #set(PRECOMP_HEADER ${CMAKE_CURRENT_BINARY_DIR}/hacked/${HEADER}.gch)
    #configure_file(${HEADER_FILE} ${PRECOMP_HEADER} COPYONLY)
    ## retrieve some info about the target's build settings
    #get_target_property(${TARGET} PRECOMP_TYPE TYPE)
    #if (PRECOMP_TYPE STREQUAL "SHARED_LIBRARY")
    #  set(PRECOMP_LIBTYPE "SHARED")
    #else ()
    #  set(PRECOMP_LIBTYPE "STATIC")
    #endif ()
    #get_target_property(${TARGET} PRECOMP_DEFINITIONS COMPILE_DEFINITIONS)
    #get_target_property(${TARGET} PRECOMP_FLAGS COMPILE_FLAGS)
    #
    ## add a new target which compiles the header
    #add_library(__precomp_header ${PRECOMP_LIBTYPE} ${PRECOMP_HEADER})
    #add_dependencies(${TARGET} __precomp_header)
    #set_target_properties(__precomp_header PROPERTIES
    #  COMPILE_DEFINITIONS ${PRECOMP_DEFINITIONS}
    #  COMPILE_FLAGS ${PRECOMP_FLAGS}
    #  HAS_CXX TRUE
    #)
    #set_source_files_properties(${PRECOMP_HEADER} PROPERTIES
    #  HEADER_FILE_ONLY FALSE
    #  KEEP_EXTENSION TRUE
    #  COMPILE_FLAGS "-x c++-header"
    #  LANGUAGE CXX
    #)
    #
    ## finally, we need to ensure that gcc can find the precompiled header
    ## this is another dirty hack
    #include_directories(BEFORE "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/__precomp_header.dir/hacked")

  endif ()
endmacro(use_precompiled_header)
