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
    @file 
        Shadows.cpp
    @brief
        Shows a few ways to use Ogre's shadowing techniques
*/

#include "SdkSample.h"
#include "OgreBillboard.h"
#include "OgreMovablePlane.h"
#include "OgrePredefinedControllers.h"

class _OgreSampleClassExport Sample_ShadowsV2 : public OgreBites::SdkSample
{
protected:
    Ogre::vector<Ogre::Entity*>::type   mCasters;
    Ogre::Entity                        *mFloorPlane;
    Ogre::Light                         *mMainLight;
    Ogre::vector<Ogre::Light*>::type    mLights;
    Ogre::Real                          mMinFlareSize;
    Ogre::Real                          mMaxFlareSize;
    bool                                mPssm;  /// Whether to enable Parallel Split Shadow Mapping

public:
    Sample_ShadowsV2();

protected:

    virtual Ogre::CompositorWorkspace* setupCompositor(void);

    // Just override the mandatory create scene method
    virtual void setupContent(void);

    void createDebugOverlays(void);
};
