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
#include "Project.h"

#include "OgreEntity.h"
#include "OgreLight.h"
#include "OgreMaterial.h"
#include "OgreMaterialManager.h"
#include "OgreSceneManager.h"

#include "MaterialController.h"
#include "ProjectEventArgs.h"

using Ogre::Entity;
using Ogre::Light;
using Ogre::MaterialManager;

Project::Project() : mActiveMaterial(NULL)
{
    registerEvents();
}

Project::Project(const String& name) : mActiveMaterial(NULL), mName(name)
{
    registerEvents();
}

Project::~Project()
{
    MaterialControllerList::iterator it;
    for(it = mMaterialControllers.begin(); it != mMaterialControllers.end(); ++it)
    {
        delete *it;
    }
    
    mMaterialControllers.clear();
}

void Project::registerEvents()
{
    registerEvent(NameChanged);
    registerEvent(MaterialAdded);
    registerEvent(MaterialRemoved);
    registerEvent(ActiveMaterialChanged);
}

const String& Project::getName() const
{
    return mName;
}

void Project::setName(const String& name)
{
    mName = name;
    
    fireEvent(NameChanged, ProjectEventArgs(this));
}

void Project::addMaterial(MaterialPtr materialPtr)
{
    MaterialController* controller = new MaterialController(materialPtr);
    mMaterialControllers.push_back(controller);
    
    fireEvent(MaterialAdded, ProjectEventArgs(this, controller));
}

void Project::createMaterial(const String& name)
{
    // TODO: Projects should probably have their own resource groups instead of using the default
    MaterialPtr materialPtr = (MaterialPtr)MaterialManager::getSingletonPtr()->create(name, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    MaterialController* controller = new MaterialController(materialPtr);
    mMaterialControllers.push_back(controller);

    fireEvent(MaterialAdded, ProjectEventArgs(this, controller));
}

void Project::removeMaterial(MaterialController* controller)
{
    MaterialControllerList::iterator it;
    for(it = mMaterialControllers.begin(); it != mMaterialControllers.end(); ++it)
    {
        if(*it == controller) 
        {
            mMaterialControllers.erase(it);
            break;
        }
    }
    
    // Consider: Should this be fired BEFORE the actual removal?
    fireEvent(MaterialRemoved, ProjectEventArgs(this, controller));
}

void Project::removeMaterial(Material* material)
{
    removeMaterial(getMaterialController(material->getName()));
}

void Project::removeMaterial(const String& name)
{
    removeMaterial(getMaterialController(name));
}

MaterialController* Project::getActiveMaterial() const
{
    return mActiveMaterial;
}

void Project::setActiveMaterial(MaterialController* controller)
{
    assert(controller);
    
    if(controller == mActiveMaterial) return;

    mActiveMaterial = controller;
    
    fireEvent(ActiveMaterialChanged, ProjectEventArgs(this));
}

void Project::setActiveMaterial(Material* material)
{
    setActiveMaterial(getMaterialController(material->getName()));
}

void Project::setActiveMaterial(const String& name)
{
    setActiveMaterial(getMaterialController(name));
}

MaterialController* Project::getMaterialController(const String& name)
{
    MaterialController* mc;
    MaterialControllerList::iterator it;
    for(it = mMaterialControllers.begin(); it != mMaterialControllers.end(); ++it)
    {
        mc = (*it);
        if(mc->getMaterial()->getName() == name) return mc;
    }

    return NULL;
}

const MaterialControllerList* Project::getMaterials() const
{
    return &mMaterialControllers;
}

void Project::open()
{
}

void Project::close()
{
}

void Project::generateScene(Ogre::SceneManager* sceneManager)
{
    sceneManager->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

    Light* light = sceneManager->createLight("MainLight");
    light->setPosition(20,80,50);

    Entity* entity = sceneManager->createEntity("head", "ogrehead.mesh");
    entity->setMaterialName(mActiveMaterial->getMaterial()->getName());

    sceneManager->getRootSceneNode()->createChildSceneNode()->attachObject(entity);
}

void Project::subscribeTo(RootEventPlugin* plugin)
{
}

void Project::OnRootInitialized(EventArgs& args)
{
}

void Project::OnRootShutdown(EventArgs& args)
{
}
