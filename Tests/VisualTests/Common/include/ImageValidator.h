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

#ifndef __ImageValidator_H__
#define __ImageValidator_H__

#include "Ogre.h"

#if OGRE_DOUBLE_PRECISION == 1
#define WITH_FLOAT_SUFFIX(x) x
#else
#define WITH_FLOAT_SUFFIX(x) x##f
#endif

/** Some functionality for comparing images */

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
    float ssim;                     // structural similarity index
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
        std::ifstream file1(Ogre::String(mDirectory1 + "/" + image + ".png").c_str(), std::ios::in | std::ios::binary);
        std::ifstream file2(Ogre::String(mDirectory2 + "/" + image + ".png").c_str(), std::ios::in | std::ios::binary);
        Ogre::DataStreamPtr data1 = Ogre::DataStreamPtr(
            OGRE_NEW Ogre::FileStreamDataStream(&file1, false));
        Ogre::DataStreamPtr data2 = Ogre::DataStreamPtr(
            OGRE_NEW Ogre::FileStreamDataStream(&file2, false));
        Ogre::Image img1 = Ogre::Image();
        Ogre::Image img2 = Ogre::Image();
        img1.load(data1, "png");
        img2.load(data2, "png");

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
        // This computes the MSE, PSNR and SSIM for the two images

        out.incorrectPixels = 0;
        Ogre::ColourValue disparity = Ogre::ColourValue(0,0,0);
        Ogre::Real ssim = 0.0;

        int width = img1.getWidth();
        int height = img1.getHeight();

        // iterate through in 8x8 chunks, so we can calc SSIM at the same time as MSE
        for(int i = 0; i < width / 8; ++i)
        {
            for(int j = 0; j < height / 8; ++j)
            {
                // number of pixels processed
                int n = 0;
                // dynamic range (just 0.0-1.0, since we're using floats)
                Ogre::Real L = 1.f;
                // constants
                Ogre::Real c1 = (0.01f * L)*(0.01f * L);
                Ogre::Real c2 = (0.03f * L)*(0.03f * L);
                // averages
                Ogre::Real avg_x = 0.f;
                Ogre::Real avg_y = 0.f;
                // variances
                Ogre::Real var_x = 0.f;
                Ogre::Real var_y = 0.f;
                // covariance
                Ogre::Real covar = 0.f;

                // iterate through the 8x8 window
                for(int k = 0; k < 8 && i * 8 + k < width; ++k)
                {
                    for(int l = 0; l < 8 && j * 8 + l < height; ++l)
                    {
                        ++n;
                        
                        Ogre::ColourValue col1 = img1.getColourAt(i*8 + k, j*8 + l, 0);
                        Ogre::ColourValue col2 = img2.getColourAt(i*8 + k, j*8 + l, 0);

                        if(col1 != col2)
                        {
                            ++out.incorrectPixels;
                            disparity += (col1 - col2) * (col1 - col2);
                        }

                        // calculations for SSIM:
                        // we'll be working with the luminosity for SSIM (computed by standard Rec. 709 definition)
                        Ogre::Real lum1 = 0.2126f * col1.r + 0.7152f * col1.g + 0.0722f * col1.b;
                        Ogre::Real lum2 = 0.2126f * col2.r + 0.7152f * col2.g + 0.0722f * col2.b;
                        Ogre::Real delta_x = lum1 - avg_x;
                        Ogre::Real delta_y = lum2 - avg_y;
                        avg_x += delta_x/(k*8+l+1);
                        avg_y += delta_y/(k*8+l+1);
                        var_x += delta_x * (lum1 - avg_x);
                        var_y += delta_y * (lum2 - avg_y);
                        covar += lum1 * lum2;                    
                    }
                }

                // more SSIM stuff:
                var_x = var_x/n;
                var_y = var_y/n;
                covar = covar/n - avg_x * avg_y;

                // calculation based on: Z. Wang, A. C. Bovik, H. R. Sheikh and E. P. Simoncelli, 
                // "Image quality assessment: From error visibility to structural 
                // similarity," IEEE Transactions on Image Processing, vol. 13,
                // no. 4, pp. 600-612, Apr. 2004.
                ssim += ((2 * avg_x * avg_y + c1) * (2 * covar + c2)) /
                        ((avg_x*avg_x + avg_y*avg_y + c1) * (var_x + var_y + c2));            
            }
        }

        // only bother with these calculations if the images aren't identical
        if(out.incorrectPixels != 0)
        {
            // average and clamp to [-1,1]
            out.ssim = std::max(WITH_FLOAT_SUFFIX(-1.0),
                                std::min(WITH_FLOAT_SUFFIX(1.0), ssim/(width*height/WITH_FLOAT_SUFFIX(64.))));

            // average the raw deviance value to get MSE
            out.mseChannels = disparity / (width*height);
            out.mse = (out.mseChannels.r + out.mseChannels.g + out.mseChannels.b) / 3.f;

            // PSNR = 20 * log10(range/sqrt(mse))
            for(int i = 0; i < 3; ++i)
                out.psnrChannels[i] = 20 * log10(1.f / sqrt(out.mseChannels[i]));
            out.psnr = 20 * log10(1.f / sqrt(out.mse));
        }

        out.passed = out.incorrectPixels == 0 || out.ssim > 0.999;
    }

private:

    Ogre::String mDirectory1;
    Ogre::String mDirectory2;

};

#endif
