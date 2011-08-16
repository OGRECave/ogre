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

#define NUM_TECHNIQUES (((int)InstanceManager::InstancingTechniquesCount) + 1)

 
#define mtsz 624
#define mtszsp 397
 
//Random number generator. Written by FrozenKnight
//Taken from http://www.cpplc.net/forum/index.php?topic=2862.0

#define MTSZ 624
#define MTSZSP 397

class MersenneTwister {
private:
	int seed[MTSZ];
	int index;
 
	void generate()
	{
		for (int i = 0; i < MTSZ; i++) 
		{
			int y = (((seed[i] << 31) & 0x80000000) + (seed[(i+1) % MTSZ] & 0x7FFFFFFF)) >> 1;
			seed[i] = seed[(i + MTSZSP) % MTSZ] ^ y;
			seed[i] ^= (y&1)? 0x9908b0df: 0;
		}
	} 
public:
	MersenneTwister()
	{
		index = 0;
		randomize();
	}

	void randomize(int seedVal = 0x12345678)
	{
		seed[0] = seedVal;
		for (int i = 1; i < MTSZ; ++i)
			seed[i] = (int)(0x6c078965*((this->seed[i-1] >> 30)+i));
	}

	unsigned int nextUInt()
	{
		unsigned int ret;
		if (index == 0)
			generate();
		ret = seed[index];

		ret ^= ret >> 11;
		ret ^= (ret << 7) & 0x9d2c5680;
		ret ^= (ret << 15) & 0xefc60000;
		ret ^= ret >> 18;

		index = (++index < mtsz)? index: 0;

		return ret;
	}

	float nextFloat()
	{
		return (nextUInt() / (float)0xFFFFFFFF);
	}
};
  

class _OgreSampleClassExport Sample_NewInstancing : public SdkSample
{
public:

	Sample_NewInstancing();

	bool frameRenderingQueued(const FrameEvent& evt);

	bool keyPressed(const OIS::KeyEvent& evt);
	

protected:
	void setupContent();

	void setupLighting();
	
	void switchInstancingTechnique();

	void switchSkinningTechnique(int index);

	void createEntities();

	void createInstancedEntities();

	void createSceneNodes();
	
	void clearScene();

	void destroyManagers();

	void cleanupContent();

	void animateUnits( float timeSinceLast );

	void moveUnits( float timeSinceLast );

	//Helper function to look towards normDir, where this vector is normalized, with fixed Yaw
	Quaternion lookAt( const Vector3 &normDir );

	void defragmentBatches();

	void setupGUI();

	void itemSelected(SelectMenu* menu);

	void buttonHit( OgreBites::Button* button );

	void checkBoxToggled(CheckBox* box);

	void sliderMoved(Slider* slider);

	void testCapabilities(const RenderSystemCapabilities* caps);

	//The difference between testCapabilities() is that features checked here aren't fatal errors.
	//which means the sample can run (with limited functionality) on those computers
	void checkHardwareSupport();

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
	std::vector<InstancedEntity*>	mMovedInstances;
	std::vector<SceneNode*>			mSceneNodes;
	std::set<AnimationState*>		mAnimations;
	InstanceManager					*mCurrentManager;
	bool							mSupportedTechniques[NUM_TECHNIQUES+1];
	const char**						mCurrentMaterialSet;
	uint16 							mCurrentFlags;

	SelectMenu						*mTechniqueMenu;
	CheckBox						*mMoveInstances;
	CheckBox						*mAnimateInstances;
	SelectMenu						*mSkinningTechniques;
	CheckBox						*mEnableShadows;
	CheckBox						*mSetStatic;
	CheckBox						*mUseSceneNodes;
	OgreBites::Button					*mDefragmentBatches;
	CheckBox						*mDefragmentOptimumCull;
	Slider							*mInstancesSlider;

	MersenneTwister randGenerator;
};

#endif
