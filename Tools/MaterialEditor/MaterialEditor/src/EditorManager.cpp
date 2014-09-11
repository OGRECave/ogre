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
#include "EditorManager.h"

#include <boost/bind.hpp>

#include <wx/aui/auibook.h>

#include "Editor.h"
#include "EditorEventArgs.h"

template<> EditorManager* Ogre::Singleton<EditorManager>::msSingleton = 0;

EditorManager& EditorManager::getSingleton(void)
{  
	assert( msSingleton );  return ( *msSingleton );  
}

EditorManager* EditorManager::getSingletonPtr(void)
{
	return msSingleton;
}

EditorManager::EditorManager(wxAuiNotebook* notebook)
: mEditorNotebook(notebook), mActiveEditor(NULL)
{
	registerEvents();

	if(mEditorNotebook != NULL)
	{
		Connect(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler(EditorManager::OnPageChanged));
		Connect(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE, wxAuiNotebookEventHandler(EditorManager::OnPageClosed));

		mEditorNotebook->PushEventHandler(this);
	}
}

EditorManager::~EditorManager()
{

}

wxAuiNotebook* EditorManager::getEditorNotebook() const
{
	return mEditorNotebook;
}

void EditorManager::setEditorNotebook(wxAuiNotebook* notebook)
{
	if(mEditorNotebook != NULL)
	{
		Disconnect(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler(EditorManager::OnPageChanged));
		Disconnect(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE, wxAuiNotebookEventHandler(EditorManager::OnPageClosed));

		mEditorNotebook->RemoveEventHandler(this);
	}
	
	mEditorNotebook = notebook;
	
	if(mEditorNotebook != NULL)
	{
		Connect(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler(EditorManager::OnPageChanged));
		Connect(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE, wxAuiNotebookEventHandler(EditorManager::OnPageClosed));

		mEditorNotebook->PushEventHandler(this);
	}
}

void EditorManager::openEditor(Editor* editor)
{
	assert(mEditorNotebook != NULL);

	mEditors.push_back(editor);
	mEditorNotebook->AddPage(editor->getControl(), editor->getName(), true);

	mEditorIndexMap[editor] = mEditorNotebook->GetPageIndex(editor->getControl());

	editor->subscribe(Editor::NameChanged, boost::bind(&EditorManager::nameChanged, this, _1));

	setActiveEditor(editor);
}

void EditorManager::closeEditor(Editor* editor)
{
	assert(mEditorNotebook != NULL);

	if(mEditorIndexMap.find(editor) != mEditorIndexMap.end())
	{
		if(mActiveEditor == editor)
		{
			mActiveEditor->deactivate();
			mActiveEditor = NULL;
		}

		int index = mEditorIndexMap[editor];
		mEditorNotebook->RemovePage(index);

		fireEvent(EditorClosed, EditorEventArgs(editor));
	}
}

Editor* EditorManager::findEditor(const wxString& name)
{
	EditorList::iterator it;
	Editor* editor = NULL;
	for(it = mEditors.begin(); it != mEditors.end(); ++it)
	{
		editor = (*it);
		if(editor->getName() == name) return editor;
	}

	return NULL;
}

Editor* EditorManager::getActiveEditor() const
{
	return mActiveEditor;
}

void EditorManager::setActiveEditor(Editor* editor)
{
	if(mActiveEditor == editor) return;

	if(mActiveEditor != NULL) mActiveEditor->deactivate();

	mActiveEditor = editor;

	if(mActiveEditor != NULL) 
	{
		mActiveEditor->activate();

		if(mEditorNotebook != NULL && (mEditorIndexMap.find(mActiveEditor) != mEditorIndexMap.end()))
		{
			mEditorNotebook->SetSelection(mEditorIndexMap[mActiveEditor]);
		}
	}

	fireEvent(ActiveEditorChanged, EditorEventArgs(mActiveEditor));
}

const EditorList& EditorManager::getEditors() const
{
	return mEditors;
}

void EditorManager::registerEvents()
{
	registerEvent(EditorOpened);
	registerEvent(EditorClosed);
	registerEvent(ActiveEditorChanged);
}

void EditorManager::nameChanged(EventArgs& args)
{
	EditorEventArgs eea = dynamic_cast<EditorEventArgs&>(args);
	Editor* editor = eea.getEditor();
	if(mEditorIndexMap.find(editor) != mEditorIndexMap.end())
	{
		int index = mEditorIndexMap[editor];
		mEditorNotebook->SetPageText(index, editor->getName());
	}
}

void EditorManager::OnPageChanged(wxAuiNotebookEvent& event)
{
	int index = event.GetSelection();

	if(mActiveEditor != NULL) 
	{
		if(mEditorIndexMap.find(mActiveEditor) != mEditorIndexMap.end())
		{
			int oldIndex = mEditorIndexMap[mActiveEditor];

			if(index != oldIndex)
			{
				mActiveEditor->deactivate();

				EditorIndexMap::iterator it;
				for(it = mEditorIndexMap.begin(); it != mEditorIndexMap.end(); ++it)
				{
					if(it->second == index)
					{
						setActiveEditor(it->first);
						break;
					}
				}
			}
		}
	}
	else
	{
		EditorIndexMap::iterator it;
		for(it = mEditorIndexMap.begin(); it != mEditorIndexMap.end(); ++it)
		{
			if(it->second == index)
			{
				setActiveEditor(it->first);
			}
		}
	}
}

void EditorManager::OnPageClosed(wxAuiNotebookEvent& event)
{
	int index = event.GetSelection();

	Editor* editor = NULL;
	EditorIndexMap::iterator it;
	for(it = mEditorIndexMap.begin(); it != mEditorIndexMap.end(); ++it)
	{
		if(it->second == index)
		{
			editor = it->first;
			editor->deactivate();
			mEditorIndexMap.erase(it);
			break;
		}
	}

	if(editor != NULL)
	{
		if(editor == mActiveEditor)
			mActiveEditor = NULL;

		EditorList::iterator lit;
		for(lit = mEditors.begin(); lit != mEditors.end(); ++lit)
		{
			if((*lit) == editor)
			{
				mEditors.erase(lit);
				break;
			}
		}	
	}

	fireEvent(EditorClosed, EditorEventArgs(editor));

	// Is this handled by OnPageChanged?
	int selIndex = event.GetSelection();
	for(it = mEditorIndexMap.begin(); it != mEditorIndexMap.end(); ++it)
	{
		if(it->second == index)
		{
			setActiveEditor(it->first);
			break;
		}
	}
}

