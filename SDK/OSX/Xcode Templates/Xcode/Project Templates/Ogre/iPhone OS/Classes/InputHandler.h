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

#include "Ogre.h"

// Use this define to signify OIS will be used as a DLL
// (so that dll import/export macros are in effect)
#define OIS_DYNAMIC_LIB

#include "OIS/OISEvents.h"
#include "OIS/OISInputManager.h"
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
#include "OIS/OISMultitouch.h"
#else
#include "OIS/OISMouse.h"
#include "OIS/OISKeyboard.h"
#endif
#include "OIS/OISJoystick.h"

class Simulation;

class InputHandler : 
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
        public OIS::MultiTouchListener
#else
        public OIS::MouseListener, 
        public OIS::KeyListener
#endif
{
private:
	OIS::InputManager *mInputManager;
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
	OIS::MultiTouch *mMouse;
#else
	OIS::Mouse *mMouse;
	OIS::Keyboard *mKeyboard;
#endif
	unsigned long m_hWnd;
	Simulation *m_simulation;	
public:
	InputHandler(Simulation *sim, unsigned long hWnd); 
	~InputHandler();

	void setWindowExtents(int width, int height) ;
	void capture();
	
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
	// MultiTouchListener
    bool touchMoved( const OIS::MultiTouchEvent &arg );
    bool touchPressed( const OIS::MultiTouchEvent &arg );
    bool touchReleased( const OIS::MultiTouchEvent &arg );
    bool touchCancelled( const OIS::MultiTouchEvent &arg );
#else
	// MouseListener
	bool mouseMoved(const OIS::MouseEvent &evt);
	bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID);
	bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID);

	// KeyListener
	bool keyPressed(const OIS::KeyEvent &evt);
	bool keyReleased(const OIS::KeyEvent &evt);
#endif
	// JoyStickListener
	bool buttonPressed(const OIS::JoyStickEvent &evt, int index);
	bool buttonReleased(const OIS::JoyStickEvent &evt, int index);
	bool axisMoved(const OIS::JoyStickEvent &evt, int index);
	bool povMoved(const OIS::JoyStickEvent &evt, int index);
};
