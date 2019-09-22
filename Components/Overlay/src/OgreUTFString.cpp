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
#include "OgreUTFString.h"

namespace Ogre {

    //--------------------------------------------------------------------------
    UTFString::_base_iterator::_base_iterator()
    {
        mString = 0;
    }
    //--------------------------------------------------------------------------
    void UTFString::_base_iterator::_seekFwd( size_type c )
    {
        mIter += c;
    }
    //--------------------------------------------------------------------------
    void UTFString::_base_iterator::_seekRev( size_type c )
    {
        mIter -= c;
    }
    //--------------------------------------------------------------------------
    void UTFString::_base_iterator::_become( const _base_iterator& i )
    {
        mIter = i.mIter;
        mString = i.mString;
    }
    //--------------------------------------------------------------------------
    bool UTFString::_base_iterator::_test_begin() const
    {
        return mIter == mString->mData.begin();
    }
    //--------------------------------------------------------------------------
    bool UTFString::_base_iterator::_test_end() const
    {
        return mIter == mString->mData.end();
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString::size_type UTFString::_base_iterator::_get_index() const
    {
        return mIter - mString->mData.begin();
    }
    //--------------------------------------------------------------------------
    void UTFString::_base_iterator::_jump_to( size_type index )
    {
        mIter = mString->mData.begin() + index;
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString::unicode_char UTFString::_base_iterator::_getCharacter() const
    {
        size_type current_index = _get_index();
        return mString->getChar( current_index );
    }
    //--------------------------------------------------------------------------
    int UTFString::_base_iterator::_setCharacter( unicode_char uc )
    {
        size_type current_index = _get_index();
        int change = mString->setChar( current_index, uc );
        _jump_to( current_index );
        return change;
    }
    //--------------------------------------------------------------------------
    void UTFString::_base_iterator::_moveNext()
    {
        _seekFwd( 1 ); // move 1 code point forward
        if ( _test_end() ) return; // exit if we hit the end
        if ( _utf16_surrogate_follow( mIter[0] ) ) {
            // landing on a follow code point means we might be part of a bigger character
            // so we test for that
            code_point lead_half = 0;
            //NB: we can't possibly be at the beginning here, so no need to test
            lead_half = mIter[-1]; // check the previous code point to see if we're part of a surrogate pair
            if ( _utf16_surrogate_lead( lead_half ) ) {
                _seekFwd( 1 ); // if so, then advance 1 more code point
            }
        }
    }
    //--------------------------------------------------------------------------
    void UTFString::_base_iterator::_movePrev()
    {
        _seekRev( 1 ); // move 1 code point backwards
        if ( _test_begin() ) return; // exit if we hit the beginning
        if ( _utf16_surrogate_follow( mIter[0] ) ) {
            // landing on a follow code point means we might be part of a bigger character
            // so we test for that
            code_point lead_half = 0;
            lead_half = mIter[-1]; // check the previous character to see if we're part of a surrogate pair
            if ( _utf16_surrogate_lead( lead_half ) ) {
                _seekRev( 1 ); // if so, then rewind 1 more code point
            }
        }
    }
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    UTFString::_fwd_iterator::_fwd_iterator()
    {

    }
    //--------------------------------------------------------------------------
    UTFString::_fwd_iterator::_fwd_iterator( const _fwd_iterator& i )
    {
        _become( i );
    }
    //--------------------------------------------------------------------------
    UTFString::_fwd_iterator& UTFString::_fwd_iterator::operator=(const _fwd_iterator& rhs)
    {
        _become(rhs);
        return *this;
    }
    //--------------------------------------------------------------------------
    UTFString::_fwd_iterator& UTFString::_fwd_iterator::operator++()
    {
        _seekFwd( 1 );
        return *this;
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString::_fwd_iterator UTFString::_fwd_iterator::operator++( int )
    {
        _fwd_iterator tmp( *this );
        _seekFwd( 1 );
        return tmp;
    }
    //--------------------------------------------------------------------------
    UTFString::_fwd_iterator& UTFString::_fwd_iterator::operator--()
    {
        _seekRev( 1 );
        return *this;
    }
    //--------------------------------------------------------------------------
    UTFString::_fwd_iterator UTFString::_fwd_iterator::operator--( int )
    {
        _fwd_iterator tmp( *this );
        _seekRev( 1 );
        return tmp;
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString::_fwd_iterator UTFString::_fwd_iterator::operator+( difference_type n )
    {
        _fwd_iterator tmp( *this );
        if ( n < 0 )
            tmp._seekRev( -n );
        else
            tmp._seekFwd( n );
        return tmp;
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString::_fwd_iterator UTFString::_fwd_iterator::operator-( difference_type n )
    {
        _fwd_iterator tmp( *this );
        if ( n < 0 )
            tmp._seekFwd( -n );
        else
            tmp._seekRev( n );
        return tmp;
    }
    //--------------------------------------------------------------------------
    UTFString::_fwd_iterator& UTFString::_fwd_iterator::operator+=( difference_type n )
    {
        if ( n < 0 )
            _seekRev( -n );
        else
            _seekFwd( n );
        return *this;
    }
    //--------------------------------------------------------------------------
    UTFString::_fwd_iterator& UTFString::_fwd_iterator::operator-=( difference_type n )
    {
        if ( n < 0 )
            _seekFwd( -n );
        else
            _seekRev( n );
        return *this;
    }
    //--------------------------------------------------------------------------
    UTFString::value_type& UTFString::_fwd_iterator::operator*() const
    {
        return *mIter;
    }
    //--------------------------------------------------------------------------
    UTFString::value_type& UTFString::_fwd_iterator::operator[]( difference_type n ) const
    {
        _fwd_iterator tmp( *this );
        tmp += n;
        return *tmp;
    }
    //--------------------------------------------------------------------------
    UTFString::_fwd_iterator& UTFString::_fwd_iterator::moveNext()
    {
        _moveNext();
        return *this;
    }
    //--------------------------------------------------------------------------
    UTFString::_fwd_iterator& UTFString::_fwd_iterator::movePrev()
    {
        _movePrev();
        return *this;
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString::unicode_char UTFString::_fwd_iterator::getCharacter() const
    {
        return _getCharacter();
    }
    //--------------------------------------------------------------------------
    int UTFString::_fwd_iterator::setCharacter( unicode_char uc )
    {
        return _setCharacter( uc );
    }
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    UTFString::_const_fwd_iterator::_const_fwd_iterator()
    {

    }
    //--------------------------------------------------------------------------
    UTFString::_const_fwd_iterator::_const_fwd_iterator( const _const_fwd_iterator& i )
    {
        _become( i );
    }
    //--------------------------------------------------------------------------
    UTFString::_const_fwd_iterator::_const_fwd_iterator( const _fwd_iterator& i )
    {
        _become( i );
    }
    //--------------------------------------------------------------------------
    UTFString::_const_fwd_iterator& UTFString::_const_fwd_iterator::operator=(const _const_fwd_iterator& rhs)
    {
        _become(rhs);
        return *this;
    }
    //--------------------------------------------------------------------------
    UTFString::_const_fwd_iterator& UTFString::_const_fwd_iterator::operator++()
    {
        _seekFwd( 1 );
        return *this;
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString::_const_fwd_iterator UTFString::_const_fwd_iterator::operator++( int )
    {
        _const_fwd_iterator tmp( *this );
        _seekFwd( 1 );
        return tmp;
    }
    //--------------------------------------------------------------------------
    UTFString::_const_fwd_iterator& UTFString::_const_fwd_iterator::operator--()
    {
        _seekRev( 1 );
        return *this;
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString::_const_fwd_iterator UTFString::_const_fwd_iterator::operator--( int )
    {
        _const_fwd_iterator tmp( *this );
        _seekRev( 1 );
        return tmp;
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString::_const_fwd_iterator UTFString::_const_fwd_iterator::operator+( difference_type n )
    {
        _const_fwd_iterator tmp( *this );
        if ( n < 0 )
            tmp._seekRev( -n );
        else
            tmp._seekFwd( n );
        return tmp;
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString::_const_fwd_iterator UTFString::_const_fwd_iterator::operator-( difference_type n )
    {
        _const_fwd_iterator tmp( *this );
        if ( n < 0 )
            tmp._seekFwd( -n );
        else
            tmp._seekRev( n );
        return tmp;
    }
    //--------------------------------------------------------------------------
    UTFString::_const_fwd_iterator& UTFString::_const_fwd_iterator::operator+=( difference_type n )
    {
        if ( n < 0 )
            _seekRev( -n );
        else
            _seekFwd( n );
        return *this;
    }
    //--------------------------------------------------------------------------
    UTFString::_const_fwd_iterator& UTFString::_const_fwd_iterator::operator-=( difference_type n )
    {
        if ( n < 0 )
            _seekFwd( -n );
        else
            _seekRev( n );
        return *this;
    }
    //--------------------------------------------------------------------------
    const UTFString::value_type& UTFString::_const_fwd_iterator::operator*() const
    {
        return *mIter;
    }
    //--------------------------------------------------------------------------
    const UTFString::value_type& UTFString::_const_fwd_iterator::operator[]( difference_type n ) const
    {
        _const_fwd_iterator tmp( *this );
        tmp += n;
        return *tmp;
    }
    //--------------------------------------------------------------------------
    UTFString::_const_fwd_iterator& UTFString::_const_fwd_iterator::moveNext()
    {
        _moveNext();
        return *this;
    }
    //--------------------------------------------------------------------------
    UTFString::_const_fwd_iterator& UTFString::_const_fwd_iterator::movePrev()
    {
        _movePrev();
        return *this;
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString::unicode_char UTFString::_const_fwd_iterator::getCharacter() const
    {
        return _getCharacter();
    }
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    UTFString::_rev_iterator::_rev_iterator()
    {

    }
    //--------------------------------------------------------------------------
    UTFString::_rev_iterator::_rev_iterator( const _rev_iterator& i )
    {
        _become( i );
    }
    //--------------------------------------------------------------------------
    UTFString::_rev_iterator& UTFString::_rev_iterator::operator=(const _rev_iterator& rhs)
    {
        _become(rhs);
        return *this;
    }
    //--------------------------------------------------------------------------
    UTFString::_rev_iterator& UTFString::_rev_iterator::operator++()
    {
        _seekRev( 1 );
        return *this;
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString::_rev_iterator UTFString::_rev_iterator::operator++( int )
    {
        _rev_iterator tmp( *this );
        _seekRev( 1 );
        return tmp;
    }
    //--------------------------------------------------------------------------
    UTFString::_rev_iterator& UTFString::_rev_iterator::operator--()
    {
        _seekFwd( 1 );
        return *this;
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString::_rev_iterator UTFString::_rev_iterator::operator--( int )
    {
        _rev_iterator tmp( *this );
        _seekFwd( 1 );
        return tmp;
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString::_rev_iterator UTFString::_rev_iterator::operator+( difference_type n )
    {
        _rev_iterator tmp( *this );
        if ( n < 0 )
            tmp._seekFwd( -n );
        else
            tmp._seekRev( n );
        return tmp;
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString::_rev_iterator UTFString::_rev_iterator::operator-( difference_type n )
    {
        _rev_iterator tmp( *this );
        if ( n < 0 )
            tmp._seekRev( -n );
        else
            tmp._seekFwd( n );
        return tmp;
    }
    //--------------------------------------------------------------------------
    UTFString::_rev_iterator& UTFString::_rev_iterator::operator+=( difference_type n )
    {
        if ( n < 0 )
            _seekFwd( -n );
        else
            _seekRev( n );
        return *this;
    }
    //--------------------------------------------------------------------------
    UTFString::_rev_iterator& UTFString::_rev_iterator::operator-=( difference_type n )
    {
        if ( n < 0 )
            _seekRev( -n );
        else
            _seekFwd( n );
        return *this;
    }
    //--------------------------------------------------------------------------
    UTFString::value_type& UTFString::_rev_iterator::operator*() const
    {
        return mIter[-1];
    }
    //--------------------------------------------------------------------------
    UTFString::value_type& UTFString::_rev_iterator::operator[]( difference_type n ) const
    {
        _rev_iterator tmp( *this );
        tmp -= n;
        return *tmp;
    }
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    UTFString::_const_rev_iterator::_const_rev_iterator()
    {

    }
    //--------------------------------------------------------------------------
    UTFString::_const_rev_iterator::_const_rev_iterator( const _const_rev_iterator& i )
    {
        _become( i );
    }
    //--------------------------------------------------------------------------
    UTFString::_const_rev_iterator::_const_rev_iterator( const _rev_iterator& i )
    {
        _become( i );
    }
    //--------------------------------------------------------------------------
    UTFString::_const_rev_iterator& UTFString::_const_rev_iterator::operator=(const _const_rev_iterator& rhs)
    {
        _become(rhs);
        return *this;
    }
    //--------------------------------------------------------------------------
    UTFString::_const_rev_iterator& UTFString::_const_rev_iterator::operator++()
    {
        _seekRev( 1 );
        return *this;
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString::_const_rev_iterator UTFString::_const_rev_iterator::operator++( int )
    {
        _const_rev_iterator tmp( *this );
        _seekRev( 1 );
        return tmp;
    }
    //--------------------------------------------------------------------------
    UTFString::_const_rev_iterator& UTFString::_const_rev_iterator::operator--()
    {
        _seekFwd( 1 );
        return *this;
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString::_const_rev_iterator UTFString::_const_rev_iterator::operator--( int )
    {
        _const_rev_iterator tmp( *this );
        _seekFwd( 1 );
        return tmp;
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString::_const_rev_iterator UTFString::_const_rev_iterator::operator+( difference_type n )
    {
        _const_rev_iterator tmp( *this );
        if ( n < 0 )
            tmp._seekFwd( -n );
        else
            tmp._seekRev( n );
        return tmp;
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString::_const_rev_iterator UTFString::_const_rev_iterator::operator-( difference_type n )
    {
        _const_rev_iterator tmp( *this );
        if ( n < 0 )
            tmp._seekRev( -n );
        else
            tmp._seekFwd( n );
        return tmp;
    }
    //--------------------------------------------------------------------------
    UTFString::_const_rev_iterator& UTFString::_const_rev_iterator::operator+=( difference_type n )
    {
        if ( n < 0 )
            _seekFwd( -n );
        else
            _seekRev( n );
        return *this;
    }
    //--------------------------------------------------------------------------
    UTFString::_const_rev_iterator& UTFString::_const_rev_iterator::operator-=( difference_type n )
    {
        if ( n < 0 )
            _seekRev( -n );
        else
            _seekFwd( n );
        return *this;
    }
    //--------------------------------------------------------------------------
    const UTFString::value_type& UTFString::_const_rev_iterator::operator*() const
    {
        return mIter[-1];
    }
    //--------------------------------------------------------------------------
    const UTFString::value_type& UTFString::_const_rev_iterator::operator[]( difference_type n ) const
    {
        _const_rev_iterator tmp( *this );
        tmp -= n;
        return *tmp;
    }
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    UTFString::UTFString()
    {
        _init();
    }
    //--------------------------------------------------------------------------
    UTFString::UTFString( const UTFString& copy )
    {
        _init();
        mData = copy.mData;
    }
    //--------------------------------------------------------------------------
    UTFString::UTFString( size_type inLength, const code_point& ch )
    {
        _init();
        assign( inLength, ch );
    }
    //--------------------------------------------------------------------------
    UTFString::UTFString( const code_point* str )
    {
        _init();
        assign( str );
    }
    //--------------------------------------------------------------------------
    UTFString::UTFString( const code_point* str, size_type inLength )
    {
        _init();
        assign( str, inLength );
    }
    //--------------------------------------------------------------------------
    UTFString::UTFString( const UTFString& str, size_type index, size_type inLength )
    {
        _init();
        assign( str, index, inLength );
    }
    //--------------------------------------------------------------------------
#if OGRE_IS_NATIVE_WCHAR_T
    UTFString::UTFString( const wchar_t* w_str )
    {
        _init();
        assign( w_str );
    }
    //--------------------------------------------------------------------------
    UTFString::UTFString( const wchar_t* w_str, size_type inLength )
    {
        _init();
        assign( w_str, inLength );
    }
#endif
    //--------------------------------------------------------------------------
    UTFString::UTFString( const std::wstring& wstr )
    {
        _init();
        assign( wstr );
    }
    //--------------------------------------------------------------------------
    UTFString::UTFString( const char* str )
    {
        _init();
        assign( str );
    }
    //--------------------------------------------------------------------------
    UTFString::UTFString( const char* str, size_type inLength )
    {
        _init();
        assign( str, inLength );
    }
    //--------------------------------------------------------------------------
    UTFString::UTFString( const std::string& str )
    {
        _init();
        assign( str );
    }
    //--------------------------------------------------------------------------
    UTFString::~UTFString()
    {
        _cleanBuffer();
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString::size_type UTFString::size() const
    {
        return mData.size();
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString::size_type UTFString::length() const
    {
        return size();
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString::size_type UTFString::length_Characters() const
    {
        const_iterator i = begin(), ie = end();
        size_type c = 0;
        while ( i != ie ) {
            i.moveNext();
            ++c;
        }
        return c;
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString::size_type UTFString::max_size() const
    {
        return mData.max_size();
    }
    //--------------------------------------------------------------------------
    void UTFString::reserve( size_type inSize )
    {
        mData.reserve( inSize );
    }
    //--------------------------------------------------------------------------
    void UTFString::resize( size_type num, const code_point& val /*= 0 */ )
    {
        mData.resize( num, val );
    }
    //--------------------------------------------------------------------------
    void UTFString::swap( UTFString& from )
    {
        mData.swap( from.mData );
    }
    //--------------------------------------------------------------------------
    bool UTFString::empty() const
    {
        return mData.empty();
    }
    //--------------------------------------------------------------------------
    const UTFString::code_point* UTFString::c_str() const
    {
        return mData.c_str();
    }
    //--------------------------------------------------------------------------
    const UTFString::code_point* UTFString::data() const
    {
        return c_str();
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString::size_type UTFString::capacity() const
    {
        return mData.capacity();
    }
    //--------------------------------------------------------------------------
    void UTFString::clear()
    {
        mData.clear();
    }
    //--------------------------------------------------------------------------
    Ogre::UTFString UTFString::substr( size_type index, size_type num /*= npos */ ) const
    {
        // this could avoid the extra copy if we used a private specialty constructor
        dstring tmpData = mData.substr( index, num );
        UTFString tmp;
        tmp.mData.swap( tmpData );
        return tmp;
    }
    //--------------------------------------------------------------------------
    void UTFString::push_back( unicode_char val )
    {
        code_point cp[2] = {
            0, 0
        };
        size_t c = _utf32_to_utf16( val, cp );
        if ( c > 0 ) push_back( cp[0] );
        if ( c > 1 ) push_back( cp[1] );
    }
    //--------------------------------------------------------------------------
#if OGRE_IS_NATIVE_WCHAR_T
    void UTFString::push_back( wchar_t val )
    {
        // we do this because the Unicode method still preserves UTF-16 code points
        mData.push_back( static_cast<code_point>( val ) );
    }
#endif
    //--------------------------------------------------------------------------
    void UTFString::push_back( code_point val )
    {
        mData.push_back( val );
    }

    void UTFString::push_back( char val )
    {
        mData.push_back( static_cast<code_point>( val ) );
    }

    bool UTFString::inString( unicode_char ch ) const
    {
        const_iterator i, ie = end();
        for ( i = begin(); i != ie; i.moveNext() ) {
            if ( i.getCharacter() == ch )
                return true;
        }
        return false;
    }

    const std::string& UTFString::asUTF8() const
    {
        _load_buffer_UTF8();
        return *mBuffer.mStrBuffer;
    }

    const char* UTFString::asUTF8_c_str() const
    {
        _load_buffer_UTF8();
        return mBuffer.mStrBuffer->c_str();
    }

    const UTFString::utf32string& UTFString::asUTF32() const
    {
        _load_buffer_UTF32();
        return *mBuffer.mUTF32StrBuffer;
    }

    const UTFString::unicode_char* UTFString::asUTF32_c_str() const
    {
        _load_buffer_UTF32();
        return mBuffer.mUTF32StrBuffer->c_str();
    }

    const std::wstring& UTFString::asWStr() const
    {
        _load_buffer_WStr();
        return *mBuffer.mWStrBuffer;
    }

    const wchar_t* UTFString::asWStr_c_str() const
    {
        _load_buffer_WStr();
        return mBuffer.mWStrBuffer->c_str();
    }

    UTFString::code_point& UTFString::at( size_type loc )
    {
        return mData.at( loc );
    }

    const UTFString::code_point& UTFString::at( size_type loc ) const
    {
        return mData.at( loc );
    }

    Ogre::UTFString::unicode_char UTFString::getChar( size_type loc ) const
    {
        const code_point* ptr = c_str();
        unicode_char uc;
        size_t l = _utf16_char_length( ptr[loc] );
        code_point cp[2] = { /* blame the code beautifier */
            0, 0
        };
        cp[0] = ptr[loc];

        if ( l == 2 && ( loc + 1 ) < mData.length() ) {
            cp[1] = ptr[loc+1];
        }
        _utf16_to_utf32( cp, uc );
        return uc;
    }

    int UTFString::setChar( size_type loc, unicode_char ch )
    {
        code_point cp[2] = { /* blame the code beautifier */
            0, 0
        };
        size_t l = _utf32_to_utf16( ch, cp );
        unicode_char existingChar = getChar( loc );
        size_t existingSize = _utf16_char_length( existingChar );
        size_t newSize = _utf16_char_length( ch );

        if ( newSize > existingSize ) {
            at( loc ) = cp[0];
            insert( loc + 1, 1, cp[1] );
            return 1;
        }
        if ( newSize < existingSize ) {
            erase( loc, 1 );
            at( loc ) = cp[0];
            return -1;
        }

        // newSize == existingSize
        at( loc ) = cp[0];
        if ( l == 2 ) at( loc + 1 ) = cp[1];
        return 0;
    }

    Ogre::UTFString::iterator UTFString::begin()
    {
        iterator i;
        i.mIter = mData.begin();
        i.mString = this;
        return i;
    }

    Ogre::UTFString::const_iterator UTFString::begin() const
    {
        const_iterator i;
        i.mIter = const_cast<UTFString*>( this )->mData.begin();
        i.mString = const_cast<UTFString*>( this );
        return i;
    }

    Ogre::UTFString::iterator UTFString::end()
    {
        iterator i;
        i.mIter = mData.end();
        i.mString = this;
        return i;
    }

    Ogre::UTFString::const_iterator UTFString::end() const
    {
        const_iterator i;
        i.mIter = const_cast<UTFString*>( this )->mData.end();
        i.mString = const_cast<UTFString*>( this );
        return i;
    }

    Ogre::UTFString::reverse_iterator UTFString::rbegin()
    {
        reverse_iterator i;
        i.mIter = mData.end();
        i.mString = this;
        return i;
    }

    Ogre::UTFString::const_reverse_iterator UTFString::rbegin() const
    {
        const_reverse_iterator i;
        i.mIter = const_cast<UTFString*>( this )->mData.end();
        i.mString = const_cast<UTFString*>( this );
        return i;
    }

    Ogre::UTFString::reverse_iterator UTFString::rend()
    {
        reverse_iterator i;
        i.mIter = mData.begin();
        i.mString = this;
        return i;
    }

    Ogre::UTFString::const_reverse_iterator UTFString::rend() const
    {
        const_reverse_iterator i;
        i.mIter = const_cast<UTFString*>( this )->mData.begin();
        i.mString = const_cast<UTFString*>( this );
        return i;
    }

    UTFString& UTFString::assign( iterator start, iterator last )
    {
        mData.assign( start.mIter, last.mIter );
        return *this;
    }

    UTFString& UTFString::assign( const UTFString& str )
    {
        mData.assign( str.mData );
        return *this;
    }

    UTFString& UTFString::assign( const code_point* str )
    {
        mData.assign( str );
        return *this;
    }

    UTFString& UTFString::assign( const code_point* str, size_type num )
    {
        mData.assign( str, num );
        return *this;
    }

    UTFString& UTFString::assign( const UTFString& str, size_type index, size_type len )
    {
        mData.assign( str.mData, index, len );
        return *this;
    }

    UTFString& UTFString::assign( size_type num, const code_point& ch )
    {
        mData.assign( num, ch );
        return *this;
    }

    UTFString& UTFString::assign( const std::wstring& wstr )
    {
        mData.clear();
        mData.reserve( wstr.length() ); // best guess bulk allocate
#ifdef WCHAR_UTF16 // if we're already working in UTF-16, this is easy
        code_point tmp;
        std::wstring::const_iterator i, ie = wstr.end();
        for ( i = wstr.begin(); i != ie; i++ ) {
            tmp = static_cast<code_point>( *i );
            mData.push_back( tmp );
        }
#else // otherwise we do it the safe way (which is still 100% safe to pass UTF-16 through, just slower)
        code_point cp[3] = {0, 0, 0};
        unicode_char tmp;
        std::wstring::const_iterator i, ie = wstr.end();
        for ( i = wstr.begin(); i != ie; i++ ) {
            tmp = static_cast<unicode_char>( *i );
            size_t l = _utf32_to_utf16( tmp, cp );
            if ( l > 0 ) mData.push_back( cp[0] );
            if ( l > 1 ) mData.push_back( cp[1] );
        }
#endif
        return *this;
    }

#if OGRE_IS_NATIVE_WCHAR_T
    UTFString& UTFString::assign( const wchar_t* w_str )
    {
        std::wstring tmp;
        tmp.assign( w_str );
        return assign( tmp );
    }

    UTFString& UTFString::assign( const wchar_t* w_str, size_type num )
    {
        std::wstring tmp;
        tmp.assign( w_str, num );
        return assign( tmp );
    }
#endif

    UTFString& UTFString::assign( const std::string& str )
    {
        size_type len = _verifyUTF8( str );
        clear(); // empty our contents, if there are any
        reserve( len ); // best guess bulk capacity growth

        // This is a 3 step process, converting each byte in the UTF-8 stream to UTF-32,
        // then converting it to UTF-16, then finally appending the data buffer

        unicode_char uc = 0;      // temporary Unicode character buffer
        unsigned char utf8buf[7]; // temporary UTF-8 buffer
        utf8buf[6] = 0;
        size_t utf8len;           // UTF-8 length
        code_point utf16buff[3];  // temporary UTF-16 buffer
        utf16buff[2] = 0;
        size_t utf16len;          // UTF-16 length

        std::string::const_iterator i, ie = str.end();
        for ( i = str.begin(); i != ie; i++ ) {
            utf8len = _utf8_char_length( static_cast<unsigned char>( *i ) ); // estimate bytes to load
            for ( size_t j = 0; j < utf8len; j++ ) { // load the needed UTF-8 bytes
                utf8buf[j] = ( static_cast<unsigned char>( *( i + j ) ) ); // we don't increment 'i' here just in case the estimate is wrong (shouldn't happen, but we're being careful)
            }
            utf8buf[utf8len] = 0; // nul terminate so we throw an exception before running off the end of the buffer
            utf8len = _utf8_to_utf32( utf8buf, uc ); // do the UTF-8 -> UTF-32 conversion
            i += utf8len - 1; // we subtract 1 for the increment of the 'for' loop

            utf16len = _utf32_to_utf16( uc, utf16buff ); // UTF-32 -> UTF-16 conversion
            append( utf16buff, utf16len ); // append the characters to the string
        }
        return *this;
    }

    UTFString& UTFString::assign( const char* str )
    {
        std::string tmp( str );
        return assign( tmp );
    }

    UTFString& UTFString::assign( const char* str, size_type num )
    {
        std::string tmp;
        tmp.assign( str, num );
        return assign( tmp );
    }

    UTFString& UTFString::append( const UTFString& str )
    {
        mData.append( str.mData );
        return *this;
    }

    UTFString& UTFString::append( const code_point* str )
    {
        mData.append( str );
        return *this;
    }

    UTFString& UTFString::append( const UTFString& str, size_type index, size_type len )
    {
        mData.append( str.mData, index, len );
        return *this;
    }

    UTFString& UTFString::append( const code_point* str, size_type num )
    {
        mData.append( str, num );
        return *this;
    }

    UTFString& UTFString::append( size_type num, code_point ch )
    {
        mData.append( num, ch );
        return *this;
    }

    UTFString& UTFString::append( iterator start, iterator last )
    {
        mData.append( start.mIter, last.mIter );
        return *this;
    }

#if OGRE_IS_NATIVE_WCHAR_T
    UTFString& UTFString::append( const wchar_t* w_str, size_type num )
    {
        std::wstring tmp( w_str, num );
        return append( tmp );
    }

    UTFString& UTFString::append( size_type num, wchar_t ch )
    {
        return append( num, static_cast<unicode_char>( ch ) );
    }
#endif
    UTFString& UTFString::append( const char* str, size_type num )
    {
        UTFString tmp( str, num );
        append( tmp );
        return *this;
    }

    UTFString& UTFString::append( size_type num, char ch )
    {
        append( num, static_cast<code_point>( ch ) );
        return *this;
    }

    UTFString& UTFString::append( size_type num, unicode_char ch )
    {
        code_point cp[2] = {0, 0};
        if ( _utf32_to_utf16( ch, cp ) == 2 ) {
            for ( size_type i = 0; i < num; i++ ) {
                append( 1, cp[0] );
                append( 1, cp[1] );
            }
        } else {
            for ( size_type i = 0; i < num; i++ ) {
                append( 1, cp[0] );
            }
        }
        return *this;
    }

    Ogre::UTFString::iterator UTFString::insert( iterator i, const code_point& ch )
    {
        iterator ret;
        ret.mIter = mData.insert( i.mIter, ch );
        ret.mString = this;
        return ret;
    }

    UTFString& UTFString::insert( size_type index, const UTFString& str )
    {
        mData.insert( index, str.mData );
        return *this;
    }

    UTFString& UTFString::insert( size_type index1, const UTFString& str, size_type index2, size_type num )
    {
        mData.insert( index1, str.mData, index2, num );
        return *this;
    }

    void UTFString::insert( iterator i, iterator start, iterator last )
    {
        mData.insert( i.mIter, start.mIter, last.mIter );
    }

    UTFString& UTFString::insert( size_type index, const code_point* str, size_type num )
    {
        mData.insert( index, str, num );
        return *this;
    }

#if OGRE_IS_NATIVE_WCHAR_T
    UTFString& UTFString::insert( size_type index, const wchar_t* w_str, size_type num )
    {
        UTFString tmp( w_str, num );
        insert( index, tmp );
        return *this;
    }
#endif

    UTFString& UTFString::insert( size_type index, const char* str, size_type num )
    {
        UTFString tmp( str, num );
        insert( index, tmp );
        return *this;
    }

    UTFString& UTFString::insert( size_type index, size_type num, code_point ch )
    {
        mData.insert( index, num, ch );
        return *this;
    }

#if OGRE_IS_NATIVE_WCHAR_T
    UTFString& UTFString::insert( size_type index, size_type num, wchar_t ch )
    {
        insert( index, num, static_cast<unicode_char>( ch ) );
        return *this;
    }
#endif

    UTFString& UTFString::insert( size_type index, size_type num, char ch )
    {
        insert( index, num, static_cast<code_point>( ch ) );
        return *this;
    }

    UTFString& UTFString::insert( size_type index, size_type num, unicode_char ch )
    {
        code_point cp[3] = {0, 0, 0};
        size_t l = _utf32_to_utf16( ch, cp );
        if ( l == 1 ) {
            return insert( index, num, cp[0] );
        }
        for ( size_type c = 0; c < num; c++ ) {
            // insert in reverse order to preserve ordering after insert
            insert( index, 1, cp[1] );
            insert( index, 1, cp[0] );
        }
        return *this;
    }

    void UTFString::insert( iterator i, size_type num, const code_point& ch )
    {
        mData.insert( i.mIter, num, ch );
    }
#if OGRE_IS_NATIVE_WCHAR_T
    void UTFString::insert( iterator i, size_type num, const wchar_t& ch )
    {
        insert( i, num, static_cast<unicode_char>( ch ) );
    }
#endif

    void UTFString::insert( iterator i, size_type num, const char& ch )
    {
        insert( i, num, static_cast<code_point>( ch ) );
    }

    void UTFString::insert( iterator i, size_type num, const unicode_char& ch )
    {
        code_point cp[3] = {0, 0, 0};
        size_t l = _utf32_to_utf16( ch, cp );
        if ( l == 1 ) {
            insert( i, num, cp[0] );
        } else {
            for ( size_type c = 0; c < num; c++ ) {
                // insert in reverse order to preserve ordering after insert
                insert( i, 1, cp[1] );
                insert( i, 1, cp[0] );
            }
        }
    }

    Ogre::UTFString::iterator UTFString::erase( iterator loc )
    {
        iterator ret;
        ret.mIter = mData.erase( loc.mIter );
        ret.mString = this;
        return ret;
    }

    Ogre::UTFString::iterator UTFString::erase( iterator start, iterator last )
    {
        iterator ret;
        ret.mIter = mData.erase( start.mIter, last.mIter );
        ret.mString = this;
        return ret;
    }

    UTFString& UTFString::erase( size_type index /*= 0*/, size_type num /*= npos */ )
    {
        if ( num == npos )
            mData.erase( index );
        else
            mData.erase( index, num );
        return *this;
    }

    UTFString& UTFString::replace( size_type index1, size_type num1, const UTFString& str )
    {
        mData.replace( index1, num1, str.mData, 0, npos );
        return *this;
    }

    UTFString& UTFString::replace( size_type index1, size_type num1, const UTFString& str, size_type num2 )
    {
        mData.replace( index1, num1, str.mData, 0, num2 );
        return *this;
    }

    UTFString& UTFString::replace( size_type index1, size_type num1, const UTFString& str, size_type index2, size_type num2 )
    {
        mData.replace( index1, num1, str.mData, index2, num2 );
        return *this;
    }

    UTFString& UTFString::replace( iterator start, iterator last, const UTFString& str, size_type num /*= npos */ )
    {
        _const_fwd_iterator st(start); //Work around for gcc, allow it to find correct overload

        size_type index1 = begin() - st;
        size_type num1 = last - st;
        return replace( index1, num1, str, 0, num );
    }

    UTFString& UTFString::replace( size_type index, size_type num1, size_type num2, code_point ch )
    {
        mData.replace( index, num1, num2, ch );
        return *this;
    }

    UTFString& UTFString::replace( iterator start, iterator last, size_type num, code_point ch )
    {
        _const_fwd_iterator st(start); //Work around for gcc, allow it to find correct overload

        size_type index1 = begin() - st;
        size_type num1 = last - st;
        return replace( index1, num1, num, ch );
    }

    int UTFString::compare( const UTFString& str ) const
    {
        return mData.compare( str.mData );
    }

    int UTFString::compare( const code_point* str ) const
    {
        return mData.compare( str );
    }

    int UTFString::compare( size_type index, size_type inLength, const UTFString& str ) const
    {
        return mData.compare( index, inLength, str.mData );
    }

    int UTFString::compare( size_type index, size_type inLength, const UTFString& str, size_type index2, size_type length2 ) const
    {
        return mData.compare( index, inLength, str.mData, index2, length2 );
    }

    int UTFString::compare( size_type index, size_type inLength, const code_point* str, size_type length2 ) const
    {
        return mData.compare( index, inLength, str, length2 );
    }

#if OGRE_IS_NATIVE_WCHAR_T
    int UTFString::compare( size_type index, size_type inLength, const wchar_t* w_str, size_type length2 ) const
    {
        UTFString tmp( w_str, length2 );
        return compare( index, inLength, tmp );
    }
#endif

    int UTFString::compare( size_type index, size_type inLength, const char* str, size_type length2 ) const
    {
        UTFString tmp( str, length2 );
        return compare( index, inLength, tmp );
    }

    Ogre::UTFString::size_type UTFString::find( const UTFString& str, size_type index /*= 0 */ ) const
    {
        return mData.find( str.c_str(), index );
    }

    Ogre::UTFString::size_type UTFString::find( const code_point* cp_str, size_type index, size_type inLength ) const
    {
        UTFString tmp( cp_str );
        return mData.find( tmp.c_str(), index, inLength );
    }

    Ogre::UTFString::size_type UTFString::find( const char* str, size_type index, size_type inLength ) const
    {
        UTFString tmp( str );
        return mData.find( tmp.c_str(), index, inLength );
    }

#if OGRE_IS_NATIVE_WCHAR_T
    Ogre::UTFString::size_type UTFString::find( const wchar_t* w_str, size_type index, size_type inLength ) const
    {
        UTFString tmp( w_str );
        return mData.find( tmp.c_str(), index, inLength );
    }
#endif

    Ogre::UTFString::size_type UTFString::find( char ch, size_type index /*= 0 */ ) const
    {
        return find( static_cast<code_point>( ch ), index );
    }

    Ogre::UTFString::size_type UTFString::find( code_point ch, size_type index /*= 0 */ ) const
    {
        return mData.find( ch, index );
    }

#if OGRE_IS_NATIVE_WCHAR_T
    Ogre::UTFString::size_type UTFString::find( wchar_t ch, size_type index /*= 0 */ ) const
    {
        return find( static_cast<unicode_char>( ch ), index );
    }
#endif

    Ogre::UTFString::size_type UTFString::find( unicode_char ch, size_type index /*= 0 */ ) const
    {
        code_point cp[3] = {0, 0, 0};
        size_t l = _utf32_to_utf16( ch, cp );
        return find( UTFString( cp, l ), index );
    }

    Ogre::UTFString::size_type UTFString::rfind( const UTFString& str, size_type index /*= 0 */ ) const
    {
        return mData.rfind( str.c_str(), index );
    }

    Ogre::UTFString::size_type UTFString::rfind( const code_point* cp_str, size_type index, size_type num ) const
    {
        UTFString tmp( cp_str );
        return mData.rfind( tmp.c_str(), index, num );
    }

    Ogre::UTFString::size_type UTFString::rfind( const char* str, size_type index, size_type num ) const
    {
        UTFString tmp( str );
        return mData.rfind( tmp.c_str(), index, num );
    }

#if OGRE_IS_NATIVE_WCHAR_T
    Ogre::UTFString::size_type UTFString::rfind( const wchar_t* w_str, size_type index, size_type num ) const
    {
        UTFString tmp( w_str );
        return mData.rfind( tmp.c_str(), index, num );
    }
#endif

    Ogre::UTFString::size_type UTFString::rfind( char ch, size_type index /*= 0 */ ) const
    {
        return rfind( static_cast<code_point>( ch ), index );
    }

    Ogre::UTFString::size_type UTFString::rfind( code_point ch, size_type index ) const
    {
        return mData.rfind( ch, index );
    }

#if OGRE_IS_NATIVE_WCHAR_T
    Ogre::UTFString::size_type UTFString::rfind( wchar_t ch, size_type index /*= 0 */ ) const
    {
        return rfind( static_cast<unicode_char>( ch ), index );
    }
#endif

    Ogre::UTFString::size_type UTFString::rfind( unicode_char ch, size_type index /*= 0 */ ) const
    {
        code_point cp[3] = {0, 0, 0};
        size_t l = _utf32_to_utf16( ch, cp );
        return rfind( UTFString( cp, l ), index );
    }

    Ogre::UTFString::size_type UTFString::find_first_of( const UTFString &str, size_type index /*= 0*/, size_type num /*= npos */ ) const
    {
        size_type i = 0;
        const size_type len = length();
        while ( i < num && ( index + i ) < len ) {
            unicode_char ch = getChar( index + i );
            if ( str.inString( ch ) )
                return index + i;
            i += _utf16_char_length( ch ); // increment by the Unicode character length
        }
        return npos;
    }

    Ogre::UTFString::size_type UTFString::find_first_of( code_point ch, size_type index /*= 0 */ ) const
    {
        UTFString tmp;
        tmp.assign( 1, ch );
        return find_first_of( tmp, index );
    }

    Ogre::UTFString::size_type UTFString::find_first_of( char ch, size_type index /*= 0 */ ) const
    {
        return find_first_of( static_cast<code_point>( ch ), index );
    }

#if OGRE_IS_NATIVE_WCHAR_T
    Ogre::UTFString::size_type UTFString::find_first_of( wchar_t ch, size_type index /*= 0 */ ) const
    {
        return find_first_of( static_cast<unicode_char>( ch ), index );
    }
#endif

    Ogre::UTFString::size_type UTFString::find_first_of( unicode_char ch, size_type index /*= 0 */ ) const
    {
        code_point cp[3] = {0, 0, 0};
        size_t l = _utf32_to_utf16( ch, cp );
        return find_first_of( UTFString( cp, l ), index );
    }

    Ogre::UTFString::size_type UTFString::find_first_not_of( const UTFString& str, size_type index /*= 0*/, size_type num /*= npos */ ) const
    {
        size_type i = 0;
        const size_type len = length();
        while ( i < num && ( index + i ) < len ) {
            unicode_char ch = getChar( index + i );
            if ( !str.inString( ch ) )
                return index + i;
            i += _utf16_char_length( ch ); // increment by the Unicode character length
        }
        return npos;
    }

    Ogre::UTFString::size_type UTFString::find_first_not_of( code_point ch, size_type index /*= 0 */ ) const
    {
        UTFString tmp;
        tmp.assign( 1, ch );
        return find_first_not_of( tmp, index );
    }

    Ogre::UTFString::size_type UTFString::find_first_not_of( char ch, size_type index /*= 0 */ ) const
    {
        return find_first_not_of( static_cast<code_point>( ch ), index );
    }

#if OGRE_IS_NATIVE_WCHAR_T
    Ogre::UTFString::size_type UTFString::find_first_not_of( wchar_t ch, size_type index /*= 0 */ ) const
    {
        return find_first_not_of( static_cast<unicode_char>( ch ), index );
    }
#endif

    Ogre::UTFString::size_type UTFString::find_first_not_of( unicode_char ch, size_type index /*= 0 */ ) const
    {
        code_point cp[3] = {0, 0, 0};
        size_t l = _utf32_to_utf16( ch, cp );
        return find_first_not_of( UTFString( cp, l ), index );
    }

    Ogre::UTFString::size_type UTFString::find_last_of( const UTFString& str, size_type index /*= npos*/, size_type num /*= npos */ ) const
    {
        size_type i = 0;
        const size_type len = length();
        if ( index > len ) index = len - 1;

        while ( i < num && ( index - i ) != npos ) {
            size_type j = index - i;
            // careful to step full Unicode characters
            if ( j != 0 && _utf16_surrogate_follow( at( j ) ) && _utf16_surrogate_lead( at( j - 1 ) ) ) {
                j = index - ++i;
            }
            // and back to the usual dull test
            unicode_char ch = getChar( j );
            if ( str.inString( ch ) )
                return j;
            i++;
        }
        return npos;
    }

    Ogre::UTFString::size_type UTFString::find_last_of( code_point ch, size_type index /*= npos */ ) const
    {
        UTFString tmp;
        tmp.assign( 1, ch );
        return find_last_of( tmp, index );
    }

#if OGRE_IS_NATIVE_WCHAR_T
    Ogre::UTFString::size_type UTFString::find_last_of( wchar_t ch, size_type index /*= npos */ ) const
    {
        return find_last_of( static_cast<unicode_char>( ch ), index );
    }
#endif

    Ogre::UTFString::size_type UTFString::find_last_of( unicode_char ch, size_type index /*= npos */ ) const
    {
        code_point cp[3] = {0, 0, 0};
        size_t l = _utf32_to_utf16( ch, cp );
        return find_last_of( UTFString( cp, l ), index );
    }

    Ogre::UTFString::size_type UTFString::find_last_not_of( const UTFString& str, size_type index /*= npos*/, size_type num /*= npos */ ) const
    {
        size_type i = 0;
        const size_type len = length();
        if ( index > len ) index = len - 1;

        while ( i < num && ( index - i ) != npos ) {
            size_type j = index - i;
            // careful to step full Unicode characters
            if ( j != 0 && _utf16_surrogate_follow( at( j ) ) && _utf16_surrogate_lead( at( j - 1 ) ) ) {
                j = index - ++i;
            }
            // and back to the usual dull test
            unicode_char ch = getChar( j );
            if ( !str.inString( ch ) )
                return j;
            i++;
        }
        return npos;
    }

    Ogre::UTFString::size_type UTFString::find_last_not_of( code_point ch, size_type index /*= npos */ ) const
    {
        UTFString tmp;
        tmp.assign( 1, ch );
        return find_last_not_of( tmp, index );
    }

    Ogre::UTFString::size_type UTFString::find_last_not_of( char ch, size_type index /*= npos */ ) const
    {
        return find_last_not_of( static_cast<code_point>( ch ), index );
    }

#if OGRE_IS_NATIVE_WCHAR_T
    Ogre::UTFString::size_type UTFString::find_last_not_of( wchar_t ch, size_type index /*= npos */ ) const
    {
        return find_last_not_of( static_cast<unicode_char>( ch ), index );
    }
#endif

    Ogre::UTFString::size_type UTFString::find_last_not_of( unicode_char ch, size_type index /*= npos */ ) const
    {
        code_point cp[3] = {0, 0, 0};
        size_t l = _utf32_to_utf16( ch, cp );
        return find_last_not_of( UTFString( cp, l ), index );
    }

    bool UTFString::operator<( const UTFString& right ) const
    {
        return compare( right ) < 0;
    }

    bool UTFString::operator<=( const UTFString& right ) const
    {
        return compare( right ) <= 0;
    }

    UTFString& UTFString::operator=( const UTFString& s )
    {
        return assign( s );
    }

    UTFString& UTFString::operator=( code_point ch )
    {
        clear();
        return append( 1, ch );
    }

    UTFString& UTFString::operator=( char ch )
    {
        clear();
        return append( 1, ch );
    }

#if OGRE_IS_NATIVE_WCHAR_T
    UTFString& UTFString::operator=( wchar_t ch )
    {
        clear();
        return append( 1, ch );
    }
#endif

    UTFString& UTFString::operator=( unicode_char ch )
    {
        clear();
        return append( 1, ch );
    }

    bool UTFString::operator>( const UTFString& right ) const
    {
        return compare( right ) > 0;
    }

    bool UTFString::operator>=( const UTFString& right ) const
    {
        return compare( right ) >= 0;
    }

    bool UTFString::operator==( const UTFString& right ) const
    {
        return compare( right ) == 0;
    }

    bool UTFString::operator!=( const UTFString& right ) const
    {
        return !operator==( right );
    }

    UTFString::code_point& UTFString::operator[]( size_type index )
    {
        return at( index );
    }

    const UTFString::code_point& UTFString::operator[]( size_type index ) const
    {
        return at( index );
    }

    UTFString::operator std::string() const 
    {
        return std::string( asUTF8() );
    }
    
    //! implicit cast to std::wstring
    UTFString::operator std::wstring() const 
    {
        return std::wstring( asWStr() );
    }

    bool UTFString::_utf16_independent_char( code_point cp )
    {
        if ( 0xD800 <= cp && cp <= 0xDFFF ) // tests if the cp is within the surrogate pair range
            return false; // it matches a surrogate pair signature
        return true; // everything else is a standalone code point
    }

    bool UTFString::_utf16_surrogate_lead( code_point cp )
    {
        if ( 0xD800 <= cp && cp <= 0xDBFF ) // tests if the cp is within the 2nd word of a surrogate pair
            return true; // it is a 1st word
        return false; // it isn't
    }

    bool UTFString::_utf16_surrogate_follow( code_point cp )
    {
        if ( 0xDC00 <= cp && cp <= 0xDFFF ) // tests if the cp is within the 2nd word of a surrogate pair
            return true; // it is a 2nd word
        return false; // everything else isn't
    }

    size_t UTFString::_utf16_char_length( code_point cp )
    {
        if ( 0xD800 <= cp && cp <= 0xDBFF ) // test if cp is the beginning of a surrogate pair
            return 2; // if it is, then we are 2 words long
        return 1; // otherwise we are only 1 word long
    }

    size_t UTFString::_utf16_char_length( unicode_char uc )
    {
        if ( uc > 0xFFFF ) // test if uc is greater than the single word maximum
            return 2; // if so, we need a surrogate pair
        return 1; // otherwise we can stuff it into a single word
    }

    size_t UTFString::_utf16_to_utf32( const code_point in_cp[2], unicode_char& out_uc )
    {
        const code_point& cp1 = in_cp[0];
        const code_point& cp2 = in_cp[1];
        bool wordPair = false;

        // does it look like a surrogate pair?
        if ( 0xD800 <= cp1 && cp1 <= 0xDBFF ) {
            // looks like one, but does the other half match the algorithm as well?
            if ( 0xDC00 <= cp2 && cp2 <= 0xDFFF )
                wordPair = true; // yep!
        }

        if ( !wordPair ) { // if we aren't a 100% authentic surrogate pair, then just copy the value
            out_uc = cp1;
            return 1;
        }

        unsigned short cU = cp1, cL = cp2; // copy upper and lower words of surrogate pair to writable buffers
        cU -= 0xD800; // remove the encoding markers
        cL -= 0xDC00;

        out_uc = ( cU & 0x03FF ) << 10; // grab the 10 upper bits and set them in their proper location
        out_uc |= ( cL & 0x03FF ); // combine in the lower 10 bits
        out_uc += 0x10000; // add back in the value offset

        return 2; // this whole operation takes to words, so that's what we'll return
    }

    size_t UTFString::_utf32_to_utf16( const unicode_char& in_uc, code_point out_cp[2] )
    {
        if ( in_uc <= 0xFFFF ) { // we blindly preserve sentinel values because our decoder understands them
            out_cp[0] = static_cast<code_point>(in_uc);
            return 1;
        }
        unicode_char uc = in_uc; // copy to writable buffer
        unsigned short tmp; // single code point buffer
        uc -= 0x10000; // subtract value offset

        //process upper word
        tmp = static_cast<unsigned short>(( uc >> 10 ) & 0x03FF); // grab the upper 10 bits
        tmp += 0xD800; // add encoding offset
        out_cp[0] = tmp; // write

        // process lower word
        tmp = static_cast<unsigned short>(uc & 0x03FF); // grab the lower 10 bits
        tmp += 0xDC00; // add encoding offset
        out_cp[1] = tmp; // write

        return 2; // return used word count (2 for surrogate pairs)
    }

    bool UTFString::_utf8_start_char( unsigned char cp )
    {
        return ( cp & ~_cont_mask ) != _cont;
    }

    size_t UTFString::_utf8_char_length( unsigned char cp )
    {
        if ( !( cp & 0x80 ) ) return 1;
        if (( cp & ~_lead1_mask ) == _lead1 ) return 2;
        if (( cp & ~_lead2_mask ) == _lead2 ) return 3;
        if (( cp & ~_lead3_mask ) == _lead3 ) return 4;
        if (( cp & ~_lead4_mask ) == _lead4 ) return 5;
        if (( cp & ~_lead5_mask ) == _lead5 ) return 6;
        throw invalid_data( "invalid UTF-8 sequence header value" );
    }

    size_t UTFString::_utf8_char_length( unicode_char uc )
    {
        /*
        7 bit:  U-00000000 - U-0000007F: 0xxxxxxx
        11 bit: U-00000080 - U-000007FF: 110xxxxx 10xxxxxx
        16 bit: U-00000800 - U-0000FFFF: 1110xxxx 10xxxxxx 10xxxxxx
        21 bit: U-00010000 - U-001FFFFF: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        26 bit: U-00200000 - U-03FFFFFF: 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        31 bit: U-04000000 - U-7FFFFFFF: 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        */
        if ( !( uc & ~0x0000007F ) ) return 1;
        if ( !( uc & ~0x000007FF ) ) return 2;
        if ( !( uc & ~0x0000FFFF ) ) return 3;
        if ( !( uc & ~0x001FFFFF ) ) return 4;
        if ( !( uc & ~0x03FFFFFF ) ) return 5;
        if ( !( uc & ~0x7FFFFFFF ) ) return 6;
        throw invalid_data( "invalid UTF-32 value" );
    }

    size_t UTFString::_utf8_to_utf32( const unsigned char in_cp[6], unicode_char& out_uc )
    {
        size_t len = _utf8_char_length( in_cp[0] );
        if ( len == 1 ) { // if we are only 1 byte long, then just grab it and exit
            out_uc = in_cp[0];
            return 1;
        }

        unicode_char c = 0; // temporary buffer
        size_t i = 0;
        switch ( len ) { // load header byte
            case 6:
                c = in_cp[i] & _lead5_mask;
                break;
            case 5:
                c = in_cp[i] & _lead4_mask;
                break;
            case 4:
                c = in_cp[i] & _lead3_mask;
                break;
            case 3:
                c = in_cp[i] & _lead2_mask;
                break;
            case 2:
                c = in_cp[i] & _lead1_mask;
                break;
        }

        for ( ++i; i < len; i++ ) { // load each continuation byte
            if (( in_cp[i] & ~_cont_mask ) != _cont )
                throw invalid_data( "bad UTF-8 continuation byte" );
            c <<= 6;
            c |= ( in_cp[i] & _cont_mask );
        }

        out_uc = c; // write the final value and return the used byte length
        return len;
    }

    size_t UTFString::_utf32_to_utf8( const unicode_char& in_uc, unsigned char out_cp[6] )
    {
        size_t len = _utf8_char_length( in_uc ); // predict byte length of sequence
        unicode_char c = in_uc; // copy to temp buffer

        //stuff all of the lower bits
        for ( size_t i = len - 1; i > 0; i-- ) {
            out_cp[i] = static_cast<unsigned char>((( c ) & _cont_mask ) | _cont);
            c >>= 6;
        }

        //now write the header byte
        switch ( len ) {
            case 6:
                out_cp[0] = static_cast<unsigned char>((( c ) & _lead5_mask ) | _lead5);
                break;
            case 5:
                out_cp[0] = static_cast<unsigned char>((( c ) & _lead4_mask ) | _lead4);
                break;
            case 4:
                out_cp[0] = static_cast<unsigned char>((( c ) & _lead3_mask ) | _lead3);
                break;
            case 3:
                out_cp[0] = static_cast<unsigned char>((( c ) & _lead2_mask ) | _lead2);
                break;
            case 2:
                out_cp[0] = static_cast<unsigned char>((( c ) & _lead1_mask ) | _lead1);
                break;
            case 1:
            default:
                out_cp[0] = static_cast<unsigned char>(( c ) & 0x7F);
                break;
        }

        // return the byte length of the sequence
        return len;
    }

    Ogre::UTFString::size_type UTFString::_verifyUTF8( const unsigned char* c_str )
    {
        std::string tmp( reinterpret_cast<const char*>( c_str ) );
        return _verifyUTF8( tmp );
    }

    Ogre::UTFString::size_type UTFString::_verifyUTF8( const std::string& str )
    {
        std::string::const_iterator i, ie = str.end();
        i = str.begin();
        size_type length = 0;

        while ( i != ie ) {
            // characters pass until we find an extended sequence
            if (( *i ) & 0x80 ) {
                unsigned char c = ( *i );
                size_t contBytes = 0;

                // get continuation byte count and test for overlong sequences
                if (( c & ~_lead1_mask ) == _lead1 ) { // 1 additional byte
                    if ( c == _lead1 ) throw invalid_data( "overlong UTF-8 sequence" );
                    contBytes = 1;

                } else if (( c & ~_lead2_mask ) == _lead2 ) { // 2 additional bytes
                    contBytes = 2;
                    if ( c == _lead2 ) { // possible overlong UTF-8 sequence
                        c = ( *( i + 1 ) ); // look ahead to next byte in sequence
                        if (( c & _lead2 ) == _cont ) throw invalid_data( "overlong UTF-8 sequence" );
                    }

                } else if (( c & ~_lead3_mask ) == _lead3 ) { // 3 additional bytes
                    contBytes = 3;
                    if ( c == _lead3 ) { // possible overlong UTF-8 sequence
                        c = ( *( i + 1 ) ); // look ahead to next byte in sequence
                        if (( c & _lead3 ) == _cont ) throw invalid_data( "overlong UTF-8 sequence" );
                    }

                } else if (( c & ~_lead4_mask ) == _lead4 ) { // 4 additional bytes
                    contBytes = 4;
                    if ( c == _lead4 ) { // possible overlong UTF-8 sequence
                        c = ( *( i + 1 ) ); // look ahead to next byte in sequence
                        if (( c & _lead4 ) == _cont ) throw invalid_data( "overlong UTF-8 sequence" );
                    }

                } else if (( c & ~_lead5_mask ) == _lead5 ) { // 5 additional bytes
                    contBytes = 5;
                    if ( c == _lead5 ) { // possible overlong UTF-8 sequence
                        c = ( *( i + 1 ) ); // look ahead to next byte in sequence
                        if (( c & _lead5 ) == _cont ) throw invalid_data( "overlong UTF-8 sequence" );
                    }
                }

                // check remaining continuation bytes for
                while ( contBytes-- ) {
                    c = ( *( ++i ) ); // get next byte in sequence
                    if (( c & ~_cont_mask ) != _cont )
                        throw invalid_data( "bad UTF-8 continuation byte" );
                }
            }
            length++;
            i++;
        }
        return length;
    }

    void UTFString::_init()
    {
        mBuffer.mVoidBuffer = 0;
        mBufferType = bt_none;
        mBufferSize = 0;
    }

    void UTFString::_cleanBuffer() const
    {
        if ( mBuffer.mVoidBuffer != 0 ) {
            switch ( mBufferType ) {
                case bt_string:
                    delete mBuffer.mStrBuffer;
                    break;
                case bt_wstring:
                    delete mBuffer.mWStrBuffer;
                    break;
                case bt_utf32string:
                    delete mBuffer.mUTF32StrBuffer;
                    break;
                case bt_none: // under the worse of circumstances, this is all we can do, and hope it works out
                default:
                    //delete mBuffer.mVoidBuffer;
                    // delete void* is undefined, don't do that
                    assert("This should never happen - mVoidBuffer should never contain something if we "
                        "don't know the type");
                    break;
            }
            mBuffer.mVoidBuffer = 0;
            mBufferSize = 0;
            mBufferType = bt_none;
        }
    }

    void UTFString::_getBufferStr() const
    {
        if ( mBufferType != bt_string ) {
            _cleanBuffer();
            mBuffer.mStrBuffer = new std::string();
            mBufferType = bt_string;
        }
        mBuffer.mStrBuffer->clear();
    }

    void UTFString::_getBufferWStr() const
    {
        if ( mBufferType != bt_wstring ) {
            _cleanBuffer();
            mBuffer.mWStrBuffer = new std::wstring();
            mBufferType = bt_wstring;
        }
        mBuffer.mWStrBuffer->clear();
    }

    void UTFString::_getBufferUTF32Str() const
    {
        if ( mBufferType != bt_utf32string ) {
            _cleanBuffer();
            mBuffer.mUTF32StrBuffer = new utf32string();
            mBufferType = bt_utf32string;
        }
        mBuffer.mUTF32StrBuffer->clear();
    }

    void UTFString::_load_buffer_UTF8() const
    {
        _getBufferStr();
        std::string& buffer = ( *mBuffer.mStrBuffer );
        buffer.reserve( length() );

        unsigned char utf8buf[6] = "";
        char* charbuf = ( char* )utf8buf;
        unicode_char c;
        size_t len;

        const_iterator i, ie = end();
        for ( i = begin(); i != ie; i.moveNext() ) {
            c = i.getCharacter();
            len = _utf32_to_utf8( c, utf8buf );
            size_t j = 0;
            while ( j < len )
                buffer.push_back( charbuf[j++] );
        }
    }

    void UTFString::_load_buffer_WStr() const
    {
        _getBufferWStr();
        std::wstring& buffer = ( *mBuffer.mWStrBuffer );
        buffer.reserve( length() ); // may over reserve, but should be close enough
#ifdef WCHAR_UTF16 // wchar_t matches UTF-16
        const_iterator i, ie = end();
        for ( i = begin(); i != ie; ++i ) {
            buffer.push_back(( wchar_t )( *i ) );
        }
#else // wchar_t fits UTF-32
        unicode_char c;
        const_iterator i, ie = end();
        for ( i = begin(); i != ie; i.moveNext() ) {
            c = i.getCharacter();
            buffer.push_back(( wchar_t )c );
        }
#endif
    }

    void UTFString::_load_buffer_UTF32() const
    {
        _getBufferUTF32Str();
        utf32string& buffer = ( *mBuffer.mUTF32StrBuffer );
        buffer.reserve( length() ); // may over reserve, but should be close enough

        unicode_char c;

        const_iterator i, ie = end();
        for ( i = begin(); i != ie; i.moveNext() ) {
            c = i.getCharacter();
            buffer.push_back( c );
        }
    }

}
