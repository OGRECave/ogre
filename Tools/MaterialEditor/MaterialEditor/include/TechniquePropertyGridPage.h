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