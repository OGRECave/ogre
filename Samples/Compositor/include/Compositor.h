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

#ifndef _CompositorDemo_H_
#define _CompositorDemo_H_

#include "OgreConfigFile.h"
#include "OgreStringConverter.h"
#include "OgreException.h"

#include "SdkSample.h"
#include "SamplePlugin.h"

using namespace Ogre;
using namespace OgreBites;

#define COMPOSITORS_PER_PAGE 10

class _OgreSampleClassExport Sample_Compositor : public SdkSample
{
public:
	Sample_Compositor();

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
	bool touchPressed(const OIS::MultiTouchEvent& evt);
	bool touchReleased(const OIS::MultiTouchEvent& evt);
	bool touchMoved(const OIS::MultiTouchEvent& evt);
#else
	bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
	bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
	bool mouseMoved(const OIS::MouseEvent& evt);
#endif

    void setupContent(void);
    void cleanupContent(void);

	bool frameRenderingQueued(const FrameEvent& evt);
	
	void checkBoxToggled(OgreBites::CheckBox * box);
	void buttonHit(OgreBites::Button* button);        
	void itemSelected(OgreBites::SelectMenu* menu);

protected:
	
	void createCamera(void);
	void createControls(void);
    void createScene(void);
    void createEffects(void);
	void createTextures(void);

	void registerCompositors();
	void changePage(size_t pageNum);
	
	SceneNode * mSpinny;
	StringVector mCompositorNames;
	size_t mActiveCompositorPage;
	size_t mNumCompositorPages;	

	String mDebugCompositorName;
	SelectMenu* mDebugTextureSelectMenu;
	TextureUnitState* mDebugTextureTUS;

};


#endif	// end _CompositorDemo_H_
