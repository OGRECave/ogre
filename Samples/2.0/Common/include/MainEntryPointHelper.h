
#include <iostream>

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMainApp( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT );
#else
int mainApp();
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE hInst2, LPSTR strCmdLine, INT intParam )
#else
int main()
#endif
{
    int retVal = -1;
    try
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        retVal = WinMainApp( hInst, hInst2, strCmdLine, intParam );
#else
        retVal = mainApp();
#endif
    }
    catch( Ogre::Exception& e )
    {
   #if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!",
                    MB_OK | MB_ICONERROR | MB_TASKMODAL );
   #else
        std::cerr << "An exception has occured: " <<
                     e.getFullDescription().c_str() << std::endl;
   #endif
    }

    return retVal;
}
