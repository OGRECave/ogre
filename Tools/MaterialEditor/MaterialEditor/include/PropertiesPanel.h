/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
