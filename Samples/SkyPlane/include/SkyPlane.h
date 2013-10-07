#ifndef __SkyPlane_H__
#define __SkyPlane_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_SkyPlane : public SdkSample
{
public:

	Sample_SkyPlane()
	{
		mInfo["Title"] = "Sky Plane";
		mInfo["Description"] = "Shows how to use skyplanes (fixed-distance planes used for backgrounds).";
		mInfo["Thumbnail"] = "thumb_skyplane.png";
		mInfo["Category"] = "Environment";
	}

protected:

	void setupContent()
	{
		const IdString workspaceName( "SkyPlane Workspace" );
		CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
		if( !compositorManager->hasWorkspaceDefinition( workspaceName ) )
			compositorManager->createBasicWorkspaceDef( workspaceName, ColourValue::Black );
		compositorManager->addWorkspace( mSceneMgr, mWindow, mCamera, workspaceName, true );

		// setup some basic lighting for our scene
        mSceneMgr->setAmbientLight(ColourValue(0.3, 0.3, 0.3));
        SceneNode *lightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		lightNode->setPosition(20, 80, 50);
		lightNode->attachObject( mSceneMgr->createLight() );
        
		// create a skyplane 5000 units away, facing down, 10000 square units large, with 3x texture tiling
        mSceneMgr->setSkyPlane(true, Plane(0, -1, 0, 5000), "Examples/SpaceSkyPlane", 10000, 3);

        // and finally... omg it's a DRAGON!
        mSceneMgr->getRootSceneNode()->attachObject(mSceneMgr->createEntity("dragon.mesh"));

		// turn around and look at the DRAGON!
		mCamera->yaw(Degree(210));
		mCamera->pitch(Degree(-10));
	}
};

#endif
