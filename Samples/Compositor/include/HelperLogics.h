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

#ifndef __HelperLogics_H__
#define __HelperLogics_H__

#include "OgrePrerequisites.h"
#include "OgreCompositorLogic.h"
#include "OgreCompositorInstance.h"

/**
   The simple types of compositor logics will all do the same thing.
   Attach a listener to the created compositor.

   Demo note:
   This file contains three compositor logics for the HDR, Heat Vision
   and Gaussian Blur compositors.  If you wish to use these
   compositors in your application, make sure to add this code as
   well.  This code fits the plugin architecture pretty well, but was
   not exported to a plugin for simplification of the SDK package.
   @see CompositorDemo::loadResources
*/
#include "ListenerFactoryLogic.h"

/// The compositor logic for the heat vision compositor.
class HeatVisionLogic : public ListenerFactoryLogic
{
 protected:
    /** @copydoc ListenerFactoryLogic::createListener */
    Ogre::CompositorInstance::Listener* createListener(Ogre::CompositorInstance* instance) override;
};

/// The compositor logic for the HDR compositor.
class HDRLogic : public ListenerFactoryLogic
{
 protected:
    /** @copydoc ListenerFactoryLogic::createListener */
    Ogre::CompositorInstance::Listener* createListener(Ogre::CompositorInstance* instance) override;
};

/// The compositor logic for the gaussian blur compositor.
class GaussianBlurLogic : public ListenerFactoryLogic
{
 protected:
    /** @copydoc ListenerFactoryLogic::createListener */
    Ogre::CompositorInstance::Listener* createListener(Ogre::CompositorInstance* instance) override;
};

#endif
