#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# set pre/post commands:
#set(CTEST_CUSTOM_PRE_TEST "")
set(CTEST_CUSTOM_POST_TEST "cmake -P Tests/VisualTests/PostTest.cmake")
