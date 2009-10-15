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
#include "simulation.h"
#include "OgreStringConverter.h"

Simulation::Simulation() {
	m_state = STARTUP;
}

Simulation::~Simulation() {
}

SimulationState Simulation::getCurrentState() {
	return m_state;
}

// For the sake of clarity, I am not using actual thread synchronization 
// objects to serialize access to this resource. You would want to protect
// this block with a mutex or critical section, etc.
bool Simulation::lockState() {
	if (m_locked == false) {

		m_locked = true;
		return true;
	}
	else
		return false;
}

bool Simulation::unlockState() {
	if (m_locked == true) {
		m_locked = false;
		return true;
	}
	else
		return false;
}

bool Simulation::requestStateChange(SimulationState newState) {
	if (m_state == STARTUP) {
		m_locked = false;
		m_state = newState;

		return true;
	}

	// this state cannot be changed once initiated
	if (m_state == SHUTDOWN) {
		return false;
	}

	if ((m_state == GUI || m_state == SIMULATION || m_state == LOADING || m_state == CANCEL_LOADING) && 
			(newState != STARTUP) && (newState != m_state)) {
		m_state = newState;
		return true;
	}
	else
		return false;
}

void Simulation::setFrameTime(float ms) {
	m_frame_time = ms;
}


