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

#include "OgreGTKWindow.h"
#include "OgreGTKGLSupport.h"
#include "OgreRenderSystem.h"
#include "OgreRoot.h"
#include "OgreLogManager.h"

using namespace Ogre;

OGREWidget::OGREWidget(bool useDepthBuffer) : 
	Gtk::GL::DrawingArea()
{
    Glib::RefPtr<Gdk::GL::Config> glconfig;

    Gdk::GL::ConfigMode mode = Gdk::GL::MODE_RGBA | Gdk::GL::MODE_DOUBLE;
    if (useDepthBuffer)
        mode |= Gdk::GL::MODE_DEPTH;

    glconfig = Gdk::GL::Config::create(mode);
    if (glconfig.is_null())
    {
    	LogManager::getSingleton().logMessage("[gtk] GLCONFIG BLOWUP");
    }

    // Inherit GL context from Ogre main context
    set_gl_capability(glconfig, GTKGLSupport::getSingleton().getMainContext());

    add_events(Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
}

// OGREWidget TODO: 
// - resize events et al
// - Change aspect ratio

GTKWindow::GTKWindow():
	mGtkWindow(0)
{
    	//kit = Gtk::Main::instance();
	
	// Should  this move to GTKGLSupport?
    	// Gtk::GL::init(0, NULL);
	// Already done in GTKGLSupport	

	mWidth = 0;
	mHeight  = 0;
}

GTKWindow::~GTKWindow()
{
}
bool GTKWindow::pump_events()
{
    Gtk::Main *kit = Gtk::Main::instance();
    if (kit->events_pending())
    {
        kit->iteration(false);
        return true;
    }
    return false;
}

OGREWidget* GTKWindow::get_ogre_widget()
{
    return ogre;
}

void GTKWindow::create(const String& name, unsigned int width, unsigned int height, unsigned int colourDepth, 
                       bool fullScreen, int left, int top, bool depthBuffer, 
                       void* miscParam, ...)
{
   	mName = name;
	mWidth = width;
	mHeight = height;

	if(!miscParam) {
    		mGtkWindow = new Gtk::Window();
    		mGtkWindow->set_title(mName);

    		if (fullScreen)
    		{
				mIsFullScreen = true;
        		mGtkWindow->set_decorated(false);
        		mGtkWindow->fullscreen();
    		}
    		else
    		{
				mIsFullScreen = false;
        		mGtkWindow->set_default_size(mWidth, mHeight);
        		mGtkWindow->move(left, top); 
    		}
	} else {
		// If miscParam is not 0, a parent widget has been passed in,
		// we will handle this later on after the widget has been created.
	}

    	ogre = Gtk::manage(new OGREWidget(depthBuffer));
    	ogre->set_size_request(width, height);

	ogre->signal_delete_event().connect(SigC::slot(*this, &GTKWindow::on_delete_event));
	ogre->signal_expose_event().connect(SigC::slot(*this, &GTKWindow::on_expose_event));

	if(mGtkWindow) {
    		mGtkWindow->add(*ogre);
    		mGtkWindow->show_all();
	}
	if(miscParam) {
		// Attach it!
		// Note that the parent widget *must* be visible already at this point,
		// or the widget won't get realized in time for the GLinit that follows
		// this call. This is usually the case for Glade generated windows, anyway.
		reinterpret_cast<Gtk::Container*>(miscParam)->add(*ogre);
		ogre->show();
	}
	//ogre->realize();
}

void GTKWindow::destroy()
{
    	Root::getSingleton().getRenderSystem()->detachRenderTarget( this->getName() );
	// We could detach the widget from its parent and destroy it here too,
	// but then again, it is managed so we rely on GTK to destroy it.
	delete mGtkWindow;
	mGtkWindow = 0;

}

void GTKWindow::setFullscreen(bool fullScreen, unsigned int width, unsigned int height)
{

	if((fullScreen) && (!mIsFullScreen))
	{
		mIsFullScreen = true;
		mGtkWindow->fullscreen();
		ogre->set_size_request(width, height);
	}
	else if((!fullScreen) && (mIsFullScreen))
	{
		mIsFullScreen = false;
		mGtkWindow->unfullscreen();
		ogre->set_size_request(width, height);
	}
}

bool GTKWindow::isActive() const
{
    return ogre->is_realized();
}

bool GTKWindow::isClosed() const
{
    return ogre->is_visible();
}

void GTKWindow::reposition(int left, int top)
{
	if(mGtkWindow)
    		mGtkWindow->move(left, top);
}

void GTKWindow::resize(unsigned int width, unsigned int height)
{
	if(mGtkWindow)
    		mGtkWindow->resize(width, height);
}

void GTKWindow::swapBuffers(bool waitForVSync)
{
    	Glib::RefPtr<Gdk::GL::Window> glwindow = ogre->get_gl_window();
    	glwindow->swap_buffers();
}

void GTKWindow::copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer)
{
	if ((dst.left < 0) || (dst.right > mWidth) ||
		(dst.top < 0) || (dst.bottom > mHeight) ||
		(dst.front != 0) || (dst.back != 1))
	{
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
					"Invalid box.",
					"GTKWindow::copyContentsToMemory" );
	}

	if (buffer == FB_AUTO)
	{
		buffer = mIsFullScreen? FB_FRONT : FB_BACK;
	}

	GLenum format = Ogre::GLPixelUtil::getGLOriginFormat(dst.format);
	GLenum type = Ogre::GLPixelUtil::getGLOriginDataType(dst.format);

	if ((format == GL_NONE) || (type == 0))
	{
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
					"Unsupported format.",
					"GTKWindow::copyContentsToMemory" );
	}

	glReadBuffer((buffer == FB_FRONT)? GL_FRONT : GL_BACK);
	glReadPixels((GLint)dst.left, (GLint)dst.top,
				 (GLsizei)dst.getWidth(), (GLsizei)dst.getHeight(),
				 format, type, dst.data);

	//vertical flip
	{
		size_t rowSpan = dst.getWidth() * PixelUtil::getNumElemBytes(dst.format);
		size_t height = dst.getHeight();
		uchar *tmpData = new uchar[rowSpan * height];
		uchar *srcRow = (uchar *)dst.data, *tmpRow = tmpData + (height - 1) * rowSpan;

		while (tmpRow >= tmpData)
		{
			memcpy(tmpRow, srcRow, rowSpan);
			srcRow += rowSpan;
			tmpRow -= rowSpan;
		}
		memcpy(dst.data, tmpData, rowSpan * height);

		delete [] tmpData;
	}
}

void GTKWindow::getCustomAttribute( const String& name, void* pData )
{
	if( name == "GTKMMWINDOW" )
	{
		Gtk::Window **win = static_cast<Gtk::Window **>(pData);
		// Oh, the burdens of multiple inheritance
		*win = mGtkWindow;
		return;
	}
	else if( name == "GTKGLMMWIDGET" )
	{
	    	Gtk::GL::DrawingArea **widget = static_cast<Gtk::GL::DrawingArea **>(pData);
		*widget = ogre;
		return;
	}
	else if( name == "isTexture" )
	{
		bool *b = reinterpret_cast< bool * >( pData );
		*b = false;
		return;
	}
	RenderWindow::getCustomAttribute(name, pData);
}


bool GTKWindow::on_delete_event(GdkEventAny* event)
{
    Root::getSingleton().getRenderSystem()->detachRenderTarget( this->getName() );
    return false;
}

bool GTKWindow::on_expose_event(GdkEventExpose* event)
{
    // Window exposed, update interior
    //std::cout << "Window exposed, update interior" << std::endl;
    // TODO: time between events, as expose events can be sent crazily fast
    update();
    return false;
}
