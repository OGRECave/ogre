# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

if (UNIX)
  set(USER_HOME_DIRECTORY $ENV{HOME})
elseif (WIN32)
  string(REPLACE "\\" "/" USER_HOME_DIRECTORY "$ENV{HOMEDRIVE}$ENV{HOMEPATH}")
# other platforms?
endif ()

exec_program("cmake -E remove ${USER_HOME_DIRECTORY}/TestResults_*RenderingSubsystem.txt")
