#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Find DirectX11 SDK
# Define:
# DirectX11_FOUND
# DirectX11_INCLUDE_DIRS
# DirectX11_LIBRARIES

if(WIN32) # The only platform it makes sense to check for DirectX11 SDK
	include(FindPkgMacros)
	findpkg_begin(DirectX11)

	if( MSVC AND NOT MSVC90 )
		# Windows 10.x SDK
		get_filename_component(kit10_dir "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]" REALPATH)
		file(GLOB W10SDK_VERSIONS RELATIVE ${kit10_dir}/Include ${kit10_dir}/Include/10.*) 			# enumerate pre-release and not yet known release versions
		list(APPEND W10SDK_VERSIONS "10.0.10240.0" "10.0.14393.0" "10.0.15063.0" "10.0.16299.0")	# enumerate well known release versions
		list(REMOVE_DUPLICATES W10SDK_VERSIONS)
		list(SORT W10SDK_VERSIONS)
		list(REVERSE W10SDK_VERSIONS)																# sort from high to low
		if(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
			list(INSERT W10SDK_VERSIONS 0 ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})				# prefer version passed by CMake, if any
		endif()
		foreach(W10SDK_VER ${W10SDK_VERSIONS})
			find_path(DirectX11_INCLUDE_DIR NAMES d3d11.h HINTS "${kit10_dir}/Include/${W10SDK_VER}/um" "C:/Program Files (x86)/Windows Kits/10/Include/${W10SDK_VER}/um" "C:/Program Files/Windows Kits/10/Include/${W10SDK_VER}/um")
		endforeach()
		# Windows 8.1 SDK
		if(NOT DirectX11_INCLUDE_DIR)
			find_path(DirectX11_INCLUDE_DIR NAMES d3d11.h HINTS "C:/Program Files (x86)/Windows Kits/8.1/include/um" "C:/Program Files/Windows Kits/8.1/include/um")
		endif()
		# Windows 8.0 SDK
		if(NOT DirectX11_INCLUDE_DIR)
			find_path(DirectX11_INCLUDE_DIR NAMES d3d11.h HINTS "C:/Program Files (x86)/Windows Kits/8.0/include/um" "C:/Program Files/Windows Kits/8.0/include/um")
		endif()
	endif()

	if(DirectX11_INCLUDE_DIR)
		# No need to specify full path to libraries, proper version would be found as part of SDK, in one of the following places
		# "C:/Program Files (x86)/Windows Kits/10/lib/${W10SDK_VER}/um/${MSVC_CXX_ARCHITECTURE_ID}/"
		# "C:/Program Files (x86)/Windows Kits/8.1/lib/winv6.3/um/${MSVC_CXX_ARCHITECTURE_ID}/"
		# "C:/Program Files (x86)/Windows Kits/8.0/lib/win8/um/${MSVC_CXX_ARCHITECTURE_ID}/"
		# "C:/Program Files (x86)/Windows Phone Kits/8.1/lib/${MSVC_CXX_ARCHITECTURE_ID}/"
		# "C:/Program Files (x86)/Windows Phone Kits/8.0/lib/${MSVC_CXX_ARCHITECTURE_ID}/"
		set(DirectX11_LIBRARY d3d11.lib dxgi.lib dxguid.lib) 
		# but it is usefull to output selected version to the log
		message(STATUS "Found DirectX11 headers: ${DirectX11_INCLUDE_DIR}")
	endif()

	# Legacy Direct X SDK
	if( NOT DirectX11_INCLUDE_DIR OR NOT DirectX11_LIBRARY )
		# Get path, convert backslashes as ${ENV_DXSDK_DIR}
		getenv_path(DXSDK_DIR)
		getenv_path(DIRECTX_HOME)
		getenv_path(DIRECTX_ROOT)
		getenv_path(DIRECTX_BASE)

		# construct search paths
		set(DirectX11_PREFIX_PATH
		"${DXSDK_DIR}" "${ENV_DXSDK_DIR}"
		"${DIRECTX_HOME}" "${ENV_DIRECTX_HOME}"
		"${DIRECTX_ROOT}" "${ENV_DIRECTX_ROOT}"
		"${DIRECTX_BASE}" "${ENV_DIRECTX_BASE}"
		"C:/apps_x86/Microsoft DirectX SDK*"
		"C:/Program Files (x86)/Microsoft DirectX SDK*"
		"C:/apps/Microsoft DirectX SDK*"
		"C:/Program Files/Microsoft DirectX SDK*"
		"$ENV{ProgramFiles}/Microsoft DirectX SDK*"
		)

		create_search_paths(DirectX11)
		# redo search if prefix path changed
		clear_if_changed(DirectX11_PREFIX_PATH
			DirectX11_LIBRARY
			DirectX11_INCLUDE_DIR
		)

		# dlls are in DirectX11_ROOT_DIR/Developer Runtime/x64|x86
		# lib files are in DirectX11_ROOT_DIR/Lib/x64|x86
		if(CMAKE_SIZEOF_VOID_P EQUAL 8)
			set(DirectX11_LIBPATH_SUFFIX "x64")
		else(CMAKE_SIZEOF_VOID_P EQUAL 8)
			set(DirectX11_LIBPATH_SUFFIX "x86")
		endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

		# look for D3D11 components
		find_path(DirectX11_INCLUDE_DIR NAMES d3d11.h HINTS ${DirectX11_INC_SEARCH_PATH})
		find_library(DirectX11_DXERR_LIBRARY NAMES DxErr HINTS ${DirectX11_LIB_SEARCH_PATH} PATH_SUFFIXES ${DirectX11_LIBPATH_SUFFIX})
		find_library(DirectX11_DXGUID_LIBRARY NAMES dxguid HINTS ${DirectX11_LIB_SEARCH_PATH} PATH_SUFFIXES ${DirectX11_LIBPATH_SUFFIX})
		find_library(DirectX11_DXGI_LIBRARY NAMES dxgi HINTS ${DirectX11_LIB_SEARCH_PATH} PATH_SUFFIXES ${DirectX11_LIBPATH_SUFFIX})
		find_library(DirectX11_D3DCOMPILER_LIBRARY NAMES d3dcompiler HINTS ${DirectX11_LIB_SEARCH_PATH} PATH_SUFFIXES ${DirectX11_LIBPATH_SUFFIX})
		find_library(DirectX11_D3D11_LIBRARY NAMES d3d11 HINTS ${DirectX11_LIB_SEARCH_PATH} PATH_SUFFIXES ${DirectX11_LIBPATH_SUFFIX})
		find_library(DirectX11_D3DX11_LIBRARY NAMES d3dx11 HINTS ${DirectX11_LIB_SEARCH_PATH} PATH_SUFFIXES ${DirectX11_LIBPATH_SUFFIX})

		if (DirectX11_INCLUDE_DIR AND DirectX11_D3D11_LIBRARY)
		  set(DirectX11_D3D11_LIBRARIES ${DirectX11_D3D11_LIBRARIES}
			${DirectX11_D3D11_LIBRARY}
			${DirectX11_DXGI_LIBRARY}
			${DirectX11_DXGUID_LIBRARY}
			${DirectX11_D3DCOMPILER_LIBRARY}
		  )
		endif ()
		if (DirectX11_D3DX11_LIBRARY)
			set(DirectX11_D3D11_LIBRARIES ${DirectX11_D3D11_LIBRARIES} ${DirectX11_D3DX11_LIBRARY})
		endif ()
		if (DirectX11_DXERR_LIBRARY)
			set(DirectX11_D3D11_LIBRARIES ${DirectX11_D3D11_LIBRARIES} ${DirectX11_DXERR_LIBRARY})
		endif ()

		set(DirectX11_LIBRARY
			${DirectX11_D3D11_LIBRARIES}
		)

		mark_as_advanced(DirectX11_D3D11_LIBRARY
						 DirectX11_D3DX11_LIBRARY
						 DirectX11_DXERR_LIBRARY
						 DirectX11_DXGUID_LIBRARY
						 DirectX11_DXGI_LIBRARY
						 DirectX11_D3DCOMPILER_LIBRARY)
	endif () # Legacy Direct X SDK

	findpkg_finish(DirectX11)

endif(WIN32)
