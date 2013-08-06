#ifndef __MeshLod_H__
#define __MeshLod_H__

#include "OgreLodConfigSerializer.h"

#include "SdkSample.h"

#include "OgreLodConfig.h"
#include "OgreDistanceLodStrategy.h"
#include "OgrePixelCountLodStrategy.h"

#include "OgreQueuedProgressiveMeshGenerator.h"
#include "OgreProgressiveMeshGenerator.h"

#include "OgreMeshSerializer.h"
#include "OgreMeshSerializerImpl.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_MeshLod :
	public SdkSample
{
public:

	Sample_MeshLod() :
		mHeadEntity(0)
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
	void cleanupControls(){
		mTrayMgr->clearTray(TL_TOPLEFT);
		mTrayMgr->clearTray(TL_TOPRIGHT);
		mTrayMgr->clearTray(TL_TOP);
	}
	void setupControls(int uimode = 0)
	{
		cleanupControls();

		OgreBites::SelectMenu* objectType = mTrayMgr->createLongSelectMenu(TL_TOPLEFT, "cmbModels", "Model:", 150, 8);
		objectType->addItem("sinbad.mesh");
		objectType->addItem("ogrehead.mesh");
		objectType->addItem("knot.mesh");
		objectType->addItem("fish.mesh");
		objectType->addItem("penguin.mesh");
		objectType->addItem("ninja.mesh");
		objectType->addItem("dragon.mesh");
		objectType->addItem("athene.mesh");
		objectType->addItem("sibenik.mesh");

		// Add all meshes from popular:
		Ogre::StringVectorPtr meshes = Ogre::ResourceGroupManager::getSingleton().findResourceNames("Popular", "*.mesh");
		Ogre::StringVector::iterator it, itEnd;
		it = meshes->begin();
		itEnd = meshes->end();
		for(; it != itEnd; it++){
			objectType->addItem(*it);
		}

		/*
		Basic options
		-cmbModel
		-sldReductionValue
		-chkShowWireframe
		-chkAutoconfigure
		*/
		mWireframe = mTrayMgr->createCheckBox(TL_TOPLEFT, "chkShowWireframe", "Show wireframe", 200);
		mUseVertexNormals = mTrayMgr->createCheckBox(TL_TOPLEFT, "chkUseVertexNormals", "Use vertex normals", 200);
		mReductionSlider = mTrayMgr->createThickSlider(TL_TOPLEFT, "sldReductionValue", "Reduced vertices", 200, 50, 0, 100, 101);
		mTrayMgr->createButton(TL_TOPLEFT, "btnReduceMore","Reduce More");
		mTrayMgr->createButton(TL_TOPLEFT, "btnReduceLess","Reduce Less");

		/*
		Level options
		-cmbLodLevels
		-btnAddLodLevel
		-btnRemoveSelectedLodLevel
		-chkForceSelectedLodLevel
		*/
		mDistanceLabel = mTrayMgr->createLabel(TL_TOPRIGHT, "lblDistance", "Distance: ", 250);
		mLodLevelList = mTrayMgr->createLongSelectMenu(TL_TOPRIGHT, "cmbLodLevels", "Lod level:", 150, 4);
		mTrayMgr->createButton(TL_TOPRIGHT, "btnRemoveSelectedLodLevel","Remove level", 200);
		mTrayMgr->createButton(TL_TOPRIGHT, "btnAddLodLevel","Add level", 200);
		

		/*
		Serializer options:
		-btnSaveMesh
		-btnShowLodFromMesh
		*/
		mTrayMgr->createButton(TL_TOPRIGHT, "btnSaveMesh", "Save mesh", 200);
		mTrayMgr->createButton(TL_TOPRIGHT, "btnShowMesh", "Show Lod from mesh", 200);
		mTrayMgr->createButton(TL_TOPRIGHT, "btnAutoconfigure", "Show autoconfigured mesh", 200);
		/*
		Profile options
		-cmbProfiledVertices
		-btnKeepVertex
		-btnRemoveSelectedVertex
		*/
		mProfileList = mTrayMgr->createLongSelectMenu(TL_TOPRIGHT, "cmbProfiledVertices", "Profile:", 180, 4);
		mTrayMgr->createButton(TL_TOPRIGHT, "btnRemoveFromProfile","Remove from profile", 200.0);
		mTrayMgr->createButton(TL_TOPRIGHT, "btnAddToProfile","Add to profile", 200.0);

		//mTrayMgr->createTextBox(TL_TOPRIGHT, "Help","Help", 200, 200)
		//	->setText("The last reduced vertex is the selected vertex. Use the slider to select the vertex, then decide to keep or remove it. You can export the Lod buffers into the .mesh file after configuration.");

		mTrayMgr->showCursor();

	}

	void changeSelectedMesh(const Ogre::String& name)
	{
		if(mHeadEntity){
			mSceneMgr->destroyEntity(mHeadEntity);
			mHeadEntity = 0;
			saveConfig();
		}
		mLodConfig.mesh = Ogre::MeshManager::getSingleton().load(name, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		if(mLodConfig.mesh->getBounds().isNull() || mLodConfig.mesh->getBoundingSphereRadius() == 0.0){
			mTrayMgr->showOkDialog("Error", "Failed to load mesh!");
			return;
		}
		mHeadEntity = mSceneMgr->createEntity(name, mLodConfig.mesh);
		mHeadNode->attachObject(mHeadEntity);
		mCamera->setPosition(Ogre::Vector3(0, 0, 0));
		mCamera->moveRelative(Ogre::Vector3(0, 0, mLodConfig.mesh->getBoundingSphereRadius() * 2));
		mCamera->setNearClipDistance(mLodConfig.mesh->getBoundingSphereRadius() / 16);
		mCamera->setFarClipDistance(mLodConfig.mesh->getBoundingSphereRadius() * 256);

		size_t vertexCount = getUniqueVertexCount(mLodConfig.mesh);
		mReductionSlider->setRange(0,vertexCount,vertexCount+1,false);
		mLodLevelList->clearItems();
		mWorkLevel.distance = std::numeric_limits<Ogre::Real>::max();
		mWorkLevel.reductionMethod = LodLevel::VRM_CONSTANT;
		mWorkLevel.reductionValue = 0.0;

		loadConfig();

		if(mLodLevelList->getNumItems() > 0){
			loadLodLevel(mLodLevelList->getSelectionIndex());
		}

	}

	size_t getUniqueVertexCount(Ogre::MeshPtr mesh)
	{
		// The vertex buffer contains the same vertex position multiple times.
		// To get the count of the vertices, which has unique positions, we can use progressive mesh.
		// It is constructing a mesh grid at the beginning, so if we reduce 0%, we will get the unique vertex count.
		Ogre::LodConfig lodConfig;
		lodConfig.mesh = mesh;
		lodConfig.strategy = DistanceLodStrategy::getSingletonPtr();
		LodLevel lodLevel;
		lodLevel.distance = std::numeric_limits<Real>::max();
		lodLevel.reductionMethod = LodLevel::VRM_PROPORTIONAL;
		lodLevel.reductionValue = 0.0;
		lodConfig.levels.push_back(lodLevel);
		ProgressiveMeshGenerator pm;
		pm.generateLodLevels(lodConfig);
		return lodConfig.levels[0].outUniqueVertexCount;
	}

	void cleanupContent()
	{
		if(mHeadEntity){
			mSceneMgr->destroyEntity(mHeadEntity);
			mHeadEntity = 0;
			saveConfig();
		}
	}
	
	void buttonHit(Button* button) {
		if(button->getName() == "btnReduceMore") {
			mReductionSlider->setValue(mReductionSlider->getValue()+1);
		} else if(button->getName() == "btnReduceLess") {
			mReductionSlider->setValue(mReductionSlider->getValue()-1);
		} else if(button->getName() == "btnAddToProfile") {
			addToProfile(std::numeric_limits<Real>::max());
		} else if(button->getName() == "btnRemoveFromProfile") {
			if(hasProfileData()){
				LodProfile& profile = mLodConfig.advanced.profile;
				profile.erase(profile.begin() + mProfileList->getSelectionIndex());
				mProfileList->removeItem(mProfileList->getSelectionIndex());
			}
		} else if(button->getName() == "btnRemoveSelectedLodLevel") {
			removeLodLevel();
		} else if(button->getName() == "btnAddLodLevel") {
			addLodLevel();
		} else if(button->getName() == "btnAutoconfigure") {
			mTrayMgr->destroyAllWidgetsInTray(TL_TOP);
			mTrayMgr->createLabel(TL_TOP, "lblWhatYouSee", "Showing autoconfigured LOD", 300);
			loadAutomaticLod();
			forceLodLevel(-1); // disable Lod level forcing
		} else if(button->getName() == "btnShowMesh") {
			mTrayMgr->destroyAllWidgetsInTray(TL_TOP);
			mTrayMgr->createLabel(TL_TOP, "lblWhatYouSee", "Showing LOD from mesh file", 300);
			if(mHeadEntity){
				mSceneMgr->destroyEntity(mHeadEntity);
				mHeadEntity = 0;
			}
			mLodConfig.mesh->reload();
			mHeadEntity = mSceneMgr->createEntity(mLodConfig.mesh->getName(), mLodConfig.mesh);
			mHeadNode->attachObject(mHeadEntity);
			forceLodLevel(-1); // disable Lod level forcing
		} else if(button->getName() == "btnSaveMesh") {
			if(!mTrayMgr->getTrayContainer(TL_TOP)->isVisible()){
				ProgressiveMeshGenerator pm; // Non-threaded
				pm.generateLodLevels(mLodConfig);
				forceLodLevel(-1); // disable
			}
			Ogre::ResourceGroupManager& resourceGroupMgr = Ogre::ResourceGroupManager::getSingleton();
			Ogre::String group = mLodConfig.mesh->getGroup();
			// If we don't add * to the name, the pattern matcher will not find it.
			Ogre::String name = "*" + mLodConfig.mesh->getName();
			Ogre::FileInfoListPtr locPtr = resourceGroupMgr.findResourceFileInfo(group, name);
			if(!locPtr->empty()){
				Ogre::FileInfo& info = locPtr->at(0);
				if(info.archive->getType() != "FileSystem") {
					// Only FileSystem archive type is supported.
					return;
				}
				Ogre::String filename = info.archive->getName();
				if(filename.back() != '/' && filename.back() != '\\')
					filename += '/';
				filename += info.path;
				if(filename.back() != '/' && filename.back() != '\\')
					filename += '/';
				filename += info.filename;

				Ogre::MeshSerializer ms;
				ms.exportMesh(mLodConfig.mesh.get(), filename);
			}
			if(!mTrayMgr->getTrayContainer(TL_TOP)->isVisible()){
				updateLod(true);
			}
		}
	}

	void addLodLevel() {
		
		LodLevel lvl;
		lvl.distance = getCameraDistance();
		lvl.reductionMethod = LodLevel::VRM_CONSTANT;
		lvl.reductionValue = mReductionSlider->getValue();
		Real distepsilon = lvl.distance + lvl.distance * 0.001;
		size_t i = 0;
		bool addLevel = true;
		for(; i < mLodConfig.levels.size(); i++){
			if(mLodConfig.levels[i].distance < distepsilon){
				addLevel = false;
				break;
			}
		}
		if(/*mLodConfig.levels.empty() || */addLevel){
			mLodConfig.levels.push_back(lvl);
			mLodLevelList->addItem(Ogre::StringConverter::toString(lvl.distance) + "px");
			mLodLevelList->selectItem(mLodLevelList->getNumItems() - 1, false);
		} else {
			mLodConfig.levels.insert(mLodConfig.levels.begin() + i, lvl);
			mLodLevelList->insertItem(i, Ogre::StringConverter::toString(lvl.distance) + "px");
			mLodLevelList->selectItem(i, false);
		}
	}
	void loadLodLevel(int id) {
		assert(mLodConfig.levels[id].reductionMethod == LodLevel::VRM_CONSTANT);
		mWorkLevel = mLodConfig.levels[id];
		mReductionSlider->setValue(mWorkLevel.reductionValue, false);
		mLodLevelList->selectItem(id, false);
		moveCameraToPixelDistance(mWorkLevel.distance);
		updateLod(true);
	}
	void moveCameraToPixelDistance(Ogre::Real pixels){
		Ogre::PixelCountLodStrategy& strategy = PixelCountLodStrategy::getSingleton();
		Ogre::Real distance = mLodConfig.mesh->getBoundingSphereRadius() * 4;
		const Ogre::Real epsilon = pixels * 0.000001;
		const int iterations = 64;
		mCamera->setPosition(Ogre::Vector3(0, 0, 0));
		mCamera->moveRelative(Ogre::Vector3(0, 0, distance));
		// We need to find a distance, which is bigger then requested
		for(int i=0;i<iterations;i++){
			Ogre::Real curPixels = strategy.getValue(mHeadEntity, mCameraMan->getCamera());
			if (curPixels > pixels) {
				distance *= 2.0;
				mCamera->moveRelative(Ogre::Vector3(0, 0, distance));
			} else {
				break;
			}
		}
		// Binary search for distance
		for(int i=0;i<iterations;i++){
			Ogre::Real curPixels = strategy.getValue(mHeadEntity, mCameraMan->getCamera());
			if(std::abs(curPixels - pixels) < epsilon){
				break;
			}
			distance /= 2;
			if (curPixels > pixels) {
				// move camera further
				mCamera->moveRelative(Ogre::Vector3(0, 0, distance));
			} else {
				// move camera nearer
				mCamera->moveRelative(Ogre::Vector3(0, 0, -distance));
			}
		}
	}
	void removeLodLevel() {
		if(mLodConfig.levels.empty()){
			return;
		}
		int selectedLevel = mLodLevelList->getSelectionIndex();
		mLodConfig.levels.erase(mLodConfig.levels.begin() + selectedLevel);
		mLodLevelList->removeItem(selectedLevel);
	}

	Real getCameraDistance(){
		if(mLodConfig.mesh->getBoundingSphereRadius() != 0.0){
			return PixelCountLodStrategy::getSingleton().getValue(mHeadEntity, mCameraMan->getCamera());
		} else {
			return 0.0;
		}


	}
	
	bool hasProfileData(){
		return !mLodConfig.advanced.profile.empty();
	}
	
	Ogre::String getResourceFullPath(MeshPtr& mesh){
		Ogre::ResourceGroupManager& resourceGroupMgr = Ogre::ResourceGroupManager::getSingleton();
		Ogre::String group = mesh->getGroup();
		// If we don't add * to the name, the pattern matcher will not find it.
		Ogre::String name = "*" + mLodConfig.mesh->getName();
		Ogre::FileInfoListPtr locPtr = resourceGroupMgr.findResourceFileInfo(group, name);
		assert(locPtr->size() == 1);
		
		Ogre::FileInfo& info = locPtr->at(0);
		if(info.archive->getType() != "FileSystem") {
			mTrayMgr->showOkDialog("Operation canceled", "Only FileSystem archive type can be altered.");
			return "";
		}

		Ogre::String filename = info.archive->getName();
		if(filename.back() != '/' && filename.back() != '\\')
			filename += '/';
		filename += info.path;
		if(filename.back() != '/' && filename.back() != '\\')
			filename += '/';
		filename += info.filename;

		return filename;
	}
	
	void sliderMoved(Slider* slider)
	{
		if (slider->getName() == "sldReductionValue") {
			mWorkLevel.reductionValue = slider->getValue();
			updateLod(true);
		}
	}
	
	void itemSelected(SelectMenu* menu)
	{
		if (menu->getName() == "cmbModels") {
			changeSelectedMesh(menu->getSelectedItem());
		} else if(menu->getName() == "cmbLodLevels") {
			loadLodLevel(menu->getSelectionIndex());
		}
	}

	void checkBoxToggled(OgreBites::CheckBox * box)
	{
		if(box->getName() == "chkUseVertexNormals") {
			mLodConfig.advanced.useVertexNormals = box->isChecked();
			updateLod(true);
		} else if (box->getName() == "chkShowWireframe") {
			mCameraMan->getCamera()->setPolygonMode(mWireframe->isChecked() ? PM_WIREFRAME : PM_SOLID);
		}
	}

	void updateLod(bool useWorkLod)
	{
		if(mLodConfig.mesh->getBoundingSphereRadius() == 0){
			// failed to load mesh
			return;
		}
		if(mLodConfig.levels.empty()){
			useWorkLod = true;
		}
		mTrayMgr->destroyAllWidgetsInTray(TL_TOP);
		loadUserLod(useWorkLod);
	}

	// Disable Lod distance by forcing given Lod Level.
	void forceLodLevel(int index){
		if(index == -1) {
			// Clear forced Lod level
			mHeadEntity->setMeshLodBias(1.0, 0, std::numeric_limits<unsigned short>::max());
		} else {
			mHeadEntity->setMeshLodBias(1.0, index, index);
		}
	}
	
	void loadUserLod(bool useWorkLod)
	{
		// Remove outdated Lod requests to reduce delay.
		PMWorker::getSingleton().clearPendingLodRequests();
		QueuedProgressiveMeshGenerator pm; // Threaded
		//ProgressiveMeshGenerator pm; // Non-threaded
		if(!useWorkLod){
			pm.generateLodLevels(mLodConfig);
			forceLodLevel(-1); // disable
		} else {
			Ogre::LodConfig config(mLodConfig);
			config.levels.clear();
			config.levels.push_back(mWorkLevel);
			pm.generateLodLevels(config);
			if(config.mesh->getNumLodLevels() > 1){
				forceLodLevel(1); // Force Lod1
			} else { // We don't have any Lod levels.
				forceLodLevel(-1); // disable
			}
		}
		
	}

	// This produces acceptable output on any kind of mesh.
	void loadAutomaticLod()
	{
		// Remove outdated Lod requests to reduce delay.
		PMWorker::getSingleton().clearPendingLodRequests();
		QueuedProgressiveMeshGenerator pm;
		pm.generateAutoconfiguredLodLevels(mLodConfig.mesh);
	}

	void addToProfile(Real cost){
		Ogre::LodConfig config(mLodConfig);
		config.levels.clear();
		config.levels.push_back(mWorkLevel);
		ProgressiveMeshGenerator pm;
		pm.generateLodLevels(config);

		ProfiledEdge pv;
		if(pm._getLastVertexPos(pv.src)){
			pm._getLastVertexCollapseTo(pv.dst);
			// Prevent duplicates if you edit the same vertex twice.
			int size = mLodConfig.advanced.profile.size();
			for(int i=0;i<size;i++){
				ProfiledEdge& v = mLodConfig.advanced.profile[i];
				if(v.src == pv.src && v.dst == pv.src){
					v.cost = cost;
					updateLod(true);
					mProfileList->selectItem(i, false);
					return;
				}
			}
			// Copy profile in queued build.
			pv.cost = cost;
			mLodConfig.advanced.profile.push_back(pv);
			updateLod(true);
			mProfileList->addItem(Ogre::StringConverter::toString(pv.src));
			mProfileList->selectItem(mProfileList->getNumItems() - 1, false);
		} else {
			mTrayMgr->showOkDialog("Error", "No vertex selected, because the mesh is not reduced.");
		}
	}

	void saveConfig(){

		std::string filename(mLodConfig.mesh->getName());
		filename += ".lodconfig";
		Ogre::LodConfigSerializer lcs;
		lcs.exportLodConfig(mLodConfig, filename);
	}

	bool frameStarted(const Ogre::FrameEvent& evt){
		mDistanceLabel->setCaption("Distance: " + Ogre::StringConverter::toString(getCameraDistance()) + "px");
		return true;
	}

	bool loadConfig(){
		mLodConfig.advanced = Ogre::LodConfig::Advanced();
		mLodConfig.strategy = Ogre::PixelCountLodStrategy::getSingletonPtr();
		mLodConfig.levels.clear();
		mLodConfig.advanced.profile.clear();

		// The mesh should already be set.
		assert(mLodConfig.mesh.get());

		std::string filename(mLodConfig.mesh->getName());
		filename += ".lodconfig";
		Ogre::LodConfigSerializer lcs;
		lcs.importLodConfig(&mLodConfig, filename);

		mLodLevelList->clearItems();
		for(size_t i = 0; i < mLodConfig.levels.size(); i++){
			mLodLevelList->addItem(Ogre::StringConverter::toString(mLodConfig.levels[i].distance) + "px");
		}
		
		mProfileList->clearItems();
		for(size_t i = 0; i < mLodConfig.advanced.profile.size(); i++){
			mProfileList->addItem(Ogre::StringConverter::toString(mLodConfig.advanced.profile[i].src));
		}

		mUseVertexNormals->setChecked(mLodConfig.advanced.useVertexNormals, false);
		return true;
	}

	LodLevel mWorkLevel; // Current Lod Level, which we are seeing.
	LodConfig mLodConfig; // Current LodConfig, which we are editing.
	Entity* mHeadEntity;
	SceneNode* mHeadNode;
	OgreBites::CheckBox* mUseVertexNormals;
	OgreBites::CheckBox* mWireframe;
	OgreBites::SelectMenu* mProfileList;
	OgreBites::SelectMenu* mLodLevelList;
	OgreBites::Slider* mReductionSlider;
	OgreBites::Label* mDistanceLabel;
};

#endif
