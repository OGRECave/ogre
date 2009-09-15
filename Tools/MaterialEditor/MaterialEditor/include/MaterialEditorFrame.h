/*
-------------------------------------------------------------------------
This source file is a part of OGRE
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
THE SOFTWARE
-------------------------------------------------------------------------
*/
#ifndef _MATERIALEDITORFRAME_H_
#define _MATERIALEDITORFRAME_H_

#include <wx/wx.h>

#include "OgreRenderSystem.h"
#include "OgreRoot.h"

class wxAuiManager;
class wxAuiNotebook;
class wxNotebook;
class wxPropertyGridManager;
class wxTreeCtrl;

namespace
{
	class RenderSystem;
	class Root;
}

class DocPanel;
class EventArgs;
class LogPanel;
class PropertiesPanel;
class ResourcePanel;
class WorkspacePanel;
class wxOgre;

class MaterialEditorFrame : public wxFrame
{
public:
	MaterialEditorFrame(wxWindow* parent = NULL);
	~MaterialEditorFrame();

protected:
	void createAuiManager(void);
	void createAuiNotebookPane(void);
	void createManagementPane(void);
	void createInformationPane(void);
	void createPropertiesPane();
	void createOgrePane(void);

	void createMenuBar(void);
	void createFileMenu(void);
	void createEditMenu(void);
	void createViewMenu(void);
	void createToolsMenu(void);
	void createWindowMenu(void);
	void createHelpMenu(void);

	void OnActivate(wxActivateEvent& event);

	void OnActiveEditorChanged(EventArgs& args);

	void OnNewProject(wxCommandEvent& event);
	void OnNewMaterial(wxCommandEvent& event);
	void OnFileOpen(wxCommandEvent& event);
	void OnFileSave(wxCommandEvent& event);
	void OnFileSaveAs(wxCommandEvent& event);
	void OnFileClose(wxCommandEvent& event);
	void OnFileExit(wxCommandEvent& event);
	void OnEditUndo(wxCommandEvent& event);
	void OnEditRedo(wxCommandEvent& event);
	void OnEditCut(wxCommandEvent& event);
	void OnEditCopy(wxCommandEvent& event);
	void OnEditPaste(wxCommandEvent& event);
	void OnViewOpenGL(wxCommandEvent& event);
	void OnViewDirectX(wxCommandEvent& event);

private:
	wxMenuBar* mMenuBar;
	wxMenu* mFileMenu;
	wxMenu* mEditMenu;
	wxMenu* mViewMenu;
	wxMenu* mToolsMenu;
	wxMenu* mWindowMenu;
	wxMenu* mHelpMenu;

	wxAuiManager* mAuiManager;

	wxAuiNotebook* mAuiNotebook;
	wxAuiNotebook* mManagementNotebook;
	wxAuiNotebook* mInformationNotebook;
	WorkspacePanel* mWorkspacePanel;
	ResourcePanel* mResourcePanel;
	PropertiesPanel* mPropertiesPanel;

	Ogre::Root* mRoot;
	Ogre::Entity* mEntity;
	
	LogPanel* mLogPanel;
	DocPanel* mDocPanel;
	wxOgre* mOgreControl;

	Ogre::RenderSystem* mDirectXRenderSystem;
	Ogre::RenderSystem* mOpenGLRenderSystem;

	DECLARE_EVENT_TABLE();
};

#endif // _MATERIALEDITORFRAME_H_