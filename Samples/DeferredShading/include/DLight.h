/**
*******************************************************************************
Copyright (c) W.J. van der Laan

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software  and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to use, 
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so, subject 
to the following conditions:

The above copyright notice and this permission notice shall be included in all copies 
or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION 
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*******************************************************************************
*/
#ifndef H_WJ_DLight
#define H_WJ_DLight

#include "OgreSimpleRenderable.h"
#include "MaterialGenerator.h"

/** Deferred light geometry. Each instance matches a normal light.
	Should not be created by the user.
	XXX support other types of light other than point lights.
 */
class DLight: public Ogre::SimpleRenderable
{
public:
	DLight(MaterialGenerator *gen, Ogre::Light* parentLight);
	~DLight();

	/** Update the information from the light that matches this one 
	 */
	void updateFromParent();

	/** Update the information that is related to the camera
	 */
	void updateFromCamera(Ogre::Camera* camera);

	/** Does this light cast shadows?
	*/
	virtual bool getCastChadows() const;

	/** @copydoc MovableObject::getBoundingRadius */
	virtual Ogre::Real getBoundingRadius(void) const;
	/** @copydoc Renderable::getSquaredViewDepth */
	virtual Ogre::Real getSquaredViewDepth(const Ogre::Camera*) const;
	/** @copydoc Renderable::getMaterial */
	virtual const Ogre::MaterialPtr& getMaterial(void) const;
	/** @copydoc Renderable::getBoundingRadius */
	virtual void getWorldTransforms(Ogre::Matrix4* xform) const;
protected:

	/** Check if the camera is inside a light
	*/
	bool isCameraInsideLight(Ogre::Camera* camera);

	/** Create geometry for this light.
	*/
	void rebuildGeometry(float radius);

	/** Create a sphere geometry.
	*/
	void createSphere(float radius, int nRings, int nSegments);

	/** Create a rectangle.
	*/
	void createRectangle2D();
	
	/** Create a cone.
	*/
	void createCone(float radius, float height, int nVerticesInBase);

	/** Set constant, linear, quadratic Attenuation terms 
	 */
	void setAttenuation(float c, float b, float a);

	/** Set the specular colour
	 */
	void setSpecularColour(const Ogre::ColourValue &col);

	/// The light that this DLight renders
	Ogre::Light* mParentLight;
	/// Mode to ignore world orientation/position
	bool bIgnoreWorld;
	/// Bounding sphere radius
	float mRadius;
	/// Deferred shading system this minilight is part of
	MaterialGenerator *mGenerator;
	/// Material permutation
	Ogre::uint32 mPermutation;
};

#endif
