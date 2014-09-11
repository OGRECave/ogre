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
#ifndef _WORKSPACEPANEL_H_
#define _WORKSPACEPANEL_H_

#include <map>

#include <wx/panel.h>
#include <wx/treectrl.h>

class wxBitmapButton;
class wxCommandEvent;
class wxFlexGridSizer;
class wxImageList;
class wxMenu;

class EventArgs;
class MaterialController;
class PassController;
class Project;
class TechniqueController;

typedef std::map<Project*, wxTreeItemId> ProjectIdMap;
typedef std::map<MaterialController*, wxTreeItemId> MaterialIdMap;
typedef std::map<TechniqueController*, wxTreeItemId> TechniqueIdMap;
typedef std::map<PassController*, wxTreeItemId> PassIdMap;

class WorkspacePanel : public wxPanel
{
public:
	WorkspacePanel(wxWindow* parent,
				   wxWindowID id = wxID_ANY,
				   const wxPoint& pos = wxDefaultPosition,
				   const wxSize& size = wxDefaultSize,
				   long style = wxTAB_TRAVERSAL | wxNO_BORDER,
				   const wxString& name = wxT("Workspace Panel"));

	virtual ~WorkspacePanel();

	void OnRightClick(wxTreeEvent& event);
	void OnActivate(wxTreeEvent& event);
	void OnSelectionChanged(wxTreeEvent& event);
	void OnNewProject(wxCommandEvent& event);
	void OnNewMaterialScript(wxCommandEvent& event);
	void OnNewMaterial(wxCommandEvent& event);
	void OnNewTechnique(wxCommandEvent& event);
	void OnNewPass(wxCommandEvent& event);
	void OnAddMaterial(wxCommandEvent& event);
	void OnEdit(wxCommandEvent& event);
	void OnUpdateProjectMenuItem(wxUpdateUIEvent& event);
	void OnUpdateMaterialMenuItem(wxUpdateUIEvent& event);
	void OnUpdateTechniqueMenuItem(wxUpdateUIEvent& event);
	void OnUpdatePassMenuItem(wxUpdateUIEvent& event);

	// Workspace Event Handlers
	void projectAdded(EventArgs& args);
	void projectRemoved(EventArgs& args);

	// Project Event Handlers
	void projectNameChanged(EventArgs& args);
	void projectMaterialAdded(EventArgs& args);
	void projectMaterialRemoved(EventArgs& args);

	// Material Event Handlers
	void materialNameChanged(EventArgs& args);
	void materialTechniqueAdded(EventArgs& args);
	void materialTechniqueRemoved(EventArgs& args);

	// Technique Event Handlers
	void techniqueNameChanged(EventArgs& args);
	void techniquePassAdded(EventArgs& args);
	void techniquePassRemoved(EventArgs& args);

protected:
	void createPanel();
	wxImageList* getImageList();
	
	void appendNewMenu(wxMenu* menu);
	void showContextMenu(wxPoint point, wxTreeItemId id);
	void appendProjectMenuItems(wxMenu* menu);
	void appendMaterialMenuItems(wxMenu* memu);
	void appendTechniqueMenuItems(wxMenu* menu);
	void appendPassMenuItems(wxMenu* menu);

	Project* getProject(wxTreeItemId id);
	MaterialController* getMaterial(wxTreeItemId id);
	TechniqueController* getTechnique(wxTreeItemId id);
	PassController* getPass(wxTreeItemId id);

	bool isWorkspace(wxTreeItemId id);
	bool isProject(wxTreeItemId id);
	bool isMaterial(wxTreeItemId id);
	bool isTechnique(wxTreeItemId id);
	bool isPass(wxTreeItemId id);

	// Event Handling Utils
	void subscribe(Project* project);
	void subscribe(MaterialController* material);
	void subscribe(TechniqueController* technique);

	wxImageList* mImageList;
	wxFlexGridSizer* mSizer;
	wxPanel* mToolBarPanel;
	wxTreeCtrl* mTreeCtrl;

	wxTreeItemId mRootId;

	wxMenu* mNewMenu;

	ProjectIdMap mProjectIdMap;
	MaterialIdMap mMaterialIdMap;
	TechniqueIdMap mTechniqueIdMap;
	PassIdMap mPassIdMap;

	DECLARE_EVENT_TABLE()
};

#endif // _WORKSPACEPANEL_H_

