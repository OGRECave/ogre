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

#ifndef __TestResults_H__
#define __TestResults_H__

#include "Ogre.h"
#include "tinyhtml.h"
#include "ImageCompare.h"
#include "TestBatch.h"

/** A collection of functions for outputting comparison results */

/** Writes a table with some info about a test batch
 *        @param set The set 
 *        @param name The name to use in the header above the table */
HtmlElement* writeBatchInfoTable(const TestBatch& set, Ogre::String name)
{
    // main div
    HtmlElement* column = new HtmlElement("div");
    column->appendAttribute("class", "img_column");
    // add a bit of header text
    column->appendElement("h3")->appendText(name);
    // make the table, and rows for each stat
    HtmlElement* table = column->appendElement("table");
    HtmlElement* row = table->appendElement("tr");
    row->appendElement("th")->appendText("Time:");
    row->appendElement("td")->appendText(set.timestamp);
    row = table->appendElement("tr");
    row->appendElement("th")->appendText("Version:");
    row->appendElement("td")->appendText(set.version);
    row = table->appendElement("tr");
    row->appendElement("th")->appendText("Resolution:");
    row->appendElement("td")->appendText(Ogre::StringConverter::toString(set.resolutionX) 
        + " x " + Ogre::StringConverter::toString(set.resolutionY));
    row = table->appendElement("tr");
    row->appendElement("th")->appendText("Comment:");
    row->appendElement("td")->appendText(set.comment);
    // return the whole thing, ready to be attached into a larger document
    return column;
}
//-----------------------------------------------------------------------

/** Summarizes the results of a single test (side-by-side images, pass/fail,
 *    more stats and such to come...). Returns an html div with summary markup */
HtmlElement* summarizeSingleResult(ComparisonResult& result, const TestBatch& set1, const TestBatch& set2)
{
    // container and header
    HtmlElement* container = new HtmlElement("div");
    container->appendAttribute("id",result.testName);
    container->appendElement("h2")->appendText(result.testName 
        + " (frame " + Ogre::StringConverter::toString(result.frame) + ")");
    HtmlElement* content = container->appendElement("div");
    // if failed, we give it a different class, and make it red
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

    return container;
}
//-----------------------------------------------------------------------

/** Gets HTML summary of a comparison (as a string) */
Ogre::String writeHTML(const TestBatch& set1, const TestBatch& set2, ComparisonResultVectorPtr results)
{
    std::stringstream output;

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
    // For the moment it's hosted on my personal site, for convenience
    css->appendAttribute("href","http://www.rileyadams.net/gsoc/output.css");
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
    contentDiv->pushChild(writeBatchInfoTable(set1, "Reference Set:"));
    contentDiv->pushChild(writeBatchInfoTable(set2, "Test Set:"));
    contentDiv->appendElement("hr");
    // summarize results
    size_t numPassed = 0;
    for(int i = 0; i < results->size(); ++i)
        if((*results)[i].passed)
            ++numPassed;
    contentDiv->appendElement("p")->appendText(
        Ogre::StringConverter::toString(numPassed) + " of " 
        + Ogre::StringConverter::toString(results->size()) + " tests passed.");
    contentDiv->appendElement("hr");
    // add thumbnails
    HtmlElement* thumbs = contentDiv->appendElement("p");
    for(int i = 0; i < results->size(); ++i)
    {
        HtmlElement* anchor = thumbs->appendElement("a");
        anchor->appendAttribute("href", Ogre::String("#") + (*results)[i].testName);
        anchor->appendAttribute("title", (*results)[i].testName);
        HtmlElement* img = anchor->appendElement("img");
        img->appendAttribute("src",set2.name + "/" + (*results)[i].image);
        img->appendAttribute("class", (*results)[i].passed ? "thumb" : "thumb_fail");
    }
    // add side-by-side images and summary for each test
    for(int i = 0; i < results->size(); ++i)
        body->pushChild(summarizeSingleResult((*results)[i], set1, set2));

    // print to the stream and return
    output<<html.print();
    return output.str();
}
//-----------------------------------------------------------------------

#endif
