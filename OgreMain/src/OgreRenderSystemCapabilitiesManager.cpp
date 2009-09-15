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

#include "OgreStringVector.h"
#include "OgreLogManager.h"
#include "OgreArchiveManager.h"
#include "OgreArchive.h"
#include "OgreStringConverter.h"

#include "OgreException.h"
#include "OgreRenderSystemCapabilitiesManager.h"
#include "OgreRenderSystemCapabilitiesSerializer.h"



namespace Ogre {

    //-----------------------------------------------------------------------
    template<> RenderSystemCapabilitiesManager* Singleton<RenderSystemCapabilitiesManager>::ms_Singleton = 0;
    RenderSystemCapabilitiesManager* RenderSystemCapabilitiesManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    RenderSystemCapabilitiesManager& RenderSystemCapabilitiesManager::getSingleton(void)
    {
        assert( ms_Singleton );  return ( *ms_Singleton );
    }

    //-----------------------------------------------------------------------
    RenderSystemCapabilitiesManager::RenderSystemCapabilitiesManager() : mSerializer(0), mScriptPattern("*.rendercaps")
    {
        mSerializer = OGRE_NEW RenderSystemCapabilitiesSerializer();
    }
    //-----------------------------------------------------------------------
    RenderSystemCapabilitiesManager::~RenderSystemCapabilitiesManager()
    {
        for (CapabilitiesMap::iterator it = mCapabilitiesMap.begin(), end = mCapabilitiesMap.end(); it != end; it++)
        {
        // free memory in RenderSystemCapabilities*
            OGRE_DELETE it->second;
        }

        OGRE_DELETE mSerializer;
    }

    //-----------------------------------------------------------------------
    void RenderSystemCapabilitiesManager::parseCapabilitiesFromArchive(const String& filename, const String& archiveType, bool recursive)
    {
        // get the list of .rendercaps files
        Archive* arch = ArchiveManager::getSingleton().load(filename, archiveType);
        StringVectorPtr files = arch->find(mScriptPattern, recursive);

        // loop through .rendercaps files and load each one
        for (StringVector::iterator it = files->begin(), end = files->end(); it != end; it++)
        {
            DataStreamPtr stream = arch->open(*it);
            mSerializer->parseScript(stream);
            stream->close();
        }
    }

    RenderSystemCapabilities* RenderSystemCapabilitiesManager::loadParsedCapabilities(const String& name)
    {
        return mCapabilitiesMap[name];
    }

    /** Method used by RenderSystemCapabilitiesSerializer::parseScript */
    void RenderSystemCapabilitiesManager::_addRenderSystemCapabilities(const String& name, RenderSystemCapabilities* caps)
    {
        mCapabilitiesMap.insert(CapabilitiesMap::value_type(name, caps));
    }
}





