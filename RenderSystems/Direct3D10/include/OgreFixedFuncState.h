/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#ifndef __FixedFuncState_H__
#define __FixedFuncState_H__

#include "OgrePrerequisites.h"
#include "OgreGpuProgram.h"
#include "OgreColourValue.h"
#include "OgreBlendMode.h"
#include "OgreCommon.h"
#include "OgreLight.h"
#include "OgreTextureUnitState.h"
#include "OgreRenderSystem.h"

namespace Ogre {


	class TextureLayerState
	{
	protected:
		TextureType mTextureType;
		TexCoordCalcMethod mTexCoordCalcMethod;
		LayerBlendModeEx mLayerBlendOperationEx;
		uint8 mCoordIndex;
	public:
		TextureLayerState();
		~TextureLayerState();
		TextureType getTextureType() const;
		void setTextureType(TextureType val);
		TexCoordCalcMethod getTexCoordCalcMethod() const;
		void setTexCoordCalcMethod(TexCoordCalcMethod val);
		LayerBlendModeEx getLayerBlendModeEx() const;
		void setLayerBlendModeEx(LayerBlendModeEx val);
		uint8 getCoordIndex() const;
		void setCoordIndex(uint8 val);
	};

	typedef vector<TextureLayerState>::type TextureLayerStateList;

	class VertexBufferElement
	{
	protected:
		VertexElementSemantic mVertexElementSemantic;
		VertexElementType mVertexElementType;
		unsigned short mVertexElementIndex;
	public:
		VertexElementSemantic getVertexElementSemantic() const;
		void setVertexElementSemantic(VertexElementSemantic val);
		VertexElementType getVertexElementType() const;
		void setVertexElementType(VertexElementType val);
		unsigned short getVertexElementIndex() const;
		void setVertexElementIndex(unsigned short val);
	};

	typedef vector<VertexBufferElement>::type VertexBufferElementList;

	class VertexBufferDeclaration
	{
	protected:
		VertexBufferElementList mVertexBufferElementList;
	public:
		VertexBufferDeclaration();
		~VertexBufferDeclaration();
		const VertexBufferElementList & getVertexBufferElementList() const;
		void setVertexBufferElementList(const VertexBufferElementList & val);

		const bool operator<(const VertexBufferDeclaration & other) const;

		bool hasColor() const;
		uint8 getTexcoordCount() const;
		unsigned short numberOfTexcoord() const;
		unsigned short countVertexElementSemantic( VertexElementSemantic semantic ) const;

	};

	class GeneralFixedFuncState // this is a class of the "global parameters" so I can use memcmp on them
	{
	protected:
		/// Normalisation
		bool mNormaliseNormals;
		//-------------------------------------------------------------------------
		// Alpha reject settings
		CompareFunction mAlphaRejectFunc;
		//-------------------------------------------------------------------------
		// Fog
		FogMode mFogMode;
		//-------------------------------------------------------------------------
		/// Lighting enabled?
		bool mLightingEnabled;

#define LIGHT_TYPES_COUNT Light::LT_SPOTLIGHT + 1 // better to update the LightTypes enum
		// a counter for each of the light types
		uint8 mLightFromTypeCount[LIGHT_TYPES_COUNT];
		/// Shading options
		ShadeOptions mShadeOptions;
	public:
		/// Default constructor
		GeneralFixedFuncState();
		~GeneralFixedFuncState();

		/** Sets whether or not dynamic lighting is enabled.
		@param
		enabled
		If true, dynamic lighting is performed on geometry with normals supplied, geometry without
		normals will not be displayed.
		@par
		If false, no lighting is applied and all geometry will be full brightness.
		*/
		void setLightingEnabled(bool enabled);

		/** Returns whether or not dynamic lighting is enabled.
		*/
		bool getLightingEnabled(void) const;

		/** Sets the type of light shading required
		@note
		The default shading method is Gouraud shading.
		*/
		void setShadingMode( ShadeOptions mode );

		/** Returns the type of light shading to be used.
		*/
		ShadeOptions getShadingMode(void) const;

		/** Sets the fogging mode applied to this pass.
		@remarks
		Fogging is an effect that is applied as polys are rendered. Sometimes, you want
		fog to be applied to an entire scene. Other times, you want it to be applied to a few
		polygons only. This pass-level specification of fog parameters lets you easily manage
		both.
		@par
		The SceneManager class also has a setFog method which applies scene-level fog. This method
		lets you change the fog behaviour for this pass compared to the standard scene-level fog.
		@param
		overrideScene If true, you authorise this pass to override the scene's fog params with it's own settings.
		If you specify false, so other parameters are necessary, and this is the default behaviour for passes.
		@param
		mode Only applicable if overrideScene is true. You can disable fog which is turned on for the
		rest of the scene by specifying FOG_NONE. Otherwise, set a pass-specific fog mode as
		defined in the enum FogMode.
		@param
		colour The colour of the fog. Either set this to the same as your viewport background colour,
		or to blend in with a skydome or skybox.
		@param
		expDensity The density of the fog in FOG_EXP or FOG_EXP2 mode, as a value between 0 and 1.
		The default is 0.001.
		@param
		linearStart Distance in world units at which linear fog starts to encroach.
		Only applicable if mode is FOG_LINEAR.
		@param
		linearEnd Distance in world units at which linear fog becomes completely opaque.
		Only applicable if mode is FOG_LINEAR.
		*/
		void setFogMode( FogMode mode );

		/** Returns the fog mode for this pass.
		@note
		Only valid if getFogOverride is true.
		*/
		FogMode getFogMode(void) const;

		/** Sets the alpha reject function. See setAlphaRejectSettings for more information.
		*/
		void setAlphaRejectFunction(CompareFunction func);

		/** Gets the alpha reject function. See setAlphaRejectSettings for more information.
		*/
		CompareFunction getAlphaRejectFunction(void) const;


		/** If set to true, this forces normals to be normalised dynamically 
		by the hardware for this pass.
		@remarks
		This option can be used to prevent lighting variations when scaling an
		object - normally because this scaling is hardware based, the normals 
		get scaled too which causes lighting to become inconsistent. By default the
		SceneManager detects scaled objects and does this for you, but 
		this has an overhead so you might want to turn that off through
		SceneManager::setNormaliseNormalsOnScale(false) and only do it per-Pass
		when you need to.
		*/
		void setNormaliseNormals(bool normalise);

		/** Returns true if this pass has auto-normalisation of normals set. */
		bool getNormaliseNormals(void) const;

		const uint8 getLightTypeCount(const Light::LightTypes type) const;
		void setLightTypeCount(const Light::LightTypes type, const uint8 val);
		const uint8 getTotalNumberOfLights() const;
		void resetLightTypeCounts();
		void addOnetoLightTypeCount(const Light::LightTypes type);

	};

    /** Class defining a fixed function state.
    @remarks
	The Fixed Function Pipeline (FFP abbreviated) is one of currently two methods of modifying 
	the graphic output. The other is the Programmable Pipeline also known as Shaders.
	With the FFP you can choose one of those algorithms and several ways to set or 
	modify the factors. There is only a handful of predefined algorithms and you cannot 
	add handcrafted ones. Hence the name Fixed Function Pipeline.
	One of the big differences of d3d10 from previous versions of d3d and also from openGL is
	the it doesn't have support for the FFP - the motivation for this class cames from the needs
	of the d3d10 render system. 
	Usually you will get better performance if you use the FFP and not the FFP shader emulation.
	The second common use for this class is to generate the base code for a new shader.
	    */
    class FixedFuncState
    {
	public:
    protected:
		GeneralFixedFuncState mGeneralFixedFuncState;
        //-------------------------------------------------------------------------
        /// Storage of texture layer states
        TextureLayerStateList mTextureLayerStateList;
		//-------------------------------------------------------------------------
    public:
        /// Default constructor
		FixedFuncState();
		~FixedFuncState();

		const TextureLayerStateList & getTextureLayerStateList() const;
		void setTextureLayerStateList(const TextureLayerStateList & val);
		GeneralFixedFuncState & getGeneralFixedFuncState();

		const bool operator<(const FixedFuncState & other) const;

	};

}

#endif
