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
