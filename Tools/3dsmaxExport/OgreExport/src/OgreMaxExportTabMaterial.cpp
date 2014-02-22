#include "OgreExport.h"
#include "resource.h"

static OgreMaxExport_Material* sInst;

void OgreMaxExport_Material::onInitDialog(HWND hDlg) {
    OgreMaxExport_TabPaneHandler::onInitDialog(hDlg);
}

void OgreMaxExport_Material::onDestroy() {
    update();
}

// read the contents from the dialog controls
void OgreMaxExport_Material::update() {

}

// for the sake of sanity, keep the dlgproc and the handler class implementation here in the same source file
INT_PTR CALLBACK MaterialTabDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {

    switch(message) {
    case WM_INITDIALOG:
        sInst = (OgreMaxExport_Material*) lParam;

        sInst->onInitDialog(hDlg);
        SetWindowPos(hDlg, HWND_TOP, 10, 40, 0, 0, SWP_NOSIZE);
        ShowWindow(hDlg, SW_SHOW);
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        }
        break;

    case WM_DESTROY:
        sInst->onDestroy();
        break;
    }
    return FALSE;
}
