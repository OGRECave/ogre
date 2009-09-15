/*
-----------------------------------------------------------------------------
This source file is part of OGRE 
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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

#include "OgreExport.h"

#include "windows.h"
#include "resource.h"

#include <string>
#include <fstream>
#include <list>
#include <queue>

#include "max.h"
#include "plugapi.h"
#include "stdmat.h"
#include "impexp.h"
#include "CS/BipedApi.h"
#include "CS/KeyTrack.h"
#include "CS/phyexp.h"
#include "iparamb2.h"
#include "iskin.h"

static OgreMaxExport* _exp = 0;

OgreMaxExport::OgreMaxExport(HINSTANCE hInst) :
	m_meshExporter(m_config), 
	m_meshXMLExporter(m_config, m_materialMap),
	m_materialExporter(m_config, m_materialMap),
	m_tabGeneral(m_config, this),
	m_tabMesh(m_config, this),
	m_tabSkeletalAnimation(m_config, this),
	m_tabVertexAnimation(m_config, this),
	m_tabMaterial(m_config, this)
{

	m_hInstance = hInst;
	m_hWndDlgExport = 0;
	m_tabDisplay = NULL;
}

OgreMaxExport::~OgreMaxExport() {
}

int	OgreMaxExport::DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options) {

	// this sets the export path as well
	m_config.setExportFilename(name);
	m_config.load(); // can only do this after the export fname has been set and parsed out into its parts

	// if material and skeleton filenames are not set, do so now
	if (m_config.getMaterialFilename() == "")
		m_config.setMaterialFilename(m_config.getExportBasename() + ".material");

	if (m_config.getSkeletonFilename() == "")
		m_config.setSkeletonFilename(m_config.getExportBasename() + ".skeleton");

	m_meshExporter.setMaxInterface(ei, i);
	m_meshXMLExporter.setMaxInterface(ei, i);

	m_tabGeneral.setExportInterface(ei, i);
	m_tabMesh.setExportInterface(ei, i);
	m_tabSkeletalAnimation.setExportInterface(ei, i);
	m_tabVertexAnimation.setExportInterface(ei, i);
	m_tabMaterial.setExportInterface(ei, i);

	// Max will supply a nonzero (specifically, SCENE_EXPORT_SELECTED) value for options if the user
	// chose "Export Selected..." instead of "Export..." from the File menu
	//m_meshExporter.setExportSelected(options == SCENE_EXPORT_SELECTED);
	//m_meshXMLExporter.setExportSelected(options == SCENE_EXPORT_SELECTED);
	m_config.setExportSelected(options == SCENE_EXPORT_SELECTED);

	int result = DialogBoxParam(m_hInstance,
									MAKEINTRESOURCE(IDD_BINARY_EXPORT),
									GetActiveWindow(),
									ExportPropertiesDialogProc,
									(LPARAM) this);

	switch (result) {
		case 0:
			return IMPEXP_CANCEL;
			break;
		case 1:
			MessageBox(GetActiveWindow(), "Export Succeeded", "Sucessful Export", MB_ICONINFORMATION);
			return IMPEXP_SUCCESS;
			break;
		default:
			return IMPEXP_FAIL;
			break;
	}
}

INT_PTR CALLBACK ExportPropertiesDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {

	switch(message) {
		case WM_INITDIALOG:
			_exp = (OgreMaxExport*) lParam;

			if (_exp == 0) {
				MessageBox(NULL, "Error: Cannot initialize exporter options dialog, aborting", "Error", MB_ICONEXCLAMATION);
				EndDialog(hDlg, 0);
				return TRUE;
			}

			_exp->m_hWndDlgExport = hDlg;

			return _exp->setupExportProperties();
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
				case IDC_EXPORT:
					if (_exp->export())
						EndDialog(hDlg, 1);
					else
						EndDialog(hDlg, 2);
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlg, 0);
					return TRUE;
			}
			break;
		case WM_NOTIFY:
			switch(wParam) {
			case IDC_TABCONTROL:
				switch(((LPNMHDR)lParam)->code) {
				case TCN_SELCHANGE:
					_exp->onTabSelectChange(((LPNMHDR)lParam)->hwndFrom, ((LPNMHDR)lParam)->idFrom);
					break;
				}
				break;
			}
			break;
	}
	return FALSE;

}

bool OgreMaxExport::export() {

	// do mesh export -- either/both of XML and binary
	if (m_config.getExportXMLMesh()) {

		if (m_tabDisplay != 0)
			m_tabDisplay->update();

		OgreMax::MeshXMLExporter::OutputMap output;

		if (m_meshXMLExporter.buildMeshXML(output)) {
			m_config.save();

			// write each of the files and their contents
			OgreMax::MeshXMLExporter::OutputMap::iterator it = output.begin();

			while (it != output.end()) {
				std::ofstream of;
				of.open(it->first.c_str(), std::ios::out);
				of << it->second;
				of.close();
				it++;
			}

			// export material for this (these) mesh(es)
			/*
			if (m_config.getExportMaterial()) {

				std::string materialScript;

				std::ofstream of;
				if (m_materialExporter.buildMaterial(materialScript)) {
					of.open((m_config.getExportPath() + "\\" + m_config.getMaterialFilename()).c_str(), std::ios::out);

					if (of.is_open()) {
						of << materialScript;
						of.close();
					}
				}
			}
			*/
		}
		else {
		}
	}

	return true;
}

void OgreMaxExport::onTabSelectChange(HWND hWnd, INT id) {
	int iSel = TabCtrl_GetCurSel(hWnd);

	// clear any existing property page
	if (m_tabDisplay != NULL) {
		m_tabDisplay->update();
		DestroyWindow(m_tabDisplay->getDialogHandle());
	}

	// create a new property page
	switch(iSel) {
		case 0:
			CreateDialogParam(
				m_hInstance,
				MAKEINTRESOURCE(IDD_TAB_GENERAL),
				hWnd,
				GeneralTabDialogProc,
				(LPARAM)&m_tabGeneral);
			m_tabDisplay = &m_tabGeneral;
			break;
		case 1:
			CreateDialogParam(
				m_hInstance,
				MAKEINTRESOURCE(IDD_TAB_MESH),
				hWnd,
				MeshTabDialogProc,
				(LPARAM)&m_tabMesh);
			m_tabDisplay = &m_tabMesh;
			break;
		case 2:
			CreateDialogParam(
				m_hInstance,
				MAKEINTRESOURCE(IDD_TAB_SKELETAL_ANIMATION),
				hWnd,
				SkeletalAnimationTabDialogProc,
				(LPARAM)&m_tabSkeletalAnimation);
			m_tabDisplay = &m_tabSkeletalAnimation;
			break;
		case 3:
			CreateDialogParam(
				m_hInstance,
				MAKEINTRESOURCE(IDD_TAB_VERTEX_ANIMATION),
				hWnd,
				VertexAnimationTabDialogProc,
				(LPARAM)&m_tabVertexAnimation);
			m_tabDisplay = &m_tabVertexAnimation;
			break;
		case 4:
			CreateDialogParam(
				m_hInstance,
				MAKEINTRESOURCE(IDD_TAB_MATERIAL),
				hWnd,
				MaterialTabDialogProc,
				(LPARAM)&m_tabMaterial);
			m_tabDisplay = &m_tabMaterial;
			break;
	}
}

BOOL OgreMaxExport::setupExportProperties() {

	// add tabs to the tab control
	TCITEM tci;
	HWND tabCtrl = GetDlgItem(m_hWndDlgExport, IDC_TABCONTROL);

	tci.mask = TCIF_TEXT;
	tci.pszText = "General";
	TabCtrl_InsertItem(tabCtrl, 0, &tci);
	tci.pszText = "Mesh";
	TabCtrl_InsertItem(tabCtrl, 1, &tci);
	tci.pszText = "Skeletal Animation";
	TabCtrl_InsertItem(tabCtrl, 2, &tci);
	tci.pszText = "Vertex Animation";
	TabCtrl_InsertItem(tabCtrl, 3, &tci);
	tci.pszText = "Material";
	TabCtrl_InsertItem(tabCtrl, 4, &tci);

	// simulate tab select
	onTabSelectChange(tabCtrl, IDC_TABCONTROL);

	std::string filename;

	CenterWindow(m_hWndDlgExport,GetParent(m_hWndDlgExport));

	return TRUE;
}

int OgreMaxExport::ExtCount() {
	// support .xml, .mesh, .material, .skeleton
	return 4;
}

const TCHAR * OgreMaxExport::Ext(int n) {
	switch (n) {
		case 0:
			return _T("xml");
			break;
		case 1:
			return _T("mesh");
			break;
		case 2:
			return _T("skeleton");
			break;
		case 3:
			return _T("material");
			break;
		default:
			return 0;
			break;
	}
}

const TCHAR * OgreMaxExport::LongDesc() { 
	return _T("OGRE 3D Mesh/Animation/Material Exporter");
}

const TCHAR * OgreMaxExport::ShortDesc() {
	return _T("OGRE 3D Exporter");
}

const TCHAR * OgreMaxExport::AuthorName() { 
	return _T("Gregory 'Xavier' Junker");
}

const TCHAR * OgreMaxExport::CopyrightMessage() { 
	return _T("The OGRE 3D Team (c) 2006");
}

const TCHAR * OgreMaxExport::OtherMessage1() { 
	return 0;
}

const TCHAR * OgreMaxExport::OtherMessage2() { 
	return 0;
}

unsigned int OgreMaxExport::Version() { 
	return 122;
}

void OgreMaxExport::ShowAbout(HWND hWnd) {
	MessageBox(hWnd, "OGRE 3D Mesh, Material and Animation Exporter", "About", 0);
}

BOOL OgreMaxExport::SupportsOptions(int ext, DWORD options) {

	// currently, only SCENE_EXPORT_SELECTED is passed to this; we support exporting
	// of selected files only, so return TRUE (if they ever add anything later, we'll 
	// either support it too, or check what they are asking and return accordingly).
	return TRUE;
}

