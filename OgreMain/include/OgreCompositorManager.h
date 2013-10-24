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
#ifndef __CompositorManager_H__
#define __CompositorManager_H__

#include "OgrePrerequisites.h"
#include "OgreResourceManager.h"
#include "OgreCompositor.h"
#include "OgreRectangle2D.h"
#include "OgreRenderSystem.h"
#include "OgreCompositionTechnique.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
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

        /// Overridden from ResourceManager
        Resource* createImpl(const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader,
            const NameValuePairList* params);

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
		CompositorPtr getByName(const String& name, const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);


        /** @see ScriptLoader::parseScript
        */
        void parseScript(DataStreamPtr& stream, const String& groupName);

        /** Get the compositor chain for a Viewport. If there is none yet, a new
			compositor chain is registered.
			XXX We need a _notifyViewportRemoved to find out when this viewport disappears,
			so we can destroy its chain as well.
        */
        CompositorChain *getCompositorChain(Viewport *vp);

		/** Returns whether exists compositor chain for a viewport.
        */
		bool hasCompositorChain(Viewport *vp) const;

		/** Remove the compositor chain from a viewport if exists.
		*/
        void removeCompositorChain(Viewport *vp);

		/** Add a compositor to a viewport. By default, it is added to end of the chain,
			after the other compositors.
			@param vp			Viewport to modify
			@param compositor	The name of the compositor to apply
			@param addPosition	At which position to add, defaults to the end (-1).
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

		typedef set<Texture*>::type UniqueTextureSet;

		/** Utility function to get an existing pooled texture matching a given
			definition, or creating one if one doesn't exist. It also takes into
			account whether a pooled texture has already been supplied to this
			same requester already, in which case it won't give the same texture
			twice (this is important for example if you request 2 ping-pong textures, 
			you don't want to get the same texture for both requests!
		*/
		TexturePtr getPooledTexture(const String& name, const String& localName, 
			size_t w, size_t h, 
			PixelFormat f, uint aa, const String& aaHint, bool srgb, UniqueTextureSet& texturesAlreadyAssigned, 
			CompositorInstance* inst, CompositionTechnique::TextureScope scope);

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

		/** Register a custom composition pass.
		*/
		void registerCustomCompositionPass(const String& name, CustomCompositionPass* customPass);
		
		/** Get a custom composition pass by its name 
		*/
		CustomCompositionPass* getCustomCompositionPass(const String& name);

		/** Override standard Singleton retrieval.
		@remarks
		Why do we do this? Well, it's because the Singleton
		implementation is in a .h file, which means it gets compiled
		into anybody who includes it. This is needed for the
		Singleton template to work, but we actually only want it
		compiled into the implementation of the class based on the
		Singleton, not all of them. If we don't change this, we get
		link errors when trying to use the Singleton-based class from
		an outside dll.
		@par
		This method just delegates to the template version anyway,
		but the implementation stays in this single compilation unit,
		preventing link errors.
		*/
		static CompositorManager& getSingleton(void);
		/** Override standard Singleton retrieval.
		@remarks
		Why do we do this? Well, it's because the Singleton
		implementation is in a .h file, which means it gets compiled
		into anybody who includes it. This is needed for the
		Singleton template to work, but we actually only want it
		compiled into the implementation of the class based on the
		Singleton, not all of them. If we don't change this, we get
		link errors when trying to use the Singleton-based class from
		an outside dll.
		@par
		This method just delegates to the template version anyway,
		but the implementation stays in this single compilation unit,
		preventing link errors.
		*/
		static CompositorManager* getSingletonPtr(void);

	
	private:
        typedef map<Viewport*, CompositorChain*>::type Chains;
        Chains mChains;

        /** Clear composition chains for all viewports
         */
        void freeChains();

		Rectangle2D *mRectangle;

		/// List of instances
		typedef vector<CompositorInstance *>::type Instances;
		Instances mInstances;

		/// Map of registered compositor logics
		typedef map<String, CompositorLogic*>::type CompositorLogicMap;
		CompositorLogicMap mCompositorLogics;

		/// Map of registered custom composition passes
		typedef map<String, CustomCompositionPass*>::type CustomCompositionPassMap;
		CustomCompositionPassMap mCustomCompositionPasses;

		typedef vector<TexturePtr>::type TextureList;
		typedef VectorIterator<TextureList> TextureIterator;

		struct TextureDef
		{
			size_t width, height;
			PixelFormat format;
			uint fsaa;
			String fsaaHint;
			bool sRGBwrite;

			TextureDef(size_t w, size_t h, PixelFormat f, uint aa, const String& aaHint, bool srgb)
				: width(w), height(h), format(f), fsaa(aa), fsaaHint(aaHint), sRGBwrite(srgb)
			{

			}
		};
		struct TextureDefLess
		{
			bool _OgreExport operator()(const TextureDef& x, const TextureDef& y) const
			{
				if (x.format < y.format)
					return true;
				else if (x.format == y.format)
				{
					if (x.width < y.width)
						return true;
					else if (x.width == y.width)
					{
						if (x.height < y.height)
							return true;
						else if (x.height == y.height)
						{
							if (x.fsaa < y.fsaa)
								return true;
							else if (x.fsaa == y.fsaa)
							{
								if (x.fsaaHint < y.fsaaHint)
									return true;
								else if (x.fsaaHint == y.fsaaHint)
								{
									if (!x.sRGBwrite && y.sRGBwrite)
										return true;
								}

							}
						}
					}
				}
				return false;
			}
			virtual ~TextureDefLess() {}
		};
		typedef map<TextureDef, TextureList*, TextureDefLess>::type TexturesByDef;
		TexturesByDef mTexturesByDef;

		typedef std::pair<String, String> StringPair;
		typedef map<TextureDef, TexturePtr, TextureDefLess>::type TextureDefMap;
		typedef map<StringPair, TextureDefMap>::type ChainTexturesByDef;
		
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
