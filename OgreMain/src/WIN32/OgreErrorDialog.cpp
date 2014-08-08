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
#include "OgreErrorDialog.h"
#include "resource.h"

namespace {
    Ogre::ErrorDialog* errdlg;  // This is a pointer to instance, since this is a static member
}

namespace Ogre
{
    ErrorDialog::ErrorDialog()
    {
#ifdef OGRE_STATIC_LIB
		mHInstance = GetModuleHandle( NULL );
#else
        static const TCHAR staticVar;
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, &staticVar, &mHInstance);
#endif
    }

#if OGRE_ARCHITECTURE_64 == OGRE_ARCH_TYPE
    INT_PTR CALLBACK ErrorDialog::DlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
#else
    BOOL CALLBACK ErrorDialog::DlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
#endif
    {
        HWND hwndDlgItem;

        switch (iMsg)
        {

        case WM_INITDIALOG:
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

            // Fill in details of error
            hwndDlgItem = GetDlgItem(hDlg, IDC_ERRMSG);
            SetWindowText(hwndDlgItem, errdlg->mErrorMsg.c_str());

            return TRUE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case IDOK:

                EndDialog(hDlg, TRUE);
                return TRUE;
            }
        }

        return FALSE;

    }


    void ErrorDialog::display(const String& errorMessage, String logName)
    {
        // Display dialog
        // Don't return to caller until dialog dismissed
        errdlg = this;
        mErrorMsg = errorMessage;
        DialogBox(mHInstance, MAKEINTRESOURCE(IDD_DLG_ERROR), NULL, DlgProc);


    }
}
