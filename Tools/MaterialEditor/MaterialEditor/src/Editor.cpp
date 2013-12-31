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
#include "Editor.h"

#include <wx/control.h>

#include "EditorEventArgs.h"
#include "EditorInput.h"

Editor::Editor()
: mEditorInput(NULL), mName("Editor")
{
	registerEvents();
}

Editor::Editor(EditorInput* input)
: mEditorInput(input)
{
	registerEvents();
}

Editor::~Editor()
{
}

wxControl* Editor::getControl() const
{
	return mControl;
}

void Editor::setControl(wxControl* control)
{
	mControl = control;
}

void Editor::registerEvents()
{
	registerEvent(NameChanged);
	registerEvent(DirtyStateChanged);
}


EditorInput* Editor::getEditorInput() const
{
	return mEditorInput;
}

void Editor::setEditorInput(EditorInput* input)
{
	mEditorInput = input;
}

EditorContributor* Editor::getEditorContributor() const
{
	return NULL;
}

const wxString& Editor::getName() const
{
	return mName;
}

void Editor::setName(const wxString& name)
{
	mName = name;

	fireEvent(NameChanged, EditorEventArgs(this));
}

void Editor::activate()
{
	// Do nothing
}

void Editor::deactivate()
{
	// Do nothing
}

bool Editor::isDirty()
{
	return false;
}

void Editor::save()
{
	// Do nothing
}

void Editor::saveAs()
{
	// Do nothing
}

bool Editor::isSaveAsAllowed()
{
	return false;
}

bool Editor::isRedoable()
{
	return false;
}

void Editor::redo()
{
	// Do nothing
}

bool Editor::isUndoable()
{
	return false;
}

void Editor::undo()
{
	// Do nothing
}

bool Editor::isCuttable()
{
	return false;
}

void Editor::cut()
{
	// Do nothing
}

bool Editor::isCopyable()
{
	return false;
}

void Editor::copy()
{
	// Do nothing
}

bool Editor::isPastable()
{
	return false;
}

void Editor::paste()
{
	// Do nothing
}
