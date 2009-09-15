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
#ifndef _PROPERTIESPANEL_H_
#define _PROPERTIESPANEL_H_

#include <map>

#include <wx/panel.h>

class wxGridSizer;
class wxPropertyGridManager;

class EventArgs;
class MaterialController;
class MaterialEventArgs;
class MaterialPropertyGridPage;
class PassController;
class PassEventArgs;
class PassPropertyGridPage;
class ProjectEventArgs;
class SelectionEventArgs;
class TechniqueController;
class TechniqueEventArgs;
class TechniquePropertyGridPage;

//typedef std::map<Project*, ProjectP> ProjectPageMap;
typedef std::map<MaterialController*, int> MaterialPageIndexMap;
typedef std::map<TechniqueController*, int> TechniquePageIndexMap;
typedef std::map<PassController*, int> PassPageIndexMap;

class PropertiesPanel : public wxPanel
{
public:
	PropertiesPanel(wxWindow* parent,
		wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxTAB_TRAVERSAL | wxNO_BORDER,
		const wxString& name = wxT("Properties Panel"));

	virtual ~PropertiesPanel();

	void selectionChanged(EventArgs& args);

	void projectRemoved(EventArgs& args);
	void materialRemoved(EventArgs& args);
	void techniqueRemoved(EventArgs& args);
	void passRemoved(EventArgs& args);

protected:
	wxGridSizer* mGridSizer;
	wxPropertyGridManager* mPropertyGrid;

	MaterialPageIndexMap mMaterialPageIndexMap;
	TechniquePageIndexMap mTechniquePageIndexMap;
	PassPageIndexMap mPassPageIndexMap;

	DECLARE_EVENT_TABLE()
};

#endif // _PROPERTIESPANEL_H_
