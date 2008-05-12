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
#include "MaterialPage.h"

#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/dirdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "Project.h"
#include "Workspace.h"

BEGIN_EVENT_TABLE(MaterialPage, wxWizardPageSimple)
END_EVENT_TABLE()

MaterialPage::MaterialPage(wxWizard* parent)
: wxWizardPageSimple(parent)
{
	createPage();
}

MaterialPage::~MaterialPage()
{
}

void MaterialPage::createPage()
{
	mSizer = new wxBoxSizer(wxVERTICAL);
	
	// Project Label
	mProjectLabel = new wxStaticText(this, wxID_ANY, wxT("Project:"), wxDefaultPosition, wxDefaultSize, 0);
	mSizer->Add(mProjectLabel, 0, wxALL, 5);

	// Project Combo Box
	wxArrayString projectNames;
	const ProjectList* projects = Workspace::getSingletonPtr()->getProjects();
	for(ProjectList::const_iterator it = projects->begin(); it != projects->end(); ++it)
		projectNames.Add((*it)->getName().c_str());
	
	// TODO: Select first Project
	mProjectComboBox = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, projectNames, wxCB_DROPDOWN);
	mProjectComboBox->SetEditable(false);
	mSizer->Add(mProjectComboBox, 0, wxALL | wxEXPAND, 5);
	
	// Name Label
	mNameLabel = new wxStaticText(this, wxID_ANY, wxT("Name:"), wxDefaultPosition, wxDefaultSize, 0);
	mSizer->Add(mNameLabel, 0, wxALL, 5);

	// Name Text
	mNameText = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
	mSizer->Add(mNameText, 0, wxALL | wxEXPAND, 5);

	SetSizer(mSizer);
	Layout();
}

void MaterialPage::getName(wxString& name) const
{
	name = mNameText->GetValue();
}

Project* MaterialPage::getProject() const
{
	wxString project = mProjectComboBox->GetValue();

	return Workspace::getSingletonPtr()->getProject(project.c_str());
}

void MaterialPage::setProject(Project* project)
{
	mProjectComboBox->SetValue(project != NULL ? project->getName().c_str() : wxEmptyString);
}

