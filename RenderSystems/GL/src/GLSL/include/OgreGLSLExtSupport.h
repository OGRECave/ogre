/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/



#ifndef __GLSLExtSupport_H__
#define __GLSLExtSupport_H__

#include "OgreGLPrerequisites.h"

//
// OpenGL Shading Language entry points
//
namespace Ogre
{

	// forward declarations
	class GLSLProgram;
	class GLSLGpuProgram;
	class GLSLLinkProgram;
	class GLSLLinkProgramManager;
	class GLSLProgramFactory;


	/** Check for GL errors and report them in the Ogre Log.

	@param forceInfoLog if true then message from GL info log is obtained
	@param forceException if true then exception is generated if a GL error found
	*/
    void checkForGLSLError(const String& ogreMethod, const String& errorTextPrefix, const GLhandleARB obj, const bool forceInfoLog = false, const bool forceException = false);

	/** if there is a message in GL info log then post it in the Ogre Log
	@param msg the info log message string is appended to this string
	@param obj the GL object that is used to retrieve the info log
	*/
	String logObjectInfo(const String& msg, const GLhandleARB obj);


} // namespace Ogre

#endif // __GLSLExtSupport_H__
