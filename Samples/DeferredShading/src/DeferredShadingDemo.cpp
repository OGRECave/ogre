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
#include "GeomUtils.h"
#include "SharedData.h"

template<> SharedData* Singleton<SharedData>::ms_Singleton = 0;
const ColourValue SAMPLE_COLORS[] = 
    {   ColourValue::Red, ColourValue::Green, ColourValue::Blue, 
    ColourValue::White, ColourValue(1,1,0,1), ColourValue(1,0,1,1) };

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
		mMoveSpeed = 10;
	}

	bool frameRenderingQueued(const FrameEvent& evt)
	{
		if( ExampleFrameListener::frameRenderingQueued(evt) == false )
			return false;
		SharedData::getSingleton().iLastFrameTime = evt.timeSinceLastFrame;

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
			vector<Node*>::type::iterator i = SharedData::getSingleton().mLightNodes.begin();
			vector<Node*>::type::iterator iend = SharedData::getSingleton().mLightNodes.end();
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

		// "V" activate/deactivate ssao
		if (mKeyboard->isKeyDown(OIS::KC_V) && timeoutDelay==0) 
		{
			timeoutDelay = 0.5f;
			bool curMode = SharedData::getSingleton().iSystem->getSSAO();
			SharedData::getSingleton().iSystem->setSSAO(!curMode);
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
			->setCaption( "[C] Change mode, current is \"" + name  + "\". " +
			"[V] SSAO on ? " + StringConverter::toString( SharedData::getSingleton().iSystem->getSSAO() ) );

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

    //Utility function to help set scene up
    void setEntityHeight(Entity* ent, Real newHeight)
    {
        Real curHeight = ent->getMesh()->getBounds().getSize().y;
        Real scaleFactor = newHeight / curHeight;

        SceneNode* parentNode = ent->getParentSceneNode();
        parentNode->setScale(scaleFactor, scaleFactor, scaleFactor);
    }

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
        mSceneMgr->setAmbientLight(ColourValue(0.15, 0.00, 0.00));
        // Skybox
        mSceneMgr->setSkyBox(true, "DeferredDemo/SkyBox", 500);
        // Create main, static light
		Light* l1 = mSceneMgr->createLight();
        l1->setType(Light::LT_DIRECTIONAL);
        l1->setDiffuseColour(0.5f, 0.45f, 0.1f);
		l1->setDirection(1, -0.5, -0.2);
		l1->setShadowFarClipDistance(250);
		l1->setShadowFarDistance(75);
		//Turn this on to have the directional light cast shadows
		l1->setCastShadows(false);

		// Create "root" node
		SceneNode* rootNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

        // Create the cathedral - this will be the static scene
		Entity* cathedralEnt = mSceneMgr->createEntity("Cathedral", "sibenik.mesh");
        SceneNode* cathedralNode = rootNode->createChildSceneNode();
        cathedralNode->attachObject(cathedralEnt);
        //cathedralNode->scale(10, 20, 20);
		
        // Create ogre heads to decorate the wall
		Entity* ogreHead = mSceneMgr->createEntity("Head", "ogrehead.mesh");
		//rootNode->createChildSceneNode( "Head" )->attachObject( ogreHead );
        Vector3 headStartPos[2] = { Vector3(25.25,11,3), Vector3(25.25,11,-3) };
        Vector3 headDiff(-3.7,0,0);
        for (int i=0; i < 12; i++) 
        {
            char cloneName[16];
			sprintf(cloneName, "OgreHead%d", i);
            Entity* cloneHead = ogreHead->clone(cloneName);
            Vector3 clonePos = headStartPos[i%2] + headDiff*(i/2);
            if ((i/2) >= 4) clonePos.x -= 0.75;
			SceneNode* cloneNode = rootNode->createChildSceneNode(clonePos);
            cloneNode->attachObject(cloneHead);
            setEntityHeight(cloneHead, 1.5);
            if (i % 2 == 0)
            {
                cloneNode->yaw(Degree(180));
            }
        }


        // Create a pile of wood pallets
        Entity* woodPallet = mSceneMgr->createEntity("Pallet", "WoodPallet.mesh");
        Vector3 woodStartPos(10, 0.5, -5.5);
        Vector3 woodDiff(0, 0.3, 0);
        for (int i=0; i < 5; i++)
        {
            char cloneName[16];
			sprintf(cloneName, "WoodPallet%d", i);
            Entity* clonePallet = woodPallet->clone(cloneName);
            Vector3 clonePos = woodStartPos + woodDiff*i;
			SceneNode* cloneNode = rootNode->createChildSceneNode(clonePos);
            cloneNode->attachObject(clonePallet);
            setEntityHeight(clonePallet, 0.3);
            cloneNode->yaw(Degree(i*20));
        }

        // Create a bunch of knots
		Entity* knotEnt = mSceneMgr->createEntity("Knot", "knot.mesh");
		knotEnt->setMaterialName("DeferredDemo/RockWall");
		//knotEnt->setMeshLodBias(0.25f);
        Vector3 knotStartPos(25.5, 2, 5.5);
        Vector3 knotDiff(-3.7, 0, 0);
        for (int i=0; i < 5; i++)
        {
            char cloneName[16];
			sprintf(cloneName, "Knot%d", i);
            Entity* cloneKnot = knotEnt->clone(cloneName);
            Vector3 clonePos = knotStartPos + knotDiff*i;
			SceneNode* cloneNode = rootNode->createChildSceneNode(clonePos);
            cloneNode->attachObject(cloneKnot);
            setEntityHeight(cloneKnot, 3);
            cloneNode->yaw(Degree(i*17));
            cloneNode->roll(Degree(i*31));

            sprintf(cloneName, "KnotLight%d", i);
            Light* knotLight = mSceneMgr->createLight(cloneName);
            knotLight->setType(Light::LT_SPOTLIGHT);
            knotLight->setDiffuseColour(SAMPLE_COLORS[i]);
            knotLight->setSpecularColour(ColourValue::White);
            knotLight->setPosition(clonePos + Vector3(0,3,0));
            knotLight->setDirection(Vector3::NEGATIVE_UNIT_Y);
            knotLight->setSpotlightRange(Degree(25), Degree(45), 1);
            knotLight->setAttenuation(6, 1, 0.2, 0);
        }
		
		// Add a whole bunch of extra entities to fill the scene a bit
		//Entity *cloneEnt;
		//int N=4;
		//for (int n = 0; n < N; ++n)
		//{
		//	float theta = 2.0f*Math::PI*(float)n/(float)N;
		//	// Create a new node under the root
		//	SceneNode* node = mSceneMgr->createSceneNode();
		//	// Random translate
		//	Vector3 nodePos;
		//	nodePos.x = Math::SymmetricRandom() * 40.0 + Math::Sin(theta) * 500.0;
		//	nodePos.y = Math::SymmetricRandom() * 20.0 - 40.0;
		//	nodePos.z = Math::SymmetricRandom() * 40.0 + Math::Cos(theta) * 500.0;
		//	node->setPosition(nodePos);
		//	Quaternion orientation(Math::SymmetricRandom(),Math::SymmetricRandom(),Math::SymmetricRandom(),Math::SymmetricRandom());
		//	orientation.normalise();
		//	node->setOrientation(orientation);
		//	rootNode->addChild(node);
		//	// Clone knot
		//	char cloneName[12];
		//	sprintf(cloneName, "Knot%d", n);
		//	cloneEnt = knotEnt->clone(cloneName);
		//	// Attach to new node
		//	node->attachObject(cloneEnt);
		//}

        mCamera->setPosition(25, 5, 0);
        mCamera->lookAt(0,0,0);

		// show overlay
		Overlay* overlay = OverlayManager::getSingleton().getByName("Example/ShadowsOverlay");    
		overlay->show();

		mSystem = new DeferredShadingSystem(mWindow->getViewport(0), mSceneMgr, mCamera);
		SharedData::getSingleton().iSystem = mSystem;
		mSystem->initialize();
        
        
		//// Create a track for the light
  //      Animation* anim = mSceneMgr->createAnimation("LightTrack", 16);
  //      // Spline it for nice curves
  //      anim->setInterpolationMode(Animation::IM_SPLINE);
  //      // Create a track to animate the camera's node
  //      NodeAnimationTrack* track = anim->createNodeTrack(0, lightNode);
  //      // Setup keyframes
  //      TransformKeyFrame* key = track->createNodeKeyFrame(0); // A start position
  //      key->setTranslate(Vector3(300,300,-300));
  //      key = track->createNodeKeyFrame(4);//B
  //      key->setTranslate(Vector3(300,300,300));
  //      key = track->createNodeKeyFrame(8);//C
  //      key->setTranslate(Vector3(-300,300,300));
  //      key = track->createNodeKeyFrame(12);//D
  //      key->setTranslate(Vector3(-300,300,-300));
		//key = track->createNodeKeyFrame(16);//D
  //      key->setTranslate(Vector3(300,300,-300));
  //      // Create a new animation state to track this
  //      SharedData::getSingleton().mAnimState = mSceneMgr->createAnimationState("LightTrack");
  //      SharedData::getSingleton().mAnimState->setEnabled(true);

        //Create an athena statue
        Entity* athena = mSceneMgr->createEntity("Athena", "athene.mesh");
		athena->setMaterialName("DeferredDemo/DeferredAthena");
		SceneNode *aNode = rootNode->createChildSceneNode();
		aNode->attachObject( athena );
		aNode->setPosition(-8.5, 4.5, 0);
        setEntityHeight(athena, 4.0);
        aNode->yaw(Ogre::Degree(90));
		// Create some happy little lights to decorate the athena statue
		createSampleLights();

		// safely setup application's (not postfilter!) shared data
		SharedData::getSingleton().iCamera = mCamera;
		SharedData::getSingleton().iRoot = mRoot;
		SharedData::getSingleton().iWindow = mWindow;
		SharedData::getSingleton().iActivate = true;
		SharedData::getSingleton().iGlobalActivate = true;
		SharedData::getSingleton().iMainLight = l1;

        mCamera->setFarClipDistance(1000.0);
        mCamera->setNearClipDistance(0.5);
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
		vector<Light*>::type lights;
		SceneNode *parentNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("LightsParent");
		// Create light nodes
		vector<Node*>::type nodes;

        Vector4 attParams = Vector4(3,1,0,5);
        Real lightRadius = 25;

		Light *a = mSceneMgr->createLight();
		SceneNode *an = parentNode->createChildSceneNode();
		an->attachObject(a);
		a->setAttenuation(attParams.x, attParams.y, attParams.z, attParams.w);
		//a->setAttenuation(1.0f, 0.000f, 0.000f);
		an->setPosition(0,0,lightRadius);
		a->setDiffuseColour(1,0,0);
		//a->setSpecularColour(0.5,0,0);
		lights.push_back(a);
		nodes.push_back(an);

		Light *b = mSceneMgr->createLight();
		SceneNode *bn = parentNode->createChildSceneNode();
		bn->attachObject(b);
		b->setAttenuation(attParams.x, attParams.y, attParams.z, attParams.w);
		bn->setPosition(lightRadius,0,0);
		b->setDiffuseColour(1,1,0);
		//b->setSpecularColour(0.5,0.5,0);
		lights.push_back(b);
		nodes.push_back(bn);

		Light *c = mSceneMgr->createLight();
		SceneNode *cn = parentNode->createChildSceneNode();
		cn->attachObject(c);
		c->setAttenuation(attParams.x, attParams.y, attParams.z, attParams.w);
		cn->setPosition(0,0,-lightRadius);
		c->setDiffuseColour(0,1,1);
		c->setSpecularColour(0.25,1.0,1.0); // Cyan light has specular component
		lights.push_back(c);
		nodes.push_back(cn);

		Light *d = mSceneMgr->createLight();
		SceneNode *dn = parentNode->createChildSceneNode();
		dn->attachObject(d);
		d->setAttenuation(attParams.x, attParams.y, attParams.z, attParams.w);
		dn->setPosition(-lightRadius,0,0);
		d->setDiffuseColour(1,0,1);
		d->setSpecularColour(0.0,0,0.0);
		lights.push_back(d);
		nodes.push_back(dn);

		Light *e = mSceneMgr->createLight();
		SceneNode *en = parentNode->createChildSceneNode();
		en->attachObject(e);
		e->setAttenuation(attParams.x, attParams.y, attParams.z, attParams.w);
		en->setPosition(lightRadius,0,lightRadius);
		e->setDiffuseColour(0,0,1);
		e->setSpecularColour(0,0,0);
		lights.push_back(e);
		nodes.push_back(en);
		
		Light *f = mSceneMgr->createLight();
		SceneNode *fn = parentNode->createChildSceneNode();
		fn->attachObject(f);
		f->setAttenuation(attParams.x, attParams.y, attParams.z, attParams.w);
		fn->setPosition(-lightRadius,0,-lightRadius);
		f->setDiffuseColour(0,1,0);
		f->setSpecularColour(0,0.0,0.0);
		lights.push_back(f);
		nodes.push_back(fn);

		// Create marker meshes to show user where the lights are
		Entity *ent;
		GeomUtils::createSphere("PointLightMesh", 0.05f, 5, 5, true, true);
		for(vector<Light*>::type::iterator i=lights.begin(); i!=lights.end(); ++i)
		{
			Light* light = *i;
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
			//ent->setRenderQueueGroup(light->getRenderQueueGroup());
			ent->setRenderQueueGroup(DeferredShadingSystem::POST_GBUFFER_RENDER_QUEUE);
			static_cast<SceneNode*>(light->getParentNode())->attachObject(ent);
			ent->setVisible(true);
		}		

		// Store nodes for hiding/showing
		SharedData::getSingleton().mLightNodes = nodes;

		// Do some animation for node a-f
		// Generate helix structure
		float seconds_per_station = 1.0f;
		float r = 1.0;
		//Vector3 base(0,-30,0);
		Vector3 base(-8.75, 3.5, 0);

		float h=3;
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

		/*Light* spotLight = mSceneMgr->createLight("Spotlight1");
		spotLight->setType(Light::LT_SPOTLIGHT);
		spotLight->setAttenuation(200, 1.0f, 0, 0);
		spotLight->setSpotlightRange(Degree(30.0), Degree(45.0), 0.8);
		spotLight->setPosition(0,120,0);
		spotLight->setDirection(0, -1, 0);
		spotLight->setDiffuseColour(1,1,1);
		spotLight->setSpecularColour(1,1,1);*/
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


