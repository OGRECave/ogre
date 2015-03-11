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
#ifndef __MATERIALMANAGER_H__
#define __MATERIALMANAGER_H__

#include "OgrePrerequisites.h"

#include "OgreSingleton.h"
#include "OgreResourceManager.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

    class MaterialSerializer;


    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Materials
    *  @{
    */
    /** Class for managing Material settings for Ogre.
        @remarks
            Materials control the eventual surface rendering properties of geometry. This class
            manages the library of materials, dealing with programmatic registrations and lookups,
            as well as loading predefined Material settings from scripts.
        @par
            When loaded from a script, a Material is in an 'unloaded' state and only stores the settings
            required. It does not at that stage load any textures. This is because the material settings may be
            loaded 'en masse' from bulk material script files, but only a subset will actually be required.
        @par
            Because this is a subclass of ResourceManager, any files loaded will be searched for in any path or
            archive added to the resource paths/archives. See ResourceManager for details.
        @par
            For a definition of the material script format, see the Tutorials/MaterialScript.html file.
    */
    class _OgreExport MaterialManager : public ResourceManager, public Singleton<MaterialManager>
    {
    public:
        /** Listener on any general material events.
        @see MaterialManager::addListener
        */
        class Listener
        {
        public:
            /** Virtual destructor needed as class has virtual methods. */
            virtual ~Listener() { }
            /** Called if a technique for a given scheme is not found within a material,
                allows the application to specify a Technique instance manually.
            @remarks
                Material schemes allow you to switch wholesale between families of 
                techniques on a material. However they require you to define those
                schemes on the materials up-front, which might not be possible or
                desirable for all materials, particular if, for example, you wanted
                a simple way to replace all materials with another using a scheme.
            @par
                This callback allows you to handle the case where a scheme is requested
                but the material doesn't have an entry for it. You can return a
                Technique pointer from this method to specify the material technique
                you'd like to be applied instead, which can be from another material
                entirely (and probably will be). Note that it is critical that you
                only return a Technique that is supported on this hardware; there are
                utility methods like Material::getBestTechnique to help you with this.
            @param schemeIndex The index of the scheme that was requested - all 
                schemes have a unique index when created that does not alter. 
            @param schemeName The friendly name of the scheme being requested
            @param originalMaterial The material that is being processed, that 
                didn't have a specific technique for this scheme
            @param lodIndex The material level-of-detail that was being asked for, 
                in case you need to use it to determine a technique.
            @param rend Pointer to the Renderable that is requesting this technique
                to be used, so this may influence your choice of Technique. May be
                null if the technique isn't being requested in that context.
            @return A pointer to the technique to be used, or NULL if you wish to
                use the default technique for this material
            */
            virtual Technique* handleSchemeNotFound(unsigned short schemeIndex, 
                const String& schemeName, Material* originalMaterial, unsigned short lodIndex, 
                const Renderable* rend) = 0;

			/** Called right after illuminated passes were created,
				so that owner of runtime generated technique can handle this.
			@return True if notification is handled and should not be propagated further.
			*/
			virtual bool afterIlluminationPassesCreated(Technique* technique) { return false; }

			/** Called right before illuminated passes would be removed,
				so that owner of runtime generated technique can handle this.
			@return True if notification is handled and should not be propagated further.
			*/
			virtual bool beforeIlluminationPassesCleared(Technique* technique) { return false; }
        };

    protected:
        /// Serializer - Hold instance per thread if necessary
        OGRE_THREAD_POINTER(MaterialSerializer, mSerializer);
        /// Default settings
        MaterialPtr mDefaultSettings;

        /// Overridden from ResourceManager
        Resource* createImpl(const String& name, ResourceHandle handle, 
            const String& group, bool isManual, ManualResourceLoader* loader,
            const NameValuePairList* params);

        /// Scheme name -> index. Never shrinks! Should be pretty static anyway
        typedef map<String, unsigned short>::type SchemeMap;
        /// List of material schemes
        SchemeMap mSchemes;
        /// Current material scheme
        String mActiveSchemeName;
        /// Current material scheme
        unsigned short mActiveSchemeIndex;

        /// The list of per-scheme (and general) material listeners
        typedef list<Listener*>::type ListenerList;
        typedef map<String, ListenerList>::type ListenerMap;
        ListenerMap mListenerMap;

    public:
        /// Default material scheme
        static String DEFAULT_SCHEME_NAME;

        /// Create a new material
        /// @see ResourceManager::createResource
        MaterialPtr create (const String& name, const String& group,
                            bool isManual = false, ManualResourceLoader* loader = 0,
                            const NameValuePairList* createParams = 0);
        
        /// Get a resource by name
        /// @see ResourceManager::getResourceByName
        MaterialPtr getByName(const String& name, const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

        /** Default constructor.
        */
        MaterialManager();

        /** Default destructor.
        */
        virtual ~MaterialManager();

        /** Initialises the material manager, which also triggers it to 
         * parse all available .program and .material scripts. */
        void initialise(void);
        
        /** @see ScriptLoader::parseScript
        */
        void parseScript(DataStreamPtr& stream, const String& groupName);

        /** Returns a pointer to the default Material settings.
            @remarks
                Ogre comes configured with a set of defaults for newly created
                materials. If you wish to have a different set of defaults,
                simply call this method and change the returned Material's
                settings. All materials created from then on will be configured
                with the new defaults you have specified.
            @par
                The default settings begin as a single Technique with a single, non-programmable Pass:
                <ul>
                <li>ambient = ColourValue::White</li>
                <li>diffuse = ColourValue::White</li>
                <li>specular = ColourValue::Black</li>
                <li>emmissive = ColourValue::Black</li>
                <li>shininess = 0</li>
                <li>No texture unit settings (& hence no textures)</li>
                <li>SourceBlendFactor = SBF_ONE</li>
                <li>DestBlendFactor = SBF_ZERO (no blend, replace with new
                  colour)</li>
                <li>Depth buffer checking on</li>
                <li>Depth buffer writing on</li>
                <li>Depth buffer comparison function = CMPF_LESS_EQUAL</li>
                <li>Colour buffer writing on for all channels</li>
                <li>Culling mode = CULL_CLOCKWISE</li>
                <li>Ambient lighting = ColourValue(0.5, 0.5, 0.5) (mid-grey)</li>
                <li>Dynamic lighting enabled</li>
                <li>Gourad shading mode</li>
                <li>Bilinear texture filtering</li>
                </ul>
        */
        virtual MaterialPtr getDefaultSettings(void) const { return mDefaultSettings; }

        /** Internal method - returns index for a given material scheme name.
        @see Technique::setSchemeName
        */
        virtual unsigned short _getSchemeIndex(const String& name);
        /** Internal method - returns name for a given material scheme index.
        @see Technique::setSchemeName
        */
        virtual const String& _getSchemeName(unsigned short index);
        /** Internal method - returns the active scheme index.
        @see Technique::setSchemeName
        */
        virtual unsigned short _getActiveSchemeIndex(void) const;

        /** Returns the name of the active material scheme. 
        @see Technique::setSchemeName
        */
        virtual const String& getActiveScheme(void) const;
        
        /** Sets the name of the active material scheme. 
        @see Technique::setSchemeName
        */
        virtual void setActiveScheme(const String& schemeName);

        /** 
        Add a listener to handle material events. 
        If schemeName is supplied, the listener will only receive events for that certain scheme.
        */
        virtual void addListener(Listener* l, const Ogre::String& schemeName = BLANKSTRING);

        /** 
        Remove a listener handling material events. 
        If the listener was added with a custom scheme name, it needs to be supplied here as well.
        */
        virtual void removeListener(Listener* l, const Ogre::String& schemeName = BLANKSTRING);

        /// Internal method for sorting out missing technique for a scheme
        virtual Technique* _arbitrateMissingTechniqueForActiveScheme(
            Material* mat, unsigned short lodIndex, const Renderable* rend);

		/// Internal method for sorting out illumination passes for a scheme
		virtual void _notifyAfterIlluminationPassesCreated(Technique* mat);

		/// Internal method for sorting out illumination passes for a scheme
		virtual void _notifyBeforeIlluminationPassesCleared(Technique* mat);


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
        static MaterialManager& getSingleton(void);
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
        static MaterialManager* getSingletonPtr(void);

    };
    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
