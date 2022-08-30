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

#ifndef _AMBIENTLIGHT_H
#define _AMBIENTLIGHT_H

#include "OgreSimpleRenderable.h"

// Renderable for rendering Ambient component and also to
// establish the depths

// Just instantiation is sufficient
// Note that instantiation is necessary to at least establish the depths
// even if the current ambient colour is 0

// its ambient colour is same as the scene's ambient colour

// XXX Could make this a singleton/make it private to the DeferredShadingSystem e.g.

class AmbientLight : public Ogre::SimpleRenderable

{
public:
    AmbientLight();
    ~AmbientLight();

    /** @copydoc MovableObject::getBoundingRadius */
    Ogre::Real getBoundingRadius(void) const override;
    /** @copydoc Renderable::getSquaredViewDepth */
    Ogre::Real getSquaredViewDepth(const Ogre::Camera*) const override;
    /** @copydoc Renderable::getMaterial */
    const Ogre::MaterialPtr& getMaterial(void) const override;

    void getWorldTransforms(Ogre::Matrix4* xform) const override;

    void updateFromCamera(Ogre::Camera* camera);
protected:
    Ogre::Real mRadius;
    Ogre::MaterialPtr mMatPtr;
};
 
#endif
