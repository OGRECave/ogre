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

#ifndef __OverlayProfileSessionListener_H__
#define __OverlayProfileSessionListener_H__

#include "OgreOverlayPrerequisites.h"
#include "OgreProfiler.h"

namespace Ogre  {
    class Overlay;
    class OverlayContainer;
    class OverlayElement;

    /** \addtogroup Optional
    *  @{
    */
    /** \addtogroup Overlays
    *  @{
    */

    /** Concrete impl. of the ProfileSessionListener which visualizes
        the profling results using overlays.
    */
    class _OgreOverlayExport OverlayProfileSessionListener : public ProfileSessionListener
    {
    public:
        OverlayProfileSessionListener();
        virtual ~OverlayProfileSessionListener();

        /// @see ProfileSessionListener::initializeSession
        virtual void initializeSession();

        /// @see ProfileSessionListener::finializeSession
        virtual void finializeSession();

        /// @see ProfileSessionListener::displayResults
        virtual void displayResults(const ProfileInstance& instance, ulong maxTotalFrameTime);

        /// @see ProfileSessionListener::changeEnableState
        virtual void changeEnableState(bool enabled);

        /** Set the size of the profiler overlay, in pixels. */
        void setOverlayDimensions(Real width, Real height);

        /** Set the position of the profiler overlay, in pixels. */
        void setOverlayPosition(Real left, Real top);

        Real getOverlayWidth() const;
        Real getOverlayHeight() const;
        Real getOverlayLeft() const;
        Real getOverlayTop() const;

    private:
        typedef std::list<OverlayElement*> ProfileBarList;

        /** Prints the profiling results of each frame 
        @remarks Recursive, for all the little children. */
        void displayResults(ProfileInstance* instance, ProfileBarList::const_iterator& bIter, Real& maxTimeMillisecs, Real& newGuiHeight, int& profileCount);

        /** An internal function to create the container which will hold our display elements*/
        OverlayContainer* createContainer();

        /** An internal function to create a text area */
        OverlayElement* createTextArea(const String& name, Real width, Real height, Real top, Real left, 
                                    uint fontSize, const String& caption, bool show = true);

        /** An internal function to create a panel */
        OverlayElement* createPanel(const String& name, Real width, Real height, Real top, Real left, 
                                const String& materialName, bool show = true);

        /// Holds the display bars for each profile results
        ProfileBarList mProfileBars;

        /// The overlay which contains our profiler results display
        Overlay* mOverlay;

        /// The window that displays the profiler results
        OverlayContainer* mProfileGui;

        /// The height of each bar
        Real mBarHeight;

        /// The height of the stats window
        Real mGuiHeight;

        /// The width of the stats window
        Real mGuiWidth;

        /// The horz position of the stats window
        Real mGuiLeft;

        /// The vertical position of the stats window
        Real mGuiTop;

        /// The size of the indent for each profile display bar
        Real mBarIndent;

        /// The width of the border between the profile window and each bar
        Real mGuiBorderWidth;

        /// The width of the min, avg, and max lines in a profile display
        Real mBarLineWidth;

        /// The distance between bars
        Real mBarSpacing;

        /// The max number of profiles we can display
        uint mMaxDisplayProfiles;
    };
}
#endif
