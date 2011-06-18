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

#ifndef __TestBatch_H__
#define __TestBatch_H__

#include "Ogre.h"
#include "ImageCompare.h"

class TestBatch;
typedef std::set<TestBatch,std::greater<TestBatch> > TestBatchSet;
typedef Ogre::SharedPtr<TestBatchSet> TestBatchSetPtr;

/** Represents the output from running a batch of tests
 *        (i.e. a single run of the TestContext) */
class TestBatch
{
public:

    // image files from this batch
    std::vector<Ogre::String> images;
    // This set's name (usually of the form: [TestPluginName]_[Timestamp])
    Ogre::String name;
    // version string
    Ogre::String version;
    // timestamp (when the batch was run)
    Ogre::String timestamp;
    // user comment made at the time of the run
    Ogre::String comment;
    // resolution the batch was run at
    size_t resolutionX;
    size_t resolutionY;

    /** Initialize based on a config file
     *        @param info Reference to loaded config file with details about the set 
     *        @param directory The full path to this set's directory */
    TestBatch(Ogre::ConfigFile& info, Ogre::String directory):mDirectory(directory)
    {
        // fill out basic info
        Ogre::String res = info.getSetting("Resolution","Info");
        resolutionX = atoi(res.c_str());
        resolutionY = atoi(res.substr(res.find('x')+1).c_str());
        version = info.getSetting("Version","Info");
        timestamp = info.getSetting("Time","Info");
        comment = info.getSetting("Comment","Info");
        name = info.getSetting("Name","Info");
        // grab image names
        Ogre::ConfigFile::SettingsIterator it = info.getSettingsIterator("Tests");
        while(it.hasMoreElements())
            images.push_back(it.getNext());
    }

    /** Returns whether or not the passed in set is comparable
        this means they must have the same resolution and image (test)
        names. */
    bool canCompareWith(const TestBatch& other) const
    {
        if(resolutionX != other.resolutionX ||
            resolutionY != other.resolutionY ||
            images.size() != other.images.size())
            return false;

        for(int i = 0; i < images.size(); ++i)
            if(images[i] != other.images[i])
                return false;

        return true;
    }

    /** Gets the full path to the image at the specificied index */
    Ogre::String getImagePath(size_t index) const
    {
        return mDirectory + "/" + images[index] + ".png";
    }

    /** Does image comparison on all images between these two sets */
    ComparisonResultVectorPtr compare(const TestBatch& other) const
    {
        ComparisonResultVectorPtr out(OGRE_NEW_T(ComparisonResultVector, Ogre::MEMCATEGORY_GENERAL)(), Ogre::SPFM_DELETE_T);
        if(!canCompareWith(other))
        {
            out.setNull();
        }
        else
        {
            for(int i = 0; i < images.size(); ++i)
                out->push_back(compareImages(getImagePath(i), other.getImagePath(i)));
        }
        return out;
    }

    /** Greater than operator, so they can be sorted chronologically */
    bool operator>(const TestBatch& other) const
    {
        // due to the way timestamps are formatted, lexicographical ordering will also be chronological
        return timestamp > other.timestamp;
    }

    /** Loads all test batches found in a directory and returns a reference counted ptr 
     *    to a set containing all the valid batches */
    static TestBatchSetPtr loadTestBatches(Ogre::String directory)
    {
        TestBatchSetPtr out(OGRE_NEW_T(TestBatchSet, Ogre::MEMCATEGORY_GENERAL)(), Ogre::SPFM_DELETE_T);
        // use ArchiveManager to get a list of all subdirectories
        Ogre::Archive* testDir = Ogre::ArchiveManager::getSingleton().load(directory, "FileSystem");
        Ogre::StringVectorPtr tests = testDir->list(false, true);
        for(int i = 0; i < tests->size(); ++i)
        {
            Ogre::ConfigFile info;
            
            // look for info.cfg, if none found, must not be a batch directory
            try
            {
                info.load(directory + (*tests)[i] + "/info.cfg");
            }
            catch(Ogre::FileNotFoundException e)
            {
                continue;
            }

            out->insert(TestBatch(info, directory + (*tests)[i]));
        }

        return out;
    }

private:

    Ogre::String mDirectory;

};



#endif
