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
#ifndef __SdkTrays_H__
#define __SdkTrays_H__

#include "OgreBitesPrerequisites.h"
#include "OgreOverlay.h"
#include "OgreOverlaySystem.h"
#include "OgreOverlayManager.h"
#include "OgreBorderPanelOverlayElement.h"
#include "OgreTextAreaOverlayElement.h"
#include "OgreFontManager.h"
#include "OgreTimer.h"
#include "OgreRoot.h"
#include "OgreCamera.h"
#include "OgreRenderWindow.h"
#include "OgreInput.h"

#include <iomanip>

namespace OgreBites
{
    /** \addtogroup Optional
    *  @{
    */
    /** \addtogroup Bites
    *  @{
    */
    /** @defgroup Trays Trays
     * Simplistic GUI System build with Overlays
     * @{
     */
    enum TrayLocation   /// enumerator values for widget tray anchoring locations
    {
        TL_TOPLEFT,
        TL_TOP,
        TL_TOPRIGHT,
        TL_LEFT,
        TL_CENTER,
        TL_RIGHT,
        TL_BOTTOMLEFT,
        TL_BOTTOM,
        TL_BOTTOMRIGHT,
        TL_NONE
    };

    enum ButtonState   /// enumerator values for button states
    {
        BS_UP,
        BS_OVER,
        BS_DOWN
    };

    // forward widget class declarations
    class Widget;
    class Button;
    class SelectMenu;
    class Label;
    class Slider;
    class CheckBox;

    /**
    Listener class for responding to tray events.
    */
    class _OgreBitesExport TrayListener
    {
    public:

        virtual ~TrayListener() {}
        virtual void buttonHit(Button* button) {}
        virtual void itemSelected(SelectMenu* menu) {}
        virtual void labelHit(Label* label) {}
        virtual void sliderMoved(Slider* slider) {}
        virtual void checkBoxToggled(CheckBox* box) {}
        virtual void okDialogClosed(const Ogre::DisplayString& message) {}
        virtual void yesNoDialogClosed(const Ogre::DisplayString& question, bool yesHit) {}
    };

    /**
    Abstract base class for all widgets.
    */
    class _OgreBitesExport Widget
    {
    public:
            
        Widget();

        virtual ~Widget() {}

        void cleanup();

        /**
        Static utility method to recursively delete an overlay element plus
        all of its children from the system.
        */
        static void nukeOverlayElement(Ogre::OverlayElement* element);

        /**
        Static utility method to check if the cursor is over an overlay element.
        */
        static bool isCursorOver(Ogre::OverlayElement* element, const Ogre::Vector2& cursorPos, Ogre::Real voidBorder = 0);

        /**
        Static utility method used to get the cursor's offset from the center
        of an overlay element in pixels.
        */
        static Ogre::Vector2 cursorOffset(Ogre::OverlayElement* element, const Ogre::Vector2& cursorPos);

        /**
        Static utility method used to get the width of a caption in a text area.
        */
        static Ogre::Real getCaptionWidth(const Ogre::DisplayString& caption, Ogre::TextAreaOverlayElement* area);

        /**
        Static utility method to cut off a string to fit in a text area.
        */
        static void fitCaptionToArea(const Ogre::DisplayString& caption, Ogre::TextAreaOverlayElement* area, Ogre::Real maxWidth);

        Ogre::OverlayElement* getOverlayElement()
        {
            return mElement;
        }

        const Ogre::String& getName()
        {
            return mElement->getName();
        }

        TrayLocation getTrayLocation()
        {
            return mTrayLoc;
        }

        void hide()
        {
            mElement->hide();
        }

        void show()
        {
            mElement->show();
        }

        bool isVisible()
        {
            return mElement->isVisible();
        }

        // callbacks

        virtual void _cursorPressed(const Ogre::Vector2& cursorPos) {}
        virtual void _cursorReleased(const Ogre::Vector2& cursorPos) {}
        virtual void _cursorMoved(const Ogre::Vector2& cursorPos, float wheelDelta) {}
        virtual void _focusLost() {}

        // internal methods used by TrayManager. do not call directly.

        void _assignToTray(TrayLocation trayLoc) { mTrayLoc = trayLoc; }
        void _assignListener(TrayListener* listener) { mListener = listener; }

    protected:

        Ogre::OverlayElement* mElement;
        TrayLocation mTrayLoc;
        TrayListener* mListener;
    };

    typedef std::vector<Widget*> WidgetList;

    /**
    Basic button class.
    */
    class _OgreBitesExport Button : public Widget
    {
    public:

        /// Do not instantiate any widgets directly. Use TrayManager.
        Button(const Ogre::String& name, const Ogre::DisplayString& caption, Ogre::Real width);

        virtual ~Button() {}

        const Ogre::DisplayString& getCaption()
        {
            return mTextArea->getCaption();
        }

        void setCaption(const Ogre::DisplayString& caption);

        const ButtonState& getState() { return mState; }

        void _cursorPressed(const Ogre::Vector2& cursorPos) override;

        void _cursorReleased(const Ogre::Vector2& cursorPos) override;

        void _cursorMoved(const Ogre::Vector2& cursorPos, float wheelDelta) override;

        void _focusLost() override;

    protected:

        void setState(const ButtonState& bs);

        ButtonState mState;
        Ogre::BorderPanelOverlayElement* mBP;
        Ogre::TextAreaOverlayElement* mTextArea;
        bool mFitToContents;
    };  

    /**
    Scrollable text box widget.
    */
    class _OgreBitesExport TextBox : public Widget
    {
    public:

        /// Do not instantiate any widgets directly. Use TrayManager.
        TextBox(const Ogre::String& name, const Ogre::DisplayString& caption, Ogre::Real width, Ogre::Real height);

        void setPadding(Ogre::Real padding);

        Ogre::Real getPadding()
        {
            return mPadding;
        }

        const Ogre::DisplayString& getCaption()
        {
            return mCaptionTextArea->getCaption();
        }

        void setCaption(const Ogre::DisplayString& caption)
        {
            mCaptionTextArea->setCaption(caption);
        }

        const Ogre::DisplayString& getText()
        {
            return mText;
        }

        /**
        Sets text box content. Most of this method is for wordwrap.
        */
        void setText(const Ogre::DisplayString& text);

        /**
        Sets text box content horizontal alignment.
        */
        void setTextAlignment(Ogre::TextAreaOverlayElement::Alignment ta);

        void clearText()
        {
            setText("");
        }

        void appendText(const Ogre::DisplayString& text)
        {
            setText(getText() + text);
        }

        /**
        Makes adjustments based on new padding, size, or alignment info.
        */
        void refitContents();

        /**
        Sets how far scrolled down the text is as a percentage.
        */
        void setScrollPercentage(Ogre::Real percentage);

        /**
        Gets how far scrolled down the text is as a percentage.
        */
        Ogre::Real getScrollPercentage()
        {
            return mScrollPercentage;
        }

        /**
        Gets how many lines of text can fit in this window.
        */
        unsigned int getHeightInLines()
        {
            return (unsigned int) ((mElement->getHeight() - 2 * mPadding - mCaptionBar->getHeight() + 5) / mTextArea->getCharHeight());
        }

        void _cursorPressed(const Ogre::Vector2& cursorPos) override;

        void _cursorReleased(const Ogre::Vector2& cursorPos) override
        {
            mDragging = false;
        }

        void _cursorMoved(const Ogre::Vector2& cursorPos, float wheelDelta) override;

        void _focusLost() override
        {
            mDragging = false;  // stop dragging if cursor was lost
        }

    protected:

        /**
        Decides which lines to show.
        */
        void filterLines();

        Ogre::TextAreaOverlayElement* mTextArea;
        Ogre::BorderPanelOverlayElement* mCaptionBar;
        Ogre::TextAreaOverlayElement* mCaptionTextArea;
        Ogre::BorderPanelOverlayElement* mScrollTrack;
        Ogre::PanelOverlayElement* mScrollHandle;
        Ogre::DisplayString mText;
        Ogre::StringVector mLines;
        Ogre::Real mPadding;
        bool mDragging;
        Ogre::Real mScrollPercentage;
        Ogre::Real mDragOffset;
        unsigned int mStartingLine;
    };

    /**
    Basic selection menu widget.
    */
    class _OgreBitesExport SelectMenu : public Widget
    {
    public:

        /// Do not instantiate any widgets directly. Use TrayManager.
        SelectMenu(const Ogre::String& name, const Ogre::DisplayString& caption, Ogre::Real width,
            Ogre::Real boxWidth, size_t maxItemsShown);
        void copyItemsFrom(SelectMenu* other);
        bool isExpanded()
        {
            return mExpanded;
        }

        const Ogre::DisplayString& getCaption()
        {
            return mTextArea->getCaption();
        }

        void setCaption(const Ogre::DisplayString& caption);

        const Ogre::StringVector& getItems()
        {
            return mItems;
        }

        size_t getNumItems()
        {
            return mItems.size();
        }

        void setItems(const Ogre::StringVector& items);

        void addItem(const Ogre::DisplayString& item)
        {
            mItems.push_back(item);
            setItems(mItems);
        }

        void insertItem(size_t index, const Ogre::DisplayString& item)
        {
            mItems.insert(mItems.begin() + index, item);
            setItems(mItems);
        }

        void removeItem(const Ogre::DisplayString& item);

        void removeItem(size_t index);

        void clearItems();

        void selectItem(size_t index, bool notifyListener = true);

        bool containsItem(const Ogre::DisplayString& item);

        void selectItem(const Ogre::DisplayString& item, bool notifyListener = true);

        Ogre::DisplayString getSelectedItem();

        int getSelectionIndex()
        {
            return mSelectionIndex;
        }

        void _cursorPressed(const Ogre::Vector2& cursorPos) override;

        void _cursorReleased(const Ogre::Vector2& cursorPos) override
        {
            mDragging = false;
        }

        void _cursorMoved(const Ogre::Vector2& cursorPos, float wheelDelta) override;

        void _focusLost() override
        {
            if (mExpandedBox->isVisible()) retract();
        }

    protected:

        /**
        Internal method - sets which item goes at the top of the expanded menu.
        */
        void setDisplayIndex(unsigned int index);

        /**
        Internal method - cleans up an expanded menu.
        */
        void retract();

        Ogre::BorderPanelOverlayElement* mSmallBox;
        Ogre::BorderPanelOverlayElement* mExpandedBox;
        Ogre::TextAreaOverlayElement* mTextArea;
        Ogre::TextAreaOverlayElement* mSmallTextArea;
        Ogre::BorderPanelOverlayElement* mScrollTrack;
        Ogre::PanelOverlayElement* mScrollHandle;
        std::vector<Ogre::BorderPanelOverlayElement*> mItemElements;
        size_t mMaxItemsShown;
        size_t mItemsShown;
        bool mCursorOver;
        bool mExpanded;
        bool mFitToContents;
        bool mDragging;
        Ogre::StringVector mItems;
        int mSelectionIndex;
        int mHighlightIndex;
        int mDisplayIndex;
        Ogre::Real mDragOffset;
    };

    /**
    Basic label widget.
    */
    class _OgreBitesExport Label : public Widget
    {
    public:

        /// Do not instantiate any widgets directly. Use TrayManager.
        Label(const Ogre::String& name, const Ogre::DisplayString& caption, Ogre::Real width);

        const Ogre::DisplayString& getCaption()
        {
            return mTextArea->getCaption();
        }

        void setCaption(const Ogre::DisplayString& caption)
        {
            mTextArea->setCaption(caption);
        }

        void _cursorPressed(const Ogre::Vector2& cursorPos) override;

        bool _isFitToTray()
        {
            return mFitToTray;
        }

    protected:

        Ogre::TextAreaOverlayElement* mTextArea;
        bool mFitToTray;
    };

    /**
    Basic separator widget.
    */
    class _OgreBitesExport Separator : public Widget
    {
    public:

        /// Do not instantiate any widgets directly. Use TrayManager.
        Separator(const Ogre::String& name, Ogre::Real width);

        bool _isFitToTray()
        {
            return mFitToTray;
        }

    protected:

        bool mFitToTray;
    };

    /**
    Basic slider widget.
    */
    class _OgreBitesExport Slider : public Widget
    {
    public:

        /// Do not instantiate any widgets directly. Use TrayManager.
        Slider(const Ogre::String& name, const Ogre::DisplayString& caption, Ogre::Real width, Ogre::Real trackWidth,
            Ogre::Real valueBoxWidth, Ogre::Real minValue, Ogre::Real maxValue, unsigned int snaps);

        /**
        Sets the minimum value, maximum value, and the number of snapping points.
        */
        void setRange(Ogre::Real minValue, Ogre::Real maxValue, unsigned int snaps, bool notifyListener = true);

        const Ogre::DisplayString& getValueCaption()
        {
            return mValueTextArea->getCaption();
        }
        
        /**
        You can use this method to manually format how the value is displayed.
        */
        void setValueCaption(const Ogre::DisplayString& caption)
        {
            mValueTextArea->setCaption(caption);
        }

        void setValue(Ogre::Real value, bool notifyListener = true);

        Ogre::Real getValue()
        {
            return mValue;
        }

        const Ogre::DisplayString& getCaption()
        {
            return mTextArea->getCaption();
        }

        void setCaption(const Ogre::DisplayString& caption);

        void _cursorPressed(const Ogre::Vector2& cursorPos) override;

        void _cursorReleased(const Ogre::Vector2& cursorPos) override;

        void _cursorMoved(const Ogre::Vector2& cursorPos, float wheelDelta) override;

        void _focusLost() override
        {
            mDragging = false;
        }

    protected:

        /**
        Internal method - given a percentage (from left to right), gets the
        value of the nearest marker.
        */
        Ogre::Real getSnappedValue(Ogre::Real percentage)
        {
            percentage = Ogre::Math::saturate(percentage);
            unsigned int whichMarker = (unsigned int) (percentage * (mMaxValue - mMinValue) / mInterval + 0.5);
            return float(whichMarker) * mInterval + mMinValue;
        }

        Ogre::TextAreaOverlayElement* mTextArea;
        Ogre::TextAreaOverlayElement* mValueTextArea;
        Ogre::BorderPanelOverlayElement* mTrack;
        Ogre::PanelOverlayElement* mHandle;
        bool mDragging;
        bool mFitToContents;
        Ogre::Real mDragOffset;
        Ogre::Real mValue;
        Ogre::Real mMinValue;
        Ogre::Real mMaxValue;
        Ogre::Real mInterval;
    };

    /**
    Basic parameters panel widget.
    */
    class _OgreBitesExport ParamsPanel : public Widget
    {
    public:

        /// Do not instantiate any widgets directly. Use TrayManager.
        ParamsPanel(const Ogre::String& name, Ogre::Real width, unsigned int lines);

        void setAllParamNames(const Ogre::StringVector& paramNames);

        const Ogre::StringVector& getAllParamNames()
        {
            return mNames;
        }

        void setAllParamValues(const Ogre::StringVector& paramValues);

        void setParamValue(const Ogre::DisplayString& paramName, const Ogre::DisplayString& paramValue);

        void setParamValue(unsigned int index, const Ogre::DisplayString& paramValue);

        Ogre::DisplayString getParamValue(const Ogre::DisplayString& paramName);

        Ogre::DisplayString getParamValue(unsigned int index);

        const Ogre::StringVector& getAllParamValues()
        {
            return mValues;
        }

    protected:

        /**
        Internal method - updates text areas based on name and value lists.
        */
        void updateText();

        Ogre::TextAreaOverlayElement* mNamesArea;
        Ogre::TextAreaOverlayElement* mValuesArea;
        Ogre::StringVector mNames;
        Ogre::StringVector mValues;
    };

    /**
    Basic check box widget.
    */
    class _OgreBitesExport CheckBox : public Widget
    {
    public:

        /// Do not instantiate any widgets directly. Use TrayManager.
        CheckBox(const Ogre::String& name, const Ogre::DisplayString& caption, Ogre::Real width);

        const Ogre::DisplayString& getCaption()
        {
            return mTextArea->getCaption();
        }

        void setCaption(const Ogre::DisplayString& caption);

        bool isChecked()
        {
            return mX->isVisible();
        }

        void setChecked(bool checked, bool notifyListener = true);

        void toggle(bool notifyListener = true);

        void _cursorPressed(const Ogre::Vector2& cursorPos) override;

        void _cursorMoved(const Ogre::Vector2& cursorPos, float wheelDelta) override;

        void _focusLost() override;

    protected:

        Ogre::TextAreaOverlayElement* mTextArea;
        Ogre::BorderPanelOverlayElement* mSquare;
        Ogre::OverlayElement* mX;
        bool mFitToContents;
        bool mCursorOver;
    };

    /**
    Custom, decorative widget created from a template.
    */
    class _OgreBitesExport DecorWidget : public Widget
    {
    public:

        /// Do not instantiate any widgets directly. Use TrayManager.
        DecorWidget(const Ogre::String& name, const Ogre::String& templateName);
    };

    /**
    Basic progress bar widget.
    */
    class _OgreBitesExport ProgressBar : public Widget
    {
    public:

        /// Do not instantiate any widgets directly. Use TrayManager.
        ProgressBar(const Ogre::String& name, const Ogre::DisplayString& caption, Ogre::Real width, Ogre::Real commentBoxWidth);

        /**
        Sets the progress as a percentage.
        */
        void setProgress(Ogre::Real progress);

        /**
        Gets the progress as a percentage.
        */
        Ogre::Real getProgress()
        {
            return mProgress;
        }

        const Ogre::DisplayString& getCaption()
        {
            return mTextArea->getCaption();
        }

        void setCaption(const Ogre::DisplayString& caption)
        {
            mTextArea->setCaption(caption);
        }

        const Ogre::DisplayString& getComment()
        {
            return mCommentTextArea->getCaption();
        }

        void setComment(const Ogre::DisplayString& comment)
        {
            mCommentTextArea->setCaption(comment);
        }


    protected:

        Ogre::TextAreaOverlayElement* mTextArea;
        Ogre::TextAreaOverlayElement* mCommentTextArea;
        Ogre::OverlayElement* mMeter;
        Ogre::OverlayElement* mFill;
        Ogre::Real mProgress;
    };

    /**
    Main class to manage a cursor, backdrop, trays and widgets.
    */
    class _OgreBitesExport TrayManager : public TrayListener, public Ogre::ResourceGroupListener, public InputListener
    {
    public:

        /**
        Creates backdrop, cursor, and trays.
        */
        TrayManager(const Ogre::String& name, Ogre::RenderWindow* window, TrayListener* listener = 0);

        /**
        Destroys background, cursor, widgets, and trays.
        */
        virtual ~TrayManager();

        /**
        Converts a 2D screen coordinate (in pixels) to a 3D ray into the scene.
        */
        static Ogre::Ray screenToScene(Ogre::Camera* cam, const Ogre::Vector2& pt);

        /**
        Converts a 3D scene position to a 2D screen position (in relative screen size, 0.0-1.0).
        */
        static Ogre::Vector2 sceneToScreen(Ogre::Camera* cam, const Ogre::Vector3& pt);

        // these methods get the underlying overlays and overlay elements

        Ogre::OverlayContainer* getTrayContainer(TrayLocation trayLoc) { return mTrays[trayLoc]; }
        Ogre::Overlay* getBackdropLayer() { return mBackdropLayer; }
        Ogre::Overlay* getTraysLayer() { return mTraysLayer; }
        Ogre::Overlay* getCursorLayer() { return mCursorLayer; }
        Ogre::OverlayContainer* getBackdropContainer() { return mBackdrop; }
        Ogre::OverlayContainer* getCursorContainer() { return mCursor; }
        Ogre::OverlayElement* getCursorImage() { return mCursor->getChild(mCursor->getName() + "/CursorImage"); }

        void setListener(TrayListener* listener)
        {
            mListener = listener;
        }

        TrayListener* getListener()
        {
            return mListener;
        }

        void showAll();

        void hideAll();

        /**
        Displays specified material on backdrop, or the last material used if
        none specified. Good for pause menus like in the browser.
        */
        void showBackdrop(const Ogre::String& materialName = Ogre::BLANKSTRING);

        void hideBackdrop()
        {
            mBackdropLayer->hide();
        }

        /**
        Displays specified material on cursor, or the last material used if
        none specified. Used to change cursor type.
        */
        void showCursor(const Ogre::String& materialName = Ogre::BLANKSTRING);

        void hideCursor();

        /**
        Updates cursor position based on unbuffered mouse state. This is necessary
        because if the tray manager has been cut off from mouse events for a time,
        the cursor position will be out of date.
        */
        void refreshCursor();

        void showTrays();

        void hideTrays();

        bool isCursorVisible() { return mCursorLayer->isVisible(); }
        bool isBackdropVisible() { return mBackdropLayer->isVisible(); }
        bool areTraysVisible() { return mTraysLayer->isVisible(); }

        /**
        Sets horizontal alignment of a tray's contents.
        */
        void setTrayWidgetAlignment(TrayLocation trayLoc, Ogre::GuiHorizontalAlignment gha);

        // padding and spacing methods

        void setWidgetPadding(Ogre::Real padding);

        void setWidgetSpacing(Ogre::Real spacing);
        void setTrayPadding(Ogre::Real padding);

        virtual Ogre::Real getWidgetPadding() const { return mWidgetPadding; }
        virtual Ogre::Real getWidgetSpacing() const { return mWidgetSpacing; }
        virtual Ogre::Real getTrayPadding() const { return mTrayPadding; }

        /**
        Fits trays to their contents and snaps them to their anchor locations.
        */
        virtual void adjustTrays();

        /**
        Returns a 3D ray into the scene that is directly underneath the cursor.
        */
        Ogre::Ray getCursorRay(Ogre::Camera* cam);

        Button* createButton(TrayLocation trayLoc, const Ogre::String& name, const Ogre::String& caption, Ogre::Real width = 0);

        TextBox* createTextBox(TrayLocation trayLoc, const Ogre::String& name, const Ogre::DisplayString& caption,
            Ogre::Real width, Ogre::Real height);

        SelectMenu* createThickSelectMenu(TrayLocation trayLoc, const Ogre::String& name, const Ogre::DisplayString& caption,
            Ogre::Real width, unsigned int maxItemsShown, const Ogre::StringVector& items = Ogre::StringVector());

        SelectMenu* createLongSelectMenu(TrayLocation trayLoc, const Ogre::String& name, const Ogre::DisplayString& caption,
            Ogre::Real width, Ogre::Real boxWidth, unsigned int maxItemsShown, const Ogre::StringVector& items = Ogre::StringVector());

        SelectMenu* createLongSelectMenu(TrayLocation trayLoc, const Ogre::String& name, const Ogre::DisplayString& caption,
            Ogre::Real boxWidth, unsigned int maxItemsShown, const Ogre::StringVector& items = Ogre::StringVector());

        Label* createLabel(TrayLocation trayLoc, const Ogre::String& name, const Ogre::DisplayString& caption, Ogre::Real width = 0);

        Separator* createSeparator(TrayLocation trayLoc, const Ogre::String& name, Ogre::Real width = 0);

        Slider* createThickSlider(TrayLocation trayLoc, const Ogre::String& name, const Ogre::DisplayString& caption,
            Ogre::Real width, Ogre::Real valueBoxWidth, Ogre::Real minValue, Ogre::Real maxValue, unsigned int snaps);

        Slider* createLongSlider(TrayLocation trayLoc, const Ogre::String& name, const Ogre::DisplayString& caption, Ogre::Real width,
            Ogre::Real trackWidth, Ogre::Real valueBoxWidth, Ogre::Real minValue, Ogre::Real maxValue, unsigned int snaps);

        Slider* createLongSlider(TrayLocation trayLoc, const Ogre::String& name, const Ogre::DisplayString& caption,
            Ogre::Real trackWidth, Ogre::Real valueBoxWidth, Ogre::Real minValue, Ogre::Real maxValue, unsigned int snaps);

        ParamsPanel* createParamsPanel(TrayLocation trayLoc, const Ogre::String& name, Ogre::Real width, unsigned int lines);

        ParamsPanel* createParamsPanel(TrayLocation trayLoc, const Ogre::String& name, Ogre::Real width,
            const Ogre::StringVector& paramNames);

        CheckBox* createCheckBox(TrayLocation trayLoc, const Ogre::String& name, const Ogre::DisplayString& caption,
            Ogre::Real width = 0);

        DecorWidget* createDecorWidget(TrayLocation trayLoc, const Ogre::String& name, const Ogre::String& templateName);

        ProgressBar* createProgressBar(TrayLocation trayLoc, const Ogre::String& name, const Ogre::DisplayString& caption,
            Ogre::Real width, Ogre::Real commentBoxWidth);

        /**
        Shows frame statistics widget set in the specified location.
        */
        void showFrameStats(TrayLocation trayLoc, size_t place = -1);

        /**
        Hides frame statistics widget set.
        */
        void hideFrameStats();

        bool areFrameStatsVisible()
        {
            return mFpsLabel != 0;
        }

        /**
        Toggles visibility of advanced statistics.
        */
        void toggleAdvancedFrameStats()
        {
            if (mFpsLabel) labelHit(mFpsLabel);
        }

        /**
        Shows logo in the specified location.
        */
        void showLogo(TrayLocation trayLoc, size_t place = -1);

        void hideLogo();

        bool isLogoVisible()
        {
            return mLogo != 0;
        }

        /**
        Shows loading bar. Also takes job settings: the number of resource groups
        to be initialised, the number of resource groups to be loaded, and the
        proportion of the job that will be taken up by initialisation. Usually,
        script parsing takes up most time, so the default value is 0.7.
        */
        void showLoadingBar(unsigned int numGroupsInit = 1, unsigned int numGroupsLoad = 1,
            Ogre::Real initProportion = 0.7f);

        void hideLoadingBar();

        bool isLoadingBarVisible()
        {
            return mLoadBar != 0;
        }

        /**
        Pops up a message dialog with an OK button.
        */
        void showOkDialog(const Ogre::DisplayString& caption, const Ogre::DisplayString& message);

        /**
        Pops up a question dialog with Yes and No buttons.
        */
        void showYesNoDialog(const Ogre::DisplayString& caption, const Ogre::DisplayString& question);

        /**
        Hides whatever dialog is currently showing.
        */
        void closeDialog();

        /**
        Determines if any dialog is currently visible.
        */
        bool isDialogVisible();

        /**
        Gets a widget from a tray by name.
        */
        Widget* getWidget(TrayLocation trayLoc, const Ogre::String& name);

        /**
        Gets a widget by name.
        */
        Widget* getWidget(const Ogre::String& name);

        /**
        Gets the number of widgets in total.
        */
        unsigned int getNumWidgets();

        /**
        Gets all the widgets of a specific tray.
        */
        const WidgetList& getWidgets(TrayLocation trayLoc) const {
            return mWidgets[trayLoc];
        }

        /**
        Gets a widget's position in its tray.
        */
        int locateWidgetInTray(Widget* widget);

        /**
        Destroys a widget.
        */
        void destroyWidget(Widget* widget);

        void destroyWidget(TrayLocation trayLoc, size_t place)
        {
            destroyWidget(mWidgets[trayLoc][place]);
        }

        void destroyWidget(TrayLocation trayLoc, const Ogre::String& name)
        {
            destroyWidget(getWidget(trayLoc, name));
        }

        void destroyWidget(const Ogre::String& name)
        {
            destroyWidget(getWidget(name));
        }

        /**
        Destroys all widgets in a tray.
        */
        void destroyAllWidgetsInTray(TrayLocation trayLoc);

        /**
        Destroys all widgets.
        */
        void destroyAllWidgets();

        /**
        Adds a widget to a specified tray at given position, or at the end if unspecified or invalid
        */
        void moveWidgetToTray(Widget* widget, TrayLocation trayLoc, size_t place = -1);

        void moveWidgetToTray(const Ogre::String& name, TrayLocation trayLoc, size_t place = -1)
        {
            moveWidgetToTray(getWidget(name), trayLoc, place);
        }

        void moveWidgetToTray(TrayLocation currentTrayLoc, const Ogre::String& name, TrayLocation targetTrayLoc,
            size_t place = -1)
        {
            moveWidgetToTray(getWidget(currentTrayLoc, name), targetTrayLoc, place);
        }

        void moveWidgetToTray(TrayLocation currentTrayLoc, size_t currentPlace, TrayLocation targetTrayLoc,
            size_t targetPlace = -1)
        {
            moveWidgetToTray(mWidgets[currentTrayLoc][currentPlace], targetTrayLoc, targetPlace);
        }

        /**
        Removes a widget from its tray. Same as moving it to the null tray.
        */
        void removeWidgetFromTray(Widget* widget)
        {
            moveWidgetToTray(widget, TL_NONE);
        }

        void removeWidgetFromTray(const Ogre::String& name)
        {
            removeWidgetFromTray(getWidget(name));
        }

        void removeWidgetFromTray(TrayLocation trayLoc, const Ogre::String& name)
        {
            removeWidgetFromTray(getWidget(trayLoc, name));
        }

        void removeWidgetFromTray(TrayLocation trayLoc, size_t place)
        {
            removeWidgetFromTray(mWidgets[trayLoc][place]);
        }

        /**
        Removes all widgets from a widget tray.
        */
        void clearTray(TrayLocation trayLoc);

        /**
        Removes all widgets from all widget trays.
        */
        void clearAllTrays();

        /**
        Process frame events. Updates frame statistics widget set and deletes
        all widgets queued for destruction.
        */
        void frameRendered(const Ogre::FrameEvent& evt) override;

        void windowUpdate();

        void resourceGroupScriptingStarted(const Ogre::String& groupName, size_t scriptCount) override
        {
            mLoadInc = mGroupInitProportion / float(scriptCount);
            mLoadBar->setCaption("Parsing...");
            windowUpdate();
        }

        void scriptParseStarted(const Ogre::String& scriptName, bool& skipThisScript) override
        {
            mLoadBar->setComment(scriptName);
            windowUpdate();
        }

        void scriptParseEnded(const Ogre::String& scriptName, bool skipped) override
        {
            mLoadBar->setProgress(mLoadBar->getProgress() + mLoadInc);
            windowUpdate();
        }

        void resourceGroupLoadStarted(const Ogre::String& groupName, size_t resourceCount) override
        {
            mLoadInc = mGroupLoadProportion / float(resourceCount);
            mLoadBar->setCaption("Loading...");
            windowUpdate();
        }

        void resourceLoadStarted(const Ogre::ResourcePtr& resource) override
        {
            mLoadBar->setComment(resource->getName());
            windowUpdate();
        }

        void resourceLoadEnded() override
        {
            mLoadBar->setProgress(mLoadBar->getProgress() + mLoadInc);
            windowUpdate();
        }

        void customStageStarted(const Ogre::String& description) override
        {
            mLoadBar->setComment(description);
            windowUpdate();
        }

        void customStageEnded() override
        {
            mLoadBar->setProgress(mLoadBar->getProgress() + mLoadInc);
            windowUpdate();
        }

        /**
        Toggles visibility of advanced statistics.
        */
        void labelHit(Label* label) override;

        /**
        Destroys dialog widgets, notifies listener, and ends high priority session.
        */
        void buttonHit(Button* button) override;

        /**
        Processes mouse button down events. Returns true if the event was
        consumed and should not be passed on to other handlers.
        */
        bool mousePressed(const MouseButtonEvent& evt) override;

        /**
        Processes mouse button up events. Returns true if the event was
        consumed and should not be passed on to other handlers.
        */
        bool mouseReleased(const MouseButtonEvent& evt) override;

        /**
        Updates cursor position. Returns true if the event was
        consumed and should not be passed on to other handlers.
        */
        bool mouseMoved(const MouseMotionEvent& evt) override;

        bool mouseWheelRolled(const MouseWheelEvent& evt) override;

    protected:

        /**
        Internal method to prioritise / deprioritise expanded menus.
        */
        void setExpandedMenu(SelectMenu* m);

        Ogre::String mName;                   // name of this tray system
        Ogre::RenderWindow* mWindow;          // render window
        Ogre::Overlay* mBackdropLayer;        // backdrop layer
        Ogre::Overlay* mTraysLayer;           // widget layer
        Ogre::Overlay* mPriorityLayer;        // top priority layer
        Ogre::Overlay* mCursorLayer;          // cursor layer
        Ogre::OverlayContainer* mBackdrop;    // backdrop
        Ogre::OverlayContainer* mTrays[10];   // widget trays
        WidgetList mWidgets[10];              // widgets
        WidgetList mWidgetDeathRow;           // widget queue for deletion
        Ogre::OverlayContainer* mCursor;      // cursor
        TrayListener* mListener;           // tray listener
        Ogre::Real mWidgetPadding;            // widget padding
        Ogre::Real mWidgetSpacing;            // widget spacing
        Ogre::Real mTrayPadding;              // tray padding
        bool mTrayDrag;                       // a mouse press was initiated on a tray
        SelectMenu* mExpandedMenu;            // top priority expanded menu widget
        TextBox* mDialog;                     // top priority dialog widget
        Ogre::OverlayContainer* mDialogShade; // top priority dialog shade
        Button* mOk;                          // top priority OK button
        Button* mYes;                         // top priority Yes button
        Button* mNo;                          // top priority No button
        bool mCursorWasVisible;               // cursor state before showing dialog
        Label* mFpsLabel;                     // FPS label
        ParamsPanel* mStatsPanel;             // frame stats panel
        DecorWidget* mLogo;                   // logo
        ProgressBar* mLoadBar;                // loading bar
        Ogre::Real mGroupInitProportion;      // proportion of load job assigned to initialising one resource group
        Ogre::Real mGroupLoadProportion;      // proportion of load job assigned to loading one resource group
        Ogre::Real mLoadInc;                  // loading increment
        Ogre::GuiHorizontalAlignment mTrayWidgetAlign[10];   // tray widget alignments
        Ogre::Timer* mTimer;                  // Root::getSingleton().getTimer()
        unsigned long mLastStatUpdateTime;    // The last time the stat text were updated
        Ogre::Vector2 mCursorPos;             // current cursor position

    };
    /** @} */
    /** @} */
    /** @} */
}
#endif
