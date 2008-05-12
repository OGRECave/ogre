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
#include "ProjectPage.h"

#include <wx/button.h>
#include <wx/dirdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

BEGIN_EVENT_TABLE(ProjectPage, wxWizardPageSimple)
	EVT_BUTTON(wxID_ANY, ProjectPage::OnBrowse)
END_EVENT_TABLE()

ProjectPage::ProjectPage(wxWizard* parent)
: wxWizardPageSimple(parent)
{
	createPage();
}

ProjectPage::~ProjectPage()
{
}

void ProjectPage::createPage()
{
	mSizer = new wxBoxSizer(wxVERTICAL);
	
	mNameLabel = new wxStaticText(this, wxID_ANY, wxT("Name:"), wxDefaultPosition, wxDefaultSize, 0);
	mSizer->Add(mNameLabel, 0, wxALL, 5);

	mNameText = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
	mSizer->Add(mNameText, 0, wxALL | wxEXPAND, 5);

	mLocationLabel = new wxStaticText(this, wxID_ANY, wxT("Location:"), wxDefaultPosition, wxDefaultSize, 0);
	mSizer->Add(mLocationLabel, 0, wxALL, 5);

	mLocationText = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
	mSizer->Add(mLocationText, 0, wxALL | wxEXPAND, 5);

	mBrowseButton = new wxButton(this, wxID_ANY, wxT("Browse..."), wxDefaultPosition, wxDefaultSize, 0);
	mSizer->Add(mBrowseButton, 0, wxALL | wxALIGN_RIGHT, 5);

	SetSizer(mSizer);
	Layout();
}

void ProjectPage::getName(wxString& name) const
{
	name = mNameText->GetValue();
}

void ProjectPage::getLocation(wxString& location) const
{
	location = mLocationText->GetValue();
}

void ProjectPage::OnBrowse(wxCommandEvent& event)
{
	wxDirDialog* dlg = new wxDirDialog(this, wxT("Project Directory"), wxT("/"), wxDD_NEW_DIR_BUTTON);
	if(dlg->ShowModal()== wxID_OK)
	{
		mLocationText->SetValue(dlg->GetPath());
		mLocationText->SetSelection(0, mLocationText->GetValue().length() - 1);
	}
}