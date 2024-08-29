#include "SamplePlugin.h"
#include "OgreShaderSubRenderState.h"
#include "ShaderSystem.h"
#include "OgreShaderExInstancedViewports.h"
#include "OgreShaderExTextureAtlasSampler.h"
#include "OgreBillboard.h"
using namespace Ogre;
using namespace OgreBites;
using namespace Ogre::RTShader;

//-----------------------------------------------------------------------
const String DIRECTIONAL_LIGHT_NAME     = "DirectionalLight";
const String POINT_LIGHT_NAME           = "PointLight";
const String INSTANCED_VIEWPORTS_NAME   = "InstancedViewports";
const String ADD_LOTS_OF_MODELS_NAME    = "AddLotsOfModels";
const String SPOT_LIGHT_NAME            = "SpotLight";
const String PER_PIXEL_FOG_BOX          = "PerPixelFog";
const String ATLAS_AUTO_BORDER_MODE     = "AutoBorderAtlasing";
const String MAIN_ENTITY_MESH           = "ShaderSystem.mesh";
const String SPECULAR_BOX               = "SpecularBox";
const String REFLECTIONMAP_BOX          = "ReflectionMapBox";
const String REFLECTIONMAP_POWER_SLIDER = "ReflectionPowerSlider";
const String MAIN_ENTITY_NAME           = "MainEntity";
const String FLUSH_BUTTON_NAME          = "FlushShaderCache";
const String LAYERBLEND_BUTTON_NAME     = "ChangeLayerBlendType";
const String MODIFIER_VALUE_SLIDER      = "ModifierValueSlider";
const String SAMPLE_MATERIAL_GROUP      = "RTShaderSystemMaterialsGroup";
const int MESH_ARRAY_SIZE = 2;
const String MESH_ARRAY[MESH_ARRAY_SIZE] =
{
    MAIN_ENTITY_MESH,
    "knot.mesh"
};

const char* blendModes[] = {"default",     "normal",       "lighten",     "darken",      "multiply",   "average",
                            "add",         "subtract",     "difference",  "negation",    "exclusion",  "screen",
                            "overlay",     "hard_light",   "soft_light",  "color_dodge", "color_burn", "linear_dodge",
                            "linear_burn", "linear_light", "vivid_light", "pin_light",   "hard_mix",   "reflect",
                            "glow",        "phoenix",      "saturation",  "color",       "luminosity"};
#define NUM_BLEND_MODES (sizeof(blendModes) / sizeof(blendModes[0]))

//-----------------------------------------------------------------------
Sample_ShaderSystem::Sample_ShaderSystem() :
    mLayeredBlendingEntity(NULL)
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
    mTextureAtlasFactory = NULL;
    mInstancedViewportsEnable = false;
    mInstancedViewportsSubRenderState = NULL;
    mInstancedViewportsFactory = NULL;
    mBbsFlare = NULL;
    mAddedLotsOfModels = false;
    mNumberOfModelsAdded = 0;
    mCurrentBlendMode = NUM_BLEND_MODES - 1;
}
//-----------------------------------------------------------------------
Sample_ShaderSystem::~Sample_ShaderSystem()
{
}
//-----------------------------------------------------------------------

void Sample_ShaderSystem::_shutdown()
{
    mShaderGenerator->getRenderState(MSN_SHADERGEN)->resetToBuiltinSubRenderStates();
    destroyInstancedViewports();
    SdkSample::_shutdown();
}

//-----------------------------------------------------------------------

void Sample_ShaderSystem::checkBoxToggled(CheckBox* box)
{
    const String& cbName = box->getName();

    if (cbName == SPECULAR_BOX)
    {
        setSpecularEnable(box->isChecked());
    }
    else if (cbName == DIRECTIONAL_LIGHT_NAME)
    {
        updateLightState(cbName, box->isChecked());     
    }
    else if (cbName == POINT_LIGHT_NAME)
    {
        updateLightState(cbName, box->isChecked());
    }
    else if (cbName == INSTANCED_VIEWPORTS_NAME)
    {
        updateInstancedViewports(box->isChecked());
    }
    else if (cbName == ADD_LOTS_OF_MODELS_NAME)
    {
        updateAddLotsOfModels(box->isChecked());
    }
    else if (cbName == SPOT_LIGHT_NAME)
    {
        updateLightState(cbName, box->isChecked());
    }
    else if (cbName == PER_PIXEL_FOG_BOX)
    {
        setPerPixelFogEnable(box->isChecked());
    }
    else if (cbName == ATLAS_AUTO_BORDER_MODE)
    {
        setAtlasBorderMode(box->isChecked());
    }
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::itemSelected(SelectMenu* menu)
{
    if (menu == mLightingModelMenu)
    {
        int curModelIndex = menu->getSelectionIndex();

        if (curModelIndex >= SSLM_PerPixelLighting&& curModelIndex <= SSLM_NormalMapLightingObjectSpace)
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
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::buttonHit( OgreBites::Button* b )
{
    // Case the shader cache should be flushed.
    if (b->getName() == FLUSH_BUTTON_NAME)
    {               
        mShaderGenerator->flushShaderCache();
    }

    // Case the blend layer type modified.
    else if (b->getName() == LAYERBLEND_BUTTON_NAME && mLayerBlendSubRS)
    {   
        changeTextureLayerBlendMode();
        
    }
}

//--------------------------------------------------------------------------
void Sample_ShaderSystem::sliderMoved(Slider* slider)
{
    if (slider->getName() == REFLECTIONMAP_POWER_SLIDER && mReflectionMapSubRS)
    {
        String luminance = std::to_string(slider->getValue());

        // Grab the instances set and update them with the new reflection power value.
        // The instances are the actual sub render states that have been assembled to create the final shaders.
        // Every time that the shaders have to be re-generated (light changes, fog changes etc..) a new set of sub render states
        // based on the template sub render states assembled for each pass.
        // From that set of instances a CPU program is generated and afterward a GPU program finally generated.
        auto instanceSet = mReflectionMapSubRS->getAccessor()->getSubRenderStateInstanceSet();
        for (auto inst : instanceSet)
        {
            inst->setParameter("luminance", luminance);
        }
    }   

    if (slider->getName() == MODIFIER_VALUE_SLIDER)
    {
        if (mLayeredBlendingEntity != NULL)
        {
            Ogre::Real val = mModifierValueSlider->getValue();
            mLayeredBlendingEntity->getSubEntity(0)->setCustomParameter(2, Vector4f(val,val,val,0));
        }
    }
}

//-----------------------------------------------------------------------
bool Sample_ShaderSystem::frameRenderingQueued( const FrameEvent& evt )
{
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
void Sample_ShaderSystem::setupContent()
{
    mTextureAtlasFactory = OGRE_NEW TextureAtlasSamplerFactory;
    mShaderGenerator->addSubRenderStateFactory(mTextureAtlasFactory);

    // Setup default effects values.
    mCurLightingModel       = SSLM_PerPixelLighting;
    mPerPixelFogEnable      = false;
    mSpecularEnable         = false;
    mReflectionMapSubRS     = NULL;
    mLayerBlendSubRS        = NULL;

    mRayQuery = mSceneMgr->createRayQuery(Ray());
    mTargetObj = NULL;


    // Set ambient lighting.
    mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));

    // Setup the sky box,
    mSceneMgr->setSkyBox(true, "Examples/SceneSkyBox2");

    MeshManager::getSingleton().createPlane("Myplane",
        ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Plane(Vector3::UNIT_Y, 0),
        1500,1500,25,25,true,1,60,60,Vector3::UNIT_Z);

    Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
    pPlaneEnt->setMaterialName("Examples/Rockwall");
    pPlaneEnt->setCastShadows(false);
    mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0,0,0))->attachObject(pPlaneEnt);

    mCamera->setNearClipDistance(30);

    // Load sample meshes and generate tangent vectors.
    for (const auto & curMeshName : MESH_ARRAY)
    {
        MeshPtr pMesh = MeshManager::getSingleton().load(curMeshName,
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,    
            HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, 
            HardwareBuffer::HBU_STATIC_WRITE_ONLY, 
            true, true); //so we can still read it

        // Build tangent vectors, all our meshes use only 1 texture coordset 
        pMesh->buildTangentVectors();
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
    MaterialPtr matMainEnt = mSceneMgr->getEntity(MAIN_ENTITY_NAME)->getSubEntity(0)->getMaterial()->clone("_RTSS_Export");

    entity = mSceneMgr->createEntity("ExportedMaterialEntity", MAIN_ENTITY_MESH);
    entity->setMaterial(matMainEnt);
    childNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    childNode->setPosition(0.0, 200.0, -200.0);
    childNode->attachObject(entity);

    // Create texture layer blending demonstration entity.
    mLayeredBlendingEntity = mSceneMgr->createEntity("LayeredBlendingMaterialEntity", MAIN_ENTITY_MESH);
    mLayeredBlendingEntity->setMaterialName("RTSS/LayeredBlending");
    mLayeredBlendingEntity->getSubEntity(0)->setCustomParameter(2, Vector4f(0));
    childNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    childNode->setPosition(300.0, 200.0, -200.0);
    childNode->attachObject(mLayeredBlendingEntity);

    // Grab the render state of the material.
    auto renderState = mShaderGenerator->getRenderState(MSN_SHADERGEN, "RTSS/LayeredBlending", RGN_DEFAULT, 0);

    if (renderState)
    {           
        // Search for the texture layer blend sub state.
        if (auto srs = renderState->getSubRenderState(SRS_LAYERED_BLENDING))
        {
            mLayerBlendSubRS = static_cast<LayeredBlending*>(srs);
        }
    }


    // Create per pixel lighting demonstration entity.
    entity = mSceneMgr->createEntity("PerPixelEntity", "knot.mesh");
    entity->setMaterialName("RTSS/PerPixel_SinglePass");
    childNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    childNode->setPosition(300.0, 100.0, -100.0);
    childNode->attachObject(entity);

    // Create normal map lighting demonstration entity.
    entity = mSceneMgr->createEntity("NormalMapEntity", "knot.mesh");
    entity->setMaterialName("RTSS/NormalMapping_SinglePass");
    childNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    childNode->setPosition(-300.0, 100.0, -100.0);
    childNode->attachObject(entity);

    // OpenGL ES 2.0 does not support texture atlases. But ES 3.0 does!
    if (Root::getSingletonPtr()->getRenderSystem()->getName().find("OpenGL ES 2") == String::npos
            || Root::getSingletonPtr()->getRenderSystem()->getNativeShadingLanguageVersion() >= 300)
    {
        RTShader::RenderState* pMainRenderState =
            RTShader::ShaderGenerator::getSingleton().createOrRetrieveRenderState(MSN_SHADERGEN).first;
        pMainRenderState->addTemplateSubRenderState(
            mShaderGenerator->createSubRenderState<RTShader::TextureAtlasSampler>());

        // Create texture atlas object and node
        ManualObject* atlasObject = createTextureAtlasObject();
        childNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        childNode->setPosition(-600.0, 0, -850.0);
        childNode->attachObject(atlasObject);
    }

    createDirectionalLight();
    createPointLight();
    createSpotLight();
    
    setupUI();


    mCameraNode->setPosition(0.0, 300.0, 450.0);
    mCameraNode->lookAt(Vector3(0.0, 150.0, 0.0), Node::TS_PARENT);

    // Make this viewport work with shader generator scheme.
    mViewport->setMaterialScheme(MSN_SHADERGEN);
    
    // a friendly reminder
    StringVector names;
    names.push_back("Help");
    mTrayMgr->createParamsPanel(TL_TOPLEFT, "Help", 100, names)->setParamValue(0, "H/F1");

    updateSystemShaders();
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::setupUI()
{
    // Create language label
    mLanguage = mTrayMgr->createLabel(TL_TOPLEFT, "Language", "Language: "+mShaderGenerator->getTargetLanguage(), 220);

    // create check boxes to toggle lights. 
    mDirLightCheckBox = mTrayMgr->createCheckBox(TL_TOPLEFT, DIRECTIONAL_LIGHT_NAME, "Directional Light", 220);
    mPointLightCheckBox = mTrayMgr->createCheckBox(TL_TOPLEFT, POINT_LIGHT_NAME, "Point Light", 220);
    mSpotLightCheckBox = mTrayMgr->createCheckBox(TL_TOPLEFT, SPOT_LIGHT_NAME, "Spot Light", 220);

    mInstancedViewportsCheckBox = mTrayMgr->createCheckBox(TL_TOPLEFT, INSTANCED_VIEWPORTS_NAME, "Instanced Viewports", 220);
    mAddLotsOfModels = mTrayMgr->createCheckBox(TL_TOPLEFT, ADD_LOTS_OF_MODELS_NAME, "Add lots of models", 220);
    
    mDirLightCheckBox->setChecked(true);
    mPointLightCheckBox->setChecked(true);
    mSpotLightCheckBox->setChecked(false);
    mInstancedViewportsCheckBox->setChecked(false);
    mAddLotsOfModels->setChecked(false);


#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
    mTrayMgr->createCheckBox(TL_TOPLEFT, PER_PIXEL_FOG_BOX, "Per Pixel Fog", 220)->setChecked(mPerPixelFogEnable);
#endif

#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
    mTrayMgr->createCheckBox(TL_TOPLEFT, ATLAS_AUTO_BORDER_MODE, "Atlas auto border", 220)->setChecked(true);
    setAtlasBorderMode(true);
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
    mShadowMenu->addItem("PSSM debug");
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

    mLightingModelMenu = mTrayMgr->createLongSelectMenu(TL_BOTTOM, "TargetModelLighting", "", 240, 230, 10);    
    mLightingModelMenu ->addItem("Per Pixel");
    mLightingModelMenu ->addItem("Cook Torrance");
    mLightingModelMenu ->addItem("Image Based");
    mLightingModelMenu ->addItem("Normal Map - Tangent Space");
    mLightingModelMenu ->addItem("Normal Map - Object Space");

    mReflectionPowerSlider = mTrayMgr->createThickSlider(TL_BOTTOM, REFLECTIONMAP_POWER_SLIDER, "IBL Luminace", 240, 80, 0, 5, 100);
    mReflectionPowerSlider->setValue(0.5, false);

#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
    mLayerBlendLabel = mTrayMgr->createLabel(TL_RIGHT, "Blend Type", "Blend Type", 240);
    mTrayMgr->createButton(TL_RIGHT, LAYERBLEND_BUTTON_NAME, "Change Blend Type", 220);
    mModifierValueSlider = mTrayMgr->createThickSlider(TL_RIGHT, MODIFIER_VALUE_SLIDER, "Modifier", 240, 80, 0, 1, 100);
    mModifierValueSlider->setValue(0.0,false);  
    // Update the caption.
    mLayerBlendLabel->setCaption(blendModes[mCurrentBlendMode]);
#endif

    mTrayMgr->showCursor();
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::cleanupContent()
{   
    // UnLoad sample meshes and generate tangent vectors.
    for (const auto & curMeshName : MESH_ARRAY)
    {
        MeshManager::getSingleton().unload(curMeshName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    }
    
    MeshManager::getSingleton().remove(MAIN_ENTITY_MESH, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    mTargetEntities.clear();

    MeshManager::getSingleton().remove("Myplane", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    mSceneMgr->destroyQuery(mRayQuery);
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::setCurrentLightingModel(ShaderSystemLightingModel lightingModel)
{
    if (mCurLightingModel != lightingModel)
    {
        mCurLightingModel  = lightingModel;

        updateSystemShaders();
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
void Sample_ShaderSystem::setPerPixelFogEnable( bool enable )
{
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
    if (mPerPixelFogEnable != enable)
    {
        mPerPixelFogEnable = enable;

        // Grab the scheme render state.
        RenderState* schemRenderState = mShaderGenerator->getRenderState(MSN_SHADERGEN);
        // Search for the fog sub state.
        auto fogSubRenderState = schemRenderState->getSubRenderState(SRS_FOG);

        // Select the desired fog calculation mode.
        fogSubRenderState->setParameter("calc_mode", mPerPixelFogEnable ? "per_pixel" : "per_vertex");

        // Invalidate the scheme in order to re-generate all shaders based technique related to this scheme.
        mShaderGenerator->invalidateScheme(Ogre::MSN_SHADERGEN);
    }
#endif

}

void Sample_ShaderSystem::setAtlasBorderMode( bool enable )
{
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
    TextureAtlasSamplerFactory::getSingleton().setDefaultAtlasingAttributes(
        TextureAtlasSamplerFactory::ipmRelative, 1, enable);
    mShaderGenerator->invalidateScheme(Ogre::MSN_SHADERGEN);
#endif
}


//-----------------------------------------------------------------------
void Sample_ShaderSystem::updateSystemShaders() 
{
    for (auto e : mTargetEntities)
    {
        generateShaders(e);
    }   
}

//-----------------------------------------------------------------------
static void setNormalMap(Pass* pass, const String& normalMapName)
{
    if(pass->getNumTextureUnitStates() > 1)
    {
        // remove the previous normal map
        pass->removeTextureUnitState(1);
    }

    auto tu = pass->createTextureUnitState(normalMapName);
    ShaderGenerator::_markNonFFP(tu);
}

void Sample_ShaderSystem::generateShaders(Entity* entity)
{
    for (unsigned int i=0; i < entity->getNumSubEntities(); ++i)
    {
        SubEntity* curSubEntity = entity->getSubEntity(i);
        MaterialPtr curMaterial = curSubEntity->getMaterial();
        bool success;

        // Create the shader based technique of this material.
        success = mShaderGenerator->createShaderBasedTechnique(*curMaterial, MSN_DEFAULT, MSN_SHADERGEN);

        // Setup custom shader sub render states according to current setup.
        if (success)
        {                   
            Pass* curPass = curMaterial->getTechnique(0)->getPass(0);

            bool cookTorrance = mCurLightingModel == SSLM_CookTorranceLighting || mCurLightingModel == SSLM_ImageBasedLighting;

            if (mSpecularEnable)
            {
                curPass->setSpecular(!cookTorrance ? ColourValue::White : ColourValue(0.1, 0.0));
                curPass->setShininess(32.0);
            }
            else
            {
                curPass->setSpecular(!cookTorrance ? ColourValue::Black : ColourValue(1.0, 0.0));
                curPass->setShininess(0.0);
            }
            

            // Grab the first pass render state. 
            // NOTE: For more complicated samples iterate over the passes and build each one of them as desired.
            RTShader::RenderState* renderState = mShaderGenerator->getRenderState(
                MSN_SHADERGEN, *curMaterial);

            // Remove all sub render states.
            renderState->resetToBuiltinSubRenderStates();


#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
            if (mCurLightingModel == SSLM_CookTorranceLighting || mCurLightingModel == SSLM_ImageBasedLighting)
            {
                auto lightModel = mShaderGenerator->createSubRenderState(RTShader::SRS_COOK_TORRANCE_LIGHTING);
                renderState->addTemplateSubRenderState(lightModel);
            }
            else if (mCurLightingModel == SSLM_PerPixelLighting)
            {
                RTShader::SubRenderState* perPixelLightModel = mShaderGenerator->createSubRenderState(RTShader::SRS_PER_PIXEL_LIGHTING);
                
                renderState->addTemplateSubRenderState(perPixelLightModel);             
            }
            else if (mCurLightingModel == SSLM_NormalMapLightingObjectSpace)
            {
                // Apply normal map only on main entity.
                if (entity->getName() == MAIN_ENTITY_NAME)
                {
                    RTShader::SubRenderState* normalMapSubRS = mShaderGenerator->createSubRenderState(RTShader::SRS_NORMALMAP);

                    normalMapSubRS->setParameter("normalmap_space", "object_space");
                    setNormalMap(curPass, "Panels_Normal_Obj.png");
                    normalMapSubRS->setParameter("texture_index", "1");
                    renderState->addTemplateSubRenderState(normalMapSubRS);
                }

                // It is secondary entity -> use simple per pixel lighting.
                else
                {
                    RTShader::SubRenderState* perPixelLightModel = mShaderGenerator->createSubRenderState(RTShader::SRS_PER_PIXEL_LIGHTING);
                    renderState->addTemplateSubRenderState(perPixelLightModel);
                }               
            }

            if (mCurLightingModel == SSLM_NormalMapLightingTangentSpace ||
                mCurLightingModel == SSLM_CookTorranceLighting || mCurLightingModel == SSLM_ImageBasedLighting)
            {
                // Apply normal map only on main entity.
                if (entity->getName() == MAIN_ENTITY_NAME)
                {
                    RTShader::SubRenderState* normalMapSubRS = mShaderGenerator->createSubRenderState(RTShader::SRS_NORMALMAP);

                    normalMapSubRS->setParameter("normalmap_space", "tangent_space");
                    setNormalMap(curPass, "Panels_Normal_Tangent.png");
                    normalMapSubRS->setParameter("texture_index", "1");

                    renderState->addTemplateSubRenderState(normalMapSubRS);
                }

                // It is secondary entity -> use simple per pixel lighting.
                else
                {
                    RTShader::SubRenderState* perPixelLightModel = mShaderGenerator->createSubRenderState(RTShader::SRS_PER_PIXEL_LIGHTING);
                    renderState->addTemplateSubRenderState(perPixelLightModel);
                }
            }
#endif
            if (mCurLightingModel == SSLM_ImageBasedLighting)
            {               
                RTShader::SubRenderState* subRenderState = mShaderGenerator->createSubRenderState(RTShader::SRS_IMAGE_BASED_LIGHTING);
                subRenderState->setParameter("texture", "studio_garden.jpg");
                subRenderState->setParameter("luminance", std::to_string(mReflectionPowerSlider->getValue()));
                renderState->addTemplateSubRenderState(subRenderState);
                mReflectionMapSubRS = subRenderState;               
            }
            else
            {
                mReflectionMapSubRS = NULL;
            }
                                
            // Invalidate this material in order to re-generate its shaders.
            mShaderGenerator->invalidateMaterial(MSN_SHADERGEN,
                                                 *curMaterial);
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
    light->setDiffuseColour(0.65, 0.15, 0.15);
    light->setSpecularColour(0.5, 0.5, 0.5);

    // create pivot node
    mDirectionalLightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    mDirectionalLightNode->setDirection(dir);

    // Create billboard set.
    mBbsFlare = mSceneMgr->createBillboardSet();
    mBbsFlare->setMaterialName("Examples/Flare3");
    mBbsFlare->createBillboard(-dir * 500.0)->setColour(light->getDiffuseColour());
    mBbsFlare->setCastShadows(false);
    
    mDirectionalLightNode->attachObject(mBbsFlare);
    mDirectionalLightNode->attachObject(light);
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::createPointLight()
{
    Light*  light;

    light = mSceneMgr->createLight(POINT_LIGHT_NAME);
    light->setType(Light::LT_POINT);
    light->setCastShadows(false);
    light->setDiffuseColour(0.15, 0.65, 0.15);
    light->setSpecularColour(0.5, 0.5, 0.5);    
    light->setAttenuation(250.0, 0.5, 0, 0.00003);

    // create pivot node
    mPointLightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    
    BillboardSet* bbs;
    
    // Create billboard set.
    bbs = mSceneMgr->createBillboardSet();
    bbs->setMaterialName("Examples/Flare3");
    bbs->createBillboard(200, 100, 0)->setColour(light->getDiffuseColour());
    bbs->setCastShadows(false);

    mPointLightNode->attachObject(bbs);
    SceneNode* ln = mPointLightNode->createChildSceneNode(Vector3(200, 100, 0));
    ln->attachObject(light);
    ln->setDirection(1, 0, 0);
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::createSpotLight()
{
    Light*  light;
    light = mSceneMgr->createLight(SPOT_LIGHT_NAME);
    light->setType(Light::LT_SPOTLIGHT);
    light->setCastShadows(false);
    light->setSpotlightRange(Degree(20.0), Degree(25.0), 0.95);
    light->setDiffuseColour(0.15, 0.15, 0.65);
    light->setSpecularColour(0.5, 0.5, 0.5);    
    light->setAttenuation(1000.0, 1.0, 0.0005, 0.0);

    auto ln = mCameraNode->createChildSceneNode(Vector3::UNIT_Y * 20);
    ln->attachObject(light);
}

void Sample_ShaderSystem::addModelToScene(const String &  modelName)
{
    mNumberOfModelsAdded++;
    for(int i = 0 ; i < 8 ; i++)
    {
        float scaleFactor = 30;
        Entity* entity;
        SceneNode* childNode;
        entity = mSceneMgr->createEntity(modelName);
        mLotsOfModelsEntities.push_back(entity);
        childNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mLotsOfModelsNodes.push_back(childNode);
        childNode->setPosition(mNumberOfModelsAdded * scaleFactor, 15,  i * scaleFactor);
        childNode->attachObject(entity);
        MeshPtr modelMesh = MeshManager::getSingleton().getByName(modelName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        Vector3 modelSize = modelMesh->getBounds().getSize();
        childNode->scale(1 / modelSize.x * scaleFactor, 
                         1 / modelSize.y * scaleFactor, 
                         1 / modelSize.z * scaleFactor
                         );
    }
}

void Sample_ShaderSystem::updateAddLotsOfModels(bool addThem)
{
    if (mAddedLotsOfModels != addThem)
    {
        mAddedLotsOfModels = addThem;
        
        if(mNumberOfModelsAdded == 0)
        {
            addModelToScene("Barrel.mesh");
            addModelToScene("facial.mesh");
            addModelToScene("fish.mesh");
            addModelToScene("ninja.mesh");
            addModelToScene("penguin.mesh");
            addModelToScene("razor.mesh");
            addModelToScene("RZR-002.mesh");
            addModelToScene("tudorhouse.mesh");
            addModelToScene("WoodPallet.mesh");
        }
        for (auto & n : mLotsOfModelsNodes)
        {
            n->setVisible(mAddedLotsOfModels);
        }
        
    }
}
//-----------------------------------------------------------------------
void Sample_ShaderSystem::updateInstancedViewports(bool enabled)
{
    if (mInstancedViewportsEnable != enabled)
    {
        mInstancedViewportsEnable = enabled;

        if (mInstancedViewportsEnable)
        {
            mCamera->setCullingFrustum(&mInfiniteFrustum);

            // having problems with bb...
            mDirectionalLightNode->detachObject(mBbsFlare);
        }
        else
        {
            mCamera->setCullingFrustum(NULL);
            mDirectionalLightNode->attachObject(mBbsFlare);
        }



        if(mInstancedViewportsEnable)
        {
            createInstancedViewports();
        }
        else
        {
            destroyInstancedViewports();
        }

    }
}
//-----------------------------------------------------------------------
void Sample_ShaderSystem::updateLightState(const String& lightName, bool visible)
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
            mDirectionalLightNode->setVisible(visible, false);
        }

        // Spot light has no scene node representation.
        else
        {
            mSceneMgr->getLight(lightName)->setVisible(visible);
        }

        // Invalidate the scheme in order to re-generate all shaders based technique related to this scheme.
        mShaderGenerator->invalidateScheme(Ogre::MSN_SHADERGEN);
    }
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::applyShadowType(int menuIndex)
{
    // Grab the scheme render state.                                                
    Ogre::RTShader::RenderState* schemRenderState = mShaderGenerator->getRenderState(Ogre::MSN_SHADERGEN);


    // No shadow
    if (menuIndex == 0)
    {
        mSceneMgr->setShadowTechnique(SHADOWTYPE_NONE);

#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
        if (auto srs = schemRenderState->getSubRenderState(SRS_SHADOW_MAPPING))
        {
            schemRenderState->removeSubRenderState(srs);
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
    else if (menuIndex >= 1)
    {
        mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE_INTEGRATED);
        mSceneMgr->setShadowFarDistance(3000);

        // 3 textures per directional light
        mSceneMgr->setShadowTextureCountPerLightType(Ogre::Light::LT_DIRECTIONAL, 3);
        mSceneMgr->setShadowTextureSettings(512, 3, PF_DEPTH16);
        mSceneMgr->setShadowTextureSelfShadow(true);

        // Leave only directional light.
        mDirLightCheckBox->setChecked(true);
        mPointLightCheckBox->setChecked(false);
        mSpotLightCheckBox->setChecked(false);

        mTrayMgr->removeWidgetFromTray(mDirLightCheckBox);
        mDirLightCheckBox->hide();

        // shadow camera setup
        PSSMShadowCameraSetup* pssmSetup = new PSSMShadowCameraSetup();
        pssmSetup->calculateSplitPoints(3, mCamera->getNearClipDistance(), mSceneMgr->getShadowFarDistance());
        pssmSetup->setSplitPadding(mCamera->getNearClipDistance()*2);
        pssmSetup->setOptimalAdjustFactor(0, 2);
        pssmSetup->setOptimalAdjustFactor(1, 1);
        pssmSetup->setOptimalAdjustFactor(2, 0.5);

        mSceneMgr->setShadowCameraSetup(ShadowCameraSetupPtr(pssmSetup));

    
        auto subRenderState = mShaderGenerator->createSubRenderState(SRS_SHADOW_MAPPING);
        subRenderState->setParameter("split_points", pssmSetup->getSplitPoints());
        subRenderState->setParameter("debug", menuIndex > 1);
        schemRenderState->addTemplateSubRenderState(subRenderState);        
    }
#endif

    // Invalidate the scheme in order to re-generate all shaders based technique related to this scheme.
    mShaderGenerator->invalidateScheme(Ogre::MSN_SHADERGEN);
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::testCapabilities( const RenderSystemCapabilities* caps )
{
    if(RTShader::ShaderGenerator::getSingleton().getTargetLanguage() != "null")
        return;

    OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "RTSS not supported on your system");
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::unloadResources()
{
    if (mTextureAtlasFactory != NULL)
    {
        mTextureAtlasFactory->destroyAllInstances();
        mShaderGenerator->removeSubRenderStateFactory(mTextureAtlasFactory);
        OGRE_DELETE mTextureAtlasFactory;
        mTextureAtlasFactory = NULL;
    }
}

//-----------------------------------------------------------------------
void Sample_ShaderSystem::pickTargetObject( const MouseButtonEvent &evt )
{
    int xPos   = evt.x;
    int yPos   = evt.y;
    int width  = mWindow->getWidth(), height = mWindow->getHeight();

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

    if (mTargetObj->getMovableType() == MOT_ENTITY)
    {
        Entity* targetEnt = static_cast<Entity*>(mTargetObj);
        targetObjMaterialName = targetEnt->getSubEntity(0)->getMaterialName();
    }
    
    mTargetObjMatName->setCaption(targetObjMaterialName);

    if (mViewport->getMaterialScheme() == MSN_SHADERGEN)
    {       
        MaterialPtr matMainEnt        = MaterialManager::getSingleton().getByName(targetObjMaterialName);

        if (!matMainEnt == false)
        {
            Technique* shaderGeneratedTech = NULL;

            for (unsigned int i=0; i < matMainEnt->getNumTechniques(); ++i)
            {
                Technique* curTech = matMainEnt->getTechnique(i);

                if (curTech->getSchemeName() == MSN_SHADERGEN)
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
void Sample_ShaderSystem::changeTextureLayerBlendMode()
{
    // Update the next blend layer mode.
    mCurrentBlendMode = (mCurrentBlendMode + 1) % NUM_BLEND_MODES;

    mLayerBlendSubRS->setBlendMode(1, blendModes[mCurrentBlendMode]);
    mShaderGenerator->invalidateMaterial(MSN_SHADERGEN, "RTSS/LayeredBlending", RGN_DEFAULT);

    mLayerBlendLabel->setCaption(blendModes[mCurrentBlendMode]);
}

//-----------------------------------------------------------------------
bool Sample_ShaderSystem::mousePressed(const MouseButtonEvent& evt)
{
    if (mTrayMgr->mousePressed(evt)) 
        return true;
    if (evt.button == BUTTON_LEFT)     
        mTrayMgr->hideCursor();  // hide the cursor if user left-clicks in the scene            
    if (evt.button == BUTTON_RIGHT) 
        pickTargetObject(evt);

    return true;
}

//-----------------------------------------------------------------------
bool Sample_ShaderSystem::mouseReleased(const MouseButtonEvent& evt)
{
    if (mTrayMgr->mouseReleased(evt)) 
        return true;
    if (evt.button == BUTTON_LEFT) 
        mTrayMgr->showCursor();  // unhide the cursor if user lets go of LMB

    return true;
}

//-----------------------------------------------------------------------
bool Sample_ShaderSystem::mouseMoved(const MouseMotionEvent& evt)
{
    // only rotate the camera if cursor is hidden
    if (mTrayMgr->isCursorVisible()) 
        mTrayMgr->mouseMoved(evt);
    else 
        mCameraMan->mouseMoved(evt);


    return true;
}
//-----------------------------------------------------------------------

void Sample_ShaderSystem::destroyInstancedViewports()
{
    if (mInstancedViewportsSubRenderState)
    {
        Ogre::RTShader::RenderState* renderState = mShaderGenerator->getRenderState(Ogre::MSN_SHADERGEN);
        renderState->removeSubRenderState(mInstancedViewportsSubRenderState);
        mInstancedViewportsSubRenderState = NULL;
    }

    mShaderGenerator->invalidateScheme(Ogre::MSN_SHADERGEN);
    mShaderGenerator->validateScheme(Ogre::MSN_SHADERGEN);

    destroyInstancedViewportsFactory();

}
//-----------------------------------------------------------------------
void Sample_ShaderSystem::destroyInstancedViewportsFactory()
{
    if (mInstancedViewportsFactory != NULL)
    {
        mInstancedViewportsFactory->destroyAllInstances();
        mShaderGenerator->removeSubRenderStateFactory(mInstancedViewportsFactory);
        delete mInstancedViewportsFactory;
        mInstancedViewportsFactory = NULL;
    }
}
//-----------------------------------------------------------------------

void Sample_ShaderSystem::createInstancedViewports()
{
    if (mInstancedViewportsFactory == NULL)
    {
        mInstancedViewportsFactory = OGRE_NEW ShaderExInstancedViewportsFactory;    
        mShaderGenerator->addSubRenderStateFactory(mInstancedViewportsFactory);
    }

    Ogre::Vector2 monitorCount(2.0, 2.0);
    mInstancedViewportsSubRenderState = mShaderGenerator->createSubRenderState<RTShader::ShaderExInstancedViewports>();
    Ogre::RTShader::ShaderExInstancedViewports* shaderExInstancedViewports 
        = static_cast<Ogre::RTShader::ShaderExInstancedViewports*>(mInstancedViewportsSubRenderState);
    shaderExInstancedViewports->setMonitorsCount(monitorCount);
    Ogre::RTShader::RenderState* renderState = mShaderGenerator->getRenderState(Ogre::MSN_SHADERGEN);
    renderState->addTemplateSubRenderState(mInstancedViewportsSubRenderState);

    // Invalidate the scheme in order to re-generate all shaders based technique related to this scheme.
    mShaderGenerator->invalidateScheme(Ogre::MSN_SHADERGEN);
    mShaderGenerator->validateScheme(Ogre::MSN_SHADERGEN);
}

void Sample_ShaderSystem::createMaterialForTexture( const String & texName, bool isTextureAtlasTexture )
{
    MaterialManager * matMgr = MaterialManager::getSingletonPtr();
    if ( !matMgr->resourceExists(texName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME) )
    {
        MaterialPtr newMat = matMgr->create(texName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        newMat->getTechnique(0)->getPass(0)->setLightingEnabled(false);
        TextureUnitState* pState = newMat->getTechnique(0)->getPass(0)->createTextureUnitState(texName);
        if(isTextureAtlasTexture) 
        {
            // to solve wrap edge bleed
            pState->setTextureFiltering(TFO_TRILINEAR);
        }
    }

}

ManualObject* Sample_ShaderSystem::createTextureAtlasObject()
{
    TextureAtlasSamplerFactory * textureAtlasSamplerFactory = 
        static_cast<TextureAtlasSamplerFactory *>(mShaderGenerator->getSubRenderStateFactory(TextureAtlasSampler::Type));
    TextureAtlasTablePtr textureAtlasTable(new TextureAtlasTable);

    DataStreamPtr taiFile = Ogre::ResourceGroupManager::getSingleton().openResource("TextureAtlasSampleWrap.tai");

    textureAtlasSamplerFactory->addTexutreAtlasDefinition(taiFile, textureAtlasTable);

    //Generate the geometry that will seed the particle system
    ManualObject* textureAtlasObject = mSceneMgr->createManualObject("TextureAtlasObject");

    int sliceSize = 30.0;
    int wrapSize = 5.0;

    String curMatName;

    // create original texture geometry
    for( size_t i = 0 ; i < textureAtlasTable->size() ; i++ )
    {
        bool changeMat = (curMatName != (*textureAtlasTable)[i].atlasTextureName);

        if (changeMat)
        {
            if (curMatName.empty() == false) // we don't want to end before we begin
            {
                textureAtlasObject->end();
            }

            curMatName = (*textureAtlasTable)[i].originalTextureName;
            createMaterialForTexture(curMatName, false);
            textureAtlasObject->begin(curMatName, RenderOperation::OT_TRIANGLE_LIST);
        }

        // triangle 0
        textureAtlasObject->position(i * sliceSize, 0, 0); //Position
        textureAtlasObject->textureCoord(0,0); //UV

        textureAtlasObject->position(i * sliceSize, 0, sliceSize); //Position
        textureAtlasObject->textureCoord(0,wrapSize); //UV

        textureAtlasObject->position((i + 1) * sliceSize, 0 , sliceSize); //Position
        textureAtlasObject->textureCoord(wrapSize,wrapSize); //UV

        // triangle 1
        textureAtlasObject->position(i * sliceSize, 0, 0); //Position
        textureAtlasObject->textureCoord(0,0); //UV

        textureAtlasObject->position((i + 1) * sliceSize, 0, sliceSize); //Position
        textureAtlasObject->textureCoord(wrapSize,wrapSize); //UV

        textureAtlasObject->position((i + 1) * sliceSize, 0, 0); //Position
        textureAtlasObject->textureCoord(wrapSize, 0); //UV

    }

    // create texture atlas geometry
    for( size_t i = 0 ; i < (*textureAtlasTable).size() ; i++ )
    {
        bool changeMat = (curMatName != (*textureAtlasTable)[i].atlasTextureName);

        if (changeMat)
        {
            if (curMatName.empty() == false) // we don't want to end before we begin
            {
                textureAtlasObject->end();
            }

            curMatName = (*textureAtlasTable)[i].atlasTextureName;
            createMaterialForTexture(curMatName, true);
            textureAtlasObject->begin(curMatName, RenderOperation::OT_TRIANGLE_LIST);
        }

        // triangle 0
        textureAtlasObject->position(i * sliceSize, 0, sliceSize); //Position
        textureAtlasObject->textureCoord(0,0); //UV
        textureAtlasObject->textureCoord((*textureAtlasTable)[i].indexInAtlas); //Texture ID

        textureAtlasObject->position(i * sliceSize, 0, sliceSize * 2); //Position
        textureAtlasObject->textureCoord(0,wrapSize); //UV
        textureAtlasObject->textureCoord((*textureAtlasTable)[i].indexInAtlas); //Texture ID

        textureAtlasObject->position((i + 1) * sliceSize, 0 , sliceSize * 2); //Position
        textureAtlasObject->textureCoord(wrapSize,wrapSize); //UV
        textureAtlasObject->textureCoord((*textureAtlasTable)[i].indexInAtlas); //Texture ID

        // triangle 1
        textureAtlasObject->position(i * sliceSize, 0, sliceSize); //Position
        textureAtlasObject->textureCoord(0,0); //UV
        textureAtlasObject->textureCoord((*textureAtlasTable)[i].indexInAtlas); //Texture ID

        textureAtlasObject->position((i + 1) * sliceSize, 0, sliceSize * 2); //Position
        textureAtlasObject->textureCoord(wrapSize,wrapSize); //UV
        textureAtlasObject->textureCoord((*textureAtlasTable)[i].indexInAtlas); //Texture ID

        textureAtlasObject->position((i + 1) * sliceSize, 0, sliceSize); //Position
        textureAtlasObject->textureCoord(wrapSize, 0); //UV
        textureAtlasObject->textureCoord((*textureAtlasTable)[i].indexInAtlas); //Texture ID

    }

    textureAtlasObject->end();

    return textureAtlasObject;
}

