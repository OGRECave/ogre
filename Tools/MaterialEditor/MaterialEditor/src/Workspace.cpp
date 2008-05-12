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
#include "Workspace.h"

#include "EventArgs.h"
#include "Project.h"
#include "WorkspaceEventArgs.h"

template<> Workspace* Ogre::Singleton<Workspace>::ms_Singleton = 0;

Workspace& Workspace::getSingleton(void)
{  
	assert( ms_Singleton );  return ( *ms_Singleton );  
}

Workspace* Workspace::getSingletonPtr(void)
{
	return ms_Singleton;
}

Workspace::Workspace()
{
	registerEvents();
}

Workspace::~Workspace()
{
	ProjectList::iterator it;
	for(it = mProjects.begin(); it != mProjects.end(); ++it)
	{
		delete *it;
	}
}

void Workspace::registerEvents()
{
	registerEvent(ProjectAdded);
	registerEvent(ProjectRemoved);
}

void Workspace::addProject(Project* project)
{
	mProjects.push_back(project);
	
	fireEvent(ProjectAdded, WorkspaceEventArgs(this, project));
}

void Workspace::removeProject(Project* project)
{
	mProjects.remove(project);
	fireEvent(ProjectAdded, WorkspaceEventArgs(this, project));
	delete project;
}

void Workspace::removeProject(const String& name)
{
	removeProject(getProject(name));
}

Project* Workspace::getProject(const String& name)
{
	Project* p;
	ProjectList::iterator it;
	for(it = mProjects.begin(); it != mProjects.end(); ++it)
	{
		p = (*it);
		if(p->getName() == name) return p;
	}

	return NULL;
}

const ProjectList* Workspace::getProjects() const
{
	return &mProjects;
}