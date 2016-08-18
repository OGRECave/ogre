/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

#ifndef INCL_OGRE_GTKWINDOW_H
#define INCL_OGRE_GTKWINDOW_H

#include "OgreRenderWindow.h"

#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <gtkglmm.h>

#include <GL/gl.h>

namespace Ogre {

class OGREWidget : public Gtk::GL::DrawingArea
{
public:
    OGREWidget(bool useDepthBuffer);
};
    
class GTKWindow : public RenderWindow, public SigC::Object
{
public:
    GTKWindow();
    ~GTKWindow();

    /**
    * Pump the main loop to actually generate events
    * @returns false when there are no events left to pump
    */
    bool pump_events();

    /**
    * Get the actual widget that is housing OGRE's GL view.
    */
    OGREWidget* get_ogre_widget();

    void create(const String& name, unsigned int width, unsigned int height, unsigned int colourDepth,
                bool fullScreen, int left, int top, bool depthBuffer, 
                void* miscParam, ...);

    void setFullscreen(bool fullScreen, unsigned int width, unsigned int height);
    void destroy(void);
    bool isActive(void) const;
    bool isClosed(void) const;
    void reposition(int left, int top);
    void resize(unsigned int width, unsigned int height);
    void swapBuffers();
    void copyContentsToMemory(const Box& src, const PixelBox &dst, FrameBuffer buffer);

    bool requiresTextureFlipping() const { return false; }

    /**
     * Get a custom, GTK specific attribute. The specific attributes
     * are:
     * GTKMMWINDOW      The Gtk::Window instance (Rendering window)
     * GTKGLMMWIDGET    The Gtk::GL::DrawingArea instance (Ogre widget)
     */
    void getCustomAttribute( const String& name, void* pData );
protected:
    // Signal handlers
    bool on_delete_event(GdkEventAny* event);
    bool on_expose_event(GdkEventExpose* event);
    // bool SimpleGLScene::on_configure_event(GdkEventConfigure* event) 
private:
    //Gtk::Main* kit;
    Gtk::Window *mGtkWindow;
    OGREWidget* ogre;
}; // class GTKWindow

}; // namespace Ogre

#endif // INCL_OGRE_GTKWINDOW_H
