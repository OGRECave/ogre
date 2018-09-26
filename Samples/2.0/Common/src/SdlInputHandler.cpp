
//Thanks to Jordan Milne and Scrawl for allowing to use their
//sdlinputwrapper files as base under the MIT license

#include "SdlInputHandler.h"
#include "InputListeners.h"

#if OGRE_USE_SDL2

#include <SDL_syswm.h>

namespace Demo
{
    SdlInputHandler::SdlInputHandler( SDL_Window *sdlWindow,
                                      MouseListener *mouseListener,
                                      KeyboardListener *keyboardListener,
                                      JoystickListener *joystickListener ) :
        mSdlWindow( sdlWindow ),
        mLogicSystem( 0 ),
        mMouseListener( mouseListener ),
        mKeyboardListener( keyboardListener ),
        mJoystickListener( joystickListener ),
        mWantRelative( false ),
        mWantMouseGrab( false ),
        mWantMouseVisible( true ),
        mIsMouseRelative( !mWantRelative ),
        mWrapPointerManually( false ),
        mGrabPointer( false ),
        mMouseInWindow( true ),
        mWindowHasFocus( true ),
        mWarpX( 0 ),
        mWarpY( 0 ),
        mWarpCompensate( false )
    {
    }
    //-----------------------------------------------------------------------------------
    SdlInputHandler::~SdlInputHandler()
    {
    }
    //-----------------------------------------------------------------------------------
    void SdlInputHandler::handleWindowEvent( const SDL_Event& evt )
    {
        switch( evt.window.event )
        {
            case SDL_WINDOWEVENT_ENTER:
                mMouseInWindow = true;
                updateMouseSettings();
                break;
            case SDL_WINDOWEVENT_LEAVE:
                mMouseInWindow = false;
                updateMouseSettings();
                break;
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                mWindowHasFocus = true;
                updateMouseSettings();
                break;
            case SDL_WINDOWEVENT_FOCUS_LOST:
                mWindowHasFocus = false;
                updateMouseSettings();
                break;
        }
    }
    //-----------------------------------------------------------------------------------
    void SdlInputHandler::_handleSdlEvents( const SDL_Event& evt )
    {
        switch( evt.type )
        {
            case SDL_MOUSEMOTION:
                // Ignore this if it happened due to a warp
                if( !handleWarpMotion(evt.motion) )
                {
                    // If in relative mode, don't trigger events unless window has focus
                    if( (!mWantRelative || mWindowHasFocus) && mMouseListener )
                        mMouseListener->mouseMoved( evt );

                    // Try to keep the mouse inside the window
                    if (mWindowHasFocus)
                        wrapMousePointer( evt.motion );

                    if( mLogicSystem )
                        mGraphicsSystem->queueSendMessage( mLogicSystem, Mq::SDL_EVENT, evt );
                }
                break;
            case SDL_MOUSEWHEEL:
                {
                    if( mMouseListener )
                        mMouseListener->mouseMoved( evt );

                    if( mLogicSystem )
                        mGraphicsSystem->queueSendMessage( mLogicSystem, Mq::SDL_EVENT, evt );
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                {
                    if( mMouseListener )
                        mMouseListener->mousePressed( evt.button, evt.button.button );

                    if( mLogicSystem )
                        mGraphicsSystem->queueSendMessage( mLogicSystem, Mq::SDL_EVENT, evt );
                }
                break;
            case SDL_MOUSEBUTTONUP:
                {
                    if( mMouseListener )
                        mMouseListener->mouseReleased( evt.button, evt.button.button );

                    if( mLogicSystem )
                        mGraphicsSystem->queueSendMessage( mLogicSystem, Mq::SDL_EVENT, evt );
                }
                break;
            case SDL_KEYDOWN:
                {
                    if( !evt.key.repeat && mKeyboardListener )
                        mKeyboardListener->keyPressed( evt.key );

                    if( mLogicSystem )
                        mGraphicsSystem->queueSendMessage( mLogicSystem, Mq::SDL_EVENT, evt );
                }
                break;
            case SDL_KEYUP:
                {
                    if( !evt.key.repeat && mKeyboardListener )
                        mKeyboardListener->keyReleased( evt.key );

                    if( mLogicSystem )
                        mGraphicsSystem->queueSendMessage( mLogicSystem, Mq::SDL_EVENT, evt );
                }
                break;
            case SDL_TEXTINPUT:
                {
                    if( mKeyboardListener )
                        mKeyboardListener->textInput( evt.text );

                    if( mLogicSystem )
                        mGraphicsSystem->queueSendMessage( mLogicSystem, Mq::SDL_EVENT, evt );
                }
                break;
            case SDL_JOYAXISMOTION:
                {
                    if( mJoystickListener )
                        mJoystickListener->joyAxisMoved( evt.jaxis, evt.jaxis.axis );

                    if( mLogicSystem )
                        mGraphicsSystem->queueSendMessage( mLogicSystem, Mq::SDL_EVENT, evt );
                }
                break;
            case SDL_JOYBUTTONDOWN:
                {
                    if( mJoystickListener )
                        mJoystickListener->joyButtonPressed( evt.jbutton, evt.jbutton.button );

                    if( mLogicSystem )
                        mGraphicsSystem->queueSendMessage( mLogicSystem, Mq::SDL_EVENT, evt );
                }
                break;
            case SDL_JOYBUTTONUP:
                {
                    if( mJoystickListener )
                        mJoystickListener->joyButtonReleased( evt.jbutton, evt.jbutton.button );

                    if( mLogicSystem )
                        mGraphicsSystem->queueSendMessage( mLogicSystem, Mq::SDL_EVENT, evt );
                }
                break;
            case SDL_JOYDEVICEADDED:
                //SDL_JoystickOpen(evt.jdevice.which);
                //std::cout << "Detected a new joystick: " << SDL_JoystickNameForIndex(evt.jdevice.which) << std::endl;
                break;
            case SDL_JOYDEVICEREMOVED:
                //std::cout << "A joystick has been removed" << std::endl;
                break;
            case SDL_WINDOWEVENT:
                handleWindowEvent(evt);
                break;
            /*default:
                std::cerr << "Unhandled SDL event of type " << evt.type << std::endl;
                break;*/
        }
    }
    //-----------------------------------------------------------------------------------
    void SdlInputHandler::setGrabMousePointer( bool grab )
    {
        mWantMouseGrab = grab;
        updateMouseSettings();
    }
    //-----------------------------------------------------------------------------------
    void SdlInputHandler::setMouseRelative( bool relative )
    {
        mWantRelative = relative;
        updateMouseSettings();
    }
    //-----------------------------------------------------------------------------------
    void SdlInputHandler::setMouseVisible( bool visible )
    {
        mWantMouseVisible = visible;
        updateMouseSettings();
    }
    //-----------------------------------------------------------------------------------
    void SdlInputHandler::updateMouseSettings(void)
    {
        mGrabPointer = mWantMouseGrab && mMouseInWindow && mWindowHasFocus;
        SDL_SetWindowGrab( mSdlWindow, mGrabPointer ? SDL_TRUE : SDL_FALSE );

        SDL_ShowCursor( mWantMouseVisible || !mWindowHasFocus );

        bool relative = mWantRelative && mMouseInWindow && mWindowHasFocus;
        if( mIsMouseRelative == relative )
            return;

        mIsMouseRelative = relative;

        mWrapPointerManually = false;

        //Input driver doesn't support relative positioning. Do it manually.
        int success = SDL_SetRelativeMouseMode( relative ? SDL_TRUE : SDL_FALSE );
        if( !relative || (relative && success != 0) )
            mWrapPointerManually = true;

        //Remove all pending mouse events that were queued with the old settings.
        SDL_PumpEvents();
        SDL_FlushEvent( SDL_MOUSEMOTION );
    }
    //-----------------------------------------------------------------------------------
    void SdlInputHandler::warpMouse( int x, int y )
    {
        SDL_WarpMouseInWindow( mSdlWindow, x, y );
        mWarpCompensate = true;
        mWarpX = x;
        mWarpY = y;
    }
    //-----------------------------------------------------------------------------------
    void SdlInputHandler::wrapMousePointer( const SDL_MouseMotionEvent& evt )
    {
        //Don't wrap if we don't want relative movements, support
        //relative movements natively, or aren't grabbing anyways
        if( mIsMouseRelative || !mWrapPointerManually || !mGrabPointer )
            return;

        int width = 0;
        int height = 0;

        SDL_GetWindowSize( mSdlWindow, &width, &height );

        const int centerScreenX = width >> 1;
        const int centerScreenY = height >> 1;

        const int FUDGE_FACTOR_X = (width >> 2) - 1;
        const int FUDGE_FACTOR_Y = (height >> 2) - 1;

        //Warp the mouse if it's about to go outside the window
        if( evt.x <= centerScreenX - FUDGE_FACTOR_X || evt.x >= centerScreenX + FUDGE_FACTOR_X ||
            evt.y <= centerScreenY - FUDGE_FACTOR_Y || evt.y >= centerScreenY + FUDGE_FACTOR_Y )
        {
            warpMouse( centerScreenX, centerScreenY );
        }
    }
    //-----------------------------------------------------------------------------------
    bool SdlInputHandler::handleWarpMotion( const SDL_MouseMotionEvent& evt )
    {
        if( !mWarpCompensate )
            return false;

        //This was a warp event, signal the caller to eat it.
        //The if statement is broken in Windows 10, an issue with SetCursorPos. See:
        //https://github.com/gottebp/windows10-setcursorpos-bug-demonstration
        //https://discourse.libsdl.org/t/win10-fall-creators-update-breaks-mouse-warping/23526/2
        //if( evt.x == mWarpX && evt.y == mWarpY )
        {
            mWarpCompensate = false;
            return true;
        }

        return false;
    }
}
#endif
