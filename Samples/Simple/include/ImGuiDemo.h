#pragma once

#include "SdkSample.h"
#include "OgreImGuiOverlay.h"
#include <OgreImGuiInputListener.h>

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_ImGui : public SdkSample, public RenderTargetListener
{
    std::unique_ptr<ImGuiInputListener> mImguiListener;
    InputListenerChain mListenerChain;
public:
    // Basic constructor
    Sample_ImGui()
    {
        mInfo["Title"] = "Dear ImGui integration";
        mInfo["Description"] = "Overlay ImGui interactions";
        mInfo["Category"] = "Unsorted";
        mInfo["Thumbnail"] = "thumb_imgui.png";
    }

    void preViewportUpdate(const RenderTargetViewportEvent& evt)
    {
        if(!evt.source->getOverlaysEnabled()) return;
        if(!mTrayMgr->getTraysLayer()->isVisible()) return;

        ImGuiOverlay::NewFrame();

        ImGui::ShowDemoWindow();
    }

    bool keyPressed(const KeyboardEvent& evt) { return mListenerChain.keyPressed(evt); }
    bool keyReleased(const KeyboardEvent& evt) { return mListenerChain.keyReleased(evt); }
    bool mouseMoved(const MouseMotionEvent& evt) { return mListenerChain.mouseMoved(evt); }
    bool mouseWheelRolled(const MouseWheelEvent& evt) { return mListenerChain.mouseWheelRolled(evt); }
    bool mousePressed(const MouseButtonEvent& evt) { return mListenerChain.mousePressed(evt); }
    bool mouseReleased(const MouseButtonEvent& evt) { return mListenerChain.mouseReleased(evt); }
    bool textInput (const TextInputEvent& evt) { return mListenerChain.textInput (evt); }

    void setupContent(void)
    {
        auto imguiOverlay = new ImGuiOverlay();
        imguiOverlay->setZOrder(300);
        imguiOverlay->show();
        OverlayManager::getSingleton().addOverlay(imguiOverlay); // now owned by overlaymgr

        /*
            NOTE:
            Custom apps will ASSERT on ImGuiOverlay::NewFrame() and not display any UI if they
            have not registered the overlay system by calling mSceneMgr->addRenderQueueListener(mOverlaySystem).
            OgreBites::SampleBrowser does this on behalf of the ImGuiDemo but custom applications will need to
            call this themselves.  See ApplicationContextBase::createDummyScene().
        */
        mWindow->addListener(this);

        mImguiListener.reset(new ImGuiInputListener());
        mListenerChain = InputListenerChain({mTrayMgr.get(), mImguiListener.get(), mCameraMan.get()});

        mTrayMgr->showCursor();
        mCameraMan->setStyle(OgreBites::CS_ORBIT);
        mCameraMan->setYawPitchDist(Degree(0), Degree(0), 15);

        SceneNode* lightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        lightNode->setPosition(0, 10, 15);
        lightNode->attachObject(mSceneMgr->createLight("MainLight"));

        Entity* ent = mSceneMgr->createEntity("Sinbad.mesh");
        SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        node->attachObject(ent);
    }

    void cleanupContent()
    {
        OverlayManager::getSingleton().destroy("ImGuiOverlay");
        mWindow->removeListener(this);
    }
};
