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
#ifndef OWL_VOLUMERENDERABLE
#define OWL_VOLUMERENDERABLE
#include "OgrePrerequisites.h"
#include "OgreSimpleRenderable.h"

/** Direct Volume Rendering.
    TODO: LOD: reduce number of slices in distance
    TODO: option to generate normals for lighting
    @author W.J. van der Laan
*/
class VolumeRenderable: public Ogre::SimpleRenderable {
public:
    VolumeRenderable(size_t nSlices, float size, const Ogre::String & texture);
    ~VolumeRenderable();
    
    // Copydoc Ogre::SimpleRenderable::notifyCurrentCamera
    void _notifyCurrentCamera( Ogre::Camera* cam );
    void getWorldTransforms( Ogre::Matrix4* xform ) const;
    
    /**
     * Retrieves ratios of the origin-centered bounding sphere for this
     * object.
     */
    Ogre::Real getBoundingRadius() const;
    
    Ogre::Real getSquaredViewDepth(const Ogre::Camera*) const override;
protected:
    void initialise();

    size_t mSlices;
    float mSize;
    float mRadius;
    Ogre::Matrix3 mFakeOrientation;
    Ogre::String mTexture;
};
#endif
