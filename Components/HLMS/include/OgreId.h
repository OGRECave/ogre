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

#ifndef __Id_H__
#define __Id_H__

namespace Ogre
{
    /// Big projects with lots, lots of units for very long periods of time (MMORPGs?)
    /// may want to define this to 64-bit
    typedef Ogre::uint32    IdType;

    /**
        Usage:
        OGRE_NEW SceneNode( Id::generateNewId< Node >() )
    */
    class _OgreExport Id
    {
    public:
        //This function assumes creation of new objects can't be made from multiple threads!!!
        template <typename T> static IdType generateNewId()
        {
            static IdType g_currentId = 0;
            return g_currentId++;
        }
    };

    class _OgreExport IdObject
    {
    private:
        friend struct IdCmp; //Avoid calling getId()
        IdType mId;

    protected:
        /**In the rare case our derived class wants to override our Id
            (normally we don't want that, that's why it's private).
        */
        void _setId( IdType newId )         { mId = newId; }

    public:
        /** We don't call generateNewId() here, to prevent objects in the stack (i.e. local variables)
            that don't need an Id from incrementing the count; which is very dangerous if the caller
            is creating local objects from multiple threads (which should stay safe!).
            Instead our creator should do that.
        */
        IdObject( IdType id ) : mId( id ) {}

        /// Get the unique id of this object
        IdType getId() const                { return mId; }

        bool operator()( const IdObject *left, const IdObject *right )
        {
            return left->mId < right->mId;
        }

        bool operator()( const IdObject &left, const IdObject &right )
        {
            return left.mId < right.mId;
        }
    };
}

#endif
