/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
OgrePCZPlugin.cpp  -  Portal Connected Zone Scene Manager Plugin class
-----------------------------------------------------------------------------
begin                : Mon Feb 19 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update    :
-----------------------------------------------------------------------------
*/

#include "OgrePCZPlugin.h"
#include "OgrePCZSceneManager.h"
#include "OgrePortal.h"
#include "OgreAntiPortal.h"
#include "OgrePCZLight.h"
#include "OgrePCZoneFactory.h"
#include "OgreRoot.h"

namespace Ogre 
{
    const String sPluginName = "Portal Connected Zone Scene Manager";
    //---------------------------------------------------------------------
    PCZPlugin::PCZPlugin()
        :mPCZSMFactory(0)
    {
    }
    //---------------------------------------------------------------------
    const String& PCZPlugin::getName() const
    {
        return sPluginName;
    }
    //---------------------------------------------------------------------
    void PCZPlugin::install()
    {
        // Create objects
        mPCZSMFactory = OGRE_NEW PCZSceneManagerFactory();
        mPCZoneFactoryManager = OGRE_NEW PCZoneFactoryManager();
        mPCZLightFactory = OGRE_NEW PCZLightFactory();
        mPortalFactory = OGRE_NEW PortalFactory();
        mAntiPortalFactory = OGRE_NEW AntiPortalFactory();
    }
    //---------------------------------------------------------------------
    void PCZPlugin::initialise()
    {
        // Register
        Root::getSingleton().addSceneManagerFactory(mPCZSMFactory);
        Root::getSingleton().addMovableObjectFactory(mPCZLightFactory);
        Root::getSingleton().addMovableObjectFactory(mPortalFactory);
        Root::getSingleton().addMovableObjectFactory(mAntiPortalFactory);

        // set type flags to static member variable for fast access.
        PortalFactory::FACTORY_TYPE_FLAG = mPortalFactory->getTypeFlags();
        AntiPortalFactory::FACTORY_TYPE_FLAG = mAntiPortalFactory->getTypeFlags();
    }
    //---------------------------------------------------------------------
    void PCZPlugin::shutdown()
    {
        // Unregister
        Root::getSingleton().removeSceneManagerFactory(mPCZSMFactory);
        Root::getSingleton().removeMovableObjectFactory(mPCZLightFactory);
        Root::getSingleton().removeMovableObjectFactory(mPortalFactory);
        Root::getSingleton().removeMovableObjectFactory(mAntiPortalFactory);
    }
    //---------------------------------------------------------------------
    void PCZPlugin::uninstall()
    {
        // destroy 
        OGRE_DELETE mPCZSMFactory;
        mPCZSMFactory = 0;
        OGRE_DELETE mPCZoneFactoryManager;
        mPCZoneFactoryManager = 0;
        OGRE_DELETE mPCZLightFactory;
        mPCZLightFactory = 0;
        OGRE_DELETE mPortalFactory;
        mPortalFactory = 0;
        OGRE_DELETE mAntiPortalFactory;
        mAntiPortalFactory = 0;
    }


}
