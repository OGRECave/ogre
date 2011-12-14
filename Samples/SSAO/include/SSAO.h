/*
 -----------------------------------------------------------------------------
 This source file is part of OGRE
 (Object-oriented Graphics Rendering Engine)
 For the latest info, see http://www.ogre3d.org/
 
 Copyright (c) 2000-2011 Torus Knot Software Ltd
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

const String SSAO_OBJECT_MENU_NAME = "ObjectType";
const String SSAO_CAMERA_MENU_NAME = "Camera";

const String SSAO_COMPOSITOR_MENU_NAME = "Compositor";
const String SSAO_POST_MENU_NAME = "Post";
const String SSAO_CREASE_MINIMUM_NAME = "CreaseMinimum";
const String SSAO_CREASE_RANGE_NAME = "mCreaseRange";
const String SSAO_CREASE_BIAS_NAME = "mCreaseBias";
const String SSAO_CREASE_AVERAGER_NAME = "mCreaseAverager";
const String SSAO_CREASE_KERNELSIZE_NAME = "mCreaseKernelsize";

const String SSAO_SAMPLE_SPACE_NAME = "sampleSpace";
const String SSAO_SAMPLE_LENGTH_SCREENSPACE = "sampleScreenSpace";
const String SSAO_SAMPLE_LENGTH_WORLDSPACE = "sampleWorldSpace";
const String SSAO_SAMPLE_LENGTH_EXPONENT_NAME = "sampleLengthExponent";

const String SSAO_ANGLE_BIAS_NAME = "angleBias";

const String SSAO_CRYTEK_OFFSET_SCALE_NAME = "offsetScale";
const String SSAO_CRYTEK_EDGE_HIGHLIGHT_NAME = "edgeHighlight";
const String SSAO_CRYTEK_DEFAULT_ACCESSIBILITY_NAME = "defaultOcclusion";

const String SSAO_UNSHARP_KERNEL_BIAS_NAME = "kernelBias";
const String SSAO_UNSHARP_LAMBDA_NAME = "lambda";

const String SSAO_BILATERAL_PHOTOMETRIC_EXPONENT = "photometricExponent";

const String SSAO_USER_CAMERA_ITEM = "User Camera";
const String SSAO_CAMERA_SIBENIK = "Sibenik";
const String SSAO_CAMERA_CORNELL = "Cornell Box";

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

	std::vector<Widget *> mCreasePanel;
	std::vector<Widget *> mUnsharpPanel;
	std::vector<Widget *> mBilateralPanel;
	
	CheckBox* mSamplingCheckbox;
	Slider* mScreenSpaceSlider;
	Slider* mWorldSpaceSlider;

	Slider* mSampleLengthExponent;

	Slider* mAngleBiasSlider;
	Slider* mOffsetStepSlider;
	Slider* mEdgeHighlight;
	Slider* mDefaultAccessibility;

	SelectMenu* mCameraMenu;

	/**
	 * Show all the widgets in the panel
	 * @see Sample_SSAO::showWidget
	 */
    void showPanel(std::vector<Widget *> panel)
    {
        for (unsigned int i = 0; i < panel.size(); i++)
        {
            mTrayMgr->moveWidgetToTray(panel[i], TL_TOPLEFT);
            panel[i]->show();
        }
    }

	/**
	 * Hide all the widgets in the panel
	 * @see Sample_SSAO::hideWidget
	 */
    void hidePanel(std::vector<Widget *> panel)
    {
        for (unsigned int i = 0; i < panel.size(); i++)
        {
            mTrayMgr->removeWidgetFromTray(panel[i]);
            panel[i]->hide();
        }
    }

	/**
	 * Show the given widget. This sets it visible and re-adds it to the panel
	 */
    void showWidget(Widget* widget)
    {
        mTrayMgr->moveWidgetToTray(widget, TL_TOPLEFT);
        widget->show();
    }

	/**
	 * Hide the given widget. This sets it to not visible and also removes it
	 * from the panel.
	 */
    void hideWidget(Widget* widget)
    {
        mTrayMgr->removeWidgetFromTray(widget);
        widget->hide();
    }
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
    }

    StringVector getRequiredPlugins()
    {
        StringVector names;
        names.push_back("Cg Program Manager");
        return names;
    }

    void testCapabilities(const RenderSystemCapabilities* caps)
    {
        if (!caps->hasCapability(RSC_VERTEX_PROGRAM) || !caps->hasCapability(RSC_FRAGMENT_PROGRAM))
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your graphics card does not support vertex and fragment"
                        " programs, so you cannot run this sample. Sorry!", "Sample_SSAO::testCapabilities");
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
        mCameraMenu = mTrayMgr->createThickSelectMenu(TL_TOPLEFT, SSAO_CAMERA_MENU_NAME, "Camera Position", SSAO_GUI_WIDTH, 16);
        mCameraMenu->addItem(SSAO_USER_CAMERA_ITEM);
        mCameraMenu->addItem(SSAO_CAMERA_CORNELL);
        mCameraMenu->addItem(SSAO_CAMERA_SIBENIK);
        
        // --- select compositor menu ---
        SelectMenu* compositor = mTrayMgr->createThickSelectMenu(TL_TOPLEFT, SSAO_COMPOSITOR_MENU_NAME, "Compositor: ", SSAO_GUI_WIDTH, 16);
        for (unsigned int i = 0; i < mCompositorNames.size(); i++)
            compositor->addItem(mCompositorNames[i]);
        
        // --- select post filter menu ---
        SelectMenu * post = mTrayMgr->createThickSelectMenu(TL_TOPLEFT, SSAO_POST_MENU_NAME, "Post Filter: ", SSAO_GUI_WIDTH, 16);
        for (unsigned int i = 0; i < mPostNames.size(); i++)
            post->addItem(mPostNames[i]);
        
        // --- hemisphere MC sample length exponent --- //
        mSampleLengthExponent = mTrayMgr->createThickSlider(TL_TOPLEFT,
                                                            SSAO_SAMPLE_LENGTH_EXPONENT_NAME,
                                                            "Sample Length Exponent",
                                                            SSAO_GUI_WIDTH,
                                                            SSAO_GUI_VALUE_BOX_WIDTH,
                                                            0,
                                                            5,
                                                            501); // snaps ???
        
        
        // --- bilateral photometric exponent ---
        Slider* photometricExponent = mTrayMgr->createThickSlider(TL_TOPLEFT,
                                                                  SSAO_BILATERAL_PHOTOMETRIC_EXPONENT,
                                                                  "Photometric Exponent",
                                                                  SSAO_GUI_WIDTH,
                                                                  SSAO_GUI_VALUE_BOX_WIDTH,
                                                                  0,
                                                                  50,
                                                                  501); // snaps ???
        mBilateralPanel.push_back(photometricExponent);
        
        // --- crease shading options ---
        Slider* mCreaseMinimumSlider = mTrayMgr->createThickSlider(TL_TOPLEFT,
                                                                   SSAO_CREASE_MINIMUM_NAME,
                                                                   "Minimum Crease",
                                                                   SSAO_GUI_WIDTH,
                                                                   SSAO_GUI_VALUE_BOX_WIDTH,
                                                                   0,
                                                                   1,
                                                                   101); // snaps ???
        mCreasePanel.push_back(mCreaseMinimumSlider);
        
        Slider* mCreaseRange = mTrayMgr->createThickSlider(TL_TOPLEFT,
                                                           SSAO_CREASE_RANGE_NAME,
                                                           "Crease Range",
                                                           SSAO_GUI_WIDTH,
                                                           SSAO_GUI_VALUE_BOX_WIDTH,
                                                           0,
                                                           10,
                                                           101); // snaps ???
        mCreasePanel.push_back(mCreaseRange);
        
        Slider* mCreaseBias = mTrayMgr->createThickSlider(TL_TOPLEFT,
                                                          SSAO_CREASE_BIAS_NAME,
                                                          "Bias",
                                                          SSAO_GUI_WIDTH,
                                                          SSAO_GUI_VALUE_BOX_WIDTH,
                                                          0,
                                                          2,
                                                          101); // snaps ???
        mCreasePanel.push_back(mCreaseBias);
        
        Slider* mCreaseAverager = mTrayMgr->createThickSlider(TL_TOPLEFT,
                                                              SSAO_CREASE_AVERAGER_NAME,
                                                              "Averager",
                                                              SSAO_GUI_WIDTH,
                                                              SSAO_GUI_VALUE_BOX_WIDTH,
                                                              0,
                                                              100,
                                                              101); // snaps ???
        mCreasePanel.push_back(mCreaseAverager);
        
        Slider* mCreaseKernelsize = mTrayMgr->createThickSlider(TL_TOPLEFT,
                                                                SSAO_CREASE_KERNELSIZE_NAME,
                                                                "Kernel Size Bias",
                                                                SSAO_GUI_WIDTH,
                                                                SSAO_GUI_VALUE_BOX_WIDTH,
                                                                0,
                                                                10,
                                                                101); // snaps ???
        mCreasePanel.push_back(mCreaseKernelsize);
        
        // --- sample length parameter ---
        mTrayMgr->createSeparator(TL_TOPLEFT, "sep");
        mSamplingCheckbox = mTrayMgr->createCheckBox(TL_TOPLEFT, SSAO_SAMPLE_SPACE_NAME, "Sample in Screen Space", SSAO_GUI_WIDTH);
        mScreenSpaceSlider = mTrayMgr->createThickSlider(TL_TOPLEFT,
                                                         SSAO_SAMPLE_LENGTH_SCREENSPACE,
                                                         "Screen space length (in %)",
                                                         SSAO_GUI_WIDTH,
                                                         SSAO_GUI_VALUE_BOX_WIDTH,
                                                         0,
                                                         100,
                                                         10001);
        
        mWorldSpaceSlider = mTrayMgr->createThickSlider(TL_TOPLEFT,
                                                        SSAO_SAMPLE_LENGTH_WORLDSPACE,
                                                        "World Space Length (units)",
                                                        SSAO_GUI_WIDTH,
                                                        SSAO_GUI_VALUE_BOX_WIDTH,
                                                        0,
                                                        10,
                                                        10001);
        
        // --- angle bias ---
        mAngleBiasSlider = mTrayMgr->createThickSlider(TL_TOPLEFT,
                                                       SSAO_ANGLE_BIAS_NAME,
                                                       "Angle Bias (radians)",
                                                       SSAO_GUI_WIDTH,
                                                       SSAO_GUI_VALUE_BOX_WIDTH,
                                                       0,
                                                       Math::HALF_PI,
                                                       1001);
        
        // --- offset length ---
        mOffsetStepSlider = mTrayMgr->createThickSlider(TL_TOPLEFT,
                                                        SSAO_CRYTEK_OFFSET_SCALE_NAME,
                                                        "Offset Scale (% of sample length)",
                                                        SSAO_GUI_WIDTH,
                                                        SSAO_GUI_VALUE_BOX_WIDTH,
                                                        0,
                                                        100,
                                                        10001);
        
        // --- crytek edge highlight ---
        mEdgeHighlight = mTrayMgr->createThickSlider(TL_TOPLEFT,
                                                     SSAO_CRYTEK_EDGE_HIGHLIGHT_NAME,
                                                     "Edge Highlight Factor",
                                                     SSAO_GUI_WIDTH,
                                                     SSAO_GUI_VALUE_BOX_WIDTH,
                                                     0,
                                                     1,
                                                     101);
        
        // --- crytek default accesibility value for invalid samples ---
        mDefaultAccessibility = mTrayMgr->createThickSlider(TL_TOPLEFT,
                                                            SSAO_CRYTEK_DEFAULT_ACCESSIBILITY_NAME,
                                                            "Default Accessibility",
                                                            SSAO_GUI_WIDTH,
                                                            SSAO_GUI_VALUE_BOX_WIDTH,
                                                            0,
                                                            1,
                                                            101);
        
        // --- unsharp mask kernel bias ---
        Slider* mUnsharpKernelBias = mTrayMgr->createThickSlider(TL_TOPLEFT,
                                                                 SSAO_UNSHARP_KERNEL_BIAS_NAME,
                                                                 "Kernel Size Bias",
                                                                 SSAO_GUI_WIDTH,
                                                                 SSAO_GUI_VALUE_BOX_WIDTH,
                                                                 0,
                                                                 10,
                                                                 101); // snaps ???
        mUnsharpPanel.push_back(mUnsharpKernelBias);
        
        // --- unsharp mask lambda ---
        Slider* mUnsharpLambda = mTrayMgr->createThickSlider(TL_TOPLEFT,
                                                             SSAO_UNSHARP_LAMBDA_NAME,
                                                             "Unsharp Lambda",
                                                             SSAO_GUI_WIDTH,
                                                             SSAO_GUI_VALUE_BOX_WIDTH,
                                                             0,
                                                             10,
                                                             101); // snaps ???
        mUnsharpPanel.push_back(mUnsharpLambda);
        
        // setup values
        mCreaseMinimumSlider->setValue(0.2);
        mCreaseRange->setValue(1.0f);
        mCreaseBias->setValue(1.0f);
        mCreaseAverager->setValue(24);
        mCreaseKernelsize->setValue(3.0f);
        
        mSamplingCheckbox->setChecked(false);
        mScreenSpaceSlider->setValue(6.0f);
        mWorldSpaceSlider->setValue(2.0f);
        mAngleBiasSlider->setValue(0.2f);
        mOffsetStepSlider->setValue(1.0f);
        mEdgeHighlight->setValue(0.0f);
        mDefaultAccessibility->setValue(0.5f);
        
        mUnsharpKernelBias->setValue(1.0f);
        mUnsharpLambda->setValue(5.0f);
        
        photometricExponent->setValue(10.0f);
        
        mSampleLengthExponent->setValue(1.0f);
        
        
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
        mCamera->move(Vector3(0, 10, 0));
        mCamera->setFOVy(Radian(Degree(45).valueRadians())); // i.e. 60deg * 1.3.. maya and ogre use fovX and fovY
        mCamera->setFarClipDistance(400);
        mCamera->setNearClipDistance(0.1);
        mTrayMgr->showCursor();

        // sibenik
        mCamera->setPosition(27, 9, -2);
        mCamera->lookAt(Vector3(-6, 2, 1));;
        
        // setup all meshes
        for (unsigned int i = 0; i < mMeshNames.size(); i++) {
            Entity* ent = mSceneMgr->createEntity(mMeshNames[i], mMeshNames[i] + ".mesh");
            ent->setVisible(false);
            ent->setMaterialName("SSAO/GBuffer");
            
            mSceneMgr->getRootSceneNode()->attachObject(ent);
            mMeshes.push_back(ent);
        }
        mCurrentMeshIndex = 0;
        mMeshes[mCurrentMeshIndex]->setVisible(true);
        
        setupCompositors();
        
        setupControls();
        
        changeCompositor(mCompositorNames[0]);
        changePost(mPostNames[0]);
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
            showPanel(mCreasePanel);
        else
            hidePanel(mCreasePanel);
        
        if (compositor == "SSAO/UnsharpMask")
            showPanel(mUnsharpPanel);
        else
            hidePanel(mUnsharpPanel);
        
        if (compositor == "SSAO/Crytek" || compositor == "SSAO/HorizonBased" || compositor == "SSAO/HemisphereMC" || compositor == "SSAO/Volumetric")
        {
            showWidget(mSamplingCheckbox);
            mSamplingCheckbox->setChecked(mSamplingCheckbox->isChecked()); // easy way to update the sliders...
        }
        else
        {
            hideWidget(mSamplingCheckbox);
            hideWidget(mScreenSpaceSlider);
            hideWidget(mWorldSpaceSlider);
        }
        
        if (compositor == "SSAO/HemisphereMC")
            showWidget(mSampleLengthExponent);
        else
            hideWidget(mSampleLengthExponent);
        
        if (compositor == "SSAO/HorizonBased")
            showWidget(mAngleBiasSlider);
        else
            hideWidget(mAngleBiasSlider);
        
        
        if (compositor == "SSAO/Crytek")
        {
            showWidget(mOffsetStepSlider);
            showWidget(mEdgeHighlight);
            showWidget(mDefaultAccessibility);
        }
        else
        {
            hideWidget(mOffsetStepSlider);
            hideWidget(mEdgeHighlight);
            hideWidget(mDefaultAccessibility);
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
            showPanel(mBilateralPanel);
        else
            hidePanel(mBilateralPanel);
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
                mCamera->setPosition(27, 9, -2);
                mCamera->lookAt(Vector3(-6, 2, 1));
            }
            else if (menu->getSelectedItem() == SSAO_CAMERA_CORNELL)
            {
                mCamera->setPosition(0, 5, 20);
                mCamera->lookAt(Vector3(0, 5, 0));
            }
        }
    }

    void sliderMoved(Slider* slider)
    {
        if (slider->getName() == SSAO_CREASE_MINIMUM_NAME)
            setUniform("SSAO/CreaseShading", "SSAO/CreaseShading", "cMinimumCrease", slider->getValue(), false, 1);
        
        else if (slider->getName() == SSAO_CREASE_BIAS_NAME)
            setUniform("SSAO/CreaseShading", "SSAO/CreaseShading", "cBias", slider->getValue(), false, 1);
        
        else if (slider->getName() == SSAO_CREASE_AVERAGER_NAME)
            setUniform("SSAO/CreaseShading", "SSAO/CreaseShading", "cAverager", slider->getValue(), false, 1);
        
        else if (slider->getName() == SSAO_CREASE_RANGE_NAME)
            setUniform("SSAO/CreaseShading", "SSAO/CreaseShading", "cRange", slider->getValue() * slider->getValue(), false, 1);
        
        else if (slider->getName() == SSAO_CREASE_KERNELSIZE_NAME)
            setUniform("SSAO/CreaseShading", "SSAO/CreaseShading", "cKernelSize", slider->getValue(), false, 1);
        
        else if (slider->getName() == SSAO_SAMPLE_LENGTH_SCREENSPACE)
        {
            setUniform("SSAO/Crytek", "SSAO/Crytek", "cSampleLengthScreenSpace", slider->getValue()/100.0f, false, 1);
            setUniform("SSAO/HorizonBased", "SSAO/HorizonBased", "cSampleLengthScreenSpace", slider->getValue()/100.0f, false, 1);
            setUniform("SSAO/HemisphereMC", "SSAO/HemisphereMC", "cSampleLengthScreenSpace", slider->getValue()/100.0f, false, 1);
            setUniform("SSAO/Volumetric", "SSAO/Volumetric", "cSampleLengthScreenSpace", slider->getValue()/100.0f, false, 1);
        }
        else if (slider->getName() == SSAO_SAMPLE_LENGTH_WORLDSPACE)
        {
            setUniform("SSAO/Crytek", "SSAO/Crytek", "cSampleLengthWorldSpace", slider->getValue(), false, 1);
            setUniform("SSAO/HorizonBased", "SSAO/HorizonBased", "cSampleLengthWorldSpace", slider->getValue(), false, 1);
            setUniform("SSAO/HemisphereMC", "SSAO/HemisphereMC", "cSampleLengthWorldSpace", slider->getValue(), false, 1);
            setUniform("SSAO/Volumetric", "SSAO/Volumetric", "cSampleLengthWorldSpace", slider->getValue(), false, 1);
        }
        
        else if (slider->getName() == SSAO_ANGLE_BIAS_NAME)
            setUniform("SSAO/HorizonBased", "SSAO/HorizonBased", "cAngleBias", slider->getValue(), false, 1);
        
        else if (slider->getName() == SSAO_CRYTEK_OFFSET_SCALE_NAME)
            setUniform("SSAO/Crytek", "SSAO/Crytek", "cOffsetScale", slider->getValue()/100, false, 1);
        
        else if (slider->getName() == SSAO_CRYTEK_EDGE_HIGHLIGHT_NAME)
            setUniform("SSAO/Crytek", "SSAO/Crytek", "cEdgeHighlight", 2.0f - slider->getValue(), false, 1);
        
        else if (slider->getName() == SSAO_CRYTEK_DEFAULT_ACCESSIBILITY_NAME)
            setUniform("SSAO/Crytek", "SSAO/Crytek", "cDefaultAccessibility", slider->getValue(), false, 1);
        
        else if (slider->getName() == SSAO_UNSHARP_KERNEL_BIAS_NAME)
        {
            setUniform("SSAO/UnsharpMask", "SSAO/UnsharpMask/GaussianBlurY", "cKernelWidthBias", slider->getValue(), false, 1);
            setUniform("SSAO/UnsharpMask", "SSAO/UnsharpMask/GaussianBlurX", "cKernelWidthBias", slider->getValue(), false, 1);
        }
        
        else if (slider->getName() == SSAO_UNSHARP_LAMBDA_NAME)
            setUniform("SSAO/UnsharpMask", "SSAO/UnsharpMask", "cLambda", slider->getValue() * slider->getValue(), false, 1);
        
        else if (slider->getName() == SSAO_BILATERAL_PHOTOMETRIC_EXPONENT)
        {
            setUniform("SSAO/Post/CrossBilateralFilter", "SSAO/HorizonBased/CrossBilateralFilter/X", "cPhotometricExponent", slider->getValue(), false);
            setUniform("SSAO/Post/CrossBilateralFilter", "SSAO/HorizonBased/CrossBilateralFilter/Y", "cPhotometricExponent", slider->getValue(), false);
        }
        
        else if(slider->getName() == SSAO_SAMPLE_LENGTH_EXPONENT_NAME)
            setUniform("SSAO/HemisphereMC", "SSAO/HemisphereMC", "cSampleLengthExponent", slider->getValue(), false, 1);

        CompositorManager::getSingleton().setCompositorEnabled(mViewport, mCurrentCompositor, true);
        CompositorManager::getSingleton().setCompositorEnabled(mViewport, mCurrentPost, true);
    }

    void checkBoxToggled(OgreBites::CheckBox *box) 
    {
        if (box->getName() == SSAO_SAMPLE_SPACE_NAME)
        {
            setUniform("SSAO/Crytek", "SSAO/Crytek", "cSampleInScreenspace", box->isChecked(), false, 1);
            setUniform("SSAO/HorizonBased", "SSAO/HorizonBased", "cSampleInScreenspace", box->isChecked(), false, 1);
            setUniform("SSAO/HemisphereMC", "SSAO/HemisphereMC", "cSampleInScreenspace", box->isChecked(), false, 1);
            setUniform("SSAO/Volumetric", "SSAO/Volumetric", "cSampleInScreenspace", box->isChecked(), false, 1);
            CompositorManager::getSingleton().setCompositorEnabled(mViewport, mCurrentCompositor, true);
            
            if (box->isChecked()) // we sample in screen space 
            {
                mTrayMgr->removeWidgetFromTray(mWorldSpaceSlider);
                mWorldSpaceSlider->hide();
                mTrayMgr->moveWidgetToTray(mScreenSpaceSlider, TL_TOPLEFT);
                mScreenSpaceSlider->show();
            }
            else
            {
                mTrayMgr->removeWidgetFromTray(mScreenSpaceSlider);
                mScreenSpaceSlider->hide();
                mTrayMgr->moveWidgetToTray(mWorldSpaceSlider, TL_TOPLEFT);
                mWorldSpaceSlider->show();
            }
        }
    }

	// The following three methods are for mouse input
	/** @see Sample::mousePressed. */
#if (OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS) && (OGRE_PLATFORM != OGRE_PLATFORM_ANDROID)

    bool mousePressed( const OIS::MouseEvent& evt, OIS::MouseButtonID id )
    {
        if (mTrayMgr->injectMouseDown(evt, id)) 
            return true;
        if (id == OIS::MB_Left) 	
            mTrayMgr->hideCursor();  // hide the cursor if user left-clicks in the scene			
        
        return true;
    }

	/** @see Sample::mouseReleased. */
    bool mouseReleased( const OIS::MouseEvent& evt, OIS::MouseButtonID id )
    {
        if (mTrayMgr->injectMouseUp(evt, id)) 
            return true;
        if (id == OIS::MB_Left) 
            mTrayMgr->showCursor();  // unhide the cursor if user lets go of LMB
        
        return true;
    }

	/** @see Sample::mouseMoved. */
    bool mouseMoved( const OIS::MouseEvent& evt )
    {
        // only rotate the camera if cursor is hidden
        if (mTrayMgr->isCursorVisible())
            mTrayMgr->injectMouseMove(evt);
        else 
        {
            mCameraMan->injectMouseMove(evt);
            mCameraMenu->selectItem(SSAO_USER_CAMERA_ITEM);
        }
        
        return true;
    }
#endif
	/**
	 * Set the uniform value in the compositor
	 * @param compositor The name of the compositor
	 * @param material The material that contains the uniform
	 * @param uniform The name of the uniform parameter
	 * @param value The value
	 * @param setVisible Whether to set the compositor to visible or not.
	 * @param position The position at which the compositor should be added again.
	 * defaults to -1, which means that the compositor is readded at the end of the chain.
	 */
    void setUniform(Ogre::String compositor, Ogre::String material, Ogre::String uniform, float value, bool setVisible, int position = -1)
    {
        // remove compositor first???
        CompositorManager::getSingleton().removeCompositor(mViewport, compositor);
        
        (static_cast<MaterialPtr>(MaterialManager::getSingleton().getByName(material)))->getTechnique(0)->
            getPass(0)->getFragmentProgramParameters()->setNamedConstant(uniform, value);
        
        // adding again
        CompositorManager::getSingleton().addCompositor(mViewport, compositor, position);
        CompositorManager::getSingleton().setCompositorEnabled(mViewport, compositor, setVisible);
    }
};



#endif /* __SSAO_H__ */
