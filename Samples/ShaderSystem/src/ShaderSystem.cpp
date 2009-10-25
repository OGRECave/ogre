#include "SamplePlugin.h"
#include "ShaderSystem.h"
#include "ShaderExReflectionMap.h"

using namespace Ogre;
using namespace OgreBites;

//-----------------------------------------------------------------------
const String DIRECTIONAL_LIGHT_NAME		= "DirectionalLight";
const String POINT_LIGHT_NAME			= "PointLight";
const String SPOT_LIGHT_NAME			= "SpotLight";
const String MAIN_ENTITY_MESH			= "ShaderSystem.mesh";
const String SPECULAR_BOX				= "SpecularBox";
const String REFLECTIONMAP_BOX			= "ReflectionMapBox";

SamplePlugin* sp;
Sample* s;

//-----------------------------------------------------------------------
extern "C" _OgreSampleExport void dllStartPlugin()
{
	s = new Sample_ShaderSystem;
	sp = OGRE_NEW SamplePlugin(s->getInfo()["Title"] + " Sample");
	sp->addSample(s);
	Root::getSingleton().installPlugin(sp);
}
//-----------------------------------------------------------------------
extern "C" _OgreSampleExport void dllStopPlugin()
{
	Root::getSingleton().uninstallPlugin(sp); 
	OGRE_DELETE sp;
	delete s;
}


//-----------------------------------------------------------------------
Sample_ShaderSystem::Sample_ShaderSystem()
{
	mInfo["Title"] = "Shader System";
	mInfo["Description"] = "Demonstrate the capabilities of the RT Shader System component."
		"1. Fixed Function Pipeline emulation."
		"2. On the fly shader generation based on existing material."
		"3. On the fly shader synchronization with scene state (Lights, Fog)."
		"4. Built in lighting models: Per vertex, Per pixel, Normal map tangent and object space."
		"5. Pluggable custom shaders extensions."
		"6. Built in material script parsing that includes extended attributes."
		"7. Built in material script serialization."
		;
	mInfo["Thumbnail"] = "thumb_shadersystem.png";
	mInfo["Category"] = "Unsorted";
	mInfo["Help"] = "F2 Toggle Shader System globally. F3 Toggles Global Lighting Model. Modify target model attributes and scene settings and observe the generated shaders count.";
	mPointLightNode = NULL;
	mReflectionMapFactory = NULL;
}


//-----------------------------------------------------------------------
void Sample_ShaderSystem::checkBoxToggled(CheckBox* box)
{
	const String& cbName = box->getName();

	if (cbName == SPECULAR_BOX)
	{
		setSpecularEnable(box->isChecked());
	}
	else if (cbName == REFLECTIONMAP_BOX)
	{
		setReflectionMapEnable(box->isChecked());
	}
	else if (cbName == DIRECTIONAL_LIGHT_NAME)
	{
		setLightVisible(cbName, box->isChecked());		
	}
	else if (cbName == POINT_LIGHT_NAME)
	{
		setLightVisible(cbName, box->isChecked());
	}
	else if (cbName == SPOT_LIGHT_NAME)
	{
		setLightVisible(cbName, box->isChecked());
	}
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::itemSelected(SelectMenu* menu)
{
	if (menu == mLightingModelMenu)
	{
		int curModelIndex = menu->getSelectionIndex();

		if (curModelIndex >= SSLM_PerVertexLighting && curModelIndex <= SSLM_NormalMapLightingObjectSpace)
		{
			setCurrentLightingModel((ShaderSystemLightingModel)curModelIndex);
		}
	}
}

//-----------------------------------------------------------------------
bool Sample_ShaderSystem::frameRenderingQueued( const FrameEvent& evt )
{	
	if (mSceneMgr->hasLight(SPOT_LIGHT_NAME))
	{
		Light* light = mSceneMgr->getLight(SPOT_LIGHT_NAME);

		light->setPosition(mCamera->getDerivedPosition() + mCamera->getDerivedUp() * 20.0);
		light->setDirection(mCamera->getDerivedDirection());
	}

	if (mPointLightNode != NULL)
	{
		static Real sToatalTime = 0.0;
		
		sToatalTime += evt.timeSinceLastFrame;
		mPointLightNode->yaw(Degree(evt.timeSinceLastFrame * 15));
		mPointLightNode->setPosition(0.0, Math::Sin(sToatalTime) * 30.0, 0.0);
	}
	
	return SdkSample::frameRenderingQueued(evt);
}


//-----------------------------------------------------------------------
void Sample_ShaderSystem::setupView()
{	
	// setup default viewport layout and camera
	mCamera = mSceneMgr->createCamera("MainCamera");
	mViewport = mWindow->addViewport(mCamera);
	mCamera->setAspectRatio((Ogre::Real)mViewport->getActualWidth() / (Ogre::Real)mViewport->getActualHeight());
	mCamera->setNearClipDistance(5);

	mCameraMan = new SdkCameraMan(mCamera);   // create a default camera controller
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::setupContent()
{
	// Setup defualt effects values.
	mCurLightingModel 		= SSLM_PerVertexLighting;
	mSpecularEnable   		= false;
	mReflectionMapEnable	= false;

	// Set ambient lighting.
	mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));

	// Setup the sky box,
	mSceneMgr->setSkyBox(true, "Examples/SceneCubeMap2");

	Plane plane;
	plane.normal = Vector3::UNIT_Y;
	plane.d = 0;
	MeshManager::getSingleton().createPlane("Myplane",
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
		1500,1500,20,20,true,1,60,60,Vector3::UNIT_Z);

	Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
	pPlaneEnt->setMaterialName("Examples/Rockwall");
	pPlaneEnt->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0,0,0))->attachObject(pPlaneEnt);


	// Load the main entity mesh.
	MeshPtr pMesh = MeshManager::getSingleton().load(MAIN_ENTITY_MESH,
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,    
		HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, 
		HardwareBuffer::HBU_STATIC_WRITE_ONLY, 
		true, true); //so we can still read it
	
	// Build tangent vectors, all our meshes use only 1 texture coordset 
	// Note we can build into VES_TANGENT now (SM2+)
	unsigned short src, dest;
	if (!pMesh->suggestTangentVectorBuildParams(VES_TANGENT, src, dest))
	{
		pMesh->buildTangentVectors(VES_TANGENT, src, dest);		
	}

	// Load main target object.
	Entity* mainEntity = mSceneMgr->createEntity("MainEntity", MAIN_ENTITY_MESH);

	mTargetEntities.push_back(mainEntity);

	// Attach the head to the scene
	SceneNode* childNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		
	childNode->attachObject(mainEntity);

	createDirectionalLight();
	createPointLight();
	createSpotLight();
	
	// create check boxes to toggle lights.	
	mTrayMgr->createCheckBox(TL_TOPLEFT, DIRECTIONAL_LIGHT_NAME, "Directional Light")->setChecked(true);
	mTrayMgr->createCheckBox(TL_TOPLEFT, POINT_LIGHT_NAME, "Point Light")->setChecked(true);
	mTrayMgr->createCheckBox(TL_TOPLEFT, SPOT_LIGHT_NAME, "Spot Light")->setChecked(false);
	
	// create target model widgets.
	mTrayMgr->createLabel(TL_BOTTOM, "TargetModel", "Target Model Attributes");

	mTrayMgr->createCheckBox(TL_BOTTOM, SPECULAR_BOX, "Specular")->setChecked(mSpecularEnable);
	mTrayMgr->createCheckBox(TL_BOTTOM, REFLECTIONMAP_BOX, "Reflection Map")->setChecked(mReflectionMapEnable);
	
	mLightingModelMenu = mTrayMgr->createLongSelectMenu(TL_BOTTOM, "TargetModelLighting", "Target Model", 470, 290, 10);	
	mLightingModelMenu ->addItem("Per Vertex");
	mLightingModelMenu ->addItem("Per Pixel");
	mLightingModelMenu ->addItem("Normal Map - Tangent Space");
	mLightingModelMenu ->addItem("Normal Map - Object Space");

	mCamera->setPosition(0.0, 300.0, 350.0);
	mCamera->lookAt(0.0, 80.0, 0.0);

	// Make this viewport work with shader generator scheme.
	mViewport->setMaterialScheme(RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);

	// Mark system as on.
	mRTShaderSystemPanel->setParamValue(0, "On");

	mTrayMgr->showCursor();

	// a friendly reminder
	StringVector names;
	names.push_back("Help");
	mTrayMgr->createParamsPanel(TL_TOPLEFT, "Help", 100, names)->setParamValue(0, "H/F1");

	updateSystemShaders();
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::cleanupContent()
{	
	if (mReflectionMapFactory != NULL)
	{
		if (mShaderGenerator != NULL)
		{			
			mShaderGenerator->removeSubRenderStateFactory(mReflectionMapFactory);
		}		
		delete mReflectionMapFactory;
		mReflectionMapFactory = NULL;
	}
	
	MeshManager::getSingleton().remove(MAIN_ENTITY_MESH);
	mTargetEntities.clear();
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::setCurrentLightingModel(ShaderSystemLightingModel lightingModel)
{
	if (mCurLightingModel != lightingModel)
	{
		mCurLightingModel  = lightingModel;

		EntityListIterator it = mTargetEntities.begin();
		EntityListIterator itEnd = mTargetEntities.end();

		for (; it != itEnd; ++it)
		{
			generateShaders(*it);
		}		
	}
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::setSpecularEnable(bool enable)
{
	if (mSpecularEnable != enable)
	{
		mSpecularEnable = enable;
		updateSystemShaders();
	}
}


//-----------------------------------------------------------------------
void Sample_ShaderSystem::setReflectionMapEnable(bool enable)
{
	if (mReflectionMapEnable != enable)
	{
		mReflectionMapEnable = enable;
		updateSystemShaders();
	}
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::updateSystemShaders() 
{
	EntityListIterator it = mTargetEntities.begin();
	EntityListIterator itEnd = mTargetEntities.end();

	for (; it != itEnd; ++it)
	{
		generateShaders(*it);
	}	
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::generateShaders(Entity* entity)
{
	for (unsigned int i=0; i < entity->getNumSubEntities(); ++i)
	{
		SubEntity* curSubEntity = entity->getSubEntity(i);
		const String& curMaterialName = curSubEntity->getMaterialName();
		bool success;

		// Create the shader based technique of this material.
		success = mShaderGenerator->createShaderBasedTechnique(curMaterialName, 
			 				MaterialManager::DEFAULT_SCHEME_NAME,
			 				RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);

		
		// Setup custom shader sub render states according to current setup.
		if (success)
		{					
			MaterialPtr curMaterial = MaterialManager::getSingleton().getByName(curMaterialName);
			Pass* curPass = curMaterial->getTechnique(0)->getPass(0);

			if (mSpecularEnable)
			{
				curPass->setSpecular(ColourValue::White);
				curPass->setShininess(32.0);
			}
			else
			{
				curPass->setShininess(0.0);
			}
			

			// Grab the first pass render state. 
			// NOTE: For more complicated samples iterate over the passes and build each one of them as desired.
			RTShader::RenderState* renderState = mShaderGenerator->getRenderState(RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME, curMaterialName, 0);

			// Remove all sub render states.
			renderState->reset();

			if (mCurLightingModel == SSLM_PerVertexLighting)
			{
				RTShader::SubRenderState* perPerVertexLightModel = mShaderGenerator->createSubRenderState(RTShader::FFPLighting::Type);

				renderState->addSubRenderState(perPerVertexLightModel);	
			}
			else if (mCurLightingModel == SSLM_PerPixelLighting)
			{
				RTShader::SubRenderState* perPixelLightModel = mShaderGenerator->createSubRenderState(RTShader::PerPixelLighting::Type);
				
				renderState->addSubRenderState(perPixelLightModel);				
			}
			else if (mCurLightingModel == SSLM_NormalMapLightingTangentSpace)
			{
				// Apply normal map only on main entity.
				if (entity->getName() == "MainEntity")
				{
					RTShader::SubRenderState* subRenderState = mShaderGenerator->createSubRenderState(RTShader::NormalMapLighting::Type);
					RTShader::NormalMapLighting* normalMapSubRS = static_cast<RTShader::NormalMapLighting*>(subRenderState);

					normalMapSubRS->setNormalMapSpace(RTShader::NormalMapLighting::NMS_TANGENT);
					curPass->getUserObjectBindings().setUserAny(RTShader::NormalMapLighting::NormalMapTextureNameKey, Any(String("Panels_Normal_Tangent.png")));	

					renderState->addSubRenderState(normalMapSubRS);
				}

				// It is secondary entity -> use simple per pixel lighting.
				else
				{
					RTShader::SubRenderState* perPixelLightModel = mShaderGenerator->createSubRenderState(RTShader::PerPixelLighting::Type);
					renderState->addSubRenderState(perPixelLightModel);
				}				
			}
			else if (mCurLightingModel == SSLM_NormalMapLightingObjectSpace)
			{
				// Apply normal map only on main entity.
				if (entity->getName() == "MainEntity")
				{
					RTShader::SubRenderState* subRenderState = mShaderGenerator->createSubRenderState(RTShader::NormalMapLighting::Type);
					RTShader::NormalMapLighting* normalMapSubRS = static_cast<RTShader::NormalMapLighting*>(subRenderState);

					normalMapSubRS->setNormalMapSpace(RTShader::NormalMapLighting::NMS_OBJECT);
					curPass->getUserObjectBindings().setUserAny(RTShader::NormalMapLighting::NormalMapTextureNameKey, Any(String("Panels_Normal_Obj.png")));	

					renderState->addSubRenderState(normalMapSubRS);
				}
				
				// It is secondary entity -> use simple per pixel lighting.
				else
				{
					RTShader::SubRenderState* perPixelLightModel = mShaderGenerator->createSubRenderState(RTShader::PerPixelLighting::Type);
					renderState->addSubRenderState(perPixelLightModel);
				}				
			}

			if (mReflectionMapEnable)
			{
				// Create and add the custom reflection map shader extension factory to the shader generator.
				if (mReflectionMapFactory == NULL)
				{
					mReflectionMapFactory = new ShaderExReflectionMapFactory;
					mShaderGenerator->addSubRenderStateFactory(mReflectionMapFactory);
				}

				RTShader::SubRenderState* subRenderState = mShaderGenerator->createSubRenderState(ShaderExReflectionMap::Type);
				ShaderExReflectionMap* reflectionMapSubRS = static_cast<ShaderExReflectionMap*>(subRenderState);

				reflectionMapSubRS->setReflectionMapType(TEX_TYPE_CUBE_MAP);

				// Setup the textures needed by the reflection effect.
				curPass->getUserObjectBindings().setUserAny(ShaderExReflectionMap::MaskMapTextureNameKey, Any(String("Panels_refmask.png")));	
				curPass->getUserObjectBindings().setUserAny(ShaderExReflectionMap::ReflectionMapTextureNameKey, Any(String("cubescene.jpg")));
											
				renderState->addSubRenderState(subRenderState);
			}
								
			// Invalidate this material in order to re-generate its shaders.
			mShaderGenerator->invalidateMaterial(RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME, curMaterialName);
		}		
	}
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::createDirectionalLight()
{
	Light*  light;
	Vector3 dir;

	light = mSceneMgr->createLight(DIRECTIONAL_LIGHT_NAME);
    light->setType(Light::LT_DIRECTIONAL);
    dir.x = 0.5;
	dir.y = -1.0;
	dir.z = 0.3;
    dir.normalise();
    light->setDirection(dir);
    light->setDiffuseColour(0.65, 0.15, 0.15);
    light->setSpecularColour(0.5, 0.5, 0.5);

	// create pivot node
	mDirectionalLightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	BillboardSet* bbs;

	// Create billboard set.
	bbs = mSceneMgr->createBillboardSet();
	bbs->setMaterialName("Examples/Flare");
	bbs->createBillboard(-dir * 500.0)->setColour(light->getDiffuseColour());
	
	mDirectionalLightNode->attachObject(bbs);
	mDirectionalLightNode->attachObject(light);
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::createPointLight()
{
	Light*  light;
	Vector3 dir;

	light = mSceneMgr->createLight(POINT_LIGHT_NAME);
	light->setType(Light::LT_POINT);
	dir.x = 0.5;
	dir.y = 0.0;
	dir.z = 0.0f;
	dir.normalise();
	light->setDirection(dir);
	light->setDiffuseColour(0.15, 0.65, 0.15);
	light->setSpecularColour(0.5, 0.5, 0.5);	
	light->setAttenuation(200.0, 1.0, 0.0005, 0.0);

	// create pivot node
	mPointLightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	
	BillboardSet* bbs;
	
	// Create billboard set.
	bbs = mSceneMgr->createBillboardSet();
	bbs->setMaterialName("Examples/Flare");
	bbs->createBillboard(200, 100, 0)->setColour(light->getDiffuseColour());

	mPointLightNode->attachObject(bbs);
	mPointLightNode->createChildSceneNode(Vector3(200, 100, 0))->attachObject(light);
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::createSpotLight()
{
	Light*  light;
	Vector3 dir;

	light = mSceneMgr->createLight(SPOT_LIGHT_NAME);
	light->setType(Light::LT_SPOTLIGHT);
	dir.x = 0.0;
	dir.y = 0.0;
	dir.z = -1.0f;
	dir.normalise();	
	light->setSpotlightRange(Degree(20.0), Degree(25.0), 0.95);
	light->setDirection(dir);
	light->setDiffuseColour(0.15, 0.15, 0.65);
	light->setSpecularColour(0.5, 0.5, 0.5);	
	light->setAttenuation(1000.0, 1.0, 0.0005, 0.0);
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::setLightVisible(const String& lightName, bool visible)
{
	if (mSceneMgr->hasLight(lightName))
	{		
		// Case it is the point light,
		// toggle its visibility and billboard set visibility.
		if (lightName == POINT_LIGHT_NAME)
		{
			if (visible)
			{
				if (mPointLightNode->isInSceneGraph() == false)
				{
					mSceneMgr->getRootSceneNode()->addChild(mPointLightNode);
				}
			}
			else
			{
				if (mPointLightNode->isInSceneGraph() == true)
				{
					mSceneMgr->getRootSceneNode()->removeChild(mPointLightNode);
				}
			}	
			mSceneMgr->getLight(lightName)->setVisible(visible);
		}
		
		// Case it is the directional light,
		// toggle its visibility and billboard set visibility.
		else if (lightName == DIRECTIONAL_LIGHT_NAME)
		{
			SceneNode::ObjectIterator it = mDirectionalLightNode->getAttachedObjectIterator();

			while (it.hasMoreElements())  
			{
				MovableObject* o = it.getNext();
				o->setVisible(visible);
			}
		}

		// Spot light has no scene node representation.
		else
		{
			mSceneMgr->getLight(lightName)->setVisible(visible);
		}		
	}
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::exportShaderMaterial(const String& fileName, const String& materialName)
{
	// Grab material pointer.
	MaterialPtr matSerTest = MaterialManager::getSingleton().getByName(materialName);

	// Create shader based technique.
	bool success = mShaderGenerator->createShaderBasedTechnique(materialName,
		MaterialManager::DEFAULT_SCHEME_NAME,
		RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);

	// Case creation of shader based technique succeeded.
	if (success)
	{
		// Force shader generation of the given material.
		RTShader::ShaderGenerator::getSingleton().validateMaterial(RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME, materialName);

		// Grab the shader generator material serializer listener.
		MaterialSerializer::Listener* matSGListener = RTShader::ShaderGenerator::getSingleton().getMaterialSerializerListener();
		MaterialSerializer matSer;

		// Add the listener and export.
		matSer.addListener(matSGListener);
		matSer.exportMaterial(matSerTest, fileName);
	}
}

//-----------------------------------------------------------------------
Ogre::StringVector Sample_ShaderSystem::getRequiredPlugins()
{
	StringVector names;
	names.push_back("Cg Program Manager");
	return names;
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::testCapabilities( const RenderSystemCapabilities* caps )
{
	if (!caps->hasCapability(RSC_VERTEX_PROGRAM) || !(caps->hasCapability(RSC_FRAGMENT_PROGRAM)))
	{
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your graphics card does not support vertex and fragment programs, "
			"so you cannot run this sample. Sorry!", "Dot3BumpSample::testCapabilities");
	}

	if (!GpuProgramManager::getSingleton().isSyntaxSupported("arbfp1") &&
		!GpuProgramManager::getSingleton().isSyntaxSupported("ps_2_0"))
	{
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your card does not support shader model 2, "
			"so you cannot run this sample. Sorry!", "Dot3BumpSample::testCapabilities");
	}
}



#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE

bool Sample_ShaderSystem::touchPressed( const OIS::MultiTouchEvent& evt )
{
	if (mTrayMgr->injectMouseDown(evt)) 
		return true;
	if (evt.state.touchIsType(OIS::MT_Pressed)) 
		mTrayMgr->hideCursor();  // hide the cursor if user left-clicks in the scene
	return true;
}

bool Sample_ShaderSystem::touchReleased( const OIS::MultiTouchEvent& evt )
{
	if (mTrayMgr->injectMouseUp(evt)) 
		return true;
	if (evt.state.touchIsType(OIS::MT_Pressed)) 
		mTrayMgr->showCursor();  // unhide the cursor if user lets go of LMB
	return true;
}

bool Sample_ShaderSystem::touchMoved( const OIS::MultiTouchEvent& evt )
{
	// only rotate the camera if cursor is hidden
	if (mTrayMgr->isCursorVisible()) 
		mTrayMgr->injectMouseMove(evt);
	else 
		mCameraMan->injectMouseMove(evt);
	return true;
}

#else

bool Sample_ShaderSystem::mousePressed( const OIS::MouseEvent& evt, OIS::MouseButtonID id )
{
	if (mTrayMgr->injectMouseDown(evt, id)) 
		return true;
	if (id == OIS::MB_Left) 
		mTrayMgr->hideCursor();  // hide the cursor if user left-clicks in the scene

	return true;
}

bool Sample_ShaderSystem::mouseReleased( const OIS::MouseEvent& evt, OIS::MouseButtonID id )
{
	if (mTrayMgr->injectMouseUp(evt, id)) 
		return true;
	if (id == OIS::MB_Left) 
		mTrayMgr->showCursor();  // unhide the cursor if user lets go of LMB

	return true;
}

bool Sample_ShaderSystem::mouseMoved( const OIS::MouseEvent& evt )
{
	// only rotate the camera if cursor is hidden
	if (mTrayMgr->isCursorVisible()) 
		mTrayMgr->injectMouseMove(evt);
	else 
		mCameraMan->injectMouseMove(evt);

	return true;
}
#endif
