/**
Implementation of a Deferred Shading engine in OGRE, using Multiple Render Targets and
HLSL/GLSL high level language shaders.
	// W.J. :wumpus: van der Laan 2005 //

Deferred shading renders the scene to a 'fat' texture format, using a shader that outputs colour, 
normal, depth, and possible other attributes per fragment. Multi Render Target is required as we 
are dealing with many outputs which get written into multiple render textures in the same pass.

After rendering the scene in this format, the shading (lighting) can be done as a post process. 
This means that lighting is done in screen space. Adding them requires nothing more than rendering 
a screenful quad; thus the method allows for an enormous amount of lights without noticeable 
performance loss.

Little lights affecting small area ("Minilights") can be even further optimised by rendering 
their convex bounding geometry. This is also shown in this demo by 6 swarming lights.

The paper for GDC2004 on Deferred Shading can be found here:
  http://www.talula.demon.co.uk/DeferredShading.pdf
*******************************************************************************
Copyright (c) W.J. van der Laan

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software  and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to use, 
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so, subject 
to the following conditions:

The above copyright notice and this permission notice shall be included in all copies 
or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION 
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*******************************************************************************
*/
#ifndef H_WJ_DeferredShadingSystem
#define H_WJ_DeferredShadingSystem

#include "OgreCompositorInstance.h"
#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include "OgreMaterial.h"
#include "OgreRenderTargetListener.h"

class MLight;
class AmbientLight;
class MaterialGenerator;

/** System to manage Deferred Shading for a camera/render target.
 */
class DeferredShadingSystem : public Ogre::RenderTargetListener
{
public:
	DeferredShadingSystem(Ogre::Viewport *vp, Ogre::SceneManager *sm, Ogre::Camera *cam);
	~DeferredShadingSystem();

	enum DSMode
	{
		DSM_SHOWLIT = 0,     // The deferred shading mode
		DSM_SHOWCOLOUR = 1,  // Show diffuse (for debugging)
		DSM_SHOWNORMALS = 2, // Show normals (for debugging)
		DSM_SHOWDSP = 3,	 // Show depth and specular channel (for debugging)
		DSM_COUNT = 4
	};

	/** Set rendering mode (one of DSMode)
	 */
	void setMode(DSMode mode);

	DSMode getMode(void) const;

	/** Activate or deactivate system
	 */
	void setActive(bool active);

	/** Create a new MiniLight 
	 */
	MLight *createMLight();

	/** Destroy a MiniLight
	 */
	void destroyMLight(MLight *m);

	/// Visibility mask for scene
	static const Ogre::uint32 SceneVisibilityMask = 0x00000001;
	/// Visibility mask for post-processing geometry (lights, unlit particles)
	static const Ogre::uint32 PostVisibilityMask = 0x00000002;

	// Render Target Listener overrides
	virtual void preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);
protected:
	Ogre::Viewport *mViewport;
	Ogre::SceneManager *mSceneMgr;
	Ogre::Camera *mCamera;
	
	// Filters
	Ogre::CompositorInstance *mInstance[DSM_COUNT];
	// Active/inactive
	bool mActive;
	DSMode mCurrentMode;

	typedef Ogre::set<MLight*>::type LightList;

	LightList mLights;

	bool mLightMaterialsDirty;
	LightList mDirtyLightList;

	MaterialGenerator *mLightMaterialGenerator;

	void createAmbientLight(void);
	void setUpAmbientLightMaterial(void);
	AmbientLight* mAmbientLight;

    void createResources();
	void initialiseLightGeometry();

	// iterates through all the lights and sets up their materials

	// when you enable the compositor, if the compositor is the lit mode, we have to set up the light materials 
	// to that of the mrt
	void setupLightMaterials(void);

	// sets up the materials' pass' texture units 0 and 1 to texName0 and texName1
	void setupMaterial(const Ogre::MaterialPtr &mat
		, const Ogre::String& texName0
		, const Ogre::String& texName1);

	void logCurrentMode(void);
};

#endif
