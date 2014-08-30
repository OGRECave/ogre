/**********************************************************************
 *<
    FILE: DllEntry.cpp

    DESCRIPTION:Contains the Dll Entry stuff

    CREATED BY: 

    HISTORY: 

 *> Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/
#include "PhyExport.h"

extern ClassDesc2* GetPhyExportDesc();

HINSTANCE hInstance;
int controlsInit = FALSE;


BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
    hInstance = hinstDLL;               // Hang on to this DLL's instance handle.

    if (!controlsInit) {
        controlsInit = TRUE;
        InitCustomControls(hInstance);  // Initialize MAX's custom controls
        InitCommonControls();           // Initialize Win95 controls
    }
            
    return (TRUE);
}

__declspec( dllexport ) const TCHAR* LibDescription()
{
    return GetString(IDS_LIBDESCRIPTION);
}

//TODO: Must change this number when adding a new class
__declspec( dllexport ) int LibNumberClasses()
{
    return 1;
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
    switch(i) {
        case 0: return GetPhyExportDesc();
        default: return 0;
    }
}

__declspec( dllexport ) ULONG LibVersion()
{
    return VERSION_3DSMAX;
}

TCHAR *GetString(int id)
{
    static TCHAR buf[256];

    if (hInstance)
        return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
    return NULL;
}

