/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#include "OgreStableHeaders.h"
#include "OgrePrerequisites.h"
#include "OgreCommon.h"
#include "OgreString.h"
#include "OgreLogManager.h"

namespace Ogre 
{
    int findCommandLineOpts(int numargs, char** argv, UnaryOptionList& unaryOptList, 
        BinaryOptionList& binOptList)
    {
        int startIndex = 1;
        for (int i = 1; i < numargs; ++i)
        {
            String tmp(argv[i]);
            if (StringUtil::startsWith(tmp, "-"))
            {
                UnaryOptionList::iterator ui = unaryOptList.find(argv[i]);
                if(ui != unaryOptList.end())
                {
                    ui->second = true;
                    ++startIndex;
                    continue;
                }
                BinaryOptionList::iterator bi = binOptList.find(argv[i]);
                if(bi != binOptList.end())
                {
                    bi->second = argv[i+1];
                    startIndex += 2;
                    ++i;
                    continue;
                }

                // Invalid option
                LogManager::getSingleton().logMessage("Invalid option " + tmp);

            }
        }
        return startIndex;
    }

	/** General hash function, derived from here
	http://www.azillionmonkeys.com/qed/hash.html
	Original by Paul Hsieh 
	*/
#define OGRE_GET16BITS(d) (*((const uint16 *) (d)))
	uint32 _OgreExport FastHash (const char * data, int len, uint32 hashSoFar)
	{
		uint32 hash;
		uint32 tmp;
		int rem;

		if (hashSoFar)
			hash = hashSoFar;
		else
			hash = len;

		if (len <= 0 || data == NULL) return 0;

		rem = len & 3;
		len >>= 2;

		/* Main loop */
		for (;len > 0; len--) {
			hash  += OGRE_GET16BITS (data);
			tmp    = (OGRE_GET16BITS (data+2) << 11) ^ hash;
			hash   = (hash << 16) ^ tmp;
			data  += 2*sizeof (uint16);
			hash  += hash >> 11;
		}

		/* Handle end cases */
		switch (rem) {
		case 3: hash += OGRE_GET16BITS (data);
			hash ^= hash << 16;
			hash ^= data[sizeof (uint16)] << 18;
			hash += hash >> 11;
			break;
		case 2: hash += OGRE_GET16BITS (data);
			hash ^= hash << 11;
			hash += hash >> 17;
			break;
		case 1: hash += *data;
			hash ^= hash << 10;
			hash += hash >> 1;
		}

		/* Force "avalanching" of final 127 bits */
		hash ^= hash << 3;
		hash += hash >> 5;
		hash ^= hash << 4;
		hash += hash >> 17;
		hash ^= hash << 25;
		hash += hash >> 6;

		return hash;
	}


}
