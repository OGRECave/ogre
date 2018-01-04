%module(directors="1") OgreBites
%{
/* Includes the header in the wrapper code */
#include "Ogre.h"
#include "OgreBuildSettings.h"
#include "OgreApplicationContext.h"
#include "OgreSGTechniqueResolverListener.h"
#include "OgreCameraMan.h"
#include "OgreTrays.h"
#include "OgreAdvancedRenderControls.h"
#include "OgreUnifiedHighLevelGpuProgram.h"

#if OGRE_BITES_HAVE_SDL
#include "SDL_stdinc.h"
#include "SDL_events.h"
#include "SDL_keyboard.h"
#include "SDL_keycode.h"
#endif
%}

%include std_string.i
%include exception.i 
%include stdint.i
%import "Ogre.i"

#define _OgreBitesExport

#if OGRE_BITES_HAVE_SDL
#define DECLSPEC
#define SDLCALL
#define SDL_FORCE_INLINE
%ignore SDL_vsscanf;
%ignore SDL_vsnprintf;
%include "SDL_stdinc.h"
%include "SDL_keyboard.h"

// tell SWIG that we need ints here
%warnfilter(302) SDLK_ESCAPE;
%warnfilter(302) SDLK_RETURN;
%warnfilter(302) SDLK_SPACE;
%constant int SDLK_ESCAPE;
%constant int SDLK_RETURN;
%constant int SDLK_SPACE;
%include "SDL_keycode.h"

%include "SDL_events.h"
#endif

%include "OgreSGTechniqueResolverListener.h"
%feature("director") OgreBites::ApplicationContext;
%feature("director") OgreBites::InputListener;
%include "OgreInput.h"

#ifdef __ANDROID__
%{
#include <android/native_window_jni.h>
#include <android/asset_manager_jni.h>

JNIEnv* OgreJNIGetEnv();
%}

%ignore OgreBites::ApplicationContext::initApp;
%ignore OgreBites::ApplicationContext::initAppForAndroid(AAssetManager*, ANativeWindow*);
%extend OgreBites::ApplicationContext {
    void initAppForAndroid(jobject assetManager, jobject surface) {
        OgreAssert(assetManager, "assetManager is NULL");
        OgreAssert(surface, "surface is NULL");

        AAssetManager* assetMgr = AAssetManager_fromJava(OgreJNIGetEnv(), assetManager);
        ANativeWindow* nativeWnd = ANativeWindow_fromSurface(OgreJNIGetEnv(), surface);
        $self->initAppForAndroid(assetMgr, nativeWnd);
    }
}
#endif

%include "OgreApplicationContext.h"
%include "OgreCameraMan.h"
%include "OgreWindowEventUtilities.h"
// deprecated
%ignore OgreBites::TrayManager::getWidget(TrayLocation, unsigned int);
%ignore OgreBites::TrayManager::getNumWidgets(TrayLocation);
%ignore OgreBites::TrayManager::getWidgetIterator;
%ignore OgreBites::SelectMenu::getItemsCount;
%include "OgreTrays.h"
%include "OgreAdvancedRenderControls.h"