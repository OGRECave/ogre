#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Visual Test build

if (OGRE_BUILD_COMPONENT_RTSHADERSYSTEM)
    add_definitions(-DINCLUDE_RTSHADER_SYSTEM)
endif ()

add_definitions(${OGRE_VISIBILITY_FLAGS})

if(NOT ANDROID)
    # add Context directory
    add_subdirectory(Context)
endif()

# add the PlayPen test plugin's directory
add_subdirectory(PlayPen)

# add VTests plugin directory
add_subdirectory(VTests)

if(ANDROID)
    # skip the CTest stuff
    return()
endif()

# build a list of render systems to test
set(TEST_RENDER_SYSTEMS "")

if (OGRE_BUILD_RENDERSYSTEM_GL)
  list(APPEND TEST_RENDER_SYSTEMS "OpenGL Rendering Subsystem")
endif ()
if (OGRE_BUILD_RENDERSYSTEM_GL3PLUS)
  list(APPEND TEST_RENDER_SYSTEMS "OpenGL 3+ Rendering Subsystem")
endif ()
if (OGRE_BUILD_RENDERSYSTEM_D3D9)
  list(APPEND TEST_RENDER_SYSTEMS "Direct3D9 Rendering Subsystem")
endif ()
if (OGRE_BUILD_RENDERSYSTEM_D3D11)
  list(APPEND TEST_RENDER_SYSTEMS "Direct3D11 Rendering Subsystem")
endif ()
if (OGRE_BUILD_RENDERSYSTEM_GLES2)
  list(APPEND TEST_RENDER_SYSTEMS "OpenGL ES 2.x Rendering Subsystem")
endif ()

# Figure out the home directory for this system (this is where the context outputs result data for now)
if (UNIX)
  set(USER_HOME_DIRECTORY $ENV{HOME})
elseif (WIN32)
  string(REPLACE "\\" "/" USER_HOME_DIRECTORY "$ENV{HOMEDRIVE}$ENV{HOMEPATH}")
# other platforms?
endif ()

# function for running visual tests for a given render system
function (TestRenderSystem render_system)

  # strip spaces from rendersystem name
  string (REPLACE " " "" render_system_nospace ${render_system})

  # Run a CMake script that handles the test context
  add_test(NAME "TestContext_${render_system_nospace}"
    COMMAND $<TARGET_FILE_NAME:TestContext>
    -rs "${render_system}"  # Pick rendersystem
    -n "AutomatedTest"      # Name it, so it overwrites itself each run
    -m "Automated Test - ${render_system}" # A brief comment
    -o "${USER_HOME_DIRECTORY}"
    WORKING_DIRECTORY $<TARGET_FILE_DIR:TestContext>)
      
  # Set a longer timeout to avoid timeouts on the riscv hardware
  set_tests_properties("TestContext_${render_system_nospace}" PROPERTIES TIMEOUT 4500 ENVIRONMENT OGRE_ONLY_COMMON_GL_TESTS=1)

endfunction (TestRenderSystem)

# Run the tests once for each rendersystem
foreach (rs ${TEST_RENDER_SYSTEMS})
  TestRenderSystem(${rs})
endforeach ()

