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
#ifndef __Paging_H__
#define __Paging_H__

#include "SdkSample.h"
#include "OgrePaging.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_Paging : public SdkSample, public PageProvider
{
public:

	Sample_Paging()
		: mPageManager(0)
	{
		mInfo["Title"] = "Paging";
		mInfo["Description"] = "Demonstrates use of the paging plugin.";
		mInfo["Thumbnail"] = "thumb_terrain.png";
		mInfo["Category"] = "Unsorted";
		mInfo["Help"] = "Left click and drag anywhere in the scene to look around. Let go again to show "
			"cursor and access widgets. Use WASD keys to move. Use +/- keys when in edit mode to change content.";


	}

	~Sample_Paging()
	{
	}

	void testCapabilities(const RenderSystemCapabilities* caps)
	{
		if (!caps->hasCapability(RSC_VERTEX_PROGRAM) || !caps->hasCapability(RSC_FRAGMENT_PROGRAM))
		{
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your graphics card does not support vertex or fragment shaders, "
				"so you cannot run this sample. Sorry!", "TerrainSample::testCapabilities");
		}
	}

	StringVector getRequiredPlugins()
	{
		StringVector names;
		return names;
	}

	bool frameRenderingQueued(const FrameEvent& evt)
	{

		return SdkSample::frameRenderingQueued(evt);  // don't forget the parent updates!
	}

protected:

	PageManager* mPageManager;


	/*-----------------------------------------------------------------------------
	| Extends setupView to change some initial camera settings for this sample.
	-----------------------------------------------------------------------------*/
	void setupView()
	{
		SdkSample::setupView();

		mCamera->setPosition(Vector3(100,50,100));
		mCamera->lookAt(Vector3::ZERO);
		mCamera->setNearClipDistance(5);
		mCamera->setFarClipDistance(50000);

		if (mRoot->getRenderSystem()->getCapabilities()->hasCapability(RSC_INFINITE_FAR_PLANE))
		{
			mCamera->setFarClipDistance(0);   // enable infinite far clip distance if we can
		}
	}

	void setupControls()
	{
		mTrayMgr->showCursor();

		// make room for the controls
		mTrayMgr->showLogo(TL_TOPRIGHT);
		mTrayMgr->showFrameStats(TL_TOPRIGHT);
		mTrayMgr->toggleAdvancedFrameStats();

		// a friendly reminder
		StringVector names;
		names.push_back("Help");
		mTrayMgr->createParamsPanel(TL_TOPLEFT, "Help", 100, names)->setParamValue(0, "H/F1");
	}

	void setupContent()
	{
		LogManager::getSingleton().setLogDetail(LL_BOREME);

		mPageManager = OGRE_NEW PageManager();
		PagedWorld* world = mPageManager->createWorld();
		PagedWorldSection* sec = world->createSection("Grid2D", mSceneMgr);

		Grid2DPageStrategyData* data = static_cast<Grid2DPageStrategyData*>(sec->getStrategyData());

		// accept defaults for now

		mPageManager->setDebugDisplayLevel(1);

		// hook up self to provide pages procedurally
		mPageManager->setPageProvider(this);

		mCamera->setPosition(0, 100, 0);

		mPageManager->addCamera(mCamera);

		setupControls();

		setDragLook(true);

	}
	// callback on PageProvider
	bool prepareProceduralPage(Page* page, PagedWorldSection* section)
	{
		// say we populated something just so it doesn't try to load any more
		return true;
	}

	bool loadProceduralPage(Page* page, PagedWorldSection* section)
	{
		// say we populated something just so it doesn't try to load any more
		return true;
	}

	void _shutdown()
	{
		OGRE_DELETE mPageManager;
		mPageManager = 0;

		SdkSample::_shutdown();
	}

};

#endif
