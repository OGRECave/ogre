#ifndef __Fresnel_H__
#define __Fresnel_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_Fresnel : public SdkSample, public RenderTargetListener
{
public:

	Sample_Fresnel() : NUM_FISH(30), NUM_FISH_WAYPOINTS(10), FISH_PATH_LENGTH(200), FISH_SCALE(2)
	{
		mInfo["Title"] = "Fresnel";
		mInfo["Description"] = "Shows how to create reflections and refractions using render-to-texture and shaders.";
		mInfo["Thumbnail"] = "thumb_fresnel.png";
		mInfo["Category"] = "Unsorted";
	}

	StringVector getRequiredPlugins()
	{
		StringVector names;
		if(!GpuProgramManager::getSingleton().isSyntaxSupported("glsles")
		&& !GpuProgramManager::getSingleton().isSyntaxSupported("glsl150")
		&& !GpuProgramManager::getSingleton().isSyntaxSupported("hlsl"))
            names.push_back("Cg Program Manager");
		return names;
	}

	void testCapabilities(const RenderSystemCapabilities* caps)
	{
		if (!caps->hasCapability(RSC_VERTEX_PROGRAM) || !caps->hasCapability(RSC_FRAGMENT_PROGRAM))
        {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your graphics card does not support vertex and fragment"
				" programs, so you cannot run this sample. Sorry!", "FresnelSample::testCapabilities");
        }

        if (!GpuProgramManager::getSingleton().isSyntaxSupported("arbfp1") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("ps_4_0") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("ps_2_0") &&
			!GpuProgramManager::getSingleton().isSyntaxSupported("ps_1_4") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("glsles") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("glsl"))
        {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your graphics card does not support advanced fragment"
				" programs, so you cannot run this sample. Sorry!", "FresnelSample::testCapabilities");
        }
	}

    bool frameRenderingQueued(const FrameEvent &evt)
    {
		// update the fish spline path animations and loop as needed
        mFishAnimTime += evt.timeSinceLastFrame;
        while (mFishAnimTime >= FISH_PATH_LENGTH) mFishAnimTime -= FISH_PATH_LENGTH;

        for (unsigned int i = 0; i < NUM_FISH; i++)
        {
            mFishAnimStates[i]->addTime(evt.timeSinceLastFrame * 2);  // update fish swim animation

			// set the new position based on the spline path and set the direction based on displacement
			Vector3 lastPos = mFishNodes[i]->getPosition();
            mFishNodes[i]->setPosition(mFishSplines[i].interpolate(mFishAnimTime / FISH_PATH_LENGTH));
			mFishNodes[i]->setDirection(mFishNodes[i]->getPosition() - lastPos, Node::TS_PARENT, Vector3::NEGATIVE_UNIT_X);
			mFishNodes[i]->setFixedYawAxis(true);
        }

        return SdkSample::frameRenderingQueued(evt);
    }

    void preRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        mWater->setVisible(false);  // hide the water

        if (evt.source == mReflectionTarget)  // for reflection, turn on camera reflection and hide submerged entities
		{
			mCamera->enableReflection(mWaterPlane);
			for (std::vector<Entity*>::iterator i = mSubmergedEnts.begin(); i != mSubmergedEnts.end(); i++)
				(*i)->setVisible(false);
		}
		else  // for refraction, hide surface entities
		{
			for (std::vector<Entity*>::iterator i = mSurfaceEnts.begin(); i != mSurfaceEnts.end(); i++)
				(*i)->setVisible(false);
		}
    }

    void postRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        mWater->setVisible(true);  // unhide the water

        if (evt.source == mReflectionTarget)  // for reflection, turn off camera reflection and unhide submerged entities
		{
			mCamera->disableReflection();
			for (std::vector<Entity*>::iterator i = mSubmergedEnts.begin(); i != mSubmergedEnts.end(); i++)
				(*i)->setVisible(true);
		}
		else  // for refraction, unhide surface entities
		{
			for (std::vector<Entity*>::iterator i = mSurfaceEnts.begin(); i != mSurfaceEnts.end(); i++)
				(*i)->setVisible(true);
		}
    }

protected:

	void setupContent()
	{
        mCamera->setPosition(-50, 125, 760);
		mCameraMan->setTopSpeed(280);

        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));  // set ambient light

        mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");  // set a skybox

        // make the scene's main light come from above
        Light* l = mSceneMgr->createLight();
        l->setType(Light::LT_DIRECTIONAL);
        l->setDirection(Vector3::NEGATIVE_UNIT_Y);

		setupWater();
		setupProps();
		setupFish();
	}

	void setupWater()
	{
		// create our reflection & refraction render textures, and setup their render targets
		for (unsigned int i = 0; i < 2; i++)
		{
			TexturePtr tex = TextureManager::getSingleton().createManual(i == 0 ? "refraction" : "reflection",
				ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, 512, 512, 0, PF_R8G8B8, TU_RENDERTARGET);

			RenderTarget* rtt = tex->getBuffer()->getRenderTarget();
			rtt->addViewport(mCamera)->setOverlaysEnabled(false);
			rtt->addListener(this);

			if (i == 0) mRefractionTarget = rtt;
			else mReflectionTarget = rtt;
		}

		// create our water plane mesh
        mWaterPlane = Plane(Vector3::UNIT_Y, 0);
        MeshManager::getSingleton().createPlane("water", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            mWaterPlane, 700, 1300, 10, 10, true, 1, 3, 5, Vector3::UNIT_Z);

		// create a water entity using our mesh, give it the shader material, and attach it to the origin
        mWater = mSceneMgr->createEntity("Water", "water");
        mWater->setMaterialName("Examples/FresnelReflectionRefraction");
        mSceneMgr->getRootSceneNode()->attachObject(mWater);
	}

    void windowUpdate()
    {
#if OGRE_PLATFORM != OGRE_PLATFORM_NACL
        mWindow->update();
#endif
    }

	void setupProps()
	{
        Entity* ent;

		// setting up props might take a while, so create a progress bar for visual feedback
		ProgressBar* pb = mTrayMgr->createProgressBar(TL_CENTER, "FresnelBuildingBar", "Creating Props...", 280, 100);
		mTrayMgr->showBackdrop("SdkTrays/Shade");

		pb->setComment("Upper Bath");
		windowUpdate();
        ent = mSceneMgr->createEntity("UpperBath", "RomanBathUpper.mesh" );
		mSceneMgr->getRootSceneNode()->attachObject(ent);        
        mSurfaceEnts.push_back(ent);
		pb->setProgress(0.4);

		pb->setComment("Columns");
		windowUpdate();
        ent = mSceneMgr->createEntity("Columns", "columns.mesh");
		mSceneMgr->getRootSceneNode()->attachObject(ent);        
        mSurfaceEnts.push_back(ent);
		pb->setProgress(0.5);

		pb->setComment("Ogre Head");
		windowUpdate();
		ent = mSceneMgr->createEntity("Head", "ogrehead.mesh");
		ent->setMaterialName("RomanBath/OgreStone");
        mSurfaceEnts.push_back(ent);
		pb->setProgress(0.6);

		SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		headNode->setPosition(-350, 55, 130);
		headNode->yaw(Degree(90));
        headNode->attachObject(ent);

		pb->setComment("Lower Bath");
		windowUpdate();
		ent = mSceneMgr->createEntity("LowerBath", "RomanBathLower.mesh");
        mSceneMgr->getRootSceneNode()->attachObject(ent);
        mSubmergedEnts.push_back(ent);
		pb->setProgress(1);
		windowUpdate();

		mTrayMgr->destroyWidget(pb);
		mTrayMgr->hideBackdrop();
	}

	void setupFish()
	{
		mFishNodes.resize(NUM_FISH);
		mFishAnimStates.resize(NUM_FISH);
		mFishSplines.resize(NUM_FISH);

		for (unsigned int i = 0; i < NUM_FISH; i++)
        {
			// create fish entity
            Entity* ent = mSceneMgr->createEntity("Fish" + StringConverter::toString(i + 1), "fish.mesh");
            mSubmergedEnts.push_back(ent);

			// create an appropriately scaled node and attach the entity
			mFishNodes[i] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
			mFishNodes[i]->setScale(Vector3::UNIT_SCALE * FISH_SCALE);
            mFishNodes[i]->attachObject(ent);

			// enable and save the swim animation state
            mFishAnimStates[i] = ent->getAnimationState("swim");
            mFishAnimStates[i]->setEnabled(true);

            mFishSplines[i].setAutoCalculate(false);  // save the tangent calculation for when we are all done

            // generate random waypoints for the fish to swim through
            for (unsigned int j = 0; j < NUM_FISH_WAYPOINTS; j++)
            {
                Vector3 pos(Math::SymmetricRandom() * 270, -10, Math::SymmetricRandom() * 700);

                if (j > 0)  // make sure the waypoint isn't too far from the last, or our fish will be turbo-fish
                {
					const Vector3& lastPos = mFishSplines[i].getPoint(j - 1);
					Vector3 delta = pos - lastPos;
					if (delta.length() > 750) pos = lastPos + delta.normalisedCopy() * 750;
                }

                mFishSplines[i].addPoint(pos);
            }

			// close the spline and calculate all the tangents at once
            mFishSplines[i].addPoint(mFishSplines[i].getPoint(0));
            mFishSplines[i].recalcTangents();
        }

		mFishAnimTime = 0;
	}

	void cleanupContent()
	{
		mSurfaceEnts.clear();
		mSubmergedEnts.clear();
		mFishNodes.clear();
		mFishAnimStates.clear();
		mFishSplines.clear();

		MeshManager::getSingleton().remove("water");
		TextureManager::getSingleton().remove("refraction");
		TextureManager::getSingleton().remove("reflection");
	}

	const unsigned int NUM_FISH;
	const unsigned int NUM_FISH_WAYPOINTS;
	const unsigned int FISH_PATH_LENGTH; 
	const Real FISH_SCALE;
	std::vector<Entity*> mSurfaceEnts;
	std::vector<Entity*> mSubmergedEnts;
	RenderTarget* mRefractionTarget;
	RenderTarget* mReflectionTarget;
	Plane mWaterPlane;
	Entity* mWater;
	std::vector<SceneNode*> mFishNodes;
	std::vector<AnimationState*> mFishAnimStates;
	std::vector<SimpleSpline> mFishSplines;
	Real mFishAnimTime;
};

#endif
