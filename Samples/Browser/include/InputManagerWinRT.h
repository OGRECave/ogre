/*
 -----------------------------------------------------------------------------
 This source file is part of OGRE
 (Object-oriented Graphics Rendering Engine)
 For the latest info, see http://www.ogre3d.org/
 
 Copyright (c) 2000-2012 Torus Knot Software Ltd
 
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

#ifndef __SampleBrowser_NACL_H__
#define __SampleBrowser_NACL_H__

#include "OgrePlatform.h"

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
		switch(currentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Touch:	if(currentPoint->Properties->IsPrimary) return mOISMouse.OnPointerAction(currentPoint, action); break;
		case Windows::Devices::Input::PointerDeviceType::Pen:	if(currentPoint->Properties->IsPrimary) return mOISMouse.OnPointerAction(currentPoint, action); break;
		case Windows::Devices::Input::PointerDeviceType::Mouse:	if(currentPoint->Properties->IsPrimary) return mOISMouse.OnPointerAction(currentPoint, action); break;
		}
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
					if(mState.X.abs != (int)pt.X && mState.Y.abs != (int)pt.Y)
					{
						mState.X.rel = (int)pt.X - mState.X.abs;
						mState.Y.rel = (int)pt.Y - mState.Y.abs;
						mState.X.abs = (int)pt.X;
						mState.Y.abs = (int)pt.Y;
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
};

}  // namespace Ogre


#endif
