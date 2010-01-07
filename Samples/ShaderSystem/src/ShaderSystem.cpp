#include "SamplePlugin.h"
#include "ShaderSystem.h"
#include "ShaderExReflectionMap.h"

using namespace Ogre;
using namespace OgreBites;

//-----------------------------------------------------------------------
const String DIRECTIONAL_LIGHT_NAME		= "DirectionalLight";
const String POINT_LIGHT_NAME			= "PointLight";
const String SPOT_LIGHT_NAME			= "SpotLight";
const String PER_PIXEL_FOG_BOX			= "PerPixelFog";
const String MAIN_ENTITY_MESH			= "ShaderSystem.mesh";
const String SPECULAR_BOX				= "SpecularBox";
const String REFLECTIONMAP_BOX			= "ReflectionMapBox";
const String REFLECTIONMAP_POWER_SLIDER	= "ReflectionPowerSlider";
const String MAIN_ENTITY_NAME			= "MainEntity";
const String EXPORT_BUTTON_NAME			= "ExportMaterial";
const String FLUSH_BUTTON_NAME			= "FlushShaderCache";
const String SAMPLE_MATERIAL_GROUP		= "RTShaderSystemMaterialsGroup";
const int MESH_ARRAY_SIZE = 2;
const String MESH_ARRAY[MESH_ARRAY_SIZE] =
{
	MAIN_ENTITY_MESH,
	"knot.mesh"
};

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
	mInfo["Category"] = "Lighting";
	mInfo["Help"] = "F2 Toggle Shader System globally. "
				    "F3 Toggles Global Lighting Model. "
					"Modify target model attributes and scene settings and observe the generated shaders count. "
					"Press the export button in order to export current target model material. "
					"The model above the target will import this material next time the sample reloads. "
					"Right click on object to see the shaders it currently uses. "
					;
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
	else if (cbName == PER_PIXEL_FOG_BOX)
	{
		setPerPixelFogEnable(box->isChecked());
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
	else if (menu == mFogModeMenu)
	{
		int curModeIndex = menu->getSelectionIndex();

		if (curModeIndex >= FOG_NONE && curModeIndex <= FOG_LINEAR)
		{
			mSceneMgr->setFog((FogMode)curModeIndex, ColourValue(1.0, 1.0, 1.0, 0.0), 0.0015, 350.0, 1500.0);
		}		
	}
	else if (menu == mShadowMenu)
	{
		int curShadowTypeIndex = menu->getSelectionIndex();

		applyShadowType(curShadowTypeIndex);		
	}
	else if(menu == mLanguageMenu)
	{
		ShaderGenerator::getSingletonPtr()->setTargetLanguage(menu->getSelectedItem());		
	}
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::buttonHit( OgreBites::Button* b )
{
	// Case the current material of the main entity should be exported.
	if (b->getName() == EXPORT_BUTTON_NAME)
	{		
		const String& materialName = mSceneMgr->getEntity(MAIN_ENTITY_NAME)->getSubEntity(0)->getMaterialName();
		
		exportRTShaderSystemMaterial(mRTShaderLibsPath + "materials/ShaderSystemExport.material", materialName);						
	}
	// Case the shader cache should be flushed.
	else if (b->getName() == FLUSH_BUTTON_NAME)
	{				
		mShaderGenerator->flushShaderCache();
	}
}

//--------------------------------------------------------------------------
void Sample_ShaderSystem::sliderMoved(Slider* slider)
{
	if (slider->getName() == REFLECTIONMAP_POWER_SLIDER)
	{
		Real reflectionPower = slider->getValue();

		if (mReflectionMapSubRS != NULL)
		{
			ShaderExReflectionMap* reflectionMapSubRS = static_cast<ShaderExReflectionMap*>(mReflectionMapSubRS);
			
			// Since RTSS export caps based on the template sub render states we have to update the template reflection sub render state. 
			reflectionMapSubRS->setReflectionPower(reflectionPower);

			// Grab the instances set and update them with the new reflection power value.
			// The instances are the actual sub render states that have been assembled to create the final shaders.
			// Every time that the shaders have to be re-generated (light changes, fog changes etc..) a new set of sub render states 
			// based on the template sub render states assembled for each pass.
			// From that set of instances a CPU program is generated and afterward a GPU program finally generated.
			RTShader::SubRenderStateSet instanceSet = mReflectionMapSubRS->getAccessor()->getSubRenderStateInstanceSet();
			RTShader::SubRenderStateSetIterator it = instanceSet.begin();
			RTShader::SubRenderStateSetIterator itEnd = instanceSet.end();

			for (; it != itEnd; ++it)
			{
				ShaderExReflectionMap* reflectionMapSubRS = static_cast<ShaderExReflectionMap*>(*it);
				
				reflectionMapSubRS->setReflectionPower(reflectionPower);
			}
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

	updateTargetObjInfo();
	
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
	// Setup default effects values.
	mCurLightingModel 		= SSLM_PerVertexLighting;
	mPerPixelFogEnable		= false;
	mSpecularEnable   		= false;
	mReflectionMapEnable	= false;
	mReflectionMapSubRS  = NULL;

	mRayQuery = mSceneMgr->createRayQuery(Ray());
	mTargetObj = NULL;


	// Set ambient lighting.
	mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));

	// Setup the sky box,
	mSceneMgr->setSkyBox(true, "Examples/SceneCubeMap2");

	Plane plane;
	plane.normal = Vector3::UNIT_Y;
	plane.d = 0;
	MeshManager::getSingleton().createPlane("Myplane",
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
		1500,1500,25,25,true,1,60,60,Vector3::UNIT_Z);

	Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
	pPlaneEnt->setMaterialName("Examples/Rockwall");
	pPlaneEnt->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0,0,0))->attachObject(pPlaneEnt);


	// Load sample meshes and generate tangent vectors.
	for (int i=0; i < MESH_ARRAY_SIZE; ++i)
	{
		const String& curMeshName = MESH_ARRAY[i];

		MeshPtr pMesh = MeshManager::getSingleton().load(curMeshName,
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
	}
	


	Entity* entity;
	SceneNode* childNode;

	// Create the main entity and mark it as the current target object.
	entity = mSceneMgr->createEntity(MAIN_ENTITY_NAME, MAIN_ENTITY_MESH);
	mTargetEntities.push_back(entity);
	childNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();		
	childNode->attachObject(entity);
	mTargetObj = entity;
	childNode->showBoundingBox(true);

	// Create reflection entity that will show the exported material.
	const String& mainExportedMaterial = mSceneMgr->getEntity(MAIN_ENTITY_NAME)->getSubEntity(0)->getMaterialName() + "_RTSS";
	MaterialPtr matMainEnt        = MaterialManager::getSingleton().getByName(mainExportedMaterial, SAMPLE_MATERIAL_GROUP);

	entity = mSceneMgr->createEntity("ExportedMaterialEntity", MAIN_ENTITY_MESH);
	entity->setMaterial(matMainEnt);
	childNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	childNode->setPosition(0.0, 200.0, -200.0);
	childNode->attachObject(entity);

	// Create secondary entities that will be using custom RT Shader materials.
	entity = mSceneMgr->createEntity("PerPixelEntity", "knot.mesh");
	entity->setMaterialName("RTSS/PerPixel_SinglePass");
	childNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	childNode->setPosition(300.0, 100.0, -100.0);
	childNode->attachObject(entity);

	// Create secondary entities that will be using custom RT Shader materials.
	entity = mSceneMgr->createEntity("NormalMapEntity", "knot.mesh");
	entity->setMaterialName("RTSS/NormalMapping_SinglePass");
	childNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	childNode->setPosition(-300.0, 100.0, -100.0);
	childNode->attachObject(entity);
	
	createDirectionalLight();
	createPointLight();
	createSpotLight();
	
	setupUI();


	mCamera->setPosition(0.0, 300.0, 450.0);
	mCamera->lookAt(0.0, 150.0, 0.0);

	// Make this viewport work with shader generator scheme.
	mViewport->setMaterialScheme(RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);

	// Mark system as on.
	mDetailsPanel->setParamValue(11, "On");

	
	// a friendly reminder
	StringVector names;
	names.push_back("Help");
	mTrayMgr->createParamsPanel(TL_TOPLEFT, "Help", 100, names)->setParamValue(0, "H/F1");

	updateSystemShaders();
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::setupUI()
{
	// Create language selection 
	mLanguageMenu = mTrayMgr->createLongSelectMenu(TL_TOPLEFT, "LangMode", "Language", 220, 120, 10);	

	// Use GLSL in case of OpenGL render system.
	if (Ogre::Root::getSingletonPtr()->getRenderSystem()->getName().find("OpenGL") != String::npos)
	{
		mLanguageMenu->addItem("glsl");
		mShaderGenerator->setTargetLanguage("glsl");		
	}

	// Use HLSL in case of D3D9 render system.
	else if (Ogre::Root::getSingletonPtr()->getRenderSystem()->getName().find("Direct3D9") != String::npos)
	{
		mLanguageMenu->addItem("hlsl");
		mShaderGenerator->setTargetLanguage("hlsl");				
	}
	mLanguageMenu->addItem("cg");

	// create check boxes to toggle lights.	
	mDirLightCheckBox = mTrayMgr->createCheckBox(TL_TOPLEFT, DIRECTIONAL_LIGHT_NAME, "Directional Light", 220);
	mPointLightCheckBox = mTrayMgr->createCheckBox(TL_TOPLEFT, POINT_LIGHT_NAME, "Point Light", 220);
	mSpotLightCheckBox = mTrayMgr->createCheckBox(TL_TOPLEFT, SPOT_LIGHT_NAME, "Spot Light", 220);

	mDirLightCheckBox->setChecked(true);
	mPointLightCheckBox->setChecked(true);
	mSpotLightCheckBox->setChecked(false);


#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
	mTrayMgr->createCheckBox(TL_TOPLEFT, PER_PIXEL_FOG_BOX, "Per Pixel Fog", 220)->setChecked(mPerPixelFogEnable);
#endif

	// Create fog widgets.
	mFogModeMenu = mTrayMgr->createLongSelectMenu(TL_TOPLEFT, "FogMode", "Fog Mode", 220, 120, 10);	
	mFogModeMenu->addItem("None");
	mFogModeMenu->addItem("Exp");
	mFogModeMenu->addItem("Exp2");
	mFogModeMenu->addItem("Linear");

	// Create shadow menu.
	mShadowMenu = mTrayMgr->createLongSelectMenu(TL_TOPLEFT, "ShadowType", "Shadow", 220, 120, 10);	
	mShadowMenu->addItem("None");

#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
	mShadowMenu->addItem("PSSM 3");
#endif


	// Flush shader cache button.
	mTrayMgr->createButton(TL_TOPLEFT, FLUSH_BUTTON_NAME, "Flush Shader Cache", 220);

	// create target model widgets.
	mTargetObjMatName = mTrayMgr->createLabel(TL_TOPLEFT, "TargetObjMatName", "", 220);
	mTargetObjVS = mTrayMgr->createLabel(TL_TOPLEFT, "TargetObjVS", "", 220);
	mTargetObjFS = mTrayMgr->createLabel(TL_TOPLEFT, "TargetObjFS", "", 220);

	
	// Create main entity widgets.
	mTrayMgr->createLabel(TL_BOTTOM, "MainEntityLabel", "Main Entity Settings", 240);
	mTrayMgr->createCheckBox(TL_BOTTOM, SPECULAR_BOX, "Specular", 240)->setChecked(mSpecularEnable);

	// Allow reflection map only on PS3 and above since with all lights on + specular + bump we 
	// exceed the instruction count limits of PS2.
	if (GpuProgramManager::getSingleton().isSyntaxSupported("ps_3_0") ||
		GpuProgramManager::getSingleton().isSyntaxSupported("fp30"))		
	{
		mTrayMgr->createCheckBox(TL_BOTTOM, REFLECTIONMAP_BOX, "Reflection Map", 240)->setChecked(mReflectionMapEnable);
		mReflectionPowerSlider = mTrayMgr->createThickSlider(TL_BOTTOM, REFLECTIONMAP_POWER_SLIDER, "Reflection Power", 240, 80, 0, 1, 100);
		mReflectionPowerSlider->setValue(0.5, false);
	}

	mLightingModelMenu = mTrayMgr->createLongSelectMenu(TL_BOTTOM, "TargetModelLighting", "", 240, 230, 10);	
	mLightingModelMenu ->addItem("Per Vertex");

#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
	mLightingModelMenu ->addItem("Per Pixel");
	mLightingModelMenu ->addItem("Normal Map - Tangent Space");
	mLightingModelMenu ->addItem("Normal Map - Object Space");
#endif

	mTrayMgr->createButton(TL_BOTTOM, EXPORT_BUTTON_NAME, "Export Material", 240);
	
	mTrayMgr->showCursor();
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::cleanupContent()
{	
	// UnLoad sample meshes and generate tangent vectors.
	for (int i=0; i < MESH_ARRAY_SIZE; ++i)
	{
		const String& curMeshName = MESH_ARRAY[i];
		MeshManager::getSingleton().unload(curMeshName); 
	}
	
	MeshManager::getSingleton().remove(MAIN_ENTITY_MESH);
	mTargetEntities.clear();

	mSceneMgr->destroyQuery(mRayQuery);
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
void Sample_ShaderSystem::setPerPixelFogEnable( bool enable )
{
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
	if (mPerPixelFogEnable != enable)
	{
		mPerPixelFogEnable = enable;

		// Grab the scheme render state.
		RenderState* schemRenderState = mShaderGenerator->getRenderState(RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
		const SubRenderStateList& subRenderStateList = schemRenderState->getTemplateSubRenderStateList();
		SubRenderStateListConstIterator it = subRenderStateList.begin();
		SubRenderStateListConstIterator itEnd = subRenderStateList.end();
		FFPFog* fogSubRenderState = NULL;
		
		// Search for the fog sub state.
		for (; it != itEnd; ++it)
		{
			SubRenderState* curSubRenderState = *it;

			if (curSubRenderState->getType() == FFPFog::Type)
			{
				fogSubRenderState = static_cast<FFPFog*>(curSubRenderState);
				break;
			}
		}

		// Create the fog sub render state if need to.
		if (fogSubRenderState == NULL)
		{			
			SubRenderState* subRenderState = mShaderGenerator->createSubRenderState(FFPFog::Type);
			
			fogSubRenderState = static_cast<FFPFog*>(subRenderState);
			schemRenderState->addTemplateSubRenderState(fogSubRenderState);
		}
			
		
		// Select the desired fog calculation mode.
		if (mPerPixelFogEnable)
		{
			fogSubRenderState->setCalcMode(FFPFog::CM_PER_PIXEL);
		}
		else
		{
			fogSubRenderState->setCalcMode(FFPFog::CM_PER_VERTEX);
		}

		// Invalidate the scheme in order to re-generate all shaders based technique related to this scheme.
		mShaderGenerator->invalidateScheme(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
	}
#endif

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
				curPass->setSpecular(ColourValue::Black);
				curPass->setShininess(0.0);
			}
			

			// Grab the first pass render state. 
			// NOTE: For more complicated samples iterate over the passes and build each one of them as desired.
			RTShader::RenderState* renderState = mShaderGenerator->getRenderState(RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME, curMaterialName, 0);

			// Remove all sub render states.
			renderState->reset();


#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
			if (mCurLightingModel == SSLM_PerVertexLighting)
			{
				RTShader::SubRenderState* perPerVertexLightModel = mShaderGenerator->createSubRenderState(RTShader::FFPLighting::Type);

				renderState->addTemplateSubRenderState(perPerVertexLightModel);	
			}
#endif

#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
			else if (mCurLightingModel == SSLM_PerPixelLighting)
			{
				RTShader::SubRenderState* perPixelLightModel = mShaderGenerator->createSubRenderState(RTShader::PerPixelLighting::Type);
				
				renderState->addTemplateSubRenderState(perPixelLightModel);				
			}
			else if (mCurLightingModel == SSLM_NormalMapLightingTangentSpace)
			{
				// Apply normal map only on main entity.
				if (entity->getName() == MAIN_ENTITY_NAME)
				{
					RTShader::SubRenderState* subRenderState = mShaderGenerator->createSubRenderState(RTShader::NormalMapLighting::Type);
					RTShader::NormalMapLighting* normalMapSubRS = static_cast<RTShader::NormalMapLighting*>(subRenderState);
					
					normalMapSubRS->setNormalMapSpace(RTShader::NormalMapLighting::NMS_TANGENT);
					normalMapSubRS->setNormalMapTextureName("Panels_Normal_Tangent.png");	

					renderState->addTemplateSubRenderState(normalMapSubRS);
				}

				// It is secondary entity -> use simple per pixel lighting.
				else
				{
					RTShader::SubRenderState* perPixelLightModel = mShaderGenerator->createSubRenderState(RTShader::PerPixelLighting::Type);
					renderState->addTemplateSubRenderState(perPixelLightModel);
				}				
			}
			else if (mCurLightingModel == SSLM_NormalMapLightingObjectSpace)
			{
				// Apply normal map only on main entity.
				if (entity->getName() == MAIN_ENTITY_NAME)
				{
					RTShader::SubRenderState* subRenderState = mShaderGenerator->createSubRenderState(RTShader::NormalMapLighting::Type);
					RTShader::NormalMapLighting* normalMapSubRS = static_cast<RTShader::NormalMapLighting*>(subRenderState);
				
					normalMapSubRS->setNormalMapSpace(RTShader::NormalMapLighting::NMS_OBJECT);
					normalMapSubRS->setNormalMapTextureName("Panels_Normal_Obj.png");	

					renderState->addTemplateSubRenderState(normalMapSubRS);
				}
				
				// It is secondary entity -> use simple per pixel lighting.
				else
				{
					RTShader::SubRenderState* perPixelLightModel = mShaderGenerator->createSubRenderState(RTShader::PerPixelLighting::Type);
					renderState->addTemplateSubRenderState(perPixelLightModel);
				}				
			}

#endif

			if (mReflectionMapEnable)
			{				
				RTShader::SubRenderState* subRenderState = mShaderGenerator->createSubRenderState(ShaderExReflectionMap::Type);
				ShaderExReflectionMap* reflectionMapSubRS = static_cast<ShaderExReflectionMap*>(subRenderState);
			
				reflectionMapSubRS->setReflectionMapType(TEX_TYPE_CUBE_MAP);
				reflectionMapSubRS->setReflectionPower(mReflectionPowerSlider->getValue());

				// Setup the textures needed by the reflection effect.
				reflectionMapSubRS->setMaskMapTextureName("Panels_refmask.png");	
				reflectionMapSubRS->setReflectionMapTextureName("cubescene.jpg");
											
				renderState->addTemplateSubRenderState(subRenderState);
				mReflectionMapSubRS = subRenderState;				
			}
			else
			{
				mReflectionMapSubRS = NULL;
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
	light->setCastShadows(true);
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
	bbs->setMaterialName("Examples/Flare3");
	bbs->createBillboard(-dir * 500.0)->setColour(light->getDiffuseColour());
	bbs->setCastShadows(false);
	
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
	light->setCastShadows(false);
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
	bbs->setMaterialName("Examples/Flare3");
	bbs->createBillboard(200, 100, 0)->setColour(light->getDiffuseColour());
	bbs->setCastShadows(false);

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
	light->setCastShadows(false);
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
void Sample_ShaderSystem::applyShadowType(int menuIndex)
{
	// Grab the scheme render state.												
	Ogre::RTShader::RenderState* schemRenderState = mShaderGenerator->getRenderState(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);


	// No shadow
	if (menuIndex == 0)
	{
		mSceneMgr->setShadowTechnique(SHADOWTYPE_NONE);

#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
		const Ogre::RTShader::SubRenderStateList& subRenderStateList = schemRenderState->getTemplateSubRenderStateList();
		Ogre::RTShader::SubRenderStateListConstIterator it = subRenderStateList.begin();
		Ogre::RTShader::SubRenderStateListConstIterator itEnd = subRenderStateList.end();

		for (; it != itEnd; ++it)
		{
			Ogre::RTShader::SubRenderState* curSubRenderState = *it;

			// This is the pssm3 sub render state -> remove it.
			if (curSubRenderState->getType() == Ogre::RTShader::IntegratedPSSM3::Type)
			{
				schemRenderState->removeTemplateSubRenderState(*it);
				break;
			}
		}
#endif

		mTrayMgr->moveWidgetToTray(mDirLightCheckBox, TL_TOPLEFT, 1);
		mTrayMgr->moveWidgetToTray(mPointLightCheckBox, TL_TOPLEFT, 2);
		mTrayMgr->moveWidgetToTray(mSpotLightCheckBox, TL_TOPLEFT, 3);
		mDirLightCheckBox->show();
		mPointLightCheckBox->show();
		mSpotLightCheckBox->show();

	}

#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
	// Integrated shadow PSSM with 3 splits.
	else if (menuIndex == 1)
	{
		mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE_INTEGRATED);

		// 3 textures per directional light
		mSceneMgr->setShadowTextureCountPerLightType(Ogre::Light::LT_DIRECTIONAL, 3);
		mSceneMgr->setShadowTextureSettings(512, 3, PF_FLOAT32_R);
		mSceneMgr->setShadowTextureSelfShadow(true);

		// Leave only directional light.
		mDirLightCheckBox->setChecked(true);
		mPointLightCheckBox->setChecked(false);
		mSpotLightCheckBox->setChecked(false);

		mTrayMgr->removeWidgetFromTray(mDirLightCheckBox);
		mTrayMgr->removeWidgetFromTray(mPointLightCheckBox);
		mTrayMgr->removeWidgetFromTray(mSpotLightCheckBox);
		mDirLightCheckBox->hide();
		mPointLightCheckBox->hide();
		mSpotLightCheckBox->hide();
		
		
		// Set up caster material - this is just a standard depth/shadow map caster
		mSceneMgr->setShadowTextureCasterMaterial("PSSM/shadow_caster");
		
		
		// Disable fog on the caster pass.
		MaterialPtr passCaterMaterial = MaterialManager::getSingleton().getByName("PSSM/shadow_caster");
		Pass* pssmCasterPass = passCaterMaterial->getTechnique(0)->getPass(0);
		pssmCasterPass->setFog(true);

		// shadow camera setup
		PSSMShadowCameraSetup* pssmSetup = new PSSMShadowCameraSetup();
		pssmSetup->calculateSplitPoints(3, 5, 3000);
		pssmSetup->setSplitPadding(10);
		pssmSetup->setOptimalAdjustFactor(0, 2);
		pssmSetup->setOptimalAdjustFactor(1, 1);
		pssmSetup->setOptimalAdjustFactor(2, 0.5);

		mSceneMgr->setShadowCameraSetup(ShadowCameraSetupPtr(pssmSetup));

	
		Ogre::RTShader::SubRenderState* subRenderState = mShaderGenerator->createSubRenderState(Ogre::RTShader::IntegratedPSSM3::Type);	
		Ogre::RTShader::IntegratedPSSM3* pssm3SubRenderState = static_cast<Ogre::RTShader::IntegratedPSSM3*>(subRenderState);
		const PSSMShadowCameraSetup::SplitPointList& srcSplitPoints = pssmSetup->getSplitPoints();
		Ogre::RTShader::IntegratedPSSM3::SplitPointList dstSplitPoints;

		for (unsigned int i=0; i < srcSplitPoints.size(); ++i)
		{
			dstSplitPoints.push_back(srcSplitPoints[i]);
		}

		pssm3SubRenderState->setSplitPoints(dstSplitPoints);
		schemRenderState->addTemplateSubRenderState(subRenderState);		
	}
#endif

	// Invalidate the scheme in order to re-generate all shaders based technique related to this scheme.
	mShaderGenerator->invalidateScheme(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::exportRTShaderSystemMaterial(const String& fileName, const String& materialName)
{
	// Grab material pointer.
	MaterialPtr materialPtr = MaterialManager::getSingleton().getByName(materialName);

	// Create shader based technique.
	bool success = mShaderGenerator->createShaderBasedTechnique(materialName,
		MaterialManager::DEFAULT_SCHEME_NAME,
		RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);

	// Case creation of shader based technique succeeded.
	if (success)
	{
		// Force shader generation of the given material.
		RTShader::ShaderGenerator::getSingleton().validateMaterial(RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME, materialName);

		// Grab the RTSS material serializer listener.
		MaterialSerializer::Listener* matRTSSListener = RTShader::ShaderGenerator::getSingleton().getMaterialSerializerListener();
		MaterialSerializer matSer;

		// Add the custom RTSS listener to the serializer.
		// It will make sure that every custom parameter needed by the RTSS 
		// will be added to the exported material script.
		matSer.addListener(matRTSSListener);

		// Simply export the material.
		matSer.exportMaterial(materialPtr, fileName, false, false, "", materialPtr->getName() + "_RTSS");
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
			"so you cannot run this sample. Sorry!", "Sample_ShaderSystem::testCapabilities");
	}

	// Check if D3D10 shader is supported - is so - then we are OK.
	if (GpuProgramManager::getSingleton().isSyntaxSupported("ps_4_0"))
	{
		return;
	}


	if (!GpuProgramManager::getSingleton().isSyntaxSupported("arbfp1") &&
		!GpuProgramManager::getSingleton().isSyntaxSupported("ps_2_0"))
	{
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your card does not support shader model 2, "
			"so you cannot run this sample. Sorry!", "Sample_ShaderSystem::testCapabilities");
	}
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::loadResources()
{
	// Create and add the custom reflection map shader extension factory to the shader generator.	
	mReflectionMapFactory = OGRE_NEW ShaderExReflectionMapFactory;
	mShaderGenerator->addSubRenderStateFactory(mReflectionMapFactory);
	
	createPrivateResourceGroup();
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::createPrivateResourceGroup()
{
	// Create the resource group of the RT Shader System Sample.
	ResourceGroupManager& rgm = ResourceGroupManager::getSingleton();
	Ogre::StringVector groupVector = Ogre::ResourceGroupManager::getSingleton().getResourceGroups();
	Ogre::StringVector::iterator itGroup = groupVector.begin();
	Ogre::StringVector::iterator itGroupEnd = groupVector.end();
	Ogre::String shaderCoreLibsPath;
	

	for (; itGroup != itGroupEnd; ++itGroup)
	{
		Ogre::ResourceGroupManager::LocationList resLocationsList = Ogre::ResourceGroupManager::getSingleton().getResourceLocationList(*itGroup);
		Ogre::ResourceGroupManager::LocationList::iterator it = resLocationsList.begin();
		Ogre::ResourceGroupManager::LocationList::iterator itEnd = resLocationsList.end();
		bool coreLibsFound = false;

		// Find the location of the core shader libs
		for (; it != itEnd; ++it)
		{
			if ((*it)->archive->getName().find("RTShaderLib") != Ogre::String::npos)
			{
				shaderCoreLibsPath = (*it)->archive->getName() + "/";	
				coreLibsFound = true;
				break;
			}
		}

		// Core libs path found in the current group.
		if (coreLibsFound) 
			break; 
	}

	mRTShaderLibsPath = shaderCoreLibsPath;

	rgm.createResourceGroup(SAMPLE_MATERIAL_GROUP, false);
	rgm.addResourceLocation(shaderCoreLibsPath + "materials", "FileSystem", SAMPLE_MATERIAL_GROUP);		
	rgm.initialiseResourceGroup(SAMPLE_MATERIAL_GROUP);
	rgm.loadResourceGroup(SAMPLE_MATERIAL_GROUP, true);
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::unloadResources()
{
	destroyPrivateResourceGroup();

	if (mReflectionMapFactory != NULL)
	{			
		OGRE_DELETE mReflectionMapFactory;
		mReflectionMapFactory = NULL;
	}
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::destroyPrivateResourceGroup()
{
	// Destroy the resource group of the RT Shader System	
	ResourceGroupManager& rgm = ResourceGroupManager::getSingleton();

	rgm.destroyResourceGroup(SAMPLE_MATERIAL_GROUP);
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::pickTargetObject( const OIS::MouseEvent &evt )
{
	int xPos   = evt.state.X.abs;
	int yPos   = evt.state.Y.abs;
	int width  = evt.state.width;
	int height = evt.state.height;

	Ray mouseRay = mCamera->getCameraToViewportRay(xPos / float(width), yPos/float(height));
	mRayQuery->setRay(mouseRay);

	RaySceneQueryResult &result = mRayQuery->execute();
	RaySceneQueryResult::iterator it = result.begin();
	RaySceneQueryResult::iterator itEnd = result.end();

	for (; it != itEnd; ++it)
	{
		RaySceneQueryResultEntry& curEntry = *it;
		
		if (mTargetObj != NULL)
		{
			mTargetObj->getParentSceneNode()->showBoundingBox(false);			
		}

		mTargetObj = curEntry.movable;
		mTargetObj ->getParentSceneNode()->showBoundingBox(true);		
	}
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::updateTargetObjInfo()
{
	if (mTargetObj == NULL)
		return;

	String targetObjMaterialName;

	if (mTargetObj->getMovableType() == "Entity")
	{
		Entity* targetEnt = static_cast<Entity*>(mTargetObj);
		targetObjMaterialName = targetEnt->getSubEntity(0)->getMaterialName();
	}
	
	mTargetObjMatName->setCaption(targetObjMaterialName);

	if (mViewport->getMaterialScheme() == RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME)
	{		
		MaterialPtr matMainEnt        = MaterialManager::getSingleton().getByName(targetObjMaterialName);

		if (matMainEnt.isNull() == false)
		{
			Technique* shaderGeneratedTech = NULL;

			for (unsigned int i=0; i < matMainEnt->getNumTechniques(); ++i)
			{
				Technique* curTech = matMainEnt->getTechnique(i);

				if (curTech->getSchemeName() == RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME)
				{
					shaderGeneratedTech = curTech;
					break;
				}
			}

			if (shaderGeneratedTech != NULL)
			{			
				mTargetObjVS->setCaption("VS: " + shaderGeneratedTech->getPass(0)->getVertexProgramName());
				mTargetObjFS->setCaption("FS: " + shaderGeneratedTech->getPass(0)->getFragmentProgramName());
			}	

		}


	}
	else
	{		
		mTargetObjVS->setCaption("VS: N/A");
		mTargetObjFS->setCaption("FS: N/A");
	}
}

//-----------------------------------------------------------------------
bool Sample_ShaderSystem::mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
{	
	if (id == OIS::MB_Right && !mTrayMgr->injectMouseDown(evt, id)) pickTargetObject(evt);
	return SdkSample::mousePressed(evt, id);
}
