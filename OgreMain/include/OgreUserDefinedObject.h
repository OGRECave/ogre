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
#ifndef __UserDefinedObject_H__
#define __UserDefinedObject_H__

#include "OgrePrerequisites.h"

namespace Ogre {


	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup General
	*  @{
	*/
	/** This class is designed to be subclassed by OGRE users, to allow them to
        associate their own application objects with MovableObject instances
        in the engine.
    @remarks
        It's always been suggested that an OGRE application would likely comprise
        a number of game objects which would keep pointers to OGRE objects in order
        to maintain the link. However, in some cases it would be very useful to be able to
        navigate directly from an OGRE instance back to a custom application object.
        This abstract class exists for this purpose; MovableObjects hold a pointer to
        a UserDefinedObject instance, which application writers subclass in order to
        include their own attributes. Your game objects themselves may be subclasses of this
        class, or your subclasses may merely be a link between them.
    @par
        Because OGRE never uses instances of this object itself, there is very little
        definition to this class; the application is expected to add all the detail it wants.
        However, as a hint, and for debugging purposes, this class does define a 'type id',
        which it is recommended you use to differentiate between your subclasses,
        if you have more than one type.
    */
    class _OgreExport UserDefinedObject
    {
    public:
        /** Standard constructor. */
        UserDefinedObject();
        virtual ~UserDefinedObject() {}

        /** Return a number identifying the type of user defined object.
        @remarks
            Can be used to differentiate between different types of object which you attach to
            OGRE MovableObject instances. Recommend you override this in your classes if you
            use more than one type of object.
        @par
            Alternatively, you can override the getTypeName method and use that instead; 
            that version is a litle more friendly and easier to scope, but obviously 
            slightly less efficient. You choose which you prefer.
        */
        virtual long getTypeID(void) const;

        /** Return a string identifying the type of user defined object.
        @remarks
            Can be used to differentiate between different types of object which you attach to
            OGRE MovableObject instances. Recommend you override this in your classes if you
            use more than one type of object.
        @par
            Alternatively, you can override the getTypeID method and use that instead; 
            that version is a litle more efficient, but obviously 
            slightly less easy to read. You choose which you prefer.
        */
        virtual const String& getTypeName(void) const;
        
    };
    
        
	/** @} */
	/** @} */

}

#endif
