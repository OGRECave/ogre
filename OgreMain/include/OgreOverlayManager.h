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
#ifndef __OverlayManager_H__
#define __OverlayManager_H__

#include "OgrePrerequisites.h"
#include "OgreSingleton.h"
#include "OgreStringVector.h"
#include "OgreOverlay.h"
#include "OgreScriptLoader.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Overlays
	*  @{
	*/
	/** Manages Overlay objects, parsing them from .overlay files and
        storing a lookup library of them. Alo manages the creation of 
		OverlayContainers and OverlayElements, used for non-interactive 2D 
		elements such as HUDs.
    */
    class _OgreExport OverlayManager : public Singleton<OverlayManager>, public ScriptLoader, public OverlayAlloc
    {
    public:
        typedef map<String, Overlay*>::type OverlayMap;
		typedef map<String, OverlayElement*>::type ElementMap;
		typedef map<String, OverlayElementFactory*>::type FactoryMap;
    protected:
        OverlayMap mOverlayMap;
        StringVector mScriptPatterns;

        void parseNewElement( DataStreamPtr& chunk, String& elemType, String& elemName, 
            bool isContainer, Overlay* pOverlay, bool isTemplate, String templateName = String(""), OverlayContainer* container = 0);
        void parseAttrib( const String& line, Overlay* pOverlay);
        void parseElementAttrib( const String& line, Overlay* pOverlay, OverlayElement* pElement );
        void skipToNextCloseBrace(DataStreamPtr& chunk);
        void skipToNextOpenBrace(DataStreamPtr& chunk);
        
        int mLastViewportWidth, mLastViewportHeight;
        bool mViewportDimensionsChanged;

	    bool parseChildren( DataStreamPtr& chunk, const String& line,
            Overlay* pOverlay, bool isTemplate, OverlayContainer* parent = NULL);

		FactoryMap mFactories;

		ElementMap mInstances;
		ElementMap mTemplates;

		typedef set<String>::type LoadedScripts;
		LoadedScripts mLoadedScripts;




		ElementMap& getElementMap(bool isTemplate);

		OverlayElement* createOverlayElementImpl(const String& typeName, const String& instanceName, ElementMap& elementMap);

		OverlayElement* getOverlayElementImpl(const String& name, ElementMap& elementMap);
		
		bool hasOverlayElementImpl(const String& name, ElementMap& elementMap);

		void destroyOverlayElementImpl(const String& instanceName, ElementMap& elementMap);

		void destroyOverlayElementImpl(OverlayElement* pInstance, ElementMap& elementMap);

		void destroyAllOverlayElementsImpl(ElementMap& elementMap);

    public:
        OverlayManager();
        virtual ~OverlayManager();

        /// @copydoc ScriptLoader::getScriptPatterns
        const StringVector& getScriptPatterns(void) const;
        /// @copydoc ScriptLoader::parseScript
        void parseScript(DataStreamPtr& stream, const String& groupName);
        /// @copydoc ScriptLoader::getLoadingOrder
        Real getLoadingOrder(void) const;

        /** Create a new Overlay. */
        Overlay* create(const String& name);
        /** Retrieve an Overlay by name 
        @returns A pointer to the Overlay, or 0 if not found
        */
        Overlay* getByName(const String& name);
        /** Destroys an existing overlay by name */
        void destroy(const String& name);
        /** Destroys an existing overlay */
        void destroy(Overlay* overlay);
        /** Destroys all existing overlays */
        void destroyAll(void);
        typedef MapIterator<OverlayMap> OverlayMapIterator;
        OverlayMapIterator getOverlayIterator(void);

        /** Internal method for queueing the visible overlays for rendering. */
        void _queueOverlaysForRendering(Camera* cam, RenderQueue* pQueue, Viewport *vp);

        /** Method for determining if the viewport has changed dimensions. 
        @remarks This is used by pixel-based OverlayElements to work out if they need to
            recalculate their sizes.
        */
        bool hasViewportChanged(void) const;

        /** Gets the height of the destination viewport in pixels. */
        int getViewportHeight(void) const;
        
        /** Gets the width of the destination viewport in pixels. */
        int getViewportWidth(void) const;
        Real getViewportAspectRatio(void) const;


		/** Creates a new OverlayElement of the type requested.
		@remarks
		The type of element to create is passed in as a string because this
		allows plugins to register new types of component.
		@param typeName The type of element to create.
		@param instanceName The name to give the new instance.
		*/
		OverlayElement* createOverlayElement(const String& typeName, const String& instanceName, bool isTemplate = false);

		/** Gets a reference to an existing element. */
		OverlayElement* getOverlayElement(const String& name, bool isTemplate = false);

		/** Tests if an element exists. */
		bool hasOverlayElement(const String& name, bool isTemplate = false);
		
		/** Destroys a OverlayElement. 
		@remarks
		Make sure you're not still using this in an Overlay. If in
		doubt, let OGRE destroy elements on shutdown.
		*/
		void destroyOverlayElement(const String& instanceName, bool isTemplate = false);

		/** Destroys a OverlayElement. 
		@remarks
		Make sure you're not still using this in an Overlay. If in
		doubt, let OGRE destroy elements on shutdown.
		*/
		void destroyOverlayElement(OverlayElement* pInstance, bool isTemplate = false);

		/** Destroys all the OverlayElement  created so far.
		@remarks
		Best to leave this to the engine to call internally, there
		should rarely be a need to call it yourself.
		*/
		void destroyAllOverlayElements(bool isTemplate = false);

		/** Registers a new OverlayElementFactory with this manager.
		@remarks
		Should be used by plugins or other apps wishing to provide
		a new OverlayElement subclass.
		*/
		void addOverlayElementFactory(OverlayElementFactory* elemFactory);
		
		/** Get const access to the list of registered OverlayElement factories. */
		const FactoryMap& getOverlayElementFactoryMap() const {
			return mFactories;
		}

		OverlayElement* createOverlayElementFromTemplate(const String& templateName, const String& typeName, const String& instanceName, bool isTemplate = false);
		/**
		*  @remarks
		*  Creates a new OverlayElement object from the specified template name.  The new
		*  object's name, and all of it's children, will be instanceName/orignalName.
		*/
		OverlayElement* cloneOverlayElementFromTemplate(const String& templateName, const String& instanceName);

		OverlayElement* createOverlayElementFromFactory(const String& typeName, const String& instanceName);

		typedef MapIterator<ElementMap> TemplateIterator;
		/** Returns an iterator over all templates in this manager.*/
		TemplateIterator getTemplateIterator ()
		{
			return TemplateIterator (mTemplates.begin (), mTemplates.end ()) ;
		}
		/* Returns whether the Element with the given name is a Template */
		bool isTemplate (String strName) const {
			return (mTemplates.find (strName) != mTemplates.end()) ;
		}


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
        static OverlayManager& getSingleton(void);
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
        static OverlayManager* getSingletonPtr(void);
    };


	/** @} */
	/** @} */

}


#endif 
