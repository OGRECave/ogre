/*
-----------------------------------------------------------------------------
This source file is part of OGRE
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
THE SOFTWARE.
-----------------------------------------------------------------------------
*/


#ifndef __InputManagerWinRT_H__
#define __InputManagerWinRT_H__

#include "OgrePlatform.h"
#include "DirectXMath.h"
using namespace DirectX;

#if OGRE_PLATFORM != OGRE_PLATFORM_WINRT
#error This header is for use with WinRT only
#endif

#if OIS_VERSION >= 0x010300		//  OIS_VERSION >= 1.3.0
#define OIS_130_CONST			const
#else							//  OIS_VERSION == 1.2.0
#define OIS_130_CONST		
#endif

namespace OgreBites {

class InputManagerWinRT
{
public:
	enum EPointerAction
	{
		PointerPressed,
		PointerReleased,
		PointerMoved,
		PointerWheelChanged,
	};

public:
	InputManagerWinRT()
	{
#if (OGRE_WINRT_TARGET_TYPE == PHONE)
		m_tiltX = 0.0f;
		m_tiltY = 0.0f;
#endif
	}
	void Initialize()
	{
#if (OGRE_WINRT_TARGET_TYPE == PHONE)
		// Returns accelerometer ref if there is one; nullptr otherwise.
		m_accelerometer = Windows::Devices::Sensors::Accelerometer::GetDefault();
#endif
	}

	void Update( Ogre::Real timeTotal, Ogre::Real timeDelta )
	{
#if (OGRE_WINRT_TARGET_TYPE == PHONE)
 		const float UserInactivityDuration = 60.0f; // in seconds
		const float UserInactivityThreshold = 0.01f;
		 m_tiltX = 0.0f;
		 m_tiltY = 0.0f;
		// Account for touch input.
		const float touchScalingFactor = 2.0f;
		for (TouchMap::const_iterator iter = m_touches.cbegin(); iter != m_touches.cend(); ++iter)
		{
			m_tiltX += iter->second.x * touchScalingFactor;
			m_tiltY += iter->second.y * touchScalingFactor;
		}

		// Account for sensors.
		const float acceleromterScalingFactor = 3.5f;
		if (m_accelerometer != nullptr)
		{
			Windows::Devices::Sensors::AccelerometerReading^ reading =
				m_accelerometer->GetCurrentReading();

			if (reading != nullptr)
			{
				m_tiltX += static_cast<float>(reading->AccelerationX) * acceleromterScalingFactor;
				m_tiltY += static_cast<float>(reading->AccelerationY) * acceleromterScalingFactor;
			}
		}

		Windows::System::VirtualKey vkey = Windows::System::VirtualKey::None;
		Windows::UI::Core::CorePhysicalKeyStatus keystatus;
		static bool bLeftActive = false;
		static bool bRightActive = false;
		static bool bForwardActive = false;
		static bool bBackActive = false;
		bool bTiltLeft = false;
		bool bTiltRight = false;
		bool bTiltForward = false;
		bool bTiltBack = false;

		if (m_tiltX < -1)
		{
			bTiltLeft = true;
		}
		else if (m_tiltX > 1)
		{
			bTiltRight = true;
		}
		if (m_tiltY < -3)
		{
			bTiltBack = true;
		}
		else if (m_tiltY > -1.5 && m_tiltY < -0.5)
		{
			bTiltForward = true;
		}
		// Disable any key state the we tilted out of.
		if (!bTiltLeft && bLeftActive)
		{
			keystatus.IsKeyReleased = true;
			keystatus.WasKeyDown = false;
			keystatus.ScanCode = OIS::KC_A; // Imitate the 'left key' being released.
			OnKeyAction( vkey, keystatus, false);
			bLeftActive = false;
		}
		// Enable any key state the we tilted in to.
		else if (bTiltLeft && !bLeftActive)
		{
			keystatus.IsKeyReleased = false;
			keystatus.WasKeyDown = true;
			keystatus.ScanCode = OIS::KC_A; // Imitate the 'left key' being pressed.
			OnKeyAction( vkey, keystatus, true);
			bLeftActive = true;
		}

		if (!bTiltRight && bRightActive)
		{
			keystatus.IsKeyReleased = true;
			keystatus.WasKeyDown = false;
			keystatus.ScanCode = OIS::KC_D; // Imitate the 'right key' being released.
			OnKeyAction( vkey, keystatus, false);
			bRightActive = false;
		}
		else if (bTiltRight && !bRightActive)
		{
			keystatus.IsKeyReleased = false;
			keystatus.WasKeyDown = true;
			keystatus.ScanCode = OIS::KC_D; // Imitate the 'right key' being pressed.
			OnKeyAction( vkey, keystatus, true);
			bRightActive = true;
		}
		if (!bTiltBack && bBackActive)
		{
			keystatus.IsKeyReleased = true;
			keystatus.WasKeyDown = false;
			keystatus.ScanCode = OIS::KC_S; // Imitate the 'back key' being released.
			OnKeyAction( vkey, keystatus, false);
			bBackActive = false;
		}
		else if (bTiltBack && !bBackActive)
		{
			keystatus.IsKeyReleased = false;
			keystatus.WasKeyDown = true;
			keystatus.ScanCode = OIS::KC_S; // Imitate the 'back key' being pressed.
			OnKeyAction( vkey, keystatus, true);
			bBackActive = true;
		}
		if (!bTiltForward && bForwardActive)
		{
			keystatus.IsKeyReleased = true;
			keystatus.WasKeyDown = false;
			keystatus.ScanCode = OIS::KC_W; // Imitate the 'forward key' being release.
			OnKeyAction( vkey, keystatus, false);
			bForwardActive = false;
		}
		else if (bTiltForward && !bForwardActive)
		{
			keystatus.IsKeyReleased = false;
			keystatus.WasKeyDown = true;
			keystatus.ScanCode = OIS::KC_W; // Imitate the 'forward key' being pressed.
			OnKeyAction( vkey, keystatus, true);
			bForwardActive = true;
		}
#endif
	}

	bool OnKeyAction(Windows::System::VirtualKey vkey,  Windows::UI::Core::CorePhysicalKeyStatus keystatus, bool pressed)
	{
		return mOISKeyboard.OnKeyAction(vkey, keystatus, pressed);
	}

	bool OnCharacterReceived(unsigned codepoint)
	{
		return mOISKeyboard.OnCharacterReceived(codepoint);
	}

	bool OnPointerAction(Windows::UI::Input::PointerPoint^ currentPoint, EPointerAction action)
	{
#if (OGRE_WINRT_TARGET_TYPE == PHONE)
		//TODO: convert to use onpointerpressed/moved
		if (action == PointerPressed)
		{
			Windows::Foundation::Point pt = currentPoint->Position;
			Windows::UI::Input::PointerPointProperties^ properties = currentPoint->Properties;
			bool handled = false;

			// Convert from display independent pixels to resolution pixels.
			// convert from dips to screen pixels
			const OIS::MouseState& ms = mOISMouse.getMouseState();
			float deviceHeight = ms.height;
			float deviceWidth = ms.width;
			float scaleFactor = deviceWidth / 480.0f;
			float dx = (scaleFactor * pt.X);
			float dy = (scaleFactor * pt.Y);

			// fake some key presses for touch events at the top of the screen
			// Handle touch events in the bottom of the screen as special special cases to emulate Key presses.
			if (dy/deviceHeight < 0.1f)
			{
				Windows::System::VirtualKey vkey = Windows::System::VirtualKey::None;
				Windows::UI::Core::CorePhysicalKeyStatus keystatus;
				keystatus.IsKeyReleased = false;
				keystatus.WasKeyDown = true;
				if (dx/deviceWidth < 0.3f)
				{
					keystatus.ScanCode = OIS::KC_F; // Imitate the 'F' frame rate key being pressed.
					return OnKeyAction( vkey, keystatus, true);
				}
				else if (dx/deviceWidth > 0.7f)
				{
					keystatus.ScanCode = OIS::KC_R; // Imitate the 'R' being pressed.
				}
				else
				{
					keystatus.ScanCode = OIS::KC_F3; // Imitate the 'F3' being pressed.
				}
				return OnKeyAction( vkey, keystatus, true);
			}
		}
		return  mOISMouse.OnPointerAction(currentPoint, action);
#else
		switch(currentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Touch:	if(currentPoint->Properties->IsPrimary) return mOISMouse.OnPointerAction(currentPoint, action); break;
		case Windows::Devices::Input::PointerDeviceType::Pen:	if(currentPoint->Properties->IsPrimary) return mOISMouse.OnPointerAction(currentPoint, action); break;
		case Windows::Devices::Input::PointerDeviceType::Mouse:	if(currentPoint->Properties->IsPrimary) return mOISMouse.OnPointerAction(currentPoint, action); break;
		}
#endif
		return false;
	}

	InputContext GetInputContext()
	{
		InputContext ctx;
		ctx.mMouse = &mOISMouse;
		ctx.mKeyboard = &mOISKeyboard;
		return ctx;
	}

private:
	class WinRTMouse
		: public OIS::Mouse
	{
	public:
		WinRTMouse()
			: OIS::Mouse("", false, 0, nullptr)	{}

		// stubs for abstract methods
		void setBuffered(bool){};
		void capture(){};
		OIS::Interface* queryInterface(OIS::Interface::IType) {return NULL;};
		void _initialize(){};

		bool OnPointerAction(Windows::UI::Input::PointerPoint^ currentPoint, EPointerAction action)
		{
			Windows::Foundation::Point pt = currentPoint->Position;
			Windows::UI::Input::PointerPointProperties^ properties = currentPoint->Properties;
			bool handled = false;

			// clear relative states
			mState.X.rel = mState.Y.rel = mState.Z.rel = 0;

			// Convert from display independent pixels to resolution pixels.
			// convert from dips to screen pixels
#if (OGRE_WINRT_TARGET_TYPE == PHONE)
			float scaleFactor = mState.width / 480.0f;
			int dx = (int)(scaleFactor * pt.X);
			int dy = (int)(scaleFactor * pt.Y);
#else
			int dx = (int)(pt.X);
			int dy = (int)(pt.Y);
#endif
			bool bMoved = mState.X.abs != dx && mState.Y.abs != dy;
			mState.X.rel = dx - mState.X.abs;
			mState.Y.rel = dy - mState.Y.abs;
			mState.X.abs = dx;
			mState.Y.abs = dy;

			// process wheel actions
			switch(action)
			{
			case PointerWheelChanged:
				if(!properties->IsHorizontalMouseWheel)
				{
					mState.Z.rel = properties->MouseWheelDelta;
					if(mListener && mListener->mouseMoved(OIS::MouseEvent(this, mState)))
						handled = true;
				}
				break;

			case PointerPressed:
			case PointerReleased:
			case PointerMoved:
				{
					int buttons = 
						(properties->IsLeftButtonPressed ? 1 << OIS::MB_Left : 0) |
						(properties->IsRightButtonPressed ? 1 << OIS::MB_Right : 0) |
						(properties->IsMiddleButtonPressed ? 1 << OIS::MB_Middle : 0);

					// set each new bit that was not set before
					for(int buttonID = 0, bitmask = 1; (mState.buttons | buttons) != mState.buttons && bitmask != 0; ++buttonID, bitmask <<= 1)
					{
						if(bitmask & buttons & ~mState.buttons)
						{
							mState.buttons |= bitmask;
							if(mListener && mListener->mousePressed(OIS::MouseEvent(this, mState), (OIS::MouseButtonID)buttonID))
								handled = true;
						}
					}

					// reset each old bit that is not set now
					for(int buttonID = 0, bitmask = 1; (mState.buttons | buttons) != buttons && bitmask != 0; ++buttonID, bitmask <<= 1)
					{
						if(bitmask & ~buttons & mState.buttons)
						{
							mState.buttons &= ~bitmask;
							if(mListener && mListener->mouseReleased(OIS::MouseEvent(this, mState), (OIS::MouseButtonID)buttonID))
								handled = true;
						}
					}

					// process pointer moved part
					if(bMoved)
					{
						if(mListener && mListener->mouseMoved(OIS::MouseEvent(this, mState)))
						handled = true;
					}
				}
				break;
			}

			return handled;
		}
	};

	class WinRTKeyboard
		: public OIS::Keyboard
	{
	public:
		WinRTKeyboard()
			: OIS::Keyboard("", false, 0, nullptr) { memset(KeyBuffer, 0, 256); }

		// stubs for abstract methods
		void setBuffered(bool){};
		void capture(){};
		OIS::Interface* queryInterface(OIS::Interface::IType) {return NULL;};
		void _initialize(){};

		bool isKeyDown(OIS::KeyCode key) OIS_130_CONST		{ return KeyBuffer[key] != 0; };
		const std::string& getAsString(OIS::KeyCode)		{ static std::string empty; return empty; };
		void copyKeyStates(char keys[256]) OIS_130_CONST	{ memcpy(keys, KeyBuffer, 256); };

		bool OnKeyAction(Windows::System::VirtualKey vkey,  Windows::UI::Core::CorePhysicalKeyStatus keystatus, bool pressed)
		{
			OIS::KeyCode kc = (OIS::KeyCode)keystatus.ScanCode; // OIS::KeyCode is really DirectInput and WM_KEYDOWN/UP scan code

			//Store result in our keyBuffer too
			KeyBuffer[kc] = pressed;
			
			if(pressed)
			{
				//Turn on modifier
				if( kc == OIS::KC_LCONTROL || kc == OIS::KC_RCONTROL )
					mModifiers |= Ctrl;
				else if( kc == OIS::KC_LSHIFT || kc == OIS::KC_RSHIFT )
					mModifiers |= Shift;
				else if( kc == OIS::KC_LMENU || kc == OIS::KC_RMENU )
					mModifiers |= Alt;

				return mListener && mListener->keyPressed(OIS::KeyEvent(this, kc, 0)); // note - we don`t pass character as it is not known yet
			}
			else
			{
				//Turn off modifier
				if( kc == OIS::KC_LCONTROL || kc == OIS::KC_RCONTROL )
					mModifiers &= ~Ctrl;
				else if( kc == OIS::KC_LSHIFT || kc == OIS::KC_RSHIFT )
					mModifiers &= ~Shift;
				else if( kc == OIS::KC_LMENU || kc == OIS::KC_RMENU )
					mModifiers &= ~Alt;

				//Fire off event
				return mListener && mListener->keyReleased(OIS::KeyEvent(this, kc, 0));
			}
		}

		bool OnCharacterReceived(unsigned codepoint)
		{
			return mListener && mListener->keyPressed(OIS::KeyEvent(this, OIS::KC_UNASSIGNED, codepoint)); // here we pass already known char, but not pass keycode
		}

	private:
		unsigned char KeyBuffer[256];
	};

private:
	WinRTMouse mOISMouse;
	WinRTKeyboard mOISKeyboard;
	
#if (OGRE_WINRT_TARGET_TYPE == PHONE)
    typedef std::map<int, XMFLOAT2>							TouchMap;
    TouchMap												m_touches;
	Windows::Devices::Sensors::Accelerometer^				m_accelerometer;
	float													m_tiltX;
	float													m_tiltY;
#endif
};

}  // namespace Ogre


#endif
