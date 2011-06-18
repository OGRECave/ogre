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

/** Some functionality for comparing individual images */

/* Results of comparing two test images */
struct ComparisonResult
{
    bool passed;
    Ogre::String image;
    Ogre::String testName;
    size_t frame;
};

typedef std::vector<ComparisonResult> ComparisonResultVector;
typedef Ogre::SharedPtr<ComparisonResultVector> ComparisonResultVectorPtr;

/** Compares two images 
 *        @param image1 Full path to the first image
 *        @param image2 Full path to the second image */
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

#endif
