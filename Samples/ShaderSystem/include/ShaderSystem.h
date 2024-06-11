#ifndef __ShaderSystem_H__
#define __ShaderSystem_H__

#include "SdkSample.h"
#include "OgreShaderExLayeredBlending.h"

using namespace Ogre;
using namespace OgreBites;

// Lighting models.
enum ShaderSystemLightingModel
{
    SSLM_PerPixelLighting,
    SSLM_CookTorranceLighting,
    SSLM_ImageBasedLighting,
    SSLM_NormalMapLightingTangentSpace,
    SSLM_NormalMapLightingObjectSpace
};

// a hack class to get infinite frustum - needed by instanced viewports demo
// a better solution will be to check the frustums of all the viewports in a similer class
class _OgreSampleClassExport InfiniteFrustum : public Frustum
{
public:
    InfiniteFrustum() : Frustum()
    {
        mFrustumPlanes[FRUSTUM_PLANE_LEFT].normal = Vector3::NEGATIVE_UNIT_X;
        mFrustumPlanes[FRUSTUM_PLANE_LEFT].d = 9999999999999999999.0f;
        mFrustumPlanes[FRUSTUM_PLANE_RIGHT].normal = Vector3::UNIT_X;
        mFrustumPlanes[FRUSTUM_PLANE_RIGHT].d = 9999999999999999999.0f;
        mFrustumPlanes[FRUSTUM_PLANE_TOP].normal = Vector3::NEGATIVE_UNIT_Y;
        mFrustumPlanes[FRUSTUM_PLANE_TOP].d = 9999999999999999999.0f;
        mFrustumPlanes[FRUSTUM_PLANE_BOTTOM].normal = Vector3::UNIT_Y;
        mFrustumPlanes[FRUSTUM_PLANE_BOTTOM].d = 9999999999999999999.0f;
        mFrustumPlanes[FRUSTUM_PLANE_NEAR].normal = Vector3::NEGATIVE_UNIT_Z;
        mFrustumPlanes[FRUSTUM_PLANE_NEAR].d = 9999999999999999999.0f;
        mFrustumPlanes[FRUSTUM_PLANE_FAR].normal = Vector3::UNIT_Z;
        mFrustumPlanes[FRUSTUM_PLANE_FAR].d = 9999999999999999999.0f;
    }
    bool isVisible(const AxisAlignedBox& bound, FrustumPlane* culledBy = 0) const override {return true;};
    bool isVisible(const Sphere& bound, FrustumPlane* culledBy = 0) const override {return true;};
    bool isVisible(const Vector3& vert, FrustumPlane* culledBy = 0) const override {return true;};
    bool projectSphere(const Sphere& sphere, 
        Real* left, Real* top, Real* right, Real* bottom) const override {*left = *bottom = -1.0f; *right = *top = 1.0f; return true;};
    float getNearClipDistance(void) const override {return 1.0;};
    float getFarClipDistance(void) const override {return 9999999999999.0f;};
    const Plane& getFrustumPlane( unsigned short plane ) const override
    {
        return mFrustumPlanes[plane];
    }

};


// Listener class for frame updates
class _OgreSampleClassExport Sample_ShaderSystem : public SdkSample
{
public:
    Sample_ShaderSystem();
    ~Sample_ShaderSystem();
        
    void _shutdown() override;

    /** @see Sample::checkBoxToggled. */
    void checkBoxToggled(CheckBox* box) override;

    /** @see Sample::itemSelected. */
    void itemSelected(SelectMenu* menu) override;

    /** @see Sample::buttonHit. */
    void buttonHit(OgreBites::Button* b) override;

    /** @see Sample::sliderMoved. */
    void sliderMoved(Slider* slider) override;

    /** @see Sample::testCapabilities. */
    void testCapabilities(const RenderSystemCapabilities* caps) override;
    
    /** @see Sample::frameRenderingQueued. */
    bool frameRenderingQueued(const FrameEvent& evt) override;

    void updateTargetObjInfo();

    /** @see Sample::mousePressed. */
    bool mousePressed(const MouseButtonEvent& evt) override;

    /** @see Sample::mouseReleased. */
    bool mouseReleased(const MouseButtonEvent& evt) override;

    /** @see Sample::mouseMoved. */
    bool mouseMoved(const MouseMotionEvent& evt) override;

protected:

    /** Set the current lighting model. */
    void setCurrentLightingModel(ShaderSystemLightingModel lightingModel);

    /** Return the current lighting model. */
    ShaderSystemLightingModel getCurrentLightingMode() const { return mCurLightingModel; }

    /** Set specular enable state. */
    void setSpecularEnable(bool enable);

    /** Return current specular state. */
    bool getSpecularEnable() const { return mSpecularEnable; }


    /** Set fog per pixel enable state. */
    void setPerPixelFogEnable(bool enable);

    /** Set auto border adjustment mode in texture atlasing. */
    void setAtlasBorderMode( bool enable );

    /** Set instanced viewports enable state. */
    void setInstancedViewportsEnable( bool enable );

    /** Create directional light. */
    void createDirectionalLight();

    /** Create point light. */
    void createPointLight();

    /** Create spot light. */
    void createSpotLight();

    /** Toggle adding of lots of models */
    void updateAddLotsOfModels(bool addThem);
    void addModelToScene(const String &  modelName);

    /** Toggle instanced viewports */
    void updateInstancedViewports(bool enabled);

    /** Toggle light visibility. */
    void updateLightState(const String& lightName, bool visible);

    /** Update runtime generated shaders of the target entities in this demo. */
    void updateSystemShaders();

    /** Create shaders based techniques using the given entity based on its sub entities material set. */
    void generateShaders(Entity* entity);

    /** @see Sample::setupView. */
//  virtual void setupView();

    /** @see Sample::setupContent. */
    void setupContent() override;

    /** Setup the UI for the sample. */
    void setupUI();
    
    /** @see Sample::setupContent. */
    void cleanupContent() override;
    
    /** @see Sample::unloadResources. */
    void unloadResources() override;

    void createInstancedViewports();
    void destroyInstancedViewports();
    void destroyInstancedViewportsFactory();

    /** Pick the target object. */
    void pickTargetObject( const MouseButtonEvent &evt );

    /** Apply shadow type from the given shadow menu selected index. */
    void applyShadowType(int menuIndex);

    /** Change the current texture layer blend mode. */
    void changeTextureLayerBlendMode();

    ManualObject* createTextureAtlasObject();
    void createMaterialForTexture( const String & texName, bool isTextureAtlasTexture );
    // Types.
protected:
    typedef std::vector<Entity*>   EntityList;
    typedef EntityList::iterator    EntityListIterator;

    typedef std::map<String, bool>  StringMap;
    typedef StringMap::iterator      StringMapIterator;

protected:
    EntityList                          mTargetEntities;        // Target entities that will use runtime shader generated materials.    
    ShaderSystemLightingModel           mCurLightingModel;      // The current lighting model.
    SelectMenu*                         mLightingModelMenu;     // The lighting model menu.
    SelectMenu*                         mFogModeMenu;           // The fog mode menu.
    Label*                              mLanguage;          // The shading language menu.
    SelectMenu*                         mShadowMenu;            // The shadow type menu.
    bool                                mPerPixelFogEnable;     // When true the RTSS will do per pixel fog calculations.
    bool                                mSpecularEnable;        // The current specular state.  
    RTShader::SubRenderStateFactory*    mTextureAtlasFactory;
    RTShader::SubRenderState*           mInstancedViewportsSubRenderState;// todo - doc
    bool                                mInstancedViewportsEnable;      // todo - doc
    InfiniteFrustum                     mInfiniteFrustum;               // todo - doc
    BillboardSet*                       mBbsFlare;                      // todo - doc
    bool                                mAddedLotsOfModels;             // todo - doc
    std::vector<Entity *>              mLotsOfModelsEntities;          // todo - doc       
    std::vector<SceneNode *>           mLotsOfModelsNodes;             // todo - doc  
    int                                 mNumberOfModelsAdded;           // todo - doc   
    RTShader::SubRenderStateFactory *   mInstancedViewportsFactory;     // todo - doc

    RTShader::SubRenderState*           mReflectionMapSubRS;    // The reflection map sub render state.
    RTShader::LayeredBlending*          mLayerBlendSubRS;       // The layer blending sub render state.
    Label*                              mLayerBlendLabel;       // The layer blending label.
    Slider*                             mReflectionPowerSlider; // The reflection power controller slider.
    Slider*                             mModifierValueSlider;   // The value of the modifier for the layered blend controller slider.
    Entity*                             mLayeredBlendingEntity; // Entity used to show layered blending SRS
    SceneNode*                          mPointLightNode;        // Point light scene node.
    SceneNode*                          mDirectionalLightNode;  // Directional light scene node.        
    RaySceneQuery*                      mRayQuery;              // The ray scene query.
    MovableObject*                      mTargetObj;             // The current picked target object.
    Label*                              mTargetObjMatName;      // The target object material name label.
    Label*                              mTargetObjVS;           // The target object vertex shader label.
    Label*                              mTargetObjFS;           // The target object fragment shader label.
    CheckBox*                           mDirLightCheckBox;      // The directional light check box.
    CheckBox*                           mPointLightCheckBox;    // The point light check box.
    CheckBox*                           mSpotLightCheckBox;     // The spot light check box.
    CheckBox*                           mInstancedViewportsCheckBox; // The instanced viewports check box.
    CheckBox*                           mAddLotsOfModels; // The "add lots of models" check box.
    int                                 mCurrentBlendMode;
};

#endif
