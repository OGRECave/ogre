/*
-------------------------------------------------------------------------
This source file is a part of OGRE
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
THE SOFTWARE
-------------------------------------------------------------------------
*/
#pragma warning(disable:4800)

#include "MaterialPropertyGridPage.h"

#include "MaterialController.h"

BEGIN_EVENT_TABLE(MaterialPropertyGridPage, wxPropertyGridPage)
	EVT_PG_CHANGED(-1, MaterialPropertyGridPage::propertyChange)
END_EVENT_TABLE()

MaterialPropertyGridPage::MaterialPropertyGridPage(MaterialController* controller)
: mController(controller)
{
	
}

MaterialPropertyGridPage::~MaterialPropertyGridPage()
{
}

void MaterialPropertyGridPage::populate()
{
	mPropertyNameId = Append(wxStringProperty(wxT("Name"), wxPG_LABEL, mController->getMaterial()->getName()));
	mPropertyReceiveShadowsId = Append(wxBoolProperty(wxT("Receive Shadows"), wxPG_LABEL, mController->getMaterial()->getReceiveShadows()));
	mPropertyTransparencyCastsShadowsId = Append(wxBoolProperty(wxT("Transparency Casts Shadows"), wxPG_LABEL, mController->getMaterial()->getTransparencyCastsShadows()));
}

void MaterialPropertyGridPage::propertyChange(wxPropertyGridEvent& event)
{
	wxPGId id = event.GetProperty();
	if(id == mPropertyNameId)
	{
		
	}
	else if(id == mPropertyReceiveShadowsId)
	{
		mController->setReceiveShadows(event.GetPropertyValueAsBool());
	}
	else if(id == mPropertyTransparencyCastsShadowsId)
	{
		mController->setTransparencyCastsShadows(event.GetPropertyValueAsBool());
	}
}
