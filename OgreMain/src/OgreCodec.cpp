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
#include "OgreStableHeaders.h"
#include "OgreCodec.h"
#include "OgreException.h"
#include "OgreStringConverter.h"


namespace Ogre {

    map< String, Codec * >::type Codec::msMapCodecs;

    Codec::~Codec() {
    }

    StringVector Codec::getExtensions(void)
    {
        StringVector result;
        result.reserve(msMapCodecs.size());
        CodecList::const_iterator i;
        for (i = msMapCodecs.begin(); i != msMapCodecs.end(); ++i)
        {
            result.push_back(i->first);
        }
        return result;
    }

    Codec* Codec::getCodec(const String& extension)
    {
        String lwrcase = extension;
		StringUtil::toLowerCase(lwrcase);
        CodecList::const_iterator i = msMapCodecs.find(lwrcase);
        if (i == msMapCodecs.end())
        {
            String formats_str;
            if(msMapCodecs.empty())
                formats_str = "There are no formats supported (no codecs registered).";
            else
                formats_str = "Supported formats are: " + StringConverter::toString(getExtensions()) + ".";

            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                "Can not find codec for '" + extension + "' image format.\n" + 
                formats_str,
                "Codec::getCodec");
        }

        return i->second;

    }

	Codec* Codec::getCodec(char *magicNumberPtr, size_t maxbytes)
	{
		for (CodecList::const_iterator i = msMapCodecs. begin(); 
			i != msMapCodecs.end(); ++i)
		{
			String ext = i->second->magicNumberToFileExt(magicNumberPtr, maxbytes);
			if (!ext.empty())
			{
				// check codec type matches
				// if we have a single codec class that can handle many types, 
				// and register many instances of it against different types, we
				// can end up matching the wrong one here, so grab the right one
				if (ext == i->second->getType())
					return i->second;
				else
					return getCodec(ext);
			}
		}

		return 0;

	}

}
