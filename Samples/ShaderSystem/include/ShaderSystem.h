#ifndef __ShaderSystem_H__
#define __ShaderSystem_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

class ShaderSystemListener;
class ShaderSystemApplication;

// Lighting models.
enum ShaderSystemLightingModel
{
	SSLM_PerVertexLighting,
	SSLM_PerPixelLighting,
	SSLM_NormalMapLightingTangentSpace,
	SSLM_NormalMapLightingObjectSpace,		
};

// Listener class for frame updates
class _OgreSampleClassExport Sample_ShaderSystem : public SdkSample
{
public:
	Sample_ShaderSystem();

	/** @see Sample::checkBoxToggled. */
	void checkBoxToggled(CheckBox* box);

	/** @see Sample::itemSelected. */
	void itemSelected(SelectMenu* menu);

	/** @see Sample::buttonHit. */
	virtual void buttonHit(Button* b);

	/** @see Sample::getRequiredPlugins. */
	StringVector getRequiredPlugins();

	/** @see Sample::testCapabilities. */
	void testCapabilities(const RenderSystemCapabilities* caps);
	
	/** @see Sample::frameRenderingQueued. */
    bool frameRenderingQueued(const FrameEvent& evt);
	
	
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
	bool touchPressed(const OIS::MultiTouchEvent& evt);

	bool touchReleased(const OIS::MultiTouchEvent& evt);

	bool touchMoved(const OIS::MultiTouchEvent& evt);
#else
	/** @see Sample::mousePressed. */
	bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id);

	/** @see Sample::mouseReleased. */
	bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id);

	/** @see Sample::mouseMoved. */
	bool mouseMoved(const OIS::MouseEvent& evt);
#endif

protected:

	/** Set the current lighting model. */
	void setCurrentLightingModel(ShaderSystemLightingModel lightingModel);

	/** Return the current lighting model. */
	ShaderSystemLightingModel getCurrentLightingMode() const { return mCurLightingModel; }

	/** Set specular enable state. */
	void setSpecularEnable(bool enable);

	/** Return current specular state. */
	bool getSpecularEnable() const { return mSpecularEnable; }

	/** Set reflection map enable state. */
	void setReflectionMapEnable(bool enable);

	/** Return current reflection map state. */
	bool getReflectionMapEnable() const { return mReflectionMapEnable; }

	/** Set fog per pixel enable state. */
	void setPerPixelFogEnable(bool enable);

	/** Create directional light. */
	void createDirectionalLight();

	/** Create point light. */
	void createPointLight();

	/** Create spot light. */
	void createSpotLight();

	/** Toggle light visibility. */
	void setLightVisible(const String& lightName, bool visible);

	/** Update runtime generated shaders of the target entities in this demo. */
	void updateSystemShaders();

	/** Export a given material including RTSS extended attributes.*/
	void exportRTShaderSystemMaterial(const String& fileName, const String& materialName);

	/** Create shaders based techniques using the given entity based on its sub entities material set. */
	void generateShaders(Entity* entity);

	/** @see Sample::setupView. */
	virtual void setupView();

	/** @see Sample::setupContent. */
	virtual void setupContent();

	/** Setup the UI for the sample. */
	void setupUI();
	
	/** @see Sample::setupContent. */
	virtual void cleanupContent();

	/** @see Sample::loadResources. */
	void loadResources();

	/** Create private resource group. */
	void createPrivateResourceGroup();
	
	/** @see Sample::unloadResources. */
	void unloadResources();

	/** Destroy private resource group. */
	void destroyPrivateResourceGroup();

// Types.
protected:
	typedef vector<Entity*>::type	EntityList;
	typedef EntityList::iterator	EntityListIterator;

	typedef map<String, bool>::type  StringMap;
	typedef StringMap::iterator		 StringMapIterator;

protected:
	EntityList							mTargetEntities;		// Target entities that will use runtime shader generated materials.	
	ShaderSystemLightingModel			mCurLightingModel;		// The current lighting model.
	SelectMenu*							mLightingModelMenu;		// The lighting model menu.
	SelectMenu*							mFogModeMenu;			// The fog mode menu.
	bool								mPerPixelFogEnable;		// When true the RTSS will do per pixel fog calculations.
	bool								mSpecularEnable;		// The current specular state.	
	RTShader::SubRenderStateFactory*	mReflectionMapFactory;	// The custom reflection map shader extension factory.
	bool								mReflectionMapEnable;	// The current reflection map effect state.
	SceneNode*							mPointLightNode;		// Point light scene node.
	SceneNode*							mDirectionalLightNode;	// Directional light scene node.	
	Label*								mMainEntityVS;			// The main entity vertex shader label.
	Label*								mMainEntityFS;			// The main entity fragment shader label.
};

#endif
