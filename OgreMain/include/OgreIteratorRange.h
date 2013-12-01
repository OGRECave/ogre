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
#ifndef __Ogre_Iterator_Range_H__
#define __Ogre_Iterator_Range_H__

#include "OgreHeaderPrefix.h"

#if OGRE_USE_BOOST
#   if OGRE_COMPILER == OGRE_COMPILER_CLANG || OGRE_COMPILER == OGRE_COMPILER_GNUC
#       pragma GCC diagnostic push
#if OGRE_COMPILER == OGRE_COMPILER_GNUC
#       pragma GCC diagnostic ignored "-Wpragmas"
#elif OGRE_COMPILER == OGRE_COMPILER_CLANG
#       pragma GCC diagnostic ignored "-Wdocumentation"
#endif
#       pragma GCC diagnostic ignored "-Wshadow"
#       pragma GCC diagnostic ignored "-Wpadded"
#       pragma GCC diagnostic ignored "-Wweak-vtables"
#       pragma GCC diagnostic ignored "-Wall"
#       pragma GCC diagnostic ignored "-Wundef"
#   endif

#   include <boost/range.hpp>

#   if OGRE_COMPILER == OGRE_COMPILER_CLANG || OGRE_COMPILER == OGRE_COMPILER_GNUC
#       pragma GCC diagnostic pop
#   endif
#endif

namespace Ogre {

/** 
 * 
 * @brief Base for an iterator_range
 * 
 * @tparam T iterator type
 * 
 * This class implements the minimal interface of the (boost::iterator_)range concept
 *\n Also it prepairs for direct usage of boost::iterator_range by providing the real used type via iterator_range::type
 *   so that client code does not have to change when boost::iterator_range will be implemented some day.
 *\n see VectorRange MapRange or corresponding Const variants for a sample of concrete usage of the iterator_range::type
*/
template <typename T>
class iterator_range{

#if !OGRE_USE_BOOST
	
	T mBegin, mEnd;
	
	public : 
	
        /** Constructor.
        @remarks
        Provide a start and end iterator to initialise.
        */
		iterator_range( T b , T e ) : mBegin(b) , mEnd(e){}

		///access to the begin of the range
		T begin() const { return mBegin; }
		
		///access to the end of the range
		T end() const 	{ return mEnd;  } 	

		///informs if there are any elements in the range
		bool empty() const { return mBegin = mEnd ; }

		///comparison for equality
		bool equal( const iterator_range& other ) const  
		{return mBegin == other.mBegin && mEnd == other.mEnd;}

		///comparison operator for equality
		bool operator==( const iterator_range& rhs ) const
		{return equal( rhs ) ;}
	
		///comparison operator for inequality
		bool operator!=( const iterator_range& rhs ) const { return !operator==(rhs); }

		/**
		@brief typedef to fulfill container interface
		
		@note there is no distinction between const_iterator and iterator.
		
		*/				
		typedef T iterator;
		
		/**
		@brief typedef to fulfill container interface
		
		@note there is no distinction between const_iterator and iterator.
		
		*/						
		typedef T const_iterator;
		
		/// defines the real used type
		/**
			type will be defined as Ogre::iterator_range if not used with boost
			\n otherwise the type will be boost::iterator_range
		*/
		typedef iterator_range<T> type;
#else
		/// defines (this) type as boost::iterator_range
		public: typedef boost::iterator_range<T> type ;

#endif
}; 


/** 
 * 
 * @brief Predefined type
 * 
 * @tparam T iterator type
 *
 * compatility class for VectorIterator
*/
template<typename T>
struct VectorRange : public iterator_range<typename T::iterator>::type
{

	/** Constructor.
	@remarks
		Provide a container to initialise.
	*/
	VectorRange( T& c )
	: iterator_range<typename T::iterator>::type( c.begin(), c.end() )
	{}

	/** Constructor.
	@remarks
	Provide a start and end iterator to initialise.
	*/
	VectorRange( typename T::iterator b, typename T::iterator e )
	: iterator_range<typename T::iterator>::type( b, e )
	{}

	///comparison operator for equality
	bool operator==( const VectorRange& rhs ) const { return equal( rhs) ; }
	///comparison operator for inequality
	bool operator!=( const VectorRange& rhs ) const { return !equal( rhs) ; }


#ifdef __Ogre_Iterator_Wrapper_H__
	///cast operator to a VectorIterator
	operator VectorIterator<T>(){return VectorIterator<T>( this->begin(), this->end());}
	///cast operator to a ConstVectorIterator
	operator ConstVectorIterator<T>(){return ConstVectorIterator<T>( this->begin(), this->end());}
#endif
	
};

/** 
 * 
 * @brief Predefined type
 * 
 * @tparam T iterator type
 *
 * compatility class for ConstVectorIterator
*/
template<typename T>
struct ConstVectorRange : public iterator_range<typename T::const_iterator>::type
{

	/** Constructor.
	@remarks
		Provide a container to initialise.
	*/
	ConstVectorRange( const T& c )
	: iterator_range<typename T::const_iterator>::type( c.begin(), c.end() )
	{}

	/** Constructor.
	@remarks
	Provide a start and end iterator to initialise.
	*/
	ConstVectorRange( typename T::iterator b, typename T::iterator e )
	: iterator_range<typename T::const_iterator>::type( b, e )
	{}

	/** Constructor.
	@remarks
	Provide a start and end const_iterator to initialise.
	*/
	ConstVectorRange( typename T::const_iterator b, typename T::const_iterator e )
	: iterator_range<typename T::const_iterator>::type( b, e )
	{}

	/** Constructor.
	@remarks
	Provide a VectorRange to initialise.
	*/
	ConstVectorRange( const VectorRange<T>& rhs  )
	: iterator_range<typename T::const_iterator>::type( rhs.begin(), rhs.end() )
	{}

	///comparison operator for equality
	bool operator==( const ConstVectorRange& rhs ) const { return equal( rhs) ; }
	///comparison operator for inequality
	bool operator!=( const ConstVectorRange& rhs ) const { return !equal( rhs) ; }
	
	


#ifdef __Ogre_Iterator_Wrapper_H__
	///cast operator to a ConstVectorIterator
	operator ConstVectorIterator<T>(){return  ConstVectorIterator<T>( this->begin(),this->end());}
#endif
	
};



/** 
 * 
 * @brief Predefined type
 * 
 * @tparam T iterator type
 *
 * compatility class for MapIterator
*/
template<typename T>
struct MapRange : public iterator_range<typename T::iterator>::type
{
	/** Constructor.
	@remarks
		Provide a container to initialise.
	*/
	MapRange( T& c )
	: iterator_range<typename T::iterator>::type( c.begin(), c.end() )
	{}

	/** Constructor.
	@remarks
	Provide a start and end iterator to initialise.
	*/
	MapRange( typename T::iterator b, typename T::iterator e )
	: iterator_range<typename T::iterator>::type( b, e )
	{}

	///comparison operator for equality
	bool operator==( const MapRange& rhs ) const { return equal( rhs) ; }
	///comparison operator for inequality
	bool operator!=( const MapRange& rhs ) const { return !equal( rhs) ; }


#ifdef __Ogre_Iterator_Wrapper_H__
	///cast operator to a MapIterator
	operator MapIterator<T>(){return MapIterator<T>( this->begin(), this->end());}
	///cast operator to a ConstMapIterator
	operator ConstMapIterator<T>(){return ConstMapIterator<T>( this->begin(), this->end());}
#endif
	
};

/** 
 * 
 * @brief Predefined type
 * 
 * @tparam T iterator type   
 *
 * compatility class for ConstMapIterator
*/
template<typename T>
struct ConstMapRange : public iterator_range<typename T::const_iterator>::type
{

	/** Constructor.
	@remarks
		Provide a container to initialise.
	*/
	ConstMapRange( const T& c )
	: iterator_range<typename T::const_iterator>::type( c.begin(), c.end() )
	{}

	/** Constructor.
	@remarks
	Provide a start and end iterator to initialise.
	*/
	ConstMapRange( typename T::iterator b, typename T::iterator e )
	: iterator_range<typename T::const_iterator>::type( b, e )
	{}
	
	/** Constructor.
	@remarks
	Provide a start and end const_iterator to initialise.
	*/
	ConstMapRange( typename T::const_iterator b, typename T::const_iterator e )
	: iterator_range<typename T::const_iterator>::type( b, e )
	{}

	/** Constructor.
	@remarks
	Provide a MapRange to initialise.
	*/
	ConstMapRange( const MapRange<T>& rhs  )
	: iterator_range<typename T::const_iterator>::type( rhs.begin(), rhs.end() )
	{}
	
	///comparison operator for equality
	bool operator==( const ConstMapRange& rhs ) const { return equal( rhs) ; }
	///comparison operator for inequality
	bool operator!=( const ConstMapRange& rhs ) const { return !equal( rhs) ; }


#ifdef __Ogre_Iterator_Wrapper_H__
	///cast operator to a ConstMapIterator
	operator ConstMapIterator<T>(){return  ConstMapIterator<T>( this->begin(),this->end());}
#endif
	
};

#include "OgreHeaderSuffix.h"

}
#endif
