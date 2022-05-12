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
#ifndef _UserObjectsBinding_H__
#define _UserObjectsBinding_H__

#include "OgrePrerequisites.h"
#include "OgreAny.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */

    /** Class that provides convenient interface to establish a linkage between
    custom user application objects and %Ogre core classes.
    */
    class _OgreExport UserObjectBindings : public GeneralAllocatedObject
    {
    public:
        /** Sets any kind of user object on this class instance.
        @remarks
        This method allows you to associate any user object you like with
        this class. This can be a pointer back to one of your own
        classes for instance.
        @note This method is key less meaning that each call for it will override
        previous object that were set. If you need to associate multiple objects
        with this class use the extended version that takes key.
        */
        void setUserAny(const Any& anything);

        /** Retrieves the custom key less user object associated with this class.
        */
        const Any& getUserAny(void) const;

        /** Sets any kind of user object on this class instance.
        @remarks
        This method allows you to associate multiple object with this class.
        This can be a pointer back to one of your own classes for instance.
        Use a unique key to distinguish between each of these objects.
        @param key The key that this data is associate with.
        @param anything The data to associate with the given key.
        */
        void setUserAny(const String& key, const Any& anything);

        /** Retrieves the custom user object associated with this class and key.
        @param key The key that the requested user object is associated with.
        @remarks
        In case no object associated with this key the returned Any object will be empty.
        */
        const Any& getUserAny(const String& key) const;

        /** Erase the custom user object associated with this class and key from this binding.
        @param key The key that the requested user object is associated with.
        */
        void eraseUserAny(const String& key);

        /** Clear all user objects from this binding.   */
        void clear();

        UserObjectBindings() = default;

        /** Copy constructor. Performs a copy of all stored UserAny. */
        UserObjectBindings(const UserObjectBindings& other);

        UserObjectBindings& swap(UserObjectBindings& rhs);

        UserObjectBindings& operator=(const UserObjectBindings& rhs);

    // Types.
    private:
        typedef std::map<String, Any>          UserObjectsMap;
        typedef UserObjectsMap::iterator        UserObjectsMapIterator;
        typedef UserObjectsMap::const_iterator  UserObjectsMapConstIterator;

        /** Internal class that uses as data storage container.
        */
        struct Attributes : public GeneralAllocatedObject
        {
            Attributes() = default;

            /** Copy ctor. Copies the attribute storage. */
            Attributes(const Attributes& other) :
                mKeylessAny(other.mKeylessAny)
            {
                if (other.mUserObjectsMap)
                    mUserObjectsMap.reset(new UserObjectsMap(*other.mUserObjectsMap));
            }

            Any                 mKeylessAny;// Will hold key less associated user object for fast access.
            std::unique_ptr<UserObjectsMap> mUserObjectsMap;// Will hold a map between user keys to user objects.
        };

        /** \brief Protected getter for the attributes map, to allow derived classes to inspect its elements. */
        const Attributes* getAttributes() const { return mAttributes.get(); }
        Attributes* getAttributes() { return mAttributes.get(); }

    // Attributes.
    private:
        std::unique_ptr<Attributes>     mAttributes;        // Class attributes - will be allocated on demand.
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
