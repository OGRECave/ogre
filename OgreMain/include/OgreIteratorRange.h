/*
 -----------------------------------------------------------------------------
 This source file is part of OGRE
 (Object-oriented Graphics Rendering Engine)
 For the latest info, see http://www.ogre3d.org/
 
 Copyright (c) 2000-2009 Torus Knot Software Ltd
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
#ifndef __Ogre_Iterator_Range_H__
#define __Ogre_Iterator_Range_H__


//#define OGRE_USE_BOOST 1 - picked up by CMake
#if OGRE_USE_BOOST
#include <boost/range.hpp>
#endif

namespace Ogre {

/** 
 * 
 * \brief Base for an iterator_range
 * 
 * \param T iterator type   
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
		\brief typedef to fulfill container interface
		
		\note there is no distinction between const_iterator and iterator.
		
		*/				
		typedef T iterator;
		
		/**
		\brief typedef to fulfill container interface
		
		\note there is no distinction between const_iterator and iterator.
		
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
 * \brief Predefined type
 * 
 * \param T iterator type   
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
 * \brief Predefined type
 * 
 * \param T iterator type   
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
 * \brief Predefined type
 * 
 * \param T iterator type   
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
 * \brief Predefined type
 * 
 * \param T iterator type   
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



}
#endif
