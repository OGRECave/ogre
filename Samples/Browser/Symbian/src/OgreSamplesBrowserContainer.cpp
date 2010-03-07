/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
/*
 * ==============================================================================
 *  Name        : OgreSamplesBrowserContainer.cpp
 *  Part of     : OpenGLEx / OgreSamplesBrowser
 *
 * ==============================================================================
 */

// INCLUDE FILES
#include "OgreSamplesBrowserContainer.h"

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// COgreSamplesBrowserContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void COgreSamplesBrowserContainer::ConstructL(const ::TRect& aRect, CAknAppUi* aAppUi)
{
	
	iAppUi = aAppUi;
 
	// Create the native window
	CreateWindowL();

	// Do not go to full-screen in touch devices.    
	if (AknLayoutUtils::PenEnabled())
	{
		SetRect(aRect);
	}
	else
	{
		SetExtentToWholeScreen();                // Take the whole screen into use
	}
	ActivateL(); 
 
	// init render system.
	iOgreSamplesBrowser = new OgreBites::SampleBrowser();
	iOgreSamplesBrowser->initAppForSymbian(&Window(), this);
	iOgreSamplesBrowser->initApp(); 

	// Initialize timers and counters
	iFrame             = 0;
	iStartTime         = User::NTickCount();
	iLastFrameTime     = iStartTime;

	// Create an active object for animating the scene
	iPeriodic = CPeriodic::NewL( CActive::EPriorityIdle );
	iPeriodic->Start( 100, 100, TCallBack( COgreSamplesBrowserContainer::DrawCallBack, this ) );
}

// ---------------------------------------------------------
// COgreSamplesBrowserContainer::~COgreSamplesBrowserContainer()
// Destructor. Releases any used resources.
// ---------------------------------------------------------
//
COgreSamplesBrowserContainer::~COgreSamplesBrowserContainer()
    {
    delete iPeriodic;

    /* AppExit call is made to release any allocations made in AppInit.
     This call has to be made here before we destroy the rendering context. */
    if ( iOgreSamplesBrowser )
        {
        iOgreSamplesBrowser->closeApp();
        delete iOgreSamplesBrowser;
        }


    }

// ---------------------------------------------------------
// COgreSamplesBrowserContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void COgreSamplesBrowserContainer::SizeChanged()
    {
    if( iOgreSamplesBrowser )
        {
        // If example is running and OpenGL ES is initialized
        // notify it of the screen size change
        TSize size = this->Size();
        iOgreSamplesBrowser->windowMovedOrResized( );
        }
    }


// ---------------------------------------------------------
// COgreSamplesBrowserContainer::HandleResourceChange(
//     TInt aType)
// Dynamic screen resize changes by calling the
// SetExtentToWholeScreen() method again.
// ---------------------------------------------------------
//
 void COgreSamplesBrowserContainer::HandleResourceChange(TInt aType)
    {
    switch( aType )
        {
	    case KEikDynamicLayoutVariantSwitch:
	        // Do not go full-screen in touch devices.    
            if (AknLayoutUtils::PenEnabled())
            {
                SetRect(iAppUi->ClientRect());
            }
            else
            {
                SetExtentToWholeScreen();                // Take the whole screen into use
            }
	        break;
	    }
    }

// ---------------------------------------------------------
// COgreSamplesBrowserContainer::CountComponentControls() const
// Return number of controls inside this container
// ---------------------------------------------------------
//
TInt COgreSamplesBrowserContainer::CountComponentControls() const
    {
    return 0;
    }

// ---------------------------------------------------------
// COgreSamplesBrowserContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* COgreSamplesBrowserContainer::ComponentControl(TInt /*aIndex*/) const
    {
    return NULL;
    }

// ---------------------------------------------------------
// COgreSamplesBrowserContainer::Draw(const ::TRect& aRect) const
// ---------------------------------------------------------
//
void COgreSamplesBrowserContainer::Draw(const ::TRect& aRect) const
    {
		CWindowGc& gc = SystemGc();
		gc.Clear( aRect );

    }

// ---------------------------------------------------------
// COgreSamplesBrowserContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void COgreSamplesBrowserContainer::HandleControlEventL(
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
    {
    // TODO: Add any control event handler code here
    }

// ----------------------------------------------------------------------
// COgreSamplesBrowserContainer::GetTimeTick
// Returns the current time.
// ----------------------------------------------------------------------
//
TTime COgreSamplesBrowserContainer::GetTimeTick()
    {
    TTime time;
    time.UniversalTime();
    return time;
    }


// ---------------------------------------------------------
// COgreSamplesBrowserContainer::DrawCallBack( TAny* aInstance )
// Called by the CPeriodic in order to draw the graphics
// ---------------------------------------------------------
//
TInt COgreSamplesBrowserContainer::DrawCallBack( TAny* aInstance )
{
	// Cast parameter to container instance
	COgreSamplesBrowserContainer* instance = (COgreSamplesBrowserContainer*) aInstance;

	// Update the frame counts
	instance->iFrame++;

	// Compute the elapsed time in seconds since the startup of the example 
#ifdef __WINS__

	// In the emulator the tickcount runs at 200Hz
	GLfloat timeSecs = ( (GLfloat) ( User::NTickCount() - instance->iStartTime ) ) / 200.f;

#else

	// In the device the tickcount runs at 1000hz (as intended)
	GLfloat timeSecs = ( (GLfloat) ( User::NTickCount() - instance->iStartTime ) ) / 1000.f;

#endif

	GLfloat deltaTimeSecs = timeSecs - instance->iLastFrameTime;

	{
		// call render here...
		Root::getSingleton().renderOneFrame((Real) deltaTimeSecs);

		if ( !(instance->iFrame % 50 ) )
		{
			// Reset inactivity timer to keep the background light on
			User::ResetInactivityTime();

			// Suspend the current thread for a short while. Give some time
			// to other threads and AOs, avoids the ViewSrv error in ARMI and
			// THUMB release builds. One may try to decrease the callback
			// function instead of this.
			User::After( 0 );
		}
	}

	// Set the current time to be the last frame time for the upcoming frame
	instance->iLastFrameTime = timeSecs;

	return 0;


}

//--------------------------------------------------------------------------------------------------//

bool COgreSamplesBrowserContainer::_doMouseClick( int mouseButton, bool isDown )
{

	if( isDown )
	{
		mState.buttons |= 1 << mouseButton; //turn the bit flag on
		if( iOgreSamplesBrowser )
			return iOgreSamplesBrowser->mousePressed( OIS::MouseEvent( NULL, mState ), (OIS::MouseButtonID)mouseButton );
	}
	else
	{
		mState.buttons &= ~(1 << mouseButton); //turn the bit flag off
		if( iOgreSamplesBrowser )
			return iOgreSamplesBrowser->mouseReleased( OIS::MouseEvent( NULL, mState ), (OIS::MouseButtonID)mouseButton );
	}

	return true;
}

//--------------------------------------------------------------------------------------------------//
// From class CCoeControl.
void COgreSamplesBrowserContainer::HandlePointerEventL( const TPointerEvent& aPointerEvent )
{
	// get the position
    TPoint point = aPointerEvent.iPosition;
	mState.X.abs = point.iX;
	mState.Y.abs = point.iY;

	// get the button state
	switch(aPointerEvent.iType)
	{			
		/** Button 1 or pen down. */
		case TPointerEvent::EButton1Down:
			_doMouseClick(0, true);
			break;
		/** Button 1 or pen up. */
		case TPointerEvent::EButton1Up:
			_doMouseClick(0, false);
			break;
		/** Button 2 down.

		This is the middle button of a 3 button mouse. */
		case TPointerEvent::EButton2Down:
			_doMouseClick(1, true);
			break;
		/** Button 2 up.
		This is the middle button of a 3 button mouse. */
		case TPointerEvent::EButton2Up:
			_doMouseClick(1, false);
			break;
		/** Button 3 down. */
		case TPointerEvent::EButton3Down:
			_doMouseClick(2, true);
			break;
		/** Button 3 up. */
		case TPointerEvent::EButton3Up:
			_doMouseClick(2, false);
			break;
		/** Drag event.
		These events are only received when button 1 is down. */
		case TPointerEvent::EDrag:
			break;
		/** Move event.
		These events are only received when button 1 is up and the XY input mode is
		not pen. */
		case TPointerEvent::EMove:
			break;
		/** Button repeat event. */
		case TPointerEvent::EButtonRepeat:
			break;
		/** Switch on event caused by a screen tap. */
		case TPointerEvent::ESwitchOn:
			break;
		default:
			break;			
	}
	
	if( iOgreSamplesBrowser  )
		iOgreSamplesBrowser->mouseMoved( OIS::MouseEvent( NULL, mState ) );

}

//--------------------------------------------------------------------------------------------------//

bool COgreSamplesBrowserContainer::_doKeyPress( int key, unsigned int txt, bool isDown )
{

	if( isDown )
	{
		if( iOgreSamplesBrowser )
			return iOgreSamplesBrowser->keyPressed( OIS::KeyEvent( NULL, (OIS::KeyCode)key, txt ) );
	}
	else
	{
		if( iOgreSamplesBrowser )
			return iOgreSamplesBrowser->keyReleased( OIS::KeyEvent( NULL, (OIS::KeyCode)key, txt ) );
	}

	return true;
}

//--------------------------------------------------------------------------------------------------//
// From class CCoeControl.
IMPORT_C TKeyResponse COgreSamplesBrowserContainer::OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType)
{
	if (aType == EEventKeyUp || aType == EEventKeyDown)
	{

		bool isDown = (aType == EEventKeyDown);
		TUint iCode = aKeyEvent.iCode;
		TUint iScanCode = aKeyEvent.iScanCode;
		TUint iModifiers = aKeyEvent.iModifiers;
		TUint iRepeats = aKeyEvent.iRepeats;

		//Catch the dial button
		switch (aKeyEvent.iScanCode)
		{
		case 'A': _doKeyPress(OIS::KC_A, 'A', isDown); break;
		case 'B': _doKeyPress(OIS::KC_B, 'B', isDown); break;
		case 'C': _doKeyPress(OIS::KC_C, 'C', isDown); break;
		case 'D': _doKeyPress(OIS::KC_D, 'D', isDown); break;
		case 'E': _doKeyPress(OIS::KC_E, 'E', isDown); break;
		case 'F': _doKeyPress(OIS::KC_F, 'F', isDown); break;
		case 'G': _doKeyPress(OIS::KC_G, 'G', isDown); break;
		case 'H': _doKeyPress(OIS::KC_H, 'H', isDown); break;
		case 'I': _doKeyPress(OIS::KC_I, 'I', isDown); break;
		case 'J': _doKeyPress(OIS::KC_J, 'J', isDown); break;
		case 'K': _doKeyPress(OIS::KC_K, 'K', isDown); break;
		case 'L': _doKeyPress(OIS::KC_L, 'L', isDown); break;
		case 'M': _doKeyPress(OIS::KC_M, 'M', isDown); break;
		case 'N': _doKeyPress(OIS::KC_N, 'N', isDown); break;
		case 'O': _doKeyPress(OIS::KC_O, 'O', isDown); break;
		case 'P': _doKeyPress(OIS::KC_P, 'P', isDown); break;
		case 'Q': _doKeyPress(OIS::KC_Q, 'Q', isDown); break;
		case 'R': _doKeyPress(OIS::KC_R, 'R', isDown); break;
		case 'S': _doKeyPress(OIS::KC_S, 'S', isDown); break;
		case 'T': _doKeyPress(OIS::KC_T, 'T', isDown); break;
		case 'U': _doKeyPress(OIS::KC_U, 'U', isDown); break;
		case 'V': _doKeyPress(OIS::KC_V, 'V', isDown); break;
		case 'W': _doKeyPress(OIS::KC_W, 'W', isDown); break;
		case 'X': _doKeyPress(OIS::KC_X, 'X', isDown); break;
		case 'Y': _doKeyPress(OIS::KC_Y, 'Y', isDown); break;
		case 'Z': _doKeyPress(OIS::KC_Z, 'Z', isDown); break;

		case EStdKeyBackspace:		_doKeyPress(OIS::KC_BACK, '\b', isDown); break;
		case EStdKeyTab:			_doKeyPress(OIS::KC_TAB, '\t', isDown); break;
		case EStdKeyEnter:			_doKeyPress(OIS::KC_RETURN, '\r', isDown); break;
		case EStdKeyEscape:			_doKeyPress(OIS::KC_ESCAPE, 27, isDown); break;
		case EStdKeySpace:			_doKeyPress(OIS::KC_SPACE, ' ', isDown); break;
		case EStdKeyPrintScreen:	_doKeyPress(OIS::KC_SYSRQ, 0, isDown); break;			
		case EStdKeyPause:			_doKeyPress(OIS::KC_PAUSE, 0, isDown); break;
		case EStdKeyHome:			_doKeyPress(OIS::KC_HOME, 0, isDown); break;
		case EStdKeyEnd:			_doKeyPress(OIS::KC_END, 0, isDown); break;
		case EStdKeyPageUp:			_doKeyPress(OIS::KC_PGUP, 0, isDown); break;
		case EStdKeyPageDown:		_doKeyPress(OIS::KC_PGDOWN, 0, isDown); break;
		case EStdKeyInsert:			_doKeyPress(OIS::KC_INSERT, 0, isDown); break;
		case EStdKeyDelete:			_doKeyPress(OIS::KC_DELETE, 0, isDown); break;
		case EStdKeyLeftArrow:		_doKeyPress(OIS::KC_LEFT, 0, isDown); break;
		case EStdKeyRightArrow:		_doKeyPress(OIS::KC_RIGHT, 0, isDown); break;
		case EStdKeyUpArrow:		_doKeyPress(OIS::KC_UP, 0, isDown); break;
		case EStdKeyDownArrow:		_doKeyPress(OIS::KC_DOWN, 0, isDown); break;
		case EStdKeyLeftShift:		_doKeyPress(OIS::KC_LSHIFT, 0, isDown); break;
		case EStdKeyRightShift:		_doKeyPress(OIS::KC_RSHIFT, 0, isDown); break;
		case EStdKeyLeftAlt:		_doKeyPress(OIS::KC_LMENU, 0, isDown); break;
		case EStdKeyRightAlt:		_doKeyPress(OIS::KC_RMENU, 0, isDown); break;
		case EStdKeyLeftCtrl:		_doKeyPress(OIS::KC_LCONTROL, 0, isDown); break;
		case EStdKeyRightCtrl:		_doKeyPress(OIS::KC_RCONTROL, 0, isDown); break;			
			// EStdKeyLeftFunc ?
			// EStdKeyRightFunc ?				
		case EStdKeyCapsLock:		_doKeyPress(OIS::KC_CAPITAL, 0, isDown); break;			
		case EStdKeyNumLock:		_doKeyPress(OIS::KC_NUMLOCK, 0, isDown); break;			
		case EStdKeyScrollLock:		_doKeyPress(OIS::KC_SCROLL, 0, isDown); break;			

		case EStdKeyF1 : _doKeyPress(OIS::KC_F1,  0, isDown); break;			
		case EStdKeyF2 : _doKeyPress(OIS::KC_F2,  0, isDown); break;			
		case EStdKeyF3 : _doKeyPress(OIS::KC_F3,  0, isDown); break;			
		case EStdKeyF4 : _doKeyPress(OIS::KC_F4,  0, isDown); break;			
		case EStdKeyF5 : _doKeyPress(OIS::KC_F5,  0, isDown); break;			
		case EStdKeyF6 : _doKeyPress(OIS::KC_F6,  0, isDown); break;			
		case EStdKeyF7 : _doKeyPress(OIS::KC_F7,  0, isDown); break;			
		case EStdKeyF8 : _doKeyPress(OIS::KC_F8,  0, isDown); break;			
		case EStdKeyF9 : _doKeyPress(OIS::KC_F9,  0, isDown); break;			
		case EStdKeyF10: _doKeyPress(OIS::KC_F10, 0, isDown); break;			
		case EStdKeyF11: _doKeyPress(OIS::KC_F11, 0, isDown); break;			
		case EStdKeyF12: _doKeyPress(OIS::KC_F12, 0, isDown); break;			
		case EStdKeyF13: _doKeyPress(OIS::KC_F13, 0, isDown); break;			
		case EStdKeyF14: _doKeyPress(OIS::KC_F14, 0, isDown); break;			
		case EStdKeyF15: _doKeyPress(OIS::KC_F15, 0, isDown); break;			
			//case EStdKeyF16: _doKeyPress(OIS::KC_F16, 0, isDown); break;			
			//case EStdKeyF17: _doKeyPress(OIS::KC_F17, 0, isDown); break;			
			//case EStdKeyF18: _doKeyPress(OIS::KC_F18, 0, isDown); break;			
			//case EStdKeyF19: _doKeyPress(OIS::KC_F19, 0, isDown); break;			
			//case EStdKeyF20: _doKeyPress(OIS::KC_F20, 0, isDown); break;			
			//case EStdKeyF21: _doKeyPress(OIS::KC_F21, 0, isDown); break;			
			//case EStdKeyF22: _doKeyPress(OIS::KC_F22, 0, isDown); break;			
			//case EStdKeyF23: _doKeyPress(OIS::KC_F23, 0, isDown); break;			
			//case EStdKeyF24: _doKeyPress(OIS::KC_F24, 0, isDown); break;			
			//case EStdKeyF25: _doKeyPress(OIS::KC_F25, 0, isDown); break;			

			//case EStdKeyXXX: _doKeyPress(OIS::, '`', isDown); break;			
		case EStdKeyComma:				_doKeyPress(OIS::KC_COMMA, ',', isDown); break;			
		case EStdKeyFullStop:			_doKeyPress(OIS::KC_PERIOD, '.', isDown); break;			
		case EStdKeyForwardSlash:		_doKeyPress(OIS::KC_SLASH, '/', isDown); break;			
		case EStdKeyBackSlash:			_doKeyPress(OIS::KC_BACKSLASH, '\\', isDown); break;			
		case EStdKeySemiColon:			_doKeyPress(OIS::KC_SEMICOLON, ';', isDown); break;			
		case EStdKeySingleQuote:		_doKeyPress(OIS::KC_APOSTROPHE, '\'', isDown); break;			
			//case EStdKeyHash: _doKeyPress(OIS::, '#', isDown); break;			
		case EStdKeySquareBracketLeft:	_doKeyPress(OIS::KC_LBRACKET, '[', isDown); break;			
		case EStdKeySquareBracketRight: _doKeyPress(OIS::KC_RBRACKET, ']', isDown); break;			
		case EStdKeyMinus:				_doKeyPress(OIS::KC_MINUS, '-', isDown); break;			
		case EStdKeyEquals:				_doKeyPress(OIS::KC_EQUALS, '=', isDown); break;			
		case EStdKeyNkpForwardSlash:	_doKeyPress(OIS::KC_DIVIDE, '/', isDown); break;			
		case EStdKeyNkpAsterisk:		_doKeyPress(OIS::KC_MULTIPLY, '*', isDown); break;			
		case EStdKeyNkpMinus:			_doKeyPress(OIS::KC_SUBTRACT, '-', isDown); break;			
		case EStdKeyNkpPlus:			_doKeyPress(OIS::KC_ADD, '-', isDown); break;			
		case EStdKeyNkpEnter:			_doKeyPress(OIS::KC_NUMPADENTER, '\r', isDown); break;			
		case EStdKeyNkpFullStop:		_doKeyPress(OIS::KC_DECIMAL, '.', isDown); break;			

		case EStdKeyNkp1: _doKeyPress(OIS::KC_NUMPAD1, '1', isDown); break;			
		case EStdKeyNkp2: _doKeyPress(OIS::KC_NUMPAD2, '2', isDown); break;			
		case EStdKeyNkp3: _doKeyPress(OIS::KC_NUMPAD3, '3', isDown); break;			
		case EStdKeyNkp4: _doKeyPress(OIS::KC_NUMPAD4, '4', isDown); break;			
		case EStdKeyNkp5: _doKeyPress(OIS::KC_NUMPAD5, '5', isDown); break;			
		case EStdKeyNkp6: _doKeyPress(OIS::KC_NUMPAD6, '6', isDown); break;			
		case EStdKeyNkp7: _doKeyPress(OIS::KC_NUMPAD7, '7', isDown); break;			
		case EStdKeyNkp8: _doKeyPress(OIS::KC_NUMPAD8, '8', isDown); break;			
		case EStdKeyNkp9: _doKeyPress(OIS::KC_NUMPAD9, '9', isDown); break;			
		case EStdKeyNkp0: _doKeyPress(OIS::KC_NUMPAD0, '0', isDown); break;			

		case EStdKeyIncVolume: _doKeyPress(OIS::KC_VOLUMEUP, 0, isDown); break;			
		case EStdKeyDecVolume: _doKeyPress(OIS::KC_VOLUMEDOWN, 0, isDown); break;			

		case EStdKeyDictaphonePlay: _doKeyPress(OIS::KC_PLAYPAUSE, 0, isDown); break;			
		case EStdKeyDictaphoneStop: _doKeyPress(OIS::KC_MEDIASTOP, 0, isDown); break;

		default: break;
		}//switch
	}//if
	return  CCoeControl::OfferKeyEventL(aKeyEvent, aType);

}
// End of File
