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
#ifndef __CompositionTechnique_H__
#define __CompositionTechnique_H__

#include "OgrePrerequisites.h"
#include "OgrePixelFormat.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
    template <typename T> class VectorIterator;

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
    
        //The scope of a texture defined by the compositor
        enum TextureScope { 
            //Local texture - only available to the compositor passes in this technique
            TS_LOCAL, 
            //Chain texture - available to the other compositors in the chain
            TS_CHAIN, 
            //Global texture - available to everyone in every scope
            TS_GLOBAL 
        };

        /// Local texture definition
        class TextureDefinition : public CompositorInstAlloc
        {
        public:
            String name;
            //Texture definition being a reference is determined by these two fields not being empty.
            String refCompName; //If a reference, the name of the compositor being referenced
            String refTexName;  //If a reference, the name of the texture in the compositor being referenced
            uint32 width;       // 0 means adapt to target width
            uint32 height;      // 0 means adapt to target height
            TextureType type;   // either 2d or cubic
            float widthFactor;  // multiple of target width to use (if width = 0)
            float heightFactor; // multiple of target height to use (if height = 0)
            PixelFormatList formatList; // more than one means MRT
            uint8 fsaa;         // FSAA level; 1 = determine from main target (if render_scene), 0 = disable
            bool hwGammaWrite;  // Do sRGB gamma correction on write (only 8-bit per channel formats) 
            uint16 depthBufferId;//Depth Buffer's pool ID. (unrelated to "pool" variable below)
            bool pooled;        // whether to use pooled textures for this one
            TextureScope scope; // Which scope has access to this texture

            TextureDefinition() :width(0), height(0), type(TEX_TYPE_2D), widthFactor(1.0f), heightFactor(1.0f),
                fsaa(1), hwGammaWrite(false), depthBufferId(1), pooled(false), scope(TS_LOCAL) {}
        };
        /// Typedefs for several iterators
        typedef std::vector<CompositionTargetPass *> TargetPasses;
        typedef VectorIterator<TargetPasses> TargetPassIterator;
        typedef std::vector<TextureDefinition*> TextureDefinitions;
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
        TextureDefinition *getTextureDefinition(size_t idx) const { return mTextureDefinitions.at(idx); }
        
        /** Get a local texture definition with a specific name.
        */
        TextureDefinition *getTextureDefinition(const String& name) const;

        /** Get the number of local texture definitions.*/
        size_t getNumTextureDefinitions() const { return mTextureDefinitions.size(); }
        
        /** Remove all Texture Definitions
        */
        void removeAllTextureDefinitions();
        
        /** Get the TextureDefinitions in this Technique. */
        const TextureDefinitions& getTextureDefinitions() const { return mTextureDefinitions; }

        /// @deprecated use getTextureDefinitions()
        OGRE_DEPRECATED TextureDefinitionIterator getTextureDefinitionIterator(void);
        
        /** Create a new target pass, and return a pointer to it.
        */
        CompositionTargetPass *createTargetPass();
        
        /** Remove a target pass. It will also be destroyed.
        */
        void removeTargetPass(size_t idx);
        
        /** Get a target pass.
        */
        CompositionTargetPass* getTargetPass(size_t idx) const { return mTargetPasses.at(idx); }
        
        /** Get the number of target passes. */
        size_t getNumTargetPasses() const { return mTargetPasses.size(); }
        
        /** Remove all target passes.
        */
        void removeAllTargetPasses();
        
        /** Get the TargetPasses in this Technique. */
        const TargetPasses& getTargetPasses() const { return mTargetPasses; }

        /// @deprecated use getTargetPasses()
        OGRE_DEPRECATED TargetPassIterator getTargetPassIterator(void);
        
        /** Get output (final) target pass
         */
        CompositionTargetPass *getOutputTargetPass() const { return mOutputTarget; }
        
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
        
        /** Set the name of the compositor logic assigned to this technique.
            Instances of this technique will be auto-coupled with the matching logic.
        */
        void setCompositorLogicName(const String& compositorLogicName) 
            { mCompositorLogicName = compositorLogicName; }
        /** Get the compositor logic name assigned to this technique */
        const String& getCompositorLogicName() const { return mCompositorLogicName; }

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
        
        /// Optional compositor logic name
        String mCompositorLogicName;

    };
    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
