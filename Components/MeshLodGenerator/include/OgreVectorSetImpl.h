/*
 * -----------------------------------------------------------------------------
 * This source file is part of OGRE
 * (Object-oriented Graphics Rendering Engine)
 * For the latest info, see http://www.ogre3d.org/
 *
 * Copyright (c) 2000-2014 Torus Knot Software Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * -----------------------------------------------------------------------------
 */

#ifndef _VectorSetImpl_H__
#define _VectorSetImpl_H__

#include "OgreVectorSet.h"

#include "OgreException.h" // for OgreAssert

namespace Ogre
{

    template<typename T, unsigned S>
    void VectorSet<T, S>::addNotExists(const T& item)
    {
        OgreAssertDbg(find(item) == this->end(), "");
        this->push_back(item);
    }

    template<typename T, unsigned S>
    void VectorSet<T, S>::remove(iterator it)
    {
        // Thats my trick to remove an item from the vector very fast!
        // It works similar to the heap_pop().
        // It swaps the removable item to the back, then pops it.
        *it = this->back();
        this->pop_back();
    }

    template<typename T, unsigned S>
    typename VectorSet<T, S>::iterator VectorSet<T, S>::add(const T& item)
    {
        iterator it = find(item);
        if (it == this->end()) {
            this->push_back(item);
            return this->end();
        }
        return it;
    }

    template<typename T, unsigned S>
    void VectorSet<T, S>::removeExists(const T& item)
    {
        iterator it = find(item);
        OgreAssertDbg(it != this->end(), "");
        remove(it);
    }

    template<typename T, unsigned S>
    bool VectorSet<T, S>::remove(const T& item)
    {
        iterator it = find(item);
        if (it != this->end()) {
            remove(it);
            return true;
        } else {
            return false;
        }
    }

    template<typename T, unsigned S>
    void VectorSet<T, S>::replaceExists(const T& oldItem, const T& newItem)
    {
        iterator it = find(oldItem);
        OgreAssertDbg(it != this->end(), "");
        *it = newItem;
    }

    template<typename T, unsigned S>
    bool VectorSet<T, S>::has(const T& item)
    {
        return find(item) != this->end();
    }

    template<typename T, unsigned S>
    typename VectorSet<T, S>::iterator VectorSet<T, S>::find(const T& item)
    {
        return std::find(this->begin(), this->end(), item);
    }

    template<typename T, unsigned S>
    typename VectorSet<T, S>::iterator VectorSet<T, S>::findExists(
        const T& item)
    {
        iterator it = find(item);
        OgreAssertDbg(it != this->end(), "");
        return it;
    }

}
#endif
