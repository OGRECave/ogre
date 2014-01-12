#ifndef __Fresnel_H__
#define __Fresnel_H__

#include "SdkSample.h"

#include "Compositor/OgreCompositorWorkspaceListener.h"
#include "Compositor/Pass/OgreCompositorPass.h"
#include "Compositor/OgreCompositorWorkspace.h"

using namespace Ogre;
using namespace OgreBites;

const uint32 NonRefractiveSurfaces	= 0x00000001;
const uint32 RefractiveSurfaces		= 0x00000002;
const uint32 ReflectedSurfaces		= 0x00000004;
const uint32 RegularSurfaces		= NonRefractiveSurfaces|ReflectedSurfaces;

class _OgreSampleClassExport Sample_Fresnel : public SdkSample, public CompositorWorkspaceListener
{
	uint32 mPreviousVisibilityFlags;
public:

	Sample_Fresnel() : NUM_FISH(30), NUM_FISH_WAYPOINTS(10), FISH_PATH_LENGTH(200), FISH_SCALE(2)
	{
		mInfo["Title"] = "Fresnel";
		mInfo["Description"] = "Shows how to create reflections and refractions using render-to-texture, "
								"shaders and visibility mask, completely controlled via the compositor.\n"
								"See Fresnel.compositor and C++ code on how to setup the pipeline.\n\n"
								"NOTE: This demo is a bit outdated. Typical AAA games use screen-space "
								"refractions, which are bit less accurate but avoid an extra scene pass.\n"
								"This other technique is explained in GPU Gems 2, Chapter 19 "
								"'Generic Refraction Simulation'";
		mInfo["Thumbnail"] = "thumb_fresnel.png";
		mInfo["Category"] = "API Usage";
	}

	~Sample_Fresnel()
	{
	}

	StringVector getRequiredPlugins()
	{
		StringVector names;
        if (!GpuProgramManager::getSingleton().isSyntaxSupported("glsles") && !GpuProgramManager::getSingleton().isSyntaxSupported("glsl150"))
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

	virtual void passPreExecute( CompositorPass *pass )
	{
		//Demo Note: Be very careful when modifying the camera in a listener.
		//Ogre 2.0 is more sensitive to changes mid-render than 1.x. For example,
		//Ogre has already built the light list of all objects seen by all cameras.
		//If we altered the camera's position, some entities would be incorrectly
		//lit or shadowed.
		if( pass->getDefinition()->mIdentifier == 59645 )
		{
			mCamera->setAutoAspectRatio( false );
			mCamera->enableReflection(mWaterPlane);
			mCamera->enableCustomNearClipPlane(mWaterPlane);
		}
		else if( pass->getDefinition()->mIdentifier == 59646 )
		{
			mCamera->setAutoAspectRatio( false );
			mCamera->enableCustomNearClipPlane(mInvWaterPlane);
			mCamera->disableCustomNearClipPlane();
		}
		else
		{
			mCamera->setAutoAspectRatio( true );
			mCamera->disableReflection();
			mCamera->disableCustomNearClipPlane();
		}
	}

protected:

	virtual CompositorWorkspace* setupCompositor()
	{
		// The compositor scripts are also part of this sample. Go to Fresnel.compositor
		// to see the sample scripts on how to setup the rendering pipeline.
		CompositorManager2 *compositorManager = mRoot->getCompositorManager2();

		const Ogre::IdString workspaceName( "FresnelSampleWorkspace" );
		CompositorWorkspace *workspace = compositorManager->addWorkspace( mSceneMgr, mWindow,
																	mCamera, workspaceName, true );
		workspace->setListener( this );

		return workspace;
	}

	void setupContent()
	{
		mPreviousVisibilityFlags = MovableObject::getDefaultVisibilityFlags();
		MovableObject::setDefaultVisibilityFlags( RegularSurfaces );

        mCamera->setPosition(-50, 125, 760);
		mCameraMan->setTopSpeed(280);

        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));  // set ambient light

        mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");  // set a skybox

        // make the scene's main light come from above
        Light* l = mSceneMgr->createLight();
		mSceneMgr->createSceneNode()->attachObject( l );
        l->setType(Light::LT_DIRECTIONAL);
        l->setDirection(Vector3::NEGATIVE_UNIT_Y);

		setupWater();
		setupProps();
		setupFish();
	}

	void setupWater()
	{
		// create our water plane mesh
        mWaterPlane = Plane(Vector3::UNIT_Y, 0);
		mInvWaterPlane=Plane(Vector3::NEGATIVE_UNIT_Y, 0);
        MeshManager::getSingleton().createPlane("water", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            mWaterPlane, 700, 1300, 10, 10, true, 1, 3, 5, Vector3::UNIT_Z);

		// create a water entity using our mesh, give it the shader material, and attach it to the origin
        mWater = mSceneMgr->createEntity("water");
        mWater->setName("Water");
        mWater->setMaterialName("Examples/FresnelReflectionRefraction");
		mWater->setVisibilityFlags( RefractiveSurfaces );
		mWater->setRenderQueueGroup( 95 );
        mSceneMgr->getRootSceneNode()->attachObject(mWater);
	}

	void setupProps()
	{
        Entity* ent;

		// setting up props might take a while, so create a progress bar for visual feedback
		ProgressBar* pb = mTrayMgr->createProgressBar(TL_CENTER, "FresnelBuildingBar", "Creating Props...", 280, 100);
		mTrayMgr->showBackdrop("SdkTrays/Shade");

		pb->setComment("Upper Bath");
        ent = mSceneMgr->createEntity("RomanBathUpper.mesh" );
        ent->setName("UpperBath");
		mSceneMgr->getRootSceneNode()->attachObject(ent);        
        mSurfaceEnts.push_back(ent);
		pb->setProgress(0.4);

		pb->setComment("Columns");
        ent = mSceneMgr->createEntity("columns.mesh");
        ent->setName("Columns");
		mSceneMgr->getRootSceneNode()->attachObject(ent);
        mSurfaceEnts.push_back(ent);
		pb->setProgress(0.5);

		pb->setComment("Ogre Head");
		ent = mSceneMgr->createEntity("ogrehead.mesh");
        ent->setName("Head");
		ent->setMaterialName("RomanBath/OgreStone");
        mSurfaceEnts.push_back(ent);
		pb->setProgress(0.6);

		SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		headNode->setPosition(-350, 55, 130);
		headNode->yaw(Degree(90));
        headNode->attachObject(ent);

		pb->setComment("Lower Bath");
		ent = mSceneMgr->createEntity("RomanBathLower.mesh");
        ent->setName("LowerBath");
        mSceneMgr->getRootSceneNode()->attachObject(ent);
        mSubmergedEnts.push_back(ent);
		pb->setProgress(1);

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
            Entity* ent = mSceneMgr->createEntity("fish.mesh");
            ent->setName("Fish" + StringConverter::toString(i + 1));
			ent->setVisibilityFlags( NonRefractiveSurfaces ); //Fishes are underwater, and hence don't reflect
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

		//Restore global settings
		MovableObject::setDefaultVisibilityFlags( mPreviousVisibilityFlags );
	}

	const unsigned int NUM_FISH;
	const unsigned int NUM_FISH_WAYPOINTS;
	const unsigned int FISH_PATH_LENGTH; 
	const Real FISH_SCALE;
	std::vector<Entity*> mSurfaceEnts;
	std::vector<Entity*> mSubmergedEnts;
	Plane mWaterPlane;
	Plane mInvWaterPlane;
	Entity* mWater;
	std::vector<SceneNode*> mFishNodes;
	std::vector<AnimationState*> mFishAnimStates;
	std::vector<SimpleSpline> mFishSplines;
	Real mFishAnimTime;
};

#endif
