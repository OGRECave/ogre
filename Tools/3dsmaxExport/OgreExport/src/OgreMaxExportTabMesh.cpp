#include "OgreExport.h"
#include "resource.h"

static OgreMaxExport_Mesh* sInst;

void OgreMaxExport_Mesh::onInitDialog(HWND hDlg) {
    OgreMaxExport_TabPaneHandler::onInitDialog(hDlg);

    // set tab pane initial values
    CheckDlgButton(hDlg, IDC_RADIO_EXPORT_SUBMESHES, m_config.getExportMultipleFiles() ? BST_UNCHECKED : BST_CHECKED);
    CheckDlgButton(hDlg, IDC_RADIO_EXPORT_FILES, m_config.getExportMultipleFiles() ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHK_SHARE_SKELETON, m_config.getUseSingleSkeleton() ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHK_REBUILD_NORMALS, m_config.getRebuildNormals() ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHK_INVERT_NORMALS, m_config.getInvertNormals() ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHK_VERTEX_COLORS, m_config.getExportVertexColours() ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHK_MERGE_MESHES, m_config.getMergeMeshes() ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHK_BINARY_MESH, m_config.getExportBinaryMesh() ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHK_GENERATE_TANGENTS, m_config.getGenerateTangents() ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHK_GENERATE_EDGELISTS, m_config.getBuildEdgeLists() ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHK_OPTIMIZE, m_config.getOptmizeBinaryMesh() ? BST_CHECKED : BST_UNCHECKED);

    SendMessage(GetDlgItem(hDlg, IDC_COLOR_FORMAT), CB_ADDSTRING, 0, (LPARAM)_T("Direct3D"));
    SendMessage(GetDlgItem(hDlg, IDC_COLOR_FORMAT), CB_SETITEMDATA, 0, (LPARAM)(DWORD)OgreMax::DIRECT3D);
    SendMessage(GetDlgItem(hDlg, IDC_COLOR_FORMAT), CB_ADDSTRING, 0, (LPARAM)_T("OpenGL"));
    SendMessage(GetDlgItem(hDlg, IDC_COLOR_FORMAT), CB_SETITEMDATA, 1, (LPARAM)(DWORD)OgreMax::OPENGL);

    SendMessage(GetDlgItem(hDlg, IDC_ENDIAN), CB_ADDSTRING, 0, (LPARAM)_T("Native"));
    SendMessage(GetDlgItem(hDlg, IDC_ENDIAN), CB_SETITEMDATA, 0, (LPARAM)(DWORD)OgreMax::NATIVE);
    SendMessage(GetDlgItem(hDlg, IDC_ENDIAN), CB_ADDSTRING, 0, (LPARAM)_T("Big Endian"));
    SendMessage(GetDlgItem(hDlg, IDC_ENDIAN), CB_SETITEMDATA, 1, (LPARAM)(DWORD)OgreMax::BIG);
    SendMessage(GetDlgItem(hDlg, IDC_ENDIAN), CB_ADDSTRING, 0, (LPARAM)_T("Little Endian"));
    SendMessage(GetDlgItem(hDlg, IDC_ENDIAN), CB_SETITEMDATA, 2, (LPARAM)(DWORD)OgreMax::LITTLE);

    SendMessage(GetDlgItem(hDlg, IDC_COLOR_FORMAT), CB_SETCURSEL, (WPARAM)m_config.getVertexColourFormat(), 0);
    SendMessage(GetDlgItem(hDlg, IDC_ENDIAN), CB_SETCURSEL, (WPARAM)m_config.getEndian(), 0);

    CheckDlgButton(hDlg, IDC_CHK_GENERATE_LOD, m_config.getGenerateLod() ? BST_CHECKED : BST_UNCHECKED);

    std::stringstream ss;
    ss << m_config.getNumLodLevels();
    SetWindowText(GetDlgItem(hDlg, IDC_LOD_NUM_LEVELS), ss.str().c_str());

    ss.str("");
    ss << m_config.getLodDistance();
    SetWindowText(GetDlgItem(hDlg, IDC_LOD_DISTANCE), ss.str().c_str());

    ss.str("");
    ss << m_config.getLodPercentReduction();
    SetWindowText(GetDlgItem(hDlg, IDC_LOD_PERCENT), ss.str().c_str());

    ss.str("");
    ss << m_config.getLodVertexReduction();
    SetWindowText(GetDlgItem(hDlg, IDC_LOD_VERTEX_COUNT), ss.str().c_str());

    // disable controls that are not "on" in particular modes
    setControlStates();
}

void OgreMaxExport_Mesh::onDestroy() {
    update();
}

// read the contents from the dialog controls
void OgreMaxExport_Mesh::update() {

    // get tab pane values
    m_config.setExportMultipleFiles(BST_CHECKED == IsDlgButtonChecked(m_hDlg, IDC_RADIO_EXPORT_FILES));
    m_config.setUseSingleSkeleton(BST_CHECKED == IsDlgButtonChecked(m_hDlg, IDC_CHK_SHARE_SKELETON));
    m_config.setRebuildNormals(BST_CHECKED == IsDlgButtonChecked(m_hDlg, IDC_CHK_REBUILD_NORMALS));
    m_config.setInvertNormals(BST_CHECKED == IsDlgButtonChecked(m_hDlg, IDC_CHK_INVERT_NORMALS));
    m_config.setExportVertexColours(BST_CHECKED == IsDlgButtonChecked(m_hDlg, IDC_CHK_VERTEX_COLORS));
    m_config.setMergeMeshes(BST_CHECKED == IsDlgButtonChecked(m_hDlg, IDC_CHK_MERGE_MESHES));
    m_config.setExportBinaryMesh(BST_CHECKED == IsDlgButtonChecked(m_hDlg, IDC_CHK_BINARY_MESH));
    m_config.setGenerateTangents(BST_CHECKED == IsDlgButtonChecked(m_hDlg, IDC_CHK_GENERATE_TANGENTS));
    m_config.setBuildEdgeLists(BST_CHECKED == IsDlgButtonChecked(m_hDlg, IDC_CHK_GENERATE_EDGELISTS));
    m_config.setOptmizeBinaryMesh(BST_CHECKED == IsDlgButtonChecked(m_hDlg, IDC_CHK_OPTIMIZE));

    LRESULT item = SendMessage(GetDlgItem(m_hDlg, IDC_COLOR_FORMAT), CB_GETCURSEL, 0, 0);
    m_config.setVertexColourFormat((OgreMax::VertexColourFormat)SendMessage(GetDlgItem(m_hDlg, IDC_COLOR_FORMAT), CB_GETITEMDATA, item, 0));
    item = SendMessage(GetDlgItem(m_hDlg, IDC_ENDIAN), CB_GETCURSEL, 0, 0);
    m_config.setEndian((OgreMax::Endian)SendMessage(GetDlgItem(m_hDlg, IDC_ENDIAN), CB_GETITEMDATA, item, 0));

    m_config.setGenerateLod(BST_CHECKED == IsDlgButtonChecked(m_hDlg, IDC_CHK_GENERATE_LOD));

    char buf[16];
    GetWindowText(GetDlgItem(m_hDlg, IDC_LOD_NUM_LEVELS), buf, 16);
    m_config.setNumLodLevels(atoi(buf));
    GetWindowText(GetDlgItem(m_hDlg, IDC_LOD_DISTANCE), buf, 16);
    m_config.setLodDistance(atof(buf));
    GetWindowText(GetDlgItem(m_hDlg, IDC_LOD_PERCENT), buf, 16);
    m_config.setLodPercentReduction(atof(buf));
    GetWindowText(GetDlgItem(m_hDlg, IDC_LOD_VERTEX_COUNT), buf, 16);
    m_config.setLodVertexReduction(atoi(buf));
}

void OgreMaxExport_Mesh::onClickBinaryMesh() {
    m_config.setExportBinaryMesh(BST_CHECKED == IsDlgButtonChecked(m_hDlg, IDC_CHK_BINARY_MESH));
    setControlStates();
}

// set control enabled/disabled states
void OgreMaxExport_Mesh::setControlStates() {

    BOOL enable = FALSE;
    if (m_config.getExportBinaryMesh()) {
        enable = TRUE;
    }

    EnableWindow(GetDlgItem(m_hDlg, IDC_CHK_GENERATE_TANGENTS), enable);
    EnableWindow(GetDlgItem(m_hDlg, IDC_CHK_GENERATE_EDGELISTS), enable);
    EnableWindow(GetDlgItem(m_hDlg, IDC_CHK_OPTIMIZE), enable);
    EnableWindow(GetDlgItem(m_hDlg, IDC_LOD_USE_PERCENT), enable);
    EnableWindow(GetDlgItem(m_hDlg, IDC_LOD_USE_VERTEX), enable);
    EnableWindow(GetDlgItem(m_hDlg, IDC_COLOR_FORMAT), enable);
    EnableWindow(GetDlgItem(m_hDlg, IDC_ENDIAN), enable);
    EnableWindow(GetDlgItem(m_hDlg, IDC_CHK_GENERATE_LOD), enable);
    EnableWindow(GetDlgItem(m_hDlg, IDC_LOD_NUM_LEVELS), enable);
    EnableWindow(GetDlgItem(m_hDlg, IDC_LOD_DISTANCE), enable);
    EnableWindow(GetDlgItem(m_hDlg, IDC_LOD_PERCENT), enable);
    EnableWindow(GetDlgItem(m_hDlg, IDC_LOD_VERTEX_COUNT), enable);
}

// for the sake of sanity, keep the dlgproc and the handler class implementation here in the same source file
INT_PTR CALLBACK MeshTabDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {

    switch(message) {
    case WM_INITDIALOG:
        sInst = (OgreMaxExport_Mesh*) lParam;

        sInst->onInitDialog(hDlg);
        SetWindowPos(hDlg, HWND_TOP, 10, 40, 0, 0, SWP_NOSIZE);
        ShowWindow(hDlg, SW_SHOW);
        break;

    case WM_COMMAND:
        switch(HIWORD(wParam)) {
        case BN_CLICKED:
            switch(LOWORD(wParam)) {
            case IDC_CHK_BINARY_MESH:
                sInst->onClickBinaryMesh();
                break;
            }
            break;
        }
        break;

    case WM_DESTROY:
        sInst->onDestroy();
        break;
    }
    return FALSE;
}
