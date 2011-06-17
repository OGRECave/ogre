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
#include "tinyhtml.h"

/** A set of functions and structs used in comparing images,
 *    these will be used by the TestContext for generating output, 
 *    and I'll probably also make a small command line utility that will
 *    use this functionality. */

// Results of comparing two test images
struct ComparisonResult
{
    bool passed;
    Ogre::String image;
    Ogre::String testName;
    size_t frame;
    // more info to be added later.
};
//-----------------------------------------------------------------------

// Info about a test set
struct TestImageSet
{
    Ogre::String name;
    Ogre::String version;
    Ogre::String timestamp;
    Ogre::String comment;
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

	set.version = info.getSetting("Version","Info");
	set.timestamp = info.getSetting("Time","Info");
	set.comment = info.getSetting("Comment","Info");

    Ogre::Archive* testDir = Ogre::ArchiveManager::getSingleton().load(location+set.name+"/", "FileSystem");
    // get a list of the directory's contents
    Ogre::StringVectorPtr files = testDir->list(false);
	std::sort(files->begin(),files->end(),std::less<Ogre::String>());

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
    ComparisonResult out;
    out.image = image1.substr(image1.find_last_of("/") + 1);

    // find end of name
    size_t end = out.image.find_last_of("_");
    
    // extract test name from image filename
    out.testName = out.image.substr(12, end - 12);
    out.frame = atoi(out.image.substr(end+1).c_str());

	// placeholder, not exactly elegant, but it's better than nothing for the moment
	std::ifstream img1;
	std::ifstream img2;

	img1.open(image1.c_str());
	img2.open(image2.c_str());

    out.passed = false;

	if(img1.is_open() && img2.is_open())
	{
		bool foundDiff = false;
		while(img1.good())
		{
			// compare byte by byte...
			if((img1.get() != img2.get()) || (img2.good() != img1.good()))
			{
				foundDiff = true;
				break;
			}
		}
		out.passed = !foundDiff;
	}

    return out;
}
//-----------------------------------------------------------------------

/** Writes a table with some info about a test set */
HtmlElement* writeSetInfoTable(TestImageSet& set, Ogre::String name)
{
    HtmlElement* column = new HtmlElement("div");
    column->appendAttribute("class", "img_column");
    column->appendElement("h3")->appendText(name);
    HtmlElement* table = column->appendElement("table");
    HtmlElement* row = table->appendElement("tr");
    row->appendElement("th")->appendText("Time:");
    row->appendElement("td")->appendText(set.timestamp);
    row = table->appendElement("tr");
    row->appendElement("th")->appendText("Version:");
    row->appendElement("td")->appendText(set.version);
    row = table->appendElement("tr");
    row->appendElement("th")->appendText("Resolution:");
    row->appendElement("td")->appendText(Ogre::StringConverter::toString(set.resolution_x) 
		+ " x " + Ogre::StringConverter::toString(set.resolution_y));
    row = table->appendElement("tr");
    row->appendElement("th")->appendText("Comment:");
    row->appendElement("td")->appendText(set.comment);
    return column;
}
//-----------------------------------------------------------------------

/** Summarizes the results of a single test (side-by-side images, pass/fail,
 *    more stats and such to come...). Returns an html div with summary markup */
HtmlElement* summarizeSingleResult(ComparisonResult& result, TestImageSet& set1, TestImageSet& set2)
{
    HtmlElement* container = new HtmlElement("div");
    container->appendAttribute("id",result.testName);
    container->appendElement("h2")->appendText(result.testName);
    HtmlElement* content = container->appendElement("div");
    content->appendAttribute("class", Ogre::String("contentarea") 
        + (result.passed ? "" : " failed_test"));

    // first image
    HtmlElement* column1 = content->appendElement("div");
    column1->appendAttribute("class", "img_column");
    column1->appendElement("h3")->appendText("Original:");
    HtmlElement* img = column1->appendElement("img");
    img->appendAttribute("alt", result.testName + " original");
    img->appendAttribute("src", set1.name + "/" + result.image);

    // second image
    HtmlElement* column2 = content->appendElement("div");
    column2->appendAttribute("class", "img_column");
    column2->appendElement("h3")->appendText("New:");
    img = column2->appendElement("img");
    img->appendAttribute("alt", result.testName + " new");
    img->appendAttribute("src", set2.name + "/" + result.image);

    // summary
    content->appendElement("hr");
    HtmlElement* status = content->appendElement("h3");
    status->appendText("Status: ");
    HtmlElement* span = status->appendElement("span");
    span->appendText(result.passed ? "Passed" : "Failed");
    span->appendAttribute("class", result.passed ? "passed" : "failed");
    content->appendElement("p")->appendText("Frame: " + 
		Ogre::StringConverter::toString(result.frame));

    return container;
}
//-----------------------------------------------------------------------

/** Writes HTML summary of a comparison */
void writeHTML(Ogre::String outputFile, TestImageSet& set1, TestImageSet& set2, std::vector<ComparisonResult>& results)
{
    std::ofstream output;
    output.open(outputFile.c_str());
    if(output.is_open())
    {
        // just dump the doctype in beforehand, since it's formatted strangely
        output<<"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\n\t"
            <<"\"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n";

        // root 'html' tag
        HtmlElement html = HtmlElement("html");
        // add the head
        HtmlElement* head = html.appendElement("head");
        head->appendElement("title")->appendText("OGRE Visual Testing Ouput");
        // link the stylesheet
        HtmlElement* css = head->appendElement("link");
        css->appendAttribute("rel","stylesheet");
        css->appendAttribute("href","output.css");
        css->appendAttribute("type","text/css");
        // add body
        HtmlElement* body = html.appendElement("body");
        // title
        body->appendElement("h1")->appendText("OGRE Visual Test Output");
        // div for summary
        HtmlElement* summaryDiv = body->appendElement("div");
        summaryDiv->appendElement("h2")->appendText("Overall:");
        HtmlElement* contentDiv = summaryDiv->appendElement("div");
        contentDiv->appendAttribute("class", "contentarea");
        contentDiv->appendElement("hr");
        // add info tables about the sets
        contentDiv->pushChild(writeSetInfoTable(set1, "Reference Set:"));
        contentDiv->pushChild(writeSetInfoTable(set2, "Test Set:"));
        contentDiv->appendElement("hr");
        // summarize results
        size_t numPassed = 0;
        for(int i = 0; i < results.size(); ++i)
            if(results[i].passed)
                ++numPassed;
        contentDiv->appendElement("p")->appendText(
            Ogre::StringConverter::toString(numPassed) + " of " 
            + Ogre::StringConverter::toString(results.size()) + " tests passed.");
        contentDiv->appendElement("hr");
        // add thumbnails
        HtmlElement* thumbs = contentDiv->appendElement("p");
        for(int i = 0; i < results.size(); ++i)
        {
            HtmlElement* anchor = thumbs->appendElement("a");
            anchor->appendAttribute("href", Ogre::String("#") + results[i].testName);
            anchor->appendAttribute("title", results[i].testName);
            HtmlElement* img = anchor->appendElement("img");
            img->appendAttribute("src",set2.name + "/" + results[i].image);
            img->appendAttribute("class", results[i].passed ? "thumb" : "thumb_fail");
        }
        // add side-by-side images and summary for each test
        for(int i = 0; i < results.size(); ++i)
            body->pushChild(summarizeSingleResult(results[i], set1, set2));

        // print to the file and close
        output<<html.print();
        output.close();
    }
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

    writeHTML(testDirectory + "out.html", testSet2, testSet1, comparisons);
}

#endif
