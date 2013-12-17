#include "SamplePlugin.h"
#include "NewInstancing.h"

#include "Compositor/OgreCompositorShadowNodeDef.h"
#include "Compositor/Pass/PassClear/OgreCompositorPassClearDef.h"

using namespace Ogre;
using namespace OgreBites;

#ifndef OGRE_STATIC_LIB

SamplePlugin* sp;
Sample* s;

extern "C" _OgreSampleExport void dllStartPlugin()
{
	s = new Sample_NewInstancing;
	sp = OGRE_NEW SamplePlugin(s->getInfo()["Title"] + " Sample");
	sp->addSample(s);
	Root::getSingleton().installPlugin(sp);
}

extern "C" _OgreSampleExport void dllStopPlugin()
{
	Root::getSingleton().uninstallPlugin(sp); 
	OGRE_DELETE sp;
	delete s;
}

#endif

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
	"Examples/Instancing/ShaderBased/Robot",
	"Examples/Instancing/VTF/Robot",
	"Examples/Instancing/HWBasic/Robot",
	"Examples/Instancing/VTF/HW/Robot",
	"Examples/Instancing/VTF/HW/LUT/Robot",
	"Examples/Instancing/ShaderBased/Robot"
};

static const char *c_materialsTechniques_dq[] =
{
	"Examples/Instancing/ShaderBased/Robot_dq",
	"Examples/Instancing/VTF/Robot_dq",
	"Examples/Instancing/HWBasic/Robot",
	"Examples/Instancing/VTF/HW/Robot_dq",
	"Examples/Instancing/VTF/HW/LUT/Robot_dq",
	"Examples/Instancing/ShaderBased/Robot_dq"
};

static const char *c_materialsTechniques_dq_two_weights[] =
{
	"Examples/Instancing/ShaderBased/spine_dq_two_weights",
	"Examples/Instancing/VTF/spine_dq_two_weights",
	"Examples/Instancing/HWBasic/spine",
	"Examples/Instancing/VTF/HW/spine_dq_two_weights",
	"Examples/Instancing/VTF/HW/LUT/spine_dq_two_weights",
	"Examples/Instancing/ShaderBased/spine_dq_two_weights"
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
	mBackgroundColor = ColourValue( 0.6f, 0.0f, 0.6f );
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
bool Sample_NewInstancing::keyPressed(const OIS::KeyEvent& evt)
{
	//Toggle bounding boxes with B key unless the help dialog is visible
	if (evt.key == OIS::KC_B && !mTrayMgr->isDialogVisible() && mCurrentManager)
	{
		bool oldShow = mCurrentManager->getSetting( InstanceManager::SHOW_BOUNDINGBOX,
			mCurrentMaterialSet[mInstancingTechnique] );
		mCurrentManager->setSetting( InstanceManager::SHOW_BOUNDINGBOX, !oldShow );
	}

	//Switch to next instancing technique with space bar
	if (evt.key == OIS::KC_SPACE && !mTrayMgr->isDialogVisible())
		mTechniqueMenu->selectItem( (mTechniqueMenu->getSelectionIndex() + 1) % (NUM_TECHNIQUES+1) );

	return SdkSample::keyPressed(evt);
}

//------------------------------------------------------------------------------
void Sample_NewInstancing::setupContent()
{
	//Initialize the techniques and current mesh variables
	mInstancingTechnique	= 0;
	mCurrentMesh			= 0;
	mCurrentManager			= 0;

	checkHardwareSupport();

	CompositorManager2 *compositorManager = mRoot->getCompositorManager2();

	//Setup the shadow node. (it's waaay easier when done in a compositor script. But gotta showcase...)
	CompositorShadowNodeDef *shadowNode = compositorManager->addShadowNodeDefinition(
																"NewInstancing Shadows" );
	shadowNode->setNumShadowTextureDefinitions( 1 );
	ShadowTextureDefinition *texDef = shadowNode->addShadowTextureDefinition(0, 0, "shadowmap0", false);
	texDef->width	= 2048;
	texDef->height	= 2048;
	texDef->formatList.push_back( PF_FLOAT32_R );
	texDef->shadowMapTechnique = SHADOWMAP_FOCUSED;

	shadowNode->setNumTargetPass( 1 );
	{
		CompositorTargetDef *targetDef = shadowNode->addTargetPass( "shadowmap0" );
		targetDef->setNumPasses( 2 );
		{
			CompositorPassDef *passDef = targetDef->addPass( PASS_CLEAR );
			static_cast<CompositorPassClearDef*>(passDef)->mColourValue = ColourValue::White;
			passDef = targetDef->addPass( PASS_SCENE );
			passDef->mShadowMapIdx = 0;
			passDef->mIncludeOverlays = false;
		}
	}

	mEntities.reserve( NUM_INST_ROW * NUM_INST_COLUMN );
	mSceneNodes.reserve( NUM_INST_ROW * NUM_INST_COLUMN );
	
	mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");

	// create a mesh for our ground
	MeshManager::getSingleton().createPlane("ground", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		Plane(Vector3::UNIT_Y, 0), 10000, 10000, 20, 20, true, 1, 6, 6, Vector3::UNIT_Z);

	// create a ground entity from our mesh and attach it to the origin
	Entity* ground = mSceneMgr->createEntity( "ground",
												ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
												SCENE_STATIC );
	ground->setMaterialName("Examples/Instancing/Misc/Grass");
	ground->setCastShadows(false);
	mSceneMgr->getRootSceneNode( SCENE_STATIC )->attachObject(ground);

	setupLighting();

	// set initial camera position and speed
	mCamera->setPosition( 0, 120, 100 );

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

	ColourValue lightColour( 1, 0.5, 0.3 );

	//Create main (point) light
	SceneNode *lightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	Light* light = mSceneMgr->createLight();
	lightNode->attachObject( light );
	light->setDiffuseColour(lightColour);
	lightNode->setPosition( 0.0f, 25.0f, 0.0f );
	light->setSpecularColour( 0.6, 0.82, 1.0 );
	light->setAttenuation( 3500, 0.085, 0.00008, 0.00006 );
	light->setCastShadows( false );

	//Create a dummy spot light for shadows
	lightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	light = mSceneMgr->createLight();
	lightNode->attachObject( light );
	light->setType( Light::LT_SPOTLIGHT );
	light->setDiffuseColour( ColourValue( 0.15f, 0.35f, 0.44f ) );
	lightNode->setPosition( 250.0f, 200.0f, 250.0f );
	light->setDirection( (Vector3::UNIT_SCALE * -1.0f).normalisedCopy() );
	light->setSpecularColour( 0.2, 0.12, 0.11 );
	light->setAttenuation( 3500, 0.005, 0.00002, 0.00001 );
	light->setSpotlightRange( Degree(80), Degree(90) );
	light->setCastShadows( true );
}

//------------------------------------------------------------------------------
void Sample_NewInstancing::switchInstancingTechnique()
{
	randGenerator.randomize();
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
		/**if( mSetStatic->isChecked() )
			mCurrentManager->setBatchesAsStaticAndUpdate( mSetStatic->isChecked() );*/
		mSetStatic->show();
	}
	else
		mSetStatic->hide();
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
		mAnimationsLegacy.push_back( anim );
		anim->setEnabled( true );
		anim->addTime( randGenerator.nextFloat() * 10 ); //Random start offset
	}
}
//------------------------------------------------------------------------------
void Sample_NewInstancing::createInstancedEntities()
{
	SceneMemoryMgrTypes sceneMemoryMgrType = mSetStatic->isChecked() ? SCENE_STATIC : SCENE_DYNAMIC;
	for( int i=0; i<NUM_INST_ROW; ++i )
	{
		for( int j=0; j<NUM_INST_COLUMN; ++j )
		{
			//Create the instanced entity
			InstancedEntity *ent = mCurrentManager->createInstancedEntity(
										mCurrentMaterialSet[mInstancingTechnique], sceneMemoryMgrType );
			mEntities.push_back( ent );

			//HWInstancingBasic is the only technique without animation support
			if( mInstancingTechnique != InstanceManager::HWInstancingBasic)
			{
#ifndef OGRE_LEGACY_ANIMATIONS
				//Get the animation
				SkeletonInstance *skeletonInstance = ent->getSkeleton();
				SkeletonAnimation *anim = skeletonInstance->getAnimation( "Walk" );

				//Some techniques (i.e. LUT) may share the animations,
				//so ensure we add the pointer only once
				if( std::find( mAnimations.begin(), mAnimations.end(), anim ) == mAnimations.end() )
				{
					anim->setEnabled( true );
					anim->addTime(randGenerator.nextFloat() * 10); //Random start offset
					mAnimations.push_back( anim );
				}
#else
				//Get the animation
				AnimationState *anim = ent->getAnimationState( "Walk" );
				anim->setEnabled( true );
				anim->addTime( randGenerator.nextFloat() * 10); //Random start offset
				mAnimationsLegacy.push_back( anim );
#endif
			}
		}
	}
}
//------------------------------------------------------------------------------
void Sample_NewInstancing::createSceneNodes()
{
	//Here the SceneNodes are created. Since InstancedEntities derive from MovableObject,
	//they behave like regular Entities on this.
	SceneMemoryMgrTypes sceneMemoryMgrType = mSetStatic->isChecked() ? SCENE_STATIC : SCENE_DYNAMIC;
	SceneNode *rootNode = mSceneMgr->getRootSceneNode( sceneMemoryMgrType );

	for( int i=0; i<NUM_INST_ROW; ++i )
	{
		for( int j=0; j<NUM_INST_COLUMN; ++j )
		{
			int idx = i * NUM_INST_COLUMN + j;
			SceneNode *sceneNode = rootNode->createChildSceneNode( sceneMemoryMgrType );
			sceneNode->attachObject( mEntities[idx] );
			sceneNode->yaw( Radian( randGenerator.nextFloat() * 10 * 3.14159265359f )); //Random orientation
			sceneNode->setPosition( mEntities[idx]->getWorldRadiusUpdated() * (i - NUM_INST_ROW * 0.5f), 0,
									mEntities[idx]->getWorldRadiusUpdated() * (j - NUM_INST_COLUMN * 0.5f) );
			mSceneNodes.push_back( sceneNode );
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
			sceneNode->getParentSceneNode()->removeAndDestroyChild( sceneNode );

		if( mInstancingTechnique == NUM_TECHNIQUES )
			mSceneMgr->destroyEntity( static_cast<Entity*>(*itor) );
		else
			mSceneMgr->destroyInstancedEntity( static_cast<InstancedEntity*>(*itor) );

		++itor;
	}

	//Free some memory, but don't destroy the manager so when we switch this technique
	//back again it doesn't take too long
	if( mCurrentManager )
		mCurrentManager->cleanupEmptyBatches();

	mEntities.clear();
	mSceneNodes.clear();
	mAnimationsLegacy.clear();
#ifndef OGRE_LEGACY_ANIMATIONS
	mAnimations.clear();
#endif
}
//-----------------------------------------------------------------------------------
void Sample_NewInstancing::destroyManagers()
{
	mSceneMgr->destroyInstanceManager(mCurrentManager);
}

//------------------------------------------------------------------------------
void Sample_NewInstancing::cleanupContent()
{
    MeshManager::getSingleton().remove("ground");
	clearScene();
	destroyManagers();
}

//------------------------------------------------------------------------------
void Sample_NewInstancing::animateUnits( float timeSinceLast )
{
	{
		//Iterates through all AnimationSets and updates the animation being played. Demonstrates the
		//animation is unique and independent to each instance
		std::vector<AnimationState*>::const_iterator itor = mAnimationsLegacy.begin();
		std::vector<AnimationState*>::const_iterator end  = mAnimationsLegacy.end();

		while( itor != end )
		{
			(*itor)->addTime( timeSinceLast );
			++itor;
		}
	}

#ifndef OGRE_LEGACY_ANIMATIONS
	{
		std::vector<SkeletonAnimation*>::const_iterator itor = mAnimations.begin();
		std::vector<SkeletonAnimation*>::const_iterator end  = mAnimations.end();

		while( itor != end )
		{
			(*itor)->addTime( timeSinceLast );
			++itor;
		}
	}
#endif
}

//------------------------------------------------------------------------------
void Sample_NewInstancing::moveUnits( float timeSinceLast )
{
	Real fMovSpeed = 1.0f;

	if( !mEntities.empty() )
		fMovSpeed = mEntities[0]->getWorldRadius() * 0.30f;

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
			(*itor)->setOrientation( lookAt( planeNormal.reflect( vDir ).normalisedCopy() ) );
			(*itor)->setPosition( entityPos );
		}

		//Move along the direction we're looking to
		(*itor)->translate( Vector3::UNIT_X * timeSinceLast * fMovSpeed, Node::TS_LOCAL );
		++itor;
	}
}

//------------------------------------------------------------------------------
Quaternion Sample_NewInstancing::lookAt( const Vector3 &normDir )
{
	Quaternion retVal;
	Vector3 xVec = Vector3::UNIT_Y.crossProduct( normDir );
	xVec.normalise();

	Vector3 yVec = normDir.crossProduct( xVec );
	yVec.normalise();

	retVal.FromAxes( xVec, yVec, normDir );

	return retVal;
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
		const IdString workspaceName( "NewInstancingWorkspace" );
		CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
		compositorManager->removeAllWorkspaces();
		compositorManager->removeAllWorkspaceDefinitions();
		compositorManager->removeAllNodeDefinitions();
		compositorManager->createBasicWorkspaceDef( workspaceName, ColourValue( 0.6f, 0.0f, 0.6f ),
													mEnableShadows->isChecked() ?
														"NewInstancing Shadows" : IdString() );
		compositorManager->addWorkspace( mSceneMgr, mWindow, mCamera, workspaceName, true );
	}
	else if( box == mSetStatic && mCurrentManager )
	{
		clearScene();
		switchInstancingTechnique();
		//mCurrentManager->setBatchesAsStaticAndUpdate( mSetStatic->isChecked() );
	}
}

//------------------------------------------------------------------------------
void Sample_NewInstancing::sliderMoved( Slider* slider )
{
	if( slider == mInstancesSlider ) NUM_INST_ROW = static_cast<int>(mInstancesSlider->getValue());
	NUM_INST_COLUMN = static_cast<int>(mInstancesSlider->getValue());
}

//------------------------------------------------------------------------------
void Sample_NewInstancing::testCapabilities( const RenderSystemCapabilities* caps )
{
	if (!caps->hasCapability(RSC_VERTEX_PROGRAM) || !caps->hasCapability(RSC_FRAGMENT_PROGRAM))
	{
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your graphics card does not support vertex and "
			"fragment programs, so you cannot run this sample. Sorry!",
			"NewInstancing::testCapabilities");
	}

	if (!GpuProgramManager::getSingleton().isSyntaxSupported("glsl") &&
#if OGRE_NO_GLES3_SUPPORT == 0
		!GpuProgramManager::getSingleton().isSyntaxSupported("glsles") &&
#endif
		!GpuProgramManager::getSingleton().isSyntaxSupported("fp40") &&
		!GpuProgramManager::getSingleton().isSyntaxSupported("ps_2_0") &&
		!GpuProgramManager::getSingleton().isSyntaxSupported("ps_3_0") &&
		!GpuProgramManager::getSingleton().isSyntaxSupported("ps_4_0") &&
		!GpuProgramManager::getSingleton().isSyntaxSupported("ps_4_1") &&
		!GpuProgramManager::getSingleton().isSyntaxSupported("ps_5_0"))
	{
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your card does not support the shader model needed for this sample, "
                    "so you cannot run this sample. Sorry!", "NewInstancing::testCapabilities");
	}
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
