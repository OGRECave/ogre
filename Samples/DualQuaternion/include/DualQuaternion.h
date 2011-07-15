#ifndef __DualQuaternion_Sample_H__
#define __DualQuaternion_Sample_H__

#include "SdkSample.h"
#include "OgreDualQuaternion.h"
#include "OgreShaderExHardwareSkinning.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_DualQuaternion : public SdkSample
{
public:
	Sample_DualQuaternion() : ent(0), entDQ(0), accumulatedAngle(0), switchPodality(false)
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
		, mSrsHardwareSkinning(0)
#endif
	{
		mInfo["Title"] = "Dual Quaternion Skinning";
		mInfo["Description"] = "A demo of the dual quaternion skinning feature, including instancing support";
		mInfo["Thumbnail"] = "thumb_skelanim.png";
		mInfo["Category"] = "Animation";
	}

	bool frameRenderingQueued(const FrameEvent& evt)
	{
		Radian angle = Radian(2 * Math::PI * evt.timeSinceLastFrame);
				
		if(switchPodality)
		{
			angle = -angle;
		}
		
		accumulatedAngle += angle;
		Radian maxAngle = Radian(Math::PI - 0.2);
		
		if(accumulatedAngle > maxAngle)
		{
			switchPodality = true;
			angle -= accumulatedAngle - maxAngle;
			accumulatedAngle = maxAngle;
		}
		else if(accumulatedAngle < -maxAngle)
		{
			switchPodality = false;
			angle -= accumulatedAngle + maxAngle;
			accumulatedAngle = -maxAngle;
		}		
		
		Ogre::Bone* bone = entDQ->getSkeleton()->getBone(0);
		bone->rotate(Vector3::UNIT_X, angle / 2);

		bone = entDQ->getSkeleton()->getBone(1);
		bone->rotate(Vector3::UNIT_X, -angle);

		bone = ent->getSkeleton()->getBone(0);
		bone->rotate(Vector3::UNIT_X, angle / 2);

		bone = ent->getSkeleton()->getBone(1);
		bone->rotate(Vector3::UNIT_X, -angle);
		
		return SdkSample::frameRenderingQueued(evt);
	}


protected:

	void setupContent()
	{
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
		//To make glsles work the program will need to be provided with proper
		//shadow caster materials
		if (mShaderGenerator->getTargetLanguage() != "glsles")
		{
			//Add the hardware skinning to the shader generator default render state
			mSrsHardwareSkinning = mShaderGenerator->createSubRenderState(Ogre::RTShader::HardwareSkinning::Type);
			Ogre::RTShader::RenderState* renderState = mShaderGenerator->getRenderState(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
			renderState->addTemplateSubRenderState(mSrsHardwareSkinning);

			Ogre::MaterialPtr pCast1 = Ogre::MaterialManager::getSingleton().getByName("Ogre/RTShader/shadow_caster_dq_skinning_1weight_twophase");
			Ogre::MaterialPtr pCast2 = Ogre::MaterialManager::getSingleton().getByName("Ogre/RTShader/shadow_caster_dq_skinning_2weight_twophase");
			Ogre::MaterialPtr pCast3 = Ogre::MaterialManager::getSingleton().getByName("Ogre/RTShader/shadow_caster_dq_skinning_3weight_twophase");
			Ogre::MaterialPtr pCast4 = Ogre::MaterialManager::getSingleton().getByName("Ogre/RTShader/shadow_caster_dq_skinning_4weight_twophase");

			Ogre::RTShader::HardwareSkinningFactory::getSingleton().setCustomShadowCasterMaterials(RTShader::ST_DUAL_QUATERNION, pCast1, pCast2, pCast3, pCast4);

			Ogre::MaterialPtr pCast1l = Ogre::MaterialManager::getSingleton().getByName("Ogre/RTShader/shadow_caster_skinning_1weight");
			Ogre::MaterialPtr pCast2l = Ogre::MaterialManager::getSingleton().getByName("Ogre/RTShader/shadow_caster_skinning_2weight");
			Ogre::MaterialPtr pCast3l = Ogre::MaterialManager::getSingleton().getByName("Ogre/RTShader/shadow_caster_skinning_3weight");
			Ogre::MaterialPtr pCast4l = Ogre::MaterialManager::getSingleton().getByName("Ogre/RTShader/shadow_caster_skinning_4weight");

			Ogre::RTShader::HardwareSkinningFactory::getSingleton().setCustomShadowCasterMaterials(RTShader::ST_LINEAR, pCast1l, pCast2l, pCast3l, pCast4l);
		}
#endif
		// set shadow properties
		mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
		mSceneMgr->setShadowTextureSize(2048);
		mSceneMgr->setShadowColour(ColourValue(0.6, 0.6, 0.6));
		mSceneMgr->setShadowTextureCount(1);

		// add a little ambient lighting
		mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));

		SceneNode* lightsBbsNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		BillboardSet* bbs;

		// Create billboard set for lights .
		bbs = mSceneMgr->createBillboardSet();
		bbs->setMaterialName("Examples/Flare");
		lightsBbsNode->attachObject(bbs);

 		Light* l = mSceneMgr->createLight();
		Vector3 dir;
 		l->setType(Light::LT_SPOTLIGHT);
 		l->setPosition(30, 70, 0);
 		dir = -l->getPosition();
 		dir.normalise();
 		l->setDirection(dir);
 		l->setDiffuseColour(1, 1, 1);
 		bbs->createBillboard(l->getPosition())->setColour(l->getDiffuseColour());

		// create a floor mesh resource
		MeshManager::getSingleton().createPlane("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			Plane(Vector3::UNIT_Y, -1), 250, 250, 25, 25, true, 1, 15, 15, Vector3::UNIT_Z);

		// add a floor to our scene using the floor mesh we created
		Entity* floor = mSceneMgr->createEntity("Floor", "floor");
		floor->setMaterialName("Examples/Rockwall");
		floor->setCastShadows(false);
		mSceneMgr->getRootSceneNode()->attachObject(floor);

		// set camera initial transform and speed
		mCamera->setPosition(100, 20, 0);
		mCamera->lookAt(0, 10, 0);
		mCameraMan->setTopSpeed(50);

		setupModels();
	}

	void setupModels()
	{
		SceneNode* sn = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		sn->translate(0, 0, 20, Node::TS_LOCAL);

		//Create and attach a spine entity with standard skinning
		ent = mSceneMgr->createEntity("Spine", "spine.mesh");
		ent->setMaterialName("spine");
		manuallyControlBones(ent);
		sn->attachObject(ent);
		sn->scale(Vector3(20,20,20));

		sn = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		sn->translate(0, 0, -20, Node::TS_LOCAL);

		//Create and attach a spine entity with dual quaternion skinning
		entDQ = mSceneMgr->createEntity("SpineDQ", "spine.mesh");
		entDQ->setMaterialName("spineDualQuat");
		manuallyControlBones(entDQ);
		sn->attachObject(entDQ);
		sn->scale(Vector3(20,20,20));
		
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
		//To make glsles work the program will need to be provided with proper
		//shadow caster materials
		if (mShaderGenerator->getTargetLanguage() != "glsles")
		{
			//In case the system uses the RTSS, the following line will ensure
			//that the entity is using hardware animation in RTSS as well.
			RTShader::HardwareSkinningFactory::getSingleton().prepareEntityForSkinning(ent);
			RTShader::HardwareSkinningFactory::getSingleton().prepareEntityForSkinning(entDQ, RTShader::ST_DUAL_QUATERNION, false, true);

			//The following line is needed only because the Jaiqua model material has shaders and
			//as such is not automatically reflected in the RTSS system
			//RTShader::ShaderGenerator::getSingleton().createShaderBasedTechnique(
			//	ent->getSubEntity(0)->getMaterialName(),
			//	Ogre::MaterialManager::DEFAULT_SCHEME_NAME,
			//	Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME,
			//	true);
		}
#endif

		// create name and value for skinning mode
		StringVector names;
		names.push_back("Skinning");
		String value = "Software";

		// change the value if hardware skinning is enabled
		Pass* pass = entDQ->getSubEntity(0)->getMaterial()->getBestTechnique()->getPass(0);
		if (pass && pass->hasVertexProgram() && pass->getVertexProgram()->isSkeletalAnimationIncluded()) value = "Hardware";

		// create a params panel to display the skinning mode
		mTrayMgr->createParamsPanel(TL_TOPLEFT, "Skinning", 150, names)->setParamValue(0, value);
	}

	void manuallyControlBones(Entity* ent)
	{
		for(Skeleton::BoneIterator it = ent->getSkeleton()->getBoneIterator(); it.hasMoreElements(); it.moveNext())
		{
			(*it.current())->setManuallyControlled(true);
		}
	}

	void cleanupContent()
	{
		MeshManager::getSingleton().remove("floor");

#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
		//To make glsles work the program will need to be provided with proper
		//shadow caster materials
		if (mShaderGenerator->getTargetLanguage() != "glsles")
		{
			Ogre::RTShader::RenderState* renderState = mShaderGenerator->getRenderState(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
			renderState->removeTemplateSubRenderState(mSrsHardwareSkinning);
		}
#endif
	}

	Entity* ent;
	Entity* entDQ;
	Radian accumulatedAngle;
	bool switchPodality;

#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
	RTShader::SubRenderState* mSrsHardwareSkinning;
#endif
};

#endif
