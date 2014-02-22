/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
conditions of the standard open source license.
-----------------------------------------------------------------------------
*/

#ifndef _HelperLogics_H__
#define _HelperLogics_H__

#include "Compositor/OgreCompositorWorkspaceListener.h"

//Demo note :
//This file contains two listeners for the Heat Vision and Gaussian Blur postprocessing FXs.
//If you wish to use these compositor nodes in your application, make sure to add this code as well.

class SamplePostprocessWorkspaceListener : public Ogre::CompositorWorkspaceListener
{
    //Heat vision's
    float start, end, curr;
    Ogre::Timer *timer;

    int mVpWidth, mVpHeight;
    int mBloomSize;
    // Array params - have to pack in groups of 4 since this is how Cg generates them
    // also prevents dependent texture read problems if ops don't require swizzle
    float mBloomTexWeights[15][4];
    float mBloomTexOffsetsHorz[15][4];
    float mBloomTexOffsetsVert[15][4];

    void onHeatVision( Ogre::CompositorPass *pass );
    void onGaussianBlurV( Ogre::CompositorPass *pass );
    void onGaussianBlurH( Ogre::CompositorPass *pass );
public:
    SamplePostprocessWorkspaceListener();
    ~SamplePostprocessWorkspaceListener();

    //Called when each pass is about to be executed.
    virtual void passPreExecute( Ogre::CompositorPass *pass );

    void windowResized( unsigned int width, unsigned int height );
};

#endif
