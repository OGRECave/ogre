
//Thanks to Jordan Milne and Scrawl for allowing to use their
//sdlinputwrapper files as base under the MIT license

#ifndef _SdlInputHandler_H_
#define _SdlInputHandler_H_

#include "BaseSystem.h"
#include "OgrePrerequisites.h"

#include <SDL.h>

namespace Demo
{
    class MouseListener;
    class KeyboardListener;
    class JoystickListener;

    class SdlInputHandler
    {
        SDL_Window  *mSdlWindow;

        BaseSystem          *mGraphicsSystem;
        BaseSystem          *mLogicSystem;
        MouseListener       *mMouseListener;
        KeyboardListener    *mKeyboardListener;
        JoystickListener    *mJoystickListener;

        /// @see encapsulateEvent
        int                                     mCurrentEventBuffer;
        size_t                                  mEventBufferOffset;
        std::vector<std::vector<unsigned char>*>mEventBuffers;
        std::vector<int>                        mFreeEventBuffers;

        // User settings
        /// User setting. From the SDL docs: While the mouse is in relative mode, the
        /// cursor is hidden, and the driver will try to report continuous motion in
        /// the current window. Only relative motion events will be delivered, the
        /// mouse position will not change.
        bool        mWantRelative;
        /// Locks the pointer to remain inside the window.
        bool        mWantMouseGrab;
        /// Whether to show the mouse.
        bool        mWantMouseVisible;

        // Describes internal state.
        bool        mIsMouseRelative;
        /// Used when driver doesn't support relative mode.
        bool        mWrapPointerManually;
        bool        mGrabPointer;
        bool        mMouseInWindow;
        bool        mWindowHasFocus;

        Uint16      mWarpX;
        Uint16      mWarpY;
        bool        mWarpCompensate;

        void updateMouseSettings(void);

        void handleWindowEvent( const SDL_Event& evt );

        /// Moves the mouse to the specified point within the viewport
        void warpMouse( int x, int y);

        /// Prevents the mouse cursor from leaving the window.
        void wrapMousePointer( const SDL_MouseMotionEvent& evt );

        /// Internal method for ignoring relative
        /// motions as a side effect of warpMouse()
        bool handleWarpMotion( const SDL_MouseMotionEvent& evt );

        /** This handler will redirect all input events to the LogicSystem (if there is one) so that
            it can process it too. This message needs its own heap memory. We don't call new/malloc
            per event because this could damage scalability. Instead we preallocate pools in
            mEventBuffers and use that memory instead. Once the pool has been used for the entire
            frame or is full, we won't use it again until the LogicSystem tells us it's done with it.
        @par
            This function takes an SDL_Event, copies it into the pool, and returns a pointer to it
            ready to be sent to the LogicSystem.
        @remarks
            LogicSystem must send a message back telling us it is done with that ID, so that we can
            call _releaseEventBufferId.
            WARNING: Failing to do so will cause us to continously allocate new pools until RAM goes
            out.
        */
        SDL_Event* encapsulateEvent( const SDL_Event& evt );

    public:
        SdlInputHandler( SDL_Window *sdlWindow,
                         MouseListener *mouseListener,
                         KeyboardListener *keyboardListener,
                         JoystickListener *joystickListener );
        virtual ~SdlInputHandler();

        void _handleSdlEvents( const SDL_Event& evt );

        /// Locks the pointer to the window
        void setGrabMousePointer( bool grab );

        /// Set the mouse to relative positioning mode (when not supported
        /// by hardware, we emulate the behavior).
        /// From the SDL docs: While the mouse is in relative mode, the
        /// cursor is hidden, and the driver will try to report continuous
        /// motion in the current window. Only relative motion events will
        /// be delivered, the mouse position will not change.
        void setMouseRelative( bool relative );

        /// Shows or hides the mouse cursor.
        void setMouseVisible( bool visible );

        void _releaseEventBufferId( int id );
    };
}

#endif
