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
#include "OgreStableHeaders.h"

namespace Ogre {

    std::map< String, Codec * > Codec::msMapCodecs;

    Codec::~Codec() {
    }

    DataStreamPtr Codec::encode(const Any& input) const
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, getType() + " - encoding to memory not supported");
        return DataStreamPtr();
    }

    void Codec::encodeToFile(const Any& input, const String& outFileName) const
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, getType() + " - encoding to file not supported");
    }

    StringVector Codec::getExtensions(void)
    {
        StringVector result;
        result.reserve(msMapCodecs.size());
        for (auto& c : msMapCodecs)
        {
            result.push_back(c.first);
        }
        return result;
    }

    void Codec::registerCodec(Codec* pCodec)
    {
        auto ret = msMapCodecs.emplace(pCodec->getType(), pCodec);
        if (!ret.second)
            OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, pCodec->getType() + " already has a registered codec");
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
                        "Can not find codec for '" + extension + "' format.\n" + formats_str);
        }

        return i->second;
    }

    Codec* Codec::getCodec(char *magicNumberPtr, size_t maxbytes)
    {
        for (auto& c : msMapCodecs)
        {
            String ext = c.second->magicNumberToFileExt(magicNumberPtr, maxbytes);
            if (!ext.empty())
            {
                // check codec type matches
                // if we have a single codec class that can handle many types, 
                // and register many instances of it against different types, we
                // can end up matching the wrong one here, so grab the right one
                if (ext == c.second->getType())
                    return c.second;
                else
                    return getCodec(ext);
            }
        }

        return 0;
    }
}
