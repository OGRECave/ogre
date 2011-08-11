# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

if (UNIX)
  # escape spaces
  string(REPLACE " " "\ " USER_HOME_DIRECTORY "$ENV{HOMEDRIVE}$ENV{HOMEPATH}")
elseif (WIN32)
  string(REPLACE "\\" "/" USER_HOME_DIRECTORY "$ENV{HOMEDRIVE}$ENV{HOMEPATH}")
# other platforms?
endif ()

# Use platform-specific utils, since 'cmake -E remove' didn't seem to handle wildcards in Windows
if (UNIX)
  exec_program("rm ${USER_HOME_DIRECTORY}/TestResults_*RenderingSubsystem.txt")
elseif (WIN32)
  exec_program("cmd /c del \"${USER_HOME_DIRECTORY}\TestResults_*RenderingSubsystem.txt\"")
# other platforms?
endif ()
