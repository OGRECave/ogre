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
#include "WorkspacePanel.h"

#include <boost/bind.hpp>

#include <wx/bitmap.h>
#include <wx/button.h>
#include <wx/imaglist.h>
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/aui/auibook.h>

#include "OgreMaterial.h"
#include "OgreMaterialManager.h"
#include "OgreMaterialSerializer.h"
#include "OgrePass.h"
#include "OgreTechnique.h"

#include "EditorManager.h"
#include "EventArgs.h"
#include "IconManager.h"
#include "MaterialController.h"
#include "MaterialEventArgs.h"
#include "MaterialScriptEditor.h"
#include "MaterialWizard.h"
#include "PassController.h"
#include "PassWizard.h"
#include "Project.h"
#include "ProjectEventArgs.h"
#include "ProjectWizard.h"
#include "SelectionService.h"
#include "TechniqueController.h"
#include "TechniqueEventArgs.h"
#include "TechniqueWizard.h"
#include "Workspace.h"
#include "WorkspaceEventArgs.h"

#define WORKSPACE_IMAGE 0
#define PROJECT_IMAGE 1
#define MATERIAL_SCRIPT_IMAGE 2
#define MATERIAL_IMAGE 3
#define TECHNIQUE_IMAGE 4
#define PASS_IMAGE 5

namespace
{
	class Ogre::MaterialSerializer;
}

const long ID_TREE_CTRL = wxNewId();
const long ID_MENU_NEW = wxNewId();
const long ID_MENU_NEW_PROJECT = wxNewId();
const long ID_MENU_NEW_MATERIAL_SCRIPT = wxNewId();
const long ID_MENU_NEW_MATERIAL = wxNewId();
const long ID_MENU_NEW_TECHNIQUE = wxNewId();
const long ID_MENU_NEW_PASS = wxNewId();
const long ID_MENU_ADD_MATERIAL = wxNewId();
const long ID_MENU_EDIT = wxNewId();
const long ID_MENU_PASS_ENABLED = wxNewId();
const long ID_MENU_DELETE = wxNewId();

BEGIN_EVENT_TABLE(WorkspacePanel, wxPanel)
	EVT_TREE_ITEM_RIGHT_CLICK(ID_TREE_CTRL, WorkspacePanel::OnRightClick)
	EVT_TREE_ITEM_ACTIVATED(ID_TREE_CTRL, WorkspacePanel::OnActivate)
	EVT_TREE_SEL_CHANGED(ID_TREE_CTRL, WorkspacePanel::OnSelectionChanged)
	EVT_MENU(ID_MENU_NEW_PROJECT, WorkspacePanel::OnNewProject)
	EVT_MENU(ID_MENU_NEW_MATERIAL_SCRIPT, WorkspacePanel::OnNewMaterialScript)
	EVT_MENU(ID_MENU_NEW_MATERIAL, WorkspacePanel::OnNewMaterial)
	EVT_MENU(ID_MENU_NEW_TECHNIQUE, WorkspacePanel::OnNewTechnique)
	EVT_MENU(ID_MENU_NEW_PASS, WorkspacePanel::OnNewPass)
	EVT_MENU(ID_MENU_ADD_MATERIAL, WorkspacePanel::OnAddMaterial)
	EVT_MENU(ID_MENU_EDIT, WorkspacePanel::OnEdit)
	EVT_UPDATE_UI(ID_MENU_NEW_MATERIAL, WorkspacePanel::OnUpdateMaterialMenuItem)
	EVT_UPDATE_UI(ID_MENU_NEW_TECHNIQUE, WorkspacePanel::OnUpdateTechniqueMenuItem)
	EVT_UPDATE_UI(ID_MENU_NEW_PASS, WorkspacePanel::OnUpdatePassMenuItem)
END_EVENT_TABLE()

WorkspacePanel::WorkspacePanel(wxWindow* parent,
			   wxWindowID id /* = wxID_ANY */,
			   const wxPoint& pos /* = wxDefaultPosition */,
			   const wxSize& size /* = wxDefaultSize */,
			   long style /* = wxTAB_TRAVERSAL | wxNO_BORDER */,
			   const wxString& name /* = wxT("Workspace Panel")) */)
			   : mImageList(NULL), wxPanel(parent, id, pos, size, style, name)
{
	createPanel();

	Workspace::getSingletonPtr()->subscribe(Workspace::ProjectAdded, boost::bind(&WorkspacePanel::projectAdded, this, _1));
	Workspace::getSingletonPtr()->subscribe(Workspace::ProjectRemoved, boost::bind(&WorkspacePanel::projectRemoved, this, _1));
}

WorkspacePanel::~WorkspacePanel()
{
}

void WorkspacePanel::createPanel()
{
	mSizer = new wxFlexGridSizer(1, 1, 0, 0);
	mSizer->AddGrowableCol(0);
	mSizer->AddGrowableRow(0);
	mSizer->SetFlexibleDirection(wxVERTICAL);
	mSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	//mToolBarPanel = new wxPanel(this);
	//mSizer->Add(mToolBarPanel, 1, wxALL | wxEXPAND, 0);

	mTreeCtrl = new wxTreeCtrl(this, ID_TREE_CTRL, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTR_FULL_ROW_HIGHLIGHT | wxTR_HAS_BUTTONS | wxTR_SINGLE);
	mTreeCtrl->AssignImageList(getImageList());
	mRootId = mTreeCtrl->AddRoot(wxT("Workspace"), WORKSPACE_IMAGE);

	mSizer->Add(mTreeCtrl, 0, wxALL | wxEXPAND, 0);

	SetSizer(mSizer);
	Layout();
}

wxImageList* WorkspacePanel::getImageList()
{
	if(mImageList == NULL)
	{
		mImageList = new wxImageList(16, 16, true, 6);
		mImageList->Add(IconManager::getSingleton().getIcon(IconManager::WORKSPACE));
		mImageList->Add(IconManager::getSingleton().getIcon(IconManager::PROJECT));
		mImageList->Add(IconManager::getSingleton().getIcon(IconManager::MATERIAL_SCRIPT));
		mImageList->Add(IconManager::getSingleton().getIcon(IconManager::MATERIAL));
		mImageList->Add(IconManager::getSingleton().getIcon(IconManager::TECHNIQUE));
		mImageList->Add(IconManager::getSingleton().getIcon(IconManager::PASS));
	}

	return mImageList;
}

void WorkspacePanel::showContextMenu(wxPoint point, wxTreeItemId id)
{
	wxMenu contextMenu;
	appendNewMenu(&contextMenu);
	contextMenu.AppendSeparator();
	if(isProject(id)) appendProjectMenuItems(&contextMenu);
	else if(isMaterial(id)) appendMaterialMenuItems(&contextMenu);
	else if(isTechnique(id)) appendTechniqueMenuItems(&contextMenu);
	//else appendPassMenuItems(&contextMenu);
	contextMenu.Append(ID_MENU_EDIT, wxT("Edit"));
	PopupMenu(&contextMenu, point);
}

void WorkspacePanel::appendNewMenu(wxMenu* menu)
{
	wxMenu* newMenu = new wxMenu();
	newMenu->Append(ID_MENU_NEW_PROJECT, wxT("Project"));
	newMenu->Append(ID_MENU_NEW_MATERIAL_SCRIPT, wxT("Material Script"));
	newMenu->Append(ID_MENU_NEW_MATERIAL, wxT("Material"));
	newMenu->Append(ID_MENU_NEW_TECHNIQUE, wxT("Technique"));
	newMenu->Append(ID_MENU_NEW_PASS, wxT("Pass"));

	menu->AppendSubMenu(newMenu, wxT("New"));
}

void WorkspacePanel::appendProjectMenuItems(wxMenu* menu)
{
	menu->Append(ID_MENU_ADD_MATERIAL, wxT("Add Material"));
}

void WorkspacePanel::appendMaterialMenuItems(wxMenu* menu)
{
}

void WorkspacePanel::appendTechniqueMenuItems(wxMenu* menu)
{
}

void WorkspacePanel::appendPassMenuItems(wxMenu* menu)
{
	menu->AppendCheckItem(ID_MENU_PASS_ENABLED, wxT("Enabled"));
}

Project* WorkspacePanel::getProject(wxTreeItemId id)
{
	for(ProjectIdMap::iterator it = mProjectIdMap.begin(); it != mProjectIdMap.end(); ++it)
	{
		if(it->second == id) return it->first;
	}

	return NULL;
}

MaterialController* WorkspacePanel::getMaterial(wxTreeItemId id)
{
	for(MaterialIdMap::iterator it = mMaterialIdMap.begin(); it != mMaterialIdMap.end(); ++it)
	{
		if(it->second == id) return it->first;
	}

	return NULL;
}

TechniqueController* WorkspacePanel::getTechnique(wxTreeItemId id)
{
	for(TechniqueIdMap::iterator it = mTechniqueIdMap.begin(); it != mTechniqueIdMap.end(); ++it)
	{
		if(it->second == id) return it->first;
	}

	return NULL;
}

PassController* WorkspacePanel::getPass(wxTreeItemId id)
{
	for(PassIdMap::iterator it = mPassIdMap.begin(); it != mPassIdMap.end(); ++it)
	{
		if(it->second == id) return it->first;
	}

	return NULL;
}

bool WorkspacePanel::isWorkspace(wxTreeItemId id)
{
	return mRootId == id;
}

bool WorkspacePanel::isProject(wxTreeItemId id)
{
	return getProject(id) != NULL;
}

bool WorkspacePanel::isMaterial(wxTreeItemId id)
{
	return getMaterial(id) != NULL;
}

bool WorkspacePanel::isTechnique(wxTreeItemId id)
{
	return getTechnique(id) != NULL;
}

bool WorkspacePanel::isPass(wxTreeItemId id)
{
	return getPass(id) != NULL;
}

void WorkspacePanel::subscribe(Project* project)
{
	project->subscribe(Project::NameChanged, boost::bind(&WorkspacePanel::projectNameChanged, this, _1));
	project->subscribe(Project::MaterialAdded, boost::bind(&WorkspacePanel::projectMaterialAdded, this, _1));
	project->subscribe(Project::MaterialRemoved, boost::bind(&WorkspacePanel::projectMaterialRemoved, this, _1));
}

void WorkspacePanel::subscribe(MaterialController* material)
{
	material->subscribe(MaterialController::NameChanged, boost::bind(&WorkspacePanel::materialNameChanged, this, _1));
	material->subscribe(MaterialController::TechniqueAdded, boost::bind(&WorkspacePanel::materialTechniqueAdded, this, _1));
	material->subscribe(MaterialController::TechniqueRemoved, boost::bind(&WorkspacePanel::materialTechniqueRemoved, this, _1));
}

void WorkspacePanel::subscribe(TechniqueController* technique)
{
	technique->subscribe(TechniqueController::NameChanged, boost::bind(&WorkspacePanel::techniqueNameChanged, this, _1));
	technique->subscribe(TechniqueController::PassAdded, boost::bind(&WorkspacePanel::techniquePassAdded, this, _1));
	technique->subscribe(TechniqueController::PassRemoved, boost::bind(&WorkspacePanel::techniquePassRemoved, this, _1));
}

void WorkspacePanel::OnRightClick(wxTreeEvent& event)
{
	showContextMenu(event.GetPoint(), event.GetItem());
}

void WorkspacePanel::OnActivate(wxTreeEvent& event)
{
	OnEdit(event);
}

void WorkspacePanel::OnSelectionChanged(wxTreeEvent& event)
{
	SelectionList list;
	wxTreeItemId id = event.GetItem();
	if(isProject(id)) list.push_back(getProject(id));
	else if(isMaterial(id)) list.push_back(getMaterial(id));
	else if(isTechnique(id)) list.push_back(getTechnique(id));
	else if(isPass(id)) list.push_back(getPass(id));
	// else its the workspace so just leave the list empty as if nothing were selected

	SelectionService::getSingletonPtr()->setSelection(list);
}

void WorkspacePanel::OnNewProject(wxCommandEvent& event)
{
	ProjectWizard* wizard = new ProjectWizard();
	wizard->Create(this, wxID_ANY, wxT("New Project"), wxNullBitmap, wxDefaultPosition, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	wizard->RunWizard(wizard->getProjectPage()); // This seems unnatural, seems there must be a better way to deal with wizards

	wizard->Destroy();

	delete wizard;
}

void WorkspacePanel::OnNewMaterialScript(wxCommandEvent& event)
{
	wxTreeItemId id = mTreeCtrl->GetSelection();
	mTreeCtrl->AppendItem(id, "Material Script", MATERIAL_SCRIPT_IMAGE);
}

void WorkspacePanel::OnNewMaterial(wxCommandEvent& event)
{
	wxTreeItemId id = mTreeCtrl->GetSelection();

	MaterialWizard* wizard = new MaterialWizard();
	wizard->Create(this, wxID_ANY, wxT("New Project"), wxNullBitmap, wxDefaultPosition, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	wizard->getMaterialPage()->setProject(getProject(id));
	wizard->RunWizard(wizard->getMaterialPage()); // This seems unnatural, seems there must be a better way to deal with wizards

	wizard->Destroy();

	delete wizard;
}

void WorkspacePanel::OnNewTechnique(wxCommandEvent& event)
{
	Project* project = NULL;
	MaterialController* material = NULL;

	wxTreeItemId selId = mTreeCtrl->GetSelection();
	if(isProject(selId))
	{
		project = getProject(selId);
	}
	else if(isMaterial(selId))
	{
		wxTreeItemId projectId = mTreeCtrl->GetItemParent(selId);
		project = getProject(projectId);

		material = getMaterial(selId);
	}

	TechniqueWizard* wizard = new TechniqueWizard();
	wizard->Create(this, wxID_ANY, wxT("New Technique"), wxNullBitmap, wxDefaultPosition, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	wizard->getTechniquePage()->setProject(project);
	wizard->getTechniquePage()->setMaterial(material);
	wizard->RunWizard(wizard->getTechniquePage()); // This seems unnatural, seems there must be a better way to deal with wizards

	wizard->Destroy();

	delete wizard;
}

void WorkspacePanel::OnNewPass(wxCommandEvent& event)
{
	Project* project = NULL;
	MaterialController* material = NULL;
	TechniqueController* technique = NULL;

	wxTreeItemId selId = mTreeCtrl->GetSelection();
	if(isProject(selId))
	{
		project = getProject(selId);
	}
	else if(isMaterial(selId))
	{
		wxTreeItemId projectId = mTreeCtrl->GetItemParent(selId);
		project = getProject(projectId);

		material = getMaterial(selId);
	}
	else if(isTechnique(selId))
	{
		wxTreeItemId materialId = mTreeCtrl->GetItemParent(selId);
		material = getMaterial(materialId);

		wxTreeItemId projectId = mTreeCtrl->GetItemParent(materialId);
		project = getProject(projectId);
	
		technique = getTechnique(selId);
	}

	PassWizard* wizard = new PassWizard();
	wizard->Create(this, wxID_ANY, wxT("New Pass"), wxNullBitmap, wxDefaultPosition, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	wizard->getPassPage()->setProject(project);
	wizard->getPassPage()->setMaterial(material);
	wizard->getPassPage()->setTechnique(technique);
	wizard->RunWizard(wizard->getPassPage());

	wizard->Destroy();

	delete wizard;
}

void WorkspacePanel::OnAddMaterial(wxCommandEvent& event)
{
	wxFileDialog * openDialog = new wxFileDialog(this, wxT("Add a Material"), wxEmptyString, wxEmptyString,
		wxT("Material Files (*.material)|*.material|All Files (*.*)|*.*"));

	if(openDialog->ShowModal() == wxID_OK)
	{
		wxString path = openDialog->GetPath();
		if(path.EndsWith(wxT(".material")))
		{
			wxTreeItemId selId = mTreeCtrl->GetSelection();
			if(isProject(selId))
			{
				Project* project = getProject(selId);

				MaterialScriptEditor* editor = new MaterialScriptEditor(EditorManager::getSingletonPtr()->getEditorNotebook());
				editor->loadFile(path);
				int index = (int)path.find_last_of('\\');
				if(index == -1) index = (int)path.find_last_of('/');
				editor->setName((index != -1) ? path.substr(index + 1, path.Length()) : path);

				EditorManager::getSingletonPtr()->openEditor(editor);

				Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().getByName("Ogre/Earring");
				project->addMaterial(mat);
			}
		}
	}
}

void WorkspacePanel::OnEdit(wxCommandEvent& event)
{
	wxTreeItemId selId = mTreeCtrl->GetSelection();
	if(isMaterial(selId))
	{
		MaterialController* mc = getMaterial(selId);
		
		EditorManager* editorManager = EditorManager::getSingletonPtr();
		Editor* editor = editorManager->findEditor(mc->getMaterial()->getName().c_str());
		Editor* editorMat = editorManager->findEditor(Ogre::String(mc->getMaterial()->getName() + ".material").c_str());
		if(editor != NULL)
		{
			editorManager->setActiveEditor(editor);
		}
		else if(editorMat != NULL)
		{
			editorManager->setActiveEditor(editorMat);
		}
		else
		{
			MaterialSerializer* ms = new MaterialSerializer();
			ms->queueForExport(mc->getMaterial(), true);
			String script = ms->getQueuedAsString();

			MaterialScriptEditor* materialEditor = new MaterialScriptEditor(editorManager->getEditorNotebook(), wxID_ANY);
			wxString name = mc->getMaterial()->getName().c_str();
			name += wxT(".material");
			materialEditor->setName(name);
			materialEditor->SetText(script);

			editorManager->openEditor(materialEditor);
		}
	}
}

void WorkspacePanel::OnUpdateMaterialMenuItem(wxUpdateUIEvent& event)
{
	event.Enable(!Workspace::getSingletonPtr()->getProjects()->empty());
}

void WorkspacePanel::OnUpdateTechniqueMenuItem(wxUpdateUIEvent& event)
{
	bool enable = false;
	const ProjectList* projects = Workspace::getSingletonPtr()->getProjects();
	
	ProjectList::const_iterator it;
	for(it = projects->begin(); it != projects->end(); ++it)
	{
		if(!(*it)->getMaterials()->empty())
		{
			enable = true;
			break;
		}
	}
	
	event.Enable(enable);
}

void WorkspacePanel::OnUpdatePassMenuItem(wxUpdateUIEvent& event)
{
	bool enable = false;
	const ProjectList* projects = Workspace::getSingletonPtr()->getProjects();
	
	ProjectList::const_iterator pit;
	for(pit = projects->begin(); pit != projects->end(); ++pit)
	{
		const MaterialControllerList* materials = (*pit)->getMaterials();
		MaterialControllerList::const_iterator mit;
		for(mit = materials->begin(); mit != materials->end(); ++mit)
		{
			if(!(*mit)->getTechniqueControllers()->empty())
			{
				enable = true;
				break;
			}
		}
	}
	
	event.Enable(enable);
}

void WorkspacePanel::projectAdded(EventArgs& args)
{
	WorkspaceEventArgs wea = dynamic_cast<WorkspaceEventArgs&>(args);
	Project* project = wea.getProject();
	subscribe(project);

	wxTreeItemId id = mTreeCtrl->AppendItem(mRootId, project->getName().c_str(), PROJECT_IMAGE);
	mTreeCtrl->SelectItem(id, true);

	mProjectIdMap[project] = id;
}

void WorkspacePanel::projectRemoved(EventArgs& args)
{
	// TODO: Implement projectRemoved
}

void WorkspacePanel::projectNameChanged(EventArgs& args)
{
	ProjectEventArgs pea = dynamic_cast<ProjectEventArgs&>(args);
	Project* project = pea.getProject();

	wxTreeItemId projectId = mProjectIdMap[project];
	mTreeCtrl->SetItemText(projectId, project->getName().c_str());
}

void WorkspacePanel::projectMaterialAdded(EventArgs& args)
{
	ProjectEventArgs pea = dynamic_cast<ProjectEventArgs&>(args);
	Project* project = pea.getProject();
	MaterialController* material = pea.getMaterial();

	wxTreeItemId projectId = mProjectIdMap[project];
	wxTreeItemId id = mTreeCtrl->AppendItem(projectId, material->getMaterial()->getName().c_str(), MATERIAL_IMAGE);
	mTreeCtrl->SelectItem(id, true);

	mMaterialIdMap[material] = id;

	subscribe(material);
}

void WorkspacePanel::projectMaterialRemoved(EventArgs& args)
{
	// TODO: Implement projectMaterialRemoved
}

void WorkspacePanel::materialNameChanged(EventArgs& args)
{
	MaterialEventArgs mea = dynamic_cast<MaterialEventArgs&>(args);
	MaterialController* mc = mea.getMaterialController();

	wxTreeItemId materialId = mMaterialIdMap[mc];
	mTreeCtrl->SetItemText(materialId, mc->getMaterial()->getName().c_str());
}

void WorkspacePanel::materialTechniqueAdded(EventArgs& args)
{
	MaterialEventArgs mea = dynamic_cast<MaterialEventArgs&>(args);
	MaterialController* mc = mea.getMaterialController();
	TechniqueController* tc = mea.getTechniqueController();

	wxTreeItemId materialId = mMaterialIdMap[mc];
	wxTreeItemId id = mTreeCtrl->AppendItem(materialId, tc->getTechnique()->getName().c_str(), TECHNIQUE_IMAGE);
	mTreeCtrl->SelectItem(id, true);
	
	mTechniqueIdMap[tc] = id;

	subscribe(tc);
}

void WorkspacePanel::materialTechniqueRemoved(EventArgs& args)
{
	// TODO: Implement materialTechniqueRemoved
}

void WorkspacePanel::techniqueNameChanged(EventArgs& args)
{
	TechniqueEventArgs tea = dynamic_cast<TechniqueEventArgs&>(args);
	TechniqueController* tc = tea.getTechniqueController();

	wxTreeItemId techniqueId = mTechniqueIdMap[tc];
	mTreeCtrl->SetItemText(techniqueId, tc->getTechnique()->getName().c_str());
}

void WorkspacePanel::techniquePassAdded(EventArgs& args)
{
	TechniqueEventArgs tea = dynamic_cast<TechniqueEventArgs&>(args);
	TechniqueController* tc = tea.getTechniqueController();
	PassController* pc = tea.getPassController();

	wxTreeItemId techniqueId = mTechniqueIdMap[tc];
	wxTreeItemId id = mTreeCtrl->AppendItem(techniqueId, pc->getPass()->getName().c_str(), PASS_IMAGE);
	mTreeCtrl->SelectItem(id, true);

	mPassIdMap[pc] = id;
}

void WorkspacePanel::techniquePassRemoved(EventArgs& args)
{
	// TODO: Implement materialTechniqueRemoved
}

