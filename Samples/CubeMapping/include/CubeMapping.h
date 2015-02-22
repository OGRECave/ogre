#ifndef __CubeMapping_H__
#define __CubeMapping_H__

#include "SdkSample.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorWorkspaceDef.h"
#include "Compositor/OgreCompositorWorkspaceListener.h"
#include "Compositor/OgreCompositorChannel.h"

using namespace Ogre;
using namespace OgreBites;

const uint32 NonRefractiveSurfaces  = 0x00000001;
const uint32 RefractiveSurfaces     = 0x00000002; //Not used in this demo
const uint32 ReflectedSurfaces      = 0x00000004;
const uint32 RegularSurfaces        = NonRefractiveSurfaces|ReflectedSurfaces;

class _OgreSampleClassExport Sample_CubeMapping : public SdkSample, public CompositorWorkspaceListener
{
public:

    Sample_CubeMapping()
    {
        mInfo["Title"] = "Cube Mapping";
        mInfo["Description"] = "Demonstrates how to setup cube mapping with the compositor.\n"
            "The textures are manually created, one Camera & one Workspace for each cubemap is "
            "needed (i.e. useful for IBL probes - IBL = Image Based Lighting).\n\n"
            "The compositor automatically rotates the camera for each cube map face. This "
            "can be disabled setting camera_cubemap_reorient to false if the user wants to do it "
            "manually with a listener.\n\n"
            "Like in the Fresnel demo, visibility masks are used to prevent certain objects from "
            "being rendered by the reflection passes"
            "This sample creates a workspace using templates which can be located in the script "
            "'Cubemapping.compositor' to ease the setup.\n\n"
            "To understand how to fully create a Workspace definition without using scripts, refer to "
            "the Compositor demo.";
        mInfo["Thumbnail"] = "thumb_cubemap.png";
        mInfo["Category"] = "API Usage";

        MovableObject::setDefaultVisibilityFlags( RegularSurfaces );
    }

    void testCapabilities(const RenderSystemCapabilities* caps)
    {
        if (!caps->hasCapability(RSC_CUBEMAPPING))
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your graphics card does not support cube mapping, "
                "so you cannot run this sample. Sorry!", "CubeMappingSample::testCapabilities");
        }
    }

    bool frameRenderingQueued(const FrameEvent& evt)
    {
        mPivot->yaw(Radian(evt.timeSinceLastFrame));      // spin the fishy around the cube mapped one
        mFishSwim->addTime(evt.timeSinceLastFrame * 3);   // make the fishy swim
        return SdkSample::frameRenderingQueued(evt);      // don't forget the parent updates!
    }

    virtual void workspacePreUpdate(void)
    {
        /** CompositorWorkspaceListener::workspacePreUpdate is the best place to update other (manual)
            Workspaces for multiple reasons:
                1. It happens after Ogre issued D3D9's beginScene. If you want to update a workspace
                    outside beginScene/endScene pair, you will have to call Workspace::_beginUpdate(true)
                   and _endUpdate(true) yourself. This will add synchronization overhead in the API,
                   lowering performance.
                2. It happens before the whole scene is rendered, thus you can ensure your RTTs are
                   up to date.

            One alternative that allows you to forget about this listener is to use auto-updated
            workspaces, but you will have to ensure this workspace is created before your main
            workspace (the one that outputs to the RenderWindow).

            Another alternative is the one presented in the Fresnel demo: The rendering is fully
            handled inside one single workspace, and the textures are created by the Compositor
            instead of being manually created.
        */
        //mCubemapWorkspace->_beginUpdate( forceFrameBeginEnd );
        mCubemapWorkspace->_update();
        //mCubemapWorkspace->_endUpdate( forceFrameBeginEnd );
    }

protected:

    void setupContent()
    {
        mPreviousVisibilityFlags = MovableObject::getDefaultVisibilityFlags();
        MovableObject::setDefaultVisibilityFlags( RegularSurfaces );

        mWorkspace->setListener( this );

        mSceneMgr->setSkyDome(true, "Examples/CloudySky");

        // setup some basic lighting for our scene
        mSceneMgr->setAmbientLight(ColourValue(0.3, 0.3, 0.3));
        SceneNode *lightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        lightNode->setPosition(20, 80, 50);
        lightNode->attachObject( mSceneMgr->createLight() );

        createCubeMap();

        // create an ogre head, give it the dynamic cube map material, and place it at the origin
        mHead = mSceneMgr->createEntity("ogrehead.mesh",
                                         ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                                         SCENE_STATIC);
        mHead->setName("CubeMappedHead");
        mHead->setMaterialName("Examples/DynamicCubeMap");
        mHead->setVisibilityFlags( NonRefractiveSurfaces );
        mSceneMgr->getRootSceneNode(SCENE_STATIC)->attachObject(mHead);

        mPivot = mSceneMgr->getRootSceneNode()->createChildSceneNode();  // create a pivot node

        Entity* fish = mSceneMgr->createEntity("fish.mesh");
        fish->setName("Fish");
        mFishSwim = fish->getAnimationState("swim");
        mFishSwim->setEnabled(true);

        // create a child node at an offset and attach a regular ogre head and a nimbus to it
        SceneNode* node = mPivot->createChildSceneNode();
        node->setPosition(-60, 10, 0);
        node->setScale(7, 7, 7);
        node->yaw(Degree(90));
        node->attachObject(fish);

        // create a floor mesh resource
        MeshManager::getSingleton().createPlane("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Plane(Vector3::UNIT_Y, -30), 1000, 1000, 10, 10, true, 1, 8, 8, Vector3::UNIT_Z);

        // create a floor entity, give it a material, and place it at the origin
        Entity* floor = mSceneMgr->createEntity("floor",
                                                ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                                                SCENE_STATIC);
        floor->setName("Floor");
        floor->setMaterialName("Examples/BumpyMetal");
        mSceneMgr->getRootSceneNode(SCENE_STATIC)->attachObject(floor);

        // set our camera to orbit around the head and show cursor
        mCameraMan->setStyle(CS_ORBIT);
        mTrayMgr->showCursor();
    }

    void createCubeMap()
    {
        // create the camera used to render to our cubemap
        mCubeCamera = mSceneMgr->createCamera("CubeMapCamera", true, true);
        mCubeCamera->setFOVy(Degree(90));
        mCubeCamera->setAspectRatio(1);
        mCubeCamera->setFixedYawAxis(false);
        mCubeCamera->setNearClipDistance(5);

        //The default far clip distance is way too big for a cubemap-capable camara, which prevents
        //Ogre from better culling and prioritizing lights in a forward renderer.
        //TODO: Improve the Sky algorithm so that we don't need to use this absurd high number
        mCubeCamera->setFarClipDistance( /*100*/10000 );

        // create our dynamic cube map texture
        TexturePtr tex = TextureManager::getSingleton().createManual("dyncubemap",
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_CUBE_MAP, 512, 512, 0, PF_R8G8B8, TU_RENDERTARGET);

        CompositorManager2 *compositorManager = mRoot->getCompositorManager2();

        const Ogre::IdString workspaceName( "CompositorSampleCubemap_cubemap" );
        if( !compositorManager->hasWorkspaceDefinition( workspaceName ) )
        {
            CompositorWorkspaceDef *workspaceDef = compositorManager->addWorkspaceDefinition(
                                                                                    workspaceName );
            //"CubemapRendererNode" has been defined in scripts.
            //Very handy (as it 99% the same for everything)
            workspaceDef->connectOutput( "CubemapRendererNode", 0 );
        }

        CompositorChannel channel;
        channel.target = tex->getBuffer(0)->getRenderTarget(); //Any of the render targets will do
        channel.textures.push_back( tex );
        mCubemapWorkspace = compositorManager->addWorkspace( mSceneMgr, channel, mCubeCamera,
                                                             workspaceName, false );
    }

    void cleanupContent()
    {
		mSceneMgr->destroyCamera(mCubeCamera);
        MeshManager::getSingleton().remove("floor");
        TextureManager::getSingleton().remove("dyncubemap");

        //Restore global settings
        MovableObject::setDefaultVisibilityFlags( mPreviousVisibilityFlags );
    }

    Entity* mHead;
    Camera* mCubeCamera;
    RenderTarget* mTargets[6];
    SceneNode* mPivot;
    AnimationState* mFishSwim;

    CompositorWorkspace *mCubemapWorkspace;
    uint32 mPreviousVisibilityFlags;
};

#endif
