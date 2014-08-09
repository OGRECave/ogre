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
#include "OgreStableHeaders.h"
#include "OgreConfigDialog.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreException.h"
#include "resource.h"

namespace
{
    Ogre::ConfigDialog* dlg;  // This is a pointer to instance, since this is a static member
}


namespace Ogre
{
    ConfigDialog::ConfigDialog()
    {
#ifdef OGRE_STATIC_LIB
		mHInstance = GetModuleHandle( NULL );
#else
        static const TCHAR staticVar;
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, &staticVar, &mHInstance);
#endif
        mSelectedRenderSystem = 0;
    }

	ConfigDialog::~ConfigDialog()
    {
	}

#if OGRE_ARCHITECTURE_64 == OGRE_ARCH_TYPE
    INT_PTR ConfigDialog::DlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
#else
    BOOL ConfigDialog::DlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
#endif
    {
        HWND hwndDlgItem;
        static const RenderSystemList* lstRend;
        RenderSystemList::const_iterator pRend;
        static ConfigOptionMap opts;
        String err;

        int i, sel, savedSel;

        switch (iMsg)
        {

        case WM_INITDIALOG:
            // Load saved settings
            dlg->mSelectedRenderSystem = Root::getSingleton().getRenderSystem();
            // Get all render systems
            lstRend = &Root::getSingleton().getAvailableRenderers();
            pRend = lstRend->begin();            
            i = 0;
            while (pRend != lstRend->end())
            {
                hwndDlgItem = GetDlgItem(hDlg, IDC_CBO_RENDERSYSTEM);

                SendMessage(hwndDlgItem, CB_ADDSTRING, 0,
                    (LPARAM)(char*)(*pRend)->getName().c_str());

                if (*pRend == dlg->mSelectedRenderSystem)
                {
                    // Select
                    SendMessage(hwndDlgItem, CB_SETCURSEL, (WPARAM)i, 0);
                    // Refresh Options
                    // Get options from render system
                    opts = (*pRend)->getConfigOptions();
                    // Reset list box
                    hwndDlgItem = GetDlgItem(hDlg, IDC_LST_OPTIONS);
                    //SendMessage(hwndDlgItem, LB_RESETCONTENT, 0, 0);
                    // Iterate through options
                    ConfigOptionMap::iterator pOpt = opts.begin();
                    String strLine;
                    while( pOpt!= opts.end() )
                    {
                        strLine = pOpt->second.name + ": " + pOpt->second.currentValue;
                        SendMessage(hwndDlgItem, LB_ADDSTRING, 0, (LPARAM)strLine.c_str());
                        ++pOpt;
                    }
                }

                ++pRend;
                ++i;
            }

            // Center myself
            int x, y, screenWidth, screenHeight;
            RECT rcDlg;
            GetWindowRect(hDlg, &rcDlg);
            screenWidth = GetSystemMetrics(SM_CXFULLSCREEN);
            screenHeight = GetSystemMetrics(SM_CYFULLSCREEN);

            x = (screenWidth / 2) - ((rcDlg.right - rcDlg.left) / 2);
            y = (screenHeight / 2) - ((rcDlg.bottom - rcDlg.top) / 2);

            MoveWindow(hDlg, x, y, (rcDlg.right - rcDlg.left),
                (rcDlg.bottom - rcDlg.top), TRUE);

            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case IDC_CBO_RENDERSYSTEM:
                hwndDlgItem = GetDlgItem(hDlg, IDC_CBO_RENDERSYSTEM);
                sel = SendMessage( hwndDlgItem, CB_GETCOUNT, 0, 0 );

                if (HIWORD(wParam) == CBN_SELCHANGE )
                {
                    // RenderSystem selected
                    // Get selected index
                    hwndDlgItem = GetDlgItem(hDlg, IDC_CBO_RENDERSYSTEM);
                    sel = SendMessage(hwndDlgItem, CB_GETCURSEL,0,0);
                    if (sel != -1)
                    {
                        // Get RenderSystem selected
                        pRend = lstRend->begin();
                        dlg->mSelectedRenderSystem = pRend[sel];
                        // refresh options
                        // Get options from render system
                        opts = pRend[sel]->getConfigOptions();
                        // Reset list box
                        hwndDlgItem = GetDlgItem(hDlg, IDC_LST_OPTIONS);
                        SendMessage(hwndDlgItem, LB_RESETCONTENT, 0, 0);
                        // Iterate through options
                        ConfigOptionMap::iterator pOpt = opts.begin();
                        String strLine;
                        while (pOpt!=opts.end())
                        {
                            strLine = pOpt->second.name + ": " + pOpt->second.currentValue;
                            SendMessage(hwndDlgItem, LB_ADDSTRING, 0, (LPARAM)strLine.c_str());
                            ++pOpt;
                        }
                    }                    
                }

                return TRUE;

            case IDC_LST_OPTIONS:
                if (HIWORD(wParam) == LBN_SELCHANGE)
                {
                    // Selection in list box of options changed
                    // Update combo and label in edit section
                    hwndDlgItem = GetDlgItem(hDlg, IDC_LST_OPTIONS);
                    sel = SendMessage(hwndDlgItem, LB_GETCURSEL, 0, 0);
                    if (sel != -1)
                    {
                        ConfigOptionMap::iterator pOpt = opts.begin();
                        for (i = 0; i < sel; i++)
                            ++pOpt;
                        // Set label text
                        hwndDlgItem = GetDlgItem(hDlg, IDC_LBL_OPTION);
                        SetWindowText(hwndDlgItem, pOpt->second.name.c_str());
                        // Set combo options
                        hwndDlgItem = GetDlgItem(hDlg, IDC_CBO_OPTION);
                        SendMessage(hwndDlgItem, CB_RESETCONTENT, 0, 0);
                        StringVector::iterator pPoss = pOpt->second.possibleValues.begin();
                        i = 0;
                        while (pPoss!=pOpt->second.possibleValues.end())
                        {
                            SendMessage(hwndDlgItem, CB_ADDSTRING, 0, (LPARAM)pPoss[0].c_str());
                            if (pPoss[0] == pOpt->second.currentValue)
                                // Select current value
                                SendMessage(hwndDlgItem, CB_SETCURSEL, (WPARAM)i, 0);
                            ++pPoss;
                            ++i;
                        }
                        // Enable/disable combo depending on (not)immutable
                        EnableWindow(hwndDlgItem, !(pOpt->second.immutable));
                    }
                }

                return TRUE;

            case IDC_CBO_OPTION:
                if (HIWORD(wParam) == CBN_SELCHANGE)
                {
                    // Updated an option
                    // Get option
                    hwndDlgItem = GetDlgItem(hDlg, IDC_LST_OPTIONS);
                    sel = SendMessage(hwndDlgItem, LB_GETCURSEL, 0, 0);
                    savedSel = sel;
                    ConfigOptionMap::iterator pOpt = opts.begin();
                    for (i = 0; i < sel; i++)
                        ++pOpt;
                    // Get selected value
                    hwndDlgItem = GetDlgItem(hDlg, IDC_CBO_OPTION);
                    sel = SendMessage(hwndDlgItem, CB_GETCURSEL, 0, 0);

                    if (sel != -1)
                    {
                        StringVector::iterator pPoss = pOpt->second.possibleValues.begin();

                        // Set option
                        dlg->mSelectedRenderSystem->setConfigOption(
                            pOpt->second.name, pPoss[sel]);
                        // Re-retrieve options
                        opts = dlg->mSelectedRenderSystem->getConfigOptions();

                        // Reset options list box
                        hwndDlgItem = GetDlgItem(hDlg, IDC_LST_OPTIONS);
                        SendMessage(hwndDlgItem, LB_RESETCONTENT, 0, 0);
                        // Iterate through options
                        pOpt = opts.begin();
                        String strLine;
                        while (pOpt!=opts.end())
                        {
                            strLine = pOpt->second.name + ": " + pOpt->second.currentValue;
                            SendMessage(hwndDlgItem, LB_ADDSTRING, 0, (LPARAM)strLine.c_str());
                            ++pOpt;
                        }
                        // Select previously selected item
                        SendMessage(hwndDlgItem, LB_SETCURSEL, savedSel, 0);
                    }

                }
                return TRUE;

            case IDOK:
                // Set render system
                if (!dlg->mSelectedRenderSystem)
                {
                    MessageBox(NULL, "Please choose a rendering system.", "OGRE", MB_OK | MB_ICONEXCLAMATION);
                    return TRUE;
                }
                err = dlg->mSelectedRenderSystem->validateConfigOptions();
                if (err.length() > 0)
                {
                    // refresh options incase updated by validation
                    // Get options from render system
                    opts = dlg->mSelectedRenderSystem->getConfigOptions();
                    // Reset list box
                    hwndDlgItem = GetDlgItem(hDlg, IDC_LST_OPTIONS);
                    SendMessage(hwndDlgItem, LB_RESETCONTENT, 0, 0);
                    // Iterate through options
                    ConfigOptionMap::iterator pOpt = opts.begin();
                    String strLine;
                    while (pOpt!=opts.end())
                    {
                        strLine = pOpt->second.name + ": " + pOpt->second.currentValue;
                        SendMessage(hwndDlgItem, LB_ADDSTRING, 0, (LPARAM)strLine.c_str());
                        ++pOpt;
                    }
                    MessageBox(NULL, err.c_str(), "OGRE", MB_OK | MB_ICONEXCLAMATION);
                    return TRUE;
                }

                Root::getSingleton().setRenderSystem(dlg->mSelectedRenderSystem);

                EndDialog(hDlg, TRUE);
                return TRUE;

            case IDCANCEL:
                EndDialog(hDlg, FALSE);
                return TRUE;
            }
        }

        return FALSE;
    }


    bool ConfigDialog::display(void)
    {
        // Display dialog
        // Don't return to caller until dialog dismissed
        int i;
        dlg = this;
        
        i = DialogBox(mHInstance, MAKEINTRESOURCE(IDD_DLG_CONFIG), NULL, DlgProc);

        if (i == -1)
        {
            int winError = GetLastError();
            char* errDesc;
            int i;

            errDesc = new char[255];
            // Try windows errors first
            i = FormatMessage(
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                winError,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                (LPTSTR) errDesc,
                255,
                NULL
            );

            throw Exception(winError,errDesc, "ConfigDialog::display");
        }
        if (i)
            return true;
        else
            return false;

    }
}
