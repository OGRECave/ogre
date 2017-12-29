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
#include <typeinfo>
#include "OgreHeaderPrefix.h"

namespace Ogre
{
	// resolve circular dependancy
    class Any;
    template<typename ValueType> ValueType
    any_cast(const Any & operand);

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup General
    *  @{
    */
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
            reset();
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

        bool has_value() const
        {
            return mContent != NULL;
        }

        /// @deprecated use has_value() instead
        OGRE_DEPRECATED bool isEmpty() const { return !has_value(); }

        const std::type_info& type() const
        {
            return mContent ? mContent->getType() : typeid(void);
        }

        /// @deprecated use type() instead
        OGRE_DEPRECATED const std::type_info& getType() const { return type(); }

        inline friend std::ostream& operator <<
            ( std::ostream& o, const Any& v )
        {
            if (v.mContent)
                v.mContent->writeToStream(o);
            return o;
        }

        void reset()
        {
            OGRE_DELETE_T(mContent, placeholder, MEMCATEGORY_GENERAL);
            mContent = NULL;
        }

        /// @deprecated use reset() instead
        OGRE_DEPRECATED void destroy() { reset(); }

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
        /// @deprecated use Ogre::any_cast instead
        template<typename ValueType>
        OGRE_DEPRECATED ValueType operator()() const
        {
            return any_cast<ValueType>(*this);
        }

        /// @deprecated use Ogre::any_cast instead
        template <typename ValueType>
        OGRE_DEPRECATED ValueType get(void) const
        {
            return any_cast<ValueType>(*this);
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
        return operand &&
#if OGRE_COMPILER == OGRE_COMPILER_GNUC && OGRE_COMP_VER < 450
                (std::strcmp(operand->type().name(), typeid(ValueType).name()) == 0)
#else
                (operand->type() == typeid(ValueType))
#endif
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
            StringStream str;
            str << "Bad cast from type '" << operand.type().name() << "' "
                << "to '" << typeid(ValueType).name() << "'";
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                str.str(), 
                "Ogre::any_cast");
        }
        return *result;
    }
    /** @} */
    /** @} */


}

#include "OgreHeaderSuffix.h"

#endif

