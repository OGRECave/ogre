/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/
-----------------------------------------------------------------------------
The MIT License (MIT)

Copyright (c) 2015 Kenny Kerr

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
-----------------------------------------------------------------------------

This file was derived from ComPtr implementation made by Kenny Kerr for moderncpp
library http://moderncpp.com, design was described in MSDN magazine
February 2015 article "Windows with C++ - COM Smart Pointers Revisited"
https://msdn.microsoft.com/en-us/magazine/dn904668.aspx  of the same author.

Code was downgraded to C++ 98, and to more closely match Microsoft::Wrl::ComPtr,
which is actually what we want except licensing issues.

'noexcept' was removed for performance reasons, to avoid extra try/catch block 
generation that calls std::terminate. It was replaced with ASSUME_NOEXCEPT macro
that expands to VC++ specific non-compliant 'throw ()' that works like a hint and
does not call std::unexpected nor std::terminate.

'explicit operator bool()' was replaced using SafeBool idiom

ComPtr::GetAddressOf() behavior reverted to match Microsoft::Wrl::ComPtr and not 
check that smart pointer is empty - this behavior is dangerous as it allows to modify
smart pointer internals, but it is preferred as many Direct3D11 samples expect it.

-----------------------------------------------------------------------------
*/
#ifndef __OgreComPtr_H__
#define __OgreComPtr_H__

#include "OgreD3D11Prerequisites.h"
#include <winerror.h> // for HRESULT

#ifdef __MINGW32__
typedef long HRESULT;
#endif

#if !defined(ASSUME_NOEXCEPT) && defined( _MSC_VER )
#define ASSUME_NOEXCEPT throw () // use non-compliant behavior of VC++ - compile as if exceptions are not possible, performing no run-time checks
#else
#define ASSUME_NOEXCEPT // don`t define it as 'noexcept' or compliant 'throw ()' or extra try/catch frame would be generated to call std::terminate/std::unexpected
#endif

namespace Ogre
{
    /** \addtogroup RenderSystems
    *  @{
    */
    /** \addtogroup Direct3D11
    *  @{
    */

    // Helper to hide reference count management from user
    template <typename T>
    class NoAddRefRelease : public T
    {
        unsigned long __stdcall AddRef();
        unsigned long __stdcall Release();
    };

    template <typename T>
    class ComPtr
    {
        T* m_ptr;

        template <typename U> friend class ComPtr;

        void InternalAddRef() const ASSUME_NOEXCEPT
        {
            if(m_ptr)
            {
                m_ptr->AddRef();
            }
        }

        void InternalRelease() ASSUME_NOEXCEPT
        {
            if(T * temp = m_ptr)
            {
                m_ptr = 0;
                temp->Release();
            }
        }

        void InternalCopy(T * other) ASSUME_NOEXCEPT
        {
            if(m_ptr != other)
            {
                InternalRelease();
                m_ptr = other;
                InternalAddRef();
            }
        }

    public:
        typedef T Type;

    public:
        ComPtr() ASSUME_NOEXCEPT : m_ptr(0) {}

        template <typename U>
        ComPtr(U* other) ASSUME_NOEXCEPT : m_ptr(other)
        {
            InternalAddRef();
        }

        ComPtr(const ComPtr & other) ASSUME_NOEXCEPT : m_ptr(other.m_ptr)
        {
            InternalAddRef();
        }

        template <typename U>
        ComPtr(ComPtr<U> const & other) ASSUME_NOEXCEPT : m_ptr(other.m_ptr)
        {
            InternalAddRef();
        }

        ~ComPtr() ASSUME_NOEXCEPT
        {
            InternalRelease();
        }

        ComPtr& operator=(T* other) ASSUME_NOEXCEPT
        {
            InternalCopy(other);
            return *this;
        }

        template <typename U>
        ComPtr& operator=(U* other) ASSUME_NOEXCEPT
        {
            InternalCopy(other);
            return *this;
        }

        ComPtr& operator=(const ComPtr& other) ASSUME_NOEXCEPT
        {
            InternalCopy(other.m_ptr);
            return *this;
        }

        template <typename U>
        ComPtr& operator=(const ComPtr<U>& other) ASSUME_NOEXCEPT
        {
            InternalCopy(other.m_ptr);
            return *this;
        }

        void Swap(ComPtr& other) ASSUME_NOEXCEPT
        {
            T* temp = m_ptr;
            m_ptr = other.m_ptr;
            other.m_ptr = temp;
        }

#if __cplusplus >= 201103L || OGRE_COMPILER == OGRE_COMPILER_MSVC && OGRE_COMP_VER >= 1800
        explicit operator bool() const ASSUME_NOEXCEPT
        {
            return 0 != m_ptr;
        }
#else
        struct SafeBoolHelper { int member; };
        typedef int SafeBoolHelper::* SafeBoolType;
        operator SafeBoolType() const ASSUME_NOEXCEPT
        {
            return 0 != m_ptr ? &SafeBoolHelper::member : 0;
        }
#endif

        T* Get() const ASSUME_NOEXCEPT
        {
            return m_ptr;
        }

        NoAddRefRelease<T>* operator->() const ASSUME_NOEXCEPT
        {
            return static_cast<NoAddRefRelease<T> *>(m_ptr);
        }

        T** GetAddressOf() ASSUME_NOEXCEPT
        {
            return &m_ptr;
        }

        T** ReleaseAndGetAddressOf() ASSUME_NOEXCEPT
        {
            InternalRelease();
            return &m_ptr;
        }

        T* Detach() ASSUME_NOEXCEPT
        {
            T* temp = m_ptr;
            m_ptr = 0;
            return temp;
        }

        void Attach(T* other) ASSUME_NOEXCEPT
        {
            InternalRelease();
            m_ptr = other;
        }

        void Reset() ASSUME_NOEXCEPT
        {
            InternalRelease();
        }

        template <typename U>
        HRESULT As(ComPtr<U> *res) const ASSUME_NOEXCEPT
        {
            return m_ptr->QueryInterface(res->ReleaseAndGetAddressOf());
        }

    };

    template<class T> void swap(ComPtr<T>& a, ComPtr<T>& b) { return a.Swap(b); }

    template<class T> bool operator==(const ComPtr<T>& a, const ComPtr<T>& b) ASSUME_NOEXCEPT { return a.Get() == b.Get(); }
    template<class T> bool operator!=(const ComPtr<T>& a, const ComPtr<T>& b) ASSUME_NOEXCEPT { return a.Get() != b.Get(); }
    template<class T> bool operator< (const ComPtr<T>& a, const ComPtr<T>& b) ASSUME_NOEXCEPT { return a.Get() < b.Get(); }
    template<class T> bool operator> (const ComPtr<T>& a, const ComPtr<T>& b) ASSUME_NOEXCEPT { return a.Get() > b.Get(); }
    template<class T> bool operator<=(const ComPtr<T>& a, const ComPtr<T>& b) ASSUME_NOEXCEPT { return a.Get() <= b.Get(); }
    template<class T> bool operator>=(const ComPtr<T>& a, const ComPtr<T>& b) ASSUME_NOEXCEPT { return a.Get() >= b.Get(); }

    /** @} */
    /** @} */
}
#endif
