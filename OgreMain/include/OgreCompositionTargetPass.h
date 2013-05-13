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
#ifndef __CompositionTargetPass_H__
#define __CompositionTargetPass_H__

#include "OgrePrerequisites.h"
#include "OgreIteratorWrappers.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Effects
	*  @{
	*/
	/** Object representing one render to a RenderTarget or Viewport in the Ogre Composition
		framework.
	 */
	class _OgreExport CompositionTargetPass : public CompositorInstAlloc
    {
    public:
        CompositionTargetPass(CompositionTechnique *parent);
        ~CompositionTargetPass();
        
        /** Input mode of a TargetPass
        */
        enum InputMode
        {
            IM_NONE,        /// No input
            IM_PREVIOUS     /// Output of previous Composition in chain
        };
        typedef vector<CompositionPass *>::type Passes;
        typedef VectorIterator<Passes> PassIterator;
        
        /** Set input mode of this TargetPass
        */
        void setInputMode(InputMode mode);
        /** Get input mode */
        InputMode getInputMode() const;
        
        /** Set output local texture name */
        void setOutputName(const String &out);
        /** Get output local texture name */
        const String &getOutputName() const;
        
        /** Set "only initial" flag. This makes that this target pass is only executed initially 
            after the effect has been enabled.
        */
        void setOnlyInitial(bool value);
        /** Get "only initial" flag.
        */
        bool getOnlyInitial();
        
        /** Set the scene visibility mask used by this pass 
        */
        void setVisibilityMask(uint32 mask);
        /** Get the scene visibility mask used by this pass 
        */
        uint32 getVisibilityMask();

		/** Set the material scheme used by this target pass.
		@remarks
			Only applicable to targets that render the scene as
			one of their passes.
			@see Technique::setScheme.
		*/
		void setMaterialScheme(const String& schemeName);
		/** Get the material scheme used by this target pass.
		@remarks
			Only applicable to targets that render the scene as
			one of their passes.
			@see Technique::setScheme.
		*/
		const String& getMaterialScheme(void) const;
        
		/** Set whether shadows are enabled in this target pass.
		@remarks
			Only applicable to targets that render the scene as
			one of their passes.
		*/
		void setShadowsEnabled(bool enabled);
		/** Get whether shadows are enabled in this target pass.
		@remarks
			Only applicable to targets that render the scene as
			one of their passes.
		*/
		bool getShadowsEnabled(void) const;
        /** Set the scene LOD bias used by this pass. The default is 1.0,
            everything below that means lower quality, higher means higher quality.
        */
        void setLodBias(float bias);
        /** Get the scene LOD bias used by this pass 
        */
        float getLodBias();
        
        /** Create a new pass, and return a pointer to it.
        */
        CompositionPass *createPass();
        /** Remove a pass. It will also be destroyed.
        */
        void removePass(size_t idx);
        /** Get a pass.
        */
        CompositionPass *getPass(size_t idx);
        /** Get the number of passes.
        */
        size_t getNumPasses();
        
        /** Remove all passes
        */
        void removeAllPasses();
    
        /** Get an iterator over the Passes in this TargetPass. */
        PassIterator getPassIterator(void);
        
        /** Get parent object */
        CompositionTechnique *getParent();

        /** Determine if this target pass is supported on the current rendering device. 
         */
        bool _isSupported(void);

    private:
        /// Parent technique
        CompositionTechnique *mParent;
        /// Input mode
        InputMode mInputMode;
        /// (local) output texture
        String mOutputName;
        /// Passes
        Passes mPasses;
        /// This target pass is only executed initially after the effect
        /// has been enabled.
        bool mOnlyInitial;
        /// Visibility mask for this render
        uint32 mVisibilityMask;
        /// LOD bias of this render
        float mLodBias;
		/// Material scheme name
		String mMaterialScheme;
		/// Shadows option
		bool mShadowsEnabled;
    };

	/** @} */
	/** @} */
}

#include "OgreHeaderSuffix.h"

#endif
