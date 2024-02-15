#pragma once

#include "SdkSample.h"
#include "OgreImGuiOverlay.h"
#include <OgreImGuiInputListener.h>

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_ImGui : public SdkSample, public RenderTargetListener
{
public:
    // Basic constructor
    Sample_ImGui()
    {
        mInfo["Title"] = "Dear ImGui integration";
        mInfo["Description"] = "Overlay ImGui interactions";
        mInfo["Category"] = "Unsorted";
        mInfo["Thumbnail"] = "thumb_imgui.png";
    }

    void preViewportUpdate(const RenderTargetViewportEvent& evt) override
    {
        if(!evt.source->getOverlaysEnabled()) return;
        if(!mTrayMgr->getTraysLayer()->isVisible()) return;

        ImGuiOverlay::NewFrame();

        ImGui::ShowDemoWindow();
    }

    void setupContent(void) override
    {
        auto imguiOverlay = mContext->initialiseImGui();

        float vpScale = OverlayManager::getSingleton().getPixelRatio();
        ImGui::GetIO().FontGlobalScale = std::round(vpScale); // default font does not work with fractional scaling

        imguiOverlay->setZOrder(300);
        imguiOverlay->show();

        /*
            NOTE:
            Custom apps will ASSERT on ImGuiOverlay::NewFrame() and not display any UI if they
            have not registered the overlay system by calling mSceneMgr->addRenderQueueListener(mOverlaySystem).
            OgreBites::SampleBrowser does this on behalf of the ImGuiDemo but custom applications will need to
            call this themselves.  See ApplicationContextBase::createDummyScene().
        */
        mWindow->addListener(this);

        mInputListenerChain = TouchAgnosticInputListenerChain(
            mWindow, {mTrayMgr.get(), mContext->getImGuiInputListener(), mCameraMan.get()});

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

    void cleanupContent() override
    {
        OverlayManager::getSingleton().destroy("ImGuiOverlay");
        mWindow->removeListener(this);
    }
};
