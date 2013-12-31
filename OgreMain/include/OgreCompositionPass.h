/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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
#ifndef __CompositionPass_H__
#define __CompositionPass_H__

#include "OgrePrerequisites.h"
#include "OgreMaterial.h"
#include "OgreRenderSystem.h"
#include "OgreRenderQueue.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Effects
	*  @{
	*/
	/** Object representing one pass or operation in a composition sequence. This provides a 
		method to conveniently interleave RenderSystem commands between Render Queues.
	 */
	class _OgreExport CompositionPass : public CompositorInstAlloc
    {
    public:
        CompositionPass(CompositionTargetPass *parent);
        ~CompositionPass();
        
        /** Enumeration that enumerates the various composition pass types.
        */
        enum PassType
        {
            PT_CLEAR,           /// Clear target to one colour
			PT_STENCIL,			/// Set stencil operation
            PT_RENDERSCENE,     /// Render the scene or part of it
            PT_RENDERQUAD,      /// Render a full screen quad
			PT_RENDERCUSTOM		/// Render a custom sequence
        };
        
        /** Set the type of composition pass */
        void setType(PassType type);
        /** Get the type of composition pass */
        PassType getType() const;
        
		/** Set an identifier for this pass. This identifier can be used to
			"listen in" on this pass with an CompositorInstance::Listener. 
		*/
		void setIdentifier(uint32 id);
		/** Get the identifier for this pass */
		uint32 getIdentifier() const;

        /** Set the material used by this pass
			@note applies when PassType is RENDERQUAD 
		*/
        void setMaterial(const MaterialPtr& mat);
        /** Set the material used by this pass 
			@note applies when PassType is RENDERQUAD 
		*/
        void setMaterialName(const String &name);
        /** Get the material used by this pass 
			@note applies when PassType is RENDERQUAD 
		*/
        const MaterialPtr& getMaterial() const;
		/** Set the first render queue to be rendered in this pass (inclusive) 
			@note applies when PassType is RENDERSCENE
		*/
        void setFirstRenderQueue(uint8 id);
		/** Get the first render queue to be rendered in this pass (inclusive) 
			@note applies when PassType is RENDERSCENE
		*/
		uint8 getFirstRenderQueue() const;
		/** Set the last render queue to be rendered in this pass (inclusive) 
			@note applies when PassType is RENDERSCENE
		*/
        void setLastRenderQueue(uint8 id);
		/** Get the last render queue to be rendered in this pass (inclusive) 
			@note applies when PassType is RENDERSCENE
		*/
		uint8 getLastRenderQueue() const;

		/** Set the material scheme used by this pass.
		@remarks
			Only applicable to passes that render the scene.
			@see Technique::setScheme.
		*/
		void setMaterialScheme(const String& schemeName);
		/** Get the material scheme used by this pass.
		@remarks
			Only applicable to passes that render the scene.
			@see Technique::setScheme.
		*/
		const String& getMaterialScheme(void) const;

		/** Would be nice to have for RENDERSCENE:
			flags to:
				exclude transparents
				override material (at least -- color)
		*/

        /** Set the viewport clear buffers  (defaults to FBT_COLOUR|FBT_DEPTH)
            @param val is a combination of FBT_COLOUR, FBT_DEPTH, FBT_STENCIL.
			@note applies when PassType is CLEAR
        */
        void setClearBuffers(uint32 val);
        /** Get the viewport clear buffers.
			@note applies when PassType is CLEAR
        */
        uint32 getClearBuffers() const;
        /** Set the viewport clear colour (defaults to 0,0,0,0) 
			@note applies when PassType is CLEAR
		 */
        void setClearColour(ColourValue val);
        /** Get the viewport clear colour (defaults to 0,0,0,0)	
			@note applies when PassType is CLEAR
		 */
        const ColourValue &getClearColour() const;
        /** Set the viewport clear depth (defaults to 1.0) 
			@note applies when PassType is CLEAR
		*/
        void setClearDepth(Real depth);
        /** Get the viewport clear depth (defaults to 1.0) 
			@note applies when PassType is CLEAR
		*/
        Real getClearDepth() const;
		/** Set the viewport clear stencil value (defaults to 0) 
			@note applies when PassType is CLEAR
		*/
        void setClearStencil(uint32 value);
        /** Get the viewport clear stencil value (defaults to 0) 
			@note applies when PassType is CLEAR
		*/
        uint32 getClearStencil() const;

		/** Set stencil check on or off.
			@note applies when PassType is STENCIL
		*/
		void setStencilCheck(bool value);
		/** Get stencil check enable.
			@note applies when PassType is STENCIL
		*/
		bool getStencilCheck() const;
		/** Set stencil compare function.
			@note applies when PassType is STENCIL
		*/
		void setStencilFunc(CompareFunction value); 
		/** Get stencil compare function.
			@note applies when PassType is STENCIL
		*/
		CompareFunction getStencilFunc() const; 
		/** Set stencil reference value.
			@note applies when PassType is STENCIL
		*/
		void setStencilRefValue(uint32 value);
		/** Get stencil reference value.
			@note applies when PassType is STENCIL
		*/
		uint32 getStencilRefValue() const;
		/** Set stencil mask.
			@note applies when PassType is STENCIL
		*/
		void setStencilMask(uint32 value);
		/** Get stencil mask.
			@note applies when PassType is STENCIL
		*/
		uint32 getStencilMask() const;
		/** Set stencil fail operation.
			@note applies when PassType is STENCIL
		*/
		void setStencilFailOp(StencilOperation value);
		/** Get stencil fail operation.
			@note applies when PassType is STENCIL
		*/
		StencilOperation getStencilFailOp() const;
		/** Set stencil depth fail operation.
			@note applies when PassType is STENCIL
		*/
		void setStencilDepthFailOp(StencilOperation value);
		/** Get stencil depth fail operation.
			@note applies when PassType is STENCIL
		*/
		StencilOperation getStencilDepthFailOp() const;
		/** Set stencil pass operation.
			@note applies when PassType is STENCIL
		*/
		void setStencilPassOp(StencilOperation value);
		/** Get stencil pass operation.
			@note applies when PassType is STENCIL
		*/
		StencilOperation getStencilPassOp() const;
		/** Set two sided stencil operation.
			@note applies when PassType is STENCIL
		*/
		void setStencilTwoSidedOperation(bool value);
		/** Get two sided stencil operation.
			@note applies when PassType is STENCIL
		*/
		bool getStencilTwoSidedOperation() const;

		/// Inputs (for material used for rendering the quad)
		struct InputTex
		{
			/// Name (local) of the input texture (empty == no input)
			String name;
			/// MRT surface index if applicable
			size_t mrtIndex;
			InputTex() : name(StringUtil::BLANK), mrtIndex(0) {}
			InputTex(const String& _name, size_t _mrtIndex = 0)
				: name(_name), mrtIndex(_mrtIndex) {}
		};

        /** Set an input local texture. An empty string clears the input.
            @param id    Input to set. Must be in 0..OGRE_MAX_TEXTURE_LAYERS-1
            @param input Which texture to bind to this input. An empty string clears the input.
			@param mrtIndex Which surface of an MRT to retrieve
			@note applies when PassType is RENDERQUAD 
        */
        void setInput(size_t id, const String &input=StringUtil::BLANK, size_t mrtIndex=0);
        
        /** Get the value of an input.
            @param id    Input to get. Must be in 0..OGRE_MAX_TEXTURE_LAYERS-1.
			@note applies when PassType is RENDERQUAD 
        */
        const InputTex &getInput(size_t id) const;
        
        /** Get the number of inputs used.
			@note applies when PassType is RENDERQUAD 
        */
        size_t getNumInputs() const;
        
        /** Clear all inputs.
			@note applies when PassType is RENDERQUAD 
        */
        void clearAllInputs();
        
        /** Get parent object 
			@note applies when PassType is RENDERQUAD 
		*/
        CompositionTargetPass *getParent();

        /** Determine if this target pass is supported on the current rendering device. 
         */
        bool _isSupported(void);

        /** Set quad normalised positions [-1;1]x[-1;1]
            @note applies when PassType is RENDERQUAD
         */
        void setQuadCorners(Real left,Real top,Real right,Real bottom);

        /** Get quad normalised positions [-1;1]x[-1;1]
            @note applies when PassType is RENDERQUAD 
         */
        bool getQuadCorners(Real & left,Real & top,Real & right,Real & bottom) const;
            
		/** Sets the use of camera frustum far corners provided in the quad's normals
			@note applies when PassType is RENDERQUAD 
		*/
		void setQuadFarCorners(bool farCorners, bool farCornersViewSpace);

		/** Returns true if camera frustum far corners are provided in the quad.
			@note applies when PassType is RENDERQUAD 
		*/
		bool getQuadFarCorners() const;

		/** Returns true if the far corners provided in the quad are in view space
			@note applies when PassType is RENDERQUAD 
		*/
		bool getQuadFarCornersViewSpace() const;

		/** Set the type name of this custom composition pass.
			@note applies when PassType is RENDERCUSTOM
			@see CompositorManager::registerCustomCompositionPass
		*/
		void setCustomType(const String& customType);

		/** Get the type name of this custom composition pass.
			@note applies when PassType is RENDERCUSTOM
			@see CompositorManager::registerCustomCompositionPass
		*/
		const String& getCustomType() const;

    private:
        /// Parent technique
        CompositionTargetPass *mParent;
        /// Type of composition pass
        PassType mType;
		/// Identifier for this pass
		uint32 mIdentifier;
        /// Material used for rendering
        MaterialPtr mMaterial;
        /// [first,last] render queue to render this pass (in case of PT_RENDERSCENE)
		uint8 mFirstRenderQueue;
		uint8 mLastRenderQueue;
		/// Material scheme name
		String mMaterialScheme;
        /// Clear buffers (in case of PT_CLEAR)
        uint32 mClearBuffers;
        /// Clear colour (in case of PT_CLEAR)
        ColourValue mClearColour;
		/// Clear depth (in case of PT_CLEAR)
		Real mClearDepth;
		/// Clear stencil value (in case of PT_CLEAR)
		uint32 mClearStencil;
        /** Inputs (for material used for rendering the quad).
            An empty string signifies that no input is used */
        InputTex mInputs[OGRE_MAX_TEXTURE_LAYERS];
		/// Stencil operation parameters
		bool mStencilCheck;
		CompareFunction mStencilFunc; 
		uint32 mStencilRefValue;
		uint32 mStencilMask;
		StencilOperation mStencilFailOp;
		StencilOperation mStencilDepthFailOp;
		StencilOperation mStencilPassOp;
		bool mStencilTwoSidedOperation;

        /// True if quad should not cover whole screen
        bool mQuadCornerModified;
        /// quad positions in normalised coordinates [-1;1]x[-1;1] (in case of PT_RENDERQUAD)
        Real mQuadLeft;
        Real mQuadTop;
        Real mQuadRight;
        Real mQuadBottom;

		bool mQuadFarCorners, mQuadFarCornersViewSpace;
		/// The type name of the custom composition pass.
		String mCustomType;
    };
	/** @} */
	/** @} */

}

#include "OgreHeaderSuffix.h"

#endif
