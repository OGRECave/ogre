/*
 -----------------------------------------------------------------------------
 This source file is part of OGRE
 (Object-oriented Graphics Rendering Engine)
 For the latest info, see http://www.ogre3d.org/
 
 Copyright (c) 2000-2009 Torus Knot Software Ltd
 Also see acknowledgements in Readme.html
 
 You may use this sample code for anything you like, it is not covered by the
 same license as the rest of the engine.
 -----------------------------------------------------------------------------
*/

#include "OgreConfigFile.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgreInstancedGeometry.h"
#include "SdkSample.h"
#include "SamplePlugin.h"

using namespace Ogre;
using namespace OgreBites;

#define maxObjectsPerBatch 80
#ifndef FLT_MAX
#  define FLT_MAX         3.402823466e+38F        /* max value */
#endif

const size_t numTypeMeshes = 4;

Ogre::String meshes[]=
{ 
	"razor", //0
	"knot", 
	"tudorhouse",
	"WoodPallet"//6
};

enum CurrentGeomOpt{
	INSTANCE_OPT,
	STATIC_OPT,
	ENTITY_OPT
};

class _OgreSampleClassExport  Sample_Instancing : public SdkSample
{
public:

	//-----------------------------------------------------------------------
	Sample_Instancing()  
	{ 
		mInfo["Title"] = "Instancing";
		mInfo["Description"] = "A demo of different methods to handle a large number of objects.";
		mInfo["Thumbnail"] = "thumb_instancing.png";
		mInfo["Category"] = "Geometry";
	}
	
	//-----------------------------------------------------------------------
	bool frameRenderingQueued(const FrameEvent& evt)
	{
		burnCPU();
		return SdkSample::frameRenderingQueued(evt);
	}

protected:

	void setObjectCount(size_t val)
	{
		mNumMeshes=val;
	};
	//-----------------------------------------------------------------------
	void setBurnedTime(double timeBurned)
	{
		mBurnAmount=timeBurned;
	};
	//-----------------------------------------------------------------------
	void changeSelectedMesh(size_t number)
	{	
		mSelectedMesh=number;
	}

	//-----------------------------------------------------------------------
	void burnCPU()
	{
		double mStartTime = mTimer->getMicroseconds()/1000000.0f; //convert into seconds
		double mCurTime =  mStartTime;
		double mStopTime = mLastTime + mBurnAmount;
		double mCPUUsage;

		while( mCurTime < mStopTime )
		{
			mCurTime = mTimer->getMicroseconds()/1000000.0f; //convert into seconds
		}

		if( mCurTime - mLastTime > 0.00001f )
			mCPUUsage = (mCurTime - mStartTime) / (mCurTime - mLastTime) * 100.0f;
		else
			mCPUUsage = FLT_MAX;

		mLastTime = mTimer->getMicroseconds()/1000000.0f; //convert into seconds
		int time = mCPUUsage+0.5f;
	}
	//-----------------------------------------------------------------------
	void destroyCurrentGeomOpt()
	{
		switch(mCurrentGeomOpt)
		{
		case INSTANCE_OPT: destroyInstanceGeom(); break;
		case STATIC_OPT: destroyStaticGeom(); break;
		case ENTITY_OPT: destroyEntityGeom(); break;
		}

		assert (mNumRendered == posMatrices.size ());
		for (size_t i = 0; i < mNumRendered; i++)
		{
			delete [] posMatrices[i];
		}

		posMatrices.clear();
	}
	//-----------------------------------------------------------------------
	void createCurrentGeomOpt()
	{
		objectCount = mNumMeshes;
		mNumRendered = 1;

		while(objectCount>maxObjectsPerBatch)
		{
			mNumRendered++;
			objectCount-=maxObjectsPerBatch;
		}

		assert (mSelectedMesh < numTypeMeshes);
		MeshPtr m = MeshManager::getSingleton ().getByName (meshes[mSelectedMesh] + ".mesh");
		if (m.isNull ())
		{
			m = MeshManager::getSingleton ().load (meshes[mSelectedMesh] + ".mesh", 
				ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
		}
		const Real radius = m->getBoundingSphereRadius ();

		// could/should print on screen mesh name, 
		//optimisation type, 
		//mesh vertices num, 
		//32 bit or not, 
		//etC..



		posMatrices.resize (mNumRendered);
		posMatrices.reserve (mNumRendered);


		vector <Vector3 *>::type posMatCurr;
		posMatCurr.resize (mNumRendered);
		posMatCurr.reserve (mNumRendered);
		for (size_t i = 0; i < mNumRendered; i++)
		{
			posMatrices[i] = new Vector3[mNumMeshes];
			posMatCurr[i] = posMatrices[i];
		}

		size_t i = 0, j = 0;
		for (size_t p = 0; p < mNumMeshes; p++)
		{
			for (size_t k = 0; k < mNumRendered; k++)
			{
				posMatCurr[k]->x = radius*i;
				posMatCurr[k]->y = k*radius;

				posMatCurr[k]->z = radius*j;
				posMatCurr[k]++;
			}
			if (++j== 10) 
			{
				j = 0;
				i++;
			}

		}
		posMatCurr.clear ();


		switch(mCurrentGeomOpt)
		{
		case INSTANCE_OPT:createInstanceGeom();break;
		case STATIC_OPT:createStaticGeom ();break;
		case ENTITY_OPT: createEntityGeom ();break;
		}
	}
	//-----------------------------------------------------------------------
	void createInstanceGeom()
	{
		if (Root::getSingleton ().getRenderSystem ()->getCapabilities ()->hasCapability (RSC_VERTEX_PROGRAM) == false)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Your video card doesn't support batching", "Demo_Instance::createScene");
		}

		Entity *ent = mSceneMgr->createEntity(meshes[mSelectedMesh], meshes[mSelectedMesh] + ".mesh");	


		renderInstance.reserve(mNumRendered);
		renderInstance.resize(mNumRendered);

		//Load a mesh to read data from.	
		InstancedGeometry* batch = new InstancedGeometry(mSceneMgr, 
			meshes[mSelectedMesh] + "s" );
		batch->setCastShadows(true);

		batch->setBatchInstanceDimensions (Vector3(1000000, 1000000, 1000000));
		const size_t batchSize = (mNumMeshes > maxObjectsPerBatch) ? maxObjectsPerBatch :mNumMeshes;
		setupInstancedMaterialToEntity(ent);
		for(size_t i = 0; i < batchSize ; i++)
		{
			batch->addEntity(ent, Vector3::ZERO);
		}
		batch->setOrigin(Vector3::ZERO);

		batch->build();


		for (size_t k = 0; k < mNumRendered-1; k++)
		{
			batch->addBatchInstance();
		}
		InstancedGeometry::BatchInstanceIterator regIt = batch->getBatchInstanceIterator();
		size_t k = 0;
		while (regIt.hasMoreElements ())
		{

			InstancedGeometry::BatchInstance *r = regIt.getNext();

			InstancedGeometry::BatchInstance::InstancedObjectIterator bit = r->getObjectIterator();
			int j = 0;
			while(bit.hasMoreElements())
			{
				InstancedGeometry::InstancedObject* obj = bit.getNext();

				const Vector3 position (posMatrices[k][j]);								
				obj->setPosition(position);
				++j;

			}
			k++;
			
		}
		batch->setVisible(true);
		renderInstance[0] = batch;

		mSceneMgr->destroyEntity (ent);
	}
	void setupInstancedMaterialToEntity(Entity*ent)
	{
		for (Ogre::uint i = 0; i < ent->getNumSubEntities(); ++i)
		{
			SubEntity* se = ent->getSubEntity(i);
			String materialName= se->getMaterialName();
			se->setMaterialName(buildInstancedMaterial(materialName));
		}
	}
	String buildInstancedMaterial(const String &originalMaterialName)
	{

		// already instanced ?
		if (StringUtil::endsWith (originalMaterialName, "/instanced"))
			return originalMaterialName;

		MaterialPtr originalMaterial = MaterialManager::getSingleton ().getByName (originalMaterialName);

		// if originalMat doesn't exists use "Instancing" material name
		const String instancedMaterialName (originalMaterial.isNull() ? "Instancing" : originalMaterialName + "/Instanced");
		MaterialPtr  instancedMaterial = MaterialManager::getSingleton ().getByName (instancedMaterialName);

		// already exists ?
		if (instancedMaterial.isNull())
		{
			instancedMaterial = originalMaterial->clone(instancedMaterialName);
			instancedMaterial->load();
			Technique::PassIterator pIt = instancedMaterial->getBestTechnique ()->getPassIterator();
			while (pIt.hasMoreElements())
			{

				Pass * const p = pIt.getNext();
				p->setVertexProgram("Instancing", false);
				p->setShadowCasterVertexProgram("InstancingShadowCaster");


			}
		}
		instancedMaterial->load();
		return instancedMaterialName;


	}
	//-----------------------------------------------------------------------
	void destroyInstanceGeom()
	{
		delete renderInstance[0];
		renderInstance.clear();
	}
	//-----------------------------------------------------------------------
	void createStaticGeom()
	{
		Entity *ent = mSceneMgr->createEntity(meshes[mSelectedMesh], meshes[mSelectedMesh] + ".mesh");	

		renderStatic.reserve (mNumRendered);
		renderStatic.resize (mNumRendered);

		StaticGeometry* geom = new StaticGeometry (mSceneMgr, 
			meshes[mSelectedMesh] + "s");

		geom->setRegionDimensions (Vector3(1000000, 1000000, 1000000));
		size_t k = 0;
		size_t y = 0;
		for (size_t i = 0; i < mNumMeshes; i++)
		{
			if (y==maxObjectsPerBatch)
			{
				y=0;
				k++;
			}
			geom->addEntity (ent, posMatrices[k][y]);
			y++;
		}
		geom->setCastShadows(true);
		geom->build ();
		renderStatic[0] = geom;
		mSceneMgr->destroyEntity (ent);
	}
	//-----------------------------------------------------------------------
	void destroyStaticGeom()
	{

		delete renderStatic[0];

		renderStatic.clear();
	}
	//-----------------------------------------------------------------------
	void createEntityGeom()
	{
		size_t k = 0;
		size_t y = 0;
		renderEntity.reserve (mNumMeshes);
		renderEntity.resize (mNumMeshes);
		nodes.reserve (mNumMeshes);
		nodes.resize (mNumMeshes);

		for (size_t i = 0; i < mNumMeshes; i++)
		{
			if (y==maxObjectsPerBatch)
			{
				y=0;
				k++;
			}

			nodes[i]=mSceneMgr->getRootSceneNode()->createChildSceneNode("node"+StringConverter::toString(i));
			renderEntity[i]=mSceneMgr->createEntity(meshes[mSelectedMesh]+StringConverter::toString(i), meshes[mSelectedMesh] + ".mesh");	
			nodes[i]->attachObject(renderEntity[i]);
			nodes[i]->setPosition(posMatrices[k][y]);

			y++;
		}

	}
	//-----------------------------------------------------------------------
	void destroyEntityGeom()
	{
		size_t i;
		size_t j=0;
		for (i=0;i<mNumMeshes;i++)
		{
			String name=nodes[i]->getName();
			mSceneMgr->destroySceneNode(name);
			mSceneMgr->destroyEntity(renderEntity[i]);
			j++;
		}
	}

	void setCurrentGeometryOpt(CurrentGeomOpt opt)
	{
		mCurrentGeomOpt=opt;
	}
   
	//-----------------------------------------------------------------------
	// Just override the mandatory create scene method
	void setupContent()
	{
		// Set ambient light
		mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));
		Light* l = mSceneMgr->createLight("MainLight");
		//add a skybox
		mSceneMgr->setSkyBox(true, "Examples/MorningSkyBox", 1000);
		//setup the light
		l->setType(Light::LT_DIRECTIONAL);
		l->setDirection(-0.5, -0.5, 0);

		mCamera->setPosition(500,500, 1500);
		mCamera->lookAt(0,0,0);
		setDragLook(true);

		   Plane plane;
        plane.normal = Vector3::UNIT_Y;
        plane.d = 100;
        MeshManager::getSingleton().createPlane("Myplane",
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
            1500,1500,20,20,true,1,5,5,Vector3::UNIT_Z);
        Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
        pPlaneEnt->setMaterialName("Examples/Rockwall");
        pPlaneEnt->setCastShadows(false);
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);
		
		CompositorManager::getSingleton().addCompositor(mViewport,"Bloom");

		setupControls();

		const GpuProgramManager::SyntaxCodes &syntaxCodes = GpuProgramManager::getSingleton().getSupportedSyntax();
		for (GpuProgramManager::SyntaxCodes::const_iterator iter = syntaxCodes.begin();iter != syntaxCodes.end();++iter)
		{
			LogManager::getSingleton().logMessage("supported syntax : "+(*iter));
		}
		
		mNumMeshes = 160;
		mNumRendered = 0;
		mSelectedMesh = 0;
		mBurnAmount = 0;
		mCurrentGeomOpt = INSTANCE_OPT;
		createCurrentGeomOpt();
		
		mTimer = new Ogre::Timer();
		mLastTime = mTimer->getMicroseconds() / 1000000.0f;
	}

	//-----------------------------------------------------------------------
	void setupControls()
	{
		SelectMenu* technique = mTrayMgr->createThickSelectMenu(TL_TOPLEFT, "TechniqueType", "Instancing Technique", 200, 3);
		technique->addItem("Instancing");
		technique->addItem("Static Geometry");
		technique->addItem("Independent Entities");
		
		SelectMenu* objectType = mTrayMgr->createThickSelectMenu(TL_TOPLEFT, "ObjectType", "Object : ", 200, 4);
		objectType->addItem("razor");
		objectType->addItem("knot");
		objectType->addItem("tudorhouse");
		objectType->addItem("woodpallet");

		mTrayMgr->createThickSlider(TL_TOPLEFT, "ObjectCountSlider", "Object count", 200, 50, 0, 1000, 101)->setValue(160, false);

		mTrayMgr->createThickSlider(TL_TOPLEFT, "CPUOccupationSlider", "CPU Load (ms)", 200, 75, 0, 1000.0f / 60, 20);

		mTrayMgr->createCheckBox(TL_TOPLEFT, "ShadowCheckBox", "Shadows", 200);

		mTrayMgr->createCheckBox(TL_TOPLEFT, "PostEffectCheckBox", "Post Effect", 200);

		mTrayMgr->showCursor();
	}

	void cleanupContent()
	{
		destroyCurrentGeomOpt();
		delete mTimer;
	}
	
	void sliderMoved(Slider* slider)
	{
		if (slider->getName() == "ObjectCountSlider")
		{
			destroyCurrentGeomOpt();
			setObjectCount((size_t)slider->getValue());
			createCurrentGeomOpt();
		}
		else if (slider->getName() == "CPUOccupationSlider")
		{
			setBurnedTime(slider->getValue() / 1000.0f);
		}
	}

	void itemSelected(SelectMenu* menu)
	{
		if (menu->getName() == "TechniqueType")
		{
			//Menu items are synchronized with enum
			CurrentGeomOpt selectedOption = (CurrentGeomOpt)menu->getSelectionIndex();
			destroyCurrentGeomOpt();
			setCurrentGeometryOpt(selectedOption);
			createCurrentGeomOpt();
		}
		else if (menu->getName() == "ObjectType")
		{
			destroyCurrentGeomOpt();
			changeSelectedMesh(menu->getSelectionIndex());
			createCurrentGeomOpt();
		}
	}

	void checkBoxToggled(CheckBox* box)
	{
		if (box->getName() == "ShadowCheckBox")
		{
			if (box->isChecked())
			{
				mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
			}
			else
			{
				mSceneMgr->setShadowTechnique(SHADOWTYPE_NONE);
			}
		} 
		else if (box->getName() == "PostEffectCheckBox")
		{
			CompositorManager::getSingleton().setCompositorEnabled(mViewport,"Bloom",box->isChecked());
		}
	}

	double mAvgFrameTime;
	size_t mSelectedMesh;
	size_t mNumMeshes;
	size_t objectCount;
	String mDebugText;
	CurrentGeomOpt mCurrentGeomOpt;

	size_t mNumRendered;

	Ogre::Timer*mTimer;
	double mLastTime, mBurnAmount;

	vector <InstancedGeometry *>::type renderInstance;
	vector <StaticGeometry *>::type renderStatic;
	vector <Entity *>::type renderEntity;
	vector <SceneNode *>::type nodes; 
	vector <Vector3 *>::type posMatrices;
};
