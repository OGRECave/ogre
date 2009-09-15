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
#ifndef _EDITOR_H_
#define _EDITOR_H_

#include <wx/string.h>

#include "OgreString.h"

#include "EventArgs.h"
#include "EventContainer.h"

class wxControl;

class EditorContributor;
class EditorInput;

using Ogre::String;

class Editor : public EventContainer
{
public:
	enum EditorEvent
	{
		NameChanged,
		DirtyStateChanged
	};

	Editor();
	Editor(EditorInput* input);
	virtual ~Editor();

	wxControl* getControl() const;
	void setControl(wxControl* control);

	EditorInput* getEditorInput() const;
	void setEditorInput(EditorInput* input);
	
	EditorContributor* getEditorContributor() const;

	const wxString& getName() const;
	void setName(const wxString& name);

	virtual void activate();
	virtual void deactivate();

	virtual bool isDirty();
	virtual void save();
	virtual void saveAs();
	virtual bool isSaveAsAllowed();

	virtual bool isRedoable();
	virtual void redo();
	virtual bool isUndoable();
	virtual void undo();

	virtual bool isCuttable();
	virtual void cut();
	virtual bool isCopyable();
	virtual void copy();
	virtual bool isPastable();
	virtual void paste();

private:
	void registerEvents();

	EditorInput* mEditorInput;
	wxControl* mControl;
	wxString mName;
};

#endif // _EDITOR_H_
