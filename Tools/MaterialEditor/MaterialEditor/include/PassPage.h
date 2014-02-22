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
#ifndef _PASSPAGE_H_
#define _PASSPAGE_H_

#include <wx/wizard.h>

class wxBoxSizer;
class wxComboBox;
class wxStaticText;
class wxTextCtrl;

#include "Project.h"
#include "MaterialController.h"

class TechniqueController;

class PassPage : public wxWizardPageSimple
{
public:
    PassPage(wxWizard* parent);
    PassPage(wxWizard* parent, Project* project);
    PassPage(wxWizard* parent, Project* project, MaterialController* mc);
    PassPage(wxWizard* parent, Project* project, MaterialController* mc, TechniqueController* tc);
    virtual ~PassPage();

    void getName(wxString& name) const;

    Project* getProject() const;
    void setProject(Project* project);

    MaterialController* getMaterial() const;
    void setMaterial(MaterialController* mc);

    TechniqueController* getTechnique() const;
    void setTechnique(TechniqueController* mc);

    void populateMaterials(const MaterialControllerList* materials);
    void populateTechniques(const TechniqueControllerList* techniques);

    void OnProjectSelected(wxCommandEvent& event);
    void OnMaterialSelected(wxCommandEvent& event);
    //void OnTechniqueSelected(wxCommandEvent& event);

protected:
    void createPage();

    wxBoxSizer* mSizer;
    wxStaticText* mProjectLabel;
    wxComboBox* mProjectComboBox;
    wxStaticText* mMaterialLabel;
    wxComboBox* mMaterialComboBox;
    wxStaticText* mTechniqueLabel;
    wxComboBox* mTechniqueComboBox;
    wxStaticText* mNameLabel;
    wxTextCtrl* mNameText;

    Project* mProject;
    MaterialController* mMaterial;
    TechniqueController* mTechnique;

    DECLARE_EVENT_TABLE()
};

#endif // _PASSPAGE_H_
