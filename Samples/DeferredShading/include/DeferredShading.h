/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/

/**
Implementation of a Deferred Shading engine in OGRE, using Multiple Render Targets and
CG high level language shaders.
    // W.J. :wumpus: van der Laan 2005 / Noam Gat 2009 //

Deferred shading renders the scene to a 'fat' texture format, using a shader that outputs colour, 
normal, depth, and possible other attributes per fragment. Multi Render Target is required as we 
are dealing with many outputs which get written into multiple render textures in the same pass.

After rendering the scene in this format, the shading (lighting) can be done as a post process. 
This means that lighting is done in screen space, using light-representing geometry (sphere for
point light, cone for spot light and quad for directional) to render their contribution.

The wiki article explaining this demo can be found here :
  http://www.ogre3d.org/wiki/index.php/Deferred_Shading
*/

#ifndef H_DeferredShadingSystem
#define H_DeferredShadingSystem

#include "SdkSample.h"

#include "OgreCompositorInstance.h"
#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include "OgreMaterial.h"
#include "OgreRenderTargetListener.h"
#include "GeomUtils.h"

using namespace Ogre;
using namespace OgreBites;

/** System to manage Deferred Shading for a camera/render target.
 *  @note With the changes to the compositor framework, this class just
 *      selects which compositors to enable.
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
        DSM_SHOWDSP = 3,     // Show depth and specular channel (for debugging)
        DSM_COUNT = 4
    };
    
    //The first render queue that does not get rendered into the GBuffer
    //place transparent (or other non gbuffer) objects after this one
    static const Ogre::uint8 POST_GBUFFER_RENDER_QUEUE;

    void initialize();

    /** Set rendering mode (one of DSMode)
     */
    void setMode(DSMode mode);

    DSMode getMode(void) const;

    /** Set screen space ambient occlusion mode
     */
    void setSSAO(bool ssao);
    
    bool getSSAO() const;

    /** Activate or deactivate system
     */
    void setActive(bool active);
    
protected:
    Ogre::Viewport *mViewport;
    Ogre::SceneManager *mSceneMgr;
    Ogre::Camera *mCamera;
    
    Ogre::CompositorInstance *mGBufferInstance;
    // Filters
    Ogre::CompositorInstance *mInstance[DSM_COUNT];
    Ogre::CompositorInstance* mSSAOInstance;
    // Active/inactive
    bool mActive;
    DSMode mCurrentMode;
    bool mSSAO;

    //Used to unregister compositor logics and free memory
    typedef std::map<String, CompositorLogic*> CompositorLogicMap;
    CompositorLogicMap mCompositorLogics;

    void createResources();
    
    void logCurrentMode(void);
};

#endif
