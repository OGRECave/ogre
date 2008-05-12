#include "OgreExport.h"
#include "resource.h"

static OgreMaxExport_General* sInst;

void OgreMaxExport_General::onInitDialog(HWND hDlg) {
	OgreMaxExport_TabPaneHandler::onInitDialog(hDlg);

	// set the initial dialog state
	std::stringstream ss;
	SetWindowText(GetDlgItem(m_hDlg, IDC_TXT_EXPORT_DIR), m_config.getExportPath().c_str());
	SendMessage(GetDlgItem(m_hDlg, IDC_CHK_FLIP_YZ), BM_SETCHECK, (m_config.getInvertYZ() ? BST_CHECKED : BST_UNCHECKED), 0);

	ss << m_config.getScale();
	SetWindowText(GetDlgItem(m_hDlg, IDC_TXT_SCALE), ss.str().c_str());
}

void OgreMaxExport_General::onInvertYZ(bool checked) {
	m_config.setInvertYZ(checked);
}

void OgreMaxExport_General::onSetScale(float newScale) {
}

void OgreMaxExport_General::onExportDirectoryChanged(const std::string& newDirectory) {
	m_config.setExportPath(newDirectory);
}

void OgreMaxExport_General::onDestroy() {
	update();
}

// read the contents from the dialog controls
void OgreMaxExport_General::update() {

	char buf[64];

	// export path is not settable in the dialog at this time, skip it

	// get the scale and invert-YZ settings
	m_config.setInvertYZ(BST_CHECKED==IsDlgButtonChecked(m_hDlg, IDC_CHK_FLIP_YZ));

	// we really can only do this here unless we want to process every keystroke in the scale edit box
	GetWindowText(GetDlgItem(m_hDlg, IDC_TXT_SCALE), buf, 64);
	float scale = atof(buf);

	if (scale == 0.0f) //sets to 0.0 if the text cannot be converted; change to 1.0 in that case
		scale = 1.0f; 

	m_config.setScale(scale); 
}

// for the sake of sanity, keep the dlgproc and the handler class implementation here in the same source file
INT_PTR CALLBACK GeneralTabDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {

	switch(message) {
	case WM_INITDIALOG:
		sInst = (OgreMaxExport_General*) lParam;

		sInst->onInitDialog(hDlg);
		SetWindowPos(hDlg, HWND_TOP, 10, 40, 0, 0, SWP_NOSIZE);
		ShowWindow(hDlg, SW_SHOW);
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_SELECT_EXPORT_DIR:
			switch(HIWORD(wParam)) {
			case BN_CLICKED:
				break;

			}
			break;
		case IDC_CHK_FLIP_YZ:
			sInst->onInvertYZ(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_CHK_FLIP_YZ));	
			break;
		}
		break;

	case WM_DESTROY:
		sInst->onDestroy();
		break;
	}
	return FALSE;
}

