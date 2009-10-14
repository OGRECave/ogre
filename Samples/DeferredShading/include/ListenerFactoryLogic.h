/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/

#ifndef _LISTENER_FACTORY_LOGIC_H
#define _LISTENER_FACTORY_LOGIC_H

#include <OgreCompositorInstance.h>
#include <OgreCompositorLogic.h>
#include <map>

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
	//This is the method that implementers will need to create
	virtual Ogre::CompositorInstance::Listener* createListener(Ogre::CompositorInstance* instance) = 0;
private:
	typedef std::map<Ogre::CompositorInstance*, Ogre::CompositorInstance::Listener*> ListenerMap;
	ListenerMap mListeners;

};

#endif
