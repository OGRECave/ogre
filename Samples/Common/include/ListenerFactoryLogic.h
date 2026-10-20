/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
conditions of the standard open source license.
-----------------------------------------------------------------------------
*/

#ifndef _LISTENER_FACTORY_LOGIC_H
#define _LISTENER_FACTORY_LOGIC_H

#include "OgreCompositorInstance.h"
#include "OgreCompositorLogic.h"
#include <map>
#include <memory>

//The simple types of compositor logics will all do the same thing -
//Attach a listener to the created compositor
class ListenerFactoryLogic : public Ogre::CompositorLogic
{
public:
    void compositorInstanceCreated(Ogre::CompositorInstance* newInstance) override
    {
        auto* listener = createListener(newInstance);
        newInstance->addListener(listener);
        mListeners[newInstance].reset(listener);
    }

    void compositorInstanceDestroyed(Ogre::CompositorInstance* destroyedInstance) override
    {
        auto it = mListeners.find(destroyedInstance);
        if (it == mListeners.end())
            return;

        destroyedInstance->removeListener(it->second.get());
        mListeners.erase(it);
    }

protected:
    //This is the method that implementers will need to create
    virtual Ogre::CompositorInstance::Listener* createListener(Ogre::CompositorInstance* instance) = 0;
private:
    typedef std::map<Ogre::CompositorInstance*, std::unique_ptr<Ogre::CompositorInstance::Listener>> ListenerMap;
    ListenerMap mListeners;

};

#endif
