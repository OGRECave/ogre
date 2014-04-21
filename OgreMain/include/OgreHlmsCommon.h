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
#ifndef _OgreHlmsCommon_H_
#define _OgreHlmsCommon_H_

#include "OgrePrerequisites.h"
#include "OgreIdString.h"
#include "OgreBlendMode.h"
#include "OgreVector3.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

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

        size_t find( const char *value ) const
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

    struct HlmsProperty
    {
        IdString    keyName;
        int32       value;

        HlmsProperty( IdString _keyName, int32 _value ) :
            keyName( _keyName ), value( _value ) {}

        bool operator == ( const HlmsProperty &_r) const
        {
            return this->keyName == _r.keyName && this->value == _r.value;
        }
    };

    typedef vector<HlmsProperty>::type HlmsPropertyVec;

    inline bool OrderPropertyByIdString( const HlmsProperty &_left, const HlmsProperty &_right )
    {
        return _left.keyName < _right.keyName;
    }

    typedef vector< std::pair<IdString, String> >::type HlmsParamVec;

    inline bool OrderParamVecByKey( const std::pair<IdString, String> &_left,
                                    const std::pair<IdString, String> &_right )
    {
        return _left.first < _right.second;
    }

    /** Up to 8 different HLMS generator types are allowed. The maximum values must be in sync
        with ShaderBits in RenderQueue.cpp (the higher 3 bits)
    */
    enum HlmsTypes
    {
        HLMS_PBS,       /// Physically Based Shader Generator
        HLMS_TOON,      /// Toon shading / Cel shading
        HLMS_UI,        /// Made specifically for GUIs and overlays

        HLMS_USER0,
        HLMS_USER1,
        HLMS_USER2,
        HLMS_USER3,
        HLMS_USER4,

        HLMS_MAX = 8
    };

    struct HlmsCache
    {
        uint32          hash;
        HlmsPropertyVec setProperties;
        HlmsTypes       type;

        GpuProgramPtr   vertexShader;
        GpuProgramPtr   geometryShader;
        GpuProgramPtr   tesselationHullShader;
        GpuProgramPtr   tesselationDomainShader;
        GpuProgramPtr   pixelShader;

        HlmsCache( uint32 _hash, HlmsTypes _type ) : hash( _hash ), type( _type ) {}
    };

    typedef vector<HlmsCache>::type HlmsCacheVec;

    inline bool OrderCacheByHash( const HlmsCache &_left, const HlmsCache &_right )
    {
        return _left.hash < _right.hash;
    }

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
