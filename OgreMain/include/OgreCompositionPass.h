/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef __CompositionPass_H__
#define __CompositionPass_H__

#include "OgrePrerequisites.h"
#include "OgreMaterial.h"
#include "OgreRenderSystem.h"
#include "OgreRenderQueue.h"

namespace Ogre {
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
            PT_CLEAR,           // Clear target to one colour
			PT_STENCIL,			// Set stencil operation
            PT_RENDERSCENE,     // Render the scene or part of it
            PT_RENDERQUAD       // Render a full screen quad
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
		uint8 getFirstRenderQueue();
		/** Set the last render queue to be rendered in this pass (inclusive) 
			@note applies when PassType is RENDERSCENE
		*/
        void setLastRenderQueue(uint8 id);
		/** Get the last render queue to be rendered in this pass (inclusive) 
			@note applies when PassType is RENDERSCENE
		*/
		uint8 getLastRenderQueue();

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
        uint32 getClearBuffers();
        /** Set the viewport clear colour (defaults to 0,0,0,0) 
			@note applies when PassType is CLEAR
		 */
        void setClearColour(ColourValue val);
        /** Get the viewport clear colour (defaults to 0,0,0,0)	
			@note applies when PassType is CLEAR
		 */
        const ColourValue &getClearColour();
        /** Set the viewport clear depth (defaults to 1.0) 
			@note applies when PassType is CLEAR
		*/
        void setClearDepth(Real depth);
        /** Get the viewport clear depth (defaults to 1.0) 
			@note applies when PassType is CLEAR
		*/
        Real getClearDepth();
		/** Set the viewport clear stencil value (defaults to 0) 
			@note applies when PassType is CLEAR
		*/
        void setClearStencil(uint32 value);
        /** Get the viewport clear stencil value (defaults to 0) 
			@note applies when PassType is CLEAR
		*/
        uint32 getClearStencil();

		/** Set stencil check on or off.
			@note applies when PassType is STENCIL
		*/
		void setStencilCheck(bool value);
		/** Get stencil check enable.
			@note applies when PassType is STENCIL
		*/
		bool getStencilCheck();
		/** Set stencil compare function.
			@note applies when PassType is STENCIL
		*/
		void setStencilFunc(CompareFunction value); 
		/** Get stencil compare function.
			@note applies when PassType is STENCIL
		*/
		CompareFunction getStencilFunc(); 
		/** Set stencil reference value.
			@note applies when PassType is STENCIL
		*/
		void setStencilRefValue(uint32 value);
		/** Get stencil reference value.
			@note applies when PassType is STENCIL
		*/
		uint32 getStencilRefValue();
		/** Set stencil mask.
			@note applies when PassType is STENCIL
		*/
		void setStencilMask(uint32 value);
		/** Get stencil mask.
			@note applies when PassType is STENCIL
		*/
		uint32 getStencilMask();
		/** Set stencil fail operation.
			@note applies when PassType is STENCIL
		*/
		void setStencilFailOp(StencilOperation value);
		/** Get stencil fail operation.
			@note applies when PassType is STENCIL
		*/
		StencilOperation getStencilFailOp();
		/** Set stencil depth fail operation.
			@note applies when PassType is STENCIL
		*/
		void setStencilDepthFailOp(StencilOperation value);
		/** Get stencil depth fail operation.
			@note applies when PassType is STENCIL
		*/
		StencilOperation getStencilDepthFailOp();
		/** Set stencil pass operation.
			@note applies when PassType is STENCIL
		*/
		void setStencilPassOp(StencilOperation value);
		/** Get stencil pass operation.
			@note applies when PassType is STENCIL
		*/
		StencilOperation getStencilPassOp();
		/** Set two sided stencil operation.
			@note applies when PassType is STENCIL
		*/
		void setStencilTwoSidedOperation(bool value);
		/** Get two sided stencil operation.
			@note applies when PassType is STENCIL
		*/
		bool getStencilTwoSidedOperation();

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
        const InputTex &getInput(size_t id);
        
        /** Get the number of inputs used.
			@note applies when PassType is RENDERQUAD 
        */
        size_t getNumInputs();
        
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
        /// Clear buffers (in case of PT_CLEAR)
        uint32 mClearBuffers;
        /// Clear colour (in case of PT_CLEAR)
        ColourValue mClearColour;
		/// Clear depth (in case of PT_CLEAR)
		Real mClearDepth;
		/// Clear stencil value (in case of PT_CLEAR)
		uint32 mClearStencil;
        /// Inputs (for material used for rendering the quad)
        /// An empty string signifies that no input is used
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

        /// true if quad should not cover whole screen
        bool mQuadCornerModified;
        /// quad positions in normalised coordinates [-1;1]x[-1;1] (in case of PT_RENDERQUAD)
        Real mQuadLeft;
        Real mQuadTop;
        Real mQuadRight;
        Real mQuadBottom;

		bool mQuadFarCorners, mQuadFarCornersViewSpace;
    };

}

#endif
