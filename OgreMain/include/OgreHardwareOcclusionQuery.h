/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#ifndef _HardwareOcclusionQuery__
#define _HardwareOcclusionQuery__

// Precompiler options
#include "OgrePrerequisites.h"

namespace Ogre {



    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup RenderSystem
    *  @{
    */
/**
  * Query how many pixels have passed the per-fragment tests.
  *
  * Create one OcclusionQuery per outstanding query or one per tested object
  *
  * Then, in the rendering loop:
  * 1. Draw all occluders
  * 2. @ref begin()
  * 3. Draw the polygons to be tested
  * 4. @ref end()
  *
  * Results must be pulled using @ref waitForResult()
  */
class _OgreExport HardwareOcclusionQuery : public RenderSysAlloc
{
public:
    HardwareOcclusionQuery();

    virtual ~HardwareOcclusionQuery();

    /**
      * Starts the hardware occlusion query
      */
    void begin() { beginOcclusionQuery(); }
    virtual void beginOcclusionQuery() = 0;

    /**
      * Ends the hardware occlusion test
      */
    void end() { endOcclusionQuery(); }
    virtual void endOcclusionQuery() = 0;

    /**
      * Waits until the query result is available.
      * use @ref resultReady() if just want to test if the result is available.
      * @retval result will get the resulting number of fragments.
      * @return True if success or false if not.
      */
    bool waitForResult(unsigned int* result) { return pullOcclusionQuery(result); }
    virtual bool pullOcclusionQuery(unsigned int* result) = 0;

    /**
      * Let's you get the last pixel count with out doing the hardware occlusion test.
      * This function won't give you new values, just the old value.
      * @return The last fragment count from the last test.
      */
    uint32 getLastResult() const { return mPixelCount; }
    OGRE_DEPRECATED uint32 getLastQuerysPixelcount() const { return getLastResult(); }

    /**
      * Lets you know when query is done, or still be processed by the Hardware
      * @return true if query is finished.
      */
    bool resultReady() { return !isStillOutstanding(); }
    virtual bool isStillOutstanding(void) = 0;

    protected:
        /// Number of visible pixels determined by last query
        uint32 mPixelCount;
        /// Has the query returned a result yet?
        bool         mIsQueryResultStillOutstanding;
};

    /** @} */
    /** @} */
}
#endif

