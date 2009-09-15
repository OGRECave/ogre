/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
