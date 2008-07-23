/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) Any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
Any WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
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
// -- Based on boost::any, original copyright information follows --
// Copyright Kevlin Henney, 2000, 2001, 2002. All rights reserved.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompAnying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -- End original copyright --

#ifndef __OGRE_ANY_H__
#define __OGRE_ANY_H__

#include "OgrePrerequisites.h"
#include "OgreException.h"
#include "OgreString.h"
#include <algorithm>
#include <typeinfo>


namespace Ogre
{
	/** Variant type that can hold Any other type.
	*/
	class Any 
    {
    public: // constructors

        Any()
          : mContent(0)
        {
        }

        template<typename ValueType>
        explicit Any(const ValueType & value)
          : mContent(OGRE_NEW_T(holder<ValueType>, MEMCATEGORY_GENERAL)(value))
        {
        }

        Any(const Any & other)
          : mContent(other.mContent ? other.mContent->clone() : 0)
        {
        }

        virtual ~Any()
        {
            OGRE_DELETE_T(mContent, placeholder, MEMCATEGORY_GENERAL);
        }

    public: // modifiers

        Any& swap(Any & rhs)
        {
            std::swap(mContent, rhs.mContent);
            return *this;
        }

        template<typename ValueType>
        Any& operator=(const ValueType & rhs)
        {
            Any(rhs).swap(*this);
            return *this;
        }

        Any & operator=(const Any & rhs)
        {
            Any(rhs).swap(*this);
            return *this;
        }

    public: // queries

        bool isEmpty() const
        {
            return !mContent;
        }

        const std::type_info& getType() const
        {
            return mContent ? mContent->getType() : typeid(void);
        }

		inline friend std::ostream& operator <<
			( std::ostream& o, const Any& v )
		{
			if (v.mContent)
				v.mContent->writeToStream(o);
			return o;
		}


    protected: // types

		class placeholder 
        {
        public: // structors
    
            virtual ~placeholder()
            {
            }

        public: // queries

            virtual const std::type_info& getType() const = 0;

            virtual placeholder * clone() const = 0;
    
			virtual void writeToStream(std::ostream& o) = 0;

        };

        template<typename ValueType>
        class holder : public placeholder
        {
        public: // structors

            holder(const ValueType & value)
              : held(value)
            {
            }

        public: // queries

            virtual const std::type_info & getType() const
            {
                return typeid(ValueType);
            }

            virtual placeholder * clone() const
            {
                return OGRE_NEW_T(holder, MEMCATEGORY_GENERAL)(held);
            }

			virtual void writeToStream(std::ostream& o)
			{
				o << held;
			}


        public: // representation

            ValueType held;

        };



    protected: // representation
        placeholder * mContent;

        template<typename ValueType>
        friend ValueType * any_cast(Any *);


    public: 

	    template<typename ValueType>
    	ValueType operator()() const
    	{
			if (!mContent) 
			{
				OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
					"Bad cast from uninitialised Any", 
					"Any::operator()");
			}
			else if(getType() == typeid(ValueType))
			{
             	return static_cast<Any::holder<ValueType> *>(mContent)->held;
			}
			else
			{
				StringUtil::StrStreamType str;
				str << "Bad cast from type '" << getType().name() << "' "
					<< "to '" << typeid(ValueType).name() << "'";
				OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
					 str.str(), 
					"Any::operator()");
			}
		}

		

    };


	/** Specialised Any class which has built in arithmetic operators, but can 
		hold only types which support operator +,-,* and / .
	*/
	class AnyNumeric : public Any
	{
	public:
		AnyNumeric()
		: Any()
		{
		}

		template<typename ValueType>
		AnyNumeric(const ValueType & value)
			
		{
			mContent = OGRE_NEW_T(numholder<ValueType>, MEMCATEGORY_GENERAL)(value);
		}

		AnyNumeric(const AnyNumeric & other)
            : Any()
		{
			mContent = other.mContent ? other.mContent->clone() : 0; 
		}

	protected:
		class numplaceholder : public Any::placeholder
		{
		public: // structors

			~numplaceholder()
			{
			}
			virtual placeholder* add(placeholder* rhs) = 0;
			virtual placeholder* subtract(placeholder* rhs) = 0;
			virtual placeholder* multiply(placeholder* rhs) = 0;
			virtual placeholder* multiply(Real factor) = 0;
			virtual placeholder* divide(placeholder* rhs) = 0;
		};

		template<typename ValueType>
		class numholder : public numplaceholder
		{
		public: // structors

			numholder(const ValueType & value)
				: held(value)
			{
			}

		public: // queries

			virtual const std::type_info & getType() const
			{
				return typeid(ValueType);
			}

			virtual placeholder * clone() const
			{
				return OGRE_NEW_T(numholder, MEMCATEGORY_GENERAL)(held);
			}

			virtual placeholder* add(placeholder* rhs)
			{
				return OGRE_NEW_T(numholder, MEMCATEGORY_GENERAL)(held + static_cast<numholder*>(rhs)->held);
			}
			virtual placeholder* subtract(placeholder* rhs)
			{
				return OGRE_NEW_T(numholder, MEMCATEGORY_GENERAL)(held - static_cast<numholder*>(rhs)->held);
			}
			virtual placeholder* multiply(placeholder* rhs)
			{
				return OGRE_NEW_T(numholder, MEMCATEGORY_GENERAL)(held * static_cast<numholder*>(rhs)->held);
			}
			virtual placeholder* multiply(Real factor)
			{
				return OGRE_NEW_T(numholder, MEMCATEGORY_GENERAL)(held * factor);
			}
			virtual placeholder* divide(placeholder* rhs)
			{
				return OGRE_NEW_T(numholder, MEMCATEGORY_GENERAL)(held / static_cast<numholder*>(rhs)->held);
			}
			virtual void writeToStream(std::ostream& o)
			{
				o << held;
			}

		public: // representation

			ValueType held;

		};

		/// Construct from holder
		AnyNumeric(placeholder* pholder)
		{
			mContent = pholder;
		}

	public:
		AnyNumeric & operator=(const AnyNumeric & rhs)
		{
			AnyNumeric(rhs).swap(*this);
			return *this;
		}
		AnyNumeric operator+(const AnyNumeric& rhs) const
		{
			return AnyNumeric(
				static_cast<numplaceholder*>(mContent)->add(rhs.mContent));
		}
		AnyNumeric operator-(const AnyNumeric& rhs) const
		{
			return AnyNumeric(
				static_cast<numplaceholder*>(mContent)->subtract(rhs.mContent));
		}
		AnyNumeric operator*(const AnyNumeric& rhs) const
		{
			return AnyNumeric(
				static_cast<numplaceholder*>(mContent)->multiply(rhs.mContent));
		}
		AnyNumeric operator*(Real factor) const
		{
			return AnyNumeric(
				static_cast<numplaceholder*>(mContent)->multiply(factor));
		}
		AnyNumeric operator/(const AnyNumeric& rhs) const
		{
			return AnyNumeric(
				static_cast<numplaceholder*>(mContent)->divide(rhs.mContent));
		}
		AnyNumeric& operator+=(const AnyNumeric& rhs)
		{
			*this = AnyNumeric(
				static_cast<numplaceholder*>(mContent)->add(rhs.mContent));
			return *this;
		}
		AnyNumeric& operator-=(const AnyNumeric& rhs)
		{
			*this = AnyNumeric(
				static_cast<numplaceholder*>(mContent)->subtract(rhs.mContent));
			return *this;
		}
		AnyNumeric& operator*=(const AnyNumeric& rhs)
		{
			*this = AnyNumeric(
				static_cast<numplaceholder*>(mContent)->multiply(rhs.mContent));
			return *this;
		}
		AnyNumeric& operator/=(const AnyNumeric& rhs)
		{
			*this = AnyNumeric(
				static_cast<numplaceholder*>(mContent)->divide(rhs.mContent));
			return *this;
		}




	};


    template<typename ValueType>
    ValueType * any_cast(Any * operand)
    {
        return operand && operand->getType() == typeid(ValueType)
                    ? &static_cast<Any::holder<ValueType> *>(operand->mContent)->held
                    : 0;
    }

    template<typename ValueType>
    const ValueType * any_cast(const Any * operand)
    {
        return any_cast<ValueType>(const_cast<Any *>(operand));
    }

    template<typename ValueType>
    ValueType any_cast(const Any & operand)
    {
        const ValueType * result = any_cast<ValueType>(&operand);
        if(!result)
		{
			StringUtil::StrStreamType str;
			str << "Bad cast from type '" << operand.getType().name() << "' "
				<< "to '" << typeid(ValueType).name() << "'";
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
				str.str(), 
				"Ogre::any_cast");
		}
        return *result;
    }


}

#endif

