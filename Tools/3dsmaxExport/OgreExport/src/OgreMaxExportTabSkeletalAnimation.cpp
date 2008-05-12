#include "OgreExport.h"
#include "resource.h"

static OgreMaxExport_SkeletalAnimation* sInst;

void OgreMaxExport_SkeletalAnimation::onInitDialog(HWND hDlg) {
	OgreMaxExport_TabPaneHandler::onInitDialog(hDlg);

	// set up animation listbox
	int frameStart = m_i->GetAnimRange().Start();
	int frameEnd = m_i->GetAnimRange().End();

	HWND anims = GetDlgItem(m_hDlg, IDC_LIST_ANIMATIONS);

	Rect r;
	GetWindowRect(anims, &r);

	LVCOLUMN lvc;
	ZeroMemory(&lvc, sizeof(LVCOLUMN));
	lvc.mask = LVCF_TEXT | LVCF_WIDTH;
	lvc.cx = r.w() * 0.6;
	lvc.pszText = "Animation";
	ListView_InsertColumn(anims, 0, &lvc);
	lvc.cx = r.w() * 0.2;
	lvc.pszText = "Begin";
	ListView_InsertColumn(anims, 1, &lvc);
	lvc.pszText = "End";
	ListView_InsertColumn(anims, 2, &lvc);

	// add a spanning entry to the animation list as a default
	LVITEM lvi;
	char buf[32];
	ZeroMemory(&lvi, sizeof(LVITEM));

	lvi.mask = LVIF_TEXT;
	lvi.pszText = "Animation";
	lvi.iItem = 10000;
	int idx = ListView_InsertItem(anims, &lvi);

	sprintf(buf, "%d", frameStart / GetTicksPerFrame());
	lvi.iItem = idx;
	lvi.iSubItem = 1;
	lvi.pszText = buf;
	ListView_SetItem(anims, &lvi);

	sprintf(buf, "%d", frameEnd / GetTicksPerFrame());
	lvi.iSubItem = 2;
	lvi.pszText = buf;
	ListView_SetItem(anims, &lvi);

	// populate the frame range info box
	sprintf(buf, "%d to %d", frameStart / GetTicksPerFrame(), frameEnd / GetTicksPerFrame());
	SendMessage(GetDlgItem(m_hDlg, IDC_TXT_FRAME_RANGE), WM_SETTEXT, 0, (LPARAM)buf);
	SendMessage(GetDlgItem(m_hDlg, IDC_TXT_FPS), WM_SETTEXT, 0, (LPARAM)_T("1.0"));
}

void OgreMaxExport_SkeletalAnimation::onDestroy() {
	update();
}

// read the contents from the dialog controls
void OgreMaxExport_SkeletalAnimation::update() {

}

void OgreMaxExport_SkeletalAnimation::onAddAnimation() {
	addAnimation();
}

void OgreMaxExport_SkeletalAnimation::onDeleteAnimation() {
	deleteAnimation();
}

void OgreMaxExport_SkeletalAnimation::addAnimation() {
	char buf[256];
	int start, end;
	HWND anims = GetDlgItem(m_hDlg, IDC_LIST_ANIMATIONS);

	SendMessage(GetDlgItem(m_hDlg, IDC_TXT_FPS), WM_GETTEXT, 256, (LPARAM)buf);
	float fps = atof(buf);

	if (fps <= 0.0) {
		MessageBox(NULL, "FPS must be >= 0.0", "Invalid Entry", MB_ICONEXCLAMATION);
		return;
	}

	int minAnimTime = m_i->GetAnimRange().Start() / GetTicksPerFrame();
	int maxAnimTime = m_i->GetAnimRange().End() / GetTicksPerFrame();

	// get animation start and end times
	SendMessage(GetDlgItem(m_hDlg, IDC_TXT_ANIM_START), WM_GETTEXT, 256, (LPARAM)buf);
	start = atoi(buf);

	if (start < minAnimTime) {
		sprintf(buf, "Start time must be >= %d", start);
		MessageBox(NULL, buf, "Invalid Entry", MB_ICONEXCLAMATION);
		return;
	}

	SendMessage(GetDlgItem(m_hDlg, IDC_TXT_ANIM_END), WM_GETTEXT, 256, (LPARAM)buf);
	end = atoi(buf);

	if (end > maxAnimTime) {
		sprintf(buf, "End time must be <= %d", end);
		MessageBox(NULL, buf, "Invalid Entry", MB_ICONEXCLAMATION);
		return;
	}

	// get animation name
	SendMessage(GetDlgItem(m_hDlg, IDC_TXT_ANIMATION_NAME), WM_GETTEXT, 256, (LPARAM)buf);
	std::string name(buf);

	if (name.length() == 0) {
		MessageBox(NULL, "Animation name must not be empty", "Invalid Entry", MB_ICONEXCLAMATION);
		return;
	}

	// if, after all that, we have valid data, stick it in the listview
	LVITEM lvi;
	ZeroMemory(&lvi, sizeof(LVITEM));

	lvi.mask = LVIF_TEXT;
	lvi.pszText = buf;
	lvi.iItem = 10000;
	int idx = ListView_InsertItem(anims, &lvi);

	lvi.iItem = idx;
	lvi.iSubItem = 1;
	sprintf(buf, "%d", start);
	lvi.pszText = buf;
	ListView_SetItem(anims, &lvi);
	lvi.iSubItem = 2;
	sprintf(buf, "%d", end);
	lvi.pszText = buf;
	ListView_SetItem(anims, &lvi);

	// Finally, clear out the entry controls
	SetWindowText(GetDlgItem(m_hDlg, IDC_TXT_ANIMATION_NAME), "");
	SetWindowText(GetDlgItem(m_hDlg, IDC_TXT_ANIM_START), "");
	SetWindowText(GetDlgItem(m_hDlg, IDC_TXT_ANIM_END), "");
}

void OgreMaxExport_SkeletalAnimation::deleteAnimation() {

	HWND anims = GetDlgItem(m_hDlg, IDC_LIST_ANIMATIONS);

	// delete selected animation(s) from the listview
	int idx;
	while ((idx=ListView_GetNextItem(anims, -1, LVNI_SELECTED)) != -1)
		ListView_DeleteItem(anims, idx);
}

// for the sake of sanity, keep the dlgproc and the handler class implementation here in the same source file
INT_PTR CALLBACK SkeletalAnimationTabDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {

	switch(message) {
	case WM_INITDIALOG:
		sInst = (OgreMaxExport_SkeletalAnimation*) lParam;

		sInst->onInitDialog(hDlg);
		SetWindowPos(hDlg, HWND_TOP, 10, 40, 0, 0, SWP_NOSIZE);
		ShowWindow(hDlg, SW_SHOW);
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_CMD_DELETE_ANIMATION:
			sInst->onDeleteAnimation();
			break;
		case IDC_CMD_ADD_ANIMATION:
			sInst->onAddAnimation();
			break;
		}
		break;

	case WM_NOTIFY:
		break;

	case WM_DESTROY:
		sInst->onDestroy();
		break;
	}
	return FALSE;
}
