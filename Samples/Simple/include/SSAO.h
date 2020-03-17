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

#include "SdkSample.h"

#ifndef __SSAO_H__
#define __SSAO_H__

using namespace Ogre;
using namespace OgreBites;

#define SSAO_GUI_WIDTH 250
#define SSAO_GUI_TRACK_WIDTH 100
#define SSAO_GUI_VALUE_BOX_WIDTH 50

#define SSAO_OBJECT_MENU_NAME "ObjectType"
#define SSAO_CAMERA_MENU_NAME "Camera"

#define SSAO_COMPOSITOR_MENU_NAME "Compositor"
#define SSAO_POST_MENU_NAME "Post"
#define SSAO_CREASE_MINIMUM_NAME "CreaseMinimum"
#define SSAO_CREASE_RANGE_NAME "mCreaseRange"
#define SSAO_CREASE_BIAS_NAME "mCreaseBias"
#define SSAO_CREASE_AVERAGER_NAME "mCreaseAverager"
#define SSAO_CREASE_KERNELSIZE_NAME "mCreaseKernelsize"

#define SSAO_MODUALTE "mdoulate"
#define SSAO_SAMPLE_SPACE_NAME "sampleSpace"
#define SSAO_SAMPLE_LENGTH_SCREENSPACE "sampleScreenSpace"
#define SSAO_SAMPLE_LENGTH_WORLDSPACE "sampleWorldSpace"
#define SSAO_SAMPLE_LENGTH_EXPONENT_NAME "sampleLengthExponent"

#define SSAO_ANGLE_BIAS_NAME "angleBias"

#define SSAO_CRYTEK_OFFSET_SCALE_NAME "offsetScale"
#define SSAO_CRYTEK_EDGE_HIGHLIGHT_NAME "edgeHighlight"
#define SSAO_CRYTEK_DEFAULT_ACCESSIBILITY_NAME "defaultOcclusion"

#define SSAO_UNSHARP_KERNEL_BIAS_NAME "kernelBias"
#define SSAO_UNSHARP_LAMBDA_NAME "lambda"

#define SSAO_BILATERAL_PHOTOMETRIC_EXPONENT "photometricExponent"

#define SSAO_USER_CAMERA_ITEM "User Camera"
#define SSAO_CAMERA_SIBENIK "Sibenik"
#define SSAO_CAMERA_CORNELL "Cornell Box"

/** Class for handling materials who did not specify techniques for rendering
 *  themselves into the GBuffer.
 */
class _OgreSampleClassExport SSAOGBufferSchemeHandler : public Ogre::MaterialManager::Listener
{
public:
    SSAOGBufferSchemeHandler()
    {
        mGBufRefMat = Ogre::MaterialManager::getSingleton().getByName("SSAO/GBuffer");
        RTShader::ShaderGenerator::getSingleton().validateMaterial("GBuffer", "SSAO/GBuffer");
        mGBufRefMat->load();
    }

    /** @copydoc MaterialManager::Listener::handleSchemeNotFound */
    virtual Ogre::Technique* handleSchemeNotFound(unsigned short schemeIndex, 
        const Ogre::String& schemeName, Ogre::Material* originalMaterial, unsigned short lodIndex, 
        const Ogre::Renderable* rend)
    {
            Technique* gBufferTech = originalMaterial->createTechnique();
            gBufferTech->setSchemeName(schemeName);
            Ogre::Pass* gbufPass = gBufferTech->createPass();
            *gbufPass = *mGBufRefMat->getBestTechnique()->getPass(0);
            return gBufferTech;
    }
private:
    Ogre::MaterialPtr mGBufRefMat;
};

class _OgreSampleClassExport Sample_SSAO : public SdkSample
{
private:
    std::vector<String> mMeshNames;
    std::vector<Entity*> mMeshes;
    int mCurrentMeshIndex;
    
    std::vector<String> mCompositorNames;
    String mCurrentCompositor;
    
    std::vector<String> mPostNames;
    String mCurrentPost;
    String mCurrentModulateScheme;

    SSAOGBufferSchemeHandler* mGBufSchemeHandler;
    Light* mLight;

public:
    Sample_SSAO()
    {
        mInfo["Title"] = "SSAO Techniques";
        mInfo["Description"] = "A demo of several Screen Space Ambient Occlusion (SSAO) shading techniques using compositors.";
        mInfo["Thumbnail"] = "thumb_ssao.png";
        mInfo["Category"] = "Lighting";
        
        mMeshNames.push_back("sibenik");
        mMeshNames.push_back("cornell");
        
        mCompositorNames.push_back("SSAO/HemisphereMC");
        mCompositorNames.push_back("SSAO/Volumetric");
        mCompositorNames.push_back("SSAO/HorizonBased");
        mCompositorNames.push_back("SSAO/Crytek");
        mCompositorNames.push_back("SSAO/CreaseShading");
        mCompositorNames.push_back("SSAO/UnsharpMask");
        mCompositorNames.push_back("SSAO/ShowDepth");
        mCompositorNames.push_back("SSAO/ShowNormals");
        mCompositorNames.push_back("SSAO/ShowViewPos");
        
        mPostNames.push_back("SSAO/Post/NoFilter");
        mPostNames.push_back("SSAO/Post/CrossBilateralFilter");
        mPostNames.push_back("SSAO/Post/SmartBoxFilter");
        mPostNames.push_back("SSAO/Post/BoxFilter");
        
        mCurrentCompositor = mCompositorNames[0];
        mCurrentPost = mPostNames[0];

        mGBufSchemeHandler = NULL;
        mLight = NULL;
    }
    
    void cleanupContent()
    {
        MaterialManager::getSingleton().removeListener(mGBufSchemeHandler, "GBuffer");
        delete mGBufSchemeHandler;
        mGBufSchemeHandler = NULL;

        CompositorManager::getSingleton().setCompositorEnabled(mViewport, mCurrentCompositor, false);
        CompositorManager::getSingleton().setCompositorEnabled(mViewport, mCurrentPost, false);
        
        CompositorManager::getSingleton().setCompositorEnabled(mViewport, "SSAO/GBuffer", false);
        CompositorManager::getSingleton().removeCompositor(mViewport, "SSAO/GBuffer");
        
        for (unsigned int i = 0; i < mCompositorNames.size(); i++)
        {
            CompositorManager::getSingleton().setCompositorEnabled(mViewport, mCompositorNames[i], false);
            CompositorManager::getSingleton().removeCompositor(mViewport, mCompositorNames[i]);
        }
        
        for (unsigned int i = 0; i < mPostNames.size(); i++)
        {
            CompositorManager::getSingleton().setCompositorEnabled(mViewport, mPostNames[i], false);
            CompositorManager::getSingleton().removeCompositor(mViewport, mPostNames[i]);
        }
        
        mMeshes.clear();
    }
    
    StringVector getRequiredPlugins()
    {
        return StringVector();
    }
    
    void testCapabilities(const RenderSystemCapabilities* caps)
    {
		if (StringUtil::startsWith(caps->getRenderSystemName(), "OpenGL ES"))
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "This demo currently only supports OpenGL and DirectX9. Sorry!",
                "Sample_SSAO:testCapabilities");
        }
    }
    
protected:
    /**
     * Setup the compositors to be used.
     */
    void setupCompositors()
    {
        
        if (CompositorManager::getSingleton().addCompositor(mViewport, "SSAO/GBuffer"))
            CompositorManager::getSingleton().setCompositorEnabled(mViewport, "SSAO/GBuffer", true);
        else
            LogManager::getSingleton().logMessage("Sample_SSAO: Failed to add GBuffer compositor\n");
        
        for (unsigned int i = 0; i < mCompositorNames.size(); i++)
        {
            if (CompositorManager::getSingleton().addCompositor(mViewport, mCompositorNames[i]))
                CompositorManager::getSingleton().setCompositorEnabled(mViewport, mCompositorNames[i], false);
            else
                LogManager::getSingleton().logMessage("Sample_SSAO: Failed to add compositor: " + mCompositorNames[i] + "\n");
        }
        
        for (unsigned int i = 0; i < mPostNames.size(); i++)
        {
            
            if (CompositorManager::getSingleton().addCompositor(mViewport, mPostNames[i]))
                CompositorManager::getSingleton().setCompositorEnabled(mViewport, mPostNames[i], false);
            else
                LogManager::getSingleton().logMessage("Sample_SSAO: Failed to add " + mPostNames[i] + " compositor\n");
        }
        
        CompositorManager::getSingleton().setCompositorEnabled(mViewport, mCurrentCompositor, true);
        CompositorManager::getSingleton().setCompositorEnabled(mViewport, mCurrentPost, true);
    }
    
    /**
     * Setup the controls, ie. the gui elements.
     */
    void setupControls(void)
    {
        // --- select mesh menu ---
        SelectMenu* objectType = mTrayMgr->createThickSelectMenu(TL_TOPLEFT, SSAO_OBJECT_MENU_NAME, "Object: ", SSAO_GUI_WIDTH, 16);
        for (unsigned int i = 0; i < mMeshNames.size(); i++)
            objectType->addItem(mMeshNames[i]);
        
        // --- select camera menu ---
        SelectMenu* cameraMenu = mTrayMgr->createThickSelectMenu(TL_TOPLEFT, SSAO_CAMERA_MENU_NAME, "Camera Position", SSAO_GUI_WIDTH, 16);
        cameraMenu->addItem(SSAO_USER_CAMERA_ITEM);
        cameraMenu->addItem(SSAO_CAMERA_CORNELL);
        cameraMenu->addItem(SSAO_CAMERA_SIBENIK);
        
        // --- select compositor menu ---
        SelectMenu* compositor = mTrayMgr->createThickSelectMenu(TL_TOPLEFT, SSAO_COMPOSITOR_MENU_NAME, "Compositor: ", SSAO_GUI_WIDTH, 16);
        for (unsigned int i = 0; i < mCompositorNames.size(); i++)
            compositor->addItem(mCompositorNames[i]);
        
        // --- select post filter menu ---
        SelectMenu * post = mTrayMgr->createThickSelectMenu(TL_TOPLEFT, SSAO_POST_MENU_NAME, "Post Filter: ", SSAO_GUI_WIDTH, 16);
        for (unsigned int i = 0; i < mPostNames.size(); i++)
            post->addItem(mPostNames[i]);
        
        // --- hemisphere MC sample length exponent --- //
        mTrayMgr->createThickSlider(TL_TOPLEFT,
                                    SSAO_SAMPLE_LENGTH_EXPONENT_NAME,
                                    "Sample Length Exponent",
                                    SSAO_GUI_WIDTH,
                                    SSAO_GUI_VALUE_BOX_WIDTH,
                                    0,
                                    5,
                                    501); // snaps ???
        
        
        // --- bilateral photometric exponent ---
        mTrayMgr->createThickSlider(TL_TOPLEFT,
                                    SSAO_BILATERAL_PHOTOMETRIC_EXPONENT,
                                    "Photometric Exponent",
                                    SSAO_GUI_WIDTH,
                                    SSAO_GUI_VALUE_BOX_WIDTH,
                                    0,
                                    50,
                                    501); // snaps ???
        
        // --- crease shading options ---
        mTrayMgr->createThickSlider(TL_TOPLEFT,
                                    SSAO_CREASE_MINIMUM_NAME,
                                    "Minimum Crease",
                                    SSAO_GUI_WIDTH,
                                    SSAO_GUI_VALUE_BOX_WIDTH,
                                    0,
                                    1,
                                    101); // snaps ???
        mTrayMgr->createThickSlider(TL_TOPLEFT,
                                    SSAO_CREASE_RANGE_NAME,
                                    "Crease Range",
                                    SSAO_GUI_WIDTH,
                                    SSAO_GUI_VALUE_BOX_WIDTH,
                                    0,
                                    10,
                                    101); // snaps ???
        mTrayMgr->createThickSlider(TL_TOPLEFT,
                                    SSAO_CREASE_BIAS_NAME,
                                    "Bias",
                                    SSAO_GUI_WIDTH,
                                    SSAO_GUI_VALUE_BOX_WIDTH,
                                    0,
                                    2,
                                    101); // snaps ???
        mTrayMgr->createThickSlider(TL_TOPLEFT,
                                    SSAO_CREASE_AVERAGER_NAME,
                                    "Averager",
                                    SSAO_GUI_WIDTH,
                                    SSAO_GUI_VALUE_BOX_WIDTH,
                                    0,
                                    100,
                                    101); // snaps ???
        
        mTrayMgr->createThickSlider(TL_TOPLEFT,
                                    SSAO_CREASE_KERNELSIZE_NAME,
                                    "Kernel Size Bias",
                                    SSAO_GUI_WIDTH,
                                    SSAO_GUI_VALUE_BOX_WIDTH,
                                    0,
                                    10,
                                    101); // snaps ???
        // --- sample length parameter ---
        mTrayMgr->createSeparator(TL_TOPLEFT, "sep");

        mTrayMgr->createCheckBox(TL_TOPLEFT, SSAO_MODUALTE, "Modulate with scene", SSAO_GUI_WIDTH);

        // --- sample length parameter ---
        mTrayMgr->createSeparator(TL_TOPLEFT, "sep2");
        mTrayMgr->createCheckBox(TL_TOPLEFT, SSAO_SAMPLE_SPACE_NAME, "Sample in Screen Space", SSAO_GUI_WIDTH);
        mTrayMgr->createThickSlider(TL_TOPLEFT,
                                    SSAO_SAMPLE_LENGTH_SCREENSPACE,
                                    "Screen space length (in %)",
                                    SSAO_GUI_WIDTH,
                                    SSAO_GUI_VALUE_BOX_WIDTH,
                                    0,
                                    100,
                                    10001);
        
        mTrayMgr->createThickSlider(TL_TOPLEFT,
                                    SSAO_SAMPLE_LENGTH_WORLDSPACE,
                                    "World Space Length (units)",
                                    SSAO_GUI_WIDTH,
                                    SSAO_GUI_VALUE_BOX_WIDTH,
                                    0,
                                    10,
                                    10001);
        
        // --- angle bias ---
        mTrayMgr->createThickSlider(TL_TOPLEFT,
                                    SSAO_ANGLE_BIAS_NAME,
                                    "Angle Bias (radians)",
                                    SSAO_GUI_WIDTH,
                                    SSAO_GUI_VALUE_BOX_WIDTH,
                                    0,
                                    Math::HALF_PI,
                                    1001);
        
        // --- offset length ---
        mTrayMgr->createThickSlider(TL_TOPLEFT,
                                    SSAO_CRYTEK_OFFSET_SCALE_NAME,
                                    "Offset Scale (% of sample length)",
                                    SSAO_GUI_WIDTH,
                                    SSAO_GUI_VALUE_BOX_WIDTH,
                                    0,
                                    100,
                                    10001);
        
        // --- crytek edge highlight ---
        mTrayMgr->createThickSlider(TL_TOPLEFT,
                                    SSAO_CRYTEK_EDGE_HIGHLIGHT_NAME,
                                    "Edge Highlight Factor",
                                    SSAO_GUI_WIDTH,
                                    SSAO_GUI_VALUE_BOX_WIDTH,
                                    0,
                                    1,
                                    101);
        
        // --- crytek default accesibility value for invalid samples ---
        mTrayMgr->createThickSlider(TL_TOPLEFT,
                                    SSAO_CRYTEK_DEFAULT_ACCESSIBILITY_NAME,
                                    "Default Accessibility",
                                    SSAO_GUI_WIDTH,
                                    SSAO_GUI_VALUE_BOX_WIDTH,
                                    0,
                                    1,
                                    101);
        
        // --- unsharp mask kernel bias ---
        mTrayMgr->createThickSlider(TL_TOPLEFT,
                                    SSAO_UNSHARP_KERNEL_BIAS_NAME,
                                    "Kernel Size Bias",
                                    SSAO_GUI_WIDTH,
                                    SSAO_GUI_VALUE_BOX_WIDTH,
                                    0,
                                    10,
                                    101); // snaps ???
        // --- unsharp mask lambda ---
        mTrayMgr->createThickSlider(TL_TOPLEFT,
                                    SSAO_UNSHARP_LAMBDA_NAME,
                                    "Unsharp Lambda",
                                    SSAO_GUI_WIDTH,
                                    SSAO_GUI_VALUE_BOX_WIDTH,
                                    0,
                                    10,
                                    101); // snaps ???
        
        // setup values
        static_cast<Slider*>(mTrayMgr->getWidget(SSAO_CREASE_MINIMUM_NAME))->setValue(0.2f);
        static_cast<Slider*>(mTrayMgr->getWidget(SSAO_CREASE_RANGE_NAME))->setValue(1.0f);
        static_cast<Slider*>(mTrayMgr->getWidget(SSAO_CREASE_BIAS_NAME))->setValue(1.0f);
        static_cast<Slider*>(mTrayMgr->getWidget(SSAO_CREASE_AVERAGER_NAME))->setValue(24);
        static_cast<Slider*>(mTrayMgr->getWidget(SSAO_CREASE_KERNELSIZE_NAME))->setValue(3.0f);
        
        static_cast<CheckBox*>(mTrayMgr->getWidget(SSAO_SAMPLE_SPACE_NAME))->setChecked(false);
        static_cast<Slider*>(mTrayMgr->getWidget(SSAO_SAMPLE_LENGTH_SCREENSPACE))->setValue(6.0f);
        static_cast<Slider*>(mTrayMgr->getWidget(SSAO_SAMPLE_LENGTH_WORLDSPACE))->setValue(2.0f);
        static_cast<Slider*>(mTrayMgr->getWidget(SSAO_ANGLE_BIAS_NAME))->setValue(0.2f);
        static_cast<Slider*>(mTrayMgr->getWidget(SSAO_CRYTEK_OFFSET_SCALE_NAME))->setValue(1.0f);
        static_cast<Slider*>(mTrayMgr->getWidget(SSAO_CRYTEK_EDGE_HIGHLIGHT_NAME))->setValue(0.0f);
        static_cast<Slider*>(mTrayMgr->getWidget(SSAO_CRYTEK_DEFAULT_ACCESSIBILITY_NAME))->setValue(0.5f);
        
        static_cast<Slider*>(mTrayMgr->getWidget(SSAO_UNSHARP_KERNEL_BIAS_NAME))->setValue(1.0f);
        static_cast<Slider*>(mTrayMgr->getWidget(SSAO_UNSHARP_LAMBDA_NAME))->setValue(5.0f);
        
        static_cast<Slider*>(mTrayMgr->getWidget(SSAO_BILATERAL_PHOTOMETRIC_EXPONENT))->setValue(10.0f);
        
        static_cast<Slider*>(mTrayMgr->getWidget(SSAO_SAMPLE_LENGTH_EXPONENT_NAME))->setValue(1.0f);
        
        mTrayMgr->showCursor();
    }
    
    /**
     * Create the scene and load the content.
     */
    void setupContent()
    {
        mViewport->setBackgroundColour(ColourValue(0.5, 0.5, 0.5, 1));
        
        // set our camera to orbit around the origin and show cursor
        mCameraMan->setStyle(CS_FREELOOK);
        mCameraMan->setTopSpeed(20.0);
        mCameraNode->translate(Vector3(0, 10, 0));
        mCamera->setFOVy(Radian(Degree(45).valueRadians())); // i.e. 60deg * 1.3.. maya and ogre use fovX and fovY
        mCamera->setFarClipDistance(400);
        mCamera->setNearClipDistance(0.1);
        mTrayMgr->showCursor();
        
        // sibenik
        mCameraNode->setPosition(27, 9, -2);
        mCameraNode->lookAt(Vector3(-6, 2, 1), Node::TS_PARENT);;
        
        // setup all meshes
        for (unsigned int i = 0; i < mMeshNames.size(); i++) {
            Entity* ent = mSceneMgr->createEntity(mMeshNames[i], mMeshNames[i] + ".mesh");
            ent->setVisible(false);
            
            mSceneMgr->getRootSceneNode()->attachObject(ent);
            mMeshes.push_back(ent);
        }
        mCurrentMeshIndex = 0;
        mMeshes[mCurrentMeshIndex]->setVisible(true);
        
        setupCompositors();
        
        setupControls();
        
        changeCompositor(mCompositorNames[0]);
        changePost(mPostNames[0]);

        mGBufSchemeHandler = new SSAOGBufferSchemeHandler();
        MaterialManager::getSingleton().addListener(mGBufSchemeHandler, "GBuffer");
    }
    
    /**
     * Change the current displayed mesh to the new mesh identified by its index.
     * @param index The index of the new mesh in the mesh vector.
     */
    void changeMesh(int index)
    {
        mMeshes[mCurrentMeshIndex]->setVisible(false);
        mMeshes[index]->setVisible(true);
        mCurrentMeshIndex = index;
    }
    
    /**
     * Change the compositor to be used.
     * @param compositor The name of the compositor
     */
    void changeCompositor(Ogre::String compositor)
    {
        CompositorManager::getSingleton().setCompositorEnabled(mViewport, mCurrentCompositor, false);
        mCurrentCompositor = compositor;
        CompositorManager::getSingleton().setCompositorEnabled(mViewport, mCurrentCompositor, true);
        
        if (compositor == "SSAO/CreaseShading")
        {
            mTrayMgr->getWidget(SSAO_CREASE_MINIMUM_NAME)->show();
            mTrayMgr->moveWidgetToTray(SSAO_CREASE_MINIMUM_NAME, TL_TOPLEFT);
            mTrayMgr->getWidget(SSAO_CREASE_RANGE_NAME)->show();
            mTrayMgr->moveWidgetToTray(SSAO_CREASE_RANGE_NAME, TL_TOPLEFT);
            mTrayMgr->getWidget(SSAO_CREASE_BIAS_NAME)->show();
            mTrayMgr->moveWidgetToTray(SSAO_CREASE_BIAS_NAME, TL_TOPLEFT);
            mTrayMgr->getWidget(SSAO_CREASE_AVERAGER_NAME)->show();
            mTrayMgr->moveWidgetToTray(SSAO_CREASE_AVERAGER_NAME, TL_TOPLEFT);
            mTrayMgr->getWidget(SSAO_CREASE_KERNELSIZE_NAME)->show();
            mTrayMgr->moveWidgetToTray(SSAO_CREASE_KERNELSIZE_NAME, TL_TOPLEFT);
        }
        else
        {
            mTrayMgr->getWidget(SSAO_CREASE_MINIMUM_NAME)->hide();
            mTrayMgr->removeWidgetFromTray(SSAO_CREASE_MINIMUM_NAME);
            mTrayMgr->getWidget(SSAO_CREASE_RANGE_NAME)->hide();
            mTrayMgr->removeWidgetFromTray(SSAO_CREASE_RANGE_NAME);
            mTrayMgr->getWidget(SSAO_CREASE_BIAS_NAME)->hide();
            mTrayMgr->removeWidgetFromTray(SSAO_CREASE_BIAS_NAME);
            mTrayMgr->getWidget(SSAO_CREASE_AVERAGER_NAME)->hide();
            mTrayMgr->removeWidgetFromTray(SSAO_CREASE_AVERAGER_NAME);
            mTrayMgr->getWidget(SSAO_CREASE_KERNELSIZE_NAME)->hide();
            mTrayMgr->removeWidgetFromTray(SSAO_CREASE_KERNELSIZE_NAME);
        }
        
        if (compositor == "SSAO/UnsharpMask")
        {
            mTrayMgr->getWidget(SSAO_UNSHARP_KERNEL_BIAS_NAME)->show();
            mTrayMgr->moveWidgetToTray(SSAO_UNSHARP_KERNEL_BIAS_NAME, TL_TOPLEFT);
            mTrayMgr->getWidget(SSAO_UNSHARP_LAMBDA_NAME)->show();
            mTrayMgr->moveWidgetToTray(SSAO_UNSHARP_LAMBDA_NAME, TL_TOPLEFT);
        }
        else
        {
            mTrayMgr->getWidget(SSAO_UNSHARP_KERNEL_BIAS_NAME)->hide();
            mTrayMgr->removeWidgetFromTray(SSAO_UNSHARP_KERNEL_BIAS_NAME);
            mTrayMgr->getWidget(SSAO_UNSHARP_LAMBDA_NAME)->hide();
            mTrayMgr->removeWidgetFromTray(SSAO_UNSHARP_LAMBDA_NAME);
        }
        
        if (compositor == "SSAO/Crytek" || compositor == "SSAO/HorizonBased" || compositor == "SSAO/HemisphereMC" || compositor == "SSAO/Volumetric")
        {
            mTrayMgr->getWidget(SSAO_SAMPLE_SPACE_NAME)->show();
            mTrayMgr->moveWidgetToTray(SSAO_SAMPLE_SPACE_NAME, TL_TOPLEFT);
            CheckBox *samplingCheckBox = (CheckBox *)mTrayMgr->getWidget(SSAO_SAMPLE_SPACE_NAME);
            samplingCheckBox->setChecked(samplingCheckBox->isChecked()); // easy way to update the sliders...
        }
        else
        {
            mTrayMgr->getWidget(SSAO_SAMPLE_SPACE_NAME)->hide();
            mTrayMgr->removeWidgetFromTray(SSAO_SAMPLE_SPACE_NAME);
            mTrayMgr->getWidget(SSAO_SAMPLE_LENGTH_SCREENSPACE)->hide();
            mTrayMgr->removeWidgetFromTray(SSAO_SAMPLE_LENGTH_SCREENSPACE);
            mTrayMgr->getWidget(SSAO_SAMPLE_LENGTH_WORLDSPACE)->hide();
            mTrayMgr->removeWidgetFromTray(SSAO_SAMPLE_LENGTH_WORLDSPACE);
        }
        
        if (compositor == "SSAO/HemisphereMC")
        {
            mTrayMgr->getWidget(SSAO_SAMPLE_LENGTH_EXPONENT_NAME)->show();
            mTrayMgr->moveWidgetToTray(SSAO_SAMPLE_LENGTH_EXPONENT_NAME, TL_TOPLEFT);
        }
        else
        {
            mTrayMgr->getWidget(SSAO_SAMPLE_LENGTH_EXPONENT_NAME)->hide();
            mTrayMgr->removeWidgetFromTray(SSAO_SAMPLE_LENGTH_EXPONENT_NAME);
        }
        
        if (compositor == "SSAO/HorizonBased")
        {
            mTrayMgr->getWidget(SSAO_ANGLE_BIAS_NAME)->show();
            mTrayMgr->moveWidgetToTray(SSAO_ANGLE_BIAS_NAME, TL_TOPLEFT);
        }
        else
        {
            mTrayMgr->getWidget(SSAO_ANGLE_BIAS_NAME)->hide();
            mTrayMgr->removeWidgetFromTray(SSAO_ANGLE_BIAS_NAME);
        }
        
        if (compositor == "SSAO/Crytek")
        {
            mTrayMgr->getWidget(SSAO_CRYTEK_OFFSET_SCALE_NAME)->show();
            mTrayMgr->moveWidgetToTray(SSAO_CRYTEK_OFFSET_SCALE_NAME, TL_TOPLEFT);
            mTrayMgr->getWidget(SSAO_CRYTEK_EDGE_HIGHLIGHT_NAME)->show();
            mTrayMgr->moveWidgetToTray(SSAO_CRYTEK_EDGE_HIGHLIGHT_NAME, TL_TOPLEFT);
            mTrayMgr->getWidget(SSAO_CRYTEK_DEFAULT_ACCESSIBILITY_NAME)->show();
            mTrayMgr->moveWidgetToTray(SSAO_CRYTEK_DEFAULT_ACCESSIBILITY_NAME, TL_TOPLEFT);
        }
        else
        {
            mTrayMgr->getWidget(SSAO_CRYTEK_OFFSET_SCALE_NAME)->hide();
            mTrayMgr->removeWidgetFromTray(SSAO_CRYTEK_OFFSET_SCALE_NAME);
            mTrayMgr->getWidget(SSAO_CRYTEK_EDGE_HIGHLIGHT_NAME)->hide();
            mTrayMgr->removeWidgetFromTray(SSAO_CRYTEK_EDGE_HIGHLIGHT_NAME);
            mTrayMgr->getWidget(SSAO_CRYTEK_DEFAULT_ACCESSIBILITY_NAME)->hide();
            mTrayMgr->removeWidgetFromTray(SSAO_CRYTEK_DEFAULT_ACCESSIBILITY_NAME);
        }
    }
    
    /**
     * Change the post filter to be used.
     * @param post The name of the new post processing filter.
     */
    void changePost(Ogre::String post)
    {
        CompositorManager::getSingleton().setCompositorEnabled(mViewport, mCurrentPost, false);
        mCurrentPost = post;
        CompositorManager::getSingleton().setCompositorEnabled(mViewport, mCurrentPost, true);
        
        if (post == "SSAO/Post/CrossBilateralFilter")
        {
            mTrayMgr->getWidget(SSAO_BILATERAL_PHOTOMETRIC_EXPONENT)->show();
            mTrayMgr->moveWidgetToTray(SSAO_BILATERAL_PHOTOMETRIC_EXPONENT, TL_TOPLEFT);
        }
        else
        {
            mTrayMgr->getWidget(SSAO_BILATERAL_PHOTOMETRIC_EXPONENT)->hide();
            mTrayMgr->removeWidgetFromTray(SSAO_BILATERAL_PHOTOMETRIC_EXPONENT);
        }
    }
    
    // sdkTray listener callbacks
    void itemSelected(SelectMenu* menu)
    {
        if (menu->getName() == SSAO_OBJECT_MENU_NAME)
            changeMesh(menu->getSelectionIndex());
        
        else if (menu->getName() == SSAO_COMPOSITOR_MENU_NAME)
            changeCompositor(menu->getSelectedItem());
        
        else if (menu->getName() == SSAO_POST_MENU_NAME)
            changePost(menu->getSelectedItem());    
        
        else if (menu->getName() == SSAO_CAMERA_MENU_NAME)
        {
            if (menu->getSelectedItem() == SSAO_CAMERA_SIBENIK)
            {
                mCameraNode->setPosition(27, 9, -2);
                mCameraNode->lookAt(Vector3(-6, 2, 1), Node::TS_PARENT);
            }
            else if (menu->getSelectedItem() == SSAO_CAMERA_CORNELL)
            {
                mCameraNode->setPosition(0, 5, 20);
                mCameraNode->lookAt(Vector3(0, 5, 0), Node::TS_PARENT);
            }
        }
    }
    
    void sliderMoved(Slider* slider)
    {
        if (slider->getName() == SSAO_CREASE_MINIMUM_NAME)
            setUniform("SSAO/CreaseShading", "cMinimumCrease", slider->getValue());
        
        else if (slider->getName() == SSAO_CREASE_BIAS_NAME)
            setUniform("SSAO/CreaseShading", "cBias", slider->getValue());
        
        else if (slider->getName() == SSAO_CREASE_AVERAGER_NAME)
            setUniform("SSAO/CreaseShading", "cAverager", slider->getValue());
        
        else if (slider->getName() == SSAO_CREASE_RANGE_NAME)
            setUniform("SSAO/CreaseShading", "cRange", slider->getValue() * slider->getValue());
        
        else if (slider->getName() == SSAO_CREASE_KERNELSIZE_NAME)
            setUniform("SSAO/CreaseShading", "cKernelSize", slider->getValue());
        
        else if (slider->getName() == SSAO_SAMPLE_LENGTH_SCREENSPACE)
        {
            setUniform("SSAO/Crytek", "cSampleLengthScreenSpace", slider->getValue()/100.0f);
            setUniform("SSAO/HorizonBased", "cSampleLengthScreenSpace", slider->getValue()/100.0f);
            setUniform("SSAO/HemisphereMC", "cSampleLengthScreenSpace", slider->getValue()/100.0f);
            setUniform("SSAO/Volumetric", "cSampleLengthScreenSpace", slider->getValue()/100.0f);
        }
        else if (slider->getName() == SSAO_SAMPLE_LENGTH_WORLDSPACE)
        {
            setUniform("SSAO/Crytek", "cSampleLengthWorldSpace", slider->getValue());
            setUniform("SSAO/HorizonBased", "cSampleLengthWorldSpace", slider->getValue());
            setUniform("SSAO/HemisphereMC", "cSampleLengthWorldSpace", slider->getValue());
            setUniform("SSAO/Volumetric", "cSampleLengthWorldSpace", slider->getValue());
        }
        
        else if (slider->getName() == SSAO_ANGLE_BIAS_NAME)
            setUniform("SSAO/HorizonBased", "cAngleBias", slider->getValue());
        
        else if (slider->getName() == SSAO_CRYTEK_OFFSET_SCALE_NAME)
            setUniform("SSAO/Crytek", "cOffsetScale", slider->getValue()/100);
        
        else if (slider->getName() == SSAO_CRYTEK_EDGE_HIGHLIGHT_NAME)
            setUniform("SSAO/Crytek", "cEdgeHighlight", 2.0f - slider->getValue());
        
        else if (slider->getName() == SSAO_CRYTEK_DEFAULT_ACCESSIBILITY_NAME)
            setUniform("SSAO/Crytek", "cDefaultAccessibility", slider->getValue());
        
        else if (slider->getName() == SSAO_UNSHARP_KERNEL_BIAS_NAME)
        {
            setUniform("SSAO/UnsharpMask/GaussianBlurY", "cKernelWidthBias", slider->getValue());
            setUniform("SSAO/UnsharpMask/GaussianBlurX", "cKernelWidthBias", slider->getValue());
        }
        
        else if (slider->getName() == SSAO_UNSHARP_LAMBDA_NAME)
            setUniform("SSAO/UnsharpMask", "cLambda", slider->getValue() * slider->getValue());
        
        else if (slider->getName() == SSAO_BILATERAL_PHOTOMETRIC_EXPONENT)
        {
            setUniform("SSAO/HorizonBased/CrossBilateralFilter/X", "cPhotometricExponent", slider->getValue());
            setUniform("SSAO/HorizonBased/CrossBilateralFilter/Y", "cPhotometricExponent", slider->getValue());
        }
        
        else if(slider->getName() == SSAO_SAMPLE_LENGTH_EXPONENT_NAME)
            setUniform("SSAO/HemisphereMC", "cSampleLengthExponent", slider->getValue());
    }
    
    void checkBoxToggled(OgreBites::CheckBox *box) 
    {
        if(box->getName() == SSAO_MODUALTE)
        {
            if (box->isChecked())
            {
                CompositorManager::getSingleton().addCompositor(mViewport, "SSAO/Post/Modulate");
                CompositorManager::getSingleton().setCompositorEnabled(mViewport, "SSAO/Post/Modulate", true);
                mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
                mLight = mSceneMgr->createLight();
                mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(30, 80, 30))->attachObject(mLight);
            }
            else
            {  
                mSceneMgr->destroyLight(mLight);
                mLight = NULL;
                CompositorManager::getSingleton().setCompositorEnabled(mViewport, "SSAO/Post/Modulate", false);
                CompositorManager::getSingleton().removeCompositor(mViewport, "SSAO/Post/Modulate");
            }
        }
        else if (box->getName() == SSAO_SAMPLE_SPACE_NAME)
        {
            setUniform("SSAO/Crytek", "cSampleInScreenspace", box->isChecked());
            setUniform("SSAO/HorizonBased", "cSampleInScreenspace", box->isChecked());
            setUniform("SSAO/HemisphereMC", "cSampleInScreenspace", box->isChecked());
            setUniform("SSAO/Volumetric", "cSampleInScreenspace", box->isChecked());

            if (box->isChecked()) // we sample in screen space 
            {
                mTrayMgr->removeWidgetFromTray(SSAO_SAMPLE_LENGTH_WORLDSPACE);
                mTrayMgr->getWidget(SSAO_SAMPLE_LENGTH_WORLDSPACE)->hide();
                mTrayMgr->moveWidgetToTray(SSAO_SAMPLE_LENGTH_SCREENSPACE, TL_TOPLEFT);
                mTrayMgr->getWidget(SSAO_SAMPLE_LENGTH_SCREENSPACE)->show();
            }
            else
            {
                mTrayMgr->removeWidgetFromTray(SSAO_SAMPLE_LENGTH_SCREENSPACE);
                mTrayMgr->getWidget(SSAO_SAMPLE_LENGTH_SCREENSPACE)->hide();
                mTrayMgr->moveWidgetToTray(SSAO_SAMPLE_LENGTH_WORLDSPACE, TL_TOPLEFT);
                mTrayMgr->getWidget(SSAO_SAMPLE_LENGTH_WORLDSPACE)->show();
            }
        }
    }
    
    // The following three methods are for mouse input
    /** @see Sample::pointerPressed. */
    bool mousePressed(const MouseButtonEvent& evt)
    {
        if (mTrayMgr->mousePressed(evt)) 
            return true;
        if (evt.button == BUTTON_LEFT)     
            mTrayMgr->hideCursor();  // hide the cursor if user left-clicks in the scene            
        
        return true;
    }
    
    /** @see Sample::mouseReleased. */
    bool mouseReleased(const MouseButtonEvent& evt)
    {
        if (mTrayMgr->mouseReleased(evt)) 
            return true;
        if (evt.button == BUTTON_LEFT) 
            mTrayMgr->showCursor();  // unhide the cursor if user lets go of LMB
        
        return true;
    }
    
    /** @see Sample::mouseMoved. */
    bool mouseMoved(const MouseMotionEvent& evt)
    {
        // only rotate the camera if cursor is hidden
        if (mTrayMgr->isCursorVisible())
            mTrayMgr->mouseMoved(evt);
        else 
        {
            mCameraMan->mouseMoved(evt);
            static_cast<SelectMenu*>(mTrayMgr->getWidget(SSAO_CAMERA_MENU_NAME))->selectItem(SSAO_USER_CAMERA_ITEM);
        }
        
        return true;
    }

    /**
     * Set the uniform value in the compositor
     * @param material The material that contains the uniform
     * @param uniform The name of the uniform parameter
     * @param value The value
     */
    void setUniform(const String& material, const String& uniform, float value)
    {
        MaterialManager::getSingleton().getByName(material)->getTechnique(0)->
        getPass(0)->getFragmentProgramParameters()->setNamedConstant(uniform, value);
        CompositorManager::getSingleton().getCompositorChain(mViewport)->_markDirty();
    }
};



#endif /* __SSAO_H__ */
