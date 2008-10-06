/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/
/* Static water simulation by eru
 * Started 29.05.2003, 20:54:37
 */
#include "ExampleApplication.h"
#include "OgreBillboardParticleRenderer.h"
#include "WaterMesh.h"

#include <iostream>

AnimationState* mAnimState;

// Mesh stuff
#define MESH_NAME "WaterMesh"
#define ENTITY_NAME "WaterEntity"
#define MATERIAL_PREFIX "Examples/Water"
#define MATERIAL_NAME "Examples/Water0"
#define COMPLEXITY 64 		// watch out - number of polys is 2*ACCURACY*ACCURACY !
#define PLANE_SIZE 3000.0f
#define CIRCLES_MATERIAL "Examples/Water/Circles"

/* Some global variables */
SceneNode *headNode ;
Overlay* waterOverlay ;
ParticleSystem *particleSystem ;
ParticleEmitter *particleEmitter ;
SceneManager *sceneMgr ;

void prepareCircleMaterial()
{
	char *bmap = new char[256 * 256 * 4] ;
	memset(bmap, 127, 256 * 256 * 4);
	for(int b=0;b<16;b++) {
		int x0 = b % 4 ;
		int y0 = b >> 2 ;
		Real radius = 4.0f + 1.4 * (float) b ;
		for(int x=0;x<64;x++) {
			for(int y=0;y<64;y++) {
				Real dist = Math::Sqrt((x-32)*(x-32)+(y-32)*(y-32)); // 0..ca.45
				dist = fabs(dist -radius -2) / 2.0f ;
				dist = dist * 255.0f;
				if (dist>255)
					dist=255 ;
				int colour = 255-(int)dist ;
				colour = (int)( ((Real)(15-b))/15.0f * (Real) colour );

				bmap[4*(256*(y+64*y0)+x+64*x0)+0]=colour ;
				bmap[4*(256*(y+64*y0)+x+64*x0)+1]=colour ;
				bmap[4*(256*(y+64*y0)+x+64*x0)+2]=colour ;
				bmap[4*(256*(y+64*y0)+x+64*x0)+3]=colour ;
			}
		}
	}

	DataStreamPtr imgstream(new MemoryDataStream(bmap, 256 * 256 * 4));
	//~ Image img;
	//~ img.loadRawData( imgstream, 256, 256, PF_A8R8G8B8 );
	//~ TextureManager::getSingleton().loadImage( CIRCLES_MATERIAL , img );
	TextureManager::getSingleton().loadRawData(CIRCLES_MATERIAL,
        ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		imgstream, 256, 256, PF_A8R8G8B8);
	MaterialPtr material =
		MaterialManager::getSingleton().create( CIRCLES_MATERIAL,
        ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	TextureUnitState *texLayer = material->getTechnique(0)->getPass(0)->createTextureUnitState( CIRCLES_MATERIAL );
	texLayer->setTextureAddressingMode( TextureUnitState::TAM_CLAMP );
	material->setSceneBlending( SBT_ADD );
	material->setDepthWriteEnabled( false ) ;
    material->load();
    // finished with bmap so release the memory
    delete [] bmap;
}


/* =========================================================================*/
/*               WaterCircle class                                          */
/* =========================================================================*/
#define CIRCLE_SIZE 500.0
#define CIRCLE_TIME 0.5f
class WaterCircle
{
private:
	String name ;
	SceneNode *node ;
	MeshPtr mesh ;
	SubMesh *subMesh ;
	Entity *entity ;
	Real tm ;
	static bool first ;
	// some buffers shared by all circles
	static HardwareVertexBufferSharedPtr posnormVertexBuffer ;
	static HardwareIndexBufferSharedPtr indexBuffer ; // indices for 2 faces
	static HardwareVertexBufferSharedPtr *texcoordsVertexBuffers ;

	float *texBufData;
	void _prepareMesh()
	{
		int i,lvl ;

		mesh = MeshManager::getSingleton().createManual(name,
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME) ;
		subMesh = mesh->createSubMesh();
		subMesh->useSharedVertices=false;

		int numVertices = 4 ;

		if (first) { // first Circle, create some static common data
			first = false ;

			// static buffer for position and normals
			posnormVertexBuffer =
				HardwareBufferManager::getSingleton().createVertexBuffer(
					6*sizeof(float), // size of one vertex data
					4, // number of vertices
					HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
					false); // no shadow buffer
			float *posnormBufData = (float*) posnormVertexBuffer->
				lock(HardwareBuffer::HBL_DISCARD);
			for(i=0;i<numVertices;i++) {
				posnormBufData[6*i+0]=((Real)(i%2)-0.5f)*CIRCLE_SIZE; // pos X
				posnormBufData[6*i+1]=0; // pos Y
				posnormBufData[6*i+2]=((Real)(i/2)-0.5f)*CIRCLE_SIZE; // pos Z
				posnormBufData[6*i+3]=0 ; // normal X
				posnormBufData[6*i+4]=1 ; // normal Y
				posnormBufData[6*i+5]=0 ; // normal Z
			}
			posnormVertexBuffer->unlock();

			// static buffers for 16 sets of texture coordinates
			texcoordsVertexBuffers = new HardwareVertexBufferSharedPtr[16];
			for(lvl=0;lvl<16;lvl++) {
				texcoordsVertexBuffers[lvl] =
					HardwareBufferManager::getSingleton().createVertexBuffer(
						2*sizeof(float), // size of one vertex data
						numVertices, // number of vertices
						HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
						false); // no shadow buffer
				float *texcoordsBufData = (float*) texcoordsVertexBuffers[lvl]->
					lock(HardwareBuffer::HBL_DISCARD);
				float x0 = (Real)(lvl % 4) * 0.25 ;
				float y0 = (Real)(lvl / 4) * 0.25 ;
				y0 = 0.75-y0 ; // upside down
				for(i=0;i<4;i++) {
					texcoordsBufData[i*2 + 0]=
						x0 + 0.25 * (Real)(i%2) ;
					texcoordsBufData[i*2 + 1]=
						y0 + 0.25 * (Real)(i/2) ;
				}
				texcoordsVertexBuffers[lvl]->unlock();
			}

			// Index buffer for 2 faces
			unsigned short faces[6] = {2,1,0,  2,3,1};
			indexBuffer =
				HardwareBufferManager::getSingleton().createIndexBuffer(
					HardwareIndexBuffer::IT_16BIT,
					6,
					HardwareBuffer::HBU_STATIC_WRITE_ONLY);
			indexBuffer->writeData(0,
				indexBuffer->getSizeInBytes(),
				faces,
				true); // true?
		}

		// Initialize vertex data
		subMesh->vertexData = new VertexData();
		subMesh->vertexData->vertexStart = 0;
		subMesh->vertexData->vertexCount = 4;
		// first, set vertex buffer bindings
		VertexBufferBinding *vbind = subMesh->vertexData->vertexBufferBinding ;
		vbind->setBinding(0, posnormVertexBuffer);
		vbind->setBinding(1, texcoordsVertexBuffers[0]);
		// now, set vertex buffer declaration
		VertexDeclaration *vdecl = subMesh->vertexData->vertexDeclaration ;
		vdecl->addElement(0, 0, VET_FLOAT3, VES_POSITION);
		vdecl->addElement(0, 3*sizeof(float), VET_FLOAT3, VES_NORMAL);
		vdecl->addElement(1, 0, VET_FLOAT2, VES_TEXTURE_COORDINATES);

		// Initialize index data
		subMesh->indexData->indexBuffer = indexBuffer;
		subMesh->indexData->indexStart = 0;
		subMesh->indexData->indexCount = 6;

		// set mesh bounds
		AxisAlignedBox circleBounds(-CIRCLE_SIZE/2.0f, 0, -CIRCLE_SIZE/2.0f,
			CIRCLE_SIZE/2.0f, 0, CIRCLE_SIZE/2.0f);
		mesh->_setBounds(circleBounds);
        mesh->load();
        mesh->touch();
	}
public:
	int lvl ;
	void setTextureLevel()
	{
		subMesh->vertexData->vertexBufferBinding->setBinding(1, texcoordsVertexBuffers[lvl]);
	}
	WaterCircle(const String& name, Real x, Real y)
	{
		this->name = name ;
		_prepareMesh();
		node = static_cast<SceneNode*> (sceneMgr->getRootSceneNode()->createChild(name));
		node->translate(x*(PLANE_SIZE/COMPLEXITY), 10, y*(PLANE_SIZE/COMPLEXITY));
		entity = sceneMgr->createEntity(name, name);
		entity->setMaterialName(CIRCLES_MATERIAL);
		node->attachObject(entity);
		tm = 0 ;
		lvl = 0 ;
		setTextureLevel();
	}
	~WaterCircle()
	{
		MeshManager::getSingleton().remove(mesh->getHandle());
		sceneMgr->destroyEntity(entity->getName());
		static_cast<SceneNode*> (sceneMgr->getRootSceneNode())->removeChild(node->getName());
	}
	void animate(Real timeSinceLastFrame)
	{
		int lastlvl = lvl ;
		tm += timeSinceLastFrame ;
		lvl = (int) ( (Real)(tm)/CIRCLE_TIME * 16 );
		if (lvl<16 && lvl!=lastlvl) {
			setTextureLevel();
		}
	}
	static void clearStaticBuffers()
	{
		posnormVertexBuffer = HardwareVertexBufferSharedPtr() ;
		indexBuffer = HardwareIndexBufferSharedPtr() ;
		for(int i=0;i<16;i++) {
			texcoordsVertexBuffers[i] = HardwareVertexBufferSharedPtr() ;
		}
		delete [] texcoordsVertexBuffers;
	}
} ;
bool WaterCircle::first = true ;
HardwareVertexBufferSharedPtr WaterCircle::posnormVertexBuffer =
	HardwareVertexBufferSharedPtr() ;
HardwareIndexBufferSharedPtr WaterCircle::indexBuffer =
	HardwareIndexBufferSharedPtr() ;
HardwareVertexBufferSharedPtr* WaterCircle::texcoordsVertexBuffers = 0 ;

/* =========================================================================*/
/*               WaterListener class                                          */
/* =========================================================================*/
// Event handler
class WaterListener: public ExampleFrameListener
{
protected:
	WaterMesh *waterMesh ;
	Entity *waterEntity ;
	int materialNumber ;
	bool skyBoxOn ;
	Real timeoutDelay ;

#define RAIN_HEIGHT_RANDOM 5
#define RAIN_HEIGHT_CONSTANT 5


	typedef std::vector<WaterCircle*> WaterCircles ;
	WaterCircles circles ;

	void processCircles(Real timeSinceLastFrame)
	{
		for(unsigned int i=0;i<circles.size();i++) {
			circles[i]->animate(timeSinceLastFrame);
		}
		bool found ;
		do {
			found = false ;
			for(WaterCircles::iterator it = circles.begin() ;
					it != circles.end();
					++it) {
				if ((*it)->lvl>=16) {
					delete (*it);
					circles.erase(it);
					found = true ;
					break ;
				}
			}
		} while (found) ;
	}

	void processParticles()
	{
		static int pindex = 0 ;
		ParticleIterator pit = particleSystem->_getIterator() ;
		while(!pit.end()) {
			Particle *particle = pit.getNext();
			Vector3 ppos = particle->position;
			if (ppos.y<=0 && particle->timeToLive>0) { // hits the water!
				// delete particle
				particle->timeToLive = 0.0f;
				// push the water
				float x = ppos.x / PLANE_SIZE * COMPLEXITY ;
				float y = ppos.z / PLANE_SIZE * COMPLEXITY ;
				float h = rand() % RAIN_HEIGHT_RANDOM + RAIN_HEIGHT_CONSTANT ;
				if (x<1) x=1 ;
				if (x>COMPLEXITY-1) x=COMPLEXITY-1;
				if (y<1) y=1 ;
				if (y>COMPLEXITY-1) y=COMPLEXITY-1;
				waterMesh->push(x,y,-h) ;
				WaterCircle *circle = new WaterCircle(
					"Circle#"+StringConverter::toString(pindex++),
					x, y);
				circles.push_back(circle);
			}
		}
	}

	/** Head animation */
	Real headDepth ;
	void animateHead(Real timeSinceLastFrame)
	{
		// sine track? :)
		static double sines[4] = {0,100,200,300};
		static const double adds[4] = {0.3,-1.6,1.1,0.5};
		static Vector3 oldPos = Vector3::UNIT_Z;
		for(int i=0;i<4;i++) {
			sines[i]+=adds[i]*timeSinceLastFrame;
		}
		Real tx = ((sin(sines[0]) + sin(sines[1])) / 4 + 0.5 ) * (float)(COMPLEXITY-2) + 1 ;
		Real ty = ((sin(sines[2]) + sin(sines[3])) / 4 + 0.5 ) * (float)(COMPLEXITY-2) + 1 ;
		waterMesh->push(tx,ty, -headDepth);
		Real step = PLANE_SIZE / COMPLEXITY ;
		headNode->resetToInitialState();
		headNode->scale(3,3,3);
		Vector3 newPos = Vector3(step*tx, headDepth, step*ty);
		Vector3 diffPos = newPos - oldPos ;
		Quaternion headRotation = Vector3::UNIT_Z.getRotationTo(diffPos);
		oldPos = newPos ;
		headNode->translate(newPos);
		headNode->rotate(headRotation);
	}

	// GUI updaters
	void updateInfoParamC()
	{
		OverlayManager::getSingleton().getOverlayElement("Example/Water/Param_C") \
			->setCaption("[1/2]Ripple speed: "+StringConverter::toString(waterMesh->PARAM_C));
	}
	void updateInfoParamD()
	{
		OverlayManager::getSingleton().getOverlayElement("Example/Water/Param_D") \
			->setCaption("[3/4]Distance: "+StringConverter::toString(waterMesh->PARAM_D));
	}
	void updateInfoParamU()
	{
		OverlayManager::getSingleton().getOverlayElement("Example/Water/Param_U") \
			->setCaption("[5/6]Viscosity: "+StringConverter::toString(waterMesh->PARAM_U));
	}
	void updateInfoParamT()
	{
		OverlayManager::getSingleton().getOverlayElement("Example/Water/Param_T") \
			->setCaption("[7/8]Frame time: "+StringConverter::toString(waterMesh->PARAM_T));
	}
	void updateInfoNormals()
	{
		OverlayManager::getSingleton().getOverlayElement("Example/Water/Normals") \
			->setCaption(String("[N]Normals: ")+((waterMesh->useFakeNormals)?"fake":"real"));
	}
	void switchNormals()
	{
		waterMesh->useFakeNormals = !waterMesh->useFakeNormals ;
		updateInfoNormals() ;
	}
	void updateInfoHeadDepth()
	{
		OverlayManager::getSingleton().getOverlayElement("Example/Water/Depth") \
			->setCaption(String("[U/J]Head depth: ")+StringConverter::toString(headDepth));
	}
	void updateInfoSkyBox()
	{
		OverlayManager::getSingleton().getOverlayElement("Example/Water/SkyBox")
			->setCaption(String("[B]SkyBox: ")+String((skyBoxOn)?"On":"Off") );
	}
	void updateMaterial()
	{
		String materialName = MATERIAL_PREFIX+StringConverter::toString(materialNumber);
		MaterialPtr material = MaterialManager::getSingleton().getByName(materialName);
		if (material.isNull())
        {
			if(materialNumber)
            {
				materialNumber = 0 ;
				updateMaterial();
				return ;
			}
            else
            {
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
					"Material "+materialName+"doesn't exist!",
					"WaterListener::updateMaterial");
			}
		}
		waterEntity->setMaterialName(materialName);
		OverlayManager::getSingleton().getOverlayElement("Example/Water/Material") \
			->setCaption(String("[M]Material: ")+materialName);
	}

	void switchMaterial()
	{
		materialNumber++;
		updateMaterial();
	}
	void switchSkyBox()
	{
		skyBoxOn = !skyBoxOn;
		sceneMgr->setSkyBox(skyBoxOn, "Examples/SceneSkyBox2");
		updateInfoSkyBox();
	}

public:
    WaterListener(RenderWindow* win, Camera* cam,
		WaterMesh *waterMesh, Entity *waterEntity)
        : ExampleFrameListener(win, cam)
    {
		this->waterMesh = waterMesh ;
		this->waterEntity = waterEntity ;
		materialNumber = 8;
		timeoutDelay = 0.0f;
		headDepth = 2.0f;
		skyBoxOn = false ;

		updateMaterial();
		updateInfoParamC();
		updateInfoParamD();
		updateInfoParamU();
		updateInfoParamT();
		updateInfoNormals();
		updateInfoHeadDepth();
		updateInfoSkyBox();
    }

 	virtual ~WaterListener ()
 	{
 		// If when you finish the application is still raining there
 		// are water circles that are still being processed
 		unsigned int activeCircles = this->circles.size ();

 		// Kill the active water circles
 		for (unsigned int i = 0; i < activeCircles; i++)
 			delete (this->circles[i]);
 	}

    bool frameRenderingQueued(const FrameEvent& evt)
    {

		if( ExampleFrameListener::frameRenderingQueued(evt) == false )
		{
			// check if we are exiting, if so, clear static HardwareBuffers to avoid segfault
			WaterCircle::clearStaticBuffers();
			return false;
		}

        mAnimState->addTime(evt.timeSinceLastFrame);

		// process keyboard events
		Real changeSpeed = evt.timeSinceLastFrame ;

		// adjust keyboard speed with SHIFT (increase) and CONTROL (decrease)
		if (mKeyboard->isKeyDown(OIS::KC_LSHIFT) || mKeyboard->isKeyDown(OIS::KC_RSHIFT)) {
			changeSpeed *= 10.0f ;
		}
		if (mKeyboard->isKeyDown(OIS::KC_LCONTROL)) { 
			changeSpeed /= 10.0f ;
		}

		// rain
		processCircles(evt.timeSinceLastFrame);
		if (mKeyboard->isKeyDown(OIS::KC_SPACE)) {
			particleEmitter->setEmissionRate(20.0f);
		} else {
			particleEmitter->setEmissionRate(0.0f);
		}
		processParticles();

		// adjust values (some macros for faster change
#define ADJUST_RANGE(_value,_plus,_minus,_minVal,_maxVal,_change,_macro) {\
	if (mKeyboard->isKeyDown(_plus)) \
		{ _value+=_change ; if (_value>=_maxVal) _value = _maxVal ; _macro ; } ; \
	if (mKeyboard->isKeyDown(_minus)) \
		{ _value-=_change; if (_value<=_minVal) _value = _minVal ; _macro ; } ; \
}

		ADJUST_RANGE(headDepth, OIS::KC_U, OIS::KC_J, 0, 10, 0.5*changeSpeed, updateInfoHeadDepth()) ;

		ADJUST_RANGE(waterMesh->PARAM_C, OIS::KC_2, OIS::KC_1, 0, 10, 0.1f*changeSpeed, updateInfoParamC()) ;

		ADJUST_RANGE(waterMesh->PARAM_D, OIS::KC_4, OIS::KC_3, 0.1, 10, 0.1f*changeSpeed, updateInfoParamD()) ;

		ADJUST_RANGE(waterMesh->PARAM_U, OIS::KC_6, OIS::KC_5, -2, 10, 0.1f*changeSpeed, updateInfoParamU()) ;

		ADJUST_RANGE(waterMesh->PARAM_T, OIS::KC_8, OIS::KC_7, 0, 10, 0.1f*changeSpeed, updateInfoParamT()) ;

		timeoutDelay-=evt.timeSinceLastFrame ;
		if (timeoutDelay<=0)
			timeoutDelay = 0;

#define SWITCH_VALUE(_key,_timeDelay, _macro) { \
		if (mKeyboard->isKeyDown(_key) && timeoutDelay==0) { \
			timeoutDelay = _timeDelay ; _macro ;} }

		SWITCH_VALUE(OIS::KC_N, 0.5f, switchNormals());

		SWITCH_VALUE(OIS::KC_M, 0.5f, switchMaterial());

		SWITCH_VALUE(OIS::KC_B, 0.5f, switchSkyBox());

		animateHead(evt.timeSinceLastFrame);

		waterMesh->updateMesh(evt.timeSinceLastFrame);

		return true;
    }
};

class WaterApplication : public ExampleApplication
{
public:
    WaterApplication()
        : waterMesh(0)
    {

    }

    ~WaterApplication() {
        delete waterMesh;
    }

protected:
	WaterMesh *waterMesh ;
	Entity *waterEntity ;

// Just override the mandatory create scene method
    void createScene(void)
    {
		sceneMgr = mSceneMgr ;
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.75, 0.75, 0.75));

        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(200,300,100);

		// Create water mesh and entity
		waterMesh = new WaterMesh(MESH_NAME, PLANE_SIZE, COMPLEXITY);
		waterEntity = mSceneMgr->createEntity(ENTITY_NAME,
			MESH_NAME);
		//~ waterEntity->setMaterialName(MATERIAL_NAME);
		SceneNode *waterNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		waterNode->attachObject(waterEntity);

        // Add a head, give it it's own node
        headNode = waterNode->createChildSceneNode();
        Entity *ent = mSceneMgr->createEntity("head", "ogrehead.mesh");
        headNode->attachObject(ent);

		// Make sure the camera track this node
        //~ mCamera->setAutoTracking(true, headNode);

		// Create the camera node, set its position & attach camera
        SceneNode* camNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		camNode->translate(0, 500, PLANE_SIZE);
		camNode->yaw(Degree(-45));
        camNode->attachObject(mCamera);

		// Create light node
        SceneNode* lightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		lightNode->attachObject(l);

        // set up spline animation of light node
        Animation* anim = mSceneMgr->createAnimation("WaterLight", 20);
		NodeAnimationTrack *track ;
        TransformKeyFrame *key ;
		// create a random spline for light
		track = anim->createNodeTrack(0, lightNode);
		key = track->createNodeKeyFrame(0);
		for(int ff=1;ff<=19;ff++) {
			key = track->createNodeKeyFrame(ff);
			Vector3 lpos (
				rand()%(int)PLANE_SIZE , //- PLANE_SIZE/2,
				rand()%300+100,
				rand()%(int)PLANE_SIZE //- PLANE_SIZE/2
				);
			key->setTranslate(lpos);
		}
		key = track->createNodeKeyFrame(20);

        // Create a new animation state to track this
        mAnimState = mSceneMgr->createAnimationState("WaterLight");
        mAnimState->setEnabled(true);

        // Put in a bit of fog for the hell of it
        //mSceneMgr->setFog(FOG_EXP, ColourValue::White, 0.0002);

		// show overlay
		waterOverlay = OverlayManager::getSingleton().getByName("Example/WaterOverlay");
		waterOverlay->show();

        // Let there be rain
        particleSystem = mSceneMgr->createParticleSystem("rain",
            "Examples/Water/Rain");
		particleEmitter = particleSystem->getEmitter(0);
        SceneNode* rNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        rNode->translate(PLANE_SIZE/2.0f, 3000, PLANE_SIZE/2.0f);
        rNode->attachObject(particleSystem);
        // Fast-forward the rain so it looks more natural
        particleSystem->fastForward(20);
		// It can't be set in .particle file, and we need it ;)
		static_cast<BillboardParticleRenderer*>(particleSystem->getRenderer())->setBillboardOrigin(BBO_BOTTOM_CENTER);

		prepareCircleMaterial();
	}

    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new WaterListener(mWindow, mCamera, waterMesh, waterEntity);
        mRoot->addFrameListener(mFrameListener);
    }

};



#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char **argv)
#endif
{
    // Create application object
    WaterApplication app;

	srand(time(0));

    try {
        app.go();
    } catch( Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: " << e.getFullDescription();
#endif
    }

    return 0;
}

#ifdef __cplusplus
}
#endif
