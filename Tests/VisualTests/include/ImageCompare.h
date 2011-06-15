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

#ifndef __ImageCompare_H__
#define __ImageCompare_H__

#include "Ogre.h"

/** A set of functions and structs used in comparing images,
 *    these will be used by the TestContext for generating output, 
 *    and I'll probably also make a small command line utility that will
 *    use this functionality. */

// Results of comparing two test images
struct ComparisonResult
{
    ComparisonResult(bool _passed):passed(_passed){}
    bool passed;
    // more info to be added later.
};
//-----------------------------------------------------------------------

// Info about a test set
struct TestImageSet
{
    Ogre::String name;
    unsigned int resolution_x;
    unsigned int resolution_y;
    std::vector<Ogre::String> images;
};
//-----------------------------------------------------------------------

/** Looks up info about a given test set */
bool getTestSetInfo(Ogre::String location, TestImageSet& set)
{
    // load up config file
    Ogre::ConfigFile info;
    try
    {
        info.load(location + set.name + "/info.cfg");
    }
    catch(Ogre::FileNotFoundException e)
    {
        return false;
    }

    Ogre::String res = info.getSetting("Resolution","Info");
    set.resolution_x = atoi(res.c_str());
    set.resolution_y = atoi(res.substr(res.find('x')+1).c_str());

    Ogre::Archive* testDir = Ogre::ArchiveManager::getSingleton().load(location+set.name+"/", "FileSystem");
    // get a list of the directory's contents
    Ogre::StringVectorPtr files = testDir->list(false);

    // find test images for the set
    for(int i = 0; i < files->size(); ++i)
    {
        if((*files)[i].find(".png") != Ogre::String::npos)
            set.images.push_back((*files)[i]);
    }

    return true;
}
//-----------------------------------------------------------------------

/** Determines whether or not two sets of images are comprable
 *        (must have same resolution, same image names, etc)*/
bool setsComparable(const TestImageSet& set1, const TestImageSet& set2)
{
    if(set1.resolution_x != set2.resolution_x 
        || set1.resolution_y != set2.resolution_y
        || set1.images.size() != set2.images.size())
        return false;
    for(int i = 0; i < set1.images.size(); ++i)
        if(set1.images[i] != set2.images[i])
            return false;
    return true;
}
//-----------------------------------------------------------------------

/** compares two images */
ComparisonResult compareImages(Ogre::String image1, Ogre::String image2)
{
    // haha, 50/50 chance of being right... actual image comparison will come later.
    return ComparisonResult(rand()%2);
}
//-----------------------------------------------------------------------

/** Compares two sets of test images, and outputs an HTML report
 *        @param testDirectory The path to the directory containing the tests
 *        @param set1 The first set to compare 
 *        @param set2 The set to compare with (this is optional, 
 *            if not passed the most recent compatible set is used) */
void compareTestSets(Ogre::String testDirectory, Ogre::String set1, Ogre::String set2="")
{
    TestImageSet testSet1;
    TestImageSet testSet2;

    testSet1.name = set1;
    if(!getTestSetInfo(testDirectory,testSet1))
        return;

    if(set2 == "")
    {
        Ogre::Archive* testDir = Ogre::ArchiveManager::getSingleton().load(testDirectory, "FileSystem");
        // get a list of the directories corresponding to tests
        Ogre::StringVectorPtr tests = testDir->list(false, true);
        // sort from newest to oldest (reverse lexicographical, since names == date of creation)
        std::sort(tests->begin(),tests->end(),std::greater<Ogre::String>());

        bool foundCompatibleSet = false;

        for(int i = 0; i < tests->size(); ++i)
        {
            testSet2.name = (*tests)[i];
            if(testSet2.name != testSet1.name && getTestSetInfo(testDirectory,testSet2))
            {
                if(setsComparable(testSet1,testSet2))
                {
                    foundCompatibleSet = true;
                    break;
                }
            }
        }

        if(!foundCompatibleSet)
            return;
    }
    else
    {
        testSet2.name = set2;
        if(!getTestSetInfo(testDirectory,testSet2))
            return;
    }

    std::vector<ComparisonResult> comparisons;

    // do the actual comparisons
    for(int i = 0; i < testSet1.images.size(); ++i)
    {
        comparisons.push_back(compareImages(testDirectory+testSet1.name+"/"+testSet1.images[i],
            testDirectory+testSet2.name+"/"+testSet2.images[i]));
    }

    // brief test output, this will probably use TinyXML when I start actual HTML output
    std::ofstream output;
    output.open(Ogre::String(testDirectory + "out.html").c_str());
    if(output.is_open())
    {
        output<<"Testing\n";
        output<<"Set 1: "<<testSet1.name<<"\n";
        output<<"Set 2: "<<testSet2.name<<"\n";
        for(int i = 0; i < comparisons.size(); ++i)
        {
            if(comparisons[i].passed)
                output<<"Test: "<<testSet1.images[i]<<" passed!\n";
            else
                output<<"Test: "<<testSet1.images[i]<<" failed!\n";
        }
        output.close();
    }
}

#endif
