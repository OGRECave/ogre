#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

#######################################################################
# This file determines which threading options are available for the
# build, depending on the dependencies found.
#######################################################################

#always available via std
set(OGRE_THREAD_SUPPORT_AVAILABLE TRUE)
set(OGRE_THREAD_DEFAULT_PROVIDER "none")
set(OGRE_THREAD_TYPE "0")

if (Boost_THREAD_FOUND AND Boost_DATE_TIME_FOUND)
	set(Boost_THREADING TRUE)
endif ()

if (Boost_THREADING AND NOT OGRE_THREAD_SUPPORT_AVAILABLE)
	#set(OGRE_THREAD_SUPPORT_AVAILABLE TRUE)
	set(OGRE_THREAD_DEFAULT_PROVIDER "boost")
	set(OGRE_THREAD_TYPE "2")
endif ()

if (POCO_FOUND AND NOT OGRE_THREAD_SUPPORT_AVAILABLE)
	#set(OGRE_THREAD_SUPPORT_AVAILABLE TRUE)
	set(OGRE_THREAD_DEFAULT_PROVIDER "poco")
	set(OGRE_THREAD_TYPE "2")
endif ()

if (TBB_FOUND AND NOT OGRE_THREAD_SUPPORT_AVAILABLE)
	#set(OGRE_THREAD_SUPPORT_AVAILABLE TRUE)
	set(OGRE_THREAD_DEFAULT_PROVIDER "tbb")
	set(OGRE_THREAD_TYPE "2")
endif ()

if (OGRE_CONFIG_THREAD_PROVIDER STREQUAL "std")
	set(OGRE_THREAD_DEFAULT_PROVIDER "std")
	set(OGRE_THREAD_TYPE "2")
endif ()

if (OGRE_THREAD_SUPPORT_AVAILABLE)
	set(OGRE_CONFIG_THREADS ${OGRE_THREAD_TYPE} CACHE STRING 
		"Enable Ogre thread support for background loading. Possible values:
		0 - Threading off.
		1 - Full background loading.
		2 - Background resource preparation."
	)
	set(OGRE_CONFIG_THREAD_PROVIDER ${OGRE_THREAD_DEFAULT_PROVIDER} CACHE STRING
		"Select the library to use for thread support. Possible values:
		boost - Boost thread library.
		poco  - Poco thread library.
		tbb   - ThreadingBuildingBlocks library.
		std   - STL thread library (requires compiler support)."
	)
else ()
	set(OGRE_CONFIG_THREADS 0)
	set(OGRE_CONFIG_THREAD_PROVIDER "none")
endif ()


# sanitise threading choices
if (NOT OGRE_CONFIG_THREADS)
	set(OGRE_CONFIG_THREAD_PROVIDER "none")
	include_directories(${OGRE_SOURCE_DIR}/OgreMain/include/Threading)
else ()
	if (OGRE_CONFIG_THREAD_PROVIDER STREQUAL "boost")
		if (NOT Boost_THREADING)
			message(STATUS "Warning: boost-thread is not available. Using ${OGRE_THREAD_DEFAULT_PROVIDER} as thread provider.")
			set(OGRE_CONFIG_THREAD_PROVIDER ${OGRE_THREAD_DEFAULT_PROVIDER})
		endif ()
	elseif (OGRE_CONFIG_THREAD_PROVIDER STREQUAL "poco")
		if (NOT POCO_FOUND)
			message(STATUS "Warning: poco is not available. Using ${OGRE_THREAD_DEFAULT_PROVIDER} as thread provider.")
			set(OGRE_CONFIG_THREAD_PROVIDER ${OGRE_THREAD_DEFAULT_PROVIDER})
		endif ()
	elseif (OGRE_CONFIG_THREAD_PROVIDER STREQUAL "tbb")
		if (NOT TBB_FOUND)
			message(STATUS "Warning: tbb is not available. Using ${OGRE_THREAD_DEFAULT_PROVIDER} as thread provider.")
			set(OGRE_CONFIG_THREAD_PROVIDER ${OGRE_THREAD_DEFAULT_PROVIDER})
		endif ()
	elseif (OGRE_CONFIG_THREAD_PROVIDER STREQUAL "std")
		#Potentially add test for compiler support
		set(OGRE_CONFIG_THREAD_PROVIDER ${OGRE_THREAD_DEFAULT_PROVIDER})
	else ()
		message(STATUS "Warning: Unknown thread provider chosen. Defaulting to ${OGRE_THREAD_DEFAULT_PROVIDER}.")
	endif ()	
endif ()
