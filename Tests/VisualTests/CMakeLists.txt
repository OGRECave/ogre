#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Visual Test build

# add Context directory
add_subdirectory(Context)

# add the PlayPen test plugin's directory
add_subdirectory(PlayPen)

# add VTests plugin directory
add_subdirectory(VTests)
