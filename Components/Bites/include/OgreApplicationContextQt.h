// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
#ifndef COMPONENTS_BITES_INCLUDE_APPLICATIONCONTEXTQT_H_
#define COMPONENTS_BITES_INCLUDE_APPLICATIONCONTEXTQT_H_

#include "OgreApplicationContextBase.h"

#include <QObject>
#include <QWindow>

#if defined(OgreBitesQt_EXPORTS)
#  define _OgreBitesQtExport Q_DECL_EXPORT
#else
#  define _OgreBitesQtExport Q_DECL_IMPORT
#endif

namespace OgreBites
{
/** \addtogroup Optional
*  @{
*/
/** \addtogroup Bites
*  @{
*/

/**
Specialization for connecting with Qt

- Set a QBasicTimer on this or call startTimer() to update all associated windows using Root::renderOneFrame.
- Use the QWindow::requestUpdate() slot to refresh a single Window using RenderWindow::update
- QWidget::createWindowContainer and QWidget::windowHandle() provide translation between QWidget and QWindow

Assumes that Ogre Main loop is used for compatibility with other implementations.
 */
class _OgreBitesQtExport ApplicationContextQt : public QObject, public ApplicationContextBase
{
    Q_OBJECT
public:
    explicit ApplicationContextQt(const Ogre::String& appName = "Ogre3D")
        : ApplicationContextBase(appName), mWindowOverride(NULL), mQtEventLoop(false)
    {
    }

    NativeWindowPair
    createWindow(const Ogre::String& name, uint32_t w = 0, uint32_t h = 0,
                 Ogre::NameValuePairList miscParams = Ogre::NameValuePairList()) override;

    /// @overload
    NativeWindowPair createWindow(QWindow* window,
                                  Ogre::NameValuePairList miscParams = Ogre::NameValuePairList());

    /**
     * allows overriding the main (first) Window with a pre-created QWindow
     * @param window
     */
    void injectMainWindow(QWindow* window)
    {
        OgreAssert(window, "window is null. Did you set WA_NativeWindow on your QWidget?");
        mWindowOverride = window;
    }

    /**
     * signal that you want to use the Qt event loop
     *
     * aka QApplication::exec() instead of Root::startRendering.
     * In this case you may want to call startTimer on this QObject
     * which ensures that Root::renderOneFrame is called periodically.
     * @param enable
     */
    void useQtEventLoop(bool enable) { mQtEventLoop = enable; }

    using ApplicationContextBase::setWindowGrab;

    void setWindowGrab(NativeWindowType* win, bool grab) override;

    using ApplicationContextBase::addInputListener;
    using ApplicationContextBase::removeInputListener;
    void addInputListener(NativeWindowType* win, InputListener* lis) override;
    void removeInputListener(NativeWindowType* win, InputListener* lis) override;

    void pollEvents() override;
    void shutdown() override;

private:
    void timerEvent(QTimerEvent *e) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    QWindow* mWindowOverride;
    bool mQtEventLoop;
};
/** @} */
/** @} */
} /* namespace Ogre */

#endif /* COMPONENTS_BITES_INCLUDE_OGREAPPLICATIONCONTEXTQT_H_ */
