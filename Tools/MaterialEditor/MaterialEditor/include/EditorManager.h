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
#ifndef _EDITORMANAGER_H_
#define _EDITORMANAGER_H_

#include <list>
#include <map>

#include <wx/event.h>

#include "OgreSingleton.h"

#include "EventContainer.h"

class wxAuiNotebook;
class wxAuiNotebookEvent;

class Editor;
class EditorInput;
class EventArgs;
class Project;
class Workspace;

typedef std::list<Editor*> EditorList;
typedef std::map<Editor*, int> EditorIndexMap;

using Ogre::String;

class EditorManager : public wxEvtHandler, public Ogre::Singleton<EditorManager>, public EventContainer
{
public:
	enum EditorManagerEvent
	{
		EditorOpened,
		EditorClosed,
		ActiveEditorChanged
	};

	EditorManager(wxAuiNotebook* notebook);
	virtual ~EditorManager();

	wxAuiNotebook* getEditorNotebook() const;
	void setEditorNotebook(wxAuiNotebook* notebook);
	
	void openEditor(Editor* editor);
	//void openEditor(EditorInput* input);
	void closeEditor(Editor* editor);
	//void closeEditor(EditorInput* input);
	Editor* findEditor(const wxString& name);

	Editor* getActiveEditor() const;
	void setActiveEditor(Editor* editor);

	const EditorList& getEditors() const;

	void nameChanged(EventArgs& args);

	void OnPageChanged(wxAuiNotebookEvent& event);
	void OnPageClosed(wxAuiNotebookEvent& event);

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
	static EditorManager& getSingleton(void);

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
	static EditorManager* getSingletonPtr(void);

protected:
	void registerEvents();

	EditorList mEditors;
	EditorIndexMap mEditorIndexMap;
	Editor* mActiveEditor;
	wxAuiNotebook* mEditorNotebook;
};

#endif // _EDITORMANAGER_H_
