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
#ifndef H_WJ_MLight
#define H_WJ_MLight

#include "OgreSimpleRenderable.h"
#include "MaterialGenerator.h"

/** Renderable minilight. Do not create this directly, but use 
	DeferredShadingSystem::createMLight.
	XXX support other types of light other than point lights.
 */
class MLight: public Ogre::SimpleRenderable
{
public:
	MLight(MaterialGenerator *gen);
	~MLight();

	/** Permutation of light materials
	 */
	enum MaterialID
	{
		MI_QUAD			= 0x1, // Rendered as fullscreen quad
		MI_ATTENUATED	= 0x2, // Rendered attenuated
		MI_SPECULAR		= 0x4  // Specular component is calculated
	};

	/** Set constant, linear, quadratic Attenuation terms 
	 */
	void setAttenuation(float c, float b, float a);

	/** Set the diffuse colour 
	 */
	void setDiffuseColour(const Ogre::ColourValue &col);
	void setDiffuseColour(float r=1.0f, float g=1.0f, float b=1.0f, float a=1.0f)
	{
		setDiffuseColour(Ogre::ColourValue(r,g,b,a));
	}

	/** Set the specular colour
	 */
	void setSpecularColour(const Ogre::ColourValue &col);
	void setSpecularColour(float r=1.0f, float g=1.0f, float b=1.0f, float a=1.0f)
	{
		setSpecularColour(Ogre::ColourValue(r,g,b,a));
	}

	/** Get diffuse colour.
	*/
	Ogre::ColourValue getDiffuseColour();

	/** Get specular colour. 
	*/
	Ogre::ColourValue getSpecularColour();

	/** Create geometry for this light.
	*/
	void rebuildGeometry(float radius);

	/** Create a sphere geometry.
	*/
	void createSphere(const float radius, const int nRings, const int nSegments);

	/** Create a rectangle.
	*/
	void createRectangle2D();

	/** @copydoc MovableObject::getBoundingRadius */
	virtual Ogre::Real getBoundingRadius(void) const;
	/** @copydoc Renderable::getSquaredViewDepth */
	virtual Ogre::Real getSquaredViewDepth(const Ogre::Camera*) const;
	/** @copydoc Renderable::getMaterial */
	virtual const Ogre::MaterialPtr& getMaterial(void) const;
protected:
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
