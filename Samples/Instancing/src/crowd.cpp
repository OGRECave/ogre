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
        Crowd.cpp
    \brief
        Shows OGRE's bezier Crowd feature
*/

#include "crowd.h"
inline Ogre::String operator +(const Ogre::String& l,const CEGUI::String& o)
{
	return l+o.c_str();
}
/*
inline CEGUI::String operator +(const CEGUI::String& l,const Ogre::String& o)
{
	return l+o.c_str();
}
*/

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char **argv)
#endif
{
    // Create application object
    CrowdApplication app;

    try {
        app.go();
    } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL );
#else
        std::cerr << "An exception has occured: " << e.getFullDescription();
#endif
    }


    return 0;
}
#ifdef __cplusplus
}
#endif

	CrowdListener::CrowdListener(RenderWindow* win, Camera* cam,CEGUI::Renderer* renderer, CrowdApplication*main)
		: ExampleFrameListener(win, cam,false,true),
		mRequestShutDown(false),
		mLMBDown(false),
		mRMBDown(false),
		mAvgFrameTime(0.1),
		mMain(main),
		mBurnAmount(0)
	{ 
		const GpuProgramManager::SyntaxCodes &syntaxCodes = GpuProgramManager::getSingleton().getSupportedSyntax();
		for (GpuProgramManager::SyntaxCodes::const_iterator iter = syntaxCodes.begin();iter != syntaxCodes.end();++iter)
		{
				LogManager::getSingleton().logMessage("supported syntax : "+(*iter));
		}
		mGUIRenderer=renderer;
		numMesh = 160;
		numRender = 0;
		meshSelected = 0;
		currentGeomOpt = INSTANCE_OPT;
		createCurrentGeomOpt();

		mMouse->setEventCallback(this);
		mKeyboard->setEventCallback(this);

		mGuiAvg   = CEGUI::WindowManager::getSingleton().getWindow("OPAverageFPS");
		mGuiCurr  = CEGUI::WindowManager::getSingleton().getWindow("OPCurrentFPS");
		mGuiBest  = CEGUI::WindowManager::getSingleton().getWindow("OPBestFPS");
		mGuiWorst = CEGUI::WindowManager::getSingleton().getWindow("OPWorstFPS");
		mGuiTris  = CEGUI::WindowManager::getSingleton().getWindow("OPTriCount");
		mGuiDbg   = CEGUI::WindowManager::getSingleton().getWindow("OPDebugMsg");
		mRoot	  = CEGUI::WindowManager::getSingleton().getWindow("root");
		mDebugOverlay->hide();
		timer = new Ogre::Timer();
		mLastTime = timer->getMicroseconds()/1000000.0f;

	}
	//-----------------------------------------------------------------------
	CrowdListener::~CrowdListener()
	{
		destroyCurrentGeomOpt();
		delete timer;
	}
	//-----------------------------------------------------------------------
	bool CrowdListener::frameRenderingQueued(const FrameEvent& evt)
	{

		burnCPU();
		vector <AnimationState*>::type::iterator it;
		for(it=animations.begin();it!=animations.end();it++)
		{
			(*it)->addTime(evt.timeSinceLastFrame);
		}
		
		updateStats();

		if(mRequestShutDown)
			return false;
		const bool returnValue = ExampleFrameListener::frameRenderingQueued(evt);
		// Call default
		return returnValue;
	}
	//-----------------------------------------------------------------------
	void CrowdListener::burnCPU(void)
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
		if(mTimeUntilNextToggle<=0)
		{
			//mDebugText="remaining for logic:"+ StringConverter::toString(time);
			mTimeUntilNextToggle=1;
		}
		
	}
	//-----------------------------------------------------------------------
	void CrowdListener::destroyCurrentGeomOpt()
	{
		switch(currentGeomOpt)
		{
			case INSTANCE_OPT:destroyInstanceGeom();break;
			case ENTITY_OPT: destroyEntityGeom ();break;
		}
	}
	//-----------------------------------------------------------------------
	void CrowdListener::createCurrentGeomOpt()
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
		MeshPtr m = MeshManager::getSingleton ().getByName ("robot.mesh");
		if (m.isNull ())
		{
			m = MeshManager::getSingleton ().load ("robot.mesh", 
				ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
		}
		const Real radius = m->getBoundingSphereRadius ();

		// could/should print on screen mesh name, 
		//optimisation type, 
		//mesh vertices num, 
		//32 bit or not, 
		//etC..


		switch(currentGeomOpt)
		{
			case INSTANCE_OPT:createInstanceGeom();break;
			case ENTITY_OPT: createEntityGeom ();break;
		}

	}
	//-----------------------------------------------------------------------
	void CrowdListener::createInstanceGeom()
	{
		if (Root::getSingleton ().getRenderSystem ()->getCapabilities ()->hasCapability (RSC_VERTEX_PROGRAM) == false)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Your video card doesn't support batching", "Demo_Instance::createScene");
		}

		Entity *ent = mCamera->getSceneManager()->createEntity("robot","robot.mesh");	
		setupInstancedMaterialToEntity(ent);

		renderInstance.reserve(numRender);
		renderInstance.resize(numRender);

		//Load a mesh to read data from.	
		InstancedGeometry* batch = new InstancedGeometry(mCamera->getSceneManager(), 
			"robots" );
		batch->setCastShadows(true);

        size_t i, k;

		batch->setBatchInstanceDimensions (Vector3(1000000, 1000000, 1000000));
		const size_t batchSize = (numMesh > maxObjectsPerBatch) ? maxObjectsPerBatch :numMesh;
		for(i = 0; i < batchSize ; i++)
		{
			batch->addEntity(ent, Vector3::ZERO);
		}
		batch->setOrigin(Vector3::ZERO);

		batch->build();


		for (k = 0; k < numRender-1; k++)
		{
			batch->addBatchInstance();
		}

		i = 0,k = 0;
		InstancedGeometry::BatchInstanceIterator regIt = batch->getBatchInstanceIterator();
		size_t baseIndexForBatch = 0;
		//create a RayQuery to get the terrain height
		RaySceneQuery* raySceneQuery=mCamera->getSceneManager()->createRayQuery(Ray(Vector3(0,0,0), Vector3::NEGATIVE_UNIT_Y));
		RaySceneQueryResult::iterator it;
		RaySceneQueryResult& qryResult=raySceneQuery->execute();
		while (regIt.hasMoreElements ())
		{

			InstancedGeometry::BatchInstance *r = regIt.getNext();

			InstancedGeometry::BatchInstance::InstancedObjectIterator bit = r->getObjectIterator();
			while(bit.hasMoreElements())
			{
				InstancedGeometry::InstancedObject* obj = bit.getNext();
				Vector3 position;
				
				position.x=10*k+500;
				position.z=10*i+500;
				position.y=0;
				raySceneQuery->setRay (Ray(position, Vector3::UNIT_Y));
				raySceneQuery->execute();
                it = qryResult.begin();

                if (it != qryResult.end() && it->worldFragment)
					position.y =  it->worldFragment->singleIntersection.y;
				
										
				obj->setPosition(position);
				obj->setScale(Vector3(0.1,0.1,0.1));
				AnimationState*anim=obj->getAnimationState("Walk");
				animations.push_back(anim);
				//offset the animation time, to show that all objects are independently animated.
				anim->setTimePosition(i+k);
				anim->setEnabled(true);
				k++;
			}

			if(k>14)
			{
				k=0;
			    i++;
			}
			
		}
		batch->setVisible(true);
		renderInstance[0] = batch;
		mCamera->getSceneManager()->destroyQuery(raySceneQuery);
		mCamera->getSceneManager()->destroyEntity (ent);
	}
	//-----------------------------------------------------------------------
	void CrowdListener::destroyInstanceGeom()
	{
		delete renderInstance[0];
		renderInstance.clear();
		animations.clear();
	}
	void CrowdListener::setupInstancedMaterialToEntity(Entity*ent)
	{
		for (Ogre::uint i = 0; i < ent->getNumSubEntities(); ++i)
		{
			SubEntity* se = ent->getSubEntity(i);
			String materialName= se->getMaterialName();
			se->setMaterialName(buildCrowdMaterial(materialName, ent->getSkeleton()->getNumBones()));
		}
	}
	String CrowdListener::buildCrowdMaterial(const String &originalMaterialName,int numBones)
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
				p->setVertexProgram("Crowd",true);
				p->setShadowCasterVertexProgram("CrowdShadowCaster");
		
			}
		}
		instancedMaterial->load();
		return instancedMaterialName;


	}
	//-----------------------------------------------------------------------
	void CrowdListener::createEntityGeom()
	{
		size_t k = 0;
		size_t y = 0;
		renderEntity.reserve (numMesh);
		renderEntity.resize (numMesh);
		nodes.reserve (numMesh);
		nodes.resize (numMesh);
		//create a RayQuery to get the terrain height
		RaySceneQuery* raySceneQuery=mCamera->getSceneManager()->createRayQuery(Ray(Vector3(0,0,0), Vector3::NEGATIVE_UNIT_Y));
		RaySceneQueryResult::iterator it;
		RaySceneQueryResult& qryResult=raySceneQuery->execute();
		for (size_t i = 0; i < numMesh; i++)
		{
		
			Vector3 position;
				
			position.x=10*y+500;
			position.y=0;
			position.z=10*k+500;	

			raySceneQuery->setRay (Ray(position, Vector3::UNIT_Y));
			raySceneQuery->execute();
            it = qryResult.begin();

            if (it != qryResult.end() && it->worldFragment)
				position.y =  it->worldFragment->singleIntersection.y;

			nodes[i]=mCamera->getSceneManager()->getRootSceneNode()->createChildSceneNode("node"+StringConverter::toString(i));
			LogManager::getSingleton().logMessage(":"+nodes[i]->getName());
			renderEntity[i]=mCamera->getSceneManager()->createEntity("robot"+StringConverter::toString(i), "robot.mesh");	
	    	nodes[i]->attachObject(renderEntity[i]);
			nodes[i]->setPosition(position);
			nodes[i]->setScale(Vector3(0.1,0.1,0.1));
			AnimationState*anim=renderEntity[i]->getAnimationState("Walk");
			animations.push_back(anim);
			anim->setTimePosition(y+k);
			anim->setEnabled(true);
			y++;
			if (y>14)
			{
				y=0;
				k++;
			}
		}
		mCamera->getSceneManager()->destroyQuery(raySceneQuery);


	}
	//-----------------------------------------------------------------------
	void CrowdListener::destroyEntityGeom()
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
		animations.clear();



	}
	//-----------------------------------------------------------------------
	bool CrowdListener::mouseMoved ( const OIS::MouseEvent &arg )
	{

		CEGUI::System::getSingleton().injectMouseMove( arg.state.X.rel, arg.state.Y.rel );
		return true;
	}	
	//-----------------------------------------------------------------------
   bool CrowdListener::mousePressed ( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
   {
       CEGUI::System::getSingleton().injectMouseButtonDown(convertOISMouseButtonToCegui(id));
		return true;
   }
	//-----------------------------------------------------------------------
   bool CrowdListener::mouseReleased (const OIS::MouseEvent &arg, OIS::MouseButtonID id)
   {
	   CEGUI::System::getSingleton().injectMouseButtonUp(convertOISMouseButtonToCegui(id));
		return true;
   }

   void CrowdListener::requestShutdown(void)
   {
	   mRequestShutDown=true;
   }
   void CrowdListener::setCurrentGeometryOpt(CurrentGeomOpt opt)
   {
	   currentGeomOpt=opt;
   }
   bool CrowdListener::handleMouseMove(const CEGUI::EventArgs& e)
	{
		using namespace CEGUI;

		if( mLMBDown)
		{
			int a =0;
			// rotate camera
			mRotX += Ogre::Degree(-((const MouseEventArgs&)e).moveDelta.d_x * mAvgFrameTime * 10.0);
			mRotY += Ogre::Degree(-((const MouseEventArgs&)e).moveDelta.d_y * mAvgFrameTime * 10.0);
			mCamera->yaw(mRotX);
			mCamera->pitch(mRotY);
			MouseCursor::getSingleton().setPosition( mLastMousePosition );
		}
			

	 return true;
	}
	//--------------------------------------------------------------------------
	bool CrowdListener::handleMouseButtonUp(const CEGUI::EventArgs& e)
	{
		using namespace CEGUI;

		
		if( ((const MouseEventArgs&)e).button == LeftButton )
		{
			mLMBDown = false;
			MouseCursor::getSingleton().setPosition( mLastMousePosition );
			CEGUI::MouseCursor::getSingleton().show();
		}


		return true;
	}

	//--------------------------------------------------------------------------
	bool CrowdListener::handleMouseButtonDown(const CEGUI::EventArgs& e)
	{
		using namespace CEGUI;

		
		if( ((const MouseEventArgs&)e).button == LeftButton )
		{
			mLMBDown = true;
			mLastMousePosition=CEGUI::MouseCursor::getSingleton().getPosition();
			CEGUI::MouseCursor::getSingleton().hide();
		}



		return true;
	}
	//--------------------------------------------------------------------------
	void CrowdListener::updateStats(void)
	{
		static CEGUI::String currFps = "Current FPS: ";
		static CEGUI::String avgFps = "Average FPS: ";
		static CEGUI::String bestFps = "Best FPS: ";
		static CEGUI::String worstFps = "Worst FPS: ";
		static CEGUI::String tris = "Triangle Count: ";


		const Ogre::RenderTarget::FrameStats& stats = mMain->getRenderWindow()->getStatistics();
        
		mGuiAvg->setText(avgFps + Ogre::StringConverter::toString(stats.avgFPS));
		mGuiCurr->setText(currFps + Ogre::StringConverter::toString(stats.lastFPS));
		mGuiBest->setText(bestFps + Ogre::StringConverter::toString(stats.bestFPS)
			+ " " + Ogre::StringConverter::toString(stats.bestFrameTime)+" ms");
		mGuiWorst->setText(worstFps + Ogre::StringConverter::toString(stats.worstFPS)
			+ " " + Ogre::StringConverter::toString(stats.worstFrameTime)+" ms");

		mGuiTris->setText(tris + Ogre::StringConverter::toString(stats.triangleCount));
		mAvgFrameTime = 1.0f/(stats.avgFPS + 1.0f);
		if (mAvgFrameTime > 0.1f) mAvgFrameTime = 0.1f;

	}
