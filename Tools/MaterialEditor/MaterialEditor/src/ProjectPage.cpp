/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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