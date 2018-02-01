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

# sanitise threading choices
if (NOT OGRE_CONFIG_THREADS)
	set(OGRE_CONFIG_THREAD_PROVIDER "none")
else ()
	if (OGRE_CONFIG_THREAD_PROVIDER STREQUAL "boost")
		if (WIN32 OR APPLE)
			# Prefer static linking in all cases
			set(Boost_USE_STATIC_LIBS TRUE)
		else ()
			# Statically linking boost to a dynamic Ogre build doesn't work on Linux 64bit
			set(Boost_USE_STATIC_LIBS ${OGRE_STATIC})
		endif ()
		if (APPLE AND APPLE_IOS)
			set(Boost_USE_MULTITHREADED OFF)
		endif()

		if(ANDROID)
			# FindBoost needs extra hint on android 
			set(Boost_COMPILER -gcc)
		endif()

		find_package(Boost COMPONENTS thread QUIET)
		if (NOT Boost_THREAD_FOUND)
			message(WARNING "boost-thread is not available. Using ${OGRE_THREAD_DEFAULT_PROVIDER} as thread provider.")
			set(OGRE_CONFIG_THREAD_PROVIDER ${OGRE_THREAD_DEFAULT_PROVIDER})
		endif ()
	elseif (OGRE_CONFIG_THREAD_PROVIDER STREQUAL "poco")
		find_package(POCO)
		if (NOT POCO_FOUND)
			message(WARNING "poco is not available. Using ${OGRE_THREAD_DEFAULT_PROVIDER} as thread provider.")
			set(OGRE_CONFIG_THREAD_PROVIDER ${OGRE_THREAD_DEFAULT_PROVIDER})
		endif ()
	elseif (OGRE_CONFIG_THREAD_PROVIDER STREQUAL "tbb")
		find_package(TBB)
		if (NOT TBB_FOUND)
			message(WARNING "tbb is not available. Using ${OGRE_THREAD_DEFAULT_PROVIDER} as thread provider.")
			set(OGRE_CONFIG_THREAD_PROVIDER ${OGRE_THREAD_DEFAULT_PROVIDER})
		endif ()
	elseif (OGRE_CONFIG_THREAD_PROVIDER STREQUAL "std")
	else ()
		message(WARNING "Unknown thread provider chosen. Defaulting to std.")
	endif ()	
endif ()
