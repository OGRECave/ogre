#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Since Windows cmd utils seem touchy about forward slash path separators
string (REPLACE "/" "\\" FORMATTED_RESULT_FILE ${TEST_RESULT_FILE})

# Output results to the command line, for CTest to regex search
exec_program("cmd" ARGS "/c type ${FORMATTED_RESULT_FILE}")
