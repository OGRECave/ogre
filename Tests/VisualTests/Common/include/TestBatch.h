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

#ifndef __TestBatch_H__
#define __TestBatch_H__

#include "Ogre.h"
#include "ImageValidator.h"

class TestBatch;
typedef std::set<TestBatch,std::greater<TestBatch> > TestBatchSet;

/** Represents the output from running a batch of tests
 *        (i.e. a single run of the TestContext) */
class TestBatch : public Ogre::GeneralAllocatedObject
{
public:

    // image files from this batch
    std::vector<Ogre::String> images;
    // This set's name (usually of the form: [TestPluginName]_[Timestamp])
    Ogre::String name;
    // name of the tets plugin 
    Ogre::String plugin;
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
        const Ogre::ConfigFile::SettingsMultiMap& tests = info.getSettings("Tests");
        for(Ogre::ConfigFile::SettingsMultiMap::const_iterator i = tests.begin(); i != tests.end(); ++i)
            images.push_back(i->second);
    }

    /** Manually initialize a batch object
     *        @param batchName The name of the overall test batch
     *        @param pluginName The name of the test plugin being used 
     *        @param timestamp The time the test was begun
     *        @param resx The width of the render window used
     *        @param resy The height of the render window used 
     *        @param directory The directory this batch is saved to */
    TestBatch(Ogre::String batchName, Ogre::String pluginName,
        Ogre::String t, size_t resx, size_t resy, Ogre::String directory)
        :name(batchName)
        ,plugin(pluginName)
        ,timestamp(t)
        ,comment("")
        ,resolutionX(resx)
        ,resolutionY(resy)
        ,mDirectory(directory)
    {
        StringStream ver;
        ver<<OGRE_VERSION_MAJOR<<"."<<OGRE_VERSION_MINOR<<" ("<<
            OGRE_VERSION_NAME<<") "<<OGRE_VERSION_SUFFIX;
        version = ver.str();
    }

    /** Returns whether or not the passed in set is comparable
        this means they must have the same resolution and image (test)
        names. */
    bool canCompareWith(const TestBatch& other) const
    {
        if (resolutionX != other.resolutionX ||
            resolutionY != other.resolutionY ||
            images.size() != other.images.size())
            return false;

        for (unsigned int i = 0; i < images.size(); ++i)
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
    ComparisonResultVector compare(const TestBatch& other) const
    {
        ComparisonResultVector out;
        if (!canCompareWith(other))
        {
            out.clear();
        }
        else
        {
            ImageValidator validator = ImageValidator(mDirectory, other.mDirectory);
            for (unsigned int i = 0; i < images.size(); ++i)
                out.push_back(validator.compare(images[i]));
        }
        return out;
    }

    // write the config file for this set
    void writeConfig()
    {
        // save a small .cfg file with some details about the batch
        std::ofstream config;
        config.open(Ogre::String(mDirectory + "/info.cfg").c_str());

        if (config.is_open())
        {
            config<<"[Info]\n";
            config<<"Name="<<name<<"\n";
            config<<"Time="<<timestamp<<"\n";
            config<<"Resolution="<<resolutionX<<"x"<<resolutionY<<"\n";
            config<<"Comment="<<comment<<"\n";
            config<<"Version="<<version<<"\n";
            config<<"[Tests]\n";

            // add entries for each image
            for (unsigned int i = 0; i < images.size(); ++i)
                config<<"Test="<<images[i]<<"\n";

            config.close();
        }
    }

    /** Greater than operator, so they can be sorted chronologically */
    bool operator>(const TestBatch& other) const
    {
        // due to the way timestamps are formatted, lexicographical ordering will also be chronological
        return timestamp > other.timestamp;
    }

    /** Loads all test batches found in a directory and returns a reference counted ptr 
     *    to a set containing all the valid batches */
    static TestBatchSet loadTestBatches(Ogre::String directory)
    {
        TestBatchSet out;
        // use ArchiveManager to get a list of all subdirectories
        Ogre::Archive* testDir = Ogre::ArchiveManager::getSingleton().load(directory, "FileSystem", true);
        Ogre::StringVectorPtr tests = testDir->list(false, true);
        for (unsigned int i = 0; i < tests->size(); ++i)
        {
            Ogre::ConfigFile info;
            
            // look for info.cfg, if none found, must not be a batch directory
            try
            {
                info.load(directory + (*tests)[i] + "/info.cfg");
            }
            catch (Ogre::FileNotFoundException& e)
            {
                continue;
            }

            out.insert(TestBatch(info, directory + (*tests)[i]));
        }

        return out;
    }

private:

    Ogre::String mDirectory;

};



#endif
