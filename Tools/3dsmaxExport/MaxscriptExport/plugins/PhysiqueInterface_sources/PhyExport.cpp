/**********************************************************************
 *<
    FILE: PhyExport.cpp

    DESCRIPTION:    Appwizard generated plugin

    CREATED BY: 

    HISTORY: 

 *> Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "PhyExport.h"

#define PHYEXPORT_CLASS_ID  Class_ID(0x3dc3c42e, 0x185dc5f2)



class PhyExport : public GUP {
    public:
        static HWND hParams;

        // GUP Methods
        DWORD   Start           ( );
        void    Stop            ( );
        DWORD   Control         ( DWORD parameter );
        
        // Loading/Saving
        IOResult Save(ISave *isave);
        IOResult Load(ILoad *iload);

        
        //Constructor/Destructor

        PhyExport();
        ~PhyExport();       

};


class PhyExportClassDesc:public ClassDesc2 {
    public:
    int             IsPublic() { return 1; }
    void *          Create(BOOL loading = FALSE) { return new PhyExport(); }
    const TCHAR *   ClassName() { return GetString(IDS_CLASS_NAME); }
    SClass_ID       SuperClassID() { return GUP_CLASS_ID; }
    Class_ID        ClassID() { return PHYEXPORT_CLASS_ID; }
    const TCHAR*    Category() { return GetString(IDS_CATEGORY); }

    const TCHAR*    InternalName() { return _T("PhyExport"); }  // returns fixed parsable name (scripter-visible name)
    HINSTANCE       HInstance() { return hInstance; }               // returns owning module handle

};


static PhyExportClassDesc PhyExportDesc;
ClassDesc2* GetPhyExportDesc() { return &PhyExportDesc; }



PhyExport::PhyExport()
{

}

PhyExport::~PhyExport()
{

}

// Activate and Stay Resident
DWORD PhyExport::Start( ) {
    
    // TODO: Do plugin initialization here
    
    // TODO: Return if you want remain loaded or not
    return GUPRESULT_KEEP;
}

void PhyExport::Stop( ) {
    // TODO: Do plugin un-initialization here
}

DWORD PhyExport::Control( DWORD parameter ) {
    return 0;
}

IOResult PhyExport::Save(ISave *isave)
{
    return IO_OK;
}

IOResult PhyExport::Load(ILoad *iload)
{
    return IO_OK;
}

