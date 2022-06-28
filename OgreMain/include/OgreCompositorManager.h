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
#ifndef __CompositorManager_H__
#define __CompositorManager_H__

#include "OgrePrerequisites.h"
#include "OgreResourceManager.h"
#include "OgreRenderSystem.h"
#include "OgreCompositionTechnique.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

    class Rectangle2D;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */
    /** Class for managing Compositor settings for Ogre. Compositors provide the means
        to flexibly "composite" the final rendering result from multiple scene renders
        and intermediate operations like rendering fullscreen quads. This makes
        it possible to apply postfilter effects, HDRI postprocessing, and shadow
        effects to a Viewport.
        @par
            When loaded from a script, a Compositor is in an 'unloaded' state and only stores the settings
            required. It does not at that stage load any textures. This is because the material settings may be
            loaded 'en masse' from bulk material script files, but only a subset will actually be required.
        @par
            Because this is a subclass of ResourceManager, any files loaded will be searched for in any path or
            archive added to the resource paths/archives. See ResourceManager for details.
    */
    class _OgreExport CompositorManager : public ResourceManager, public Singleton<CompositorManager>
    {
    public:
        CompositorManager();
        virtual ~CompositorManager();

        /** Initialises the Compositor manager, which also triggers it to
            parse all available .compositor scripts. */
        void initialise(void);

        /**
         * Create a new compositor
         * @see ResourceManager::createResource
         */
        CompositorPtr create (const String& name, const String& group,
                            bool isManual = false, ManualResourceLoader* loader = 0,
                            const NameValuePairList* createParams = 0);

        /// Get a resource by name
        /// @see ResourceManager::getResourceByName
        CompositorPtr getByName(const String& name, const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME) const;

        /** Get the compositor chain for a Viewport. If there is none yet, a new
            compositor chain is registered.
            XXX We need a _notifyViewportRemoved to find out when this viewport disappears,
            so we can destroy its chain as well.
        */
        CompositorChain *getCompositorChain(Viewport *vp);

        /** Returns whether exists compositor chain for a viewport.
        */
        bool hasCompositorChain(const Viewport *vp) const;

        /** Remove the compositor chain from a viewport if exists.
        */
        void removeCompositorChain(const Viewport *vp);

        /** Add a compositor to a viewport. By default, it is added to end of the chain,
            after the other compositors.
            @param vp           Viewport to modify
            @param compositor   The name of the compositor to apply
            @param addPosition  At which position to add, defaults to the end (-1).
            @return pointer to instance, or 0 if it failed.
        */
        CompositorInstance *addCompositor(Viewport *vp, const String &compositor, int addPosition=-1);

        /** Remove a compositor from a viewport
        */
        void removeCompositor(Viewport *vp, const String &compositor);

        /** Set the state of a compositor on a viewport to enabled or disabled.
            Disabling a compositor stops it from rendering but does not free any resources.
            This can be more efficient than using removeCompositor and addCompositor in cases
            the filter is switched on and off a lot.
        */
        void setCompositorEnabled(Viewport *vp, const String &compositor, bool value);

        /** Get a textured fullscreen 2D rectangle, for internal use.
        */
        Renderable *_getTexturedRectangle2D();

        /** Overridden from ResourceManager since we have to clean up chains too. */
        void removeAll(void);

        /** Internal method for forcing all active compositors to recreate their resources. */
        void _reconstructAllCompositorResources();

        typedef std::set<Texture*> UniqueTextureSet;

        /** Utility function to get an existing pooled texture matching a given
            definition, or creating one if one doesn't exist. It also takes into
            account whether a pooled texture has already been supplied to this
            same requester already, in which case it won't give the same texture
            twice (this is important for example if you request 2 ping-pong textures, 
            you don't want to get the same texture for both requests!
        */
        TexturePtr getPooledTexture(const String& name, const String& localName, 
            uint32 w, uint32 h,
            PixelFormat f, uint aa, const String& aaHint, bool srgb, UniqueTextureSet& texturesAlreadyAssigned, 
            CompositorInstance* inst, CompositionTechnique::TextureScope scope, TextureType type = TEX_TYPE_2D);

        /** Free pooled textures from the shared pool (compositor instances still 
            using them will keep them in memory though). 
        */
        void freePooledTextures(bool onlyIfUnreferenced = true);

        /** Register a compositor logic for listening in to expecting composition
            techniques.
        */
        void registerCompositorLogic(const String& name, CompositorLogic* logic);

        /** Removes a listener for compositor logic registered with registerCompositorLogic
        */
        void unregisterCompositorLogic(const String& name);
        
        /** Get a compositor logic by its name
        */
        CompositorLogic* getCompositorLogic(const String& name);

		/** Check if a compositor logic exists
		*/
		bool hasCompositorLogic(const String& name);
		
        /** Register a custom composition pass.
        */
        void registerCustomCompositionPass(const String& name, CustomCompositionPass* customPass);

        void unregisterCustomCompositionPass(const String& name);

        /** Get a custom composition pass by its name 
        */
        CustomCompositionPass* getCustomCompositionPass(const String& name);

		/** Check if a compositor pass exists
		*/
        bool hasCustomCompositionPass(const String& name);

        /**
        Relocates a compositor chain from one viewport to another
        @param sourceVP The viewport to take the chain from
        @param destVP The viewport to connect the chain to
        */
        void _relocateChain(Viewport* sourceVP, Viewport* destVP);

        /// @copydoc Singleton::getSingleton()
        static CompositorManager& getSingleton(void);

        /// @copydoc Singleton::getSingleton()
        static CompositorManager* getSingletonPtr(void);
    
    private:
        Resource* createImpl(const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader,
            const NameValuePairList* params) override;

        typedef std::map<const Viewport*, CompositorChain*> Chains;
        Chains mChains;

        /** Clear composition chains for all viewports
         */
        void freeChains();

        Rectangle2D *mRectangle;

        /// List of instances
        typedef std::vector<CompositorInstance *> Instances;
        Instances mInstances;

        /// Map of registered compositor logics
        typedef std::map<String, CompositorLogic*> CompositorLogicMap;
        CompositorLogicMap mCompositorLogics;

        /// Map of registered custom composition passes
        typedef std::map<String, CustomCompositionPass*> CustomCompositionPassMap;
        CustomCompositionPassMap mCustomCompositionPasses;

        typedef std::vector<TexturePtr> TextureList;
        typedef VectorIterator<TextureList> TextureIterator;

        struct TextureDef
        {
            size_t width, height;
            TextureType type;
            PixelFormat format;
            uint fsaa;
            String fsaaHint;
            bool sRGBwrite;

            TextureDef(size_t w, size_t h, TextureType t, PixelFormat f, uint aa, const String& aaHint,
                       bool srgb)
                : width(w), height(h), type(t), format(f), fsaa(aa), fsaaHint(aaHint), sRGBwrite(srgb)
            {
            }

            bool operator<(const TextureDef& y) const
            {
                return std::tie(width, height, type, format, fsaa, fsaaHint, sRGBwrite) <
                       std::tie(y.width, y.height, y.type, y.format, y.fsaa, y.fsaaHint, y.sRGBwrite);
            }
        };
        typedef std::map<TextureDef, TextureList> TexturesByDef;
        TexturesByDef mTexturesByDef;

        typedef std::pair<String, String> StringPair;
        typedef std::map<TextureDef, TexturePtr> TextureDefMap;
        typedef std::map<StringPair, TextureDefMap> ChainTexturesByDef;
        
        ChainTexturesByDef mChainTexturesByDef;

        bool isInputPreviousTarget(CompositorInstance* inst, const Ogre::String& localName);
        bool isInputPreviousTarget(CompositorInstance* inst, TexturePtr tex);
        bool isInputToOutputTarget(CompositorInstance* inst, const Ogre::String& localName);
        bool isInputToOutputTarget(CompositorInstance* inst, TexturePtr tex);

    };
    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
