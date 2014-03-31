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
#ifndef _OgreHlms_H_
#define _OgreHlms_H_

//#include "OgrePrerequisites.h"
//#include "OgreHeaderPrefix.h"
#define _OgreExport
#include <string>
#include <vector>
#include <algorithm>
#include <map>
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
typedef int int32;
typedef short int16;
typedef signed char int8;
typedef unsigned __int64 uint64;
typedef __int64 int64;

#include "OgreIdString.h"

namespace Ogre
{
	typedef std::string String;
    typedef std::vector<String> StringVec;

    class SubStringRef
    {
        String const *mOriginal;
        size_t  mStart;
        size_t  mEnd;

    public:
        SubStringRef( const String *original, size_t start ) :
            mOriginal( original ),
            mStart( start ),
            mEnd( original->size() )
        {
            assert( start <= original->size() );
        }

        SubStringRef( const String *original, size_t start, size_t end ) :
            mOriginal( original ),
            mStart( start ),
            mEnd( end )
        {
            assert( start <= end );
            assert( end <= original->size() );
        }

        SubStringRef( const String *original, String::const_iterator start ) :
            mOriginal( original ),
            mStart( start - original->begin() ),
            mEnd( original->size() )
        {
        }

        size_t find( char *value ) const
        {
            size_t retVal = mOriginal->find( value, mStart );
            if( retVal >= mEnd )
                retVal = String::npos;
            else if( retVal != String::npos )
                retVal -= mStart;

            return retVal;
        }

        size_t find( const String &value ) const
        {
            size_t retVal = mOriginal->find( value, mStart );
            if( retVal >= mEnd )
                retVal = String::npos;
            else if( retVal != String::npos )
                retVal -= mStart;

            return retVal;
        }

        void setStart( size_t newStart )            { mStart = std::min( newStart, mOriginal->size() ); }
        void setEnd( size_t newEnd )                { mEnd = std::min( newEnd, mOriginal->size() ); }
        size_t getStart(void) const                 { return mStart; }
        size_t getEnd(void) const                   { return mEnd; }
        size_t getSize(void) const                  { return mEnd - mStart; }
        String::const_iterator begin() const        { return mOriginal->begin() + mStart; }
        String::const_iterator end() const          { return mOriginal->begin() + mEnd; }
        const String& getOriginalBuffer() const     { return *mOriginal; }
    };

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

    struct HlmsParams
    {
        uint32      hash;
        typedef std::vector<std::pair<IdString, String>> ParamVec;
        ParamVec    params;
    };

    /** HLMS stands for "High Level Material System". */
    class _OgreExport Hlms //: public PassAlloc
    {
    protected:
        struct Property
        {
            IdString    keyName;
            int32       value;

            Property( IdString _keyName, int32 _value ) :
                keyName( _keyName ), value( _value ) {}
        };

        static bool OrderPropertyByIdString( const Property &_left, const Property &_right )
        {
            return _left.keyName < _right.keyName;
        }

        uint32 mHash;

        typedef std::vector<Property> PropertyVec;
        typedef std::map<IdString, String> PiecesMap;
        PropertyVec mSetProperties;
        PiecesMap   mPieces;

        /** Inserts common properties about the current Renderable,
            such as hlms_skeleton hlms_uv_count, etc
        */
        void setCommonProperties();

        void setProperty(IdString key, int32 value );
        int32 getProperty( IdString key, int32 defaultVal=0 ) const;

        enum ExpressionType
        {
            EXPR_OPERATOR_OR,        //||
            EXPR_OPERATOR_AND,       //&&
            EXPR_OBJECT,             //(...)
            EXPR_VAR
        };

        struct Expression
        {
            bool                    result;
            bool                    negated;
            ExpressionType          type;
            std::vector<Expression> children;
            String                  value;

            Expression() : type( EXPR_VAR ), result( false ), negated( false ) {}
        };

        typedef std::vector<Expression> ExpressionVec;

        static void copy( String &outBuffer, const SubStringRef &inSubString, size_t length );
        static void repeat( String &outBuffer, const SubStringRef &inSubString, size_t length,
                            size_t passNum, const String &counterVar );
        bool parseForEach( const String &inBuffer, String &outBuffer ) const;
        bool parseProperties( String &inBuffer, String &outBuffer ) const;
        bool collectPieces( const String &inBuffer, String &outBuffer );
        bool insertPieces( String &inBuffer, String &outBuffer ) const;
        bool parseCounter( const String &inBuffer, String &outBuffer );
        bool parse( const String &inBuffer, String &outBuffer ) const;

        /** Goes through 'buffer', starting from startPos (inclusive) looking for the given
            character while skipping whitespace. If any character other than whitespace or
            EOLs if found returns String::npos
        @return
            String::npos if not found or wasn't the next character. If found, the position
            in the buffer, from start
        */
        static size_t findNextCharacter( const String &buffer, size_t startPos, char character );

        static void findBlockEnd( SubStringRef &outSubString , bool &syntaxError );

        bool evaluateExpression( SubStringRef &outSubString, bool &outSyntaxError ) const;
        bool evaluateExpressionRecursive( ExpressionVec &expression, bool &outSyntaxError ) const;
        static size_t evaluateExpressionEnd( const SubStringRef &outSubString );

        static void evaluateParamArgs( SubStringRef &outSubString, StringVec &outArgs,
                                       bool &outSyntaxError );

        static size_t calculateLineCount(const String &buffer, size_t idx );
        static size_t calculateLineCount( const SubStringRef &subString );
        /**
        */

        void updateHash();

    public:
		Hlms();
        virtual ~Hlms();

        //MaterialPtr generateFor( Renderable *renderable, MovableObject *movableObject );
        void generateFor();
    };
    /** @} */
    /** @} */

}

//#include "OgreHeaderSuffix.h"

#endif
