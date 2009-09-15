/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#ifndef _TECHNIQUEPAGE_H_
#define _TECHNIQUEPAGE_H_

#include <wx/wizard.h>

#include "Project.h"

class wxBoxSizer;
class wxComboBox;
class wxStaticText;
class wxTextCtrl;

class TechniqueController;

class TechniquePage : public wxWizardPageSimple
{
public:
	TechniquePage(wxWizard* parent);
	TechniquePage(wxWizard* parent, Project* project);
	TechniquePage(wxWizard* parent, Project* project, MaterialController* mc);
	virtual ~TechniquePage();

	void getName(wxString& name) const;

	Project* getProject() const;
	void setProject(Project* project);

	MaterialController* getMaterial() const;
	void setMaterial(MaterialController* mc);

	void populateMaterials(const MaterialControllerList* materials);

	void OnProjectSelected(wxCommandEvent& event);

protected:
	void createPage();

	wxBoxSizer* mSizer;
	wxStaticText* mProjectLabel;
	wxComboBox* mProjectComboBox;
	wxStaticText* mMaterialLabel;
	wxComboBox* mMaterialComboBox;
	wxStaticText* mNameLabel;
	wxTextCtrl* mNameText;

	Project* mProject;
	MaterialController* mMaterial;

	DECLARE_EVENT_TABLE()
};

#endif // _TECHNIQUEPAGE_H_
