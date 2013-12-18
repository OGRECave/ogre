/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2009 Torus Knot Software Ltd

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

#ifndef INCL_OGRE_GTKGLSUPPORT_H
#define INCL_OGRE_GTKGLSUPPORT_H

#include "OgreGL3PlusSupport.h"

#include <gtkmm/main.h>
#include <gtkglmm.h>

namespace Ogre {

class OGREWidget;

/**
 * GL support in a GTK window.
 *
 * I made this a Singleton, so that the main context can be queried by
 * GTKWindows.
 */
class GTKGLSupport : public GL3PlusSupport, public Singleton<GTKGLSupport>
{
public:
    GTKGLSupport();
    void addConfig();
    void setConfigOptions(const String& name, const String& value);
    String validateConfig();
    RenderWindow* createWindow(bool autoCreateWindow, 
                               GL3PlusRenderSystem* renderSystem, const String& windowTitle);
    RenderWindow* newWindow(const String& name, unsigned int width, unsigned int height, 
                            unsigned int colourDepth, bool fullScreen, int left, int top,
                            bool depthBuffer, RenderWindow* parentWindowHandle,
                            bool vsync);
    void start();
    void stop();
    void begin_context(RenderTarget *_target = 0);
    void end_context();
    void initialiseExtensions(void);
    bool checkMinGLVersion(const String& v) const;
    bool checkExtension(const String& ext) const;
    void* getProcAddress(const String& procname);

    Glib::RefPtr<const Gdk::GL::Context> getMainContext() const; 

    /** Override standard Singleton retrieval.
    @remarks
    Why do we do this? Well, it's because the Singleton
    implementation is in a .h file, which means it gets compiled
    into anybody who includes it. This is needed for the
    Singleton template to work, but we actually only want it
    compiled into the implementation of the class based on the
    Singleton, not all of them. If we don't change this, we get
    link errors when trying to use the Singleton-based class from
    an outside dll.
    @par
    This method just delegates to the template version anyway,
    but the implementation stays in this single compilation unit,
    preventing link errors.
    */
    static GTKGLSupport& getSingleton(void);
    /** Override standard Singleton retrieval.
    @remarks
    Why do we do this? Well, it's because the Singleton
    implementation is in a .h file, which means it gets compiled
    into anybody who includes it. This is needed for the
    Singleton template to work, but we actually only want it
    compiled into the implementation of the class based on the
    Singleton, not all of them. If we don't change this, we get
    link errors when trying to use the Singleton-based class from
    an outside dll.
    @par
    This method just delegates to the template version anyway,
    but the implementation stays in this single compilation unit,
    preventing link errors.
    */
    static GTKGLSupport* getSingletonPtr(void);
private:
    int _context_ref;
    Gtk::Main _kit;
    //OGREWidget* _ogre_widget;

    Glib::RefPtr<Gdk::GL::Context> _main_context;
    Glib::RefPtr<Gdk::GL::Window> _main_window;
}; // class GTKGLSupport

}; // namespace Ogre

#endif // INCL_OGRE_GTKGLSUPPORT_H
