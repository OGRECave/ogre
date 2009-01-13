/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
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
