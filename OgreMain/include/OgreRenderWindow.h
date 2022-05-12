/*-------------------------------------------------------------------------
This source file is a part of OGRE
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
THE SOFTWARE
-------------------------------------------------------------------------*/
#ifndef __RenderWindow_H__
#define __RenderWindow_H__

#include "OgrePrerequisites.h"

#include "OgreRenderTarget.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup RenderSystem
    *  @{
    */
    /** Manages the target rendering window.
        @remarks
            This class handles a window into which the contents
            of a scene are rendered. There is a many-to-1 relationship
            between instances of this class an instance of RenderSystem
            which controls the rendering of the scene. There may be
            more than one window in the case of level editor tools etc.
            This class is abstract since there may be
            different implementations for different windowing systems.
        @remarks
            Instances are created and communicated with by the render system
            although client programs can get a reference to it from
            the render system if required for resizing or moving.
            Note that you can have multiple viewpoints
            in the window for effects like rear-view mirrors and
            picture-in-picture views (see Viewport and Camera).
        @author
            Steven Streeting
        @version
            1.0
    */
    class _OgreExport RenderWindow : public RenderTarget
    {

    public:
        /** Default constructor.
        */
        RenderWindow();

        /** Many windowing systems that support HiDPI displays use special points to specify
            size of the windows and controls, so that windows and controls with hardcoded
            sizes does not become too small on HiDPI displays. Such points have constant density
            ~ 100 points per inch (probably 96 on Windows and 72 on Mac), that is independent
            of pixel density of real display, and are used through the all windowing system.

            Sometimes, such view points are chosen bigger for output devices that are viewed
            from larger distances, like 30" TV comparing to 30" monitor, therefore maintaining
            constant points angular density rather than constant linear density.

            In any case, all such windowing system provides the way to convert such view points
            to pixels, be it DisplayProperties::LogicalDpi on WinRT or backingScaleFactor on MacOSX.
            We use pixels consistently through the Ogre, but window/view management functions
            takes view points for convenience, as does the rest of windowing system. Such parameters
            are named using xxxxPt pattern, and should not be mixed with pixels without being
            converted using getViewPointToPixelScale() function.

            Sometimes such scale factor can change on-the-fly, for example if window is dragged
            to monitor with different DPI. In such situation, window size in view points is usually
            preserved by windowing system, and Ogre should adjust pixel size of RenderWindow.
        */
        virtual float getViewPointToPixelScale() { return 1.0f; }

        /** Creates & displays the new window.
            @param name the internal window name. Not necessarily the title.
            @param
                widthPt The width of the window in view points.
            @param
                heightPt The height of the window in view points.
            @param
                fullScreen If true, the window fills the screen,
                with no title bar or border.
            @param
                miscParams A variable number of pointers to platform-specific arguments. The
                actual requirements must be defined by the implementing subclasses.
        */
        virtual void create(const String& name, unsigned int widthPt, unsigned int heightPt,
                bool fullScreen, const NameValuePairList *miscParams) = 0;

        /** Alter fullscreen mode options. 
        Nothing will happen unless the settings here are different from the
            current settings.
        @note Only implemented by few RenderSystems. Prefer native windowing API.
        @param fullScreen Whether to use fullscreen mode or not. 
        @param widthPt The new width to use
        @param heightPt The new height to use
        */
        virtual void setFullscreen(bool fullScreen, unsigned int widthPt, unsigned int heightPt)
                { (void)fullScreen; (void)widthPt; (void)heightPt; }
        
        /** Destroys the window.
        */
        virtual void destroy(void) = 0;

        /** Alter the size of the window.
        */
        virtual void resize(unsigned int widthPt, unsigned int heightPt) = 0;

        /** Query the current size and position from an external window handle.
            @note most of the time you already know the size and should call @ref resize instead.
        */
        virtual void windowMovedOrResized() {}

        /** Reposition the window.

        @note Only implemented by few RenderSystems. Prefer native windowing API.
        */
        virtual void reposition(int leftPt, int topPt) {}

        /** Indicates whether the window is visible (not minimized or obscured)
        */
        virtual bool isVisible(void) const { return true; }

        /** Set the visibility state
        */
        virtual void setVisible(bool visible)
        { (void)visible; }

        /** Indicates whether the window was set to hidden (not displayed)
        */
        virtual bool isHidden(void) const { return false; }

        /** Hide (or show) the window. If called with hidden=true, this
            will make the window completely invisible to the user.
        @remarks
            Setting a window to hidden is useful to create a dummy primary
            RenderWindow hidden from the user so that you can create and
            recreate your actual RenderWindows without having to recreate
            all your resources.
        */
        virtual void setHidden(bool hidden)
        { (void)hidden; }

        /** Enable or disable vertical sync for the RenderWindow.
        */
        virtual void setVSyncEnabled(bool vsync)
        { (void)vsync; }

        /** Indicates whether vertical sync is activated for the window.
        */
        virtual bool isVSyncEnabled() const { return false; }

        /** Set the vertical sync interval. This indicates the number of vertical retraces to wait for
            before swapping buffers. A value of 1 is the default.
        */
        virtual void setVSyncInterval(unsigned int interval)
        { (void)interval; }

        /** Returns the vertical sync interval. 
        */
        unsigned int getVSyncInterval() const { return mVSyncInterval; }
        

        /** Overridden from RenderTarget, flags invisible windows as inactive
        */
        virtual bool isActive(void) const { return mActive && isVisible(); }

        /** Indicates whether the window has been closed by the user.
        */
        virtual bool isClosed(void) const { return mClosed; }
        
        /** Indicates whether the window is the primary window. The
            primary window is special in that it is destroyed when 
            ogre is shut down, and cannot be destroyed directly.
            This is the case because it holds the context for vertex,
            index buffers and textures.
        */
        virtual bool isPrimary(void) const;

        /** Returns true if window is running in fullscreen mode.
        */
        virtual bool isFullScreen(void) const;

        /** Overloaded version of getMetrics from RenderTarget, including extra details
            specific to windowing systems. Result is in pixels.
        */
        void getMetrics(unsigned int& width, unsigned int& height, int& left, int& top) const;

        /// Override since windows don't usually have alpha
        PixelFormat suggestPixelFormat() const { return PF_BYTE_RGB; }

        /** Returns true if the window will automatically de-activate itself when it loses focus.
        */
        bool isDeactivatedOnFocusChange() const;

        /** Indicates whether the window will automatically deactivate itself when it loses focus.
          * @param deactivate a value of 'true' will cause the window to deactivate itself when it loses focus.  'false' will allow it to continue to render even when window focus is lost.
          * @note 'true' is the default behavior.
          */
        void setDeactivateOnFocusChange(bool deactivate);

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
        virtual void _notifySurfaceDestroyed() = 0;
        virtual void _notifySurfaceCreated(void* nativeWindow, void* config = NULL) = 0;
#endif

    protected:
        bool mIsFullScreen;
        bool mIsPrimary;
        bool mAutoDeactivatedOnFocusChange;
        bool mClosed;
        int mLeft;
        int mTop;
        unsigned int mVSyncInterval;
        
        /** Indicates that this is the primary window. Only to be called by
            Ogre::Root
        */
        void _setPrimary() { mIsPrimary = true; }
        
        friend class Root;
    };
    /** @} */
    /** @} */

} // Namespace
#endif
