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

#ifndef INCL_OGRE_GTKGLSUPPORT_H
#define INCL_OGRE_GTKGLSUPPORT_H

#include "OgreGLSupport.h"

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
class GTKGLSupport : public GLSupport, public Singleton<GTKGLSupport>
{
public:
    GTKGLSupport();
    void addConfig();
    void setConfigOptions(const String& name, const String& value);
    String validateConfig();
    RenderWindow* createWindow(bool autoCreateWindow, 
                               GLRenderSystem* renderSystem, const String& windowTitle);
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
