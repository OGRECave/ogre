# -----------------------------------------------------------------------------
# Find DirectX SDK
# Define:
# DirectX_FOUND
# DirectX_INCLUDE_DIR
# DirectX_LIBRARY
# DirectX_ROOT_DIR

if(WIN32) # The only platform it makes sense to check for DirectX SDK
  include(FindPkgMacros)
  findpkg_begin(DirectX)
  
  # Get path, convert backslashes as ${ENV_DXSDK_DIR}
  getenv_path(DXSDK_DIR)
  
  # construct search paths
  set(DirectX_PREFIX_PATH 
    "${DXSDK_DIR}" 
	"${ENV_DXSDK_DIR}"
    "C:/apps_x86/Microsoft DirectX SDK*"
    "C:/Program Files (x86)/Microsoft DirectX SDK*"
    "C:/apps/Microsoft DirectX SDK*"
    "C:/Program Files/Microsoft DirectX SDK*"
	"$ENV{ProgramFiles}/Microsoft DirectX SDK*"
  )
  create_search_paths(DirectX)
  # redo search if prefix path changed
  clear_if_changed(DirectX_PREFIX_PATH
    DirectX_LIBRARY
	DirectX_INCLUDE_DIR
	DirectX_ROOT_DIR
  )
  
  find_path(DirectX_INCLUDE_DIR NAMES d3d9.h HINTS ${DirectX_INC_SEARCH_PATH})
  # dlls are in DirectX_ROOT_DIR/Developer Runtime/x64|x86
  # lib files are in DirectX_ROOT_DIR/Lib/x64|x86
  if(CMAKE_CL_64)
    set(DirectX_LIBPATH_SUFFIX "x64")
  else(CMAKE_CL_64)
    set(DirectX_LIBPATH_SUFFIX "x86")
  endif(CMAKE_CL_64)
  find_library(DirectX_LIBRARY NAMES d3d9 HINTS ${DirectX_LIB_SEARCH_PATH} PATH_SUFFIXES ${DirectX_LIBPATH_SUFFIX})
  
  if(DirectX_INCLUDE_DIR)
    set(DirectX_ROOT_DIR "${DirectX_INCLUDE_DIR}/..") 
  endif(DirectX_INCLUDE_DIR)

  findpkg_finish(DirectX)

  # look for D3D10.1 components
  if (DirectX_FOUND)
    find_path(DirectX_D3D10_INCLUDE_DIR NAMES d3d10_1shader.h HINTS ${DirectX_INCLUDE_DIR} NO_DEFAULT_PATH)
	get_filename_component(DirectX_LIBRARY_DIR "${DirectX_LIBRARY}" PATH)
	message(STATUS "DX lib dir: ${DirectX_LIBRARY_DIR}")
	find_library(DirectX_D3D10_LIBRARY NAMES d3d10 HINTS ${DirectX_LIBRARY_DIR} NO_DEFAULT_PATH)
	if (DirectX_D3D10_INCLUDE_DIR AND DirectX_D3D10_LIBRARY)
	  set(DirectX_D3D10_FOUND TRUE)
	  set(DirectX_D3D10_INCLUDE_DIRS ${DirectX_D3D10_INCLUDE_DIR})
	  set(DirectX_D3D10_LIBRARIES ${DirectX_D3D10_LIBRARY})
	endif ()
  endif ()

endif(WIN32)
