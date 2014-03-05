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
	VolumeRenderable(Ogre::IdType id, Ogre::ObjectMemoryManager *objectMemoryManager,
					 size_t nSlices, float size, const Ogre::String & texture);
    ~VolumeRenderable();
    
    // Copydoc Ogre::SimpleRenderable::notifyCurrentCamera
    virtual void _updateRenderQueue(Ogre::RenderQueue* queue, Ogre::Camera *camera,
                                    const Ogre::Camera *lodCamera);
    void getWorldTransforms( Ogre::Matrix4* xform ) const;
    
    /**
     * Returns the camera-relative squared depth of this renderable.
     */
    Ogre::Real getSquaredViewDepth(const Ogre::Camera*) const;
protected:
    void initialise();

    size_t mSlices;
	float mSize;
    Ogre::Matrix3 mFakeOrientation;
    Ogre::String mTexture;
    Ogre::TextureUnitState *mUnit;
};
#endif
