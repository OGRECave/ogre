// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include "OgreApplicationContextQt.h"

#include <cctype>

#include "OgreRoot.h"
#include "OgreRenderWindow.h"

#include <QEvent>
#include <QWindow>
#include <QGuiApplication>
#include <QResizeEvent>
#include <QKeyEvent>

namespace OgreBites
{
    static Event convert(const QEvent* in)
    {
        static QPoint lastMousePos;

        Event out;

        out.type = 0;

        switch(in->type())
        {
        case QEvent::KeyPress:
            out.type = KEYDOWN;
            OGRE_FALLTHROUGH;
        case QEvent::KeyRelease:
            // ignore auto repeated key-up events to match SDL
            if (!out.type && !static_cast<const QKeyEvent*>(in)->isAutoRepeat())
                out.type = KEYUP;
            {
                auto* key = static_cast<const QKeyEvent*>(in);
                out.key.repeat = key->isAutoRepeat();
                switch(key->key())
                {
                case Qt::Key_Escape:
                    out.key.keysym.sym = SDLK_ESCAPE;
                    break;
                case Qt::Key_Return:
                    out.key.keysym.sym = SDLK_RETURN;
                    break;
                default:
                    out.key.keysym.sym = std::tolower(key->key());
                    break;
                }
                // out.key.keysym.mod = key->modifiers();
            }
            break;
        case QEvent::MouseButtonRelease:
            out.type = MOUSEBUTTONUP;
            OGRE_FALLTHROUGH;
        case QEvent::MouseButtonPress:
            if(!out.type)
                out.type = MOUSEBUTTONDOWN;

            {
                auto* mouse = static_cast<const QMouseEvent*>(in);
                out.button.x = mouse->x();
                out.button.y = mouse->y();
                out.button.button = mouse->button();

                if(out.button.button == Qt::RightButton)
                    out.button.button = BUTTON_RIGHT;
            }
            break;
        case QEvent::Wheel:
            out.type = MOUSEWHEEL;
            out.wheel.y = static_cast<const QWheelEvent*>(in)->angleDelta().y();
            out.wheel.y = out.wheel.y > 0 ? 1 : -1;
            break;
        case QEvent::MouseMove:
            out.type = MOUSEMOTION;
            {
                auto* mouse = static_cast<const QMouseEvent*>(in);
                out.motion.x = mouse->x();
                out.motion.y = mouse->y();
                out.motion.xrel = mouse->x() - lastMousePos.x();
                out.motion.yrel = mouse->y() - lastMousePos.y();

                lastMousePos = mouse->pos();

                //out.motion.windowID = in.motion.windowID;
            }
            break;
        case QEvent::TouchBegin:
            out.type = FINGERDOWN;
            OGRE_FALLTHROUGH;
        case QEvent::TouchEnd:
            if(!out.type)
                out.type = FINGERUP;
            OGRE_FALLTHROUGH;
        case QEvent::TouchUpdate:
            if(!out.type)
                out.type = FINGERMOTION;
            {
                auto* touch = static_cast<const QTouchEvent*>(in);
                out.tfinger.x = touch->touchPoints()[0].pos().x();
                out.tfinger.y = touch->touchPoints()[0].pos().y();
                //out.tfinger.dx = in.tfinger.dx;
                //out.tfinger.dy = in.tfinger.dy;
                out.tfinger.fingerId = touch->touchPoints()[0].id();
            }
            break;
        default:
            break;
        }
        return out;
    }

    NativeWindowPair ApplicationContextQt::createWindow(QWindow* window, Ogre::NameValuePairList miscParams)
    {
        OgreAssert(QGuiApplication::instance(), "you must create a QGuiApplication first");
        if(mWindows.empty())
        {
            QGuiApplication::instance()->installEventFilter(this);
        }

        auto p = mRoot->getRenderSystem()->getRenderWindowDescription();
        miscParams.insert(p.miscParams.begin(), p.miscParams.end());
        p.miscParams = miscParams;
        p.name = window->title().toStdString();
        p.width = window->width();
        p.height= window->height();

        p.miscParams["externalWindowHandle"] = std::to_string(size_t(window->winId()));

        if (!mWindows.empty())
        {
            // additional windows should reuse the context
            p.miscParams["currentGLContext"] = "true";
        }

        NativeWindowPair ret;
        ret.native = (NativeWindowType*)window;
        ret.render = mRoot->createRenderWindow(p);
        mWindows.push_back(ret);

        return ret;
    }

    void ApplicationContextQt::setWindowGrab(NativeWindowType* win, bool grab)
    {
        grab ? ((QWindow*)win)->setCursor(Qt::BlankCursor) : ((QWindow*)win)->unsetCursor();
    }

    NativeWindowPair ApplicationContextQt::createWindow(const Ogre::String& name, uint32_t w, uint32_t h,
                                                        Ogre::NameValuePairList miscParams)
    {
        if(mWindowOverride)
        {
            return createWindow(mWindowOverride, miscParams);
        }

        auto p = mRoot->getRenderSystem()->getRenderWindowDescription();
        if(w > 0 && h > 0)
        {
            p.width = w;
            p.height= h;
        }

        QWindow* win = new QWindow();
        win->setTitle(QString::fromStdString(name));
        win->resize(p.width, p.height);

        if(p.useFullScreen){
           win->showFullScreen();
        }

        auto ret = createWindow(win, miscParams);
        win->show();
        return ret;
    }

    void ApplicationContextQt::addInputListener(NativeWindowType* win, InputListener* lis)
    {
        mInputListeners.insert(std::make_pair(((QWindow*)win)->winId(), lis));
    }


    void ApplicationContextQt::removeInputListener(NativeWindowType* win, InputListener* lis)
    {
        mInputListeners.erase(std::make_pair(((QWindow*)win)->winId(), lis));
    }

    void ApplicationContextQt::pollEvents()
    {
        // assume events get polled by QApplication otherwise
        if(!mQtEventLoop)
            QCoreApplication::instance()->processEvents();
    }

    void ApplicationContextQt::timerEvent(QTimerEvent *e)
    {
        getRoot()->renderOneFrame();
    }

    bool ApplicationContextQt::eventFilter(QObject *obj, QEvent *event)
    {
        QWindow* w = dynamic_cast<QWindow*>(obj);

        if(mWindows.empty() || !w)
        {
            // not applicable
            return QObject::eventFilter(obj, event);
        }

        switch (event->type())
        {
        case QEvent::Quit:
            mRoot->queueEndRendering();
            break;
        case QEvent::UpdateRequest:
            for(auto & window : mWindows)
            {
                if(w->winId() != ((QWindow*)window.native)->winId())
                    continue;
                window.render->update();
            }
            break;
        case QEvent::Resize:
            for(auto & window : mWindows)
            {
                if(w->winId() != ((QWindow*)window.native)->winId())
                    continue;

                Ogre::RenderWindow* win = window.render;
                win->resize(w->width(), w->height());
                windowResized(win);
            }
            break;
        default:
            _fireInputEvent(convert(event), w->winId());
            break;
        }

        return QObject::eventFilter(obj, event);
    }

    void ApplicationContextQt::shutdown()
    {
        ApplicationContextBase::shutdown();

        for(WindowList::iterator it = mWindows.begin(); it != mWindows.end(); ++it)
        {
            // FIXME: what if native is created externally? rather leak then risking double free
            //if(it->native)
            //    delete (QWidget*)it->native;
        }
        if(!mWindows.empty()) {
            QGuiApplication::instance()->removeEventFilter(this);
        }

        mWindows.clear();
    }

} /* namespace Ogre */
