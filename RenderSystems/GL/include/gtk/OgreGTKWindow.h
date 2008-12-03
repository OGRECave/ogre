/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
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
    void swapBuffers(bool waitForVSync);
    void copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer);

    bool requiresTextureFlipping() const { return false; }

	/**
	 * Get a custom, GTK specific attribute. The specific attributes
	 * are:
	 * GTKMMWINDOW		The Gtk::Window instance (Rendering window)
	 * GTKGLMMWIDGET	The Gtk::GL::DrawingArea instance (Ogre widget)
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
