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

#ifndef __HTMLWriter_H__
#define __HTMLWriter_H__

#include "Ogre.h"
#include "TinyHTML.h"
#include "TestResultWriter.h"

class HtmlWriter : public TestResultWriter
{
public:

    HtmlWriter(const TestBatch& set1, const TestBatch& set2, ComparisonResultVectorPtr results)
        :TestResultWriter(set1, set2, results){}

protected:

    virtual Ogre::String getOutput()
    {
        StringStream output;

        // just dump the doctype in beforehand, since it's formatted strangely
        output<<"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\n\t"
            <<"\"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n";

        // root 'html' tag
        HtmlElement html = HtmlElement("html");
        
        // add the head
        HtmlElement* head = html.appendElement("head");
        head->appendElement("title")->appendText("OGRE Visual Testing Output");

        // link the stylesheet
        HtmlElement* css = head->appendElement("link");
        css->appendAttribute("rel","stylesheet");

        // Hosted as an attachment on the wiki
        css->appendAttribute("href","http://www.ogre3d.org/tikiwiki/tiki-download_wiki_attachment.php?attId=142");
        css->appendAttribute("type","text/css");
        
        // link a little javascript
        HtmlElement* js = head->appendElement("script");
        js->appendAttribute("src","http://www.ogre3d.org/tikiwiki/tiki-download_wiki_attachment.php?attId=143");
        js->appendText("");// so it doesn't self close
        
        // </head>

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
        contentDiv->pushChild(writeBatchInfoTable(mSet1, "Reference Set:"));
        contentDiv->pushChild(writeBatchInfoTable(mSet2, "Test Set:"));
        contentDiv->appendElement("hr");

        // summarize results

        // tests may include multiple images
        size_t numPassed = 0;
        size_t numTests = 0;

        for (unsigned int i = 0; i < mResults->size(); ++i)
        {
            ++numTests;
            Ogre::String testName = (*mResults)[i].testName;
            bool passed = true;
            unsigned int j = i;

            for (; j < mResults->size() && (*mResults)[j].testName == testName; ++j)
            {
                if(!(*mResults)[j].passed)
                    passed = false;
            }

            i = j - 1;

            if(passed)
                ++numPassed;
        }

        contentDiv->appendElement("h3")->appendText(
            Ogre::StringConverter::toString(numPassed) + " of " 
            + Ogre::StringConverter::toString(numTests) + " tests passed.");
        contentDiv->appendElement("hr");
        
        // add thumbnails
        HtmlElement* thumbs = contentDiv->appendElement("p");
        for (unsigned int i = 0; i < mResults->size(); ++i)
        {
            HtmlElement* anchor = thumbs->appendElement("a");
            anchor->appendAttribute("href", Ogre::String("#") + (*mResults)[i].testName);
            anchor->appendAttribute("title", (*mResults)[i].testName);
            HtmlElement* img = anchor->appendElement("img");
            img->appendAttribute("src",mSet2.name + "/" + (*mResults)[i].image);
            img->appendAttribute("class", (*mResults)[i].passed ? "thumb" : "thumb_fail");
        }

        // add side-by-side images and summary for each test
        for (unsigned int i = 0; i < mResults->size(); ++i)
        {
            
            // since a test can have multiple images, we find all images with this tets name
            Ogre::String testName = (*mResults)[i].testName;
            bool passed = true;
            unsigned int j = i;
            std::vector<ComparisonResult*> results;

            for (; j < mResults->size() && (*mResults)[j].testName == testName; ++j)
            {
                results.push_back(&(*mResults)[j]);
                if(!(*mResults)[j].passed)
                    passed = false;
            }

            i = j - 1;

            body->pushChild(summarizeSingleResult(results, passed, mSet1, mSet2));
        }

        // print to the stream and return
        output<<html.print();
        return output.str();
    }
    //-----------------------------------------------------------------------

    /** Summarizes the results of a single test (side-by-side images, pass/fail,
     *    more stats and such to come...). Returns an html div with summary markup */
    HtmlElement* summarizeSingleResult(const std::vector<ComparisonResult*>& result, bool passed, const TestBatch& set1, const TestBatch& set2)
    {
        // container and header
        HtmlElement* container = OGRE_NEW HtmlElement("div");
        container->appendAttribute("id",result[0]->testName);
        container->appendElement("h2")->appendText(result[0]->testName);
            //+ " (frame " + Ogre::StringConverter::toString(result.frame) + ")");
        HtmlElement* content = container->appendElement("div");
        // if failed, we give it a different class, and make it red
        content->appendAttribute("class", Ogre::String("contentarea") 
            + (passed ? "" : " failed_test"));

        // summary
        content->appendElement("hr");
        HtmlElement* status = content->appendElement("h3");
        status->appendText("Status: ");
        HtmlElement* span = status->appendElement("span");
        span->appendText(passed ? "Passed" : "Failed");
        span->appendAttribute("class", passed ? "passed" : "failed");

        // if more than one image for this test, and it failed, report how many passed
        if(!passed && result.size() > 1)
        {
            int p = 0;

            for(unsigned int i = 0; i < result.size(); ++i)
                if(result[i]->passed)
                    ++p;

            content->appendElement("h4")->appendText(
                Ogre::StringConverter::toString(p) + " of " + 
                Ogre::StringConverter::toString(result.size()) + " images passed.");
        }

        // loop over images
        for(unsigned int i = 0; i < result.size(); ++i)
        {
            // add a divider
            content->appendElement("hr");

            // add a frame label if more than one image
            if(result.size() > 1)
                content->appendElement("h4")->appendText("Frame " + Ogre::StringConverter::toString(result[i]->frame) + ":");

            HtmlElement* imageBox = content->appendElement("div");

            // first image
            HtmlElement* column1 = imageBox->appendElement("div");
            column1->appendAttribute("class", Ogre::String("img_column") + (result[i]->passed ? "" : " failed_test"));
            column1->appendElement("h3")->appendText("Original:");
            HtmlElement* img = column1->appendElement("img");
            img->appendAttribute("alt", result[i]->testName + Ogre::StringConverter::toString(result[i]->frame) + " original");
            img->appendAttribute("src", set1.name + "/" + result[i]->image);

            // second image
            HtmlElement* column2 = imageBox->appendElement("div");
            column2->appendAttribute("class", Ogre::String("img_column") + (result[i]->passed ? "" : " failed_test"));
            column2->appendElement("h3")->appendText("New:");
            img = column2->appendElement("img");
            img->appendAttribute("alt", result[i]->testName + Ogre::StringConverter::toString(result[i]->frame) + " new");
            img->appendAttribute("src", set2.name + "/" + result[i]->image);

            imageBox->appendElement("h4")->appendText("Comparison Summary:");
            
            if(result[i]->incorrectPixels)
            {
                HtmlElement* absDiff = imageBox->appendElement("p");
                absDiff->appendAttribute("class", "diffreport");
                absDiff->appendText(Ogre::StringConverter::toString(result[i]->incorrectPixels) +
                    " pixels differed.");

                HtmlElement* mse = imageBox->appendElement("p");
                mse->appendAttribute("class", "diffreport");
                mse->appendElement("strong")->appendText(" MSE | ");
                mse->appendText("Overall: " + formatFloat(result[i]->mse) + " | ");
                mse->appendText("R: " + formatFloat(result[i]->mseChannels.r) + " | ");
                mse->appendText("G: " + formatFloat(result[i]->mseChannels.g) + " | ");
                mse->appendText("B: " + formatFloat(result[i]->mseChannels.b) + " |");

                HtmlElement* psnr = imageBox->appendElement("p");
                psnr->appendAttribute("class", "diffreport");
                psnr->appendElement("strong")->appendText("PSNR| ");
                psnr->appendText("Overall: " + formatFloat(result[i]->psnr) + " | ");
                psnr->appendText("R: " + formatFloat(result[i]->psnrChannels.r) + " | ");
                psnr->appendText("G: " + formatFloat(result[i]->psnrChannels.g) + " | ");
                psnr->appendText("B: " + formatFloat(result[i]->psnrChannels.b) + " |");

                HtmlElement* ssim = imageBox->appendElement("p");
                ssim->appendAttribute("class", "diffreport");
                ssim->appendText("Structural Similarity Index: " + formatFloat(result[i]->ssim));
            }
            else
            {
                imageBox->appendElement("p")->appendText("Images are identical.");
            }
        }

        return container;
    }
    //-----------------------------------------------------------------------

    /** Writes a table with some info about a test batch
     *        @param set The set 
     *        @param name The name to use in the header above the table */
    HtmlElement* writeBatchInfoTable(const TestBatch& set, Ogre::String name)
    {
        // main div
        HtmlElement* column = OGRE_NEW HtmlElement("div");
        column->appendAttribute("class", "img_column");

        // add a bit of header text
        column->appendElement("h3")->appendText(name);

        // make the table, and rows for each stat
        HtmlElement* table = column->appendElement("table");
        HtmlElement* row = table->appendElement("tr");
        row->appendElement("th")->appendText("Name:");
        row->appendElement("td")->appendText(set.name);
        row = table->appendElement("tr");
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

    // helper that formats a float nicely for output
    static Ogre::String formatFloat(float num, unsigned int length=6)
    {
        std::stringstream ss;
        ss.setf(std::ios::fixed, std::ios::floatfield);
        ss.setf(std::ios::showpoint);
        ss.precision(6);
        ss<<num;
        Ogre::String out = "";
        ss>>out;
        out = out.substr(0, length);
        if(out.size() < length)
            while(out.size() < length)
                out += "0";
        return out;
    }    
    //-----------------------------------------------------------------------
};

#endif
