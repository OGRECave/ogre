/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2015 Torus Knot Software Ltd

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

#pragma once


#include "Ogre.h"			
#include "OgreHlmsPrerequisites.h"
#include "OgreIdString.h"
#include "OgreMurmurHash3.h"
#include <cstddef>

namespace Ogre
{
	/** \addtogroup Optional
	*  @{
	*/
	/** \addtogroup Hlms
	*  @{
	*/
	const String FilePatterns[] = { "_vs", "_fs", "_gs", "_ds", "_hs" };

	typedef std::vector< std::pair<IdString, String> > HlmsParamVec;

	inline bool OrderParamVecByKey(const std::pair<IdString, String> &_left,
		const std::pair<IdString, String> &_right)
    {
        return _left.first < _right.first;
    }

	inline uint32 calcHash(const void* data, size_t size)
	{
		uint32 finalHash;
		MurmurHash3_x86_32(data, size, IdString::Seed, &finalHash);
		return finalHash;
	}

	inline uint32 calcHash(const String& str)
	{
        return calcHash(str.c_str(), str.length());
	}

	inline uint32 calcHash(const StringVector& vec)
	{
		StringStream stream;
		for (unsigned int i = 0; i < vec.size(); i++)
		{
			stream << vec[i];
		}
		return calcHash(stream.str());
	}
}
