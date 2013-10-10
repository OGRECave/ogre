/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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
		T			*m_data;
		size_t		m_size;
		size_t		m_capacity;

		/** Checks whether we'll be at full capacity after adding N new elements, if so,
			increase the array size by 50%
		@remarks
			Doesn't do anything if available capacity is enough to contain the new elements
			Won't modify m_size, only m_capacity
		@param newElements
			Amount of new elements to push to the array
		*/
		void growToFit( size_t newElements )
		{
			if( m_size + newElements > m_capacity )
			{
				m_capacity = std::max( m_size + newElements, m_capacity + (m_capacity >> 1) + 1 );
				T *data = (T*)::operator new( m_capacity * sizeof(T) );
				memcpy( data, m_data, m_size * sizeof(T) );
				::operator delete( m_data );
				m_data = data;
			}
		}

	public:
		FastArray() :
			m_data( 0 ),
			m_size( 0 ),
			m_capacity( 0 )
		{
		}

		void swap( FastArray<T> &other )
		{
			std::swap( this->m_data, other.m_data );
			std::swap( this->m_size, other.m_size );
			std::swap( this->m_capacity, other.m_capacity );
		}

		FastArray( const FastArray<T> &copy ) :
				m_size( copy.m_size ),
				m_capacity( copy.m_size )
		{
			m_data = (T*)::operator new( m_size * sizeof(T) );
			for( size_t i=0; i<m_size; ++i )
			{
				new (&m_data[i]) T();
				m_data[i] = copy.m_data[i];
			}
		}

		void operator = ( const FastArray<T> &copy )
		{
			if( &copy != this )
			{
				for( size_t i=0; i<m_size; ++i )
					m_data[i].~T();
				::operator delete( m_data );

				m_size		= copy.m_size;
				m_capacity	= copy.m_size;

				m_data = (T*)::operator new( m_size * sizeof(T) );
				for( size_t i=0; i<m_size; ++i )
				{
					new (&m_data[i]) T();
					m_data[i] = copy.m_data[i];
				}
			}
		}

		/// Creates an array reserving the amount of elements (memory is not initialized)
		FastArray( size_t reserveAmount ) :
			m_size( 0 ),
			m_capacity( reserveAmount )
		{
			m_data = (T*)::operator new( reserveAmount * sizeof(T) );
		}

		/// Creates an array pushing the value N times
		FastArray( size_t count, const T &value ) :
			m_size( count ),
			m_capacity( count )
		{
			m_data = (T*)::operator new( count * sizeof(T) );
			for( size_t i=0; i<count; ++i )
			{
				new (&m_data[i]) T();
				m_data[i] = value;
			}
		}

		~FastArray()
		{
			for( size_t i=0; i<m_size; ++i )
				m_data[i].~T();
			::operator delete( m_data );
		}

		size_t size() const						{ return m_size; }
		size_t capacity() const					{ return m_capacity; }

		void push_back( const T& val )
		{
			growToFit( 1 );
			new (&m_data[m_size]) T();
			m_data[m_size] = val;
			++m_size;
		}

		void pop_back()
		{
			assert( m_size > 0 && "Can't pop a zero-sized array" );
			--m_size;
		}

		void clear()
		{
			m_size = 0;
		}

		bool empty() const						{ return m_size == 0; }

		void reserve( size_t reserveAmount )
		{
			if( reserveAmount > m_capacity )
			{
				//We don't use growToFit because it will try to increase capacity by 50%,
				//which is not the desire when calling reserve() explicitly
				m_capacity = reserveAmount;
				T *data = (T*)::operator new( m_capacity * sizeof(T) );
				memcpy( data, m_data, m_size * sizeof(T) );
				::operator delete( m_data );
				m_data = data;
			}
		}

		void resize( size_t newSize, const T &value=T() )
		{
			if( newSize > m_size )
			{
				growToFit( newSize - m_size );
				for( size_t i=m_size; i<newSize; ++i )
				{
					new (&m_data[i]) T();
					m_data[i] = value;
				}
			}

			m_size = newSize;
		}

		T& operator [] ( size_t idx )
        {
			assert( idx < m_size && "Index out of bounds" );
            return m_data[idx];
        }

		const T& operator [] ( size_t idx ) const
        {
			assert( idx < m_size && "Index out of bounds" );
            return m_data[idx];
        }

		T& back()
		{
			assert( m_size > 0 && "Can't call back with no elements" );
			return m_data[m_size-1];
		}

		const T& back() const
		{
			assert( m_size > 0 && "Can't call back with no elements" );
			return m_data[m_size-1];
		}

		T& front()
		{
			assert( m_size > 0 && "Can't call front with no elements" );
			return m_data[0];
		}

		const T& front() const
		{
			assert( m_size > 0 && "Can't call front with no elements" );
			return m_data[0];
		}

		typedef T* iterator;
		typedef const T* const_iterator;

		iterator begin()						{ return m_data; }
		const_iterator begin() const			{ return m_data; }
		iterator end()							{ return m_data + m_size; }
		const_iterator end() const				{ return m_data + m_size; }
	};
}

#endif
