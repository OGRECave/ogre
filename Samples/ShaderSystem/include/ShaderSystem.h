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

	/** @see Sample::getRequiredPlugins. */
	StringVector getRequiredPlugins();

	/** @see Sample::testCapabilities. */
	void testCapabilities(const RenderSystemCapabilities* caps);
	
	/** @see Sample::frameRenderingQueued. */
    bool frameRenderingQueued(const FrameEvent& evt);
	
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

	/** Export a given material including shader generator materials.*/
	void exportShaderMaterial(const String& fileName, const String& materialName);

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

	/** Create shaders based techniques using the given entity based on its sub entities material set. */
	void generateShaders(Entity* entity);

	/** @see Sample::setupView. */
	virtual void setupView();

	/** @see Sample::setupContent. */
	virtual void setupContent();

	/** @see Sample::setupContent. */
	virtual void cleanupContent();

	typedef vector<Entity*>::type	EntityList;
	typedef EntityList::iterator	EntityListIterator;

	typedef map<String, bool>::type  StringMap;
	typedef StringMap::iterator		 StringMapIterator;

protected:
	EntityList							mTargetEntities;		// Target entities that will use runtime shader generated materials.	
	ShaderSystemLightingModel			mCurLightingModel;		// The current lighting model.
	SelectMenu*							mLightingModelMenu;		// The lighting model menu.
	bool								mSpecularEnable;		// The current specular state.	
	RTShader::SubRenderStateFactory*	mReflectionMapFactory;	// The custom reflection map shader extension factory.
	bool								mReflectionMapEnable;	// The current reflection map effect state.
	SceneNode*							mPointLightNode;		// Point light scene node.
	SceneNode*							mDirectionalLightNode;	// Directional light scene node.
};

#endif
