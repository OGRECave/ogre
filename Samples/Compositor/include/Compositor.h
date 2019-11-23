/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

  You may use this sample code for anything you like, it is not covered by the
  same license as the rest of the engine.
  -----------------------------------------------------------------------------
*/

#ifndef __CompositorDemo_H__
#define __CompositorDemo_H__

#include "OgreConfigFile.h"
#include "OgreStringConverter.h"
#include "OgreException.h"

#include "SdkSample.h"
#include "SamplePlugin.h"

using namespace Ogre;
using namespace OgreBites;

#define COMPOSITORS_PER_PAGE 8

class _OgreSampleClassExport Sample_Compositor : public SdkSample
{
 public:
    Sample_Compositor();

    void setupContent(void);
    void cleanupContent(void);

    bool frameRenderingQueued(const FrameEvent& evt);

    void checkBoxToggled(OgreBites::CheckBox * box);
    void buttonHit(OgreBites::Button* button);
    void itemSelected(OgreBites::SelectMenu* menu);

 protected:

    void setupView(void);
    void setupControls(void);
    void setupScene(void);
    void createEffects(void);
    void createTextures(void);

    void registerCompositors();
    void changePage(size_t pageNum);

    SceneNode * mSpinny;
    StringVector mCompositorNames;
    size_t mActiveCompositorPage;
    size_t mNumCompositorPages;

    //Used to unregister compositor logics and free memory
    typedef std::map<String, CompositorLogic*> CompositorLogicMap;
    CompositorLogicMap mCompositorLogics;

    String mDebugCompositorName;
    SelectMenu* mDebugTextureSelectMenu;
    TextureUnitState* mDebugTextureTUS;

};

/**
   @file
   Compositor.cpp
   @brief
   Shows OGRE's Compositor feature
   @author
   W.J. :wumpus: van der Laan
   Ogre compositor framework
   Manuel Bua
   Postfilter ideas and original out-of-core implementation
   Jeff (nfz) Doyle
   added gui framework to demo
*/

#include "HelperLogics.h"

/*************************************************************************
                            Sample_Compositor Methods
*************************************************************************/
Sample_Compositor::Sample_Compositor()
{
    mInfo["Title"] = "Compositor";
    mInfo["Description"] = "A demo of Ogre's post-processing framework.";
    mInfo["Thumbnail"] = "thumb_comp.png";
    mInfo["Category"] = "Effects";
}


void Sample_Compositor::setupView()
{
    SdkSample::setupView();
    mCameraNode->setPosition(Ogre::Vector3(0,0,0));
    mCameraNode->lookAt(Ogre::Vector3(0,0,-300), Ogre::Node::TS_PARENT);
    mCamera->setNearClipDistance(1);
}


void Sample_Compositor::setupContent(void)
{
    // Register the compositor logics
    // See comment in beginning of HelperLogics.h for explanation
    Ogre::CompositorManager& compMgr = Ogre::CompositorManager::getSingleton();
    mCompositorLogics["GaussianBlur"] = new GaussianBlurLogic;
    mCompositorLogics["HDR"] = new HDRLogic;
    mCompositorLogics["HeatVision"] = new HeatVisionLogic;
    compMgr.registerCompositorLogic("GaussianBlur", mCompositorLogics["GaussianBlur"]);
    compMgr.registerCompositorLogic("HDR", mCompositorLogics["HDR"]);
    compMgr.registerCompositorLogic("HeatVision", mCompositorLogics["HeatVision"]);

#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
    mShaderGenerator->createScheme("HDR"); // make sure HDR viewport is handled
#endif

    createTextures();
    /// Create a couple of hard coded postfilter effects as an example of how to do it
    /// but the preferred method is to use compositor scripts.
    createEffects();

    setupScene();

    registerCompositors();

    setupControls();

#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
    setDragLook(true);
#endif
}

void Sample_Compositor::registerCompositors(void)
{
    Ogre::Viewport *vp = mViewport;

    //iterate through Compositor Managers resources and add name keys to menu
    Ogre::CompositorManager::ResourceMapIterator resourceIterator =
        Ogre::CompositorManager::getSingleton().getResourceIterator();

    // add all compositor resources to the view container
    while (resourceIterator.hasMoreElements())
    {
        Ogre::ResourcePtr resource = resourceIterator.getNext();
        const Ogre::String& compositorName = resource->getName();
        // Don't add base Ogre/Scene compositor to view
        if (Ogre::StringUtil::startsWith(compositorName, "Ogre/Scene/", false))
            continue;
        // Don't add the deferred shading compositors, thats a different demo.
        if (Ogre::StringUtil::startsWith(compositorName, "DeferredShading", false))
            continue;
        // Don't add the SSAO compositors, thats a different demo.
        if (Ogre::StringUtil::startsWith(compositorName, "SSAO", false))
            continue;
        // Don't add the TestMRT compositor, it needs extra scene setup so doesn't currently work.
        if (Ogre::StringUtil::startsWith(compositorName, "TestMRT", false))
            continue;
        // Don't add the Compute compositors, thats a different demo.
        if (Ogre::StringUtil::startsWith(compositorName, "Compute", false))
            continue;

        mCompositorNames.push_back(compositorName);
        int addPosition = -1;
        if (compositorName == "HDR")
        {
            // HDR must be first in the chain
            addPosition = 0;
        }
        try
        {
            Ogre::CompositorManager::getSingleton().addCompositor(vp, compositorName, addPosition);
            Ogre::CompositorManager::getSingleton().setCompositorEnabled(vp, compositorName, false);
        } catch (...) {
            /// Warn user
            LogManager::getSingleton().logMessage("Could not load compositor " + compositorName, LML_CRITICAL);
        }
    }

    mNumCompositorPages = (mCompositorNames.size() / COMPOSITORS_PER_PAGE) +
        ((mCompositorNames.size() % COMPOSITORS_PER_PAGE == 0) ? 0 : 1);
}


void Sample_Compositor::changePage(size_t pageNum)
{
    assert(pageNum < mNumCompositorPages);

    mActiveCompositorPage = pageNum;
    size_t maxCompositorsInPage = mCompositorNames.size() - (pageNum * COMPOSITORS_PER_PAGE);
    for (size_t i=0; i < COMPOSITORS_PER_PAGE; i++)
    {
        String checkBoxName = "Compositor_" + Ogre::StringConverter::toString(i);
        CheckBox* cb = static_cast<CheckBox*>(mTrayMgr->getWidget(TL_TOPLEFT, checkBoxName));
        if (i < maxCompositorsInPage)
        {
            String compositorName = mCompositorNames[pageNum * COMPOSITORS_PER_PAGE + i];
            CompositorInstance *tmpCompo = CompositorManager::getSingleton().getCompositorChain(mViewport)
                ->getCompositor(compositorName);

            cb->setCaption(compositorName);

            if( tmpCompo )
            {
                cb->setChecked( tmpCompo->getEnabled(), false );
                cb->show();
            }
            else
            {
                cb->setChecked( false, false );
                cb->hide();
            }

        }
        else
        {
            cb->hide();
        }
    }

    OgreBites::Button* pageButton = static_cast<OgreBites::Button*>(mTrayMgr->getWidget(TL_TOPLEFT, "PageButton"));
    Ogre::StringStream ss;
    ss << "Compositors " << pageNum + 1 << "/" << mNumCompositorPages;
    pageButton->setCaption(ss.str());
}


void Sample_Compositor::cleanupContent(void)
{
    mDebugTextureTUS->setContentType(TextureUnitState::CONTENT_NAMED);
    CompositorManager::getSingleton().removeCompositorChain(mViewport);
    mCompositorNames.clear();

    TextureManager::getSingleton().remove("DitherTex", "General");
    TextureManager::getSingleton().remove("HalftoneVolume", "General");

    Ogre::CompositorManager& compMgr = Ogre::CompositorManager::getSingleton();
    CompositorLogicMap::const_iterator itor = mCompositorLogics.begin();
    CompositorLogicMap::const_iterator end  = mCompositorLogics.end();
    while( itor != end )
    {
        compMgr.unregisterCompositorLogic( itor->first );
        delete itor->second;
        ++itor;
    }
    mCompositorLogics.clear();
    MeshManager::getSingleton().remove("Myplane", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
}


void Sample_Compositor::setupControls(void)
{
    mTrayMgr->createButton(TL_TOPLEFT, "PageButton", "Compositors", 175);

    for (size_t i=0; i < COMPOSITORS_PER_PAGE; i++)
    {
        String checkBoxName = "Compositor_" + Ogre::StringConverter::toString(i);
        CheckBox* cb = mTrayMgr->createCheckBox(TL_TOPLEFT, checkBoxName, "Compositor", 175);
        cb->hide();
    }

    changePage(0);

    mDebugTextureSelectMenu = mTrayMgr->createThickSelectMenu(TL_TOPRIGHT, "DebugRTTSelectMenu", "Debug RTT", 180, 5);
    mDebugTextureSelectMenu->addItem("None");

    mTrayMgr->createSeparator(TL_TOPRIGHT, "DebugRTTSep1");  // this is a hack to give the debug RTT a bit more room

    DecorWidget* debugRTTPanel = mTrayMgr->createDecorWidget(TL_NONE, "DebugRTTPanel", "SdkTrays/Picture");
    OverlayContainer* debugRTTContainer = (OverlayContainer*)debugRTTPanel->getOverlayElement();
    mDebugTextureTUS = debugRTTContainer->getMaterial()->getBestTechnique()->getPass(0)->getTextureUnitState(0);
    //mDebugTextureTUS->setTextureName("CompositorDemo/DebugView");
    debugRTTContainer->setDimensions(128, 128);
    debugRTTContainer->getChild("DebugRTTPanel/PictureFrame")->setDimensions(144, 144);
    debugRTTPanel->hide();

    mTrayMgr->createSeparator(TL_TOPRIGHT, "DebugRTTSep2");  // this is a hack to give the debug RTT a bit more room

    mTrayMgr->showCursor();
    mTrayMgr->showLogo(TL_BOTTOMLEFT);
    mTrayMgr->toggleAdvancedFrameStats();
}


void Sample_Compositor::checkBoxToggled(OgreBites::CheckBox * box)
{
    if (Ogre::StringUtil::startsWith(box->getName(), "Compositor_", false))
    {
        String compositorName = box->getCaption();

        String activeTex = mDebugTextureSelectMenu->getSelectedItem();

        if (!box->isChecked())
        {
            //Remove the items from the debug menu and remove debug texture if from disabled compositor
            bool debuggingRemovedTex = StringUtil::startsWith(activeTex, compositorName, false);
            if (debuggingRemovedTex)
            {
                mDebugTextureTUS->setContentType(TextureUnitState::CONTENT_NAMED);
                mDebugTextureSelectMenu->selectItem(0, true);
            }
            for (unsigned int i = 1; i < mDebugTextureSelectMenu->getNumItems(); i++)
            {
                if (StringUtil::startsWith(mDebugTextureSelectMenu->getItems()[i], compositorName, false))
                {
                    mDebugTextureSelectMenu->removeItem(i);
                    i--;
                }
            }
            if (!debuggingRemovedTex)
            {
                //Selection clears itself when removing items. Restore.
                mDebugTextureSelectMenu->selectItem(activeTex, false);
            }
        }

        CompositorManager::getSingleton().setCompositorEnabled(mViewport, compositorName, box->isChecked());


        if (box->isChecked())
        {
            //Add the items to the selectable texture menu
            CompositorInstance* instance = CompositorManager::getSingleton().getCompositorChain(mViewport)->getCompositor(compositorName);
            if (instance)
            {
                const CompositionTechnique::TextureDefinitions& defs =
                    instance->getTechnique()->getTextureDefinitions();
                CompositionTechnique::TextureDefinitions::const_iterator defIter;
                for (defIter = defs.begin(); defIter != defs.end(); ++defIter)
                {
                    CompositionTechnique::TextureDefinition* texDef = *defIter;
                    size_t numTextures = texDef->formatList.size();
                    if (numTextures > 1)
                    {
                        for (size_t i=0; i<numTextures; i++)
                        {
                            //Dirty string composition. NOT ROBUST!
                            mDebugTextureSelectMenu->addItem(compositorName + ";" + texDef->name + ";" +
                                                             Ogre::StringConverter::toString((Ogre::uint32)i));
                        }
                    }
                    else
                    {
                        mDebugTextureSelectMenu->addItem(compositorName + ";" + texDef->name);
                    }
                }
                mDebugTextureSelectMenu->selectItem(activeTex, false);
            }
        }
    }
}


void Sample_Compositor::buttonHit(OgreBites::Button* button)
{
    size_t nextPage = (mActiveCompositorPage + 1) % mNumCompositorPages;
    changePage(nextPage);
}


void Sample_Compositor::itemSelected(OgreBites::SelectMenu* menu)
{
    if (menu->getSelectionIndex() == 0)
    {
        mDebugTextureTUS->setContentType(TextureUnitState::CONTENT_NAMED);
        mTrayMgr->getWidget("DebugRTTPanel")->hide();
        mTrayMgr->removeWidgetFromTray("DebugRTTPanel");
        return;
    }

    mTrayMgr->getWidget("DebugRTTPanel")->show();
    mTrayMgr->moveWidgetToTray("DebugRTTPanel", TL_TOPRIGHT, mTrayMgr->getWidgets(TL_TOPRIGHT).size() - 1);
    StringVector parts = StringUtil::split(menu->getSelectedItem(), ";");
    mDebugTextureTUS->setContentType(TextureUnitState::CONTENT_COMPOSITOR);

    if (parts.size() == 2)
    {
        mDebugTextureTUS->setCompositorReference(parts[0], parts[1]);
    }
    else
    {
        mDebugTextureTUS->setCompositorReference(parts[0], parts[1],
                                                 StringConverter::parseUnsignedInt(parts[2]));
    }
}


void Sample_Compositor::setupScene(void)
{
    mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_MODULATIVE);
    mSceneMgr->setShadowFarDistance(1000);

    Ogre::MovableObject::setDefaultVisibilityFlags(0x00000001);

    // Set ambient light
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.3, 0.3, 0.2));

    Ogre::Light* l = mSceneMgr->createLight("Light2");
    l->setType(Ogre::Light::LT_DIRECTIONAL);
    l->setDiffuseColour(1, 1, 0.8);
    l->setSpecularColour(1, 1, 1);

    auto ln = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    ln->setDirection(Vector3(-1,-1,0).normalisedCopy());
    ln->attachObject(l);


    Ogre::Entity* pEnt;

    // House
    pEnt = mSceneMgr->createEntity( "1", "tudorhouse.mesh" );
    Ogre::SceneNode* n1 = mSceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::Vector3(350, 450, -200));
    n1->attachObject( pEnt );

    pEnt = mSceneMgr->createEntity( "2", "tudorhouse.mesh" );
    Ogre::SceneNode* n2 = mSceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::Vector3(-350, 450, -200));
    n2->attachObject( pEnt );

    pEnt = mSceneMgr->createEntity( "3", "knot.mesh" );
    mSpinny = mSceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::Vector3(0, 0, 300));
    mSpinny->attachObject( pEnt );
    pEnt->setMaterialName("Examples/MorningCubeMap");

    mSceneMgr->setSkyBox(true, "Examples/MorningSkyBox");


    Ogre::Plane plane;
    plane.normal = Ogre::Vector3::UNIT_Y;
    plane.d = 100;
    Ogre::MeshManager::getSingleton().createPlane("Myplane",
                                                  Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
                                                  1500, 1500, 10, 10, true, 1, 5, 5, Ogre::Vector3::UNIT_Z);
    Ogre::Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
    pPlaneEnt->setMaterialName("Examples/Rockwall");
    pPlaneEnt->setCastShadows(false);
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);

    mCameraNode->setPosition(-400, 50, 900);
    mCameraNode->lookAt(Ogre::Vector3(0,80,0), Ogre::Node::TS_PARENT);
}


bool Sample_Compositor::frameRenderingQueued(const FrameEvent& evt)
{
    mSpinny->yaw(Ogre::Degree(10 * evt.timeSinceLastFrame));
    return SdkSample::frameRenderingQueued(evt);
}


/// Create the hard coded postfilter effects
void Sample_Compositor::createEffects(void)
{
    // Glass compositor is loaded from script but here is the hard coded equivalent
    /// Glass effect
    //          CompositorPtr comp2 = CompositorManager::getSingleton().create(
    //                          "Glass", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
    //                  );
    //          {
    //                  CompositionTechnique *t = comp2->createTechnique();
    //                  {
    //                          CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("rt0");
    //                          def->width = 0;
    //                          def->height = 0;
    //                          def->format = PF_R8G8B8;
    //                  }
    //                  {
    //                          CompositionTargetPass *tp = t->createTargetPass();
    //                          tp->setInputMode(CompositionTargetPass::IM_PREVIOUS);
    //                          tp->setOutputName("rt0");
    //                  }
    //                  {
    //                          CompositionTargetPass *tp = t->getOutputTargetPass();
    //                          tp->setInputMode(CompositionTargetPass::IM_NONE);
    //                          { CompositionPass *pass = tp->createPass(CompositionPass::PT_RENDERQUAD);
    //                          pass->setMaterialName("Ogre/Compositor/GlassPass");
    //                          pass->setInput(0, "rt0");
    //                          }
    //                  }
    //          }
    /// Motion blur effect
    Ogre::CompositorPtr comp3 = Ogre::CompositorManager::getSingleton().create(
        "Motion Blur", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
    );
    {
        Ogre::CompositionTechnique *t = comp3->createTechnique();
        {
            Ogre::CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("scene");
            def->width = 0;
            def->height = 0;
            def->formatList.push_back(Ogre::PF_R8G8B8);
        }
        {
            Ogre::CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("sum");
            def->width = 0;
            def->height = 0;
            def->formatList.push_back(Ogre::PF_R8G8B8);
        }
        {
            Ogre::CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("temp");
            def->width = 0;
            def->height = 0;
            def->formatList.push_back(Ogre::PF_R8G8B8);
        }
        /// Render scene
        {
            Ogre::CompositionTargetPass *tp = t->createTargetPass();
            tp->setInputMode(Ogre::CompositionTargetPass::IM_PREVIOUS);
            tp->setOutputName("scene");
        }
        /// Initialisation pass for sum texture
        {
            Ogre::CompositionTargetPass *tp = t->createTargetPass();
            tp->setInputMode(Ogre::CompositionTargetPass::IM_PREVIOUS);
            tp->setOutputName("sum");
            tp->setOnlyInitial(true);
        }
        /// Do the motion blur
        {
            Ogre::CompositionTargetPass *tp = t->createTargetPass();
            tp->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
            tp->setOutputName("temp");
            { Ogre::CompositionPass *pass = tp->createPass(Ogre::CompositionPass::PT_RENDERQUAD);
                pass->setMaterialName("Ogre/Compositor/Combine");
                pass->setInput(0, "scene");
                pass->setInput(1, "sum");
            }
        }
        /// Copy back sum texture
        {
            Ogre::CompositionTargetPass *tp = t->createTargetPass();
            tp->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
            tp->setOutputName("sum");
            { Ogre::CompositionPass *pass = tp->createPass(Ogre::CompositionPass::PT_RENDERQUAD);
                pass->setMaterialName("Ogre/Compositor/Copyback");
                pass->setInput(0, "temp");
            }
        }
        /// Display result
        {
            Ogre::CompositionTargetPass *tp = t->getOutputTargetPass();
            tp->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
            { Ogre::CompositionPass *pass = tp->createPass(Ogre::CompositionPass::PT_RENDERQUAD);
                pass->setMaterialName("Ogre/Compositor/MotionBlur");
                pass->setInput(0, "sum");
            }
        }
    }
    /// Heat vision effect
    Ogre::CompositorPtr comp4 = Ogre::CompositorManager::getSingleton().create(
        "Heat Vision", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
    );
    {
        Ogre::CompositionTechnique *t = comp4->createTechnique();
        t->setCompositorLogicName("HeatVision");
        {
            Ogre::CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("scene");
            def->width = 256;
            def->height = 256;
            def->formatList.push_back(Ogre::PF_R8G8B8);
        }
        {
            Ogre::CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("temp");
            def->width = 256;
            def->height = 256;
            def->formatList.push_back(Ogre::PF_R8G8B8);
        }
        /// Render scene
        {
            Ogre::CompositionTargetPass *tp = t->createTargetPass();
            tp->setInputMode(Ogre::CompositionTargetPass::IM_PREVIOUS);
            tp->setOutputName("scene");
        }
        /// Light to heat pass
        {
            Ogre::CompositionTargetPass *tp = t->createTargetPass();
            tp->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
            tp->setOutputName("temp");
            {
                Ogre::CompositionPass *pass = tp->createPass(Ogre::CompositionPass::PT_RENDERQUAD);
                pass->setIdentifier(0xDEADBABE); /// Identify pass for use in listener
                pass->setMaterialName("Fury/HeatVision/LightToHeat");
                pass->setInput(0, "scene");
            }
        }
        /// Display result
        {
            Ogre::CompositionTargetPass *tp = t->getOutputTargetPass();
            tp->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
            {
                Ogre::CompositionPass *pass = tp->createPass(Ogre::CompositionPass::PT_RENDERQUAD);
                pass->setMaterialName("Fury/HeatVision/Blur");
                pass->setInput(0, "temp");
            }
        }
    }
}


void Sample_Compositor::createTextures(void)
{
    using namespace Ogre;

    TexturePtr tex = TextureManager::getSingleton().createManual(
        "HalftoneVolume",
        "General",
        TEX_TYPE_3D,
        64,64,64,
        0,
        PF_L8,
        TU_DYNAMIC_WRITE_ONLY
    );

    MaterialManager::getSingleton()
        .getByName("Ogre/Compositor/Halftone", "General")
        ->getTechnique(0)
        ->getPass(0)
        ->getTextureUnitState("noise")
        ->setTexture(tex);

    if(tex)
    {
        HardwarePixelBufferSharedPtr ptr = tex->getBuffer(0,0);
        ptr->lock(HardwareBuffer::HBL_DISCARD);
        const PixelBox &pb = ptr->getCurrentLock();
        Ogre::uint8 *data = pb.data;

        size_t height = pb.getHeight();
        size_t width = pb.getWidth();
        size_t depth = pb.getDepth();
        size_t rowPitch = pb.rowPitch;
        size_t slicePitch = pb.slicePitch;

        for (size_t z = 0; z < depth; ++z)
        {
            for (size_t y = 0; y < height; ++y)
            {
                for(size_t x = 0; x < width; ++x)
                {
                    float fx = 32-(float)x+0.5f;
                    float fy = 32-(float)y+0.5f;
                    float fz = 32-((float)z)/3+0.5f;
                    float distanceSquare = fx*fx+fy*fy+fz*fz;
                    data[slicePitch*z + rowPitch*y + x] =  0x00;
                    if (distanceSquare < 1024.0f)
                        data[slicePitch*z + rowPitch*y + x] +=  0xFF;
                }
            }
        }
        ptr->unlock();
    }
    Ogre::Viewport *vp = mWindow->getViewport(0);

    TexturePtr tex2 = TextureManager::getSingleton().createManual(
        "DitherTex",
        "General",
        TEX_TYPE_2D,
        vp->getActualWidth(),vp->getActualHeight(),1,
        0,
        PF_L8,
        TU_DYNAMIC_WRITE_ONLY
    );

    MaterialManager::getSingleton()
        .getByName("Ogre/Compositor/Dither", "General")
        ->getTechnique(0)
        ->getPass(0)
        ->getTextureUnitState("noise")
        ->setTexture(tex2);

    HardwarePixelBufferSharedPtr ptr2 = tex2->getBuffer(0,0);
    ptr2->lock(HardwareBuffer::HBL_DISCARD);
    const PixelBox &pb2 = ptr2->getCurrentLock();
    Ogre::uint8 *data2 = pb2.data;

    size_t height2 = pb2.getHeight();
    size_t width2 = pb2.getWidth();
    size_t rowPitch2 = pb2.rowPitch;

    for (size_t y = 0; y < height2; ++y)
    {
        for(size_t x = 0; x < width2; ++x)
        {
            data2[rowPitch2*y + x] = Ogre::Math::RangeRandom(64.0,192);
        }
    }

    ptr2->unlock();
}

#endif  // end _CompositorDemo_H_
