#ifndef __SkeletalAnimation_H__
#define __SkeletalAnimation_H__

#include "SdkSample.h"

#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#include "OgreShaderExHardwareSkinning.h"
#endif

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_SkeletalAnimation : public SdkSample
{
public:
	Sample_SkeletalAnimation() : NUM_MODELS(6), ANIM_CHOP(8)
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
		, mSrsHardwareSkinning(0)
#endif
	{
		mInfo["Title"] = "Skeletal Animation";
		mInfo["Description"] = "A demo of the skeletal animation feature, including spline animation.";
		mInfo["Thumbnail"] = "thumb_skelanim.png";
		mInfo["Category"] = "Animation";
	}

    bool frameRenderingQueued(const FrameEvent& evt)
    {
        for (unsigned int i = 0; i < NUM_MODELS; i++)
        {
			// update sneaking animation based on speed
			mAnimStates[i]->addTime(mAnimSpeeds[i] * evt.timeSinceLastFrame);

			if (mAnimStates[i]->getTimePosition() >= ANIM_CHOP)   // when it's time to loop...
			{
				/* We need reposition the scene node origin, since the animation includes translation.
				Position is calculated from an offset to the end position, and rotation is calculated
				from how much the animation turns the character. */

				Quaternion rot(Degree(-60), Vector3::UNIT_Y);   // how much the animation turns the character

				// find current end position and the offset
				Vector3 currEnd = mModelNodes[i]->getOrientation() * mSneakEndPos + mModelNodes[i]->getPosition();
				Vector3 offset = rot * mModelNodes[i]->getOrientation() * -mSneakStartPos;

				mModelNodes[i]->setPosition(currEnd + offset);
				mModelNodes[i]->rotate(rot);

				mAnimStates[i]->setTimePosition(0);   // reset animation time
			}
        }

		return SdkSample::frameRenderingQueued(evt);
    }


protected:

	void setupContent()
	{

/*#if defined(INCLUDE_RTSHADER_SYSTEM) && defined(RTSHADER_SYSTEM_BUILD_EXT_SHADERS)
		//To make glsles work the program will need to be provided with proper
		//shadow caster materials
		if (mShaderGenerator->getTargetLanguage() != "glsles" && mShaderGenerator->getTargetLanguage() != "glsl")
		{
			//Add the hardware skinning to the shader generator default render state
			mSrsHardwareSkinning = mShaderGenerator->createSubRenderState(Ogre::RTShader::HardwareSkinning::Type);
			Ogre::RTShader::RenderState* renderState = mShaderGenerator->getRenderState(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
			renderState->addTemplateSubRenderState(mSrsHardwareSkinning);
			
			Ogre::MaterialPtr pCast1 = Ogre::MaterialManager::getSingleton().getByName("Ogre/RTShader/shadow_caster_dq_skinning_1weight");
			Ogre::MaterialPtr pCast2 = Ogre::MaterialManager::getSingleton().getByName("Ogre/RTShader/shadow_caster_dq_skinning_2weight");
			Ogre::MaterialPtr pCast3 = Ogre::MaterialManager::getSingleton().getByName("Ogre/RTShader/shadow_caster_dq_skinning_3weight");
			Ogre::MaterialPtr pCast4 = Ogre::MaterialManager::getSingleton().getByName("Ogre/RTShader/shadow_caster_dq_skinning_4weight");

			Ogre::RTShader::HardwareSkinningFactory::getSingleton().setCustomShadowCasterMaterials(
				Ogre::RTShader::ST_DUAL_QUATERNION, pCast1, pCast2, pCast3, pCast4);
		}
#endif*/
		// set shadow properties
		mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
		mSceneMgr->setShadowTextureSize(512);
		mSceneMgr->setShadowColour(ColourValue(0.6, 0.6, 0.6));
		mSceneMgr->setShadowTextureCount(2);

		// add a little ambient lighting
		mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

		SceneNode* lightsBbsNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		BillboardSet* bbs;

		// Create billboard set for lights .
		bbs = mSceneMgr->createBillboardSet();
		bbs->setMaterialName("Examples/Flare");
		lightsBbsNode->attachObject(bbs);


		// add a blue spotlight
		Light* l = mSceneMgr->createLight();
		Vector3 dir;
		l->setType(Light::LT_SPOTLIGHT);
		l->setPosition(-40, 180, -10);
		dir = -l->getPosition();
		dir.normalise();
		l->setDirection(dir);
		l->setDiffuseColour(0.0, 0.0, 0.5);
		bbs->createBillboard(l->getPosition())->setColour(l->getDiffuseColour());
		

		// add a green spotlight.
		l = mSceneMgr->createLight();
		l->setType(Light::LT_SPOTLIGHT);
		l->setPosition(0, 150, -100);
		dir = -l->getPosition();
		dir.normalise();
		l->setDirection(dir);
		l->setDiffuseColour(0.0, 0.5, 0.0);		
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
		tweakSneakAnim();

		SceneNode* sn = NULL;
		Entity* ent = NULL;
		AnimationState* as = NULL;

		for (unsigned int i = 0; i < NUM_MODELS; i++)
		{
			// create scene nodes for the models at regular angular intervals
			sn = mSceneMgr->getRootSceneNode()->createChildSceneNode();
			sn->yaw(Radian(Math::TWO_PI * (float)i / (float)NUM_MODELS));
			sn->translate(0, 0, -20, Node::TS_LOCAL);
			mModelNodes.push_back(sn);

			// create and attach a jaiqua entity
			ent = mSceneMgr->createEntity("Jaiqua" + StringConverter::toString(i + 1), "jaiqua.mesh");

#ifdef INCLUDE_RTSHADER_SYSTEM
            if (mShaderGenerator->getTargetLanguage() == "glsles")
            {
                MaterialPtr mat = MaterialManager::getSingleton().getByName("jaiqua");
                mat->getTechnique(0)->getPass(0)->setShadowCasterFragmentProgram("Ogre/BasicFragmentPrograms/PassthroughFpGLSLES");
            }
#endif
            ent->setMaterialName("jaiqua"); //"jaiquaDualQuatTest"
			sn->attachObject(ent);

/*#if defined(INCLUDE_RTSHADER_SYSTEM) && defined(RTSHADER_SYSTEM_BUILD_EXT_SHADERS)
			//To make glsles work the program will need to be provided with proper
			//shadow caster materials
			if (mShaderGenerator->getTargetLanguage() != "glsles")
			{
				//In case the system uses the RTSS, the following line will ensure
				//that the entity is using hardware animation in RTSS as well.
				RTShader::HardwareSkinningFactory::getSingleton().prepareEntityForSkinning(ent, Ogre::RTShader::ST_DUAL_QUATERNION, true, false);
				
				//The following line is needed only because the Jaiqua model material has shaders and
				//as such is not automatically reflected in the RTSS system
				RTShader::ShaderGenerator::getSingleton().createShaderBasedTechnique(
					ent->getSubEntity(0)->getMaterialName(),
					Ogre::MaterialManager::DEFAULT_SCHEME_NAME,
					Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME,
					true);
			}
#endif*/
		
			// enable the entity's sneaking animation at a random speed and loop it manually since translation is involved
			as = ent->getAnimationState("Sneak");
			as->setEnabled(true);
			as->setLoop(false);
			mAnimSpeeds.push_back(Math::RangeRandom(0.5, 1.5));
			mAnimStates.push_back(as);
		}

		// create name and value for skinning mode
		StringVector names;
		names.push_back("Skinning");
		String value = "Software";

		// change the value if hardware skinning is enabled
		MaterialPtr entityMaterial = ent->getSubEntity(0)->getMaterial();
		if(!entityMaterial.isNull())
		{
			Technique* bestTechnique = entityMaterial->getBestTechnique();
			if(bestTechnique)
			{
				Pass* pass = bestTechnique->getPass(0);
				if (pass && pass->hasVertexProgram() && pass->getVertexProgram()->isSkeletalAnimationIncluded()) 
				{
					value = "Hardware";
				}
			}
		}
		
		// create a params panel to display the skinning mode
		mTrayMgr->createParamsPanel(TL_TOPLEFT, "Skinning", 170, names)->setParamValue(0, value);
	}
	
	/*-----------------------------------------------------------------------------
	| The jaiqua sneak animation doesn't loop properly. This method tweaks the
	| animation to loop properly by altering the Spineroot bone track.
	-----------------------------------------------------------------------------*/
	void tweakSneakAnim()
	{
		// get the skeleton, animation, and the node track iterator
		SkeletonPtr skel = SkeletonManager::getSingleton().load("jaiqua.skeleton",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).staticCast<Skeleton>();
		Animation* anim = skel->getAnimation("Sneak");
		Animation::NodeTrackIterator tracks = anim->getNodeTrackIterator();

		while (tracks.hasMoreElements())   // for every node track...
		{
			NodeAnimationTrack* track = tracks.getNext();

			// get the keyframe at the chopping point
			TransformKeyFrame oldKf(0, 0);
			track->getInterpolatedKeyFrame(ANIM_CHOP, &oldKf);

			// drop all keyframes after the chopping point
			while (track->getKeyFrame(track->getNumKeyFrames()-1)->getTime() >= ANIM_CHOP - 0.3f)
				track->removeKeyFrame(track->getNumKeyFrames()-1);

			// create a new keyframe at chopping point, and get the first keyframe
			TransformKeyFrame* newKf = track->createNodeKeyFrame(ANIM_CHOP);
			TransformKeyFrame* startKf = track->getNodeKeyFrame(0);

			Bone* bone = skel->getBone(track->getHandle());

			if (bone->getName() == "Spineroot")   // adjust spine root relative to new location
			{
				mSneakStartPos = startKf->getTranslate() + bone->getInitialPosition();
				mSneakEndPos = oldKf.getTranslate() + bone->getInitialPosition();
				mSneakStartPos.y = mSneakEndPos.y;

				newKf->setTranslate(oldKf.getTranslate());
				newKf->setRotation(oldKf.getRotation());
				newKf->setScale(oldKf.getScale());
			}
			else   // make all other bones loop back
			{
				newKf->setTranslate(startKf->getTranslate());
				newKf->setRotation(startKf->getRotation());
				newKf->setScale(startKf->getScale());
			}
		}
	}

	void cleanupContent()
	{
		mModelNodes.clear();
		mAnimStates.clear();
		mAnimSpeeds.clear();
		MeshManager::getSingleton().remove("floor");
        mSceneMgr->destroyEntity("Jaiqua");

/*#if defined(INCLUDE_RTSHADER_SYSTEM) && defined(RTSHADER_SYSTEM_BUILD_EXT_SHADERS)
		//To make glsles work the program will need to be provided with proper
		//shadow caster materials
		if (mShaderGenerator->getTargetLanguage() != "glsles")
		{
			Ogre::RTShader::RenderState* renderState = mShaderGenerator->getRenderState(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
			renderState->removeTemplateSubRenderState(mSrsHardwareSkinning);
		}
#endif*/
	}

	const unsigned int NUM_MODELS;
	const Real ANIM_CHOP;

	std::vector<SceneNode*> mModelNodes;
	std::vector<AnimationState*> mAnimStates;
	std::vector<Real> mAnimSpeeds;

	Vector3 mSneakStartPos;
	Vector3 mSneakEndPos;

#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
	RTShader::SubRenderState* mSrsHardwareSkinning;
#endif
};

#endif
