/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#ifndef __ScriptLoader_H__
#define __ScriptLoader_H__

#include "OgrePrerequisites.h"
#include "OgreDataStream.h"
#include "OgreStringVector.h"
#include "OgreHeaderPrefix.h"

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
		@return
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
		@return A value representing the relative loading order of these scripts
			compared to other script users, where higher values load later.
		*/
		virtual Real getLoadingOrder(void) const  = 0;

	};

	/** @} */
	/** @} */

}

#include "OgreHeaderSuffix.h"

#endif
