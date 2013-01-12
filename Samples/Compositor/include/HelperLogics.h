/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
conditions of the standard open source license.
-----------------------------------------------------------------------------
*/

#ifndef _HelperLogics_H__
#define _HelperLogics_H__

//Demo note :
//This file contains three compositor logics for the HDR, Heat Vision and Gaussian Blur compositors.
//If you wish to use these compositors in your application, make sure to add this code as well.
//This code fits the plugin architecture pretty well, but was not exported to a plugin for simplification
//of the SDK package
//@see CompositorDemo::loadResources

#include "OgrePrerequisites.h"
#include "OgreCompositorLogic.h"
#include "OgreCompositorInstance.h"

//The simple types of compositor logics will all do the same thing -
//Attach a listener to the created compositor
class ListenerFactoryLogic : public Ogre::CompositorLogic
{
public:
	/** @copydoc CompositorLogic::compositorInstanceCreated */
	virtual void compositorInstanceCreated(Ogre::CompositorInstance* newInstance) 
	{
		Ogre::CompositorInstance::Listener* listener = createListener(newInstance);
		newInstance->addListener(listener);
		mListeners[newInstance] = listener;
	}
	
	/** @copydoc CompositorLogic::compositorInstanceDestroyed */
	virtual void compositorInstanceDestroyed(Ogre::CompositorInstance* destroyedInstance)
	{
		delete mListeners[destroyedInstance];
		mListeners.erase(destroyedInstance);
	}

protected:
	//This is the method that implementations will need to override
	virtual Ogre::CompositorInstance::Listener* createListener(Ogre::CompositorInstance* instance) = 0;
private:
	typedef std::map<Ogre::CompositorInstance*, Ogre::CompositorInstance::Listener*> ListenerMap;
	ListenerMap mListeners;

};

//The compositor logic for the heat vision compositor
class HeatVisionLogic : public ListenerFactoryLogic
{
protected:
	/** @copydoc ListenerFactoryLogic::createListener */
	virtual Ogre::CompositorInstance::Listener* createListener(Ogre::CompositorInstance* instance);
};

//The compositor logic for the hdr compositor
class HDRLogic : public ListenerFactoryLogic
{
protected:
	/** @copydoc ListenerFactoryLogic::createListener */
	virtual Ogre::CompositorInstance::Listener* createListener(Ogre::CompositorInstance* instance);
};

//The compositor logic for the gaussian blur compositor
class GaussianBlurLogic : public ListenerFactoryLogic
{
protected:
	/** @copydoc ListenerFactoryLogic::createListener */
	virtual Ogre::CompositorInstance::Listener* createListener(Ogre::CompositorInstance* instance);
};

#endif
