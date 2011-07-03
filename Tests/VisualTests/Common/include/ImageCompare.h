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
    unsigned int frame;

	// various metrics of image difference
    unsigned int incorrectPixels;
	float mse;                      // mean squared error
	Ogre::ColourValue mseChannels;
	float psnr;                     // peak signal-to-noise ratio
	Ogre::ColourValue psnrChannels;
};

typedef std::vector<ComparisonResult> ComparisonResultVector;
typedef Ogre::SharedPtr<ComparisonResultVector> ComparisonResultVectorPtr;

/** Simple object for doing image comparison between two image sets */
class ImageValidator
{
public:

    /** Constructor, takes paths to each set's directory */
    ImageValidator(Ogre::String directory1, Ogre::String directory2)
        :mDirectory1(directory1),mDirectory2(directory2) {}

    /** Compare the the two set's versions of the specified image name
     *        @param name filename of the image (filename only, no path) */
    ComparisonResult compare(Ogre::String image)
    {
        ComparisonResult out;
        out.image = image + ".png";

        // extract test name and frame from image filename
        size_t end = image.find_last_of("_");
        out.testName = image.substr(0, end);
        out.frame = atoi(image.substr(end+1).c_str());

        // load manually, so this can be done without all of Ogre initialized (i.e. for a 
        // command line utility for comparing test sets or something to that effect)
        // The FreeImage codecs must be loaded for this to work, but no resource stuff is needed.
        std::ifstream file1(Ogre::String(mDirectory1 + "/" + image + ".png").c_str());
        std::ifstream file2(Ogre::String(mDirectory2 + "/" + image + ".png").c_str());
        Ogre::DataStreamPtr data1 = Ogre::DataStreamPtr(
            OGRE_NEW Ogre::FileStreamDataStream(&file1, false));
        Ogre::DataStreamPtr data2 = Ogre::DataStreamPtr(
            OGRE_NEW Ogre::FileStreamDataStream(&file2, false));
        Ogre::Image img1 = Ogre::Image();
        Ogre::Image img2 = Ogre::Image();
        img1.load(data1);
        img2.load(data2);

        // do the actual comparison
        compare(img1,img2,out);

        return out;
    }


protected:

    /** Do the actual comparison, override this if you wish to do some
     *    alternate method of comparison 
     *        @param img1 The image data for the first image 
     *        @param img2 The image data for the second image
     *        @param out The struct we'll write results to */
    virtual void compare(const Ogre::Image& img1, const Ogre::Image& img2,
        ComparisonResult& out)
    {
        out.incorrectPixels = 0;
        Ogre::ColourValue disparity = Ogre::ColourValue(0,0,0);

		int width = img1.getWidth();
		int height = img1.getHeight();

        // just go pixel by pixel and keep track of how many are different
        for(int i = 0; i < width; ++i)
        {
            for(int j = 0; j < height; ++j)
            {
                Ogre::ColourValue c1 = img1.getColourAt(i, j, 0);
                Ogre::ColourValue c2 = img2.getColourAt(i, j, 0);
                if(c1 != c2)
                {
                    ++out.incorrectPixels;
					disparity += (c1 + c2) * (c1 + c2);
                }
            }
        }
        
		// only bother with these calculations if the images aren't identical
        if(out.incorrectPixels != 0)
        {
			out.mseChannels = disparity / (width*height);
			out.mse = (out.mseChannels.r + out.mseChannels.g + out.mseChannels.b) / 3.f;
			for(int i = 0; i < 3; ++i)
				out.psnrChannels[i] = 20 * log10(1.f / sqrt(out.mseChannels[i]));
			out.psnr = 20 * log10(1.f / sqrt(out.mse));
        }

        out.passed = out.incorrectPixels == 0;
    }

private:

    Ogre::String mDirectory1;
    Ogre::String mDirectory2;

};

#endif
