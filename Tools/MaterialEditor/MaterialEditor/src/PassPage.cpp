/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#include "PassPage.h"

#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/dirdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "OgreTechnique.h"

#include "MaterialController.h"
#include "PassController.h"
#include "Project.h"
#include "TechniqueController.h"
#include "Workspace.h"

const long ID_PROJECT_COMBO_BOX = wxNewId();
const long ID_MATERIAL_COMBO_BOX = wxNewId();
const long ID_TECHNIQUE_COMBO_BOX = wxNewId();

BEGIN_EVENT_TABLE(PassPage, wxWizardPageSimple)
	EVT_CHOICE(ID_PROJECT_COMBO_BOX, PassPage::OnProjectSelected)
	EVT_CHOICE(ID_MATERIAL_COMBO_BOX, PassPage::OnMaterialSelected)
	//EVT_CHOICE(ID_TECHNIQUE_COMBO_BOX, TechniquePage::OnTechniqueSelected)
END_EVENT_TABLE()

PassPage::PassPage(wxWizard* parent)
: wxWizardPageSimple(parent), mProject(NULL), mMaterial(NULL), mTechnique(NULL)
{
	createPage();
}

PassPage::PassPage(wxWizard* parent, Project* project)
: wxWizardPageSimple(parent), mProject(project), mMaterial(NULL), mTechnique(NULL)
{
	createPage();
}


PassPage::PassPage(wxWizard* parent, Project* project, MaterialController* mc)
: wxWizardPageSimple(parent), mProject(project), mMaterial(mc), mTechnique(NULL)
{
	createPage();
}

PassPage::PassPage(wxWizard* parent, Project* project, MaterialController* mc, TechniqueController* tc)
: wxWizardPageSimple(parent), mProject(project), mMaterial(mc), mTechnique(tc)
{
	createPage();
}


PassPage::~PassPage()
{
}

void PassPage::createPage()
{
	mSizer = new wxBoxSizer(wxVERTICAL);

	// Project Label
	mProjectLabel = new wxStaticText(this, wxID_ANY, wxT("Project:"), wxDefaultPosition, wxDefaultSize, 0);
	mSizer->Add(mProjectLabel, 0, wxALL, 0);

	// Project Combo Box
	wxArrayString projectNames;
	const ProjectList* projects = Workspace::getSingletonPtr()->getProjects();
	for(ProjectList::const_iterator it = projects->begin(); it != projects->end(); ++it)
		projectNames.Add((*it)->getName().c_str());

	// TODO: Select first Project
	mProjectComboBox = new wxComboBox(this, ID_PROJECT_COMBO_BOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, projectNames, wxCB_DROPDOWN);
	mSizer->Add(mProjectComboBox, 0, wxALL | wxEXPAND, 0);

	// Material Label
	mMaterialLabel = new wxStaticText(this, wxID_ANY, wxT("Material:"), wxDefaultPosition, wxDefaultSize, 0);
	mSizer->Add(mMaterialLabel, 0, wxALL, 0);

	// Material Combo Box
	mMaterialComboBox = new wxComboBox(this, ID_MATERIAL_COMBO_BOX);
	mSizer->Add(mMaterialComboBox, 0, wxALL | wxEXPAND, 0);

	// Technique Label
	mTechniqueLabel = new wxStaticText(this, wxID_ANY, wxT("Technique:"), wxDefaultPosition, wxDefaultSize, 0);
	mSizer->Add(mTechniqueLabel, 0, wxALL, 0);

	// Technique Combo Box
	mTechniqueComboBox = new wxComboBox(this, ID_TECHNIQUE_COMBO_BOX);
	mSizer->Add(mTechniqueComboBox, 0, wxALL | wxEXPAND, 0);

	// Name Label
	mNameLabel = new wxStaticText(this, wxID_ANY, wxT("Name:"), wxDefaultPosition, wxDefaultSize, 0);
	mSizer->Add(mNameLabel, 0, wxALL, 0);

	// Name Text
	mNameText = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
	mSizer->Add(mNameText, 0, wxALL | wxEXPAND, 0);

	SetSizer(mSizer);
	Layout();
}

void PassPage::getName(wxString& name) const
{
	name = mNameText->GetValue();
}

Project* PassPage::getProject() const
{
	wxString project = mProjectComboBox->GetValue();

	return Workspace::getSingletonPtr()->getProject(project.c_str());
}

MaterialController* PassPage::getMaterial() const
{
	wxString material = mMaterialComboBox->GetValue();

	return getProject()->getMaterialController(material.c_str());
}

TechniqueController* PassPage::getTechnique() const
{
	wxString technique = mTechniqueComboBox->GetValue();

	return getMaterial()->getTechniqueController(technique.c_str());
}

void PassPage::setProject(Project* project)
{
	mProjectComboBox->SetValue(project != NULL ? project->getName().c_str() : wxEmptyString);
	populateMaterials(project != NULL ? project->getMaterials() : NULL);
}

void PassPage::setMaterial(MaterialController* mc)
{
	mMaterialComboBox->SetValue(mc != NULL ? mc->getMaterial()->getName().c_str() : wxEmptyString);
	populateTechniques(mc != NULL ? mc->getTechniqueControllers() : NULL);
}

void PassPage::setTechnique(TechniqueController* tc)
{
	mTechniqueComboBox->SetValue(tc != NULL ? tc->getTechnique()->getName().c_str() : wxEmptyString);
}

void PassPage::OnProjectSelected(wxCommandEvent& event)
{
	Project* project = getProject();
	if(project != NULL)
		populateMaterials(project->getMaterials());
}

void PassPage::OnMaterialSelected(wxCommandEvent& event)
{
	MaterialController* mc = getMaterial();
	if(mc != NULL)
		populateTechniques(mc->getTechniqueControllers());
}

void PassPage::populateMaterials(const MaterialControllerList* materials)
{
	if(materials == NULL)
	{
		mMaterialComboBox->Clear();
		return;
	}

	wxArrayString materialNames;
	MaterialControllerList::const_iterator it;
	for(it = materials->begin(); it != materials->end(); ++it)
	{
		materialNames.Add((*it)->getMaterial()->getName().c_str());
	}

	mMaterialComboBox->Clear();
	mMaterialComboBox->Append(materialNames);
}

void PassPage::populateTechniques(const TechniqueControllerList* techniques)
{
	if(techniques == NULL)
	{
		mTechniqueComboBox->Clear();
		return;
	}

	wxArrayString techniqueNames;
	TechniqueControllerList::const_iterator it;
	for(it = techniques->begin(); it != techniques->end(); ++it)
	{
		techniqueNames.Add((*it)->getTechnique()->getName().c_str());
	}

	mTechniqueComboBox->Clear();
	mTechniqueComboBox->Append(techniqueNames);
}


