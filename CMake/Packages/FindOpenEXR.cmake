# Locate OpenEXR
# This module defines
# OPENEXR_LIBRARY
# OPENEXR_FOUND, if false, do not try to link to OpenEXR 
# OPENEXR_INCLUDE_DIR, where to find the headers
#
# $OPENEXR_DIR is an environment variable that would
# correspond to the ./configure --prefix=$OPENEXR_DIR
#
# Created by Robert Osfield. 


FIND_PATH(OPENEXR_INCLUDE_DIR OpenEXR/ImfIO.h
    $ENV{OPENEXR_DIR}/include
    $ENV{OPENEXR_DIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/include
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
    /usr/freeware/include
)

# Macro to find exr libraries (deduplicating search paths)
# example: OPENEXR_FIND_VAR(OPENEXR_IlmImf_LIBRARY IlmImf)
MACRO(OPENEXR_FIND_VAR varname libname)
    FIND_LIBRARY( ${varname}
        NAMES ${libname}
        PATHS
        $ENV{OPENEXR_DIR}/lib
        $ENV{OPENEXR_DIR}
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/lib
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        /usr/freeware/lib64
    )
ENDMACRO(OPENEXR_FIND_VAR)

# Macro to find exr libraries (and debug versions)
# example: OPENEXR_FIND(IlmImf)
MACRO(OPENEXR_FIND libname)
    OPENEXR_FIND_VAR(OPENEXR_${libname}_LIBRARY ${libname})
    OPENEXR_FIND_VAR(OPENEXR_${libname}_LIBRARY_DEBUG ${libname}d)
ENDMACRO(OPENEXR_FIND)

OPENEXR_FIND(IlmImf)
OPENEXR_FIND(IlmThread)
OPENEXR_FIND(Iex)
OPENEXR_FIND(Half)

SET(OPENEXR_FOUND NO)
IF(OPENEXR_INCLUDE_DIR AND OPENEXR_IlmImf_LIBRARY AND OPENEXR_IlmThread_LIBRARY AND OPENEXR_Iex_LIBRARY AND OPENEXR_Half_LIBRARY)
    SET(OPENEXR_LIBRARIES ${OPENEXR_IlmImf_LIBRARY} ${OPENEXR_IlmThread_LIBRARY} ${OPENEXR_Half_LIBRARY} ${OPENEXR_Iex_LIBRARY} )
    SET(OPENEXR_LIBRARIES_VARS OPENEXR_IlmImf_LIBRARY OPENEXR_IlmThread_LIBRARY OPENEXR_Half_LIBRARY OPENEXR_Iex_LIBRARY )
    SET(OPENEXR_FOUND YES)
ENDIF(OPENEXR_INCLUDE_DIR AND OPENEXR_IlmImf_LIBRARY AND OPENEXR_IlmThread_LIBRARY AND OPENEXR_Iex_LIBRARY AND OPENEXR_Half_LIBRARY)
