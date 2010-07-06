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
#ifndef _CoreFeature_H__
#define _CoreFeature_H__

#include "OgrePrerequisites.h"


namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup General
	*  @{
	*/


    /** Optional core features can subclass this to automatically register
	    themselves when Root is constructed.
		@remarks
			This is meant for features like image codecs or archive types
			which reside in the core Ogre library, but are not strictly 
			required for operations. Depending on the build setup, their code
			files will be added to the OgreMain library or not. If they are,
			they use this class to specify the code needed to register their
			features with Ogre and then register themselves with the 
			CoreFeatureRegistry via the macro OGRE_REGISTER_CORE_FEATURE.
    */
    class CoreFeature : public GeneralAllocatedObject 
    {
    public:
        virtual ~CoreFeature() {}

		/** Overload this function to specify the steps necessary to register
		  	this core feature.
		*/
		virtual void setup() = 0;
		/** Overload this function to specify the steps necessary to unregister
			this core feature.
		*/
		virtual void shutdown() = 0;
		/** Overload this function to specify the steps necessary to destroy
			resources created for this feature.
		*/
		virtual void destroy() = 0;
    };


	// empty dummy class needed for automatic feature registration
	class CoreFeatureResult {};


	/** Core features can register with this Registry to automatically be
      	setup / shutdown from Root.
      	@remarks 
			This class is designed for internal use only. Don't use directly,
			use the OGRE_REGISTER_CORE_FEATURE macro instead.
	*/
	class CoreFeatureRegistry
	{
	public:
		~CoreFeatureRegistry();

		/** Register a core feature with the registry. */
		CoreFeatureResult registerFeature(CoreFeature* feature, int priority);

		/** Create and setup all registered features. */
		void setupFeatures();

		/** Shutdown all registered features. */
		void shutdownFeatures();

		/** Destroy (free resources) all registered features. */
		void destroyFeatures();

		/** Get access to the Registry. */
		static CoreFeatureRegistry& getSingleton();

	private:
		CoreFeatureRegistry() {}
		CoreFeatureRegistry(const CoreFeatureRegistry& o);
		CoreFeatureRegistry& operator=(const CoreFeatureRegistry& o);

	private:
		vector<std::pair<int, CoreFeature*> >::type mFeatures;
	};

	/** Required hack to deal with OGRE_NEW's usage of the __FUNCTION__ macro. */
	template<class Feature>
	Feature* createFeature()
	{
		return OGRE_NEW Feature;
	}

	/** Call this macro to register a CoreFeature with the registry.
	  	@param
			The feature class to register. Must inherit from CoreFeature.
		@param
			Priority. Higher priority features will be loaded first.
	*/
	#define OGRE_REGISTER_CORE_FEATURE(FeatureClass, priority) \
		namespace { \
			Ogre::CoreFeatureResult _reg_##__LINE__ = \
				Ogre::CoreFeatureRegistry::getSingleton().registerFeature(createFeature<FeatureClass>(), priority); \
		}

	/** @} */
	/** @} */

} // namespace

#endif
