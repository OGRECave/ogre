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

#ifndef __SHAREDDATA_H
#define __SHAREDDATA_H

#include "Ogre.h"
#include "DeferredShading.h"

class SharedData : public Ogre::Singleton<SharedData> {

public:

        SharedData();
        ~SharedData();

        /// @copydoc Singleton::getSingleton()
        static SharedData& getSingleton(void);
        /// @copydoc Singleton::getSingleton()
        static SharedData* getSingletonPtr(void);

        // shared data across the application
        Ogre::Root *iRoot;
        Ogre::Camera *iCamera;
        Ogre::RenderWindow *iWindow;

        DeferredShadingSystem *iSystem;
        bool iActivate;
        bool iGlobalActivate;

        Ogre::Light *iMainLight;

        std::vector<Ogre::Node*> mLightNodes;

};

#endif
