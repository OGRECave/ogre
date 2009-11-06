/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/

/**
\file 
Instancing.cpp
\brief
Shows OGRE's bezier instancing feature
*/

#include "Instancing.h"

InstancingListener::InstancingListener(Camera* cam,Sample_Instancing* main) :
mMain(main),
mCamera(cam),
mBurnAmount(0)
{ 
	const GpuProgramManager::SyntaxCodes &syntaxCodes = GpuProgramManager::getSingleton().getSupportedSyntax();
	for (GpuProgramManager::SyntaxCodes::const_iterator iter = syntaxCodes.begin();iter != syntaxCodes.end();++iter)
	{
		LogManager::getSingleton().logMessage("supported syntax : "+(*iter));
	}
	
	numMesh = 160;
	numRender = 0;
	meshSelected = 0;
	currentGeomOpt = INSTANCE_OPT;
	createCurrentGeomOpt();
	
	timer = new Ogre::Timer();
	mLastTime = timer->getMicroseconds()/1000000.0f;

}
//-----------------------------------------------------------------------
InstancingListener::~InstancingListener()
{
	destroyCurrentGeomOpt();
	delete timer;
}
//-----------------------------------------------------------------------
bool InstancingListener::frameRenderingQueued(const FrameEvent& evt)
{
	burnCPU();
	return true;
}
//-----------------------------------------------------------------------
void InstancingListener::burnCPU(void)
{
	double mStartTime = timer->getMicroseconds()/1000000.0f; //convert into seconds
	double mCurTime =  mStartTime;
	double mStopTime = mLastTime + mBurnAmount;
	double mCPUUsage;

	while( mCurTime < mStopTime )
	{
		mCurTime = timer->getMicroseconds()/1000000.0f; //convert into seconds
	}

	if( mCurTime - mLastTime > 0.00001f )
		mCPUUsage = (mCurTime - mStartTime) / (mCurTime - mLastTime) * 100.0f;
	else
		mCPUUsage = FLT_MAX;

	mLastTime = timer->getMicroseconds()/1000000.0f; //convert into seconds
	int time = mCPUUsage+0.5f;
}
//-----------------------------------------------------------------------
void InstancingListener::destroyCurrentGeomOpt()
{
	switch(currentGeomOpt)
	{
	case INSTANCE_OPT:destroyInstanceGeom();break;
	case STATIC_OPT:destroyStaticGeom ();break;
	case ENTITY_OPT: destroyEntityGeom ();break;
	}

	assert (numRender == posMatrices.size ());
	for (size_t i = 0; i < numRender; i++)
	{
		delete [] posMatrices[i];
	}
	posMatrices.clear();
}
//-----------------------------------------------------------------------
void InstancingListener::createCurrentGeomOpt()
{
	LogManager::getSingleton().logMessage("geom deleted");
	objectCount=numMesh;
	numRender=1;
	while(objectCount>maxObjectsPerBatch)
	{
		numRender++;
		objectCount-=maxObjectsPerBatch;
	}

	assert (meshSelected < numTypeMeshes);
	MeshPtr m = MeshManager::getSingleton ().getByName (meshes[meshSelected] + ".mesh");
	if (m.isNull ())
	{
		m = MeshManager::getSingleton ().load (meshes[meshSelected] + ".mesh", 
			ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
	}
	const Real radius = m->getBoundingSphereRadius ();

	// could/should print on screen mesh name, 
	//optimisation type, 
	//mesh vertices num, 
	//32 bit or not, 
	//etC..



	posMatrices.resize (numRender);
	posMatrices.reserve (numRender);


	vector <Vector3 *>::type posMatCurr;
	posMatCurr.resize (numRender);
	posMatCurr.reserve (numRender);
	for (size_t i = 0; i < numRender; i++)
	{
		posMatrices[i] = new Vector3[numMesh];
		posMatCurr[i] = posMatrices[i];
	}

	size_t i = 0, j = 0;
	for (size_t p = 0; p < numMesh; p++)
	{
		for (size_t k = 0; k < numRender; k++)
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


	switch(currentGeomOpt)
	{
	case INSTANCE_OPT:createInstanceGeom();break;
	case STATIC_OPT:createStaticGeom ();break;
	case ENTITY_OPT: createEntityGeom ();break;
	}
}
//-----------------------------------------------------------------------
void InstancingListener::createInstanceGeom()
{
	if (Root::getSingleton ().getRenderSystem ()->getCapabilities ()->hasCapability (RSC_VERTEX_PROGRAM) == false)
	{
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Your video card doesn't support batching", "Demo_Instance::createScene");
	}

	Entity *ent = mCamera->getSceneManager()->createEntity(meshes[meshSelected], meshes[meshSelected] + ".mesh");	


	renderInstance.reserve(numRender);
	renderInstance.resize(numRender);

	//Load a mesh to read data from.	
	InstancedGeometry* batch = new InstancedGeometry(mCamera->getSceneManager(), 
		meshes[meshSelected] + "s" );
	batch->setCastShadows(true);

	batch->setBatchInstanceDimensions (Vector3(1000000, 1000000, 1000000));
	const size_t batchSize = (numMesh > maxObjectsPerBatch) ? maxObjectsPerBatch :numMesh;
	setupInstancedMaterialToEntity(ent);
	for(size_t i = 0; i < batchSize ; i++)
	{
		batch->addEntity(ent, Vector3::ZERO);
	}
	batch->setOrigin(Vector3::ZERO);

	batch->build();


	for (size_t k = 0; k < numRender-1; k++)
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

	mCamera->getSceneManager()->destroyEntity (ent);
}
void InstancingListener::setupInstancedMaterialToEntity(Entity*ent)
{
	for (Ogre::uint i = 0; i < ent->getNumSubEntities(); ++i)
	{
		SubEntity* se = ent->getSubEntity(i);
		String materialName= se->getMaterialName();
		se->setMaterialName(buildInstancedMaterial(materialName));
	}
}
String InstancingListener::buildInstancedMaterial(const String &originalMaterialName)
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
void InstancingListener::destroyInstanceGeom()
{
	delete renderInstance[0];
	renderInstance.clear();
}
//-----------------------------------------------------------------------
void InstancingListener::createStaticGeom()
{
	Entity *ent = mCamera->getSceneManager()->createEntity(meshes[meshSelected], meshes[meshSelected] + ".mesh");	

	renderStatic.reserve (numRender);
	renderStatic.resize (numRender);

	StaticGeometry* geom = new StaticGeometry (mCamera->getSceneManager(), 
		meshes[meshSelected] + "s");

	geom->setRegionDimensions (Vector3(1000000, 1000000, 1000000));
	size_t k = 0;
	size_t y = 0;
	for (size_t i = 0; i < numMesh; i++)
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
	mCamera->getSceneManager ()->destroyEntity (ent);
}
//-----------------------------------------------------------------------
void InstancingListener::destroyStaticGeom()
{

	delete renderStatic[0];

	renderStatic.clear();
}
//-----------------------------------------------------------------------
void InstancingListener::createEntityGeom()
{
	size_t k = 0;
	size_t y = 0;
	renderEntity.reserve (numMesh);
	renderEntity.resize (numMesh);
	nodes.reserve (numMesh);
	nodes.resize (numMesh);

	for (size_t i = 0; i < numMesh; i++)
	{
		if (y==maxObjectsPerBatch)
		{
			y=0;
			k++;
		}
		LogManager::getSingleton().logMessage("marche3");
		nodes[i]=mCamera->getSceneManager()->getRootSceneNode()->createChildSceneNode("node"+StringConverter::toString(i));
		LogManager::getSingleton().logMessage(":"+nodes[i]->getName());
		renderEntity[i]=mCamera->getSceneManager()->createEntity(meshes[meshSelected]+StringConverter::toString(i), meshes[meshSelected] + ".mesh");	
		nodes[i]->attachObject(renderEntity[i]);
		nodes[i]->setPosition(posMatrices[k][y]);

		y++;
	}

}
//-----------------------------------------------------------------------
void InstancingListener::destroyEntityGeom()
{
	size_t i;
	size_t j=0;
	for (i=0;i<numMesh;i++)
	{
		LogManager::getSingleton().logMessage(" " +nodes[i]->getName());
		LogManager::getSingleton().logMessage(StringConverter::toString(j)+":"+StringConverter::toString(j<numMesh));
		String name=nodes[i]->getName();
		mCamera->getSceneManager()->destroySceneNode(name);
		mCamera->getSceneManager()->destroyEntity(renderEntity[i]);
		j++;
	}
}

void InstancingListener::setCurrentGeometryOpt(CurrentGeomOpt opt)
{
	currentGeomOpt=opt;
}

SamplePlugin* sp;
Sample* s;

extern "C" _OgreSampleExport void dllStartPlugin()
{
	s = new Sample_Instancing;
	sp = OGRE_NEW SamplePlugin(s->getInfo()["Title"] + " Sample");
	sp->addSample(s);
	Root::getSingleton().installPlugin(sp);
}

extern "C" _OgreSampleExport void dllStopPlugin()
{
	Root::getSingleton().uninstallPlugin(sp); 
	OGRE_DELETE sp;
	delete s;
}
