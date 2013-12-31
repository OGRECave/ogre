/*
-----------------------------------------------------------------------------
This source file is part of OGRE
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
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#ifndef _TECHNIQUEPROPERTYGRIDPAGE_H_
#define _TECHNIQUEPROPERTYGRIDPAGE_H_

#include <wx/wx.h>
#include <wx/propgrid/manager.h>

class TechniqueController;
class EventArgs;

class TechniquePropertyGridPage : public wxPropertyGridPage
{
public:
	TechniquePropertyGridPage(TechniqueController* controller);
	virtual ~TechniquePropertyGridPage();

	virtual void populate();

	void nameChanged(EventArgs& args);
	void schemeNameChanged(EventArgs& args);
	void lodIndexChanged(EventArgs& args);

protected:
	virtual void propertyChanged(wxPropertyGridEvent& event);
	
	TechniqueController* mController;

	wxPGId mNameId;
	wxPGId mSchemeNameId;
	wxPGId mLodIndexId;

	DECLARE_EVENT_TABLE();
};

#endif // _TECHNIQUEPROPERTYGRIDPAGE_H_