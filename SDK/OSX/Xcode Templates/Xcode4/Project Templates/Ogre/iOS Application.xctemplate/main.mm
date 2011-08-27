//|||||||||||||||||||||||||||||||||||||||||||||||

#include "OgreDemoApp.h"
#include "AppDelegate.h"

//|||||||||||||||||||||||||||||||||||||||||||||||

#if OGRE_PLATFORM == PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

//|||||||||||||||||||||||||||||||||||||||||||||||

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
int main(int argc, char **argv)
#endif
{
#if defined(OGRE_IS_IOS)
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	int retVal = UIApplicationMain(argc, argv, @"UIApplication", @"AppDelegate");
	[pool release];
	return retVal;
#elif (OGRE_PLATFORM == OGRE_PLATFORM_APPLE) && __LP64__
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    
    mAppDelegate = [[AppDelegate alloc] init];
    [[NSApplication sharedApplication] setDelegate:mAppDelegate];
	int retVal = NSApplicationMain(argc, (const char **) argv);
    
	[pool release];
    
	return retVal;
#else
    
	try
    {
		DemoApp demo;
		demo.startDemo();
    }
	catch(std::exception& e)
    {
#if OGRE_PLATFORM == PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBoxA(NULL, e.what(), "An exception has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        fprintf(stderr, "An exception has occurred: %s\n", e.what());
#endif
    }
    
#endif
    return 0;
}

//|||||||||||||||||||||||||||||||||||||||||||||||
