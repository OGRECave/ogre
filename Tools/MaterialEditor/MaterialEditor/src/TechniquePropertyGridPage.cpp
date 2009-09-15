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
#include "TechniquePropertyGridPage.h"

#include <boost/bind.hpp>

#include <wx/propgrid/advprops.h>

#include "OgreTechnique.h"

#include "TechniqueController.h"
#include "TechniqueEventArgs.h"

BEGIN_EVENT_TABLE(TechniquePropertyGridPage, wxPropertyGridPage)
	EVT_PG_CHANGED(-1, TechniquePropertyGridPage::propertyChanged)
END_EVENT_TABLE()

TechniquePropertyGridPage::TechniquePropertyGridPage(TechniqueController* controller)
: mController(controller)
{
	mController->subscribe(TechniqueController::NameChanged, boost::bind(&TechniquePropertyGridPage::nameChanged, this, _1));
	mController->subscribe(TechniqueController::SchemeChanged, boost::bind(&TechniquePropertyGridPage::schemeNameChanged, this, _1));
	mController->subscribe(TechniqueController::LodIndexChanged, boost::bind(&TechniquePropertyGridPage::lodIndexChanged, this, _1));
}

TechniquePropertyGridPage::~TechniquePropertyGridPage()
{
}

void TechniquePropertyGridPage::populate()
{
	mNameId = Append(wxStringProperty(wxT("Name"), wxPG_LABEL, mController->getTechnique()->getName()));
	mSchemeNameId = Append(wxStringProperty(wxT("Scheme Name"), wxPG_LABEL, mController->getTechnique()->getSchemeName()));
	mLodIndexId = Append(wxIntProperty(wxT("LOD Index"), wxPG_LABEL, mController->getTechnique()->getLodIndex()));
}

void TechniquePropertyGridPage::propertyChanged(wxPropertyGridEvent& event)
{
	wxPGId id = event.GetProperty();
	if(id == mNameId)
	{
		mController->setName(event.GetPropertyValueAsString().c_str());
	}
	else if(id == mSchemeNameId)
	{
		mController->setSchemeName(event.GetPropertyValueAsString().c_str());
	}
	else if(id = mLodIndexId)
	{
		mController->setLodIndex(event.GetPropertyValueAsInt());
	}
}

void TechniquePropertyGridPage::nameChanged(EventArgs& args)
{
	TechniqueEventArgs tea = dynamic_cast<TechniqueEventArgs&>(args);
	TechniqueController* tc = tea.getTechniqueController();

	wxPGProperty* prop = GetPropertyPtr(mNameId);
	if(prop == NULL) return;
	prop->SetValueFromString(tc->getTechnique()->getName().c_str());
}

void TechniquePropertyGridPage::schemeNameChanged(EventArgs& args)
{
	TechniqueEventArgs tea = dynamic_cast<TechniqueEventArgs&>(args);
	TechniqueController* tc = tea.getTechniqueController();

	wxPGProperty* prop = GetPropertyPtr(mSchemeNameId);
	if(prop == NULL) return;
	prop->SetValueFromString(tc->getTechnique()->getSchemeName().c_str());
}

void TechniquePropertyGridPage::lodIndexChanged(EventArgs& args)
{
	TechniqueEventArgs tea = dynamic_cast<TechniqueEventArgs&>(args);
	TechniqueController* tc = tea.getTechniqueController();

	wxPGProperty* prop = GetPropertyPtr(mLodIndexId);
	if(prop == NULL) return;
	prop->SetValueFromInt(tc->getTechnique()->getLodIndex());
}
















