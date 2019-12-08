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
#ifndef H_THINGRENDERABLE
#define H_THINGRENDERABLE

#include "OgrePrerequisites.h"
#include "OgreSimpleRenderable.h"
#include "OgreQuaternion.h"
#include "OgreVector.h"

/** Quad fragments that rotate around origin (0,0,0) in a random orbit, always oriented to 0,0,0.
    @author W.J. van der Laan
*/
class ThingRenderable: public Ogre::SimpleRenderable {
public:
    /** Create one this object.
        @param radius   Radius of orbits
        @param count    Number of quads
        @param qsize    Size of quads
     */
    ThingRenderable(float radius, size_t count, float qsize);
    ~ThingRenderable();
    /**
     * Retrieves ratios of the origin-centered bounding sphere for this
     * object.
     */
    Ogre::Real getBoundingRadius() const;
    /**
     * Returns the camera-relative squared depth of this renderable.
     */
    Ogre::Real getSquaredViewDepth(const Ogre::Camera*) const;
    /**
     * Notify that t seconds have elapsed.
     */
    void addTime(float t);
protected:
    void initialise();
    void fillBuffer();

    Ogre::HardwareVertexBufferSharedPtr vbuf;// Our vertex buffer
    float mRadius;
    size_t mCount;
    float mQSize;
    std::vector <Ogre::Quaternion> things;
    std::vector <Ogre::Quaternion> orbits;
};

#endif
