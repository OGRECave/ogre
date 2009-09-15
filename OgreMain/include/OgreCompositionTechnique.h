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
#ifndef __CompositionTechnique_H__
#define __CompositionTechnique_H__

#include "OgrePrerequisites.h"
#include "OgrePixelFormat.h"
#include "OgreIteratorWrappers.h"

namespace Ogre {
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Effects
	*  @{
	*/
	/** Base composition technique, can be subclassed in plugins.
     */
	class _OgreExport CompositionTechnique : public CompositorInstAlloc
    {
    public:
        CompositionTechnique(Compositor *parent);
        virtual ~CompositionTechnique();
    
        /// Local texture definition
        class TextureDefinition : public CompositorInstAlloc
        {
        public:
            String name;
            size_t width;       // 0 means adapt to target width
            size_t height;      // 0 means adapt to target height
			float widthFactor;  // multiple of target width to use (if width = 0)
			float heightFactor; // multiple of target height to use (if height = 0)
            PixelFormatList formatList; // more than one means MRT
			bool fsaa;			// FSAA enabled; true = determine from main target (if render_scene), false = disable
			bool hwGammaWrite;	// Do sRGB gamma correction on write (only 8-bit per channel formats) 
			bool shared;		// whether to use shared textures for this one

			TextureDefinition() :width(0), height(0), widthFactor(1.0f), heightFactor(1.0f), 
				fsaa(true), hwGammaWrite(false), shared(false) {}
        };
        /// Typedefs for several iterators
        typedef vector<CompositionTargetPass *>::type TargetPasses;
        typedef VectorIterator<TargetPasses> TargetPassIterator;
        typedef vector<TextureDefinition*>::type TextureDefinitions;
        typedef VectorIterator<TextureDefinitions> TextureDefinitionIterator;
        
        /** Create a new local texture definition, and return a pointer to it.
            @param name     Name of the local texture
        */
        TextureDefinition *createTextureDefinition(const String &name);
        
        /** Remove and destroy a local texture definition.
        */
        void removeTextureDefinition(size_t idx);
        
        /** Get a local texture definition.
        */
        TextureDefinition *getTextureDefinition(size_t idx);
        
		/** Get a local texture definition with a specific name.
		*/
		TextureDefinition *getTextureDefinition(const String& name);

		/** Get the number of local texture definitions.
        */
        size_t getNumTextureDefinitions();
        
        /** Remove all Texture Definitions
        */
        void removeAllTextureDefinitions();
        
        /** Get an iterator over the TextureDefinitions in this Technique. */
        TextureDefinitionIterator getTextureDefinitionIterator(void);
        
        /** Create a new target pass, and return a pointer to it.
        */
        CompositionTargetPass *createTargetPass();
        
        /** Remove a target pass. It will also be destroyed.
        */
        void removeTargetPass(size_t idx);
        
        /** Get a target pass.
        */
        CompositionTargetPass *getTargetPass(size_t idx);
        
        /** Get the number of target passes.
        */
        size_t getNumTargetPasses();
        
        /** Remove all target passes.
        */
        void removeAllTargetPasses();
        
        /** Get an iterator over the TargetPasses in this Technique. */
        TargetPassIterator getTargetPassIterator(void);
        
        /** Get output (final) target pass
         */
        CompositionTargetPass *getOutputTargetPass();
        
        /** Determine if this technique is supported on the current rendering device. 
		@param allowTextureDegradation True to accept a reduction in texture depth
         */
        virtual bool isSupported(bool allowTextureDegradation);
        
		/** Assign a scheme name to this technique, used to switch between 
			multiple techniques by choice rather than for hardware compatibility.
		*/
		virtual void setSchemeName(const String& schemeName);
		/** Get the scheme name assigned to this technique. */
		const String& getSchemeName() const { return mSchemeName; }
        
        /** Get parent object */
        Compositor *getParent();
    private:
        /// Parent compositor
        Compositor *mParent;
        /// Local texture definitions
        TextureDefinitions mTextureDefinitions;
        
        /// Intermediate target passes
        TargetPasses mTargetPasses;
        /// Output target pass (can be only one)
        CompositionTargetPass *mOutputTarget;  

		/// Optional scheme name
		String mSchemeName;

    };
	/** @} */
	/** @} */

}

#endif
