
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

#ifndef __CppUnitResultWriter_H__
#define __CppUnitResultWriter_H__

#include "Ogre.h"
#include "TinyHTML.h"
#include "ImageValidator.h"
#include "TestBatch.h"
#include "TestResultWriter.h"

/** Writes a simple plain text file with pass/fail result for each test */
class CppUnitResultWriter : public TestResultWriter
{
public:

    CppUnitResultWriter(const TestBatch& set1, const TestBatch& set2, const ComparisonResultVector& results)
        :TestResultWriter(set1, set2, results){}

protected:

    virtual Ogre::String getOutput()
    {
        StringStream out;
        int numFailed = 0;
        out << "<?xml version=\"1.0\" encoding='ISO-8859-1' standalone='yes' ?>" << std::endl;
        out << "<TestRun>" << std::endl;

        out << "  <FailedTests>" << std::endl;
        for(size_t i = 0; i < mResults.size(); ++i)
        {
            Ogre::String test = mResults[i].testName;
            size_t j = i;

            // a test may have multiple images, so we check all, and fail the whole test if any one fails
            for(; j < mResults.size() && mResults[j].testName == test; ++j)
            {
                if(!mResults[j].passed)
                {
                    numFailed++;

                    // Start counting at 10000 to make sure that they are unique test id's.
                    out << "    <FailedTest id=\"" << i + 10000 << "\">" << std::endl;
                    out << "      <Name>VisualTests::test" << test << "</Name>" << std::endl;
                    out << "      <FailureType>Error</FailureType>" << std::endl;
                    out << "      <Message>Visual Test Image Mismatch</Message>" << std::endl;
                    out << "    </FailedTest>" << std::endl;
                }
            }

            i = j - 1;
        }
        out << "  </FailedTests>" << std::endl;

        out << "  <SuccessfulTests>" << std::endl;
        for(size_t i = 0; i < mResults.size(); ++i)
        {
            Ogre::String test = mResults[i].testName;
            size_t j = i;

            // a test may have multiple images, so we check all, and fail the whole test if any one fails
            for(; j < mResults.size() && mResults[j].testName == test; ++j)
            {
                if(mResults[j].passed)
                {
                    // Start counting at 10000 to make sure that they are unique test id's.
                    out << "    <Test id=\"" << i + 10000 << "\">" << std::endl;
                    out << "      <Name>VisualTests::test" << test << "</Name>" << std::endl;
                    out << "    </Test>" << std::endl;
                }
            }

            i = j - 1;
        }
        out << "  </SuccessfulTests>" << std::endl;

        out << "  <Statistics>" << std::endl;
        out << "    <Tests>" << mResults.size() << "</Tests>" << std::endl;
        out << "    <FailuresTotal>" << numFailed << "</FailuresTotal>" << std::endl;
        out << "    <Errors>" << numFailed << "</Errors>" << std::endl;
        out << "    <Failures>0</Failures>" << std::endl;
        out << "  </Statistics>" << std::endl;
        out << "</TestRun>" << std::endl;

        return out.str();
    }
};

#endif
