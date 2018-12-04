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

#include "VolumeTex.h"

#include "OgreTexture.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreTextureManager.h"
#include "OgreLogManager.h"
#include <sstream>

#include "VolumeRenderable.h"
#include "ThingRenderable.h"
#include "Julia.h"

namespace {
SimpleRenderable *vrend;
SimpleRenderable *trend;
SceneNode *snode,*fnode;
}

void Sample_VolumeTex::setupContent()
{
    if (!ResourceGroupManager::getSingleton().resourceGroupExists("VolumeRenderable"))
    {
        ResourceGroupManager::getSingleton().createResourceGroup("VolumeRenderable");
    }
    // Create dynamic texture
    ptex = TextureManager::getSingleton().createManual(
                "DynaTex","VolumeRenderable", TEX_TYPE_3D, 64, 64, 64, 0, PF_BYTE_RGBA);

    // Set ambient light
    mSceneMgr->setAmbientLight(ColourValue(0.6, 0.6, 0.6));
    mSceneMgr->setSkyBox(true, "Examples/MorningSkyBox", 50 );

    //mRoot->getRenderSystem()->clearFrameBuffer(FBT_COLOUR, ColourValue(255,255,255,0));

    // Create a light
    Light* l = mSceneMgr->createLight("MainLight");
    l->setDiffuseColour(0.75, 0.75, 0.80);
    l->setSpecularColour(0.9, 0.9, 1);
    mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-100,80,50))->attachObject(l);

    // Create volume renderable
    snode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0,0,0));

    vrend = new VolumeRenderable(32, 750.0f, "DynaTex");
    snode->attachObject( vrend );

    trend = new ThingRenderable(90.0f, 32, 7.5f);
    MaterialPtr mat = MaterialManager::getSingleton().getByName("Examples/VTDarkStuff", "General");
    trend->setMaterial(mat);
    snode->attachObject(trend);

    // Ogre head node
    fnode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0,0,0));
    // Load ogre head
    Entity* head = mSceneMgr->createEntity("head", "ogrehead.mesh");
    fnode->attachObject(head);

    // Animation for ogre head
    // Create a track for the light
    Animation* anim = mSceneMgr->createAnimation("OgreTrack", 10);
    // Spline it for nice curves
    anim->setInterpolationMode(Animation::IM_SPLINE);
    // Create a track to animate the camera's node
    NodeAnimationTrack* track = anim->createNodeTrack(0, fnode);
    // Setup keyframes
    TransformKeyFrame* key = track->createNodeKeyFrame(0); // A startposition
    key->setTranslate(Vector3(0.0f, -15.0f, 0.0f));
    key = track->createNodeKeyFrame(5);//B
    key->setTranslate(Vector3(0.0f, 15.0f, 0.0f));
    key = track->createNodeKeyFrame(10);//C
    key->setTranslate(Vector3(0.0f, -15.0f, 0.0f));
    // Create a new animation state to track this
    auto animState = mSceneMgr->createAnimationState("OgreTrack");
    animState->setEnabled(true);
    auto& controllerMgr = ControllerManager::getSingleton();
    controllerMgr.createFrameTimePassthroughController(AnimationStateControllerValue::create(animState, true));

    //mFountainNode->attachObject(pSys2);

    //Setup defaults
    global_real = 0.4f;
    global_imag = 0.6f;
    global_theta = 0.0f;

    // show GUI
    createControls();

    mCameraMan->setStyle(CS_ORBIT);

    generate();
}

bool Sample_VolumeTex::frameRenderingQueued(const FrameEvent &evt)
{
    //snode->roll(Degree(evt.timeSinceLastFrame * 20.0f));
    //fnode->roll(Degree(evt.timeSinceLastFrame * 20.0f));
    static_cast<ThingRenderable*>(trend)->addTime(evt.timeSinceLastFrame * 0.05f);
    return SdkSample::frameRenderingQueued(evt);
}

void Sample_VolumeTex::cleanupContent()
{
    TextureManager::getSingleton().remove("DynaTex", "VolumeRenderable");
    delete vrend;
    delete trend;
}

void Sample_VolumeTex::generate()
{
    /* Evaluate julia fractal for each point */
    Julia julia(global_real, global_imag, global_theta);
    const float scale = 2.5;
    const float vcut = 29.0f;
    const float vscale = 1.0f/vcut;

    HardwarePixelBufferSharedPtr buffer = ptex->getBuffer(0, 0);
    Ogre::StringStream d;
    d << "HardwarePixelBuffer " << buffer->getWidth() << " " << buffer->getHeight() << " " << buffer->getDepth();
    LogManager::getSingleton().logMessage(d.str());

    buffer->lock(HardwareBuffer::HBL_NORMAL);
    const PixelBox &pb = buffer->getCurrentLock();
    d.str("");
    d << "PixelBox " << pb.getWidth() << " " << pb.getHeight() << " " << pb.getDepth() << " " << pb.rowPitch << " " << pb.slicePitch << " " << pb.data << " " << PixelUtil::getFormatName(pb.format);
    LogManager::getSingleton().logMessage(d.str());

    Ogre::uint32 *pbptr = reinterpret_cast<Ogre::uint32*>(pb.data);
    for(size_t z=pb.front; z<pb.back; z++)
    {
        for(size_t y=pb.top; y<pb.bottom; y++)
        {
            for(size_t x=pb.left; x<pb.right; x++)
            {
                if(z==pb.front || z==(pb.back-1) || y==pb.top|| y==(pb.bottom-1) ||
                        x==pb.left || x==(pb.right-1))
                {
                    // On border, must be zero
                    pbptr[x] = 0;
                }
                else
                {
                    float val = julia.eval(((float)x/pb.getWidth()-0.5f) * scale,
                                           ((float)y/pb.getHeight()-0.5f) * scale,
                                           ((float)z/pb.getDepth()-0.5f) * scale);
                    if(val > vcut)
                        val = vcut;

                    PixelUtil::packColour((float)x/pb.getWidth(), (float)y/pb.getHeight(), (float)z/pb.getDepth(), (1.0f-(val*vscale))*0.7f, PF_BYTE_RGBA, &pbptr[x]);

                }
            }
            pbptr += pb.rowPitch;
        }
        pbptr += pb.getSliceSkip();
    }
    buffer->unlock();
}
