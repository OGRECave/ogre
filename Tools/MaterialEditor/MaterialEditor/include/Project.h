/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
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
