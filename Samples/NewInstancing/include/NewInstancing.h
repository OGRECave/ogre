#ifndef __NewInstancing_H__
#define __NewInstancing_H__

#include "SdkSample.h"
#include "OgreInstancedEntity.h"

using namespace Ogre;
using namespace OgreBites;

static const char *c_instancingTechniques[] =
{
	"Shader Based",
	"Vertex Texture Fetch (VTF)",
	"Hardware Instancing",
	"No Instancing"
};

static const char *c_materialsTechniques[] =
{
	"Examples/Instancing/ShaderBased/Robot",
	"Examples/Instancing/VTF/Robot",
	"Examples/Instancing/VTF/Robot",
	"Examples/Instancing/ShaderBased/Robot"
};

static const char *c_meshNames[] =
{
	"robot.mesh"
};

#define NUM_TECHNIQUES ((int)InstanceManager::InstancingTechniquesCount)

class _OgreSampleClassExport Sample_NewInstancing : public SdkSample
{
public:

	Sample_NewInstancing() : NUM_INST_ROW(50), NUM_INST_COLUMN(50)
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

    bool frameRenderingQueued(const FrameEvent& evt)
    {
		if( mAnimateInstances->isChecked() )
			animateUnits( evt.timeSinceLastEvent );

		if( mMoveInstances->isChecked() )
			moveUnits( evt.timeSinceLastEvent );

		return SdkSample::frameRenderingQueued(evt);        // don't forget the parent class updates!
    }

	bool keyPressed(const OIS::KeyEvent& evt)
	{
		//Toggle bounding boxes with B key unless the help dialog is visible
		if (evt.key == OIS::KC_B && !mTrayMgr->isDialogVisible() && mCurrentManager)
			mCurrentManager->showBoundingBoxes( !mCurrentManager->getShowBoundingBoxes() );

		//Switch to next instancing technique with space bar
		if (evt.key == OIS::KC_SPACE && !mTrayMgr->isDialogVisible())
			mTechniqueMenu->selectItem( (mTechniqueMenu->getSelectionIndex() + 1) % (NUM_TECHNIQUES+1) );

		return SdkSample::keyPressed(evt);
	}

protected:
	void setupContent()
	{
		//Initialize the techniques and current mesh variables
		mInstancingTechnique	= 0;
		mCurrentMesh			= 0;
		mCurrentManager			= 0;
		for( int i=0; i<NUM_TECHNIQUES; ++i )
			mInstanceManagers[i] = 0;

		checkHardwareSupport();

		mSceneMgr->setShadowTechnique( SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED );
		mSceneMgr->setShadowTextureConfig( 0, 2048, 2048, PF_FLOAT32_R );
		mSceneMgr->setShadowTextureSelfShadow( true );
		mSceneMgr->setShadowCasterRenderBackFaces( true );

		//LiSPSMShadowCameraSetup *shadowCameraSetup = new LiSPSMShadowCameraSetup();
		FocusedShadowCameraSetup *shadowCameraSetup = new FocusedShadowCameraSetup();
		//PlaneOptimalShadowCameraSetup *shadowCameraSetup = new PlaneOptimalShadowCameraSetup();
		
		mSceneMgr->setShadowCameraSetup( ShadowCameraSetupPtr(shadowCameraSetup) );

		mEntities.reserve( NUM_INST_ROW * NUM_INST_COLUMN );
		mSceneNodes.reserve( NUM_INST_ROW * NUM_INST_COLUMN );
		mAnimations.reserve( NUM_INST_ROW * NUM_INST_COLUMN );

		mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");

		// create a mesh for our ground
		MeshManager::getSingleton().createPlane("ground", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			Plane(Vector3::UNIT_Y, 0), 1000, 1000, 20, 20, true, 1, 6, 6, Vector3::UNIT_Z);
		
		// create a ground entity from our mesh and attach it to the origin
		Entity* ground = mSceneMgr->createEntity("Ground", "ground");
		ground->setMaterialName("Examples/Instancing/Misc/Grass");
		ground->setCastShadows(false);
		mSceneMgr->getRootSceneNode()->attachObject(ground);

		setupLighting();

		// set initial camera position and speed
		mCamera->setPosition( 0, 50, 100 );

		setupGUI();

#if OGRE_PLATFORM != OGRE_PLATFORM_IPHONE
		setDragLook(true);
#endif

		switchInstancingTechnique();
	}

	void setupLighting()
	{
		mSceneMgr->setAmbientLight( ColourValue( 0.40f, 0.40f, 0.40f ) );

		ColourValue lightColour( 1, 0.5, 0.3 );

		//Create main (point) light
		Light* light = mSceneMgr->createLight();
		light->setDiffuseColour(lightColour);
		light->setPosition( 0.0f, 25.0f, 0.0f );
		light->setSpecularColour( 0.6, 0.82, 1.0 );
		light->setAttenuation( 3500, 0.085, 0.00008, 0.00006 );
		light->setCastShadows( false );

		//Create a dummy spot light for shadows
		light = mSceneMgr->createLight();
		light->setType( Light::LT_SPOTLIGHT );
		light->setDiffuseColour( ColourValue( 0.15f, 0.35f, 0.44f ) );
		light->setPosition( 250.0f, 200.0f, 250.0f );
		light->setDirection( (Vector3::UNIT_SCALE * -1.0f).normalisedCopy() );
		light->setSpecularColour( 0.2, 0.12, 0.11 );
		light->setAttenuation( 3500, 0.005, 0.00002, 0.00001 );
		light->setSpotlightRange( Degree(80), Degree(90) );
		light->setCastShadows( true );
		light->setLightMask( 0x00000000 );
	}

	void switchInstancingTechnique()
	{
		//mInstancingTechnique = (mInstancingTechnique+1) % (NUM_TECHNIQUES+1);
		mInstancingTechnique = mTechniqueMenu->getSelectionIndex();

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
			if( !mInstanceManagers[mInstancingTechnique] )
			{
				InstanceManager::InstancingTechnique technique;
				switch( mInstancingTechnique )
				{
				case 0: technique = InstanceManager::ShaderBased; break;
				case 1: technique = InstanceManager::TextureVTF; break;
				case 2: technique = InstanceManager::HardwareInstancing; break;
				}

				mInstanceManagers[mInstancingTechnique] = mSceneMgr->createInstanceManager(
				"InstanceMgr" + StringConverter::toString(mInstancingTechnique), c_meshNames[mCurrentMesh],
				ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, technique,
				NUM_INST_ROW * NUM_INST_COLUMN, IM_USEALL );
			}

			mCurrentManager = mInstanceManagers[mInstancingTechnique];

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
	}

	void createEntities()
	{
		for( int i=0; i<NUM_INST_ROW * NUM_INST_COLUMN; ++i )
		{
			//Create the non-instanced entity. Use the same shader as shader-based because:
			//a. To prove we can (runs without modification! :-) )
			//b. Make a fair comparision
			Entity *ent = mSceneMgr->createEntity( c_meshNames[mCurrentMesh] );
			ent->setMaterialName( c_materialsTechniques[NUM_TECHNIQUES] );
			mEntities.push_back( ent );

			//Get the animation
			AnimationState *anim = ent->getAnimationState( "Walk" );
			anim->setEnabled( true );
			anim->addTime( i * i * i * 0.001f ); //Random start offset
			mAnimations.push_back( anim );
		}
	}

	void createInstancedEntities()
	{
		for( int i=0; i<NUM_INST_ROW * NUM_INST_COLUMN; ++i )
		{
			//Create the instanced entity
			InstancedEntity *ent = mCurrentManager->createInstancedEntity(
												c_materialsTechniques[mInstancingTechnique] );
			mEntities.push_back( ent );

			//Get the animation
			AnimationState *anim = ent->getAnimationState( "Walk" );
			anim->setEnabled( true );
			anim->addTime( i * i * i * 0.001f  ); //Random start offset
			mAnimations.push_back( anim );
		}
	}

	void createSceneNodes()
	{
		//Here the SceneNodes are created. Since InstancedEntities derive from MovableObject,
		//they behave like regular Entities on this.
		SceneNode *rootNode = mSceneMgr->getRootSceneNode();

		for( int i=0; i<NUM_INST_ROW; ++i )
		{
			for( int j=0; j<NUM_INST_COLUMN; ++j )
			{
				int idx = i * NUM_INST_COLUMN + j;
				SceneNode *sceneNode = rootNode->createChildSceneNode();
				sceneNode->attachObject( mEntities[idx] );
				sceneNode->setScale( Vector3( 0.1f ) );
				sceneNode->yaw( Radian( (i+1) * (j+1) * (i+1) * (j+1) ) ); //Random orientation
				sceneNode->setPosition( mEntities[idx]->getBoundingRadius() * (i - NUM_INST_ROW * 0.5f), 0,
										mEntities[idx]->getBoundingRadius() * (j - NUM_INST_COLUMN * 0.5f) );

				mSceneNodes.push_back( sceneNode );
			}
		}
	}

	void clearScene()
	{
		std::vector<MovableObject*>::const_iterator itor = mEntities.begin();
		std::vector<MovableObject*>::const_iterator end  = mEntities.end();

		//Note: Destroying the instance manager automatically destroys all instanced entities
		//created by this manager (beware of not leaving reference to those pointers)
		while( itor != end )
		{
			SceneNode *sceneNode = (*itor)->getParentSceneNode();
			sceneNode->detachAllObjects();
			sceneNode->getParentSceneNode()->removeAndDestroyChild( sceneNode->getName() );

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
		mSceneNodes.clear();
		mAnimations.clear();
	}

	void destroyManagers()
	{
		for( int i=0; i<NUM_TECHNIQUES; ++i )
		{
			if( mInstanceManagers[i] )
			{
				mSceneMgr->destroyInstanceManager( mInstanceManagers[i] );
				mInstanceManagers[i] = 0;
			}
		}
	}

	void cleanupContent()
	{
		clearScene();
		destroyManagers();
	}

	void animateUnits( float timeSinceLast )
	{
		//Iterates through all AnimationSets and updates the animation being played. Demonstrates the
		//animation is unique and independent to each instance
		std::vector<AnimationState*>::const_iterator itor = mAnimations.begin();
		std::vector<AnimationState*>::const_iterator end  = mAnimations.end();

		while( itor != end )
		{
			(*itor)->addTime( timeSinceLast );
			++itor;
		}
	}

	void moveUnits( float timeSinceLast )
	{
		Real fMovSpeed = 1.0f;
		
		if( !mEntities.empty() )
			fMovSpeed = mEntities[0]->getBoundingRadius() * 0.30f;

		//Randomly move the units along their normal, bouncing around invisible walls
		std::vector<SceneNode*>::const_iterator itor = mSceneNodes.begin();
		std::vector<SceneNode*>::const_iterator end  = mSceneNodes.end();

		while( itor != end )
		{
			//Calculate bounces
			Vector3 entityPos = (*itor)->getPosition();
			Vector3 planeNormal = Vector3::ZERO;
			if( (*itor)->getPosition().x < -500.0f )
			{
				planeNormal = Vector3::UNIT_X;
				entityPos.x = -499.0f;
			}
			else if( (*itor)->getPosition().x > 500.0f )
			{
				planeNormal = Vector3::NEGATIVE_UNIT_X;
				entityPos.x = 499.0f;
			}
			else if( (*itor)->getPosition().z < -500.0f )
			{
				planeNormal = Vector3::UNIT_Z;
				entityPos.z = -499.0f;
			}
			else if( (*itor)->getPosition().z > 500.0f )
			{
				planeNormal = Vector3::NEGATIVE_UNIT_Z;
				entityPos.z = 499.0f;
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

	//Helper function to look towards normDir, where this vector is normalized, with fixed Yaw
	Quaternion lookAt( const Vector3 &normDir )
	{
		Quaternion retVal;
		Vector3 xVec = Vector3::UNIT_Y.crossProduct( normDir );
        xVec.normalise();

        Vector3 yVec = normDir.crossProduct( xVec );
        yVec.normalise();

        retVal.FromAxes( xVec, yVec, normDir );

		return retVal;
	}

	void defragmentBatches()
	{
		//Defragment batches is used after many InstancedEntities were removed (and you won't
		//be requesting more). However, then the optimize cull option is on, it can cause
		//quite a perf. boost on large batches (i.e. VTF) even if not a single instance was ever removed.
		if( mCurrentManager )
			mCurrentManager->defragmentBatches( mDefragmentOptimumCull->isChecked() );
	}

	void setupGUI()
	{
		mTechniqueMenu = mTrayMgr->createLongSelectMenu(
			TL_TOPLEFT, "TechniqueSelectMenu", "Technique", 300, 200, 5);
		for( int i=0; i<NUM_TECHNIQUES+1; ++i )
		{
			String text = c_instancingTechniques[i];
			if( !mSupportedTechniques[i] )
				text = "Unsupported: " + text;
			mTechniqueMenu->addItem( text );
		}

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

		//Controls to control batch defragmentation on the fly
		mDefragmentBatches =  mTrayMgr->createButton(TL_TOP, "DefragmentBatches",
															"Defragment Batches", 175);
		mDefragmentOptimumCull = mTrayMgr->createCheckBox(TL_TOP, "DefragmentOptimumCull",
															"Optimum Cull", 175);
		mDefragmentOptimumCull->setChecked(true);

		//Slider to control max number of instances
		mInstancesSlider = mTrayMgr->createThickSlider( TL_TOPLEFT, "InstancesSlider", "Instances (NxN)",
														300, 50, 4, 100, 97 );
		mInstancesSlider->setValue( NUM_INST_ROW );

		mTrayMgr->showCursor();
	}

	void itemSelected(SelectMenu* menu)
	{
		if (menu == mTechniqueMenu)
		{
			clearScene();
			switchInstancingTechnique();
		}
	}

	void buttonHit( OgreBites::Button* button )
	{
		if( button == mDefragmentBatches ) defragmentBatches();
	}

	void checkBoxToggled(CheckBox* box)
	{
		if( box == mEnableShadows ) mSceneMgr->setShadowTechnique( mEnableShadows->isChecked() ?
									SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED : SHADOWTYPE_NONE );
	}

	void sliderMoved(Slider* slider)
	{
		if( slider == mInstancesSlider ) NUM_INST_ROW = static_cast<int>(mInstancesSlider->getValue());
										 NUM_INST_COLUMN = static_cast<int>(mInstancesSlider->getValue());
	}

	void testCapabilities(const RenderSystemCapabilities* caps)
	{
		if (!caps->hasCapability(RSC_VERTEX_PROGRAM) || !caps->hasCapability(RSC_FRAGMENT_PROGRAM))
        {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your graphics card does not support vertex and "
				"fragment programs, so you cannot run this sample. Sorry!",
				"NewInstancing::testCapabilities");
        }

        if (!GpuProgramManager::getSingleton().isSyntaxSupported("glsl") &&
			!GpuProgramManager::getSingleton().isSyntaxSupported("fp40") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("ps_2_0") &&
			!GpuProgramManager::getSingleton().isSyntaxSupported("ps_3_0") )
        {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "This sample needs at least Shader Model 2.0+ or "
				"GLSL to work. Your GPU is too old. Sorry!", "NewInstancing::testCapabilities");
        }
	}

	//The difference between testCapabilities() is that features checked here aren't fatal errors.
	//which means the sample can run (with limited functionality) on those computers
	void checkHardwareSupport()
	{
		//Check Technique support
		for( int i=0; i<NUM_TECHNIQUES; ++i )
		{
			InstanceManager::InstancingTechnique technique;
			switch( i )
			{
			case 0: technique = InstanceManager::ShaderBased; break;
			case 1: technique = InstanceManager::TextureVTF; break;
			case 2: technique = InstanceManager::HardwareInstancing; break;
			}

			const size_t numInstances = mSceneMgr->getNumInstancesPerBatch( c_meshNames[mCurrentMesh],
									ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									c_materialsTechniques[i], technique, NUM_INST_ROW * NUM_INST_COLUMN,
									IM_USEALL );
			
			mSupportedTechniques[i] = numInstances > 0;

			if ( (technique == InstanceManager::HardwareInstancing) && (mSupportedTechniques[i]) )
			{
				RenderSystem* rs = Root::getSingleton().getRenderSystem();
				mSupportedTechniques[i] = rs->getCapabilities()->hasCapability(RSC_VERTEX_BUFFER_INSTANCE_DATA);
			}
		}

		//Non instancing is always supported
		mSupportedTechniques[NUM_TECHNIQUES] = true;
	}

	//You can also use a union type to switch between Entity and InstancedEntity almost flawlessly:
	/*
	union FusionEntity
	{
		Entity			entity
		InstancedEntity	instancedEntity;
	};
	*/
	int NUM_INST_ROW;
	int NUM_INST_COLUMN;
	int								mInstancingTechnique;
	int								mCurrentMesh;
	std::vector<MovableObject*>		mEntities;
	std::vector<SceneNode*>			mSceneNodes;
	std::vector<AnimationState*>	mAnimations;
	InstanceManager					*mInstanceManagers[NUM_TECHNIQUES];
	InstanceManager					*mCurrentManager;
	bool							mSupportedTechniques[NUM_TECHNIQUES+1];

	SelectMenu						*mTechniqueMenu;
	CheckBox						*mMoveInstances;
	CheckBox						*mAnimateInstances;
	CheckBox						*mEnableShadows;
	OgreBites::Button							*mDefragmentBatches;
	CheckBox						*mDefragmentOptimumCull;
	Slider							*mInstancesSlider;
};

#endif
