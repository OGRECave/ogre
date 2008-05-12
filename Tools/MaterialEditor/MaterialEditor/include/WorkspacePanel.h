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

