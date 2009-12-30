/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/

#ifndef _OceanDemo_H_
#define _OceanDemo_H_

#include "SdkSample.h"
#include "SamplePlugin.h"
#include "MaterialControls.h"

using namespace Ogre;
using namespace OgreBites;

#define MINSPEED .150f
#define MOVESPEED 30
#define MAXSPEED 1.800f

#define CONTROLS_PER_PAGE 5

enum OceanMaterial {
	OCEAN1_CG,
	OCEAN1_NATIVE,
	OCEAN2_CG,
	OCEAN2_NATIVE
};

class _OgreSampleClassExport OceanDemo : public SdkSample
{
public:
	OceanDemo();
protected:
	//Things from the frame listener
	Ogre::Vector3 mTranslateVector;
    int mSceneDetailIndex;
    float mUpdateFreq;
	bool mSpinLight;
    // just to stop toggles flipping too fast
    Ogre::TextureFilterOptions mFiltering;
    int mAniso;

	Ogre::SceneNode*	  mMainNode;
	Ogre::Entity*         mOceanSurfaceEnt;

	size_t				  mCurrentMaterial;
	size_t				  mCurrentPage;
	size_t				  mNumPages;
	Ogre::MaterialPtr	  mActiveMaterial;
	Ogre::Pass*			  mActivePass;
	Ogre::GpuProgramPtr	  mActiveFragmentProgram;
	Ogre::GpuProgramPtr	  mActiveVertexProgram;
	Ogre::GpuProgramParametersSharedPtr mActiveFragmentParameters;
	Ogre::GpuProgramParametersSharedPtr mActiveVertexParameters;
	Real				  mRotateSpeed;
	Slider* mShaderControls[CONTROLS_PER_PAGE];
	
	ShaderControlsContainer    mShaderControlContainer;
    MaterialControlsContainer mMaterialControlsContainer;
	
	void setupGUI();
	void setupScene();
	virtual void setupContent();
	virtual void cleanupContent();
    
	void sliderMoved(Slider* slider);
	void buttonHit(Button* button);
	void checkBoxToggled(CheckBox* box);
	void selectOceanMaterial(OceanMaterial newMaterial);
	void itemSelected(SelectMenu* menu);
	void changePage(size_t nextPage = -1);
	virtual bool frameRenderingQueued(const FrameEvent& evt);
	
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
	bool OceanDemo::touchPressed(const OIS::MultiTouchEvent& evt);
	bool OceanDemo::touchReleased(const OIS::MultiTouchEvent& evt);
	bool OceanDemo::touchMoved(const OIS::MultiTouchEvent& evt);
#else
	bool OceanDemo::mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
	bool OceanDemo::mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
	bool OceanDemo::mouseMoved(const OIS::MouseEvent& evt);
#endif

};


#endif	// end _OceanDemo_H_
