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
#include "TechniquePage.h"

#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/dirdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "MaterialController.h"
#include "Workspace.h"

const long ID_MATERIAL_COMBO_BOX = wxNewId();

BEGIN_EVENT_TABLE(TechniquePage, wxWizardPageSimple)
    EVT_CHOICE(ID_MATERIAL_COMBO_BOX, TechniquePage::OnProjectSelected)
END_EVENT_TABLE()

TechniquePage::TechniquePage(wxWizard* parent)
: wxWizardPageSimple(parent), mProject(NULL), mMaterial(NULL)
{
    createPage();
}

TechniquePage::TechniquePage(wxWizard* parent, Project* project)
: wxWizardPageSimple(parent), mProject(project), mMaterial(NULL)
{
    createPage();
}


TechniquePage::TechniquePage(wxWizard* parent, Project* project, MaterialController* mc)
: wxWizardPageSimple(parent), mProject(project), mMaterial(mc)
{
    createPage();
}


TechniquePage::~TechniquePage()
{
}

void TechniquePage::createPage()
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
    mSizer->Add(mProjectComboBox, 0, wxALL | wxEXPAND, 5);

    // Material Label
    mMaterialLabel = new wxStaticText(this, wxID_ANY, wxT("Material:"), wxDefaultPosition, wxDefaultSize, 0);
    mSizer->Add(mMaterialLabel, 0, wxALL, 5);

    // Material Combo Box
    mMaterialComboBox = new wxComboBox(this, ID_MATERIAL_COMBO_BOX);
    mSizer->Add(mMaterialComboBox, 0, wxALL | wxEXPAND, 5);

    // Name Label
    mNameLabel = new wxStaticText(this, wxID_ANY, wxT("Name:"), wxDefaultPosition, wxDefaultSize, 0);
    mSizer->Add(mNameLabel, 0, wxALL, 5);

    // Name Text
    mNameText = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
    mSizer->Add(mNameText, 0, wxALL | wxEXPAND, 5);

    SetSizer(mSizer);
    Layout();
}

void TechniquePage::getName(wxString& name) const
{
    name = mNameText->GetValue();
}

Project* TechniquePage::getProject() const
{
    wxString project = mProjectComboBox->GetValue();

    return Workspace::getSingletonPtr()->getProject(project.c_str());
}

MaterialController* TechniquePage::getMaterial() const
{
    wxString material = mMaterialComboBox->GetValue();

    return getProject()->getMaterialController(material.c_str());
}

void TechniquePage::setProject(Project* project)
{
    mProjectComboBox->SetValue(project != NULL ? project->getName().c_str() : wxEmptyString);
    populateMaterials(project->getMaterials());
}

void TechniquePage::setMaterial(MaterialController* mc)
{
    mMaterialComboBox->SetValue(mc != NULL ? mc->getMaterial()->getName().c_str() : wxEmptyString);
}

void TechniquePage::OnProjectSelected(wxCommandEvent& event)
{
    Project* project = getProject();
    if(project != NULL)
        populateMaterials(project->getMaterials());
}

void TechniquePage::populateMaterials(const MaterialControllerList* materials)
{
    wxArrayString materialNames;
    MaterialControllerList::const_iterator it;
    for(it = materials->begin(); it != materials->end(); ++it)
    {
        materialNames.Add((*it)->getMaterial()->getName().c_str());
    }

    mMaterialComboBox->Clear();
    mMaterialComboBox->Append(materialNames);
}

