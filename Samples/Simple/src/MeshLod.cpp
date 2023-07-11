#include "OgreComponents.h"
#ifdef OGRE_BUILD_COMPONENT_MESHLODGENERATOR
#include "MeshLod.h"

using namespace Ogre;
using namespace OgreBites;

#include "OgreLodConfigSerializer.h"
#include "OgreMeshLodGenerator.h"
#include "OgreLodCollapseCostQuadric.h"
#include "OgreLodOutsideMarker.h"
#include "OgreLodData.h"
#include "OgreLod0Stripifier.h"

#include "OgrePixelCountLodStrategy.h"
#include "OgreMeshSerializer.h"

Sample_MeshLod::Sample_MeshLod()
{
    mInfo["Title"] = "Mesh Lod";
    mInfo["Description"] = "Shows how to add Lod levels to a mesh using the ProgressiveMesh class.";
    mInfo["Thumbnail"] = "thumb_meshlod.png";
    mInfo["Category"] = "Unsorted";
}

void Sample_MeshLod::setupContent()
{
    mCameraMan->setStyle(CS_ORBIT);

    mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));  // set ambient light

    // make the scene's main light come from above
    Light* l = mSceneMgr->createLight();
    l->setType(Light::LT_DIRECTIONAL);
    SceneNode* ln = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    ln->setDirection(Vector3::NEGATIVE_UNIT_Y);
    ln->attachObject(l);

    // create a node for the model
    mMeshNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    mMeshEntity = NULL;
#if SHOW_MESH_HULL
    mHullNode = mMeshNode->createChildSceneNode();
    mHullNode->scale(1.001,1.001,1.001);
    mHullEntity = NULL;
#endif
    if(!MeshLodGenerator::getSingletonPtr()) {
        new MeshLodGenerator();
    }
    MeshLodGenerator::getSingleton().setInjectorListener(this);

    // setup gui
    setupControls();

    // load mesh
    changeSelectedMesh("Sinbad.mesh");
}

void Sample_MeshLod::cleanupContent()
{
    MeshLodGenerator::getSingleton().removeInjectorListener();
    if(mMeshEntity){
        mSceneMgr->destroyEntity(mMeshEntity);
        mMeshEntity = 0;
    }
    cleanupControls();
}

void Sample_MeshLod::setupControls( int uimode /*= 0*/ )
{
    cleanupControls();

    SelectMenu* models = mTrayMgr->createLongSelectMenu(TL_TOPLEFT, "cmbModels", "Model:", 150, 8);
    models->addItem("Sinbad.mesh");
    models->addItem("ogrehead.mesh");
    models->addItem("knot.mesh");
    models->addItem("fish.mesh");
    models->addItem("penguin.mesh");
    models->addItem("ninja.mesh");
    models->addItem("dragon.mesh");
    models->addItem("athene.mesh");
    models->addItem("sibenik.mesh");

    // Add all meshes from popular:
    StringVectorPtr meshes = ResourceGroupManager::getSingleton().findResourceNames("General", "*.mesh");
    StringVector::iterator it, itEnd;
    it = meshes->begin();
    itEnd = meshes->end();
    for(; it != itEnd; it++){
        models->addItem(*it);
    }


    // Basic options:
    mWireframe = mTrayMgr->createCheckBox(TL_TOPLEFT, "chkShowWireframe", "Show wireframe", 200);
    mUseVertexNormals = mTrayMgr->createCheckBox(TL_TOPLEFT, "chkUseVertexNormals", "Use vertex normals", 200);
    mOutsideWeightSlider = mTrayMgr->createThickSlider(TL_TOPLEFT, "sldOutsideWeight", "Weighten outside", 200, 50, 0, 100, 101);
    mOutsideWalkAngle = mTrayMgr->createThickSlider(TL_TOPLEFT, "sldOutsideWalkAngle", "Outside angle", 200, 50, -1, 1, 201);
    mManualMeshes = mTrayMgr->createLongSelectMenu(TL_TOPLEFT, "cmbManualMesh", "Manual LOD:", 100, 8);
    mManualMeshes->copyItemsFrom(models);
    mManualMeshes->insertItem(0,"");
    mReductionSlider = mTrayMgr->createThickSlider(TL_TOPLEFT, "sldReductionValue", "Reduced vertices", 200, 50, 0, 100, 101);
    mTrayMgr->createButton(TL_TOPLEFT, "btnReduceMore","Reduce More");
    mTrayMgr->createButton(TL_TOPLEFT, "btnReduceLess","Reduce Less");

    // Level options:
    mDistanceLabel = mTrayMgr->createLabel(TL_TOPRIGHT, "lblDistance", "Distance: ", 250);
    mLodLevelList = mTrayMgr->createLongSelectMenu(TL_TOPRIGHT, "cmbLodLevels", "Lod level:", 150, 4);
    mTrayMgr->createButton(TL_TOPRIGHT, "btnAddLodLevel","Add level", 220);
    mTrayMgr->createButton(TL_TOPRIGHT, "btnRemoveSelectedLodLevel","Remove level", 220);
    mTrayMgr->createButton(TL_TOPRIGHT, "btnRemoveInitialLodLevel","Remove level #0", 220);

    // Serializer options:
    mTrayMgr->createButton(TL_TOPRIGHT, "btnShowAll", "Show all levels", 220);
    mTrayMgr->createButton(TL_TOPRIGHT, "btnAutoconfigure", "Show autoconfigured LODs", 220);
    mTrayMgr->createButton(TL_TOPRIGHT, "btnShowMesh", "Show LODs stored in mesh", 220);
    mTrayMgr->createButton(TL_TOPRIGHT, "btnSaveMesh", "Save mesh", 220);
    mTrayMgr->createButton(TL_TOPRIGHT, "btnRestoreMesh", "Restore original mesh", 220);

    // Profile options
    mProfileList = mTrayMgr->createLongSelectMenu(TL_TOPRIGHT, "cmbProfiledVertices", "Profile:", 180, 4);
    mTrayMgr->createButton(TL_TOPRIGHT, "btnRemoveFromProfile","Remove from profile", 220.0);
    mTrayMgr->createButton(TL_TOPRIGHT, "btnAddToProfile","Add to profile", 220.0);

    //mTrayMgr->createTextBox(TL_TOPRIGHT, "Help","Help", 200, 200)
    //  ->setText("The last reduced vertex is the selected vertex. Use the slider to select the vertex, then decide to keep or remove it. You can export the Lod buffers into the .mesh file after configuration.");

    mTrayMgr->showCursor();
}

void Sample_MeshLod::cleanupControls()
{
    mTrayMgr->clearTray(TL_TOPLEFT);
    mTrayMgr->clearTray(TL_TOPRIGHT);
    mTrayMgr->clearTray(TL_TOP);
}
void Sample_MeshLod::recreateEntity()
{
    // If you change the lod of a mesh, every entity referencing it should be recreated.
    if(mMeshEntity){
        mSceneMgr->destroyEntity(mMeshEntity);
        mMeshEntity = 0; // createEntity may throw exception, so it is safer to reset to 0.
    }
    mMeshEntity = mSceneMgr->createEntity(mLodConfig.mesh->getName(), mLodConfig.mesh);
    mMeshNode->attachObject(mMeshEntity);
}
void Sample_MeshLod::changeSelectedMesh( const String& name )
{
    if(mMeshEntity){
        mSceneMgr->destroyEntity(mMeshEntity);
        mMeshEntity = 0;
    }
    mLodConfig.mesh = MeshManager::getSingleton().load(name, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    if(mLodConfig.mesh->getBounds().isNull() || mLodConfig.mesh->getBoundingSphereRadius() == 0.0){
        mTrayMgr->showOkDialog("Error", "Failed to load mesh!");
        return;
    }
    mMeshEntity = mSceneMgr->createEntity(name, mLodConfig.mesh);
    mMeshNode->attachObject(mMeshEntity);
    mCameraMan->setYawPitchDist(Radian(0), Radian(0), mLodConfig.mesh->getBoundingSphereRadius() * 2);
    mCamera->setNearClipDistance(mLodConfig.mesh->getBoundingSphereRadius() / 16);
    mCamera->setFarClipDistance(mLodConfig.mesh->getBoundingSphereRadius() * 256);

    size_t vertexCount = getUniqueVertexCount(mLodConfig.mesh);
    mReductionSlider->setRange(0,vertexCount,vertexCount+1,false);
    mOutsideWeightSlider->setValue(0, false);
    mOutsideWalkAngle->setValue(0, false);
    mLodLevelList->clearItems();
    mManualMeshes->selectItem(0, false);
    mWorkLevel.distance = 1.0;
    mWorkLevel.reductionMethod = LodLevel::VRM_CONSTANT;
    mWorkLevel.reductionValue = 0.0;
    mWorkLevel.manualMeshName = "";

    loadConfig();

    if(mLodLevelList->getNumItems() > 0){
        loadLodLevel(mLodLevelList->getSelectionIndex());
    } else {
        loadUserLod();
    }
#if SHOW_MESH_HULL
    const String meshHullName("ConvexHull.mesh");
    if(mHullEntity){
        mHullNode->detachObject(mHullEntity);
        mSceneMgr->destroyEntity(mHullEntity);
        // Removes from the resources list.
        mHullEntity = NULL;
        Ogre::MeshManager::getSingleton().remove(meshHullName);
    }

    LodConfig inputConfig(mLodConfig.mesh);
    LodInputProviderPtr input;
    LodCollapseCostPtr cost;
    LodDataPtr data;
    LodOutputProviderPtr output;
    LodCollapserPtr collapser;
    MeshLodGenerator::getSingleton()._resolveComponents(inputConfig, cost, data, input, output, collapser);

    input->initData(data.get());
    LodOutsideMarker outsideMarker(data->mVertexList, data->mMeshBoundingSphereRadius, 0.0);
    MeshPtr meshHull = outsideMarker.createConvexHullMesh(meshHullName);

    mHullEntity = mSceneMgr->createEntity(meshHull);
    mHullNode->attachObject(mHullEntity);
#endif
}

bool Sample_MeshLod::loadConfig()
{
    mLodConfig.advanced = LodConfig::Advanced();
    mLodConfig.strategy = PixelCountLodStrategy::getSingletonPtr();
    mLodConfig.levels.clear();
    mLodConfig.advanced.profile.clear();

    // The mesh should already be set.
    assert(mLodConfig.mesh.get());

    String filename(mLodConfig.mesh->getName());
    filename += ".lodconfig";
    LodConfigSerializer lcs;
    lcs.importLodConfig(&mLodConfig, filename);

    mLodLevelList->clearItems();
    for(auto & level : mLodConfig.levels){
        mLodLevelList->addItem(StringConverter::toString(level.distance) + "px");
    }

    mProfileList->clearItems();
    for(auto & i : mLodConfig.advanced.profile){
        mProfileList->addItem(StringConverter::toString(i.src));
    }

    mUseVertexNormals->setChecked(mLodConfig.advanced.useVertexNormals, false);
    mOutsideWeightSlider->setValue(std::sqrt(mLodConfig.advanced.outsideWeight), false);
    mOutsideWalkAngle->setValue(mLodConfig.advanced.outsideWalkAngle, false);
    return true;
}

void Sample_MeshLod::saveConfig()
{
    String filename(mLodConfig.mesh->getName());
    filename += ".lodconfig";
    LodConfigSerializer lcs;
    lcs.exportLodConfig(mLodConfig, filename);
}

void Sample_MeshLod::loadAutomaticLod()
{
    // Remove outdated Lod requests to reduce delay.
    MeshLodGenerator::getSingleton().clearPendingLodRequests();

    MeshLodGenerator& gen = MeshLodGenerator::getSingleton();
    //gen.generateAutoconfiguredLodLevels(mLodConfig.mesh);
    LodConfig lodConfig;
    gen.getAutoconfig(mLodConfig.mesh, lodConfig);
    lodConfig.advanced.useBackgroundQueue = ENABLE_THREADING;
    lodConfig.advanced.useCompression = ENABLE_COMPRESSION;
    lodConfig.advanced.preventPunchingHoles = PREVENT_HOLES_BREAKS;
    lodConfig.advanced.preventBreakingLines = PREVENT_HOLES_BREAKS;
    lodConfig.advanced.profile = mLodConfig.advanced.profile;
    lodConfig.advanced.useVertexNormals = mLodConfig.advanced.useVertexNormals;
    gen.generateLodLevels(lodConfig);
    recreateEntity();
}

void Sample_MeshLod::loadUserLod( bool useWorkLod )
{
    if(mLodConfig.mesh->getBoundingSphereRadius() == 0){
        // failed to load mesh
        return;
    }
    if(mLodConfig.levels.empty()){
        useWorkLod = true;
    }
    mTrayMgr->destroyAllWidgetsInTray(TL_TOP);
    // Remove outdated Lod requests to reduce delay.
    MeshLodGenerator::getSingleton().clearPendingLodRequests();

    MeshLodGenerator& gen = MeshLodGenerator::getSingleton();
    mLodConfig.advanced.useBackgroundQueue = ENABLE_THREADING;
    mLodConfig.advanced.useCompression = ENABLE_COMPRESSION;
    mLodConfig.advanced.preventPunchingHoles = PREVENT_HOLES_BREAKS;
    mLodConfig.advanced.preventBreakingLines = PREVENT_HOLES_BREAKS;
    if(!useWorkLod){
        gen.generateLodLevels(mLodConfig);
#if !ENABLE_THREADING
        recreateEntity(); // Needed for manual Lod levels
#endif
        forceLodLevel(-1);
    } else {
        LodConfig config(mLodConfig);
        config.levels.clear();
        config.levels.push_back(mWorkLevel);
        gen.generateLodLevels(config);
        //gen.generateLodLevels(config, new LodCollapseCostQuadric()); // Use quadric error
#if !ENABLE_THREADING
        recreateEntity(); // Needed for manual Lod levels
#endif
        forceLodLevel(1);
    }
}
void Sample_MeshLod::forceLodLevel(int lodLevelID, bool forceDelayed)
{
    mForcedLodLevel = lodLevelID;
    // These are the requirements for async Lod generation
    if(!forceDelayed || !ENABLE_THREADING || OGRE_THREAD_SUPPORT == 0){
        if(lodLevelID == -1 || mLodConfig.mesh->getNumLodLevels() <= 1) {
            // Clear forced Lod level
            mMeshEntity->setMeshLodBias(1.0, 0, std::numeric_limits<unsigned short>::max());
        } else {
            mMeshEntity->setMeshLodBias(1.0, lodLevelID, lodLevelID);
        }
    }
}
size_t Sample_MeshLod::getUniqueVertexCount( MeshPtr mesh )
{

    // The vertex buffer contains the same vertex position multiple times.
    // To get the count of the vertices, which has unique positions, we can use progressive mesh.
    // It is constructing a mesh grid at the beginning, so if we reduce 0%, we will get the unique vertex count.
    LodConfig lodConfig(mesh, PixelCountLodStrategy::getSingletonPtr());
    lodConfig.advanced.useBackgroundQueue = false; // Non-threaded
    lodConfig.advanced.useCompression = ENABLE_COMPRESSION;
    lodConfig.advanced.preventPunchingHoles = PREVENT_HOLES_BREAKS;
    lodConfig.advanced.preventBreakingLines = PREVENT_HOLES_BREAKS;
    lodConfig.createGeneratedLodLevel(0, 0);
    MeshLodGenerator& gen = MeshLodGenerator::getSingleton();
    gen.generateLodLevels(lodConfig);
    //ProgressiveMeshGenerator pm;
    //pm.generateLodLevels(lodConfig);
    return lodConfig.levels[0].outUniqueVertexCount;
}

void Sample_MeshLod::addLodLevel()
{
    LodLevel lvl(mWorkLevel);
    lvl.distance = getCameraLODValue();
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
        mLodLevelList->addItem(StringConverter::toString(lvl.distance) + "px");
        mLodLevelList->selectItem(mLodLevelList->getNumItems() - 1, false);
    } else {
        mLodConfig.levels.insert(mLodConfig.levels.begin() + i, lvl);
        mLodLevelList->insertItem(i, StringConverter::toString(lvl.distance) + "px");
        mLodLevelList->selectItem(i, false);
    }
}

void Sample_MeshLod::loadLodLevel( int id )
{
    assert(mLodConfig.levels[id].reductionMethod == LodLevel::VRM_CONSTANT);
    mWorkLevel = mLodConfig.levels[id];
    mReductionSlider->setValue(mWorkLevel.reductionValue, false);
    mLodLevelList->selectItem(id, false);
    mManualMeshes->selectItem(mWorkLevel.manualMeshName, false);
    loadUserLod();
    moveCameraToPixelDistance(mWorkLevel.distance);
}

void Sample_MeshLod::removeLodLevel()
{
    if(mLodConfig.levels.empty()){
        return;
    }
    int selectedLevel = mLodLevelList->getSelectionIndex();
    mLodConfig.levels.erase(mLodConfig.levels.begin() + selectedLevel);
    mLodLevelList->removeItem(selectedLevel);
}

void Sample_MeshLod::removeInitialLodLevel()
{
    Ogre::Real stripValue = mLodConfig.levels.empty() ? mWorkLevel.reductionValue : mLodConfig.levels[0].reductionValue;
    if(mWorkLevel.reductionMethod == LodLevel::VRM_CONSTANT && stripValue > 0.0)
    {
        Lod0Stripifier stripifier;
        if(stripifier.StripLod0Vertices(mLodConfig.mesh))
        {
            if(!mLodConfig.levels.empty())
            {
                mLodConfig.levels.erase(mLodConfig.levels.begin());
                mLodLevelList->removeItem(0);
                loadLodLevel(0);
            }
            else
            {
                mWorkLevel.reductionValue = 0.0;
                mReductionSlider->setValue(mWorkLevel.reductionValue, false);
                loadUserLod();
            }
        }
    }
}

Real Sample_MeshLod::getCameraLODValue()
{
    if(mLodConfig.mesh->getBoundingSphereRadius() != 0.0){
        return PixelCountLodStrategy::getSingleton().getValue(mMeshEntity, mCamera);
    } else {
        return 0.0;
    }
}

void Sample_MeshLod::moveCameraToPixelDistance( Real pixels )
{
    PixelCountLodStrategy& strategy = PixelCountLodStrategy::getSingleton();
    Real distance = mLodConfig.mesh->getBoundingSphereRadius() * 4;
    const Real epsilon = pixels * 0.000001;
    const int iterations = 64;
    mCameraNode->setPosition(Vector3(0, 0, 0));
    mCameraNode->translate(Vector3(0, 0, distance), Node::TS_LOCAL);
    // We need to find a distance, which is bigger then requested
    for(int i=0;i<iterations;i++){
        Real curPixels = strategy.getValue(mMeshEntity, mCamera);
        if (curPixels > pixels) {
            distance *= 2.0;
            mCameraNode->translate(Vector3(0, 0, distance), Node::TS_LOCAL);
        } else {
            break;
        }
    }
    // Binary search for distance
    for(int i=0;i<iterations;i++){
        Real curPixels = strategy.getValue(mMeshEntity, mCamera);
        if(std::abs(curPixels - pixels) < epsilon){
            break;
        }
        distance /= 2;
        if (curPixels > pixels) {
            // move camera further
            mCameraNode->translate(Vector3(0, 0, distance), Node::TS_LOCAL);
        } else {
            // move camera nearer
            mCameraNode->translate(Vector3(0, 0, -distance), Node::TS_LOCAL);
        }
    }
}

bool Sample_MeshLod::getResourceFullPath(MeshPtr& mesh, String& outPath)
{
    ResourceGroupManager& resourceGroupMgr = ResourceGroupManager::getSingleton();
    String group = mesh->getGroup();
    String name = mesh->getName();
    Ogre::FileInfo* info = NULL;
    FileInfoListPtr locPtr = resourceGroupMgr.listResourceFileInfo(group);
    FileInfoList::iterator it, itEnd;
    it = locPtr->begin();
    itEnd = locPtr->end();
    for (; it != itEnd; it++) {
        if (StringUtil::startsWith(name, it->filename)) {
            info = &*it;
            break;
        }
    }
    if(!info) {
        outPath = name;
        return false;
    }
    outPath = info->archive->getName();
    if (outPath[outPath .size()-1] != '/' && outPath[outPath .size()-1] != '\\') {
        outPath += '/';
    }
    outPath += info->path;
    if (outPath[outPath .size()-1] != '/' && outPath[outPath .size()-1] != '\\') {
        outPath += '/';
    }
    outPath += info->filename;

    return (info->archive->getType() == "FileSystem");
}

void Sample_MeshLod::addToProfile( Real cost )
{
    LodConfig config(mLodConfig);
    config.levels.clear();
    config.levels.push_back(mWorkLevel);
    config.advanced.useBackgroundQueue = false;
    config.advanced.useCompression = ENABLE_COMPRESSION;
    config.advanced.preventPunchingHoles = PREVENT_HOLES_BREAKS;
    config.advanced.preventBreakingLines = PREVENT_HOLES_BREAKS;
    MeshLodGenerator& gen = MeshLodGenerator::getSingleton();
    LodCollapserPtr collapser(new LodCollapser());
    LodDataPtr data(new LodData());
    gen.generateLodLevels(config, LodCollapseCostPtr(), data, LodInputProviderPtr(), LodOutputProviderPtr(), collapser);
    
    ProfiledEdge pv;
    if(collapser->_getLastVertexPos(data.get(), pv.src)){
        collapser->_getLastVertexCollapseTo(data.get(), pv.dst);
        // Prevent duplicates if you edit the same vertex twice.
        size_t size = mLodConfig.advanced.profile.size();
        for(uint i=0;i<size;i++){
            ProfiledEdge& v = mLodConfig.advanced.profile[i];
            if(v.src == pv.src && v.dst == pv.dst){
                v.cost = cost;
                mProfileList->selectItem(i, false);
                loadUserLod();
                return;
            }
        }
        // Copy profile in queued build.
        pv.cost = cost;
        mLodConfig.advanced.profile.push_back(pv);
        mProfileList->addItem(StringConverter::toString(pv.src));
        mProfileList->selectItem(mProfileList->getNumItems() - 1, false);
    } else {
        mTrayMgr->showOkDialog("Error", "No vertex selected, because the mesh is not reduced.");
    }
    loadUserLod();
}

bool Sample_MeshLod::frameStarted( const FrameEvent& evt )
{
    mDistanceLabel->setCaption(StringUtil::format("Pixel Count: %d px", int(getCameraLODValue())));
    return true;
}

void Sample_MeshLod::checkBoxToggled( CheckBox * box )
{
    if(box->getName() == "chkUseVertexNormals") {
        mLodConfig.advanced.useVertexNormals = box->isChecked();
        loadUserLod();
    } else if (box->getName() == "chkShowWireframe") {
        mCamera->setPolygonMode(mWireframe->isChecked() ? PM_WIREFRAME : PM_SOLID);
    }
}

void Sample_MeshLod::itemSelected( SelectMenu* menu )
{
    if (menu->getName() == "cmbModels") {
        changeSelectedMesh(menu->getSelectedItem());
    } else if(menu->getName() == "cmbLodLevels") {
        loadLodLevel(menu->getSelectionIndex());
    } else if(menu->getName() == "cmbManualMesh") {
        mWorkLevel.manualMeshName = menu->getSelectedItem();
        loadUserLod();
    }
}

void Sample_MeshLod::sliderMoved(Slider* slider)
{
    if (slider->getName() == "sldReductionValue") {
        mWorkLevel.reductionValue = slider->getValue();
        loadUserLod();
    } else if (slider->getName() == "sldOutsideWeight") {
        if(mOutsideWeightSlider->getValue() == 100){
            mLodConfig.advanced.outsideWeight = LodData::NEVER_COLLAPSE_COST;
        } else {
            mLodConfig.advanced.outsideWeight = (mOutsideWeightSlider->getValue() * mOutsideWeightSlider->getValue()) / 10000;
        }
        loadUserLod();
    } else if (slider->getName() == "sldOutsideWalkAngle") {
        mLodConfig.advanced.outsideWalkAngle = mOutsideWalkAngle->getValue();
        loadUserLod();
    }
    
}

void Sample_MeshLod::buttonHit( OgreBites::Button* button )
{
    if(button->getName() == "btnReduceMore") {
        mReductionSlider->setValue(mReductionSlider->getValue()+1);
    } else if(button->getName() == "btnReduceLess") {
        mReductionSlider->setValue(mReductionSlider->getValue()-1);
    } else if(button->getName() == "btnAddToProfile") {
        addToProfile(std::numeric_limits<Real>::max());
    } else if(button->getName() == "btnRemoveFromProfile") {
        if(!mLodConfig.advanced.profile.empty()){
            LodProfile& profile = mLodConfig.advanced.profile;
            profile.erase(profile.begin() + mProfileList->getSelectionIndex());
            mProfileList->removeItem(mProfileList->getSelectionIndex());
            loadUserLod();
        }
    } else if(button->getName() == "btnAddLodLevel") {
        addLodLevel();
    } else if(button->getName() == "btnRemoveSelectedLodLevel") {
        removeLodLevel();
    } else if(button->getName() == "btnRemoveInitialLodLevel") {
        removeInitialLodLevel();
    } else if(button->getName() == "btnAutoconfigure") {
        mTrayMgr->destroyAllWidgetsInTray(TL_TOP);
        mTrayMgr->createLabel(TL_TOP, "lblWhatYouSee", "Showing autoconfigured LOD", 300);
        loadAutomaticLod();
        forceLodLevel(-1); // disable Lod level forcing
    } else if (button->getName() == "btnShowAll") {
        loadUserLod(false);
        mTrayMgr->destroyAllWidgetsInTray(TL_TOP);
        mTrayMgr->createLabel(TL_TOP, "lblWhatYouSee", "Showing all LOD levels", 300);
        forceLodLevel(-1); // disable Lod level forcing
    } else if(button->getName() == "btnShowMesh") {
        mTrayMgr->destroyAllWidgetsInTray(TL_TOP);
        mTrayMgr->createLabel(TL_TOP, "lblWhatYouSee", "Showing LOD from mesh file", 300);
        if(mMeshEntity){
            mSceneMgr->destroyEntity(mMeshEntity);
            mMeshEntity = 0;
        }
        mLodConfig.mesh->reload(Resource::LF_DEFAULT);
        mMeshEntity = mSceneMgr->createEntity(mLodConfig.mesh->getName(), mLodConfig.mesh);
        mMeshNode->attachObject(mMeshEntity);
        forceLodLevel(-1); // disable Lod level forcing
        //String filename("");
        //getResourceFullPath(mLodConfig.mesh, filename);
        //mTrayMgr->showOkDialog("Success", "Showing mesh from: " + filename);
    } else if(button->getName() == "btnSaveMesh") {
        if(!mTrayMgr->getTrayContainer(TL_TOP)->isVisible() && !mLodConfig.levels.empty()){
            MeshLodGenerator::getSingleton().clearPendingLodRequests();
            MeshLodGenerator& gen = MeshLodGenerator::getSingleton();
            mLodConfig.advanced.useBackgroundQueue = false; // Non-threaded
            mLodConfig.advanced.useCompression = ENABLE_COMPRESSION;
            mLodConfig.advanced.preventPunchingHoles = PREVENT_HOLES_BREAKS;
            mLodConfig.advanced.preventBreakingLines = PREVENT_HOLES_BREAKS;
            gen.generateLodLevels(mLodConfig);
            forceLodLevel(-1); // disable Lod level forcing
        }
        String filename("");
        if(!getResourceFullPath(mLodConfig.mesh, filename) || filename == "") {
            mTrayMgr->showOkDialog("Error", "'" + filename + "' is not a writable path!");
        } else {
            if(!FileSystemLayer::fileExists(filename + ".orig"))
                FileSystemLayer::renameFile(filename, filename + ".orig");
            MeshSerializer ms;
            ms.exportMesh(mLodConfig.mesh, filename);
            mTrayMgr->showOkDialog("Success", "Mesh saved to: " + filename);
        }
        if(!mTrayMgr->getTrayContainer(TL_TOP)->isVisible()){
            loadUserLod();
        }
    }
    else if(button->getName() == "btnRestoreMesh") {
        String filename("");
        if(getResourceFullPath(mLodConfig.mesh, filename) && filename != "") {
            if(FileSystemLayer::fileExists(filename + ".orig"))
                FileSystemLayer::renameFile(filename + ".orig", filename);
        }
        changeSelectedMesh(mLodConfig.mesh->getName());
    }
}

bool Sample_MeshLod::shouldInject( LodWorkQueueRequest* request )
{
    return true;
}

void Sample_MeshLod::injectionCompleted( LodWorkQueueRequest* request )
{
    recreateEntity(); // Needed for manual lod levels.
    forceLodLevel(mForcedLodLevel, false);
}
#endif
