/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2019 Torus Knot Software Ltd

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

#ifndef __ResourceLocationPriorityTest_H__
#define __ResourceLocationPriorityTest_H__

#include "OgreArchive.h"
#include "OgreArchiveFactory.h"
#include "OgreDataStream.h"

// Barebones archive containing a single 1-byte file "dummyArchiveTest" whose
// contents are an unsigned char that increments on each construction of the
// archive.
class DummyArchive : public Ogre::Archive
{
public:
    DummyArchive(const Ogre::String& name, const Ogre::String& archType)
        : Ogre::Archive(name, archType), mContents(DummyArchive::makeContents()) {}

    virtual ~DummyArchive() {}

    bool exists(const Ogre::String& name) const override { return name == "dummyArchiveTest"; }

    Ogre::StringVectorPtr find(const Ogre::String& pattern, bool recursive = true, bool dirs = false) const override
    {
        Ogre::StringVectorPtr results = std::make_shared<Ogre::StringVector>();
        if (dirs) return results;
        if (Ogre::StringUtil::match("dummyArchiveTest", pattern))
        {
            results->push_back("dummyArchiveTest");
        }
        return results;
    }

    Ogre::FileInfoListPtr findFileInfo(const Ogre::String& pattern, bool recursive = true,
                                               bool dirs = false) const override
    {
        Ogre::FileInfoListPtr results = std::make_shared<Ogre::FileInfoList>();
        if (dirs) return results;
        if (Ogre::StringUtil::match("dummyArchiveTest", pattern))
        {
            results->push_back(Ogre::FileInfo{this, "dummyArchiveTest", "/", "dummyArchiveTest", 0, 1});
        }
        return results;
    }

    time_t getModifiedTime(const Ogre::String& filename) const override { return 0; }

    bool isCaseSensitive() const override { return true; }

    Ogre::StringVectorPtr list(bool recursive = true, bool dirs = false) const override
    {
        Ogre::StringVectorPtr results = std::make_shared<Ogre::StringVector>();
        if (dirs) return results;
        results->push_back("dummyArchiveTest");
        return results;
    }

    Ogre::FileInfoListPtr listFileInfo(bool recursive = true, bool dirs = false) const override
    {
        Ogre::FileInfoListPtr results = std::make_shared<Ogre::FileInfoList>();
        if (dirs) return results;
        results->push_back(Ogre::FileInfo{this, "dummyArchiveTest", "/", "dummyArchiveTest", 0, 1});
        return results;
    }

    void load() override {}

    void unload() override {}

    Ogre::DataStreamPtr open(const Ogre::String& filename, bool readOnly = true) const override
    {
        if (filename == "dummyArchiveTest")
        {
            unsigned char* ptr = OGRE_ALLOC_T(unsigned char, 1, Ogre::MEMCATEGORY_GENERAL);
            *ptr = mContents;
            return std::make_shared<Ogre::MemoryDataStream>(ptr, 1, true, true);
        }
        return Ogre::MemoryDataStreamPtr();
    }

private:
    static unsigned char makeContents()
    {
        // Don't start at zero so it's obvious if things aren't initialized.
        static unsigned char counter = 1;
        return counter++;
    }

    unsigned char mContents;
};

class DummyArchiveFactory : public Ogre::ArchiveFactory
{
public:
    virtual ~DummyArchiveFactory() {}

    Ogre::Archive* createInstance(const Ogre::String& name, bool) override
    {
        return OGRE_NEW DummyArchive(name, "DummyArchive");
    }

    void destroyInstance(Ogre::Archive* ptr) override { OGRE_DELETE ptr; }

    const Ogre::String& getType() const override
    {
        static Ogre::String type = "DummyArchive";
        return type;
    }
};

#endif
