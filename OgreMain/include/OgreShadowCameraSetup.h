/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
*/
#ifndef __ShadowCameraSetup_H__
#define __ShadowCameraSetup_H__

#include "OgrePrerequisites.h"
#include "OgreMovablePlane.h"
#include "OgreSharedPtr.h"


namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Scene
	*  @{
	*/
	/** This class allows you to plug in new ways to define the camera setup when
		rendering and projecting shadow textures.
	@remarks
		The default projection used when rendering shadow textures is a uniform
		frustum. This is pretty straight forward but doesn't make the best use of 
		the space in the shadow map since texels closer to the camera will be larger, 
		resulting in 'jaggies'. There are several ways to distribute the texels
		in the shadow texture differently, and this class allows you to override
		that. 
	@par
		Ogre is provided with several alternative shadow camera setups, including
		LiSPSM (LiSPSMShadowCameraSetup) and Plane Optimal (PlaneOptimalShadowCameraSetup).
		Others can of course be written to incorporate other algorithms. All you 
		have to do is instantiate one of these classes and enable it using 
		SceneManager::setShadowCameraSetup (global) or Light::setCustomShadowCameraSetup
		(per light). In both cases the instance is wrapped in a SharedPtr which means
		it will  be deleted automatically when no more references to it exist.
	@note
        Shadow map matrices, being projective matrices, have 15 degrees of freedom.
		3 of these degrees of freedom are fixed by the light's position.  4 are used to
		affinely affect z values.  6 affinely affect u,v sampling.  2 are projective
		degrees of freedom.  This class is meant to allow custom methods for 
		handling optimization.
    */
	class _OgreExport ShadowCameraSetup : public ShadowDataAlloc
	{
	public:
		/// Function to implement -- must set the shadow camera properties
		virtual void getShadowCamera (const SceneManager *sm, const Camera *cam, 
									  const Viewport *vp, const Light *light, Camera *texCam, size_t iteration) const = 0;
		/// Need virtual destructor in case subclasses use it
		virtual ~ShadowCameraSetup() {}

	};



	/** Implements default shadow camera setup
        @remarks
            This implements the default shadow camera setup algorithm.  This is what might
			be referred to as "normal" shadow mapping.  
    */
	class _OgreExport DefaultShadowCameraSetup : public ShadowCameraSetup
	{
	public:
		/// Default constructor
		DefaultShadowCameraSetup();
		/// Destructor
		virtual ~DefaultShadowCameraSetup();

		/// Default shadow camera setup
		virtual void getShadowCamera (const SceneManager *sm, const Camera *cam, 
									  const Viewport *vp, const Light *light, Camera *texCam, size_t iteration) const;
	};



	typedef SharedPtr<ShadowCameraSetup> ShadowCameraSetupPtr;
	/** @} */
	/** @} */

}

#endif
