#ifndef __MeshLod_H__
#define __MeshLod_H__

#include "SdkSample.h"

#include "OgreLodConfig.h"
#include "OgreDistanceLodStrategy.h"
#include "OgrePixelCountLodStrategy.h"

#include "OgreQueuedProgressiveMeshGenerator.h"
#include "OgreProgressiveMeshGenerator.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_MeshLod :
	public SdkSample
{
public:

	Sample_MeshLod() :
		mHeadEntity(0), mUserReductionValue(0.5)
	{
		mInfo["Title"] = "Mesh Lod";
		mInfo["Description"] = "Shows how to add Lod levels to a mesh using the ProgressiveMesh class.";
		mInfo["Thumbnail"] = "thumb_meshlod.png";
		mInfo["Category"] = "Unsorted";
	}
protected:

	void setupContent()
	{

		mCameraMan->setStyle(CS_ORBIT);

        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));  // set ambient light

        // make the scene's main light come from above
        Light* l = mSceneMgr->createLight();
        l->setType(Light::LT_DIRECTIONAL);
        l->setDirection(Vector3::NEGATIVE_UNIT_Y);

		// create a node for the model
		mHeadNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

		// setup gui
		setupControls();

		// load mesh
		changeSelectedMesh("sinbad.mesh");
	}

	void setupControls()
	{
		
		SelectMenu* objectType = mTrayMgr->createThickSelectMenu(TL_TOPLEFT, "ObjectType", "Object:", 200, 4);
		objectType->addItem("sinbad.mesh");
		objectType->addItem("ogrehead.mesh");
		objectType->addItem("knot.mesh");
		objectType->addItem("fish.mesh");
		objectType->addItem("penguin.mesh");
		objectType->addItem("ninja.mesh");
		objectType->addItem("dragon.mesh");
		objectType->addItem("athene.mesh");
		objectType->addItem("sibenik.mesh");

		/*
		// Add all meshes from popular:
		Ogre::StringVectorPtr meshes = Ogre::ResourceGroupManager::getSingleton().findResourceNames("Popular", "*.mesh");
		for(auto& meshName : *meshes){
			objectType->addItem(meshName);
		}
		*/

		mWireframe = mTrayMgr->createCheckBox(TL_TOPLEFT, "wireframe", "Show wireframe");
		mAutoconfig = mTrayMgr->createCheckBox(TL_TOPLEFT, "autoconfig", "Autoconfigure");
		mThreaded = mTrayMgr->createCheckBox(TL_TOPLEFT, "threaded", "Background thread");
		mThreaded->setChecked(true, false);
		OgreBites::Slider* slider = mTrayMgr->createThickSlider(TL_TOPLEFT, "ReductionSlider", "Percentage", 200, 50, 0, 100, 101);
		slider->setValue(50, false); // 50%
		mUserReductionValue = 0.5; // 50%

		mTrayMgr->showCursor();
	}

	void changeSelectedMesh(const Ogre::String& name)
	{	
		if(mHeadEntity){
			mSceneMgr->destroyEntity(mHeadEntity);
			mHeadEntity = 0;
		}
		mHeadMesh = Ogre::MeshManager::getSingleton().load(name, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		mHeadEntity = mSceneMgr->createEntity(name, mHeadMesh);
		mHeadNode->attachObject(mHeadEntity);
		mCamera->setPosition(Ogre::Vector3(0, 0, 0));
		mCamera->moveRelative(Ogre::Vector3(0, 0, mHeadMesh->getBoundingSphereRadius() * 2));
		mCamera->setNearClipDistance(mHeadMesh->getBoundingSphereRadius() / 16);
		mCamera->setFarClipDistance(mHeadMesh->getBoundingSphereRadius() * 256);

		updateLod();
	}
	void cleanupContent()
	{
		if(mHeadEntity){
			mSceneMgr->destroyEntity(mHeadEntity);
			mHeadEntity = 0;
		}
	}
	
	void sliderMoved(Slider* slider)
	{
		if (slider->getName() == "ReductionSlider") {
			mUserReductionValue = slider->getValue() / 100.0f;
			mAutoconfig->setChecked(false, false);
			updateLod();
		}
	}

	void itemSelected(SelectMenu* menu)
	{
		if (menu->getName() == "ObjectType") {
			changeSelectedMesh(menu->getSelectedItem());
		}
	}
	void checkBoxToggled(OgreBites::CheckBox * box)
	{
		updateLod();
	}

	void updateLod()
	{
		if (mAutoconfig->isChecked()) {
			loadAutomaticLod(mHeadMesh);
			forceLodLevel(-1); // disable
		} else {
			loadUserLod(mHeadMesh, mUserReductionValue);
			if(mHeadMesh->getNumLodLevels() > 1){
				forceLodLevel(1); // Force Lod1
			} else { // We don't have any Lod levels.
				forceLodLevel(-1); // disable
			}
		}
		if (mWireframe->isChecked()) {
			mCameraMan->getCamera()->setPolygonMode(PM_WIREFRAME);
		} else {
			mCameraMan->getCamera()->setPolygonMode(PM_SOLID);
		}
	}
	void forceLodLevel(int index){
		if(index == -1) {
			// Clear forced Lod level
			mHeadEntity->setMeshLodBias(1.0, 0, std::numeric_limits<unsigned short>::max());
		} else {
			mHeadEntity->setMeshLodBias(1.0, index, index);
		}
	}
	
	void loadUserLod(Ogre::MeshPtr& mesh, Real reductionValue)
	{
		Ogre::LodConfig lodConfig;
		lodConfig.levels.clear();
		lodConfig.mesh = mesh;
		lodConfig.strategy = DistanceLodSphereStrategy::getSingletonPtr();
		LodLevel lodLevel;
		lodLevel.reductionMethod = LodLevel::VRM_PROPORTIONAL;
		lodLevel.distance = 1;
		lodLevel.reductionValue = reductionValue;
		lodConfig.levels.push_back(lodLevel);
		if(mThreaded->isChecked()){
			clearLodQueue();
			QueuedProgressiveMeshGenerator pm;
			pm.generateLodLevels(lodConfig);
		} else {
			ProgressiveMeshGenerator pm;
			pm.generateLodLevels(lodConfig);
		}
	}

	// This produces acceptable output on any kind of mesh.
	void loadAutomaticLod(Ogre::MeshPtr& mesh)
	{
		if(mThreaded->isChecked()){
			clearLodQueue();
			QueuedProgressiveMeshGenerator pm;
			pm.generateAutoconfiguredLodLevels(mesh);
		} else {
			ProgressiveMeshGenerator pm;
			pm.generateAutoconfiguredLodLevels(mesh);
		}
	}

	void clearLodQueue(){
		// Remove outdated Lod requests to reduce delay.
		PMWorker::getSingleton().clearPendingLodRequests();
	}

	MeshPtr mHeadMesh;
	Entity* mHeadEntity;
	SceneNode* mHeadNode;
	Real mUserReductionValue;
	OgreBites::CheckBox* mWireframe;
	OgreBites::CheckBox* mAutoconfig;
	OgreBites::CheckBox* mThreaded;
};

#endif
