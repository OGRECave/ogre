/**
Demo of Deferred Shading in OGRE using Multiple Render Targets and HLSL/GLSL high level 
language shaders.
	// W.J. :wumpus: van der Laan 2005 //

Deferred shading renders the scene to a 'fat' texture format, using a shader that outputs colour, 
normal, depth, and possible other attributes per fragment. Multi Render Target is required as we 
are dealing with many outputs which get written into multiple render textures in the same pass.

After rendering the scene in this format, the shading (lighting) can be done as a post process. 
This means that lighting is done in screen space. Adding them requires nothing more than rendering 
a screenful quad; thus the method allows for an enormous amount of lights without noticeable 
performance loss.

Little lights affecting small area ("Minilights") can be even further optimised by rendering 
their convex bounding geometry. This is also shown in this demo by 6 swarming lights.

The paper for GDC2004 on Deferred Shading can be found here:
  http://www.talula.demon.co.uk/DeferredShading.pdf

This demo source file is in the public domain.
*/

#include "Ogre.h"
#include "ExampleApplication.h"
#include "ExampleFrameListener.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#include "DeferredShading.h"
#include "MLight.h"
#include "GeomUtils.h"
class SharedData : public Ogre::Singleton<SharedData> {

public:

	SharedData()
		: iRoot(0),
		  iCamera(0),
		  iWindow(0),
		  mAnimState(0),
		  mMLAnimState(0),
		  iMainLight(0)
	{
		iActivate = false;
	}

		~SharedData() {}

		// shared data across the application
		Real iLastFrameTime;
		Root *iRoot;
		Camera *iCamera;
		RenderWindow *iWindow;

		DeferredShadingSystem *iSystem;
		bool iActivate;
		bool iGlobalActivate;

		// Animation state for big lights
		AnimationState* mAnimState;
		// Animation state for light swarm
		AnimationState* mMLAnimState;

		MLight *iMainLight;

		std::vector<Node*> mLightNodes;

};
template<> SharedData* Singleton<SharedData>::ms_Singleton = 0;

class RenderToTextureFrameListener : public ExampleFrameListener
{
protected:
	Real timeoutDelay ;
	Vector3 oldCamPos;
	Quaternion oldCamOri;
public:
	RenderToTextureFrameListener(RenderWindow* window, Camera* maincam)
		:ExampleFrameListener(window, maincam), 
		oldCamPos(0,0,0), oldCamOri(0,0,0,0)
	{
		timeoutDelay = 0;
		mMoveSpeed = 200;
	}

	bool frameRenderingQueued(const FrameEvent& evt)
	{
		if( ExampleFrameListener::frameRenderingQueued(evt) == false )
			return false;
		SharedData::getSingleton().iLastFrameTime = evt.timeSinceLastFrame;

		if (SharedData::getSingleton().mAnimState)
			SharedData::getSingleton().mAnimState->addTime(evt.timeSinceLastFrame);
		if (SharedData::getSingleton().mMLAnimState)
			SharedData::getSingleton().mMLAnimState->addTime(evt.timeSinceLastFrame);
		return true;
	}

	virtual bool processUnbufferedKeyInput(const FrameEvent& evt) {
		bool retval = ExampleFrameListener::processUnbufferedKeyInput(evt);

		// "C" switch filters
		if (mKeyboard->isKeyDown(OIS::KC_C) && timeoutDelay==0) 
		{
			timeoutDelay = 0.5f;

			DeferredShadingSystem* iSystem = SharedData::getSingleton().iSystem;

			iSystem->setMode(
				(DeferredShadingSystem::DSMode)
				((iSystem->getMode() + 1)%DeferredShadingSystem::DSM_COUNT)
				);

			updateOverlays();
		}

		// "B" activate/deactivate minilight rendering
		if (mKeyboard->isKeyDown(OIS::KC_B) && timeoutDelay==0) 
		{
			timeoutDelay = 0.5f;
			SharedData::getSingleton().iActivate = !SharedData::getSingleton().iActivate;
			// Hide/show all minilights
			std::vector<Node*>::iterator i = SharedData::getSingleton().mLightNodes.begin();
			std::vector<Node*>::iterator iend = SharedData::getSingleton().mLightNodes.end();
			for(; i!=iend; ++i)
			{
				static_cast<SceneNode*>(*i)->setVisible(SharedData::getSingleton().iActivate, true);
			}
			
			updateOverlays();
		}
		// "G" activate/deactivate global light rendering
		if (mKeyboard->isKeyDown(OIS::KC_G) && timeoutDelay==0) 
		{
			timeoutDelay = 0.5f;
			SharedData::getSingleton().iGlobalActivate = !SharedData::getSingleton().iGlobalActivate;
			SharedData::getSingleton().iMainLight->setVisible(SharedData::getSingleton().iGlobalActivate);
			updateOverlays();
		}

		timeoutDelay -= evt.timeSinceLastFrame;
		if (timeoutDelay <= 0) timeoutDelay = 0;

		return retval;
	}

	void updateOverlays() 
	{
		OverlayManager::getSingleton().getOverlayElement( "Example/Shadows/ShadowTechniqueInfo" )
			->setCaption( "" );

		OverlayManager::getSingleton().getOverlayElement( "Example/Shadows/MaterialInfo" )
			->setCaption( "");

		OverlayManager::getSingleton().getOverlayElement( "Example/Shadows/ShadowTechnique" )
			->setCaption( "[B] MiniLights active: " + StringConverter::toString( SharedData::getSingleton().iActivate ) );

		std::string name;
		switch(SharedData::getSingleton().iSystem->getMode())
		{
		case DeferredShadingSystem::DSM_SHOWLIT:
			name="ShowLit"; break;
		case DeferredShadingSystem::DSM_SHOWCOLOUR:
			name="ShowColour"; break;
		case DeferredShadingSystem::DSM_SHOWNORMALS:
			name="ShowNormals"; break;
		case DeferredShadingSystem::DSM_SHOWDSP:
			name="ShowDepthSpecular"; break;
		}
		OverlayManager::getSingleton().getOverlayElement( "Example/Shadows/Materials" )
			->setCaption( "[C] Change mode, current is \"" 
			+ name 
			+ "\"");

		OverlayManager::getSingleton().getOverlayElement( "Example/Shadows/Info" )
			->setCaption( "[G] Global lights active: " + StringConverter::toString( SharedData::getSingleton().iGlobalActivate ) );

	}
};


class RenderToTextureApplication : public ExampleApplication, public RenderTargetListener
{
public:
    RenderToTextureApplication() : mPlane(0) {
		new SharedData();
		mPlane = 0;
		mSystem = 0;
	}
    
	~RenderToTextureApplication()
    {
		delete ( SharedData::getSingletonPtr() );

        delete mPlane;
		delete mSystem;
	}


protected:
    MovablePlane* mPlane;
    Entity* mPlaneEnt;
    SceneNode* mPlaneNode;
	DeferredShadingSystem *mSystem;

    // Just override the mandatory create scene method
    void createScene(void)
    {
		RenderSystem *rs = Root::getSingleton().getRenderSystem();
		const RenderSystemCapabilities* caps = rs->getCapabilities();
        if (!caps->hasCapability(RSC_VERTEX_PROGRAM) || !(caps->hasCapability(RSC_FRAGMENT_PROGRAM)))
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your card does not support vertex and fragment programs, so cannot "
                "run this demo. Sorry!", 
                "DeferredShading::createScene");
        }
		if (caps->getNumMultiRenderTargets()<2)
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your card does not support at least two simultaneous render targets, so cannot "
                "run this demo. Sorry!", 
                "DeferredShading::createScene");
        }

		// Prepare athene mesh for normalmapping
        MeshPtr pAthene = MeshManager::getSingleton().load("athene.mesh", 
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        unsigned short src, dest;
        if (!pAthene->suggestTangentVectorBuildParams(VES_TANGENT, src, dest))
            pAthene->buildTangentVectors(VES_TANGENT, src, dest);
		// Prepare knot mesh for normal mapping
		pAthene = MeshManager::getSingleton().load("knot.mesh", 
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        if (!pAthene->suggestTangentVectorBuildParams(VES_TANGENT, src, dest))
            pAthene->buildTangentVectors(VES_TANGENT, src, dest);

        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.15));
        // Skybox
        mSceneMgr->setSkyBox(true, "DeferredDemo/SkyBox");

		// Create "root" node
		SceneNode* rootNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

		Entity* athena = mSceneMgr->createEntity("Athena", "athene.mesh");
		athena->setMaterialName("DeferredDemo/DeferredAthena");
		SceneNode *aNode = rootNode->createChildSceneNode();
		aNode->attachObject( athena );
		aNode->setPosition(-100, 40, 100);

		// Create a prefab plane
		mPlane = new MovablePlane("ReflectPlane");
		mPlane->d = 0;
		mPlane->normal = Vector3::UNIT_Y;
		MeshManager::getSingleton().createCurvedPlane("ReflectionPlane", 
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
			*mPlane,
			2000, 2000, -1000,
			20, 20, 
			true, 1, 10, 10, Vector3::UNIT_Z);
		mPlaneEnt = mSceneMgr->createEntity( "Plane", "ReflectionPlane" );
		mPlaneNode = rootNode->createChildSceneNode();
		mPlaneNode->attachObject(mPlaneEnt);
		mPlaneNode->translate(-5, -30, 0);
		//mPlaneNode->roll(Degree(5));
		mPlaneEnt->setMaterialName("DeferredDemo/Ground");

		// Create an entity from a model (will be loaded automatically)
		Entity* knotEnt = mSceneMgr->createEntity("Knot", "knot.mesh");
		knotEnt->setMaterialName("DeferredDemo/RockWall");
		knotEnt->setMeshLodBias(0.25f);

		// Create an entity from a model (will be loaded automatically)
		Entity* ogreHead = mSceneMgr->createEntity("Head", "ogrehead.mesh");
		ogreHead->getSubEntity(0)->setMaterialName("DeferredDemo/Ogre/Eyes");// eyes
		ogreHead->getSubEntity(1)->setMaterialName("DeferredDemo/Ogre/Skin"); 
		ogreHead->getSubEntity(2)->setMaterialName("DeferredDemo/Ogre/EarRing"); // earrings
		ogreHead->getSubEntity(3)->setMaterialName("DeferredDemo/Ogre/Tusks"); // tusks
		rootNode->createChildSceneNode( "Head" )->attachObject( ogreHead );

		// Add a whole bunch of extra entities to fill the scene a bit
		Entity *cloneEnt;
		int N=4;
		for (int n = 0; n < N; ++n)
		{
			float theta = 2.0f*Math::PI*(float)n/(float)N;
			// Create a new node under the root
			SceneNode* node = mSceneMgr->createSceneNode();
			// Random translate
			Vector3 nodePos;
			nodePos.x = Math::SymmetricRandom() * 40.0 + Math::Sin(theta) * 500.0;
			nodePos.y = Math::SymmetricRandom() * 20.0 - 40.0;
			nodePos.z = Math::SymmetricRandom() * 40.0 + Math::Cos(theta) * 500.0;
			node->setPosition(nodePos);
			Quaternion orientation(Math::SymmetricRandom(),Math::SymmetricRandom(),Math::SymmetricRandom(),Math::SymmetricRandom());
			orientation.normalise();
			node->setOrientation(orientation);
			rootNode->addChild(node);
			// Clone knot
			char cloneName[12];
			sprintf(cloneName, "Knot%d", n);
			cloneEnt = knotEnt->clone(cloneName);
			// Attach to new node
			node->attachObject(cloneEnt);

		}

        mCamera->setPosition(-50, 100, 500);
        mCamera->lookAt(0,0,0);

		// show overlay
		Overlay* overlay = OverlayManager::getSingleton().getByName("Example/ShadowsOverlay");    
		overlay->show();

		mSystem = new DeferredShadingSystem(mWindow->getViewport(0), mSceneMgr, mCamera);

		// Create main, moving light
		MLight* l1 = mSystem->createMLight();//"MainLight");
        l1->setDiffuseColour(0.75f, 0.7f, 0.8f);
		l1->setSpecularColour(0.85f, 0.9f, 1.0f);
		
		SceneNode *lightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		lightNode->attachObject(l1);

		// Create a track for the light
        Animation* anim = mSceneMgr->createAnimation("LightTrack", 16);
        // Spline it for nice curves
        anim->setInterpolationMode(Animation::IM_SPLINE);
        // Create a track to animate the camera's node
        NodeAnimationTrack* track = anim->createNodeTrack(0, lightNode);
        // Setup keyframes
        TransformKeyFrame* key = track->createNodeKeyFrame(0); // A start position
        key->setTranslate(Vector3(300,300,-300));
        key = track->createNodeKeyFrame(4);//B
        key->setTranslate(Vector3(300,300,300));
        key = track->createNodeKeyFrame(8);//C
        key->setTranslate(Vector3(-300,300,300));
        key = track->createNodeKeyFrame(12);//D
        key->setTranslate(Vector3(-300,300,-300));
		key = track->createNodeKeyFrame(16);//D
        key->setTranslate(Vector3(300,300,-300));
        // Create a new animation state to track this
        SharedData::getSingleton().mAnimState = mSceneMgr->createAnimationState("LightTrack");
        SharedData::getSingleton().mAnimState->setEnabled(true);

		// Create some happy little lights
		createSampleLights();

		// safely setup application's (not postfilter!) shared data
		SharedData::getSingleton().iCamera = mCamera;
		SharedData::getSingleton().iRoot = mRoot;
		SharedData::getSingleton().iWindow = mWindow;
		SharedData::getSingleton().iActivate = true;
		SharedData::getSingleton().iGlobalActivate = true;
		SharedData::getSingleton().iSystem = mSystem;
		SharedData::getSingleton().iMainLight = l1;
	}

    void createFrameListener(void)
    {
        mFrameListener= new RenderToTextureFrameListener(mWindow, mCamera);
		// initialize overlays
		static_cast<RenderToTextureFrameListener*>(mFrameListener)->updateOverlays();
        mRoot->addFrameListener(mFrameListener);
    }

	void createSampleLights()
	{
		// Create some lights		
		std::vector<MLight*> lights;
		SceneNode *parentNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("LightsParent");
		// Create light nodes
		std::vector<Node*> nodes;

		MLight *a = mSystem->createMLight();
		SceneNode *an = parentNode->createChildSceneNode();
		an->attachObject(a);
		a->setAttenuation(1.0f, 0.001f, 0.002f);
		//a->setAttenuation(1.0f, 0.000f, 0.000f);
		an->setPosition(0,0,25);
		a->setDiffuseColour(1,0,0);
		//a->setSpecularColour(0.5,0,0);
		lights.push_back(a);
		nodes.push_back(an);

		MLight *b = mSystem->createMLight();
		SceneNode *bn = parentNode->createChildSceneNode();
		bn->attachObject(b);
		b->setAttenuation(1.0f, 0.001f, 0.003f);
		bn->setPosition(25,0,0);
		b->setDiffuseColour(1,1,0);
		//b->setSpecularColour(0.5,0.5,0);
		lights.push_back(b);
		nodes.push_back(bn);

		MLight *c = mSystem->createMLight();
		SceneNode *cn = parentNode->createChildSceneNode();
		cn->attachObject(c);
		c->setAttenuation(1.0f, 0.001f, 0.004f);
		cn->setPosition(0,0,-25);
		c->setDiffuseColour(0,1,1);
		c->setSpecularColour(0.25,1.0,1.0); // Cyan light has specular component
		lights.push_back(c);
		nodes.push_back(cn);

		MLight *d = mSystem->createMLight();
		SceneNode *dn = parentNode->createChildSceneNode();
		dn->attachObject(d);
		d->setAttenuation(1.0f, 0.002f, 0.002f);
		dn->setPosition(-25,0,0);
		d->setDiffuseColour(1,0,1);
		d->setSpecularColour(0.0,0,0.0);
		lights.push_back(d);
		nodes.push_back(dn);

		MLight *e = mSystem->createMLight();
		SceneNode *en = parentNode->createChildSceneNode();
		en->attachObject(e);
		e->setAttenuation(1.0f, 0.002f, 0.0025f);
		en->setPosition(25,0,25);
		e->setDiffuseColour(0,0,1);
		e->setSpecularColour(0,0,0);
		lights.push_back(e);
		nodes.push_back(en);
		
		MLight *f = mSystem->createMLight();
		SceneNode *fn = parentNode->createChildSceneNode();
		fn->attachObject(f);
		f->setAttenuation(1.0f, 0.0015f, 0.0021f);
		fn->setPosition(-25,0,-25);
		f->setDiffuseColour(0,1,0);
		f->setSpecularColour(0,0.0,0.0);
		lights.push_back(f);
		nodes.push_back(fn);

		// Create marker meshes to show user where the lights are
		Entity *ent;
		GeomUtils::createSphere("PointLightMesh", 1.0f, 5, 5, true, true);
		for(std::vector<MLight*>::iterator i=lights.begin(); i!=lights.end(); ++i)
		{
			MLight* light = *i;
			ent = mSceneMgr->createEntity(light->getName()+"v", "PointLightMesh");
			String matname = light->getName()+"m";
			// Create coloured material
			MaterialPtr mat = MaterialManager::getSingleton().create(matname,
                ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            Pass* pass = mat->getTechnique(0)->getPass(0);
            pass->setDiffuse(0.0f,0.0f,0.0f,1.0f);
			pass->setAmbient(0.0f,0.0f,0.0f);
			pass->setSelfIllumination(light->getDiffuseColour());

			ent->setMaterialName(matname);
			ent->setRenderQueueGroup(light->getRenderQueueGroup());
			static_cast<SceneNode*>(light->getParentNode())->attachObject(ent);
		}		

		// Store nodes for hiding/showing
		SharedData::getSingleton().mLightNodes = nodes;

		// Do some animation for node a-f
		// Generate helix structure
		float seconds_per_station = 1.0f;
		float r=35;
		//Vector3 base(0,-30,0);
		Vector3 base(-100, -30, 85);

		float h=120;
		const size_t s_to_top = 16;
		const size_t stations = s_to_top*2-1;
		float ascend = h/((float)s_to_top);
		float stations_per_revolution = 3.5f;
		size_t skip = 2; // stations between lights
		Vector3 station_pos[stations];
		for(int x=0; x<s_to_top; ++x)
		{
			float theta = ((float)x/stations_per_revolution)*2.0f*Math::PI;
			station_pos[x] = base+Vector3(Math::Sin(theta)*r, ascend*x, Math::Cos(theta)*r);
		}
		for(int x=s_to_top; x<stations; ++x)
		{
			float theta = ((float)x/stations_per_revolution)*2.0f*Math::PI;
			station_pos[x] = base+Vector3(Math::Sin(theta)*r, h-ascend*(x-s_to_top), Math::Cos(theta)*r);
		}
		// Create a track for the light swarm
		Animation* anim = mSceneMgr->createAnimation("LightSwarmTrack", stations*seconds_per_station);
		// Spline it for nice curves
		anim->setInterpolationMode(Animation::IM_SPLINE);
		for(unsigned int x=0; x<nodes.size(); ++x)
		{
			// Create a track to animate the camera's node
			NodeAnimationTrack* track = anim->createNodeTrack(x, nodes[x]);
			for(int y=0; y<=stations; ++y)
			{
				// Setup keyframes
				TransformKeyFrame* key = track->createNodeKeyFrame(y*seconds_per_station); // A start position
				key->setTranslate(station_pos[(x*skip+y)%stations]);
				// Make sure size of light doesn't change
				key->setScale(nodes[x]->getScale());
			}
		}
		// Create a new animation state to track this
		SharedData::getSingleton().mMLAnimState = mSceneMgr->createAnimationState("LightSwarmTrack");
		SharedData::getSingleton().mMLAnimState->setEnabled(true);
	}

};





#ifdef __cplusplus
extern "C" {
#endif

//#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
//	INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
//#else
	int main(int argc, char *argv[])
//#endif
	{
		// Create application object
		RenderToTextureApplication app;

		try {
			app.go();
		} catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
			std::cerr << "An exception has occurred: " <<
				e.getFullDescription().c_str() << std::endl;
#endif
		}

		return 0;
	}

#ifdef __cplusplus
}
#endif


