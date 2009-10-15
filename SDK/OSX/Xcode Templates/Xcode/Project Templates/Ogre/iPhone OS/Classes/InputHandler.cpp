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
#include "InputHandler.h"
#include "OgreStringConverter.h"
#include "Simulation.h"

InputHandler::InputHandler(Simulation *sim, unsigned long hWnd)  {
	
	OIS::ParamList pl;
	pl.insert(OIS::ParamList::value_type("WINDOW", Ogre::StringConverter::toString(hWnd)));
	
	m_hWnd = hWnd;
	mInputManager = OIS::InputManager::createInputSystem( pl );
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
	mMouse = static_cast<OIS::MultiTouch*>(mInputManager->createInputObject( OIS::OISMultiTouch, true ));
#else
	mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));
	mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true));
	mKeyboard->setEventCallback(this);
#endif
	mMouse->setEventCallback(this);

	m_simulation = sim;
}

InputHandler::~InputHandler() {

    if( mInputManager )
    {
        mInputManager->destroyInputObject( mMouse );
#if OGRE_PLATFORM != OGRE_PLATFORM_IPHONE
        mInputManager->destroyInputObject( mKeyboard );
#endif        
        OIS::InputManager::destroyInputSystem(mInputManager);
        mInputManager = 0;
    }
}

void InputHandler::capture() {
	mMouse->capture();
#if OGRE_PLATFORM != OGRE_PLATFORM_IPHONE
	mKeyboard->capture();
#endif
}

void  InputHandler::setWindowExtents(int width, int height){
	//Set Mouse Region.. if window resizes, we should alter this to reflect as well
#if OGRE_PLATFORM != OGRE_PLATFORM_IPHONE
	const OIS::MouseState &ms = mMouse->getMouseState();
	ms.width = width;
	ms.height = height;
#endif
}


#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
// MultiTouchListener
bool InputHandler::touchMoved(const OIS::MultiTouchEvent &evt) {
	return true;
}

bool InputHandler::touchPressed(const OIS::MultiTouchEvent &evt) {
	return true;
}

bool InputHandler::touchReleased(const OIS::MultiTouchEvent &evt) {
	return true;
}

bool InputHandler::touchCancelled(const OIS::MultiTouchEvent &evt) {
	return true;
}
#else
// MouseListener
bool InputHandler::mouseMoved(const OIS::MouseEvent &evt) {
	return true;
}

bool InputHandler::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID btn) {
	return true;
}

bool InputHandler::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID btn) {
	return true;
}

// KeyListener
bool InputHandler::keyPressed(const OIS::KeyEvent &evt) {
	return true;
}

bool InputHandler::keyReleased(const OIS::KeyEvent &evt) {
	if (evt.key == OIS::KC_ESCAPE)
		m_simulation->requestStateChange(SHUTDOWN);
    
	return true;
}
#endif

// JoyStickListener
bool InputHandler::buttonPressed(const OIS::JoyStickEvent &evt, int index) {
	return true;
}

bool InputHandler::buttonReleased(const OIS::JoyStickEvent &evt, int index) {
	return true;
}

bool InputHandler::axisMoved(const OIS::JoyStickEvent &evt, int index) {
	return true;
}

bool InputHandler::povMoved(const OIS::JoyStickEvent &evt, int index) {
	return true;
}

