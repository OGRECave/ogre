// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include "OgreAdvancedRenderControls.h"
#include "OgreTextureManager.h"
#include "OgreMaterialManager.h"

#include "OgreTrays.h"

namespace OgreBites {
AdvancedRenderControls::AdvancedRenderControls(TrayManager* trayMgr, Ogre::Camera* cam)
    : mCamera(cam), mTrayMgr(trayMgr) {
    mRoot = Ogre::Root::getSingletonPtr();

    // create a params panel for displaying sample details
    Ogre::StringVector items;
    items.push_back("cam.pX");
    items.push_back("cam.pY");
    items.push_back("cam.pZ");
    items.push_back("");
    items.push_back("cam.oW");
    items.push_back("cam.oX");
    items.push_back("cam.oY");
    items.push_back("cam.oZ");
    items.push_back("");
    items.push_back("Filtering");
    items.push_back("Poly Mode");

#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
    mShaderGenerator = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
    items.push_back("RT Shaders");
    items.push_back("Lighting Model");
    items.push_back("Compact Policy");
    items.push_back("Generated VS");
    items.push_back("Generated FS");
#endif

    mDetailsPanel = mTrayMgr->createParamsPanel(TL_NONE, "DetailsPanel", 200, items);
    mDetailsPanel->hide();

    mDetailsPanel->setParamValue(9, "Bilinear");
    mDetailsPanel->setParamValue(10, "Solid");

#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
    mDetailsPanel->setParamValue(11, "Off");
    if (!mRoot->getRenderSystem()->getCapabilities()->hasCapability(Ogre::RSC_FIXED_FUNCTION)) {
        mDetailsPanel->setParamValue(11, "On");
    }

    mDetailsPanel->setParamValue(12, "Pixel");
    mDetailsPanel->setParamValue(13, "Low");
    mDetailsPanel->setParamValue(14, "0");
    mDetailsPanel->setParamValue(15, "0");
#endif
}

AdvancedRenderControls::~AdvancedRenderControls() {
    mTrayMgr->destroyWidget(mDetailsPanel);
}

bool AdvancedRenderControls::keyPressed(const KeyboardEvent& evt) {
    if (mTrayMgr->isDialogVisible())
        return true; // don't process any more keys if dialog is up

    int key = evt.keysym.sym;

    if (key == 'f') // toggle visibility of advanced frame stats
    {
        mTrayMgr->toggleAdvancedFrameStats();
    } else if (key == 'g') // toggle visibility of even rarer debugging details
    {
        if (mDetailsPanel->getTrayLocation() == TL_NONE) {
            mTrayMgr->moveWidgetToTray(mDetailsPanel, TL_TOPRIGHT, 0);
            mDetailsPanel->show();
        } else {
            mTrayMgr->removeWidgetFromTray(mDetailsPanel);
            mDetailsPanel->hide();
        }
    } else if (key == 't') // cycle texture filtering mode
    {
        Ogre::String newVal;
        Ogre::TextureFilterOptions tfo;
        unsigned int aniso;

        Ogre::FilterOptions mip = Ogre::MaterialManager::getSingleton().getDefaultTextureFiltering(Ogre::FT_MIP);

        switch (Ogre::MaterialManager::getSingleton().getDefaultTextureFiltering(Ogre::FT_MAG)) {
        case Ogre::FO_LINEAR:
            if (mip == Ogre::FO_POINT) {
                newVal = "Trilinear";
                tfo = Ogre::TFO_TRILINEAR;
                aniso = 1;
            } else {
                newVal = "Anisotropic";
                tfo = Ogre::TFO_ANISOTROPIC;
                aniso = 8;
            }
            break;
        case Ogre::FO_ANISOTROPIC:
            newVal = "None";
            tfo = Ogre::TFO_NONE;
            aniso = 1;
            break;
        default:
            newVal = "Bilinear";
            tfo = Ogre::TFO_BILINEAR;
            aniso = 1;
            break;
        }

        Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
        Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(aniso);
        mDetailsPanel->setParamValue(9, newVal);
    } else if (key == 'r') // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::PolygonMode pm;

        switch (mCamera->getPolygonMode()) {
        case Ogre::PM_SOLID:
            newVal = "Wireframe";
            pm = Ogre::PM_WIREFRAME;
            break;
        case Ogre::PM_WIREFRAME:
            newVal = "Points";
            pm = Ogre::PM_POINTS;
            break;
        default:
            newVal = "Solid";
            pm = Ogre::PM_SOLID;
            break;
        }

        mCamera->setPolygonMode(pm);
        mDetailsPanel->setParamValue(10, newVal);
    } else if (key == SDLK_F5) // refresh all textures
    {
        Ogre::TextureManager::getSingleton().reloadAll();
    }
    else if (key == SDLK_F6)   // take a screenshot
    {
        mCamera->getViewport()->getTarget()->writeContentsToTimestampedFile("screenshot", ".png");
    }
    // Toggle visibility of profiler window
    else if (key == 'p')
    {
        if (auto prof = Ogre::Profiler::getSingletonPtr())
            prof->setEnabled(!prof->getEnabled());
    }
#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
    // Toggle schemes.
    else if (key == SDLK_F2) {
        if (mRoot->getRenderSystem()->getCapabilities()->hasCapability(Ogre::RSC_FIXED_FUNCTION)) {
            Ogre::Viewport* mainVP = mCamera->getViewport();
            const Ogre::String& curMaterialScheme = mainVP->getMaterialScheme();

            if (curMaterialScheme == Ogre::MSN_DEFAULT)
            {
                mainVP->setMaterialScheme(Ogre::MSN_SHADERGEN);
                mDetailsPanel->setParamValue(11, "On");
            }
            else if (curMaterialScheme == Ogre::MSN_SHADERGEN)
            {
                mainVP->setMaterialScheme(Ogre::MSN_DEFAULT);
                mDetailsPanel->setParamValue(11, "Off");
            }
        }
    }
#   ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
    // Toggles per pixel per light model.
    else if (key == SDLK_F3) {
        static bool useFFPLighting = true;

        //![rtss_per_pixel]
        // Grab the scheme render state.
        Ogre::RTShader::RenderState* schemRenderState = mShaderGenerator->getRenderState(Ogre::MSN_SHADERGEN);

        // Add per pixel lighting sub render state to the global scheme render state.
        // It will override the default FFP lighting sub render state.
        if (useFFPLighting) {
            auto perPixelLightModel = mShaderGenerator->createSubRenderState("FFP_Lighting");

            schemRenderState->addTemplateSubRenderState(perPixelLightModel);
        }
        //![rtss_per_pixel]

        // Search the per pixel sub render state and remove it.
        else {
            for (auto srs : schemRenderState->getSubRenderStates()) {
                // This is the per pixel sub render state -> remove it.
                if (srs->getType() == "FFP_Lighting") {
                    schemRenderState->removeSubRenderState(srs);
                    break;
                }
            }
        }

        // Invalidate the scheme in order to re-generate all shaders based technique related to this
        // scheme.
        mShaderGenerator->invalidateScheme(Ogre::MSN_SHADERGEN);

        // Update UI.
        if (!useFFPLighting)
            mDetailsPanel->setParamValue(12, "Pixel");
        else
            mDetailsPanel->setParamValue(12, "Vertex");
        useFFPLighting = !useFFPLighting;
    }
#   endif
    // Switch vertex shader outputs compaction policy.
    else if (key == SDLK_F4) {
        switch (mShaderGenerator->getVertexShaderOutputsCompactPolicy()) {
        case Ogre::RTShader::VSOCP_LOW:
            mShaderGenerator->setVertexShaderOutputsCompactPolicy(Ogre::RTShader::VSOCP_MEDIUM);
            mDetailsPanel->setParamValue(13, "Medium");
            break;

        case Ogre::RTShader::VSOCP_MEDIUM:
            mShaderGenerator->setVertexShaderOutputsCompactPolicy(Ogre::RTShader::VSOCP_HIGH);
            mDetailsPanel->setParamValue(13, "High");
            break;

        case Ogre::RTShader::VSOCP_HIGH:
            mShaderGenerator->setVertexShaderOutputsCompactPolicy(Ogre::RTShader::VSOCP_LOW);
            mDetailsPanel->setParamValue(13, "Low");
            break;
        }

        // Invalidate the scheme in order to re-generate all shaders based technique related to this
        // scheme.
        mShaderGenerator->invalidateScheme(Ogre::MSN_SHADERGEN);
    }
#endif // INCLUDE_RTSHADER_SYSTEM

    return InputListener::keyPressed(evt);
}

void AdvancedRenderControls::frameRendered(const Ogre::FrameEvent& evt) {
    using namespace Ogre;
    if (!mTrayMgr->isDialogVisible() && mDetailsPanel->isVisible())
    {
        // if details panel is visible, then update its contents
        mDetailsPanel->setParamValue(0, Ogre::StringConverter::toString(mCamera->getDerivedPosition().x));
        mDetailsPanel->setParamValue(1, Ogre::StringConverter::toString(mCamera->getDerivedPosition().y));
        mDetailsPanel->setParamValue(2, Ogre::StringConverter::toString(mCamera->getDerivedPosition().z));
        mDetailsPanel->setParamValue(4, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().w));
        mDetailsPanel->setParamValue(5, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().x));
        mDetailsPanel->setParamValue(6, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().y));
        mDetailsPanel->setParamValue(7, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().z));

#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
        mDetailsPanel->setParamValue(14, StringConverter::toString(mShaderGenerator->getShaderCount(GPT_VERTEX_PROGRAM)));
        mDetailsPanel->setParamValue(15, StringConverter::toString(mShaderGenerator->getShaderCount(GPT_FRAGMENT_PROGRAM)));
#endif
    }
}

} /* namespace OgreBites */
