#ifndef __MeshLod_H__
#define __MeshLod_H__

#include "SamplePlugin.h"
#include "SdkSample.h"
#include "OgreLodConfig.h"
#include "OgreMeshLodGenerator.h"
#include "OgreLodWorkQueueInjectorListener.h"

// To reduce checkboxes some developer features can be enabled with macros.
#define SHOW_MESH_HULL 0
#define ENABLE_THREADING 1

class _OgreSampleClassExport Sample_MeshLod :
    public OgreBites::SdkSample,
    public Ogre::LodWorkQueueInjectorListener
{
public:
    Sample_MeshLod();
protected:

// Events:
    void setupContent();
    void cleanupContent();
    void setupControls(int uimode = 0);
    void cleanupControls();
    bool frameStarted(const Ogre::FrameEvent& evt);

// GUI input events:
    void buttonHit(OgreBites::Button* button);
    void sliderMoved(OgreBites::Slider* slider);
    void itemSelected(OgreBites::SelectMenu* menu);
    void checkBoxToggled(OgreBites::CheckBox * box);

// Queued Lod injector events:
    bool shouldInject(Ogre::LodWorkQueueRequest* request);
    void injectionCompleted(Ogre::LodWorkQueueRequest* request);

// Other functions:
    void changeSelectedMesh(const Ogre::String& name); // Changes current mesh to a mesh with given mesh name.
    bool loadConfig(); /// Loads the LodConfig with LodConfigSerializer for current mesh.
    void saveConfig(); /// Saves the LodConfig with LodConfigSerializer for current mesh.
    void loadUserLod(bool useWorkLod = true); /// Loads current Lod config. If useWorkLod is selected only current work Lod level will be shown.
    void forceLodLevel(int lodLevelID, bool forceDelayed = true); /// Forces given Lod Level or -1 for disable forcing.

    void loadAutomaticLod(); /// Produces acceptable output on any kind of mesh.

    size_t getUniqueVertexCount(Ogre::MeshPtr mesh); /// Returns the unique vertex count of mesh.
    bool getResourceFullPath(Ogre::MeshPtr& mesh, Ogre::String& outPath); /// Sets outPath to full resource file path. Returns true if location is writable.
    
    void addLodLevel(); /// Adds current work Lod level to the mesh Lod levels.
    void loadLodLevel(int id); /// Loads the Lod levels with id to the work Lod level.
    void removeLodLevel(); /// Removes currently selected Lod level.
    void removeInitialLodLevel(); /// Removes Lod level #0 and unused vertices.

    void addToProfile(Ogre::Real cost); /// Add the currently reduced last vertex to the profile with given cost.

    void moveCameraToPixelDistance(Ogre::Real pixels); /// Moves camera to the swapping distance of PixelCountLodStrategy with given pixels.
    Ogre::Real getCameraDistance(); /// Returns the distance between camera and mesh in pixels.
    void recreateEntity();


// Variables:
    int mForcedLodLevel; /// Currently forced Lod level or -1 for disabled.
    Ogre::LodLevel mWorkLevel; /// Current Lod Level, which we are seeing.
    Ogre::LodConfig mLodConfig; /// Current LodConfig, which we are editing.
    Ogre::Entity* mMeshEntity; /// Entity of the mesh.
    Ogre::SceneNode* mMeshNode; /// Node of the mesh.
#if SHOW_MESH_HULL
    Ogre::Entity* mHullEntity; /// Entity of the mesh hull.
    Ogre::SceneNode* mHullNode; /// Node of the mesh hull.
#endif

// GUI elements:
    OgreBites::CheckBox* mUseVertexNormals;
    OgreBites::CheckBox* mWireframe;
    OgreBites::SelectMenu* mProfileList;
    OgreBites::SelectMenu* mLodLevelList;
    OgreBites::SelectMenu* mManualMeshes;
    OgreBites::Slider* mReductionSlider;
    OgreBites::Slider* mOutsideWeightSlider;
    OgreBites::Slider* mOutsideWalkAngle;
    OgreBites::Label* mDistanceLabel;
};

#endif
