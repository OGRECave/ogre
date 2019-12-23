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
#ifndef __SampleBrowser_H__
#define __SampleBrowser_H__

#include "SampleContext.h"
#include "SamplePlugin.h"
#include "OgreTrays.h"
#include "OgreConfigFile.h"
#include "OgreTechnique.h"
#include "OgreArchiveManager.h"

#define ENABLE_SHADERS_CACHE 1

#if OGRE_PLATFORM == OGRE_PLATFORM_WINRT
#    include <sdkddkver.h>
#    if defined(_WIN32_WINNT) && _WIN32_WINNT == _WIN32_WINNT_WIN8
//      For WinRT 8.0 we only support running from the cache file.
#       undef ENABLE_SHADERS_CACHE
#       define ENABLE_SHADERS_CACHE 1
#    endif
#endif

#ifndef __OGRE_WINRT_PHONE
#define __OGRE_WINRT_PHONE 0
#endif

#ifdef OGRE_STATIC_LIB
#include "DefaultSamplesPlugin.h"
#   ifdef SAMPLES_INCLUDE_PLAYPEN
#   include "PlayPenTestPlugin.h"
#   endif
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
#   ifdef __OBJC__
#       import <UIKit/UIKit.h>
#   endif

namespace OgreBites
{
    class SampleBrowser;
}

@interface SampleBrowserGestureView : UIView
{
    OgreBites::SampleBrowser *mBrowser;
}
@property (assign) OgreBites::SampleBrowser *mBrowser;

                   @end
#endif

namespace OgreBites
{
    /*=============================================================================
      | The OGRE Sample Browser. Features a menu accessible from all samples,
      | dynamic configuration, resource reloading, node labeling, and more.
      =============================================================================*/
    class SampleBrowser : public SampleContext, public TrayListener
    {
#ifdef OGRE_STATIC_LIB
        typedef std::map<Ogre::String, Ogre::Plugin*> PluginMap;
        PluginMap mPluginNameMap;                      // A structure to map plugin names to class types
#endif
    public:

    SampleBrowser(bool nograb = false, int startSampleIndex = -1)
    : SampleContext("OGRE Sample Browser"), mGrabInput(!nograb)
        {
            mIsShuttingDown = false;
            mTrayMgr = 0;
            mLastViewCategory = 0;
            mLastViewTitle = 0;
            mLastSampleIndex = -1;
            mStartSampleIndex = startSampleIndex;
            mCategoryMenu = 0;
            mSampleMenu = 0;
            mSampleSlider = 0;
            mTitleLabel = 0;
            mDescBox = 0;
            mRendererMenu = 0;
            mCarouselPlace = 0.0f;

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            mGestureView = 0;
#endif
        }

        virtual void loadStartUpSample()
        {
            if (mStartSampleIndex != -1)
            {
                runSampleByIndex(mStartSampleIndex);
                mStartSampleIndex = -1;
            }
        }

        virtual void runSampleByIndex(int idx)
        {
            runSample(Ogre::any_cast<Sample*>(mThumbs[idx]->getUserObjectBindings().getUserAny()));
        }

        /*-----------------------------------------------------------------------------
          | Extends runSample to handle creation and destruction of dummy scene.
          -----------------------------------------------------------------------------*/
        virtual void runSample(Sample* s)
        {
            if (mCurrentSample)  // sample quitting
            {
                mCurrentSample->_shutdown();
                mCurrentSample = 0;
                mSamplePaused = false;     // don't pause next sample

                // create dummy scene and modify controls
                createDummyScene();
                mTrayMgr->showBackdrop("SdkTrays/Bands");
                mTrayMgr->showAll();
                ((Button*)mTrayMgr->getWidget("StartStop"))->setCaption("Start Sample");
            }

            if (s)  // sample starting
            {
                // destroy dummy scene and modify controls
                ((Button*)mTrayMgr->getWidget("StartStop"))->setCaption("Stop Sample");
                mTrayMgr->showBackdrop("SdkTrays/Shade");
                mTrayMgr->hideAll();
                destroyDummyScene();

                try
                {
#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
                    s->setShaderGenerator(mShaderGenerator);
#endif
                    SampleContext::runSample(s);
                }
                catch (Ogre::Exception& e)   // if failed to start, show error and fall back to menu
                {
                    destroyDummyScene();

                    s->_shutdown();

                    createDummyScene();
                    mTrayMgr->showBackdrop("SdkTrays/Bands");
                    mTrayMgr->showAll();
                    ((Button*)mTrayMgr->getWidget("StartStop"))->setCaption("Start Sample");

                    mTrayMgr->showOkDialog("Error!", e.getDescription() + "\nSource: " + e.getSource());
                }
            }
        }

        /// catch any exceptions that might drop out of event handlers implemented by Samples
        bool frameStarted(const Ogre::FrameEvent& evt)
        {
            try
            {
                return SampleContext::frameStarted(evt);
            }
            catch (Ogre::Exception& e)   // show error and fall back to menu
            {
                runSample(0);
                mTrayMgr->showOkDialog("Error!", e.getDescription() + "\nSource: " + e.getSource());
            }

            return true;
        }

        /*-----------------------------------------------------------------------------
          | Extends frameRenderingQueued to update tray manager and carousel.
          -----------------------------------------------------------------------------*/
        bool frameRenderingQueued(const Ogre::FrameEvent& evt)
        {
            // don't do all these calculations when sample's running or when in configuration screen or when no samples loaded
            if (!mLoadedSamples.empty() && mTitleLabel->getTrayLocation() != TL_NONE && (!mCurrentSample || mSamplePaused))
            {
                // makes the carousel spin smoothly toward its right position
                Ogre::Real carouselOffset = mSampleMenu->getSelectionIndex() - mCarouselPlace;
                if ((carouselOffset <= 0.001) && (carouselOffset >= -0.001)) mCarouselPlace = mSampleMenu->getSelectionIndex();
                else mCarouselPlace += carouselOffset * Ogre::Math::Clamp<Ogre::Real>(evt.timeSinceLastFrame * 15.0, -1.0, 1.0);

                // update the thumbnail positions based on carousel state
                for (int i = 0; i < (int)mThumbs.size(); i++)
                {
                    Ogre::Real thumbOffset = mCarouselPlace - i;
                    Ogre::Real phase = (thumbOffset / 2.0) - 2.8;

                    if (thumbOffset < -5 || thumbOffset > 4)    // prevent thumbnails from wrapping around in a circle
                    {
                        mThumbs[i]->hide();
                        continue;
                    }
                    else mThumbs[i]->show();

                    Ogre::Real left = Ogre::Math::Cos(phase) * 200.0;
                    Ogre::Real top = Ogre::Math::Sin(phase) * 200.0;
                    Ogre::Real scale = 1.0 / Ogre::Math::Pow((Ogre::Math::Abs(thumbOffset) + 1.0), 0.75);

                    Ogre::BorderPanelOverlayElement* frame =
                        (Ogre::BorderPanelOverlayElement*)mThumbs[i]->getChildIterator().getNext();

                    mThumbs[i]->setDimensions(128.0 * scale, 96.0 * scale);
                    frame->setDimensions(mThumbs[i]->getWidth() + 16.0, mThumbs[i]->getHeight() + 16.0);
                    mThumbs[i]->setPosition((int)(left - 80.0 - (mThumbs[i]->getWidth() / 2.0)),
                                            (int)(top - 5.0 - (mThumbs[i]->getHeight() / 2.0)));

                    if (i == mSampleMenu->getSelectionIndex()) frame->setBorderMaterialName("SdkTrays/Frame/Over");
                    else frame->setBorderMaterialName("SdkTrays/Frame");
                }
            }

            mTrayMgr->frameRendered(evt);

            return SampleContext::frameRenderingQueued(evt);
        }

        /*-----------------------------------------------------------------------------
          | Handles confirmation dialog responses.
          -----------------------------------------------------------------------------*/
        virtual void yesNoDialogClosed(const Ogre::DisplayString& question, bool yesHit)
        {
            if (question.substr(0, 14) == "This will stop" && yesHit)   // confirm unloading of samples
            {
                runSample(0);
                buttonHit((Button*)mTrayMgr->getWidget("UnloadReload"));
            }
        }

        /*-----------------------------------------------------------------------------
          | Handles button widget events.
          -----------------------------------------------------------------------------*/
        virtual void buttonHit(Button* b)
        {
            if (b->getName() == "StartStop")   // start or stop sample
            {
                if (b->getCaption() == "Start Sample")
                {
                    if (mLoadedSamples.empty()) {
                        mTrayMgr->showOkDialog("Error!", "No sample selected!");
                    } else {
                        // use the sample pointer we stored inside the thumbnail
                        Ogre::Renderable* r = mThumbs[mSampleMenu->getSelectionIndex()];
                        runSample(Ogre::any_cast<Sample*>(r->getUserObjectBindings().getUserAny()));
                    }
                } else
                    runSample(0);
            }
            else if (b->getName() == "UnloadReload")   // unload or reload sample plugins and update controls
            {
                if (b->getCaption() == "Unload Samples")
                {
                    if (mCurrentSample) mTrayMgr->showYesNoDialog("Warning!", "This will stop the current sample. Unload anyway?");
                    else
                    {
                        // save off current view and try to restore it on the next reload
                        mLastViewTitle = mSampleMenu->getSelectionIndex();
                        mLastViewCategory = mCategoryMenu->getSelectionIndex();

                        unloadSamples();
                        populateSampleMenus();
                        b->setCaption("Reload Samples");
                    }
                }
                else
                {
                    loadSamples();
                    populateSampleMenus();
                    if (!mLoadedSamples.empty()) b->setCaption("Unload Samples");

                    try  // attempt to restore the last view before unloading samples
                    {
                        mCategoryMenu->selectItem(mLastViewCategory);
                        mSampleMenu->selectItem(mLastViewTitle);
                    }
                    catch (Ogre::Exception&) {}
                }
            }
            else if (b->getName() == "Configure")   // enter configuration screen
            {
                mTrayMgr->removeWidgetFromTray("StartStop");
                mTrayMgr->removeWidgetFromTray("Configure");
#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
                mTrayMgr->removeWidgetFromTray("UnloadReload");
                mTrayMgr->removeWidgetFromTray("Quit");
                mTrayMgr->moveWidgetToTray("Apply", TL_RIGHT);
#endif
                mTrayMgr->moveWidgetToTray("Back", TL_RIGHT);

                for (unsigned int i = 0; i < mThumbs.size(); i++)
                {
                    mThumbs[i]->hide();
                }

                while (mTrayMgr->getTrayContainer(TL_CENTER)->isVisible())
                {
                    mTrayMgr->removeWidgetFromTray(TL_CENTER, 0);
                }

                while (mTrayMgr->getTrayContainer(TL_LEFT)->isVisible())
                {
                    mTrayMgr->removeWidgetFromTray(TL_LEFT, 0);
                }

                mTrayMgr->moveWidgetToTray("ConfigLabel", TL_LEFT);
                mTrayMgr->moveWidgetToTray(mRendererMenu, TL_LEFT);
                mTrayMgr->moveWidgetToTray("ConfigSeparator", TL_LEFT);

                mRendererMenu->selectItem(mRoot->getRenderSystem()->getName());

                windowResized(mWindow);
            }
            else if (b->getName() == "Back")   // leave configuration screen
            {
                while (mTrayMgr->getWidgets(mRendererMenu->getTrayLocation()).size() > 3)
                {
                    mTrayMgr->destroyWidget(mRendererMenu->getTrayLocation(), 3);
                }

                while (!mTrayMgr->getWidgets(TL_NONE).empty())
                {
                    mTrayMgr->moveWidgetToTray(TL_NONE, 0, TL_LEFT);
                }

                mTrayMgr->removeWidgetFromTray("Apply");
                mTrayMgr->removeWidgetFromTray("Back");
                mTrayMgr->removeWidgetFromTray("ConfigLabel");
                mTrayMgr->removeWidgetFromTray(mRendererMenu);
                mTrayMgr->removeWidgetFromTray("ConfigSeparator");

                mTrayMgr->moveWidgetToTray("StartStop", TL_RIGHT);
#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
                mTrayMgr->moveWidgetToTray("UnloadReload", TL_RIGHT);
#endif
                mTrayMgr->moveWidgetToTray("Configure", TL_RIGHT);
#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
                mTrayMgr->moveWidgetToTray("Quit", TL_RIGHT);
#endif

                windowResized(mWindow);
            }
            else if (b->getName() == "Apply")   // apply any changes made in the configuration screen
            {
                bool reset = false;

                auto options =
                    mRoot->getRenderSystemByName(mRendererMenu->getSelectedItem())->getConfigOptions();

                Ogre::NameValuePairList newOptions;

                // collect new settings and decide if a reset is needed

                if (mRendererMenu->getSelectedItem() != mRoot->getRenderSystem()->getName()) {
                    reset = true;
                }

                for (unsigned int i = 3; i < mTrayMgr->getWidgets(mRendererMenu->getTrayLocation()).size(); i++)
                {
                    SelectMenu* menu = (SelectMenu*)mTrayMgr->getWidgets(mRendererMenu->getTrayLocation())[i];
                    if (menu->getSelectedItem() != options[menu->getCaption()].currentValue) reset = true;
                    newOptions[menu->getCaption()] = menu->getSelectedItem();
                }

                // reset with new settings if necessary
                if (reset) reconfigure(mRendererMenu->getSelectedItem(), newOptions);
            }
            else
            {
                mRoot->queueEndRendering();   // exit browser
            }
        }

        /*-----------------------------------------------------------------------------
          | Handles menu item selection changes.
          -----------------------------------------------------------------------------*/
        virtual void itemSelected(SelectMenu* menu)
        {
            if (menu == mCategoryMenu)      // category changed, so update the sample menu, carousel, and slider
            {
                for (unsigned int i = 0; i < mThumbs.size(); i++)    // destroy all thumbnails in carousel
                {
                    Ogre::MaterialManager::getSingleton().remove(mThumbs[i]->getName(), "Essential");
                    Widget::nukeOverlayElement(mThumbs[i]);
                }
                mThumbs.clear();

                Ogre::OverlayManager& om = Ogre::OverlayManager::getSingleton();
                Ogre::String selectedCategory;

                if (menu->getSelectionIndex() != -1) selectedCategory = menu->getSelectedItem();
                else
                {
                    mTitleLabel->setCaption("");
                    mDescBox->setText("");
                }

                bool all = selectedCategory == "All";
                Ogre::StringVector sampleTitles;
                Ogre::MaterialPtr templateMat = Ogre::MaterialManager::getSingleton().getByName("SdkTrays/SampleThumbnail");

                // populate the sample menu and carousel with filtered samples
                for (SampleSet::iterator i = mLoadedSamples.begin(); i != mLoadedSamples.end(); i++)
                {
                    Ogre::NameValuePairList& info = (*i)->getInfo();

                    if (all || info["Category"] == selectedCategory)
                    {
                        Ogre::String name = "SdkTrays/SampleThumb" + Ogre::StringConverter::toString(sampleTitles.size() + 1);

                        // clone a new material for sample thumbnail
                        Ogre::MaterialPtr newMat = templateMat->clone(name);

                        Ogre::TextureUnitState* tus = newMat->getTechnique(0)->getPass(0)->getTextureUnitState(0);
                        if (Ogre::ResourceGroupManager::getSingleton().resourceExists("Essential", info["Thumbnail"]))
                            tus->setTextureName(info["Thumbnail"]);
                        else
                            tus->setTextureName("thumb_error.png");

                        // create sample thumbnail overlay
                        Ogre::BorderPanelOverlayElement* bp = (Ogre::BorderPanelOverlayElement*)
                            om.createOverlayElementFromTemplate("SdkTrays/Picture", "BorderPanel", name);
                        bp->setHorizontalAlignment(Ogre::GHA_RIGHT);
                        bp->setVerticalAlignment(Ogre::GVA_CENTER);
                        bp->setMaterialName(name);
                        bp->getUserObjectBindings().setUserAny(Ogre::Any(*i));
                        mTrayMgr->getTraysLayer()->add2D(bp);

                        // add sample thumbnail and title
                        mThumbs.push_back(bp);
                        sampleTitles.push_back((*i)->getInfo()["Title"]);
                    }
                }

                mCarouselPlace = 0;  // reset carousel

                mSampleMenu->setItems(sampleTitles);
                if (mSampleMenu->getNumItems() != 0) itemSelected(mSampleMenu);

                mSampleSlider->setRange(1, static_cast<Ogre::Real>(sampleTitles.size()), static_cast<Ogre::Real>(sampleTitles.size()));
            }
            else if (menu == mSampleMenu)    // sample changed, so update slider, label and description
            {
                if (mSampleSlider->getValue() != menu->getSelectionIndex() + 1)
                    mSampleSlider->setValue(menu->getSelectionIndex() + 1);

                Ogre::Renderable* r = mThumbs[mSampleMenu->getSelectionIndex()];
                Sample* s = Ogre::any_cast<Sample*>(r->getUserObjectBindings().getUserAny());
                mTitleLabel->setCaption(menu->getSelectedItem());
                mDescBox->setText("Category: " + s->getInfo()["Category"] + "\nDescription: " + s->getInfo()["Description"]);

                if (mCurrentSample != s) ((Button*)mTrayMgr->getWidget("StartStop"))->setCaption("Start Sample");
                else ((Button*)mTrayMgr->getWidget("StartStop"))->setCaption("Stop Sample");
            }
            else if (menu == mRendererMenu)    // renderer selected, so update all settings
            {
                while (mTrayMgr->getWidgets(mRendererMenu->getTrayLocation()).size() > 3)
                {
                    mTrayMgr->destroyWidget(mRendererMenu->getTrayLocation(), 3);
                }

                auto options = mRoot->getRenderSystemByName(menu->getSelectedItem())->getConfigOptions();

                unsigned int i = 0;

                // create all the config option select menus
                for (Ogre::ConfigOptionMap::iterator it = options.begin(); it != options.end(); it++)
                {
                    i++;
                    SelectMenu* optionMenu = mTrayMgr->createLongSelectMenu
                        (TL_LEFT, "ConfigOption" + Ogre::StringConverter::toString(i), it->first, 450, 240, 10);
                    optionMenu->setItems(it->second.possibleValues);

                    // if the current config value is not in the menu, add it
                    if(optionMenu->containsItem(it->second.currentValue) == false)
                    {
                        optionMenu->addItem(it->second.currentValue);
                    }

                    optionMenu->selectItem(it->second.currentValue);
                }

                windowResized(mWindow);
            }
        }

        /*-----------------------------------------------------------------------------
          | Handles sample slider changes.
          -----------------------------------------------------------------------------*/
        virtual void sliderMoved(Slider* slider)
        {
            // format the caption to be fraction style
            Ogre::String denom = "/" + Ogre::StringConverter::toString(mSampleMenu->getNumItems());
            slider->setValueCaption(slider->getValueCaption() + denom);

            // tell the sample menu to change if it hasn't already
            if (mSampleMenu->getSelectionIndex() != -1 && mSampleMenu->getSelectionIndex() != slider->getValue() - 1)
                mSampleMenu->selectItem(slider->getValue() - 1);
        }

        /*-----------------------------------------------------------------------------
          | Handles keypresses.
          -----------------------------------------------------------------------------*/
        virtual bool keyPressed(const KeyboardEvent& evt)
        {
            if (mTrayMgr->isDialogVisible()) return true;  // ignore keypresses when dialog is showing

            Keycode key = evt.keysym.sym;

            if (key == SDLK_ESCAPE)
            {
#if __OGRE_WINRT_PHONE
                // If there is a quit button, assume that we intended to press it via 'ESC'.
                if (mTrayMgr->areTraysVisible())
                {
                    Widget *pWidget = mTrayMgr->getWidget("Quit");
                    if (pWidget)
                    {
                        buttonHit((Button*)pWidget);  // on phone, quit entirely.
                        return false;  // now act as if we didn't handle the button to get AppModel to exit.
                    }
                }
#endif // __OGRE_WINRT_PHONE
                if (mTitleLabel->getTrayLocation() != TL_NONE)
                {
                    // if we're in the main screen and a sample's running, toggle sample pause state
                    if (mCurrentSample)
                    {
                        if (mSamplePaused)
                        {
                            mTrayMgr->hideAll();
                            unpauseCurrentSample();
                        }
                        else
                        {
                            pauseCurrentSample();
                            mTrayMgr->showAll();
                        }
                    }
                }
                else buttonHit((Button*)mTrayMgr->getWidget("Back"));  // if we're in config, just go back
            }
            else if ((key == SDLK_UP || key == SDLK_DOWN) && mTitleLabel->getTrayLocation() != TL_NONE)
            {
                // if we're in the main screen, use the up and down arrow keys to cycle through samples
                int newIndex = mSampleMenu->getSelectionIndex() + (key == SDLK_UP ? -1 : 1);
                mSampleMenu->selectItem(Ogre::Math::Clamp<size_t>(newIndex, 0, mSampleMenu->getNumItems() - 1));
            }
            else if (key == SDLK_RETURN)   // start or stop sample
            {
                if (!mLoadedSamples.empty() && (mSamplePaused || mCurrentSample == 0))
                {
                    Ogre::Renderable* r = mThumbs[mSampleMenu->getSelectionIndex()];
                    Sample* newSample = Ogre::any_cast<Sample*>(r->getUserObjectBindings().getUserAny());
                    runSample(newSample == mCurrentSample ? 0 : newSample);
                }
            }
#if OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
            else if (key == SDLK_M)   // change orientation mode
            {
                unsigned int orientationMode = (unsigned int)mWindow->getViewport(0)->getOrientationMode();
                orientationMode++;
                if (orientationMode >= 4)
                    orientationMode = 0;
                mWindow->getViewport(0)->setOrientationMode((Ogre::OrientationMode)orientationMode);
            }
#endif
            else if(key == SDLK_F9)   // toggle full screen
            {
                // Make sure we use the window size as originally requested, NOT the
                // current window size (which may have altered to fit desktop)
                auto opti = mRoot->getRenderSystem()->getConfigOptions().find(
                    "Video Mode");
                Ogre::StringVector vmopts = Ogre::StringUtil::split(opti->second.currentValue, " x");
                unsigned int w = Ogre::StringConverter::parseUnsignedInt(vmopts[0]);
                unsigned int h = Ogre::StringConverter::parseUnsignedInt(vmopts[1]);
                mWindow->setFullscreen(!mWindow->isFullScreen(), w, h);
            }
            else if(key == SDLK_F11 || key == SDLK_F12) // Decrease and increase FSAA level on the fly
            {
                // current FSAA                0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16
                unsigned decreasedFSAA[17] = { 0, 0, 1, 2, 2, 4, 4, 4, 4, 8, 8, 8, 8, 8, 8, 8, 8 };
                unsigned increasedFSAA[17] = { 2, 2, 4, 4, 8, 8, 8, 8,16,16,16,16,16,16,16,16, 0, };
                unsigned FSAA = std::min(mWindow->getFSAA(), 16U);
                unsigned newFSAA = (key == SDLK_F12) ? increasedFSAA[FSAA] : decreasedFSAA[FSAA];
                if(newFSAA != 0)
                    mWindow->setFSAA(newFSAA, mWindow->getFSAAHint());
            }

            return SampleContext::keyPressed(evt);
        }

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        void motionBegan( void )
        {
        }

        void motionEnded( void )
        {
            if (mTrayMgr->isDialogVisible()) return;  // ignore keypresses when dialog is showing

            if (mTitleLabel->getTrayLocation() != TL_NONE)
            {
                // if we're in the main screen and a sample's running, toggle sample pause state
                if (mCurrentSample)
                {
                    if (mSamplePaused)
                    {
                        mTrayMgr->hideAll();
                        unpauseCurrentSample();
                    }
                    else
                    {
                        pauseCurrentSample();
                        mTrayMgr->showAll();
                    }
                }
            }
            else buttonHit((Button*)mTrayMgr->getWidget("Back"));  // if we're in config, just go back

        }

        void motionCancelled( void )
        {
        }
#endif

        /*-----------------------------------------------------------------------------
          | Extends pointerPressed to inject mouse press into tray manager, and to check
          | for thumbnail clicks, just because we can.
          -----------------------------------------------------------------------------*/
        virtual bool mousePressed(const MouseButtonEvent& evt)
        {
            if (mTitleLabel->getTrayLocation() != TL_NONE)
            {
                for (unsigned int i = 0; i < mThumbs.size(); i++)
                {
                    if (mThumbs[i]->isVisible() && Widget::isCursorOver(mThumbs[i],
                        Ogre::Vector2(mTrayMgr->getCursorContainer()->getLeft(), mTrayMgr->getCursorContainer()->getTop()), 0))
                    {
                        mSampleMenu->selectItem(i);
                        return true;
                    }
                }
            }

            if (isCurrentSamplePaused()) return mTrayMgr->mousePressed(evt);

            return SampleContext::mousePressed(evt);
        }

        // convert and redirect
        virtual bool touchPressed(const TouchFingerEvent& evt) {
            MouseButtonEvent e;
            e.button = BUTTON_LEFT;
            return mousePressed(e);
        }

        /*-----------------------------------------------------------------------------
          | Extends pointerReleased to inject mouse release into tray manager.
          -----------------------------------------------------------------------------*/
        virtual bool mouseReleased(const MouseButtonEvent& evt)
         {
            if (isCurrentSamplePaused()) return mTrayMgr->mouseReleased(evt);

            return SampleContext::mouseReleased(evt);
        }

        // convert and redirect
        virtual bool touchReleased(const TouchFingerEvent& evt) {
            MouseButtonEvent e;
            e.button = BUTTON_LEFT;
            return mouseReleased(e);
        }

        /*-----------------------------------------------------------------------------
          | Extends pointerMoved to inject mouse position into tray manager, and checks
          | for mouse wheel movements to slide the carousel, because we can.
          -----------------------------------------------------------------------------*/
        virtual bool mouseMoved(const MouseMotionEvent& evt)
        {
            if (isCurrentSamplePaused()) return mTrayMgr->mouseMoved(evt);

            return SampleContext::mouseMoved(evt);
        }

        // convert and redirect
        virtual bool touchMoved(const TouchFingerEvent& evt) {
            MouseMotionEvent e;
            e.x = evt.x * mWindow->getWidth();
            e.y = evt.y * mWindow->getHeight();
            e.xrel = evt.dx * mWindow->getWidth();
            e.yrel = evt.dy * mWindow->getHeight();
            return mouseMoved(e);
        }

        //TODO: Handle iOS and Android.
        /** Mouse wheel scrolls the sample list.
         */
        virtual bool mouseWheelRolled(const MouseWheelEvent& evt)
        {
            if (isCurrentSamplePaused() && mTitleLabel->getTrayLocation() != TL_NONE &&
                mSampleMenu->getNumItems() != 0)
            {
                int newIndex = mSampleMenu->getSelectionIndex() - evt.y / Ogre::Math::Abs(evt.y);
                mSampleMenu->selectItem(Ogre::Math::Clamp<size_t>(newIndex, 0, mSampleMenu->getNumItems() - 1));
            }

            return SampleContext::mouseWheelRolled(evt);
        }

        /*-----------------------------------------------------------------------------
          | Extends windowResized to best fit menus on screen. We basically move the
          | menu tray to the left for higher resolutions and move it to the center
          | for lower resolutions.
          -----------------------------------------------------------------------------*/
        virtual void windowResized(Ogre::RenderWindow* rw)
        {
            if (!mTrayMgr) return;

            Ogre::OverlayContainer* center = mTrayMgr->getTrayContainer(TL_CENTER);
            Ogre::OverlayContainer* left = mTrayMgr->getTrayContainer(TL_LEFT);

            if (center->isVisible() && rw->getWidth() < 1280 - center->getWidth())
            {
                while (center->isVisible())
                {
                    mTrayMgr->moveWidgetToTray(mTrayMgr->getWidgets(TL_CENTER)[0], TL_LEFT);
                }
            }
            else if (left->isVisible() && rw->getWidth() >= 1280 - left->getWidth())
            {
                while (left->isVisible())
                {
                    mTrayMgr->moveWidgetToTray(mTrayMgr->getWidgets(TL_LEFT)[0], TL_CENTER);
                }
            }

            SampleContext::windowResized(rw);
        }

        /*-----------------------------------------------------------------------------
          | Extends setup to create dummy scene and tray interface.
          -----------------------------------------------------------------------------*/
        virtual void setup()
        {
            ApplicationContext::setup();
            mWindow = getRenderWindow();
            addInputListener(this);
            if(mGrabInput) setWindowGrab();
            else mTrayMgr->hideCursor();
#ifdef OGRE_STATIC_LIB
            mPluginNameMap["DefaultSamples"] = new DefaultSamplesPlugin();
#   ifdef SAMPLES_INCLUDE_PLAYPEN
            mPluginNameMap["PlaypenTests"] = new  PlaypenTestPlugin();
#   endif
#endif

            Sample* startupSample = loadSamples();

            // create template material for sample thumbnails
            Ogre::MaterialPtr thumbMat = Ogre::MaterialManager::getSingleton().create("SdkTrays/SampleThumbnail", "Essential");
            thumbMat->getTechnique(0)->getPass(0)->createTextureUnitState();

            setupWidgets();
            windowResized(mWindow);   // adjust menus for resolution

            // if this is our first time running, and there's a startup sample, run it
            if (startupSample && mFirstRun){
                runSample(startupSample);
            }
        }

    protected:
        /*-----------------------------------------------------------------------------
          | Overrides the default window title.
          -----------------------------------------------------------------------------*/
        virtual NativeWindowPair createWindow(const Ogre::String& name, uint32_t w, uint32_t h, Ogre::NameValuePairList miscParams)
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
            // Make sure Trays are not tiny -  we cannot easily scale the UI, therefore just reduce resolution
            float contentScaling = AConfiguration_getDensity(mAConfig)/float(ACONFIGURATION_DENSITY_HIGH);
            if(contentScaling > 1.0)
            {
                miscParams["contentScalingFactor"] = std::to_string(contentScaling);
                miscParams["FSAA"] = "2";
            }
#endif
            NativeWindowPair res = ApplicationContext::createWindow(name, w, h, miscParams);

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            mGestureView = [[SampleBrowserGestureView alloc] init];
            mGestureView.mBrowser = this;

            [[[UIApplication sharedApplication] keyWindow] addSubview:mGestureView];
#endif

            return res;
        }

        /*-----------------------------------------------------------------------------
          | Initialises only the browser's resources and those most commonly used
          | by samples. This way, additional special content can be initialised by
          | the samples that use them, so startup time is unaffected.
          -----------------------------------------------------------------------------*/
        virtual void loadResources()
        {
            Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Essential");
            mTrayMgr = new TrayManager("BrowserControls", getRenderWindow(), this);
            mTrayMgr->showBackdrop("SdkTrays/Bands");
            mTrayMgr->getTrayContainer(TL_NONE)->hide();

#if ENABLE_SHADERS_CACHE == 1
            enableShaderCache();
#endif
            // Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

            createDummyScene();

            mTrayMgr->showLoadingBar(1, 0);
            Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
            mTrayMgr->hideLoadingBar();
        }

        /*-----------------------------------------------------------------------------
          | Loads sample plugins from a configuration file.
          -----------------------------------------------------------------------------*/
        virtual Sample* loadSamples()
        {
            Sample* startupSample = 0;

            Ogre::StringVector unloadedSamplePlugins;
            Ogre::ConfigFile cfg;
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
            Ogre::Archive* apk = Ogre::ArchiveManager::getSingleton().load("", "APKFileSystem", true);
            cfg.load(apk->open(mFSLayer->getConfigFilePath("samples.cfg")));
#else
            cfg.load(mFSLayer->getConfigFilePath("samples.cfg"));
#endif

            Ogre::String sampleDir = cfg.getSetting("SampleFolder");        // Mac OS X just uses Resources/ directory
            Ogre::StringVector sampleList = cfg.getMultiSetting("SamplePlugin");
            Ogre::String startupSampleTitle = cfg.getSetting("StartupSample");

            sampleDir = Ogre::FileSystemLayer::resolveBundlePath(sampleDir);

#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE && OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
            if (sampleDir.empty()) sampleDir = ".";   // user didn't specify plugins folder, try current one
#endif

            // add slash or backslash based on platform
            char lastChar = sampleDir[sampleDir.length() - 1];
            if (lastChar != '/' && lastChar != '\\')
            {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || (OGRE_PLATFORM == OGRE_PLATFORM_WINRT)
                sampleDir += "\\";
#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX
                sampleDir += "/";
#endif
            }

            SampleSet newSamples;

            // loop through all sample plugins...
            for (Ogre::StringVector::iterator i = sampleList.begin(); i != sampleList.end(); i++)
            {
                try   // try to load the plugin
                {
#ifdef OGRE_STATIC_LIB
                    Ogre::String sampleName = *i;
                    // in debug, remove any suffix
                    if(Ogre::StringUtil::endsWith(sampleName, "_d"))
                        sampleName = sampleName.substr(0, sampleName.length()-2);

                    Ogre::Plugin* pluginInstance = mPluginNameMap[sampleName];
                    if(pluginInstance)
                    {
                        mRoot->installPlugin(pluginInstance);
                    }
#else
                    mRoot->loadPlugin(sampleDir + *i);
#endif
                }
                catch (Ogre::Exception& e)   // plugin couldn't be loaded
                {
                    Ogre::LogManager::getSingleton().logError(e.what());
                    unloadedSamplePlugins.push_back(sampleDir + *i);
                    continue;
                }

                Ogre::Plugin* p = mRoot->getInstalledPlugins().back();   // acquire plugin instance
                SamplePlugin* sp = dynamic_cast<SamplePlugin*>(p);

                if (!sp)  // this is not a SamplePlugin, so unload it
                {
                    //unloadedSamplePlugins.push_back(sampleDir + *i);
#ifdef OGRE_STATIC_LIB
                    //mRoot->uninstallPlugin(p);
#else
                    //mRoot->unloadPlugin(sampleDir + *i);
#endif
                    continue;
                }

                mLoadedSamplePlugins.push_back(sampleDir + *i);   // add to records

                // go through every sample in the plugin...
                newSamples = sp->getSamples();
                for (SampleSet::iterator j = newSamples.begin(); j != newSamples.end(); j++)
                {
                    Ogre::NameValuePairList& info = (*j)->getInfo();   // acquire custom sample info
                    Ogre::NameValuePairList::iterator k;

                    // give sample default title and category if none found
                    k= info.find("Title");
                    if (k == info.end() || k->second.empty()) info["Title"] = "Untitled";
                    k = info.find("Category");
                    if (k == info.end() || k->second.empty()) info["Category"] = "Unsorted";
                    k = info.find("Thumbnail");
                    if (k == info.end() || k->second.empty()) info["Thumbnail"] = "thumb_error.png";

                    mLoadedSamples.insert(*j);                    // add sample only after ensuring title for sorting
                    mSampleCategories.insert(info["Category"]);   // add sample category

                    if (info["Title"] == startupSampleTitle) startupSample = *j;   // we found the startup sample
                }
            }

            if (!mLoadedSamples.empty()) mSampleCategories.insert("All");   // insert a category for all samples

            if (!unloadedSamplePlugins.empty())   // show error message summarising missing or invalid plugins
            {
                Ogre::String message = "These requested sample plugins were either missing, corrupt or invalid:";

                for (unsigned int i = 0; i < unloadedSamplePlugins.size(); i++)
                {
                    message += "\n- " + unloadedSamplePlugins[i];
                }

                mTrayMgr->showOkDialog("Error!", message);
            }

            return startupSample;
        }

        /*-----------------------------------------------------------------------------
          | Unloads any loaded sample plugins.
          -----------------------------------------------------------------------------*/
        virtual void unloadSamples()
        {
#ifdef OGRE_STATIC_LIB
            const Ogre::Root::PluginInstanceList pluginList = mRoot->getInstalledPlugins();
            for(unsigned int i = 0; i < pluginList.size(); i++)
            {
                SamplePlugin* sp = dynamic_cast<SamplePlugin*>(pluginList[i]);

                // This is a sample plugin so we can unload it
                if(!sp) continue;

                mRoot->uninstallPlugin(sp);
                delete sp;
            }
#else
            for (unsigned int i = 0; i < mLoadedSamplePlugins.size(); i++)
            {
                mRoot->unloadPlugin(mLoadedSamplePlugins[i]);
            }
#endif

            mLoadedSamples.clear();
            mLoadedSamplePlugins.clear();
            mSampleCategories.clear();
        }

        /*-----------------------------------------------------------------------------
          | Sets up main page for browsing samples.
          -----------------------------------------------------------------------------*/
        virtual void setupWidgets()
        {
            mTrayMgr->destroyAllWidgets();

            // create main navigation tray
            mTrayMgr->showLogo(TL_RIGHT);
            mTrayMgr->createSeparator(TL_RIGHT, "LogoSep");
            mTrayMgr->createButton(TL_RIGHT, "StartStop", "Start Sample", 120);

#if (OGRE_PLATFORM != OGRE_PLATFORM_WINRT) && (OGRE_PLATFORM != OGRE_PLATFORM_ANDROID)
            mTrayMgr->createButton(TL_RIGHT, "UnloadReload", mLoadedSamples.empty() ? "Reload Samples" : "Unload Samples");
#endif
#if (OGRE_PLATFORM != OGRE_PLATFORM_WINRT)
            mTrayMgr->createButton(TL_RIGHT, "Configure", "Configure");
#endif
#if (OGRE_PLATFORM != OGRE_PLATFORM_ANDROID)
            mTrayMgr->createButton(TL_RIGHT, "Quit", "Quit");
#endif

            // create sample viewing controls
            float infoWidth = 250;
#if (OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS) || (OGRE_PLATFORM == OGRE_PLATFORM_ANDROID)
            infoWidth *= 0.9;
#endif
            mTitleLabel = mTrayMgr->createLabel(TL_LEFT, "SampleTitle", "");
            mDescBox = mTrayMgr->createTextBox(TL_LEFT, "SampleInfo", "Sample Info", infoWidth, 208);
            mCategoryMenu = mTrayMgr->createThickSelectMenu(TL_LEFT, "CategoryMenu", "Select Category", infoWidth, 10);
            mSampleMenu = mTrayMgr->createThickSelectMenu(TL_LEFT, "SampleMenu", "Select Sample", infoWidth, 10);
            mSampleSlider = mTrayMgr->createThickSlider(TL_LEFT, "SampleSlider", "Slide Samples", infoWidth, 80, 0, 0, 0);

            /* Sliders do not notify their listeners on creation, so we manually call the callback here
               to format the slider value correctly. */
            sliderMoved(mSampleSlider);

            // create configuration screen button tray
            mTrayMgr->createButton(TL_NONE, "Apply", "Apply Changes");
            mTrayMgr->createButton(TL_NONE, "Back", "Go Back");

            // create configuration screen label and renderer menu
            mTrayMgr->createLabel(TL_NONE, "ConfigLabel", "Configuration");
            mRendererMenu = mTrayMgr->createLongSelectMenu(TL_NONE, "RendererMenu", "Render System", 450, 240, 10);
            mTrayMgr->createSeparator(TL_NONE, "ConfigSeparator");

            // populate render system names
            Ogre::StringVector rsNames;
            Ogre::RenderSystemList rsList = mRoot->getAvailableRenderers();
            for (unsigned int i = 0; i < rsList.size(); i++)
            {
                rsNames.push_back(rsList[i]->getName());
            }
            mRendererMenu->setItems(rsNames);

            populateSampleMenus();
        }

        /*-----------------------------------------------------------------------------
          | Populates home menus with loaded samples.
          -----------------------------------------------------------------------------*/
        virtual void populateSampleMenus()
        {
            Ogre::StringVector categories;
            for (std::set<Ogre::String>::iterator i = mSampleCategories.begin(); i != mSampleCategories.end(); i++)
                categories.push_back(*i);

            mCategoryMenu->setItems(categories);
            if (mCategoryMenu->getNumItems() != 0)
                mCategoryMenu->selectItem(0);
            else
                itemSelected(mCategoryMenu);   // if there are no items, we can't select one, so manually invoke callback
        }

        /*-----------------------------------------------------------------------------
          | Overrides to recover by last sample's index instead.
          -----------------------------------------------------------------------------*/
        virtual void recoverLastSample()
        {
            // restore the view while we're at it too
            mCategoryMenu->selectItem(mLastViewCategory);
            mSampleMenu->selectItem(mLastViewTitle);

            if (mLastSampleIndex != -1)
            {
                int index = -1;
                for (SampleSet::iterator i = mLoadedSamples.begin(); i != mLoadedSamples.end(); i++)
                {
                    index++;
                    if (index == mLastSampleIndex)
                    {
                        runSample(*i);
                        (*i)->restoreState(mLastSampleState);
                        mLastSample = 0;
                        mLastSampleIndex = -1;
                        mLastSampleState.clear();
                    }
                }

                pauseCurrentSample();
                mTrayMgr->showAll();
            }

            buttonHit((Button*)mTrayMgr->getWidget("Configure"));
        }

        /*-----------------------------------------------------------------------------
          | Extends reconfigure to save the view and the index of last sample run.
          -----------------------------------------------------------------------------*/
        virtual void reconfigure(const Ogre::String& renderer, Ogre::NameValuePairList& options)
        {
            mLastViewCategory = mCategoryMenu->getSelectionIndex();
            mLastViewTitle = mSampleMenu->getSelectionIndex();

            mLastSampleIndex = -1;
            unsigned int index = -1;
            for (SampleSet::iterator i = mLoadedSamples.begin(); i != mLoadedSamples.end(); i++)
            {
                index++;
                if (*i == mCurrentSample)
                {
                    mLastSampleIndex = index;
                    break;
                }
            }

            SampleContext::reconfigure(renderer, options);
        }
    public:
        /*-----------------------------------------------------------------------------
          | Extends shutdown to destroy dummy scene and tray interface.
          -----------------------------------------------------------------------------*/
        virtual void shutdown()
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            [mGestureView release];
#endif
            if (mTrayMgr)
            {
                delete mTrayMgr;
                mTrayMgr = 0;
            }

            if (!mCurrentSample && mRoot->getRenderSystem() != NULL) destroyDummyScene();

            SampleContext::shutdown();

            mCategoryMenu = 0;
            mSampleMenu = 0;
            mSampleSlider = 0;
            mTitleLabel = 0;
            mDescBox = 0;
            mRendererMenu = 0;
            mHiddenOverlays.clear();
            mThumbs.clear();
            mCarouselPlace = 0;
            mWindow = 0;

            unloadSamples();
        }
    protected:
        /*-----------------------------------------------------------------------------
          | Extend to temporarily hide a sample's overlays while in the pause menu.
          -----------------------------------------------------------------------------*/
        virtual void pauseCurrentSample()
        {
            SampleContext::pauseCurrentSample();

            Ogre::OverlayManager::OverlayMapIterator it = Ogre::OverlayManager::getSingleton().getOverlayIterator();
            mHiddenOverlays.clear();

            while (it.hasMoreElements())
            {
                Ogre::Overlay* o = it.getNext();
                if (o->isVisible())                  // later, we don't want to unhide the initially hidden overlays
                {
                    mHiddenOverlays.push_back(o);    // save off hidden overlays so we can unhide them later
                    o->hide();
                }
            }
        }

        /*-----------------------------------------------------------------------------
        | Extend to unhide all of sample's temporarily hidden overlays.
          -----------------------------------------------------------------------------*/
        virtual void unpauseCurrentSample()
        {
            SampleContext::unpauseCurrentSample();

            for (std::vector<Ogre::Overlay*>::iterator i = mHiddenOverlays.begin(); i != mHiddenOverlays.end(); i++)
            {
                (*i)->show();
            }

            mHiddenOverlays.clear();
        }

        /*-----------------------------------------------------------------------------
        | Get the name of the RTSS shader cache file
          -----------------------------------------------------------------------------*/
        virtual Ogre::String getShaderCacheFileName()
        {
#if OGRE_DEBUG_MODE
            return "cache_d.bin";
#else
            return "cache.bin";
#endif
        }

        TrayManager* mTrayMgr;                      // SDK tray interface
        Ogre::StringVector mLoadedSamplePlugins;       // loaded sample plugins
        std::set<Ogre::String> mSampleCategories;      // sample categories
        SampleSet mLoadedSamples;                      // loaded samples
        SelectMenu* mCategoryMenu;                     // sample category select menu
        SelectMenu* mSampleMenu;                       // sample select menu
        Slider* mSampleSlider;                         // sample slider bar
        Label* mTitleLabel;                            // sample title label
        TextBox* mDescBox;                             // sample description box
        SelectMenu* mRendererMenu;                     // render system selection menu
        std::vector<Ogre::Overlay*> mHiddenOverlays;   // sample overlays hidden for pausing
        std::vector<Ogre::OverlayContainer*> mThumbs;  // sample thumbnails
        Ogre::Real mCarouselPlace;                     // current state of carousel
        int mLastViewTitle;                            // last sample title viewed
        int mLastViewCategory;                         // last sample category viewed
        int mLastSampleIndex;                          // index of last sample running
        int mStartSampleIndex;                         // directly starts the sample with the given index
    public:
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        SampleBrowserGestureView *mGestureView;
#endif
        bool mIsShuttingDown;
        bool mGrabInput;
    };
}

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS

@implementation SampleBrowserGestureView

@synthesize mBrowser;

- (BOOL)canBecomeFirstResponder
{
    return YES;
}

- (void)dealloc {
    [super dealloc];
}

- (void)motionBegan:(UIEventSubtype)motion withEvent:(UIEvent *)event {
    if(mBrowser && event.type == UIEventTypeMotion && event.subtype == UIEventSubtypeMotionShake)
        mBrowser->motionBegan();

    if ([super respondsToSelector:@selector(motionBegan:withEvent:)]) {
        [super motionBegan:motion withEvent:event];
    }
}

- (void)motionEnded:(UIEventSubtype)motion withEvent:(UIEvent *)event {
    if(mBrowser && event.type == UIEventTypeMotion && event.subtype == UIEventSubtypeMotionShake)
        mBrowser->motionEnded();

    if ([super respondsToSelector:@selector(motionEnded:withEvent:)]) {
        [super motionEnded:motion withEvent:event];
    }
}

- (void)motionCancelled:(UIEventSubtype)motion withEvent:(UIEvent *)event {
    if(mBrowser && event.type == UIEventTypeMotion && event.subtype == UIEventSubtypeMotionShake)
        mBrowser->motionCancelled();

    if ([super respondsToSelector:@selector(motionCancelled:withEvent:)]) {
        [super motionCancelled:motion withEvent:event];
    }
}
@end

#endif

#endif
