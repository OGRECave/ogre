#include "SamplePlugin.h"
#include "NewInstancing.h"

#include <random>

using namespace Ogre;
using namespace OgreBites;

static const char *c_instancingTechniques[] =
{
    "Shader Based",
    "Vertex Texture Fetch (VTF)",
    "Hardware Instancing Basic",
    "Hardware Instancing + VTF",
    "Limited Animation - Hardware Instancing + VTF",
    "No Instancing"
};

static const char *c_materialsTechniques[] =
{
    "Examples/Instancing/RTSS/Robot",
    "Examples/Instancing/VTF/Robot",
    "Examples/Instancing/HWBasic/Robot",
    "Examples/Instancing/VTF/HW/Robot",
    "Examples/Instancing/VTF/HW/LUT/Robot",
    "Examples/Instancing/RTSS/Robot"
};

static const char *c_materialsTechniques_dq[] =
{
    "Examples/Instancing/RTSS/Robot_dq",
    "Examples/Instancing/VTF/Robot_dq",
    "Examples/Instancing/HWBasic/Robot",
    "Examples/Instancing/VTF/HW/Robot_dq",
    "Examples/Instancing/VTF/HW/LUT/Robot_dq",
    "Examples/Instancing/RTSS/Robot_dq"
};

static const char *c_materialsTechniques_dq_two_weights[] =
{
    "Examples/Instancing/RTSS/spine_dq_two_weights",
    "Examples/Instancing/VTF/spine_dq_two_weights",
    "Examples/Instancing/HWBasic/spine",
    "Examples/Instancing/VTF/HW/spine_dq_two_weights",
    "Examples/Instancing/VTF/HW/LUT/spine_dq_two_weights",
    "Examples/Instancing/RTSS/spine_dq_two_weights"
};

static const char *c_meshNames[] =
{
    "robot.mesh",
    "spine.mesh"
};

//------------------------------------------------------------------------------
Sample_NewInstancing::Sample_NewInstancing() : NUM_INST_ROW(50), NUM_INST_COLUMN(50), mCurrentManager(0), mCurrentMaterialSet(c_materialsTechniques), mCurrentFlags(0), mSkinningTechniques(NULL)
{
    mInfo["Title"] = "New Instancing";
    mInfo["Description"] = "Demonstrates how to use the new InstancedManager to setup many dynamic"
        " instances of the same mesh with much less performance impact";
    mInfo["Thumbnail"] = "thumb_newinstancing.png";
    mInfo["Category"] = "Environment";
    mInfo["Help"] = "Press Space to switch Instancing Techniques.\n"
        "Press B to toggle bounding boxes.\n\n"
        "Changes in the slider take effect after switching instancing technique\n"
        "Different batch sizes give different results depending on CPU culling"
        " and instance numbers on the scene.\n\n"
        "If performance is too slow, try defragmenting batches once in a while";
}


//------------------------------------------------------------------------------
bool Sample_NewInstancing::frameRenderingQueued(const FrameEvent& evt)
{
    if( mAnimateInstances->isChecked() )
        animateUnits( evt.timeSinceLastEvent );

    if( mMoveInstances->isChecked() )
        moveUnits( evt.timeSinceLastEvent );

    return SdkSample::frameRenderingQueued(evt);        // don't forget the parent class updates!
}

//------------------------------------------------------------------------------
bool Sample_NewInstancing::keyPressed(const KeyboardEvent& evt)
{
    Keycode key = evt.keysym.sym;
    //Toggle bounding boxes with B key unless the help dialog is visible
    if (key == 'b' && !mTrayMgr->isDialogVisible() && mCurrentManager)
    {
        bool oldShow = mCurrentManager->getSetting( InstanceManager::SHOW_BOUNDINGBOX,
            mCurrentMaterialSet[mInstancingTechnique] );
        mCurrentManager->setSetting( InstanceManager::SHOW_BOUNDINGBOX, !oldShow );
    }

    //Switch to next instancing technique with space bar
    if (key == SDLK_SPACE && !mTrayMgr->isDialogVisible())
        mTechniqueMenu->selectItem( (mTechniqueMenu->getSelectionIndex() + 1) % (NUM_TECHNIQUES+1) );

    return SdkSample::keyPressed(evt);
}

//------------------------------------------------------------------------------
void Sample_NewInstancing::setupContent()
{
#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
    // Make this viewport work with shader generator scheme.
    mViewport->setMaterialScheme(MSN_SHADERGEN);
    RTShader::ShaderGenerator& rtShaderGen = RTShader::ShaderGenerator::getSingleton();
    RTShader::RenderState* schemRenderState = rtShaderGen.getRenderState(MSN_SHADERGEN);
    RTShader::SubRenderState* subRenderState = rtShaderGen.createSubRenderState("SGX_IntegratedPSSM3");
    schemRenderState->addTemplateSubRenderState(subRenderState);

    //Add the hardware skinning to the shader generator default render state
    subRenderState = mShaderGenerator->createSubRenderState<RTShader::HardwareSkinning>();
    schemRenderState->addTemplateSubRenderState(subRenderState);

    // increase max bone count for higher efficiency
    RTShader::HardwareSkinningFactory::getSingleton().setMaxCalculableBoneCount(80);

    // re-generate shaders to include new SRSs
    rtShaderGen.invalidateScheme(MSN_SHADERGEN);

    // update scheme for FFP supporting rendersystems
    MaterialManager::getSingleton().setActiveScheme(mViewport->getMaterialScheme());
#endif

    //Initialize the techniques and current mesh variables
    mInstancingTechnique    = 0;
    mCurrentMesh            = 0;
    mCurrentManager         = 0;

    checkHardwareSupport();

    mSceneMgr->setShadowTechnique( SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED );
    mSceneMgr->setShadowTextureSelfShadow( true );
    mSceneMgr->setShadowCasterRenderBackFaces( true );

    if (Ogre::Root::getSingletonPtr()->getRenderSystem()->getName().find("OpenGL ES 2") == String::npos)
    {
        mSceneMgr->setShadowTextureConfig( 0, 2048, 2048, PF_DEPTH16 );
    }
    else
    {
        // Use a smaller texture for GL ES 3.0
        mSceneMgr->setShadowTextureConfig( 0, 512, 512, PF_DEPTH16 );
    }

    mSceneMgr->setShadowCameraSetup( LiSPSMShadowCameraSetup::create() );

    mEntities.reserve( NUM_INST_ROW * NUM_INST_COLUMN );
    mSceneNodes.reserve( NUM_INST_ROW * NUM_INST_COLUMN );
    
    mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");

    // create a mesh for our ground
    MeshManager::getSingleton().createPlane("ground", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        Plane(Vector3::UNIT_Y, 0), 10000, 10000, 20, 20, true, 1, 6, 6, Vector3::UNIT_Z);

    // create a ground entity from our mesh and attach it to the origin
    Entity* ground = mSceneMgr->createEntity("Ground", "ground");
    ground->setMaterialName("Examples/GrassFloor");
    ground->setCastShadows(false);
    mSceneMgr->getRootSceneNode()->attachObject(ground);

    setupLighting();

    // set initial camera position and speed
    mCameraNode->setPosition( 0, 120, 100 );

    setupGUI();

#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
    setDragLook(true);
#endif

    switchInstancingTechnique();
}
//------------------------------------------------------------------------------
void Sample_NewInstancing::setupLighting()
{
    mSceneMgr->setAmbientLight( ColourValue( 0.40f, 0.40f, 0.40f ) );

    //Create main light
    Light* light = mSceneMgr->createLight();
    light->setType( Light::LT_DIRECTIONAL );
    light->setDiffuseColour(1, 0.5, 0.3);
    auto n = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    n->attachObject(light);
    n->setDirection(0, -1, -1);
    light->setSpecularColour( 0.6, 0.82, 1.0 );
}

//------------------------------------------------------------------------------
void Sample_NewInstancing::switchInstancingTechnique()
{
    //mInstancingTechnique = (mInstancingTechnique+1) % (NUM_TECHNIQUES+1);
    mInstancingTechnique = mTechniqueMenu->getSelectionIndex();

    if( mCurrentManager )
        mSceneMgr->destroyInstanceManager(mCurrentManager);

    if( !mSupportedTechniques[mInstancingTechnique] )
    {
        //Hide GUI features available only to instancing
        mCurrentManager = 0;
        mDefragmentBatches->hide();
        mDefragmentOptimumCull->hide();
        return;
    }

    if( mInstancingTechnique < NUM_TECHNIQUES )
    {
        //Instancing

        //Create the manager if we haven't already (i.e. first time)
        //Because we use IM_USEALL as flags, the actual num of instances per batch might be much lower
        //If you're not bandwidth limited, you may want to lift IM_VTFBESTFIT flag away

        InstanceManager::InstancingTechnique technique = InstanceManager::ShaderBased;
    
        switch( mInstancingTechnique )
        {
        case 0: technique = InstanceManager::ShaderBased; break;
        case 1: technique = InstanceManager::TextureVTF; break;
        case 2: technique = InstanceManager::HWInstancingBasic; break;
        case 3:
        case 4: technique = InstanceManager::HWInstancingVTF; break;
        }

        uint16 flags = IM_USEALL;
        flags |= mCurrentFlags;
        
        if (mInstancingTechnique == 4)
        {
            flags |= IM_VTFBONEMATRIXLOOKUP;
        }
        //Only one weight is recommended for the VTF technique, but force the use of more for the demo
        if(mInstancingTechnique == 1 && (flags & IM_USEBONEDUALQUATERNIONS))
        {
            flags &= ~IM_USEONEWEIGHT;
        }

        mCurrentManager = mSceneMgr->createInstanceManager(
            "InstanceMgr" + StringConverter::toString(mInstancingTechnique), c_meshNames[mCurrentMesh],
            ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, technique,
            NUM_INST_ROW * NUM_INST_COLUMN, flags);

        createInstancedEntities();

        //Show GUI features available only to instancing
        mDefragmentBatches->show();
        mDefragmentOptimumCull->show();
    }
    else
    {
        //Non-instancing
        createEntities();

        //Hide GUI features available only to instancing
        mCurrentManager = 0;
        mDefragmentBatches->hide();
        mDefragmentOptimumCull->hide();
    }

    createSceneNodes();

    //Show/hide "static" button, and restore config. Do this _after_ createSceneNodes()
    if( mInstancingTechnique == InstanceManager::HWInstancingBasic ||
        mInstancingTechnique == InstanceManager::HWInstancingVTF ||
        mInstancingTechnique == InstanceManager::HWInstancingVTF + 1) // instancing with lookup
    {
        if( mSetStatic->isChecked() )
            mCurrentManager->setBatchesAsStaticAndUpdate( mSetStatic->isChecked() );
        mSetStatic->show();
    }
    else
        mSetStatic->hide();
    if( mInstancingTechnique < NUM_TECHNIQUES)
    {
        mUseSceneNodes->show();
    }
    else
    {
        mUseSceneNodes->hide();
    }
}

//------------------------------------------------------------------------------
void Sample_NewInstancing::switchSkinningTechnique(int index)
{
    switch(index)
    {
        default:
        //Linear Skinning
        case 0:
            mCurrentMesh = 0;
            mCurrentMaterialSet = c_materialsTechniques;
            mCurrentFlags = 0;
            break;
        //Dual Quaternion Skinning
        case 1:
            mCurrentMesh = 0;
            mCurrentMaterialSet = c_materialsTechniques_dq;
            mCurrentFlags = IM_USEBONEDUALQUATERNIONS;
            break;
        //Dual Quaternion Skinning with two weights
        case 2:
            mCurrentMesh = 1;
            mCurrentMaterialSet = c_materialsTechniques_dq_two_weights;
            mCurrentFlags = IM_USEBONEDUALQUATERNIONS;
            break;
    };
}

//------------------------------------------------------------------------------
void Sample_NewInstancing::createEntities()
{
    std::mt19937 rng;
    for( int i=0; i<NUM_INST_ROW * NUM_INST_COLUMN; ++i )
    {
        //Create the non-instanced entity. Use the same shader as shader-based because:
        //a. To prove we can (runs without modification! :-) )
        //b. Make a fair comparison
        Entity *ent = mSceneMgr->createEntity( c_meshNames[mCurrentMesh] );
        ent->setMaterialName( mCurrentMaterialSet[NUM_TECHNIQUES] );
        mEntities.push_back( ent );

        //Get the animation
        AnimationState *anim = ent->getAnimationState( "Walk" );
        if (mAnimations.insert( anim ).second)
        {
            anim->setEnabled( true );
            anim->addTime( float(rng())/float(rng.max()) * 10 ); //Random start offset
        }
    }
}
//------------------------------------------------------------------------------
void Sample_NewInstancing::createInstancedEntities()
{
    std::mt19937 rng;
    std::mt19937 orng; // separate oriention rng for consistency with other techniques
    for( int i=0; i<NUM_INST_ROW; ++i )
    {
        for( int j=0; j<NUM_INST_COLUMN; ++j )
        {
            //Create the instanced entity
        InstancedEntity *ent = mCurrentManager->createInstancedEntity(mCurrentMaterialSet[mInstancingTechnique] );
            mEntities.push_back( ent );

            //HWInstancingBasic is the only technique without animation support
            if( mInstancingTechnique != InstanceManager::HWInstancingBasic)
            {
                //Get the animation
                AnimationState *anim = ent->getAnimationState( "Walk" );
                anim->setEnabled( true );
                anim->addTime( float(rng())/float(rng.max()) * 10); //Random start offset
                mAnimations.insert( anim );
            }

            if ((mInstancingTechnique < NUM_TECHNIQUES) && (!mUseSceneNodes->isChecked()))
            {
                mMovedInstances.push_back( ent );
                ent->setOrientation(Quaternion(Radian(float(orng())/float(orng.max()) * 10 * Math::PI), Vector3::UNIT_Y));
                ent->setPosition( Ogre::Vector3(mEntities[0]->getBoundingRadius() * (i - NUM_INST_ROW * 0.5f), 0,
                    mEntities[0]->getBoundingRadius() * (j - NUM_INST_COLUMN * 0.5f)) );
            }
        }
    }
}
//------------------------------------------------------------------------------
void Sample_NewInstancing::createSceneNodes()
{
    //Here the SceneNodes are created. Since InstancedEntities derive from MovableObject,
    //they behave like regular Entities on this.
    SceneNode *rootNode = mSceneMgr->getRootSceneNode();

    std::mt19937 rng;
    for( int i=0; i<NUM_INST_ROW; ++i )
    {
        for( int j=0; j<NUM_INST_COLUMN; ++j )
        {
            int idx = i * NUM_INST_COLUMN + j;
            if ((mInstancingTechnique >= NUM_TECHNIQUES) || (mUseSceneNodes->isChecked()))
            {
                SceneNode *sceneNode = rootNode->createChildSceneNode();
                sceneNode->attachObject( mEntities[idx] );
                sceneNode->yaw( Radian( float(rng())/float(rng.max()) * 10 * Math::PI )); //Random orientation
                sceneNode->setPosition( mEntities[idx]->getBoundingRadius() * (i - NUM_INST_ROW * 0.5f), 0,
                    mEntities[idx]->getBoundingRadius() * (j - NUM_INST_COLUMN * 0.5f) );
                mSceneNodes.push_back( sceneNode );
            }
            
        }
    }
}
//------------------------------------------------------------------------------
void Sample_NewInstancing::clearScene()
{
    std::vector<MovableObject*>::const_iterator itor = mEntities.begin();
    std::vector<MovableObject*>::const_iterator end  = mEntities.end();

    //Note: Destroying the instance manager automatically destroys all instanced entities
    //created by this manager (beware of not leaving reference to those pointers)
    while( itor != end )
    {
        SceneNode *sceneNode = (*itor)->getParentSceneNode();
        if (sceneNode)
        {
            sceneNode->detachAllObjects();
            sceneNode->getParentSceneNode()->removeAndDestroyChild( sceneNode );
        }
        if( mInstancingTechnique == NUM_TECHNIQUES )
            mSceneMgr->destroyEntity( (*itor)->getName() );
        else
            mSceneMgr->destroyInstancedEntity( static_cast<InstancedEntity*>(*itor) );

        ++itor;
    }

    //Free some memory, but don't destroy the manager so when we switch this technique
    //back again it doesn't take too long
    if( mCurrentManager )
        mCurrentManager->cleanupEmptyBatches();

    mEntities.clear();
    mMovedInstances.clear();
    mSceneNodes.clear();
    mAnimations.clear();
}
//-----------------------------------------------------------------------------------
void Sample_NewInstancing::destroyManagers()
{
    mSceneMgr->destroyInstanceManager(mCurrentManager);;
}

//------------------------------------------------------------------------------
void Sample_NewInstancing::cleanupContent()
{
    MeshManager::getSingleton().remove("ground", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    clearScene();
    destroyManagers();
}

//------------------------------------------------------------------------------
void Sample_NewInstancing::animateUnits( float timeSinceLast )
{
    //Iterates through all AnimationSets and updates the animation being played. Demonstrates the
    //animation is unique and independent to each instance
    std::set<AnimationState*>::const_iterator itor = mAnimations.begin();
    std::set<AnimationState*>::const_iterator end  = mAnimations.end();

    while( itor != end )
    {
        (*itor)->addTime( timeSinceLast );
        ++itor;
    }
}

//------------------------------------------------------------------------------
void Sample_NewInstancing::moveUnits( float timeSinceLast )
{
    Real fMovSpeed = 1.0f;

    if( !mEntities.empty() )
        fMovSpeed = mEntities[0]->getBoundingRadius() * 0.30f;

    if (!mSceneNodes.empty())
    {
        //Randomly move the units along their normal, bouncing around invisible walls
        std::vector<SceneNode*>::const_iterator itor = mSceneNodes.begin();
        std::vector<SceneNode*>::const_iterator end  = mSceneNodes.end();

        while( itor != end )
        {
            //Calculate bounces
            Vector3 entityPos = (*itor)->getPosition();
            Vector3 planeNormal = Vector3::ZERO;
            if( (*itor)->getPosition().x < -5000.0f )
            {
                planeNormal = Vector3::UNIT_X;
                entityPos.x = -4999.0f;
            }
            else if( (*itor)->getPosition().x > 5000.0f )
            {
                planeNormal = Vector3::NEGATIVE_UNIT_X;
                entityPos.x = 4999.0f;
            }
            else if( (*itor)->getPosition().z < -5000.0f )
            {
                planeNormal = Vector3::UNIT_Z;
                entityPos.z = -4999.0f;
            }
            else if( (*itor)->getPosition().z > 5000.0f )
            {
                planeNormal = Vector3::NEGATIVE_UNIT_Z;
                entityPos.z = 4999.0f;
            }

            if( planeNormal != Vector3::ZERO )
            {
                const Vector3 vDir( (*itor)->getOrientation().xAxis().normalisedCopy() );
                (*itor)->setOrientation( lookAt( planeNormal.reflect( vDir ) ) );
                (*itor)->setPosition( entityPos );
            }

            //Move along the direction we're looking to
            (*itor)->translate( Vector3::UNIT_X * timeSinceLast * fMovSpeed, Node::TS_LOCAL );
            ++itor;
        }
    }
    else
    {
        //No scene nodes (instanced entities only) 
        //Update instanced entities directly

        //Randomly move the units along their normal, bouncing around invisible walls
        std::vector<InstancedEntity*>::const_iterator itor = mMovedInstances.begin();
        std::vector<InstancedEntity*>::const_iterator end  = mMovedInstances.end();

        while( itor != end )
        {
            //Calculate bounces
            InstancedEntity* pEnt = *itor;
            Vector3 entityPos = pEnt->getPosition();
            Vector3 planeNormal = Vector3::ZERO;
            if( pEnt->getPosition().x < -5000.0f )
            {
                planeNormal = Vector3::UNIT_X;
                entityPos.x = -4999.0f;
            }
            else if( pEnt->getPosition().x > 5000.0f )
            {
                planeNormal = Vector3::NEGATIVE_UNIT_X;
                entityPos.x = 4999.0f;
            }
            else if( pEnt->getPosition().z < -5000.0f )
            {
                planeNormal = Vector3::UNIT_Z;
                entityPos.z = -4999.0f;
            }
            else if( pEnt->getPosition().z > 5000.0f )
            {
                planeNormal = Vector3::NEGATIVE_UNIT_Z;
                entityPos.z = 4999.0f;
            }

            if( planeNormal != Vector3::ZERO )
            {
                const Vector3 vDir(pEnt->getOrientation().xAxis().normalisedCopy() );
                pEnt->setOrientation( lookAt( planeNormal.reflect( vDir ) ), false );
                pEnt->setPosition( entityPos, false);
            }

            //Move along the direction we're looking to
            Vector3 transAmount = Vector3::UNIT_X * timeSinceLast * fMovSpeed;
            pEnt->setPosition( pEnt->getPosition() + pEnt->getOrientation() * transAmount );
            ++itor;
        }
    }
}

//------------------------------------------------------------------------------
Quaternion Sample_NewInstancing::lookAt( const Vector3 &normDir )
{
    return Math::lookRotation(normDir.normalisedCopy(), Vector3::UNIT_Y);
}

//------------------------------------------------------------------------------
void Sample_NewInstancing::defragmentBatches()
{
    //Defragment batches is used after many InstancedEntities were removed (and you won't
    //be requesting more). However, then the optimize cull option is on, it can cause
    //quite a perf. boost on large batches (i.e. VTF) even if not a single instance was ever removed.
    if( mCurrentManager )
        mCurrentManager->defragmentBatches( mDefragmentOptimumCull->isChecked() );
}

//------------------------------------------------------------------------------
void Sample_NewInstancing::setupGUI()
{
    mTechniqueMenu = mTrayMgr->createLongSelectMenu(
        TL_TOPLEFT, "TechniqueSelectMenu", "Technique", 450, 350, 5);
    for( int i=0; i<NUM_TECHNIQUES+1; ++i )
    {
        String text = c_instancingTechniques[i];
        if( !mSupportedTechniques[i] )
            text = "Unsupported: " + text;
        mTechniqueMenu->addItem( text );
    }
    //Check box to enable dual quaternion skinning
    mSkinningTechniques = mTrayMgr->createLongSelectMenu(TL_TOPLEFT, "SkinningTechnique", "Skinning Technique", 450, 285, 5);
    mSkinningTechniques->addItem("Linear Skinning");
    mSkinningTechniques->addItem("Dual Quaternion Skinning");
    mSkinningTechniques->addItem("Dual Quaternion Skinning (2 wgts)");

    //Check box to move the units
    mMoveInstances = mTrayMgr->createCheckBox(TL_TOPRIGHT, "MoveInstances", "Move Instances", 175);
    mMoveInstances->setChecked(false);

    //Check box to animate the units
    mAnimateInstances = mTrayMgr->createCheckBox(TL_TOPRIGHT, "AnimateInstances",
        "Animate Instances", 175);
    mAnimateInstances->setChecked(false);

    //Checkbox to toggle shadows
    mEnableShadows = mTrayMgr->createCheckBox(TL_TOPRIGHT, "EnableShadows",
        "Enable Shadows", 175);
    mEnableShadows->setChecked(true);

    //Check box to make instances static (where supported)
    mSetStatic = mTrayMgr->createCheckBox(TL_TOPRIGHT, "SetStatic", "Set Static", 175);
    mSetStatic->setChecked(false);

    //Checkbox to toggle use of scene nodes
    mUseSceneNodes = mTrayMgr->createCheckBox(TL_TOPRIGHT, "UseSceneNodes",
        "Use Scene Nodes", 175);
    mUseSceneNodes->setChecked(true, false);

    //Controls to control batch defragmentation on the fly
    mDefragmentBatches =  mTrayMgr->createButton(TL_RIGHT, "DefragmentBatches",
        "Defragment Batches", 175);
    mDefragmentOptimumCull = mTrayMgr->createCheckBox(TL_RIGHT, "DefragmentOptimumCull",
        "Optimum Cull", 175);
    mDefragmentOptimumCull->setChecked(true);

    //Slider to control max number of instances
    mInstancesSlider = mTrayMgr->createThickSlider( TL_TOPLEFT, "InstancesSlider", "Instances (NxN)",
        300, 50, 4, 100, 97 );
    mInstancesSlider->setValue( NUM_INST_ROW );

    mTrayMgr->showCursor();
}

//------------------------------------------------------------------------------
void Sample_NewInstancing::itemSelected( SelectMenu* menu )
{
    if (menu == mTechniqueMenu)
    {
        clearScene();
        switchInstancingTechnique();
    }
    else if(menu == mSkinningTechniques)
    {
        clearScene();
        switchSkinningTechnique(menu->getSelectionIndex());
        switchInstancingTechnique();
    }
}

//------------------------------------------------------------------------------
void Sample_NewInstancing::buttonHit( OgreBites::Button* button )
{
    if( button == mDefragmentBatches ) defragmentBatches();
}

//------------------------------------------------------------------------------
void Sample_NewInstancing::checkBoxToggled( CheckBox* box )
{
    if( box == mEnableShadows )
    {
        mSceneMgr->setShadowTechnique( mEnableShadows->isChecked() ?
            SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED : SHADOWTYPE_NONE );
    }
    else if( box == mSetStatic && mCurrentManager )
    {
        mCurrentManager->setBatchesAsStaticAndUpdate( mSetStatic->isChecked() );
    }
    else if (box == mUseSceneNodes)
    {
        clearScene();
        switchInstancingTechnique();
    }
}

//------------------------------------------------------------------------------
void Sample_NewInstancing::sliderMoved( Slider* slider )
{
    if( slider == mInstancesSlider ) NUM_INST_ROW = static_cast<int>(mInstancesSlider->getValue());
    NUM_INST_COLUMN = static_cast<int>(mInstancesSlider->getValue());
}

//------------------------------------------------------------------------------

void Sample_NewInstancing::checkHardwareSupport()
{
    //Check Technique support
    for( int i=0; i<NUM_TECHNIQUES; ++i )
    {
        InstanceManager::InstancingTechnique technique;
        switch( i )
        {
        case 0: technique = InstanceManager::ShaderBased; break;
        case 1: technique = InstanceManager::TextureVTF; break;
        case 2: technique = InstanceManager::HWInstancingBasic; break;
        case 3: 
        case 4: technique = InstanceManager::HWInstancingVTF; break;
        }

        uint16 flags = IM_USEALL;
        if (i == 4)
        {
            flags |= IM_VTFBONEMATRIXLOOKUP;
        }

        const size_t numInstances = mSceneMgr->getNumInstancesPerBatch( c_meshNames[mCurrentMesh],
            ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
            mCurrentMaterialSet[i], technique, NUM_INST_ROW * NUM_INST_COLUMN, flags );

        mSupportedTechniques[i] = numInstances > 0;
    }

    //Non instancing is always supported
    mSupportedTechniques[NUM_TECHNIQUES] = true;
}
