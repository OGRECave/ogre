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

#ifndef _SSAOLOGIC_H
#define _SSAOLOGIC_H

#include "ListenerFactoryLogic.h"

class SSAOLogic : public ListenerFactoryLogic
{
protected:
    /** @copydoc ListenerFactoryLogic::createListener */
    Ogre::CompositorInstance::Listener* createListener(Ogre::CompositorInstance* instance) override;
};

#endif
