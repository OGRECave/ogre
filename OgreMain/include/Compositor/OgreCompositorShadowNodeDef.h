/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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

#ifndef __CompositorShadowNodeDef_H__
#define __CompositorShadowNodeDef_H__

#include "OgreHeaderPrefix.h"

#include "Compositor/OgreCompositorNodeDef.h"

namespace Ogre
{
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Effects
	*  @{
	*/

	enum ShadowMapTechniques
	{
		SHADOWMAP_DEFAULT,
		SHADOWMAP_PLANEOPTIMAL,
		SHADOWMAP_FOCUSED,
		SHADOWMAP_LiPSSM,
		SHADOWMAP_PSSM
	};

	/** Shadow Nodes are special nodes (not to be confused with @see CompositorNode)
		that are only used for rendering shadow maps.
		Normal Compositor Nodes can share or own a ShadowNode. The ShadowNode will
		render the scene enough times to fill all shadow maps so the main scene pass
		can use them.
	@par
		ShadowNode are very flexible compared to Ogre 1.x; as they allow mixing multiple
		shadow camera setups for different lights.
    @author
		Matias N. Goldberg
    @version
        1.0
    */
	class _OgreExport CompositorShadowNodeDef : public CompositorNodeDef
	{
		friend class CompositorShadowNode;

	public:
		/// Local texture definition
        class ShadowTextureDefinition : public CompositorInstAlloc
        {
        public:
            IdString	name;
			uint		shadowTextCount;
            uint		width;
            uint		height;
            PixelFormatList formatList; // more than one means MRT
			uint		fsaa;			// FSAA level
			bool		hwGammaWrite;	// Do sRGB gamma correction on write (only 8-bit per channel formats)
			uint16		depthBufferId;	// Depth Buffer's pool ID

			size_t		light;	//Render Nth closest light
			size_t		split;	//Split for that light (only for PSSM/CSM)
			ShadowMapTechniques	shadowMapTechnique;

			ShadowTextureDefinition( ShadowMapTechniques t ) : shadowTextCount(0), width(0), height(0),
					fsaa(0), hwGammaWrite(false), depthBufferId(1), light(0), split(0),
					shadowMapTechnique(t) {}
        };

	protected:
		typedef vector<ShadowTextureDefinition>::type	ShadowMapTexDefVec;
		ShadowMapTexDefVec	mShadowMapTexDefinitions;
		ShadowMapTechniques	mDefaultTechnique;

	public:
		CompositorShadowNodeDef() : mDefaultTechnique( SHADOWMAP_DEFAULT ) {}

		/** Reserves enough memory for all texture definitions
		@remarks
			Calling this function is not obligatory, but recommended
		@param numTex
			The number of shadow textures expected to contain.
		*/
		void setNumShadowTextureDefinitions( size_t numTex )
														{ mShadowMapTexDefinitions.reserve( numTex ); }

		/** Adds a new ShadowTexture definition.
		@remarks
			WARNING: Calling this function may invalidate all previous returned pointers
			unless you've properly called setNumShadowTextureDefinitions
		*/
		ShadowTextureDefinition* addShadowTextureDefinition( size_t lightIdx );
	};

	/** @} */
	/** @} */
}

#include "OgreHeaderSuffix.h"

#endif
