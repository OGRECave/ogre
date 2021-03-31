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
    typedef void (*createFunc)( Archive**, const String& );

    //-----------------------------------------------------------------------
    template<> ArchiveManager* Singleton<ArchiveManager>::msSingleton = 0;
    ArchiveManager* ArchiveManager::getSingletonPtr(void)
    {
        return msSingleton;
    }
    ArchiveManager& ArchiveManager::getSingleton(void)
    {  
        assert( msSingleton );  return ( *msSingleton );  
    }
    //-----------------------------------------------------------------------
    ArchiveManager::ArchiveManager()
    {
    }
    //-----------------------------------------------------------------------
    Archive* ArchiveManager::load( const String& filename, const String& archiveType, bool readOnly)
    {
        ArchiveMap::iterator i = mArchives.find(filename);
        Archive* pArch = 0;

        if (i == mArchives.end())
        {
            // Search factories
            ArchiveFactoryMap::iterator it = mArchFactories.find(archiveType);
            if (it == mArchFactories.end())
            {
                // Factory not found
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
                            "Cannot find an ArchiveFactory for type '" + archiveType + "'");
            }

            pArch = it->second->createInstance(filename, readOnly);
            pArch->load();
            mArchives[filename] = pArch;

        }
        else
        {
            pArch = i->second;
            OgreAssert(pArch->isReadOnly() == readOnly, "existing archive location has different readOnly status");
        }

        return pArch;
    }
    //-----------------------------------------------------------------------
    void ArchiveManager::unload(Archive* arch)
    {
        unload(arch->getName());
    }
    //-----------------------------------------------------------------------
    void ArchiveManager::unload(const String& filename)
    {
        ArchiveMap::iterator i = mArchives.find(filename);

        if (i != mArchives.end())
        {
            i->second->unload();
            // Find factory to destroy. An archive factory created this file, it should still be there!
            ArchiveFactoryMap::iterator fit = mArchFactories.find(i->second->getType());
            assert( fit != mArchFactories.end() && "Cannot find an ArchiveFactory "
                    "to deal with archive this type" );
            fit->second->destroyInstance(i->second);
            mArchives.erase(i);
        }
    }
    //-----------------------------------------------------------------------
    ArchiveManager::ArchiveMapIterator ArchiveManager::getArchiveIterator(void)
    {
        return ArchiveMapIterator(mArchives.begin(), mArchives.end());
    }
    //-----------------------------------------------------------------------
    ArchiveManager::~ArchiveManager()
    {
        // Thanks to http://www.viva64.com/en/examples/V509/ for finding the error for us!
        // (originally, it detected we were throwing using OGRE_EXCEPT in the destructor)
        // We now raise an assert.

        // Unload & delete resources in turn
        for( ArchiveMap::iterator it = mArchives.begin(); it != mArchives.end(); ++it )
        {
            Archive* arch = it->second;
            // Unload
            arch->unload();
            // Find factory to destroy. An archive factory created this file, it should still be there!
            ArchiveFactoryMap::iterator fit = mArchFactories.find(arch->getType());
            assert( fit != mArchFactories.end() && "Cannot find an ArchiveFactory "
                    "to deal with archive this type" );
            fit->second->destroyInstance(arch);
            
        }
        // Empty the list
        mArchives.clear();
    }
    //-----------------------------------------------------------------------
    void ArchiveManager::addArchiveFactory(ArchiveFactory* factory)
    {
        mArchFactories.emplace(factory->getType(), factory);
        LogManager::getSingleton().logMessage("ArchiveFactory for type '" + factory->getType() + "' registered");
    }

}

