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
#ifndef _PROJECT_H_
#define _PROJECT_H_

#include <list>

#include <boost/signal.hpp>

#include "OgreMaterial.h"
#include "OgreString.h"

#include "EventContainer.h"

namespace Ogre
{
    class SceneManager;
}

class MaterialController;
class Project;
class RootEventPlugin;

using Ogre::Material;
using Ogre::MaterialPtr;
using Ogre::String;

typedef std::list<MaterialController*> MaterialControllerList;

class Project : public EventContainer
{
public:
    enum ProjectEvent
    {
        NameChanged,
        MaterialAdded,
        MaterialRemoved,
        ActiveMaterialChanged
    };

    Project();
    Project(const String& name);
    virtual ~Project();

    void registerEvents();

    const String& getName() const;
    void setName(const String& name);

    void addMaterial(MaterialPtr materialPtr);
    void createMaterial(const String& name);

    void removeMaterial(MaterialController* controller);
    void removeMaterial(Material* material);
    void removeMaterial(const String& name);
    
    MaterialController* getActiveMaterial() const;
    void setActiveMaterial(MaterialController* controller);
    void setActiveMaterial(Material* material);
    void setActiveMaterial(const String& name);
    
    MaterialController* getMaterialController(const String& name);
    
    const MaterialControllerList* getMaterials() const;

    void open();
    void close();

    bool isOpen();
    bool isClosed();

    void generateScene(Ogre::SceneManager* sceneManager);
    
    void OnRootInitialized(EventArgs& args);
    void OnRootShutdown(EventArgs& args);

protected:
    String mName;
    bool mOpen;
    MaterialController* mActiveMaterial;
    MaterialControllerList mMaterialControllers;
    
    void subscribeTo(RootEventPlugin* plugin);
};

#endif // _PROJECT_H_
