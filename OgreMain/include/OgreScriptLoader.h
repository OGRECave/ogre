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
#ifndef __ScriptLoader_H__
#define __ScriptLoader_H__

#include "OgrePrerequisites.h"
#include "OgreDataStream.h"
#include "OgreStringVector.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup General
	*  @{
	*/
	/** Abstract class defining the interface used by classes which wish 
		to perform script loading to define instances of whatever they manage.
	@remarks
		Typically classes of this type wish to either parse individual script files
		on demand, or be called with a group of files matching a certain pattern
		at the appropriate time. Normally this will coincide with resource loading,
		although the script use does not necessarily have to be a ResourceManager
		(which subclasses from this class), it may be simply a script loader which 
		manages non-resources but needs to be synchronised at the same loading points.
	@par
		Subclasses should add themselves to the ResourceGroupManager as a script loader
		if they wish to be called at the point a resource group is loaded, at which 
		point the parseScript method will be called with each file which matches a 
		the pattern returned from getScriptPatterns.
	*/
	class _OgreExport ScriptLoader
	{
	public:
		virtual ~ScriptLoader();
		/** Gets the file patterns which should be used to find scripts for this
			class.
		@remarks
			This method is called when a resource group is loaded if you use 
			ResourceGroupManager::_registerScriptLoader.
		@returns
			A list of file patterns, in the order they should be searched in.
		*/
		virtual const StringVector& getScriptPatterns(void) const = 0;

		/** Parse a script file.
		@param stream Weak reference to a data stream which is the source of the script
		@param groupName The name of a resource group which should be used if any resources
			are created during the parse of this script.
		*/
		virtual void parseScript(DataStreamPtr& stream, const String& groupName) = 0;

		/** Gets the relative loading order of scripts of this type.
		@remarks
			There are dependencies between some kinds of scripts, and to enforce
			this all implementors of this interface must define a loading order. 
		@returns A value representing the relative loading order of these scripts
			compared to other script users, where higher values load later.
		*/
		virtual Real getLoadingOrder(void) const  = 0;

	};

	/** @} */
	/** @} */

}


#endif
