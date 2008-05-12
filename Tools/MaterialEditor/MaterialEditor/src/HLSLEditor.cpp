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
#include "HLSLEditor.h"

HLSLEditor::HLSLEditor(wxWindow* parent, wxWindowID id /*= -1*/,
		const wxPoint& pos /*= wxDefaultPosition*/,
		const wxSize& size /*= wxDefaultSize*/,
		long style /*= wxVSCROLL*/
		) : ScintillaEditor(parent, id, pos, size, style)
{
	initialize();
}

HLSLEditor::~HLSLEditor()
{
}

void HLSLEditor::initialize()
{
	StyleClearAll();
	SetLexer(wxSCI_LEX_OMS);

	// Load keywords
	wxString path = wxT("../lexers/hlsl/keywords");
	loadKeywords(path);
	
	// Set styles
	StyleSetForeground(wxSCI_OMS_DEFAULT, wxColour(0, 0, 0));
	StyleSetFontAttr(wxSCI_OMS_DEFAULT, 10, "Courier New", false, false, false);
	StyleSetForeground(wxSCI_OMS_COMMENT, wxColour(0, 128, 0));
	StyleSetFontAttr(wxSCI_OMS_COMMENT, 10, "Courier New", false, false, false);
	StyleSetForeground(wxSCI_OMS_PRIMARY, wxColour(0, 0, 255));
	StyleSetFontAttr(wxSCI_OMS_PRIMARY, 10, "Courier New", true, false, false);
	StyleSetForeground(wxSCI_OMS_ATTRIBUTE, wxColour(136, 0, 0));
	StyleSetFontAttr(wxSCI_OMS_ATTRIBUTE, 10, "Courier New", true, false, false);
	StyleSetForeground(wxSCI_OMS_VALUE, wxColour(160, 0, 160));
	StyleSetFontAttr(wxSCI_OMS_VALUE, 10, "Courier New", false, false, false);
	StyleSetForeground(wxSCI_OMS_NUMBER, wxColour(0, 0, 128));
	StyleSetFontAttr(wxSCI_OMS_NUMBER, 10, "Courier New", false, false, false);
}
