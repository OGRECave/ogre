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

#include "SSAOLogic.h"

#include "Ogre.h"
using namespace Ogre;

class ssaoListener: public Ogre::CompositorInstance::Listener
{
public:
    
    ssaoListener(Ogre::CompositorInstance* instance) : mInstance(instance) {}
   
    // this callback we will use to modify SSAO parameters
    void notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
    {
        if (pass_id != 42) // not SSAO, return
            return;

        // this is the camera you're using
        Ogre::Camera *cam = mInstance->getChain()->getViewport()->getCamera();

        // get the pass
        Ogre::Pass *pass = mat->getBestTechnique()->getPass(0);

        // get the fragment shader parameters
        auto params = pass->getFragmentProgramParameters();
        // set the projection matrix we need
        params->setNamedConstant("ptMat", Matrix4::CLIPSPACE2DTOIMAGESPACE * cam->getProjectionMatrixWithRSDepth());
    }
private:
    Ogre::CompositorInstance* mInstance;
};

Ogre::CompositorInstance::Listener* SSAOLogic::createListener(Ogre::CompositorInstance* instance)
{
    return new ssaoListener(instance);
}
