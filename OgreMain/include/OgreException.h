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
#ifndef __Exception_H_
#define __Exception_H_

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreString.h"
#include <exception>
#include "OgreHeaderPrefix.h"

// Check for OGRE assert mode

// RELEASE_EXCEPTIONS mode
#if OGRE_ASSERT_MODE == 1
#   if OGRE_DEBUG_MODE
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

		/// Needed for compatibility with std::exception
		~Exception() throw() {}

        /** Assignment operator.
        */
        Exception & operator = (const Exception& rhs);

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


	// Specialised exceptions allowing each to be caught specifically
	// backwards-compatible since exception codes still used

	class _OgreExport UnimplementedException : public Exception 
	{
	public:
		UnimplementedException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
			: Exception(inNumber, inDescription, inSource, "UnimplementedException", inFile, inLine) {}
	};
	class _OgreExport FileNotFoundException : public Exception
	{
	public:
		FileNotFoundException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
			: Exception(inNumber, inDescription, inSource, "FileNotFoundException", inFile, inLine) {}
	};
	class _OgreExport IOException : public Exception
	{
	public:
		IOException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
			: Exception(inNumber, inDescription, inSource, "IOException", inFile, inLine) {}
	};
	class _OgreExport InvalidStateException : public Exception
	{
	public:
		InvalidStateException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
			: Exception(inNumber, inDescription, inSource, "InvalidStateException", inFile, inLine) {}
	};
	class _OgreExport InvalidParametersException : public Exception
	{
	public:
		InvalidParametersException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
			: Exception(inNumber, inDescription, inSource, "InvalidParametersException", inFile, inLine) {}
	};
	class _OgreExport ItemIdentityException : public Exception
	{
	public:
		ItemIdentityException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
			: Exception(inNumber, inDescription, inSource, "ItemIdentityException", inFile, inLine) {}
	};
	class _OgreExport InternalErrorException : public Exception
	{
	public:
		InternalErrorException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
			: Exception(inNumber, inDescription, inSource, "InternalErrorException", inFile, inLine) {}
	};
	class _OgreExport RenderingAPIException : public Exception
	{
	public:
		RenderingAPIException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
			: Exception(inNumber, inDescription, inSource, "RenderingAPIException", inFile, inLine) {}
	};
	class _OgreExport RuntimeAssertionException : public Exception
	{
	public:
		RuntimeAssertionException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
			: Exception(inNumber, inDescription, inSource, "RuntimeAssertionException", inFile, inLine) {}
	};


	/** Class implementing dispatch method in order to construct by-value
		exceptions of a derived type based on an exception code.
	*/
	class ExceptionFactory
	{
	private:
		/// Private constructor, no construction
		ExceptionFactory() {}
	public:
		static OGRE_NORETURN void throwException(
			Exception::ExceptionCodes code, int number,
			const String& desc,
			const String& src, const char* file, long line)
		{
			switch (code)
			{
			case Exception::ERR_CANNOT_WRITE_TO_FILE:	throw IOException(number, desc, src, file, line);
			case Exception::ERR_INVALID_STATE:			throw InvalidStateException(number, desc, src, file, line);
			case Exception::ERR_INVALIDPARAMS:			throw InvalidParametersException(number, desc, src, file, line);
			case Exception::ERR_RENDERINGAPI_ERROR:		throw RenderingAPIException(number, desc, src, file, line);
			case Exception::ERR_DUPLICATE_ITEM:			throw ItemIdentityException(number, desc, src, file, line);
			case Exception::ERR_ITEM_NOT_FOUND:			throw ItemIdentityException(number, desc, src, file, line);
			case Exception::ERR_FILE_NOT_FOUND:			throw FileNotFoundException(number, desc, src, file, line);
			case Exception::ERR_INTERNAL_ERROR:			throw InternalErrorException(number, desc, src, file, line);
			case Exception::ERR_RT_ASSERTION_FAILED:	throw RuntimeAssertionException(number, desc, src, file, line);
			case Exception::ERR_NOT_IMPLEMENTED:		throw UnimplementedException(number, desc, src, file, line);
			default:									throw Exception(number, desc, src, "Exception", file, line);
			}
		}
	};
	

	
#ifndef OGRE_EXCEPT
#define OGRE_EXCEPT(code, desc, src)         Ogre::ExceptionFactory::throwException(code, code, desc, src, __FILE__, __LINE__)
#define OGRE_EXCEPT_EX(code, num, desc, src) Ogre::ExceptionFactory::throwException(code, num, desc, src, __FILE__, __LINE__)
#else
#define OGRE_EXCEPT_EX(code, num, desc, src) OGRE_EXCEPT(code, desc, src)
#endif
	/** @} */
	/** @} */

} // Namespace Ogre

#include "OgreHeaderSuffix.h"

#endif
