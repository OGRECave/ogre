
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

#ifndef __SimpleResultWriter_H__
#define __SimpleResultWriter_H__

#include "Ogre.h"
#include "TinyHTML.h"
#include "ImageValidator.h"
#include "TestBatch.h"
#include "TestResultWriter.h"

/** Writes a simple plain text file with pass/fail result for each test */
class SimpleResultWriter : public TestResultWriter
{
public:

    SimpleResultWriter(const TestBatch& set1, const TestBatch& set2, ComparisonResultVectorPtr results)
        :TestResultWriter(set1, set2, results){}

protected:

    virtual Ogre::String getOutput()
    {
        StringStream out;

        for(size_t i = 0; i < mResults->size(); ++i)
        {
            Ogre::String test = (*mResults)[i].testName;
            bool passed = true;
            
            size_t j = i;

            // a test may have multiple images, so we check all, and fail the whole test if any one fails
            for(; j < mResults->size() && (*mResults)[j].testName == test; ++j)
            {
                if(!(*mResults)[j].passed)
                    passed = false;
            }

            i = j - 1;

            out << test << "=" << (passed ? "Passed" : "Failed") << "\n";
        }

        return out.str();
    }
};

#endif
