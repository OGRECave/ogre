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
#       define OgreAssert( a, b ) if( !(a) ) OGRE_EXCEPT_2( Ogre::Exception::ERR_RT_ASSERTION_FAILED, (#a " failed. " b) )
#   endif

// EXCEPTIONS mode
#elif OGRE_ASSERT_MODE == 2
#   define OgreAssert( a, b ) if( !(a) ) OGRE_EXCEPT_2( Ogre::Exception::ERR_RT_ASSERTION_FAILED, (#a " failed. " b) )
// STANDARD mode
#else
/** Checks a condition at runtime and throws exception/ aborts if it fails.
 *
 * The macros OgreAssert (and OgreAssertDbg) evaluate the specified expression.
 * If it is 0, OgreAssert raises an error (see Ogre::RuntimeAssertionException) in Release configuration
 * and aborts in Debug configuration.
 * The macro OgreAssert checks the condition in both Debug and Release configurations
 * while OgreAssertDbg is only retained in the Debug configuration.
 *
 * To always abort instead of throwing an exception or disabling OgreAssert in Release configuration altogether,
 * set OGRE_ASSERT_MODE in CMake accordingly.
 */
#   define OgreAssert( expr, mesg ) assert( (expr) && (mesg) )
#endif

#if OGRE_DEBUG_MODE
#   define OgreAssertDbg( a, b ) OgreAssert( a, b )
#else
/// replaced with OgreAssert(expr, mesg) in Debug configuration
#   define OgreAssertDbg( expr, mesg )
#endif

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup General
    *  @{
    */
    /** When thrown, provides information about an error that has occurred inside the engine.

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
        const char* typeName;
        String description;
        String source;
        const char* file;
        String fullDesc; // storage for char* returned by what()
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
            ERR_ITEM_NOT_FOUND = ERR_DUPLICATE_ITEM,
            ERR_FILE_NOT_FOUND,
            ERR_INTERNAL_ERROR,
            ERR_RT_ASSERTION_FAILED,
            ERR_NOT_IMPLEMENTED,
            ERR_INVALID_CALL
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

        /** Returns a string with the full description of this error.

                The description contains the error number, the description
                supplied by the thrower, what routine threw the exception,
                and will also supply extra platform-specific information
                where applicable. For example - in the case of a rendering
                library error, the description of the error will include both
                the place in which OGRE found the problem, and a text
                description from the 3D rendering library, if available.
        */
        const String& getFullDescription(void) const { return fullDesc; }

        /** Gets the source function.
        */
        const String &getSource() const { return source; }

        /** Gets source file name.
        */
        const char* getFile() const { return file; }

        /** Gets line number.
        */
        long getLine() const { return line; }

        /** Returns a string with only the 'description' field of this exception. Use 
            getFullDescriptionto get a full description of the error including line number,
            error number and what function threw the exception.
        */
        const String &getDescription(void) const { return description; }

        const char* what() const throw() override { return fullDesc.c_str(); }
        
    };


    /** Template struct which creates a distinct type for each exception code.
    @note
    This is useful because it allows us to create an overloaded method
    for returning different exception types by value without ambiguity. 
    From 'Modern C++ Design' (Alexandrescu 2001).
    */
    class _OgreExport UnimplementedException : public Exception 
    {
    public:
        UnimplementedException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
            : Exception(inNumber, inDescription, inSource, __FUNCTION__, inFile, inLine) {}
    };
    class _OgreExport FileNotFoundException : public Exception
    {
    public:
        FileNotFoundException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
            : Exception(inNumber, inDescription, inSource, __FUNCTION__, inFile, inLine) {}
    };
    class _OgreExport IOException : public Exception
    {
    public:
        IOException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
            : Exception(inNumber, inDescription, inSource, __FUNCTION__, inFile, inLine) {}
    };
    class _OgreExport InvalidStateException : public Exception
    {
    public:
        InvalidStateException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
            : Exception(inNumber, inDescription, inSource, __FUNCTION__, inFile, inLine) {}
    };
    class _OgreExport InvalidParametersException : public Exception
    {
    public:
        InvalidParametersException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
            : Exception(inNumber, inDescription, inSource, __FUNCTION__, inFile, inLine) {}
    };
    class _OgreExport ItemIdentityException : public Exception
    {
    public:
        ItemIdentityException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
            : Exception(inNumber, inDescription, inSource, __FUNCTION__, inFile, inLine) {}
    };
    class _OgreExport InternalErrorException : public Exception
    {
    public:
        InternalErrorException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
            : Exception(inNumber, inDescription, inSource, __FUNCTION__, inFile, inLine) {}
    };
    class _OgreExport RenderingAPIException : public Exception
    {
    public:
        RenderingAPIException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
            : Exception(inNumber, inDescription, inSource, __FUNCTION__, inFile, inLine) {}
    };
    class _OgreExport RuntimeAssertionException : public Exception
    {
    public:
        RuntimeAssertionException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
            : Exception(inNumber, inDescription, inSource, __FUNCTION__, inFile, inLine) {}
    };
    class _OgreExport InvalidCallException : public Exception
    {
    public:
        InvalidCallException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
            : Exception(inNumber, inDescription, inSource, __FUNCTION__, inFile, inLine) {}
    };

    /** Class implementing dispatch methods in order to construct by-value
        exceptions of a derived type based just on an exception code.

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
        static OGRE_NORETURN void _throwException(
            Exception::ExceptionCodes code, int number,
            const String& desc, 
            const String& src, const char* file, long line)
        {
            switch (code)
            {
            case Exception::ERR_CANNOT_WRITE_TO_FILE:   throw IOException(number, desc, src, file, line);
            case Exception::ERR_INVALID_STATE:          throw InvalidStateException(number, desc, src, file, line);
            case Exception::ERR_INVALIDPARAMS:          throw InvalidParametersException(number, desc, src, file, line);
            case Exception::ERR_RENDERINGAPI_ERROR:     throw RenderingAPIException(number, desc, src, file, line);
            case Exception::ERR_DUPLICATE_ITEM:         throw ItemIdentityException(number, desc, src, file, line);
            case Exception::ERR_FILE_NOT_FOUND:         throw FileNotFoundException(number, desc, src, file, line);
            case Exception::ERR_INTERNAL_ERROR:         throw InternalErrorException(number, desc, src, file, line);
            case Exception::ERR_RT_ASSERTION_FAILED:    throw RuntimeAssertionException(number, desc, src, file, line);
            case Exception::ERR_NOT_IMPLEMENTED:        throw UnimplementedException(number, desc, src, file, line);
            case Exception::ERR_INVALID_CALL:           throw InvalidCallException(number, desc, src, file, line);
            default:                                    throw Exception(number, desc, src, "Exception", file, line);
            }
        }
    public:
        static OGRE_NORETURN void throwException(
            Exception::ExceptionCodes code,
            const String& desc,
            const String& src, const char* file, long line)
        {
            _throwException(code, code, desc, src, file, line);
        }
    };
    

    
#ifndef OGRE_EXCEPT
#define OGRE_EXCEPT_3(code, desc, src)  Ogre::ExceptionFactory::throwException(code, desc, src, __FILE__, __LINE__)
#define OGRE_EXCEPT_2(code, desc)       Ogre::ExceptionFactory::throwException(code, desc, __FUNCTION__, __FILE__, __LINE__)
#define OGRE_EXCEPT_CHOOSER(arg1, arg2, arg3, arg4, ...) arg4
#define OGRE_EXPAND(x) x // MSVC workaround
#define OGRE_EXCEPT(...) OGRE_EXPAND(OGRE_EXCEPT_CHOOSER(__VA_ARGS__, OGRE_EXCEPT_3, OGRE_EXCEPT_2)(__VA_ARGS__))
#endif
    /** @} */
    /** @} */

} // Namespace Ogre

#include "OgreHeaderSuffix.h"

#endif
