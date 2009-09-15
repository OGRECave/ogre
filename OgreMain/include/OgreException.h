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
#ifndef __Exception_H_
#define __Exception_H_

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreString.h"
#include <exception>

// Backwards compatibility with old assert mode definitions
#if OGRE_RELEASE_ASSERT == 1
#   define OGRE_ASSERT_MODE 1
#endif

// Check for OGRE assert mode

// RELEASE_EXCEPTIONS mode
#if OGRE_ASSERT_MODE == 1
#   ifdef _DEBUG
#       define OgreAssert( a, b ) assert( (a) && (b) )

#   else
#       if OGRE_COMP != OGRE_COMPILER_BORL
#           define OgreAssert( a, b ) if( !(a) ) OGRE_EXCEPT( Ogre::Exception::ERR_RT_ASSERTION_FAILED, (b), "no function info")
#       else
#           define OgreAssert( a, b ) if( !(a) ) OGRE_EXCEPT( Ogre::Exception::ERR_RT_ASSERTION_FAILED, (b), __FUNC__ )
#       endif

#   endif

// EXCEPTIONS mode
#elif OGRE_ASSERT_MODE == 2
#   if OGRE_COMP != OGRE_COMPILER_BORL
#       define OgreAssert( a, b ) if( !(a) ) OGRE_EXCEPT( Ogre::Exception::ERR_RT_ASSERTION_FAILED, (b), "no function info")
#   else
#       define OgreAssert( a, b ) if( !(a) ) OGRE_EXCEPT( Ogre::Exception::ERR_RT_ASSERTION_FAILED, (b), __FUNC__ )
#   endif

// STANDARD mode
#else
#   define OgreAssert( a, b ) assert( (a) && (b) )

#endif

namespace Ogre {
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup General
	*  @{
	*/
	/** When thrown, provides information about an error that has occurred inside the engine.
        @remarks
            OGRE never uses return values to indicate errors. Instead, if an
            error occurs, an exception is thrown, and this is the object that
            encapsulates the detail of the problem. The application using
            OGRE should always ensure that the exceptions are caught, so all
            OGRE engine functions should occur within a
            try{} catch(Ogre::Exception& e) {} block.
        @par
            The user application should never create any instances of this
            object unless it wishes to unify its error handling using the
            same object.
    */
	class _OgreExport Exception : public std::exception
    {
    protected:
        long line;
        int number;
		String typeName;
        String description;
        String source;
        String file;
		mutable String fullDesc;
    public:
        /** Static definitions of error codes.
            @todo
                Add many more exception codes, since we want the user to be able
                to catch most of them.
        */
        enum ExceptionCodes {
            ERR_CANNOT_WRITE_TO_FILE,
            ERR_INVALID_STATE,
            ERR_INVALIDPARAMS,
            ERR_RENDERINGAPI_ERROR,
            ERR_DUPLICATE_ITEM,
            ERR_ITEM_NOT_FOUND,
            ERR_FILE_NOT_FOUND,
            ERR_INTERNAL_ERROR,
            ERR_RT_ASSERTION_FAILED, 
			ERR_NOT_IMPLEMENTED
        };

        /** Default constructor.
        */
        Exception( int number, const String& description, const String& source );

        /** Advanced constructor.
        */
        Exception( int number, const String& description, const String& source, const char* type, const char* file, long line );

        /** Copy constructor.
        */
        Exception(const Exception& rhs);

		/// Needed for  compatibility with std::exception
		~Exception() throw() {}

        /** Assignment operator.
        */
        void operator = (const Exception& rhs);

        /** Returns a string with the full description of this error.
            @remarks
                The description contains the error number, the description
                supplied by the thrower, what routine threw the exception,
                and will also supply extra platform-specific information
                where applicable. For example - in the case of a rendering
                library error, the description of the error will include both
                the place in which OGRE found the problem, and a text
                description from the 3D rendering library, if available.
        */
        virtual const String& getFullDescription(void) const;

        /** Gets the error code.
        */
        virtual int getNumber(void) const throw();

        /** Gets the source function.
        */
        virtual const String &getSource() const { return source; }

        /** Gets source file name.
        */
        virtual const String &getFile() const { return file; }

        /** Gets line number.
        */
        virtual long getLine() const { return line; }

		/** Returns a string with only the 'description' field of this exception. Use 
			getFullDescriptionto get a full description of the error including line number,
			error number and what function threw the exception.
        */
		virtual const String &getDescription(void) const { return description; }

		/// Override std::exception::what
		const char* what() const throw() { return getFullDescription().c_str(); }
        
    };


	/** Template struct which creates a distinct type for each exception code.
	@note
	This is useful because it allows us to create an overloaded method
	for returning different exception types by value without ambiguity. 
	From 'Modern C++ Design' (Alexandrescu 2001).
	*/
	template <int num>
	struct ExceptionCodeType
	{
		enum { number = num };
	};

	// Specialised exceptions allowing each to be caught specifically
	// backwards-compatible since exception codes still used

	class _OgreExport UnimplementedException : public Exception 
	{
	public:
		UnimplementedException(int number, const String& description, const String& source, const char* file, long line)
			: Exception(number, description, source, "UnimplementedException", file, line) {}
	};
	class _OgreExport FileNotFoundException : public Exception
	{
	public:
		FileNotFoundException(int number, const String& description, const String& source, const char* file, long line)
			: Exception(number, description, source, "FileNotFoundException", file, line) {}
	};
	class _OgreExport IOException : public Exception
	{
	public:
		IOException(int number, const String& description, const String& source, const char* file, long line)
			: Exception(number, description, source, "IOException", file, line) {}
	};
	class _OgreExport InvalidStateException : public Exception
	{
	public:
		InvalidStateException(int number, const String& description, const String& source, const char* file, long line)
			: Exception(number, description, source, "InvalidStateException", file, line) {}
	};
	class _OgreExport InvalidParametersException : public Exception
	{
	public:
		InvalidParametersException(int number, const String& description, const String& source, const char* file, long line)
			: Exception(number, description, source, "InvalidParametersException", file, line) {}
	};
	class _OgreExport ItemIdentityException : public Exception
	{
	public:
		ItemIdentityException(int number, const String& description, const String& source, const char* file, long line)
			: Exception(number, description, source, "ItemIdentityException", file, line) {}
	};
	class _OgreExport InternalErrorException : public Exception
	{
	public:
		InternalErrorException(int number, const String& description, const String& source, const char* file, long line)
			: Exception(number, description, source, "InternalErrorException", file, line) {}
	};
	class _OgreExport RenderingAPIException : public Exception
	{
	public:
		RenderingAPIException(int number, const String& description, const String& source, const char* file, long line)
			: Exception(number, description, source, "RenderingAPIException", file, line) {}
	};
	class _OgreExport RuntimeAssertionException : public Exception
	{
	public:
		RuntimeAssertionException(int number, const String& description, const String& source, const char* file, long line)
			: Exception(number, description, source, "RuntimeAssertionException", file, line) {}
	};


	/** Class implementing dispatch methods in order to construct by-value
		exceptions of a derived type based just on an exception code.
	@remarks
		This nicely handles construction of derived Exceptions by value (needed
		for throwing) without suffering from ambiguity - each code is turned into
		a distinct type so that methods can be overloaded. This allows OGRE_EXCEPT
		to stay small in implementation (desirable since it is embedded) whilst
		still performing rich code-to-type mapping. 
	*/
	class ExceptionFactory
	{
	private:
		/// Private constructor, no construction
		ExceptionFactory() {}
	public:
		static UnimplementedException create(
			ExceptionCodeType<Exception::ERR_NOT_IMPLEMENTED> code, 
			const String& desc, 
			const String& src, const char* file, long line)
		{
			return UnimplementedException(code.number, desc, src, file, line);
		}
		static FileNotFoundException create(
			ExceptionCodeType<Exception::ERR_FILE_NOT_FOUND> code, 
			const String& desc, 
			const String& src, const char* file, long line)
		{
			return FileNotFoundException(code.number, desc, src, file, line);
		}
		static IOException create(
			ExceptionCodeType<Exception::ERR_CANNOT_WRITE_TO_FILE> code, 
			const String& desc, 
			const String& src, const char* file, long line)
		{
			return IOException(code.number, desc, src, file, line);
		}
		static InvalidStateException create(
			ExceptionCodeType<Exception::ERR_INVALID_STATE> code, 
			const String& desc, 
			const String& src, const char* file, long line)
		{
			return InvalidStateException(code.number, desc, src, file, line);
		}
		static InvalidParametersException create(
			ExceptionCodeType<Exception::ERR_INVALIDPARAMS> code, 
			const String& desc, 
			const String& src, const char* file, long line)
		{
			return InvalidParametersException(code.number, desc, src, file, line);
		}
		static ItemIdentityException create(
			ExceptionCodeType<Exception::ERR_ITEM_NOT_FOUND> code, 
			const String& desc, 
			const String& src, const char* file, long line)
		{
			return ItemIdentityException(code.number, desc, src, file, line);
		}
		static ItemIdentityException create(
			ExceptionCodeType<Exception::ERR_DUPLICATE_ITEM> code, 
			const String& desc, 
			const String& src, const char* file, long line)
		{
			return ItemIdentityException(code.number, desc, src, file, line);
		}
		static InternalErrorException create(
			ExceptionCodeType<Exception::ERR_INTERNAL_ERROR> code, 
			const String& desc, 
			const String& src, const char* file, long line)
		{
			return InternalErrorException(code.number, desc, src, file, line);
		}
		static RenderingAPIException create(
			ExceptionCodeType<Exception::ERR_RENDERINGAPI_ERROR> code, 
			const String& desc, 
			const String& src, const char* file, long line)
		{
			return RenderingAPIException(code.number, desc, src, file, line);
		}
		static RuntimeAssertionException create(
			ExceptionCodeType<Exception::ERR_RT_ASSERTION_FAILED> code, 
			const String& desc, 
			const String& src, const char* file, long line)
		{
			return RuntimeAssertionException(code.number, desc, src, file, line);
		}

	};
	

	
#ifndef OGRE_EXCEPT
#define OGRE_EXCEPT(num, desc, src) throw Ogre::ExceptionFactory::create( \
	Ogre::ExceptionCodeType<num>(), desc, src, __FILE__, __LINE__ );
#endif
	/** @} */
	/** @} */

} // Namespace Ogre
#endif
