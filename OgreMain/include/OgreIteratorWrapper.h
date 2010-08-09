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
#ifndef __Ogre_Iterator_Wrapper_H__
#define __Ogre_Iterator_Wrapper_H__


namespace Ogre{

/** 
 * 
 * \brief Basefunctionality for IteratorWrappers
 *
 * 
 * \param T a Container like vector list map ...
 * \param IteratorType  T::iterator or T::const_iterator
 * \param ValType  T::mapped_type in case of a map, T::value_type for vector, list,...
 * 
 * have a look at VectorIteratorWrapper and MapIteratorWrapper for a concrete usage
*/
template <typename T, typename IteratorType, typename ValType>
class IteratorWrapper
{

	private:
		/// Private constructor since only the parameterised constructor should be used
		IteratorWrapper();

	protected:
		IteratorType mBegin;
		IteratorType mCurrent;
		IteratorType mEnd;
	

	public:
	
		/// type you expect to get by funktions like peekNext(Value)
		typedef ValType ValueType;
		/// type you expect to get by funktions like peekNext(Value)Ptr
		typedef ValType* PointerType;

		/**
		\brief typedef to fulfill container interface
		
		Userfull if you want to use BOOST_FOREACH
		\note there is no distinction between const_iterator and iterator.
		\n keep this in mind if you want to derivate from this class. 
		*/
		typedef IteratorType iterator;
		
		/**
		\brief typedef to fulfill container interface
		
		Userfull if you want to use BOOST_FOREACH
		\note there is no distinction between const_iterator and iterator.
		\n keep this in mind if you want to derivate from this class. 
		*/
		typedef IteratorType const_iterator;
		
		
		

		
        /** Constructor.
        @remarks
        Provide a start and end iterator to initialise.
        */
		IteratorWrapper ( IteratorType start, IteratorType last )
		: mBegin( start ), mCurrent ( start ), mEnd ( last )
		{
		}


		/** Returns true if there are more items in the collection. */
		bool hasMoreElements ( ) const
		{
			return mCurrent != mEnd;
		}


		/** Moves the iterator on one element. */
		void moveNext (  )
		{
			++mCurrent;
		}

		/** bookmark to the begin of the underlying collection */
		const IteratorType& begin() {return mBegin;}
		
		
		/** full access to the current  iterator */
		IteratorType& current(){return mCurrent;}
		
		/** bookmark to the end (one behind the last element) of the underlying collection */
		const IteratorType& end() {return mEnd;}
		

};



/** 
 * 
 * \brief Prepiared IteratorWrapper for container like std::vector 
 *
 * 
 * \param T = Container eg vector 
 * \param IteratorType  T::iterator or T::const_iterator
 * 
 * have a look at VectorIterator and ConstVectorIterator for a more concrete usage
*/
template <typename T, typename IteratorType>
class VectorIteratorWrapper : public IteratorWrapper<T, IteratorType, typename  T::value_type>
{

	public:
		typedef typename IteratorWrapper<T, IteratorType, typename  T::value_type>::ValueType ValueType ; 
		typedef typename IteratorWrapper<T, IteratorType, typename  T::value_type>::PointerType PointerType ;
	

		/**
		 * \brief c'tor
		 * 
		 * Constructor that provide a start and end iterator to initialise.
		 * 
		 * @param start start iterator 
		 * @param end end iterator 
		 */
		VectorIteratorWrapper ( IteratorType start, IteratorType last )
		: IteratorWrapper<T, IteratorType, typename T::value_type>( start, last ) 
		{
		}


		/** Returns the next(=current) element in the collection, without advancing to the next. */
		ValueType peekNext (  ) const
		{
			return *(this->mCurrent);
		}

		/** Returns a pointer to the next(=current) element in the collection, without advancing to the next afterwards. */
		PointerType peekNextPtr (  )  const
		{
			return &(*(this->mCurrent) );
		}

		/** Returns the next(=current) value element in the collection, and advances to the next. */
		ValueType getNext (  ) 
		{
			return *(this->mCurrent++);
		}	

};


/** 
 * 
 * \brief Concrete IteratorWrapper for nonconst access to the underlying container
 * 
 * \param T  Container 
 * 
*/
template <typename T>
class VectorIterator : public VectorIteratorWrapper<T,  typename T::iterator>{
	public:
        /** Constructor.
        @remarks
            Provide a start and end iterator to initialise.
        */	
		VectorIterator( typename T::iterator start, typename T::iterator last )
		: VectorIteratorWrapper<T,  typename T::iterator>(start , last )
		{
		}

        /** Constructor.
        @remarks
            Provide a container to initialise.
        */
		explicit VectorIterator( T& c )
		: VectorIteratorWrapper<T,  typename T::iterator> ( c.begin(), c.end() )
		{
		}
		
};

/** 
 * 
 * \brief Concrete IteratorWrapper for const access to the underlying container
 *
 * 
 * \param T = Container 
 * 
*/
template <typename T>
class ConstVectorIterator : public VectorIteratorWrapper<T,  typename T::const_iterator>{
	public:
        /** Constructor.
        @remarks
            Provide a start and end iterator to initialise.
        */	
		ConstVectorIterator( typename T::const_iterator start, typename T::const_iterator last )
		: VectorIteratorWrapper<T,  typename T::const_iterator> (start , last )
		{
		}

        /** Constructor.
        @remarks
            Provide a container to initialise.
        */
		explicit ConstVectorIterator ( const T& c )
		 : VectorIteratorWrapper<T,  typename T::const_iterator> (c.begin() , c.end() )
		{
		}
};





/** 
 * 
 * \brief Prepiared IteratorWrapper for key-value container
 *
 * 
 * \param T  Container  (map - or also set )
 * \param  IteratorType T::iterator or T::const_iterator
 * 
 * have a look at MapIterator and ConstMapIterator for a concrete usage
*/
template <typename T, typename IteratorType>
class MapIteratorWrapper  : public IteratorWrapper<T, IteratorType, typename T::mapped_type>
{

	public:
		/// redefined ValueType for a map/set
		typedef typename IteratorWrapper<T, IteratorType, typename  T::mapped_type>::ValueType ValueType ; 
		/// redefined PointerType for a map/set
		typedef typename IteratorWrapper<T, IteratorType, typename  T::mapped_type>::PointerType PointerType ;	
		
		///unused, just to make it clear that map/set::value_type is not a ValueType
		typedef typename T::value_type PairType ; 
		/// type you expect to get by funktions like peekNextKey
		typedef typename T::key_type KeyType;
        
        /** Constructor.
        @remarks
            Provide a start and end iterator to initialise.
        */
		MapIteratorWrapper ( IteratorType start, IteratorType last )
		: IteratorWrapper<T, IteratorType, typename T::mapped_type>( start, last ) 
		{
		}

        /** Returns the next(=current) key element in the collection, without advancing to the next. */
        KeyType peekNextKey(void) const
        {
            return this->mCurrent->first;
        }


		/** Returns the next(=current) value element in the collection, without advancing to the next. */
		ValueType peekNextValue (  ) const
		{
			return this->mCurrent->second;
		}


        /** Returns a pointer to the next/current value element in the collection, without 
        advancing to the next afterwards. */
		const PointerType peekNextValuePtr (  )  const
		{
			return &(this->mCurrent->second);
		}


        /** Returns the next(=current) value element in the collection, and advances to the next. */
        ValueType getNext()
        {
            return ((this->mCurrent++)->second) ;
        }	
	

};




/** 
 * 
 * \brief Concrete IteratorWrapper for nonconst access to the underlying key-value container
 *
 * 
 * \param T key-value container
 * 
*/
template <typename T>
class MapIterator : public MapIteratorWrapper<T,  typename T::iterator>{
	public:
	
        /** Constructor.
        @remarks
            Provide a start and end iterator to initialise.
        */	
		MapIterator( typename T::iterator start, typename T::iterator last )
		: MapIteratorWrapper<T,  typename T::iterator>(start , last )
		{
		}
		
        /** Constructor.
        @remarks
            Provide a container to initialise.
        */
		explicit MapIterator( T& c )
		: MapIteratorWrapper<T,  typename T::iterator> ( c.begin(), c.end() )
		{
		}
		
};


/** 
 * 
 * \brief Concrete IteratorWrapper for const access to the underlying key-value container
 *
 * 
 * \param T key-value container
 * 
*/
template <typename T>
class ConstMapIterator : public MapIteratorWrapper<T,  typename T::const_iterator>{
	public:
	
        /** Constructor.
        @remarks
            Provide a start and end iterator to initialise.
        */	
		ConstMapIterator( typename T::const_iterator start, typename T::const_iterator last )
		: MapIteratorWrapper<T,  typename T::const_iterator> (start , last )
		{
		}

        /** Constructor.
        @remarks
            Provide a container to initialise.
        */
		explicit ConstMapIterator ( const T& c )
		 : MapIteratorWrapper<T,  typename T::const_iterator> (c.begin() , c.end() )
		{
		}
};




}



#endif
