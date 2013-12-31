/*
-------------------------------------------------------------------------
This source file is a part of OGRE
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
THE SOFTWARE
-------------------------------------------------------------------------
*/
#include "MaterialWizard.h"

#include <wx/sizer.h>

#include "MaterialPage.h"
#include "Project.h"
#include "Workspace.h"

BEGIN_EVENT_TABLE(MaterialWizard, wxWizard)
	EVT_WIZARD_FINISHED(wxID_ANY, MaterialWizard::OnFinish)
END_EVENT_TABLE()

MaterialWizard::MaterialWizard()
{
}

MaterialWizard::~MaterialWizard()
{
}

bool MaterialWizard::Create(wxWindow* parent, int id /* = -1 */, const wxString& title /* = wxEmptyString */, const wxBitmap& bitmap /* = wxNullBitmap */, const wxPoint& pos /* = wxDefaultPosition */, long style /* = wxDEFAULT_DIALOG_STYLE */)
{
	bool result = wxWizard::Create(parent, id, title, bitmap, pos, style);

	mMaterialPage = new MaterialPage(this);

	GetPageAreaSizer()->Add(mMaterialPage);

	return result;
}

MaterialPage* MaterialWizard::getMaterialPage() const
{
	return mMaterialPage;
}

void MaterialWizard::OnFinish(wxWizardEvent& event)
{
	Project* project = mMaterialPage->getProject();

	wxString name;
	mMaterialPage->getName(name);

	project->createMaterial(name.c_str());
}
