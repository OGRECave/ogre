// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.

#include "OgreApplicationContext.h"

#include "OgreRoot.h"
#include "OgreRenderWindow.h"

#include "OgreArchiveManager.h"
#include "OgreFileSystem.h"
#include "OgreZip.h"

namespace OgreBites {

ApplicationContextAndroid::ApplicationContextAndroid(const Ogre::String& appName) : ApplicationContextBase(appName)
{
    mAAssetMgr = NULL;
    mAConfig = NULL;
}

NativeWindowPair ApplicationContextAndroid::createWindow(const Ogre::String& name, Ogre::uint32 w, Ogre::uint32 h, Ogre::NameValuePairList miscParams)
{
    miscParams["externalWindowHandle"] = Ogre::StringConverter::toString(reinterpret_cast<size_t>(mWindows[0].native));
    miscParams["androidConfig"] = Ogre::StringConverter::toString(reinterpret_cast<size_t>(mAConfig));
    miscParams["preserveContext"] = "true"; //Optionally preserve the gl context, prevents reloading all resources, this is false by default

    auto p = mRoot->getRenderSystem()->getRenderWindowDescription();
    miscParams.insert(p.miscParams.begin(), p.miscParams.end());
    p.miscParams = miscParams;
    p.name = name;

    mWindows[0].render = mRoot->createRenderWindow(p);
    return mWindows[0];
}

void ApplicationContextAndroid::initAppForAndroid(AAssetManager* assetMgr, ANativeWindow* window)
{
    mAConfig = AConfiguration_new();
    AConfiguration_fromAssetManager(mAConfig, assetMgr);
    mAAssetMgr = assetMgr;

    mWindows.resize(1);
    mWindows[0].native = window;

    initApp();
}

void ApplicationContextAndroid::_fireInputEventAndroid(AInputEvent* event, int wheel) {
    Event evt = {0};

    static TouchFingerEvent lastTouch = {0};

    if(wheel) {
        evt.type = MOUSEWHEEL;
        evt.wheel.y = wheel;
        _fireInputEvent(evt, 0);
        lastTouch.fingerId = -1; // prevent move-jump after pinch is over
        return;
    }

    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMOTION_EVENT_ACTION_MASK & AMotionEvent_getAction(event);

        switch (action) {
        case AMOTION_EVENT_ACTION_DOWN:
            evt.type = FINGERDOWN;
            break;
        case AMOTION_EVENT_ACTION_UP:
            evt.type = FINGERUP;
            break;
        case AMOTION_EVENT_ACTION_MOVE:
            evt.type = FINGERMOTION;
            break;
        default:
            return;
        }

        Ogre::RenderWindow* win = getRenderWindow();

        evt.tfinger.fingerId = AMotionEvent_getPointerId(event, 0);
        evt.tfinger.x = AMotionEvent_getRawX(event, 0) / win->getWidth() * win->getViewPointToPixelScale();
        evt.tfinger.y = AMotionEvent_getRawY(event, 0) / win->getHeight() * win->getViewPointToPixelScale();

        if(evt.type == FINGERMOTION) {
            if(evt.tfinger.fingerId != lastTouch.fingerId)
                return; // wrong finger

            evt.tfinger.dx = evt.tfinger.x - lastTouch.x;
            evt.tfinger.dy = evt.tfinger.y - lastTouch.y;
        }

        lastTouch = evt.tfinger;
    } else {
        if(AKeyEvent_getKeyCode(event) != AKEYCODE_BACK)
            return;

        evt.type = AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN ? KEYDOWN : KEYUP;
        evt.key.keysym.sym = SDLK_ESCAPE;
    }

    _fireInputEvent(evt, 0);
}

void ApplicationContextAndroid::locateResources()
{
    Ogre::ArchiveManager::getSingleton().addArchiveFactory( new Ogre::APKFileSystemArchiveFactory(mAAssetMgr) );
    Ogre::ArchiveManager::getSingleton().addArchiveFactory( new Ogre::APKZipArchiveFactory(mAAssetMgr) );
    ApplicationContextBase::locateResources();
}

void ApplicationContextAndroid::shutdown()
{
    ApplicationContextBase::shutdown();
    AConfiguration_delete(mAConfig);
}

void ApplicationContextAndroid::pollEvents()
{
    for(auto& w : mWindows)
    {
        w.render->windowMovedOrResized();
        windowResized(w.render);
    }
}

}
