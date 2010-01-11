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

#ifndef __SampleBrowser_H__
#define __SampleBrowser_H__

#include "SampleContext.h"
#include "SamplePlugin.h"
#include "SdkTrays.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
#include "macUtils.h"
#endif

#ifdef OGRE_STATIC_LIB
#include "BezierPatch.h"
#include "BSP.h"
#include "CameraTrack.h"
#include "CelShading.h"
#include "CubeMapping.h"
#include "Dot3Bump.h"
#include "DynTex.h"
#include "FacialAnimation.h"
#include "Fresnel.h"
#include "Grass.h"
#include "Lighting.h"
#include "OceanDemo.h"
#include "ParticleFX.h"
#if USE_RTSHADER_SYSTEM
#include "ShaderSystem.h"
#endif
#include "Shadows.h"
#include "SkeletalAnimation.h"
#include "SkyBox.h"
#include "SkyDome.h"
#include "SkyPlane.h"
#include "Smoke.h"
#include "SphereMapping.h"
#include "Terrain.h"
#include "TextureFX.h"
#include "Transparency.h"

typedef std::map<std::string, OgreBites::SdkSample *> PluginMap;

#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
namespace OgreBites
{
    class SampleBrowser;
};

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
	| dynamic configuration, resource reloading, node labelling, and more.
	=============================================================================*/
	class SampleBrowser : public SampleContext, public SdkTrayListener
	{
	public:

		SampleBrowser()
		{
			mTrayMgr = 0;
			mLastViewCategory = 0;
			mLastViewTitle = 0;
			mLastSampleIndex = -1;
			mCategoryMenu = 0;
			mSampleMenu = 0;
			mSampleSlider = 0;
			mTitleLabel = 0;
			mDescBox = 0;
			mRendererMenu = 0;
			mCarouselPlace = 0.0f;
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
			mGestureView = 0;
#endif
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
					SampleContext::runSample(s);
				}
				catch (Ogre::Exception e)   // if failed to start, show error and fall back to menu
				{
					s->_shutdown();

					createDummyScene();
					mTrayMgr->showBackdrop("SdkTrays/Bands");
					mTrayMgr->showAll();
					((Button*)mTrayMgr->getWidget("StartStop"))->setCaption("Start Sample");

					mTrayMgr->showOkDialog("Error!", e.getDescription() + "\nSource: " + e.getSource());
				}
			}
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

			mTrayMgr->frameRenderingQueued(evt);

			try
			{
				return SampleContext::frameRenderingQueued(evt);
			}
			catch (Ogre::Exception e)   // show error and fall back to menu
			{
				runSample(0);
				mTrayMgr->showOkDialog("Error!", e.getDescription() + "\nSource: " + e.getSource());
			}

			return true;
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
					if (mLoadedSamples.empty()) mTrayMgr->showOkDialog("Error!", "No sample selected!");
					// use the sample pointer we stored inside the thumbnail
					else runSample(Ogre::any_cast<Sample*>(mThumbs[mSampleMenu->getSelectionIndex()]->getUserAny()));
				}
				else runSample(0);
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
					catch (Ogre::Exception e) {}
				}
			}
			else if (b->getName() == "Configure")   // enter configuration screen
			{
				mTrayMgr->removeWidgetFromTray("StartStop");
				mTrayMgr->removeWidgetFromTray("UnloadReload");
				mTrayMgr->removeWidgetFromTray("Configure");
				mTrayMgr->removeWidgetFromTray("Quit");
				mTrayMgr->moveWidgetToTray("Apply", TL_RIGHT);
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
				while (mTrayMgr->getNumWidgets(mRendererMenu->getTrayLocation()) > 3)
				{
					mTrayMgr->destroyWidget(mRendererMenu->getTrayLocation(), 3);
				}

				while (mTrayMgr->getNumWidgets(TL_NONE) != 0)
				{
					mTrayMgr->moveWidgetToTray(TL_NONE, 0, TL_LEFT);
				}

				mTrayMgr->removeWidgetFromTray("Apply");
				mTrayMgr->removeWidgetFromTray("Back");
				mTrayMgr->removeWidgetFromTray("ConfigLabel");
				mTrayMgr->removeWidgetFromTray(mRendererMenu);
				mTrayMgr->removeWidgetFromTray("ConfigSeparator");

				mTrayMgr->moveWidgetToTray("StartStop", TL_RIGHT);
				mTrayMgr->moveWidgetToTray("UnloadReload", TL_RIGHT);
				mTrayMgr->moveWidgetToTray("Configure", TL_RIGHT);
				mTrayMgr->moveWidgetToTray("Quit", TL_RIGHT);

				windowResized(mWindow);
			}
			else if (b->getName() == "Apply")   // apply any changes made in the configuration screen
			{
				bool reset = false;

				Ogre::ConfigOptionMap& options =
					mRoot->getRenderSystemByName(mRendererMenu->getSelectedItem())->getConfigOptions();

				Ogre::NameValuePairList newOptions;

				// collect new settings and decide if a reset is needed

				if (mRendererMenu->getSelectedItem() != mRoot->getRenderSystem()->getName()) reset = true;

				for (unsigned int i = 3; i < mTrayMgr->getNumWidgets(mRendererMenu->getTrayLocation()); i++)
				{
					SelectMenu* menu = (SelectMenu*)mTrayMgr->getWidget(mRendererMenu->getTrayLocation(), i);
					if (menu->getSelectedItem() != options[menu->getCaption()].currentValue) reset = true;
					newOptions[menu->getCaption()] = menu->getSelectedItem();
				}

				// reset with new settings if necessary
				if (reset) reconfigure(mRendererMenu->getSelectedItem(), newOptions);
			}
			else mRoot->queueEndRendering();   // exit browser
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
					Ogre::MaterialManager::getSingleton().remove(mThumbs[i]->getName());
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
				Ogre::MaterialPtr templateMat = Ogre::MaterialManager::getSingleton().getByName("SampleThumbnail");

				// populate the sample menu and carousel with filtered samples
				for (SampleSet::iterator i = mLoadedSamples.begin(); i != mLoadedSamples.end(); i++)
				{
					Ogre::NameValuePairList& info = (*i)->getInfo();

					if (all || info["Category"] == selectedCategory)
					{
						Ogre::String name = "SampleThumb" + Ogre::StringConverter::toString(sampleTitles.size() + 1);

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
						bp->setUserAny(Ogre::Any(*i));
						mTrayMgr->getTraysLayer()->add2D(bp);

						// add sample thumbnail and title
						mThumbs.push_back(bp);
						sampleTitles.push_back((*i)->getInfo()["Title"]);
					}
				}

				mCarouselPlace = 0;  // reset carousel

				mSampleMenu->setItems(sampleTitles);
				if (mSampleMenu->getNumItems() != 0) itemSelected(mSampleMenu);

				mSampleSlider->setRange(1, sampleTitles.size(), sampleTitles.size());
			}
			else if (menu == mSampleMenu)    // sample changed, so update slider, label and description
			{
				if (mSampleSlider->getValue() != menu->getSelectionIndex() + 1)
					mSampleSlider->setValue(menu->getSelectionIndex() + 1); 

				Sample* s = Ogre::any_cast<Sample*>(mThumbs[menu->getSelectionIndex()]->getUserAny());
				mTitleLabel->setCaption(menu->getSelectedItem()); 
				mDescBox->setText("Category: " + s->getInfo()["Category"] + "\nDescription: " + s->getInfo()["Description"]);

				if (mCurrentSample != s) ((Button*)mTrayMgr->getWidget("StartStop"))->setCaption("Start Sample");
				else ((Button*)mTrayMgr->getWidget("StartStop"))->setCaption("Stop Sample");
			}
			else if (menu == mRendererMenu)    // renderer selected, so update all settings
			{
				while (mTrayMgr->getNumWidgets(mRendererMenu->getTrayLocation()) > 3)
				{
					mTrayMgr->destroyWidget(mRendererMenu->getTrayLocation(), 3);
				}

				Ogre::ConfigOptionMap& options = mRoot->getRenderSystemByName(menu->getSelectedItem())->getConfigOptions();

				unsigned int i = 0;

				// create all the config option select menus
				for (Ogre::ConfigOptionMap::iterator it = options.begin(); it != options.end(); it++)
				{
					i++;
					SelectMenu* optionMenu = mTrayMgr->createLongSelectMenu
						(TL_LEFT, "ConfigOption" + Ogre::StringConverter::toString(i), it->first, 450, 240, 10);
					optionMenu->setItems(it->second.possibleValues);
					
					// if the current config value is not in the menu, add it
					try
					{
						optionMenu->selectItem(it->second.currentValue);
					}
					catch (Ogre::Exception e)
					{
						optionMenu->addItem(it->second.currentValue);
						optionMenu->selectItem(it->second.currentValue);
					}
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
		virtual bool keyPressed(const OIS::KeyEvent& evt)
		{
			if (mTrayMgr->isDialogVisible()) return true;  // ignore keypresses when dialog is showing

			if (evt.key == OIS::KC_ESCAPE)
			{
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
			else if (evt.key == OIS::KC_UP || evt.key == OIS::KC_DOWN && mTitleLabel->getTrayLocation() != TL_NONE)
			{
				// if we're in the main screen, use the up and down arrow keys to cycle through samples
				int newIndex = mSampleMenu->getSelectionIndex() + (evt.key == OIS::KC_UP ? -1 : 1);
				mSampleMenu->selectItem(Ogre::Math::Clamp<int>(newIndex, 0, mSampleMenu->getNumItems() - 1));
			}
			else if (evt.key == OIS::KC_RETURN)   // start or stop sample
			{
				if (!mLoadedSamples.empty() && (mSamplePaused || mCurrentSample == 0))
				{
					Sample* newSample = Ogre::any_cast<Sample*>(mThumbs[mSampleMenu->getSelectionIndex()]->getUserAny());
					runSample(newSample == mCurrentSample ? 0 : newSample);
				}
			}
#if OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
            else if (evt.key == OIS::KC_M)   // change orientation mode
            {
                unsigned int orientationMode = (unsigned int)mWindow->getViewport(0)->getOrientationMode();
                orientationMode++;
                if (orientationMode >= 4)
                    orientationMode = 0;
                mWindow->getViewport(0)->setOrientationMode((Ogre::OrientationMode)orientationMode);                
            }
#endif

			try
			{
				return SampleContext::keyPressed(evt);
			}
			catch (Ogre::Exception e)   // show error and fall back to menu
			{
				runSample(0);
				mTrayMgr->showOkDialog("Error!", e.getDescription() + "\nSource: " + e.getSource());
			}

			return true;
		}

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
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
		| Extends mousePressed to inject mouse press into tray manager, and to check
		| for thumbnail clicks, just because we can.
		-----------------------------------------------------------------------------*/
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
		virtual bool touchPressed(const OIS::MultiTouchEvent& evt)
#else
		virtual bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
#endif
		{
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
            OIS::MultiTouchState state = evt.state;
            transformInputState(state);
            OIS::MultiTouchEvent orientedEvt((OIS::Object*)evt.device, state);
#else
            OIS::MouseState state = evt.state;
    #if OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
            transformInputState(state);
    #endif
            OIS::MouseEvent orientedEvt((OIS::Object*)evt.device, state);
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
			if (mTrayMgr->injectMouseDown(orientedEvt)) return true;
#else
			if (mTrayMgr->injectMouseDown(orientedEvt, id)) return true;
#endif
            
			if (mTitleLabel->getTrayLocation() != TL_NONE)
			{
				for (unsigned int i = 0; i < mThumbs.size(); i++)
				{
					if (mThumbs[i]->isVisible() && Widget::isCursorOver(mThumbs[i],
                            Ogre::Vector2(mTrayMgr->getCursorContainer()->getLeft(),
                                          mTrayMgr->getCursorContainer()->getTop()), 0))
					{
						mSampleMenu->selectItem(i);
						break;
					}
				}
			}
            
			try
			{
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
				return SampleContext::touchPressed(orientedEvt);
#else
				return SampleContext::mousePressed(orientedEvt, id);
#endif
			}
			catch (Ogre::Exception e)   // show error and fall back to menu
			{
				runSample(0);
				mTrayMgr->showOkDialog("Error!", e.getDescription() + "\nSource: " + e.getSource());
			}
            
			return true;
		}

		/*-----------------------------------------------------------------------------
		| Extends mouseReleased to inject mouse release into tray manager.
		-----------------------------------------------------------------------------*/
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
		virtual bool touchReleased(const OIS::MultiTouchEvent& evt)
#else
		virtual bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
#endif
		{
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
            OIS::MultiTouchState state = evt.state;
            transformInputState(state);
            OIS::MultiTouchEvent orientedEvt((OIS::Object*)evt.device, state);
#else
            OIS::MouseState state = evt.state;
    #if OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
            transformInputState(state);
    #endif
            OIS::MouseEvent orientedEvt((OIS::Object*)evt.device, state);
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
			if (mTrayMgr->injectMouseUp(orientedEvt)) return true;
#else
			if (mTrayMgr->injectMouseUp(orientedEvt, id)) return true;
#endif

			try
			{
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
				return SampleContext::touchReleased(orientedEvt);
#else
				return SampleContext::mouseReleased(orientedEvt, id);
#endif
			}
			catch (Ogre::Exception e)   // show error and fall back to menu
			{
				runSample(0);
				mTrayMgr->showOkDialog("Error!", e.getDescription() + "\nSource: " + e.getSource());
			}
            
			return true;
		}

		/*-----------------------------------------------------------------------------
		| Extends mouseMoved to inject mouse position into tray manager, and checks
		| for mouse wheel movements to slide the carousel, because we can.
		-----------------------------------------------------------------------------*/
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
		virtual bool touchMoved(const OIS::MultiTouchEvent& evt)
#else
		virtual bool mouseMoved(const OIS::MouseEvent& evt)
#endif
		{
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
            OIS::MultiTouchState state = evt.state;
            transformInputState(state);
            OIS::MultiTouchEvent orientedEvt((OIS::Object*)evt.device, state);
#else
            OIS::MouseState state = evt.state;
    #if OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
            transformInputState(state);
    #endif
            OIS::MouseEvent orientedEvt((OIS::Object*)evt.device, state);
#endif

			if (mTrayMgr->injectMouseMove(orientedEvt)) return true;
            
			if (!(mCurrentSample && !mSamplePaused) && mTitleLabel->getTrayLocation() != TL_NONE &&
				orientedEvt.state.Z.rel != 0 && mSampleMenu->getNumItems() != 0)
			{
				int newIndex = mSampleMenu->getSelectionIndex() - orientedEvt.state.Z.rel / Ogre::Math::Abs(orientedEvt.state.Z.rel);
				mSampleMenu->selectItem(Ogre::Math::Clamp<int>(newIndex, 0, mSampleMenu->getNumItems() - 1));
			}
            
			try
			{
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
				return SampleContext::touchMoved(orientedEvt);
#else
				return SampleContext::mouseMoved(orientedEvt);
#endif
			}
			catch (Ogre::Exception e)   // show error and fall back to menu
			{
				runSample(0);
				mTrayMgr->showOkDialog("Error!", e.getDescription() + "\nSource: " + e.getSource());
			}
            
			return true;
		}

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
        /*-----------------------------------------------------------------------------
         | Extends touchCancelled to inject an event that a touch was cancelled.
         -----------------------------------------------------------------------------*/
		virtual bool touchCancelled(const OIS::MultiTouchEvent& evt)
        {
            return true;
        }
#endif
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
					mTrayMgr->moveWidgetToTray(mTrayMgr->getWidget(TL_CENTER, 0), TL_LEFT);
				}
			}
			else if (left->isVisible() && rw->getWidth() >= 1280 - left->getWidth())
			{
				while (left->isVisible())
				{
					mTrayMgr->moveWidgetToTray(mTrayMgr->getWidget(TL_LEFT, 0), TL_CENTER);
				}
			}

			SampleContext::windowResized(rw);
		}

        /*-----------------------------------------------------------------------------
         | Extends setup to create dummy scene and tray interface.
         -----------------------------------------------------------------------------*/
		virtual void setup()
		{
#ifdef OGRE_STATIC_LIB
            mPluginNameMap["Sample_BezierPatch"]        = (OgreBites::SdkSample *) OGRE_NEW Sample_BezierPatch();
            mPluginNameMap["Sample_BSP"]                = (OgreBites::SdkSample *) OGRE_NEW Sample_BSP();
            mPluginNameMap["Sample_CameraTrack"]        = (OgreBites::SdkSample *) OGRE_NEW Sample_CameraTrack();
            mPluginNameMap["Sample_CelShading"]         = (OgreBites::SdkSample *) OGRE_NEW Sample_CelShading();
            mPluginNameMap["Sample_CubeMapping"]        = (OgreBites::SdkSample *) OGRE_NEW Sample_CubeMapping();
            mPluginNameMap["Sample_Dot3Bump"]           = (OgreBites::SdkSample *) OGRE_NEW Sample_Dot3Bump();
            mPluginNameMap["Sample_DynTex"]             = (OgreBites::SdkSample *) OGRE_NEW Sample_DynTex();
            mPluginNameMap["Sample_FacialAnimation"]    = (OgreBites::SdkSample *) OGRE_NEW Sample_FacialAnimation();
            mPluginNameMap["Sample_Fresnel"]            = (OgreBites::SdkSample *) OGRE_NEW Sample_Fresnel();
            mPluginNameMap["Sample_Grass"]              = (OgreBites::SdkSample *) OGRE_NEW Sample_Grass();
            mPluginNameMap["Sample_Lighting"]           = (OgreBites::SdkSample *) OGRE_NEW Sample_Lighting();
            mPluginNameMap["Sample_Ocean"]              = (OgreBites::SdkSample *) OGRE_NEW Sample_Ocean();
            mPluginNameMap["Sample_ParticleFX"]         = (OgreBites::SdkSample *) OGRE_NEW Sample_ParticleFX();
#if USE_RTSHADER_SYSTEM
            mPluginNameMap["Sample_ShaderSystem"]       = (OgreBites::SdkSample *) OGRE_NEW Sample_ShaderSystem();
#endif
            mPluginNameMap["Sample_Shadows"]            = (OgreBites::SdkSample *) OGRE_NEW Sample_Shadows();
            mPluginNameMap["Sample_SkeletalAnimation"]  = (OgreBites::SdkSample *) OGRE_NEW Sample_SkeletalAnimation();
            mPluginNameMap["Sample_SkyBox"]             = (OgreBites::SdkSample *) OGRE_NEW Sample_SkyBox();
            mPluginNameMap["Sample_SkyDome"]            = (OgreBites::SdkSample *) OGRE_NEW Sample_SkyDome();
            mPluginNameMap["Sample_SkyPlane"]           = (OgreBites::SdkSample *) OGRE_NEW Sample_SkyPlane();
            mPluginNameMap["Sample_Smoke"]              = (OgreBites::SdkSample *) OGRE_NEW Sample_Smoke();
            mPluginNameMap["Sample_SphereMapping"]      = (OgreBites::SdkSample *) OGRE_NEW Sample_SphereMapping();
            mPluginNameMap["Sample_Terrain"]            = (OgreBites::SdkSample *) OGRE_NEW Sample_Terrain();
            mPluginNameMap["Sample_TextureFX"]          = (OgreBites::SdkSample *) OGRE_NEW Sample_TextureFX();
            mPluginNameMap["Sample_Transparency"]       = (OgreBites::SdkSample *) OGRE_NEW Sample_Transparency();
#endif
            
			createWindow();
			setupInput();
			locateResources();
            
			Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Essential");
            
			mTrayMgr = new SdkTrayManager("BrowserControls", mWindow, mMouse, this);
			mTrayMgr->showBackdrop("SdkTrays/Bands");
			mTrayMgr->getTrayContainer(TL_NONE)->hide();
            
			createDummyScene();
			loadResources();

            if (Ogre::Root::getSingletonPtr()->getRenderSystem()->getName().find("OpenGL ES 2") != Ogre::String::npos)
            {
                Ogre::RTShader::ShaderGenerator::getSingletonPtr()->addSceneManager(mRoot->getSceneManager("DummyScene"));
                mWindow->getViewport(0)->setMaterialScheme(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
            }

			Sample* startupSample = loadSamples();
            
			Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
            
			// adds context as listener to process context-level (above the sample level) events
			mRoot->addFrameListener(this);
			Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);
            
			// create template material for sample thumbnails
			Ogre::MaterialPtr thumbMat = Ogre::MaterialManager::getSingleton().create("SampleThumbnail", "Essential");
			thumbMat->getTechnique(0)->getPass(0)->createTextureUnitState();
            
			setupWidgets();
			windowResized(mWindow);   // adjust menus for resolution

			// if this is our first time running, and there's a startup sample, run it
			if (startupSample && mFirstRun) runSample(startupSample);
		}
        
	protected:

		/*-----------------------------------------------------------------------------
		| Restores config instead of using a dialog to save time.
		| If that fails, the config dialog is shown.
		-----------------------------------------------------------------------------*/
		virtual bool oneTimeConfig()
		{
			if (!mRoot->restoreConfig()) return mRoot->showConfigDialog();
			return true;
		}		

		/*-----------------------------------------------------------------------------
		| Overrides the default window title.
		-----------------------------------------------------------------------------*/
		virtual void createWindow()
		{
			mWindow = mRoot->initialise(true, "OGRE Sample Browser");

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
            mGestureView = [[SampleBrowserGestureView alloc] init];
            mGestureView.mBrowser = this;
            
            [[[UIApplication sharedApplication] keyWindow] addSubview:mGestureView];
#endif
        }

		/*-----------------------------------------------------------------------------
		| Initialises only the browser's resources and those most commonly used
		| by samples. This way, additional special content can be initialised by
		| the samples that use them, so startup time is unaffected.
		-----------------------------------------------------------------------------*/
		virtual void loadResources()
		{
			mTrayMgr->showLoadingBar(1, 0);
			Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Popular");
			mTrayMgr->hideLoadingBar();
		}

		/*-----------------------------------------------------------------------------
		| Creates dummy scene to allow rendering GUI in viewport.
		-----------------------------------------------------------------------------*/
		virtual void createDummyScene()
		{
			mWindow->removeAllViewports();
			Ogre::SceneManager* sm = mRoot->createSceneManager(Ogre::ST_GENERIC, "DummyScene");
			Ogre::Camera* cam = sm->createCamera("DummyCamera");
			mWindow->addViewport(cam);
		}

		/*-----------------------------------------------------------------------------
		| Loads sample plugins from a configuration file.
		-----------------------------------------------------------------------------*/
		virtual Sample* loadSamples()
		{
			Sample* startupSample = 0;

			Ogre::StringVector unloadedSamplePlugins;

			Ogre::ConfigFile cfg;
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
			cfg.load(Ogre::macBundlePath() + "/Contents/Resources/samples.cfg");
#elif OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
			cfg.load(Ogre::macBundlePath() + "/samples.cfg");
#else
			cfg.load("samples.cfg");
#endif

			Ogre::String sampleDir = cfg.getSetting("SampleFolder");        // Mac OS X just uses Resources/ directory
			Ogre::StringVector sampleList = cfg.getMultiSetting("SamplePlugin");
			Ogre::String startupSampleTitle = cfg.getSetting("StartupSample");

			#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE && OGRE_PLATFORM != OGRE_PLATFORM_IPHONE
			if (sampleDir.empty()) sampleDir = ".";   // user didn't specify plugins folder, try current one
			#endif

			// add slash or backslash based on platform
			char lastChar = sampleDir[sampleDir.length() - 1];
			if (lastChar != '/' && lastChar != '\\')
			{
				#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
				sampleDir += "\\";
				#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX
				sampleDir += "/";
				#endif
			}

			// loop through all sample plugins...
			for (Ogre::StringVector::iterator i = sampleList.begin(); i != sampleList.end(); i++)
			{
				try   // try to load the plugin
				{
#ifdef OGRE_STATIC_LIB
                    OgreBites::SdkSample *pluginInstance = (OgreBites::SdkSample *) mPluginNameMap[*i];
                    if(pluginInstance)
                    {
                        OgreBites::SamplePlugin* sp = OGRE_NEW SamplePlugin(pluginInstance->getInfo()["Title"] + " Sample");

                        sp->addSample(pluginInstance);
                        mRoot->installPlugin(sp);
                    }
#else
					mRoot->loadPlugin(sampleDir + *i);
#endif
				}
				catch (Ogre::Exception e)   // plugin couldn't be loaded
				{
					unloadedSamplePlugins.push_back(sampleDir + *i);
					continue;
				}

				Ogre::Plugin* p = mRoot->getInstalledPlugins().back();   // acquire plugin instance
				SamplePlugin* sp = dynamic_cast<SamplePlugin*>(p);

				if (!sp)  // this is not a SamplePlugin, so unload it
				{
					unloadedSamplePlugins.push_back(sampleDir + *i); 
#ifdef OGRE_STATIC_LIB
					mRoot->uninstallPlugin(p);
#else
					mRoot->unloadPlugin(sampleDir + *i);
#endif
					continue;
				}
                
				mLoadedSamplePlugins.push_back(sampleDir + *i);   // add to records

				// go through every sample in the plugin...
				SampleSet newSamples = sp->getSamples();
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
                if(sp)
                    mRoot->uninstallPlugin(pluginList[i]);
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
			mTrayMgr->createButton(TL_RIGHT, "StartStop", "Start Sample");
			mTrayMgr->createButton(TL_RIGHT, "UnloadReload", mLoadedSamples.empty() ? "Reload Samples" : "Unload Samples");
			mTrayMgr->createButton(TL_RIGHT, "Configure", "Configure");
			mTrayMgr->createButton(TL_RIGHT, "Quit", "Quit");

			// create sample viewing controls
			mTitleLabel = mTrayMgr->createLabel(TL_LEFT, "SampleTitle", "");
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
			mDescBox = mTrayMgr->createTextBox(TL_LEFT, "SampleInfo", "Sample Info", 120, 100);
			mCategoryMenu = mTrayMgr->createThickSelectMenu(TL_LEFT, "CategoryMenu", "Select Category", 120, 10); 
			mSampleMenu = mTrayMgr->createThickSelectMenu(TL_LEFT, "SampleMenu", "Select Sample", 120, 10);
			mSampleSlider = mTrayMgr->createThickSlider(TL_LEFT, "SampleSlider", "Slide Samples", 120, 42, 0, 0, 0);
#else
			mDescBox = mTrayMgr->createTextBox(TL_LEFT, "SampleInfo", "Sample Info", 250, 208);
			mCategoryMenu = mTrayMgr->createThickSelectMenu(TL_LEFT, "CategoryMenu", "Select Category", 250, 10); 
			mSampleMenu = mTrayMgr->createThickSelectMenu(TL_LEFT, "SampleMenu", "Select Sample", 250, 10);
			mSampleSlider = mTrayMgr->createThickSlider(TL_LEFT, "SampleSlider", "Slide Samples", 250, 80, 0, 0, 0);
#endif
			/* Sliders do not notify their listeners on creation, so we manually call the callback here
			to format the slider value correctly. */
			sliderMoved(mSampleSlider);

			// create configuration screen button tray
			mTrayMgr->createButton(TL_NONE, "Apply", "Apply Changes");
			mTrayMgr->createButton(TL_NONE, "Back", "Go Back");

			// create configuration screen label and renderer menu
			mTrayMgr->createLabel(TL_NONE, "ConfigLabel", "Configuration");
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
			mRendererMenu = mTrayMgr->createLongSelectMenu(TL_NONE, "RendererMenu", "Render System", 216, 115, 10);
#else
			mRendererMenu = mTrayMgr->createLongSelectMenu(TL_NONE, "RendererMenu", "Render System", 450, 240, 10);
#endif
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

		/*-----------------------------------------------------------------------------
		| Extends shutdown to destroy dummy scene and tray interface.
		-----------------------------------------------------------------------------*/
		virtual void shutdown()
		{
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
            [mGestureView release];
#endif
			if (mTrayMgr)
			{
				delete mTrayMgr;
				mTrayMgr = 0;
			}

			if (!mCurrentSample) destroyDummyScene();

			mCategoryMenu = 0;
			mSampleMenu = 0;
			mSampleSlider = 0;
			mTitleLabel = 0;
			mDescBox = 0;
			mRendererMenu = 0;
			mHiddenOverlays.clear();
			mThumbs.clear();
			mCarouselPlace = 0;

			SampleContext::shutdown();

			unloadSamples();
		}

		/*-----------------------------------------------------------------------------
		| Destroys dummy scene.
		-----------------------------------------------------------------------------*/
		virtual void destroyDummyScene()
		{
			mWindow->removeAllViewports();
			mRoot->destroySceneManager(mRoot->getSceneManager("DummyScene"));
		}	

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
		| Extend to unnhide all of sample's temporarily hidden overlays.
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

		SdkTrayManager* mTrayMgr;                      // SDK tray interface
#ifdef OGRE_STATIC_LIB
        PluginMap mPluginNameMap;                      // A structure to map plugin names to class types
#endif
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
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
    public:
        SampleBrowserGestureView *mGestureView;
#endif
	};
}

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE

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
