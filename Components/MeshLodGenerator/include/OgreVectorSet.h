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

#ifndef _VectorSet_H__
#define _VectorSet_H__

#include "OgreSmallVector.h"

namespace Ogre
{

/// VectorSet is basically a helper to use a vector as a small set container.
/// Also these functions keep the code clean and fast.
/// You can insert in O(1) time, if you know that it doesn't exists.
/// You can remove in O(1) time, if you know the position of the item.
template<typename T, unsigned S>
struct VectorSet :
    public SmallVector<T, S>
{

    typedef typename SmallVector<T, S>::iterator iterator;

    void addNotExists(const T& item); // Complexity: O(1)!!
    void remove(iterator it); // Complexity: O(1)!!
    iterator add(const T& item); // Complexity: O(N)
    void removeExists(const T& item); // Complexity: O(N)
    bool remove(const T& item); // Complexity: O(N)
    void replaceExists(const T& oldItem, const T& newItem); // Complexity: O(N)
    bool has(const T& item); // Complexity: O(N)
    iterator find(const T& item); // Complexity: O(N)
    iterator findExists(const T& item); // Complexity: O(N)
};

}
#endif
