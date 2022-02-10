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

#include "ProceduralManualObject.h"
#include "OgreRenderQueue.h"

namespace Ogre
{
    //-----------------------------------------------------------------------------
    void ProceduralManualObject::_updateRenderQueue(RenderQueue* queue)
    {
        mR2vbObject->update(mManager);
        queue->addRenderable(this);
    }
}
