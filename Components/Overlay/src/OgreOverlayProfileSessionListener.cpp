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

#include "OgreOverlayProfileSessionListener.h"
#include "OgreOverlayManager.h"
#include "OgreOverlayContainer.h"
#include "OgreOverlayElement.h"
#include "OgreOverlay.h"
#include "OgreStringConverter.h"

namespace Ogre
{
    //-----------------------------------------------------------------------
    OverlayProfileSessionListener::OverlayProfileSessionListener() 
        : mOverlay(0)
        , mProfileGui(0)
        , mBarHeight(10)
        , mGuiHeight(25)
        , mGuiWidth(250)
        , mGuiLeft(0)
        , mGuiTop(0)
        , mBarIndent(250)
        , mGuiBorderWidth(10)
        , mBarLineWidth(2)
        , mBarSpacing(3)
        , mMaxDisplayProfiles(50)
        , mDisplayMode(DISPLAY_MILLISECONDS)
    {
    }
    //-----------------------------------------------------------------------
    OverlayProfileSessionListener::~OverlayProfileSessionListener()
    {
        mProfileBars.clear();
    }
    //-----------------------------------------------------------------------
    void OverlayProfileSessionListener::initializeSession()
    {
        // create a new overlay to hold our Profiler display
        mOverlay = OverlayManager::getSingleton().create("Profiler");
        mOverlay->setZOrder(500);

        // this panel will be the main container for our profile bars
        mProfileGui = createContainer();

        // we create an initial pool of 50 profile bars
        for (uint i = 0; i < mMaxDisplayProfiles; ++i) 
        {
            // this is for the profile name and the number of times it was called in a frame
            OverlayElement* element = createTextArea("profileText" + StringConverter::toString(i), 90, mBarHeight, mGuiBorderWidth + (mBarHeight + mBarSpacing) * i, 0, 14, "", false);
            mProfileGui->addChild(element);
            mProfileBars.push_back(element);

            // this indicates the current frame time
            element = createPanel("currBar" + StringConverter::toString(i), 0, mBarHeight, mGuiBorderWidth + (mBarHeight + mBarSpacing) * i, mBarIndent, "Core/ProfilerCurrent", false);
            mProfileGui->addChild(element);
            mProfileBars.push_back(element);

            // this indicates the minimum frame time
            element = createPanel("minBar" + StringConverter::toString(i), mBarLineWidth, mBarHeight, mGuiBorderWidth + (mBarHeight + mBarSpacing) * i, 0, "Core/ProfilerMin", false);
            mProfileGui->addChild(element);
            mProfileBars.push_back(element);

            // this indicates the maximum frame time
            element = createPanel("maxBar" + StringConverter::toString(i), mBarLineWidth, mBarHeight, mGuiBorderWidth + (mBarHeight + mBarSpacing) * i, 0, "Core/ProfilerMax", false);
            mProfileGui->addChild(element);
            mProfileBars.push_back(element);

            // this indicates the average frame time
            element = createPanel("avgBar" + StringConverter::toString(i), mBarLineWidth, mBarHeight, mGuiBorderWidth + (mBarHeight + mBarSpacing) * i, 0, "Core/ProfilerAvg", false);
            mProfileGui->addChild(element);
            mProfileBars.push_back(element);

            // this indicates the text of the frame time
            element = createTextArea("statText" + StringConverter::toString(i), 20, mBarHeight, mGuiBorderWidth + (mBarHeight + mBarSpacing) * i, 0, 14, "", false);
            mProfileGui->addChild(element);
            mProfileBars.push_back(element);
        }

        // throw everything all the GUI stuff into the overlay and display it
        mOverlay->add2D(mProfileGui);
    }
    //-----------------------------------------------------------------------
    void OverlayProfileSessionListener::finializeSession()
    {
        OverlayContainer* container = dynamic_cast<OverlayContainer*>(mProfileGui);
        if (container)
        {
            for (const auto& p : container->getChildren())
            {
                OverlayElement* element = p.second;
                OverlayContainer* parent = element->getParent();
                if (parent) parent->removeChild(element->getName());
                OverlayManager::getSingleton().destroyOverlayElement(element);
            }
        }
        if(mProfileGui)
            OverlayManager::getSingleton().destroyOverlayElement(mProfileGui);
        if(mOverlay)
            OverlayManager::getSingleton().destroy(mOverlay);           
            
        mProfileBars.clear();
    }
    //-----------------------------------------------------------------------
    void OverlayProfileSessionListener::displayResults(const ProfileInstance& root, ulong maxTotalFrameTime)
    {
        Real newGuiHeight = mGuiHeight;
        int profileCount = 0;
        Real maxTimeMillisecs = (Real)maxTotalFrameTime / 1000.0f;

        ProfileBarList::const_iterator bIter = mProfileBars.begin();
        ProfileInstance::ProfileChildren::const_iterator it = root.children.begin(), endit = root.children.end();
        for(;it != endit; ++it)
        {
            ProfileInstance* child = it->second;
            displayResults(child, bIter, maxTimeMillisecs, newGuiHeight, profileCount);
        }
            
        // set the main display dimensions
        mProfileGui->setMetricsMode(GMM_PIXELS);
        mProfileGui->setHeight(newGuiHeight);
        mProfileGui->setWidth(mGuiWidth * 2 + 15);
        mProfileGui->setTop(5);
        mProfileGui->setLeft(5);

        // we hide all the remaining pre-created bars
        for (; bIter != mProfileBars.end(); ++bIter) 
        {
            (*bIter)->hide();
        }
    }
    //-----------------------------------------------------------------------
    void OverlayProfileSessionListener::displayResults(ProfileInstance* instance, ProfileBarList::const_iterator& bIter, Real& maxTimeMillisecs, Real& newGuiHeight, int& profileCount)
    {
        OverlayElement* g;

        // display the profile's name and the number of times it was called in a frame
        g = *bIter;
        ++bIter;
        g->show();
        g->setCaption(String(instance->name + " (" + StringConverter::toString(instance->history.numCallsThisFrame) + ")"));
        g->setLeft(10 + instance->hierarchicalLvl * 15.0f);


        // display the main bar that show the percentage of the frame time that this
        // profile has taken
        g = *bIter;
        ++bIter;
        g->show();
        // most of this junk has been set before, but we do this to get around a weird
        // Ogre gui issue (bug?)
        g->setMetricsMode(GMM_PIXELS);
        g->setHeight(mBarHeight);

        if (mDisplayMode == DISPLAY_PERCENTAGE)
            g->setWidth( (instance->history.currentTimePercent) * mGuiWidth);
        else
            g->setWidth( (instance->history.currentTimeMillisecs / maxTimeMillisecs) * mGuiWidth);

        g->setLeft(mGuiWidth);
        g->setTop(mGuiBorderWidth + profileCount * (mBarHeight + mBarSpacing));



        // display line to indicate the minimum frame time for this profile
        g = *bIter;
        ++bIter;
        g->show();
        if(mDisplayMode == DISPLAY_PERCENTAGE)
            g->setLeft(mBarIndent + instance->history.minTimePercent * mGuiWidth);
        else
            g->setLeft(mBarIndent + (instance->history.minTimeMillisecs / maxTimeMillisecs) * mGuiWidth);

        // display line to indicate the maximum frame time for this profile
        g = *bIter;
        ++bIter;
        g->show();
        if(mDisplayMode == DISPLAY_PERCENTAGE)
            g->setLeft(mBarIndent + instance->history.maxTimePercent * mGuiWidth);
        else
            g->setLeft(mBarIndent + (instance->history.maxTimeMillisecs / maxTimeMillisecs) * mGuiWidth);

        // display line to indicate the average frame time for this profile
        g = *bIter;
        ++bIter;
        g->show();
        if(instance->history.totalCalls != 0)
        {
            if (mDisplayMode == DISPLAY_PERCENTAGE)
                g->setLeft(mBarIndent + (instance->history.totalTimePercent / instance->history.totalCalls) * mGuiWidth);
            else
                g->setLeft(mBarIndent + ((instance->history.totalTimeMillisecs / instance->history.totalCalls) / maxTimeMillisecs) * mGuiWidth);
        }
        else
            g->setLeft(mBarIndent);

        // display text
        g = *bIter;
        ++bIter;
        g->show();
        if (mDisplayMode == DISPLAY_PERCENTAGE)
        {
            g->setLeft(mBarIndent + instance->history.currentTimePercent * mGuiWidth + 2);
            g->setCaption(StringConverter::toString(instance->history.currentTimePercent * 100.0f, 3, 3) + "%");
        }
        else
        {
            g->setLeft(mBarIndent + (instance->history.currentTimeMillisecs / maxTimeMillisecs) * mGuiWidth + 2);
            g->setCaption(StringConverter::toString(instance->history.currentTimeMillisecs, 3, 3) + "ms");
        }

        // we set the height of the display with respect to the number of profiles displayed
        newGuiHeight += mBarHeight + mBarSpacing;

        ++profileCount;

        // display children
        ProfileInstance::ProfileChildren::const_iterator it = instance->children.begin(), endit = instance->children.end();
        for(;it != endit; ++it)
        {
            ProfileInstance* child = it->second;
            displayResults(child, bIter, maxTimeMillisecs, newGuiHeight, profileCount);
        }
    }
    //-----------------------------------------------------------------------
    void OverlayProfileSessionListener::changeEnableState(bool enabled) 
    {
        if (enabled) 
        {
            mOverlay->show();
        }
        else 
        {
            mOverlay->hide();
        }
    }
    //-----------------------------------------------------------------------
    OverlayContainer* OverlayProfileSessionListener::createContainer()
    {
        OverlayContainer* container = (OverlayContainer*) 
            OverlayManager::getSingleton().createOverlayElement(
                "BorderPanel", "profiler");
        container->setMetricsMode(GMM_PIXELS);
        container->setMaterialName("Core/StatsBlockCenter");
        container->setHeight(mGuiHeight);
        container->setWidth(mGuiWidth * 2 + 15);
        container->setParameter("border_size", "1 1 1 1");
        container->setParameter("border_material", "Core/StatsBlockBorder");
        container->setParameter("border_topleft_uv", "0.0000 1.0000 0.0039 0.9961");
        container->setParameter("border_top_uv", "0.0039 1.0000 0.9961 0.9961");
        container->setParameter("border_topright_uv", "0.9961 1.0000 1.0000 0.9961");
        container->setParameter("border_left_uv","0.0000 0.9961 0.0039 0.0039");
        container->setParameter("border_right_uv","0.9961 0.9961 1.0000 0.0039");
        container->setParameter("border_bottomleft_uv","0.0000 0.0039 0.0039 0.0000");
        container->setParameter("border_bottom_uv","0.0039 0.0039 0.9961 0.0000");
        container->setParameter("border_bottomright_uv","0.9961 0.0039 1.0000 0.0000");
        container->setLeft(5);
        container->setTop(5);

        return container;
    }
    //-----------------------------------------------------------------------
    OverlayElement* OverlayProfileSessionListener::createTextArea(const String& name, Real width, Real height, Real top, Real left, 
                                         uint fontSize, const String& caption, bool show)
    {
        OverlayElement* textArea = OverlayManager::getSingleton().createOverlayElement("TextArea", name);
        textArea->setMetricsMode(GMM_PIXELS);
        textArea->setWidth(width);
        textArea->setHeight(height);
        textArea->setTop(top);
        textArea->setLeft(left);
        textArea->setParameter("font_name", "SdkTrays/Value");
        textArea->setParameter("char_height", StringConverter::toString(fontSize));
        textArea->setCaption(caption);
        textArea->setParameter("colour_top", "1 1 1");
        textArea->setParameter("colour_bottom", "1 1 1");

        if (show) {
            textArea->show();
        }
        else {
            textArea->hide();
        }

        return textArea;
    }
    //-----------------------------------------------------------------------
    OverlayElement* OverlayProfileSessionListener::createPanel(const String& name, Real width, Real height, Real top, Real left, 
                                      const String& materialName, bool show)
    {
        OverlayElement* panel = 
            OverlayManager::getSingleton().createOverlayElement("Panel", name);
        panel->setMetricsMode(GMM_PIXELS);
        panel->setWidth(width);
        panel->setHeight(height);
        panel->setTop(top);
        panel->setLeft(left);
        panel->setMaterialName(materialName);

        if (show) {
            panel->show();
        }
        else {
            panel->hide();
        }

        return panel;
    }
    //-----------------------------------------------------------------------
    void OverlayProfileSessionListener::setOverlayPosition(Real left, Real top)
    {
        mGuiLeft = left;
        mGuiTop = top;

        mProfileGui->setPosition(left, top);
    }
    //---------------------------------------------------------------------
    Real OverlayProfileSessionListener::getOverlayWidth() const
    {
        return mGuiWidth;
    }
    //---------------------------------------------------------------------
    Real OverlayProfileSessionListener::getOverlayHeight() const
    {
        return mGuiHeight;
    }
    //---------------------------------------------------------------------
    Real OverlayProfileSessionListener::getOverlayLeft() const
    {
        return mGuiLeft;
    }
    //---------------------------------------------------------------------
    Real OverlayProfileSessionListener::getOverlayTop() const
    {
        return mGuiTop;
    }
    //-----------------------------------------------------------------------
    void OverlayProfileSessionListener::setOverlayDimensions(Real width, Real height)
    {
        mGuiWidth = width;
        mGuiHeight = height;
        mBarIndent = mGuiWidth;

        mProfileGui->setDimensions(width, height);
    }
    //-----------------------------------------------------------------------
}
