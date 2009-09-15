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