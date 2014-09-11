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
#ifndef __InputContext_H__
#define __InputContext_H__

#include "Ogre.h"
#include "OIS.h"

#if OIS_VERSION >= 0x010300		//  OIS_VERSION >= 1.3.0
#define OIS_WITH_MULTITOUCH		1
#else							//  OIS_VERSION == 1.2.0
#define OIS_WITH_MULTITOUCH		0
#endif

namespace OgreBites
{
	/*=============================================================================
	| Utility structure for passing OIS devices. Does not own them.
	=============================================================================*/
	struct InputContext
	{
		InputContext()
		{
			mKeyboard = 0;
			mMouse = 0;
#if OIS_WITH_MULTITOUCH
			mMultiTouch = 0;
#endif
			mAccelerometer = 0;
		}

		void capture() const
		{
			if(mKeyboard)
				mKeyboard->capture();
			if(mMouse)
				mMouse->capture();
#if OIS_WITH_MULTITOUCH
			if(mMultiTouch)
				mMultiTouch->capture();
#endif
			if(mAccelerometer)
	            mAccelerometer->capture();
		}

		bool isKeyDown(OIS::KeyCode key) const
		{
			return mKeyboard && mKeyboard->isKeyDown(key);
		}

		bool getCursorPosition(Ogre::Real& x, Ogre::Real& y) const
		{
			// prefer mouse
			if(mMouse)
			{
				x = (Ogre::Real)mMouse->getMouseState().X.abs;
				y = (Ogre::Real)mMouse->getMouseState().Y.abs;
				return true;
			}
			
#if OIS_WITH_MULTITOUCH
			// than touch device
			if(mMultiTouch)
			{
	            std::vector<OIS::MultiTouchState> states = mMultiTouch->getMultiTouchStates();
		        if(states.size() > 0)
		        {
		        	x = (Ogre::Real)states[0].X.abs;
		        	y = (Ogre::Real)states[0].Y.abs;
		        	return true;
			    }
			}
#endif

			// fallback
			x = y = 0.0;
			return false;
		}

		OIS::Keyboard* mKeyboard;         // context keyboard device
		OIS::Mouse* mMouse;               // context mouse device
#if OIS_WITH_MULTITOUCH
		OIS::MultiTouch* mMultiTouch;     // context multitouch device
#endif
		OIS::JoyStick* mAccelerometer;    // context accelerometer device
	};
}

#endif
