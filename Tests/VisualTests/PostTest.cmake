# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

if (UNIX)
  # escape spaces
  string(REPLACE " " "\\ " USER_HOME_DIRECTORY "$ENV{HOME}")
  set(USER_HOME_DIRECTORY "$ENV{HOME}")
elseif (WIN32)
  # make sure to use backslashes
  string(REPLACE "/" "\\" USER_HOME_DIRECTORY "$ENV{HOMEDRIVE}$ENV{HOMEPATH}")
# other platforms?
endif ()

# clean up output files
# Use platform-specific utils, since 'cmake -E remove' didn't seem to handle spaces in filepaths properly
if (UNIX)
  exec_program("rm ${USER_HOME_DIRECTORY}/TestResults_*RenderingSubsystem.txt")
elseif (WIN32)
  exec_program("cmd /c del \"${USER_HOME_DIRECTORY}\\TestResults_*RenderingSubsystem.txt\"")
# other platforms?
endif ()
