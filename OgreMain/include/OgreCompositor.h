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
#ifndef __Compositor_H__
#define __Compositor_H__

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
    /** Class representing a Compositor object. Compositors provide the means 
        to flexibly "composite" the final rendering result from multiple scene renders
        and intermediate operations like rendering fullscreen quads. This makes 
        it possible to apply postfilter effects, HDRI postprocessing, and shadow 
        effects to a Viewport.
     */
    class _OgreExport Compositor: public Resource
    {
    public:
        Compositor(ResourceManager* creator, const String& name, ResourceHandle handle,
            const String& group, bool isManual = false, ManualResourceLoader* loader = 0);
        ~Compositor();
        
        /// Data types for internal lists
        typedef std::vector<CompositionTechnique *> Techniques;
        typedef VectorIterator<Techniques> TechniqueIterator;
        
        /** Create a new technique, and return a pointer to it.
        */
        CompositionTechnique *createTechnique();
        
        /** Remove a technique. It will also be destroyed.
        */
        void removeTechnique(size_t idx);
        
        /** Get a technique.
        */
        CompositionTechnique *getTechnique(size_t idx);
        
        /** Get the number of techniques.
        */
        size_t getNumTechniques();
        
        /** Remove all techniques
        */
        void removeAllTechniques();
        
        /** Get an iterator over the Techniques in this compositor. */
        TechniqueIterator getTechniqueIterator(void);
        
        /** Get a supported technique.
        @remarks
            The supported technique list is only available after this compositor has been compiled,
            which typically happens on loading it. Therefore, if this method returns
            an empty list, try calling Compositor::load.
        */
        CompositionTechnique *getSupportedTechnique(size_t idx);
        
        /** Get the number of supported techniques.
        @remarks
            The supported technique list is only available after this compositor has been compiled,
            which typically happens on loading it. Therefore, if this method returns
            an empty list, try calling Compositor::load.
        */
        size_t getNumSupportedTechniques();
        
        /** Gets an iterator over all the Techniques which are supported by the current card. 
        @remarks
            The supported technique list is only available after this compositor has been compiled,
            which typically happens on loading it. Therefore, if this method returns
            an empty list, try calling Compositor::load.
        */
        TechniqueIterator getSupportedTechniqueIterator(void);

        /** Get a pointer to a supported technique for a given scheme. 
        @remarks
            If there is no specific supported technique with this scheme name, 
            then the first supported technique with no specific scheme will be returned.
        @param schemeName The scheme name you are looking for. Blank means to 
            look for techniques with no scheme associated
        */
        CompositionTechnique *getSupportedTechnique(const String& schemeName = BLANKSTRING);

        /** Get the instance name for a global texture.
        @param name The name of the texture in the original compositor definition
        @param mrtIndex If name identifies a MRT, which texture attachment to retrieve
        @return The instance name for the texture, corresponds to a real texture
        */
        const String& getTextureInstanceName(const String& name, size_t mrtIndex);

        /** Get the instance of a global texture.
        @param name The name of the texture in the original compositor definition
        @param mrtIndex If name identifies a MRT, which texture attachment to retrieve
        @return The texture pointer, corresponds to a real texture
        */
        TexturePtr getTextureInstance(const String& name, size_t mrtIndex);

        /** Get the render target for a given render texture name. 
        @remarks
            You can use this to add listeners etc, but do not use it to update the
            targets manually or any other modifications, the compositor instance 
            is in charge of this.
        */
        RenderTarget* getRenderTarget(const String& name);

    protected:
        /// @copydoc Resource::loadImpl
        void loadImpl(void);

        /// @copydoc Resource::unloadImpl
        void unloadImpl(void);
        /// @copydoc Resource::calculateSize
        size_t calculateSize(void) const;
        
        /** Check supportedness of techniques.
         */
        void compile();
    private:
        Techniques mTechniques;
        Techniques mSupportedTechniques;
        
        /// Compilation required
        /// This is set if the techniques change and the supportedness of techniques has to be
        /// re-evaluated.
        bool mCompilationRequired;

        /** Create global rendertextures.
        */
        void createGlobalTextures();
        
        /** Destroy global rendertextures.
        */
        void freeGlobalTextures();

        //TODO GSOC : These typedefs are duplicated from CompositorInstance. Solve?
        /// Map from name->local texture
        typedef std::map<String,TexturePtr> GlobalTextureMap;
        GlobalTextureMap mGlobalTextures;
        /// Store a list of MRTs we've created
        typedef std::map<String,MultiRenderTarget*> GlobalMRTMap;
        GlobalMRTMap mGlobalMRTs;
    };
    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
