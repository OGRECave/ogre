/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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
#ifndef __SharedPtr_H__
#define __SharedPtr_H__

#include "OgrePrerequisites.h"
#include "OgreAtomicScalar.h"

namespace Ogre {
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup General
	*  @{
	*/

	/// The method to use to free memory on destruction
	enum SharedPtrFreeMethod
	{
		/// Use OGRE_DELETE to free the memory
		SPFM_DELETE,
		/// Use OGRE_DELETE_T to free (only MEMCATEGORY_GENERAL supported)
		SPFM_DELETE_T,
		/// Use OGRE_FREE to free (only MEMCATEGORY_GENERAL supported)
		SPFM_FREE
	};

    struct SharedPtrInfo {
        inline SharedPtrInfo(SharedPtrFreeMethod freeMethod) 
            : useCount(1)
            , useFreeMethod(freeMethod) 
        {}

        AtomicScalar<unsigned> useCount;
        SharedPtrFreeMethod    useFreeMethod; /// If we should use OGRE_FREE instead of OGRE_DELETE
    };

	class SharedPtrDeleter
	{
	public:
		virtual void destroy()=0;

		virtual ~SharedPtrDeleter() {}
	};

	template <class T>
	class SharedPtrDeleterDelete : public SharedPtrDeleter
	{
		T*             pRep;
		public:
			SharedPtrDeleterDelete(T* rep) : pRep(rep) {}
		void destroy()
		{
			OGRE_DELETE pRep;
		}
	};

	template <class T>
	class SharedPtrDeleterDeleteT : public SharedPtrDeleter
	{
		T*             pRep;
		public:
			SharedPtrDeleterDeleteT(T* rep) : pRep(rep) {}
		void destroy()
		{
			OGRE_DELETE_T(pRep, T, MEMCATEGORY_GENERAL);
		}
	};

	template <class T>
	class SharedPtrDeleterFree : public SharedPtrDeleter
	{
		T*             pRep;
		public:
			SharedPtrDeleterFree(T* rep) : pRep(rep) {}
		void destroy()
		{
			OGRE_FREE(pRep, MEMCATEGORY_GENERAL);
		}
	};


	/** Reference-counted shared pointer, used for objects where implicit destruction is 
        required. 
    @remarks
        This is a standard shared pointer implementation which uses a reference 
        count to work out when to delete the object. 
	@par
		If OGRE_THREAD_SUPPORT is defined to be 1, use of this class is thread-safe.
    */
	template<class T> class SharedPtr
	{
        template<typename Y> friend class SharedPtr;
	protected:
		T*             pRep;
        SharedPtrInfo* pInfo;
		SharedPtrDeleter* pDel;

        SharedPtr(T* rep, SharedPtrInfo* info, SharedPtrDeleter* del) : pRep(rep), pInfo(info), pDel(del)
		{
		}

	public:
		/** Constructor, does not initialise the SharedPtr.
			@remarks
				<b>Dangerous!</b> You have to call bind() before using the SharedPtr.
		*/
		SharedPtr() : pRep(0), pInfo(0), pDel(0)
        {}

		/** Constructor.
		@param rep The pointer to take ownership of
		@param inFreeMethod The mechanism to use to free the pointer
		*/
        template< class Y>
		explicit SharedPtr(Y* rep, SharedPtrFreeMethod inFreeMethod = SPFM_DELETE) 
            : pRep(rep)
            , pInfo(rep ? OGRE_NEW_T(SharedPtrInfo, MEMCATEGORY_GENERAL)(inFreeMethod) : 0)
		{
			switch (inFreeMethod)
			{
				case SPFM_DELETE:  pDel = new SharedPtrDeleterDelete<Y>(rep);break;
				case SPFM_DELETE_T:pDel = new SharedPtrDeleterDeleteT<Y>(rep);break;
				case SPFM_FREE:    pDel = new SharedPtrDeleterFree<Y>(rep);break;
			}
		}

		SharedPtr(const SharedPtr& r)
            : pRep(r.pRep)
            , pInfo(r.pInfo)
			, pDel(r.pDel)
		{
            if (pRep) 
            {
                ++pInfo->useCount;
            }
		}

		SharedPtr& operator=(const SharedPtr& r) {
			if (pRep == r.pRep)
            {
                assert(pInfo == r.pInfo);
				return *this;
            }
			// Swap current data into a local copy
			// this ensures we deal with rhs and this being dependent
			SharedPtr<T> tmp(r);
			swap(tmp);
			return *this;
		}
		
		/* For C++11 compilers, use enable_if to only expose functions when viable
         *
         * MSVC 2012 and earlier only claim conformance to C++98. This is fortunate,
         * because they don't support default template parameters
         */
#if __cplusplus >= 201103L
        template<class Y,
            class = typename std::enable_if<std::is_convertible<Y*, T*>::value>::type>
#else
        template<class Y>
#endif
        SharedPtr(const SharedPtr<Y>& r)
            : pRep(r.getPointer())
            , pInfo(r.pInfo)
			, pDel(r.pDel)
		{
            if (pRep) 
            {
                ++pInfo->useCount;
            }
        }

		
#if __cplusplus >= 201103L
        template<class Y,
                 class = typename std::enable_if<std::is_assignable<T*, Y*>::value>::type>
#else
        template<class Y>
#endif
        SharedPtr& operator=(const SharedPtr<Y>& r)
        {
			if (pRep == r.pRep)
				return *this;
			// Swap current data into a local copy
			// this ensures we deal with rhs and this being dependent
			SharedPtr<T> tmp(r);
			swap(tmp);
			return *this;
		}

		~SharedPtr() {
            release();
		}


        template<typename Y>
        inline SharedPtr<Y> staticCast() const
        {
            if(pRep) {
                ++pInfo->useCount;
				return SharedPtr<Y>(static_cast<Y*>(pRep), pInfo, pDel);
            } else return SharedPtr<Y>();
        }

        template<typename Y>
        inline SharedPtr<Y> dynamicCast() const
        {
            if(pRep) {
                Y* rep = dynamic_cast<Y*>(pRep);
                ++pInfo->useCount;
				return SharedPtr<Y>(rep, pInfo, pDel);
            } else return SharedPtr<Y>();
        }

		inline T& operator*() const { assert(pRep); return *pRep; }
		inline T* operator->() const { assert(pRep); return pRep; }
		inline T* get() const { return pRep; }

		/** Binds rep to the SharedPtr.
			@remarks
				Assumes that the SharedPtr is uninitialised!
		*/
		void bind(T* rep, SharedPtrFreeMethod inFreeMethod = SPFM_DELETE) {
			assert(!pRep && !pInfo);
			pInfo = OGRE_NEW_T(SharedPtrInfo, MEMCATEGORY_GENERAL)(inFreeMethod);
			pRep = rep;
			switch (inFreeMethod)
			{
				case SPFM_DELETE:  pDel = new SharedPtrDeleterDelete<T>(rep);break;
				case SPFM_DELETE_T:pDel = new SharedPtrDeleterDeleteT<T>(rep);break;
				case SPFM_FREE:    pDel = new SharedPtrDeleterFree<T>(rep);break;
			}
		}

		inline bool unique() const { assert(pInfo && pInfo->useCount.get()); return pInfo->useCount.get() == 1; }
		inline unsigned int useCount() const { assert(pInfo && pInfo->useCount.get()); return pInfo->useCount.get(); }
        inline void setUseCount(unsigned value) { assert(pInfo); pInfo->useCount = value; }

		inline T* getPointer() const { return pRep; }
		inline SharedPtrFreeMethod freeMethod() const { return pInfo->useFreeMethod; }

		inline bool isNull(void) const { return pRep == 0; }

        inline void setNull(void) { 
			if (pRep)
			{
				release();
			}
        }

    protected:

        inline void release(void)
        {
            if (pRep)
            {
                assert(pInfo);
                if(--pInfo->useCount == 0)
                    destroy();
            }

            pRep = 0;
            pInfo = 0;
			pDel=0;
        }

        /** IF YOU GET A CRASH HERE, YOU FORGOT TO FREE UP POINTERS
         BEFORE SHUTTING OGRE DOWN
         Use setNull() before shutdown or make sure your pointer goes
         out of scope before OGRE shuts down to avoid this. */
        inline void destroy(void)
        {
            assert(pRep && pInfo);
			/*switch(pInfo->useFreeMethod)
			{
			case SPFM_DELETE:
				OGRE_DELETE pRep;
				break;
			case SPFM_DELETE_T:
				OGRE_DELETE_T(pRep, T, MEMCATEGORY_GENERAL);
				break;
			case SPFM_FREE:
				OGRE_FREE(pRep, MEMCATEGORY_GENERAL);
				break;
			};*/
			pDel->destroy();

            OGRE_DELETE_T(pInfo, SharedPtrInfo, MEMCATEGORY_GENERAL);
			delete pDel;
        }

		inline void swap(SharedPtr<T> &other) 
		{
			std::swap(pRep, other.pRep);
			std::swap(pInfo, other.pInfo);
			std::swap(pDel, other.pDel);
		}
	};

	template<class T, class U> inline bool operator==(SharedPtr<T> const& a, SharedPtr<U> const& b)
	{
		return a.get() == b.get();
	}

	template<class T, class U> inline bool operator!=(SharedPtr<T> const& a, SharedPtr<U> const& b)
	{
		return a.get() != b.get();
	}

	template<class T, class U> inline bool operator<(SharedPtr<T> const& a, SharedPtr<U> const& b)
	{
		return std::less<const void*>()(a.get(), b.get());
	}
	/** @} */
	/** @} */
}



#endif
