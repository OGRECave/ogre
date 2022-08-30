#ifndef __NewInstancing_H__
#define __NewInstancing_H__

#include "SdkSample.h"
#include "OgreInstancedEntity.h"

using namespace Ogre;
using namespace OgreBites;

#define NUM_IM_TECHNIQUES (((int)InstanceManager::InstancingTechniquesCount) + 1)
#define NUM_TECHNIQUES (NUM_IM_TECHNIQUES + 2)

class _OgreSampleClassExport Sample_NewInstancing : public SdkSample
{
public:

    Sample_NewInstancing();

    bool frameRenderingQueued(const FrameEvent& evt) override;

    bool keyPressed(const KeyboardEvent& evt) override;
    

protected:
    void setupContent() override;

    void setupLighting();
    
    void switchInstancingTechnique();

    void switchSkinningTechnique(int index);

    void createEntities(int technique);

    void createInstancedEntities();

    void createSceneNodes();
    
    void clearScene();

    void destroyManagers();

    void cleanupContent() override;

    void animateUnits( float timeSinceLast );

    void moveUnits( float timeSinceLast );

    //Helper function to look towards normDir, where this vector is normalized, with fixed Yaw
    Quaternion lookAt( const Vector3 &normDir );

    void defragmentBatches();

    void setupGUI();

    void itemSelected(SelectMenu* menu) override;

    void buttonHit( OgreBites::Button* button ) override;

    void checkBoxToggled(CheckBox* box) override;

    void sliderMoved(Slider* slider) override;

    //The difference between testCapabilities() is that features checked here aren't fatal errors.
    //which means the sample can run (with limited functionality) on those computers
    void checkHardwareSupport();

    //You can also use a union type to switch between Entity and InstancedEntity almost flawlessly:
    /*
    union FusionEntity
    {
        Entity          entity
        InstancedEntity instancedEntity;
    };
    */
    int NUM_INST_ROW;
    int NUM_INST_COLUMN;
    int                             mInstancingTechnique;
    int                             mCurrentMesh;
    std::vector<MovableObject*>     mEntities;
    std::vector<InstancedEntity*>   mMovedInstances;
    std::vector<SceneNode*>         mSceneNodes;
    std::set<AnimationState*>       mAnimations;
    InstanceManager                 *mCurrentManager;
    std::array<bool, NUM_TECHNIQUES> mSupportedTechniques;
    const char**                        mCurrentMaterialSet;
    uint16                          mCurrentFlags;

    SelectMenu                      *mTechniqueMenu;
    CheckBox                        *mMoveInstances;
    CheckBox                        *mAnimateInstances;
    SelectMenu                      *mSkinningTechniques;
    CheckBox                        *mEnableShadows;
    CheckBox                        *mSetStatic;
    CheckBox                        *mUseSceneNodes;
    OgreBites::Button                   *mDefragmentBatches;
    CheckBox                        *mDefragmentOptimumCull;
    Slider                          *mInstancesSlider;
};

#endif
