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
#include "OgrePlatform.h"

// Sadly we needed to add this #if to solve a NACL compiler bug...
#if (OGRE_PLATFORM == OGRE_PLATFORM_NACL) 
#include "ppapi/utility/completion_callback_factory.h"
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "OgreString.h"
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#include "SampleBrowser_OSX.h"
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
#include "SampleBrowser_iOS.h"
#elif OGRE_PLATFORM == OGRE_PLATFORM_NACL
#include "SampleBrowser_NaCl.h"
#elif OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
#include "SampleBrowser_Android.h"

SampleBrowser* OgreAndroidBridge::mBrowser = NULL;
AndroidInputInjector* OgreAndroidBridge::mInputInjector = NULL;
AndroidMultiTouch* OgreAndroidBridge::mTouch = NULL;
AndroidKeyboard* OgreAndroidBridge::mKeyboard = NULL;
Ogre::RenderWindow* OgreAndroidBridge::mRenderWnd = NULL;
Ogre::Root* OgreAndroidBridge::mRoot = NULL;
bool OgreAndroidBridge::mInit = false;

#   ifdef OGRE_STATIC_LIB
StaticPluginLoader* OgreAndroidBridge::mStaticPluginLoader = NULL;
#   endif

#endif

#include "SampleBrowser.h"

#if OGRE_PLATFORM != OGRE_PLATFORM_NACL

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR cmdLine, INT) {
    int argc = __argc;
    char** argv = __argv;
#elif OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
void android_main(struct android_app* state) {
#else
int main(int argc, char *argv[]) {
#endif
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
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
#elif OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
    // Make sure glue isn't stripped.
    app_dummy();
    
    OgreAndroidBridge::init(state);
    OgreAndroidBridge::go(state);
#else

	try
	{
        bool nograb = false;
        if (argc >= 2 && Ogre::String(argv[1]) == "nograb")
            nograb = true;

        int startUpSampleIdx = -1;
        if (argc >= 3)
        {
            startUpSampleIdx = Ogre::StringConverter::parseInt(Ogre::String(argv[2]), -1);
        }
        else if (argc >= 2)
        {
            // first parameter can be either nograb or index. in the former case, we'll just
            // get -1, which is fine.
            startUpSampleIdx = Ogre::StringConverter::parseInt(Ogre::String(argv[1]), -1);
        }
		OgreBites::SampleBrowser brows (nograb, startUpSampleIdx);
		brows.go();
	}
	catch (Ogre::Exception& e)
	{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		MessageBoxA(NULL, e.getFullDescription().c_str(), "An exception has occurred!", MB_ICONERROR | MB_TASKMODAL);
#else
		std::cerr << "An exception has occurred: " << e.getFullDescription().c_str() << std::endl;
#endif
	}
	return 0;
#endif
}

#endif    
