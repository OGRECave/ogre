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

#ifndef __OgreFastArray__
#define __OgreFastArray__

namespace Ogre
{
    /** Lightweight implementation of std::vector
    @remarks
        The problem with std::vector is that some implementations (eg. Visual Studio's) have a lot
        of range checking and debugging code that slows them down a lot. MSVC's security features
        can be disabled defining the macro "#define _SECURE_SCL 0". However that must be done as a
        global macro, and turns out we __don't__ want to disable those nice warnings and
        out-of-bounds checkings in a lot of sections of our code (except may be for extremely
        optimized Release builds)
    @par
        Since we can't enable/disable those checkings selectively, I wrote our own lightweight
        container for performance-sensitive areas where we're also very certain we won't use
        them incorrectly.
    @par
        It's partially STL compliant, for example std::for_each works with it.
        However some functions are not, for example FastArray<int> myArray(5) does not behave
        as the standard does: FastArray will reserve 5 ints, while std::vector will __push__
        5 ints and default-initialize them (in the case of ints, fill it with 5 zeros)
    @par
        Only use this container for extremely performance sensitive and you're certain you'll
        be using it correctly. If you're in doubt or don't know what to do, use std::vector
        instead.
    @par
        FastArray was created because we needed to keep multiple lists (one per thread) of
        culled MovableObjects pointers (against the camera) and then iterate through all of
        them. These multiple levels of indirection was causing MS implementation to go mad
        with a huge amount of useless bounds checking & iterator validation.
    @author
        Matias N. Goldberg
    @version
        1.0
    */
    template <typename T> class FastArray
    {
        T           *mData;
        size_t      mSize;
        size_t      mCapacity;

        /** Checks whether we'll be at full capacity after adding N new elements, if so,
            increase the array size by 50%
        @remarks
            Doesn't do anything if available capacity is enough to contain the new elements
            Won't modify mSize, only mCapacity
        @param newElements
            Amount of new elements to push to the array
        */
        void growToFit( size_t newElements )
        {
            if( mSize + newElements > mCapacity )
            {
                mCapacity = std::max( mSize + newElements, mCapacity + (mCapacity >> 1) + 1 );
                T *data = (T*)::operator new( mCapacity * sizeof(T) );
                memcpy( data, mData, mSize * sizeof(T) );
                ::operator delete( mData );
                mData = data;
            }
        }

    public:
        typedef T value_type;

        typedef T* iterator;
        typedef const T* const_iterator;

        FastArray() :
            mData( 0 ),
            mSize( 0 ),
            mCapacity( 0 )
        {
        }

        void swap( FastArray<T> &other )
        {
            std::swap( this->mData, other.mData );
            std::swap( this->mSize, other.mSize );
            std::swap( this->mCapacity, other.mCapacity );
        }

        FastArray( const FastArray<T> &copy ) :
                mSize( copy.mSize ),
                mCapacity( copy.mSize )
        {
            mData = (T*)::operator new( mSize * sizeof(T) );
            for( size_t i=0; i<mSize; ++i )
            {
                new (&mData[i]) T( copy.mData[i] );
            }
        }

        void operator = ( const FastArray<T> &copy )
        {
            if( &copy != this )
            {
                for( size_t i=0; i<mSize; ++i )
                    mData[i].~T();
                ::operator delete( mData );

                mSize       = copy.mSize;
                mCapacity   = copy.mSize;

                mData = (T*)::operator new( mSize * sizeof(T) );
                for( size_t i=0; i<mSize; ++i )
                {
                    new (&mData[i]) T( copy.mData[i] );
                }
            }
        }

        /// Creates an array reserving the amount of elements (memory is not initialized)
        FastArray( size_t reserveAmount ) :
            mSize( 0 ),
            mCapacity( reserveAmount )
        {
            mData = (T*)::operator new( reserveAmount * sizeof(T) );
        }

        /// Creates an array pushing the value N times
        FastArray( size_t count, const T &value ) :
            mSize( count ),
            mCapacity( count )
        {
            mData = (T*)::operator new( count * sizeof(T) );
            for( size_t i=0; i<count; ++i )
            {
                new (&mData[i]) T( value );
            }
        }

        ~FastArray()
        {
            for( size_t i=0; i<mSize; ++i )
                mData[i].~T();
            ::operator delete( mData );
        }

        size_t size() const                     { return mSize; }
        size_t capacity() const                 { return mCapacity; }

        void push_back( const T& val )
        {
            growToFit( 1 );
            new (&mData[mSize]) T( val );
            ++mSize;
        }

        void pop_back()
        {
            assert( mSize > 0 && "Can't pop a zero-sized array" );
            --mSize;
            mData[mSize].~T();
        }

        iterator insert( iterator where, const T& val )
        {
            size_t idx = (where - mData);

            growToFit( 1 );

            memmove( mData + idx + 1, mData + idx, (mSize - idx) *  sizeof(T) );
            new (&mData[idx]) T( val );
            ++mSize;

            return mData + idx;
        }

        /// otherBegin & otherEnd must not overlap with this->begin() and this->end()
        iterator insertPOD( iterator where, const_iterator otherBegin, const_iterator otherEnd )
        {
            size_t idx = (where - mData);

            const size_t otherSize = otherEnd - otherBegin;

            growToFit( otherSize );

            memmove( mData + idx + otherSize, mData + idx, (mSize - idx) *  sizeof(T) );

            while( otherBegin != otherEnd )
                *where++ = *otherBegin++;
            mSize += otherSize;

            return mData + idx;
        }

        void appendPOD( const_iterator otherBegin, const_iterator otherEnd )
        {
            growToFit( otherEnd - otherBegin );

            memcpy( mData + mSize, otherBegin, (otherEnd - otherBegin) *  sizeof(T) );
            mSize += otherEnd - otherBegin;
        }

        iterator erase( iterator toErase )
        {
            size_t idx = (toErase - mData);
            toErase->~T();
            memmove( mData + idx, mData + idx + 1, (mSize - idx - 1) * sizeof(T) );
            --mSize;

            return mData + idx;
        }

        iterator erase( iterator first, iterator last )
        {
            assert( first <= last && last <= end() );

            size_t idx      = (first - mData);
            size_t idxNext  = (last - mData);
            while( first != last )
            {
                first->~T();
                ++first;
            }
            memmove( mData + idx, mData + idxNext, (mSize - idxNext) * sizeof(T) );
            mSize -= idxNext - idx;

            return mData + idx;
        }

        iterator erasePOD( iterator first, iterator last )
        {
            assert( first <= last && last <= end() );

            size_t idx      = (first - mData);
            size_t idxNext  = (last - mData);
            memmove( mData + idx, mData + idxNext, (mSize - idxNext) * sizeof(T) );
            mSize -= idxNext - idx;

            return mData + idx;
        }

        void clear()
        {
            for( size_t i=0; i<mSize; ++i )
                mData[i].~T();
            mSize = 0;
        }

        bool empty() const                      { return mSize == 0; }

        void reserve( size_t reserveAmount )
        {
            if( reserveAmount > mCapacity )
            {
                //We don't use growToFit because it will try to increase capacity by 50%,
                //which is not the desire when calling reserve() explicitly
                mCapacity = reserveAmount;
                T *data = (T*)::operator new( mCapacity * sizeof(T) );
                memcpy( data, mData, mSize * sizeof(T) );
                ::operator delete( mData );
                mData = data;
            }
        }

        void resize( size_t newSize, const T &value=T() )
        {
            if( newSize > mSize )
            {
                growToFit( newSize - mSize );
                for( size_t i=mSize; i<newSize; ++i )
                {
                    new (&mData[i]) T( value );
                }
            }

            mSize = newSize;
        }

        T& operator [] ( size_t idx )
        {
            assert( idx < mSize && "Index out of bounds" );
            return mData[idx];
        }

        const T& operator [] ( size_t idx ) const
        {
            assert( idx < mSize && "Index out of bounds" );
            return mData[idx];
        }

        T& back()
        {
            assert( mSize > 0 && "Can't call back with no elements" );
            return mData[mSize-1];
        }

        const T& back() const
        {
            assert( mSize > 0 && "Can't call back with no elements" );
            return mData[mSize-1];
        }

        T& front()
        {
            assert( mSize > 0 && "Can't call front with no elements" );
            return mData[0];
        }

        const T& front() const
        {
            assert( mSize > 0 && "Can't call front with no elements" );
            return mData[0];
        }

        iterator begin()                        { return mData; }
        const_iterator begin() const            { return mData; }
        iterator end()                          { return mData + mSize; }
        const_iterator end() const              { return mData + mSize; }
    };
}

#endif
