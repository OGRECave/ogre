/*
 -----------------------------------------------------------------------------
 This source file is part of OGRE
 (Object-oriented Graphics Rendering Engine)
 For the latest info, see http://www.ogre3d.org
 
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

#import "OgreOSXCocoaWindow.h"

#import "OgreOSXCocoaWindowDelegate.h"
#include "OgreWindowEventUtilities.h"

using namespace Ogre;

@implementation OSXCocoaWindowDelegate

@synthesize ogreWindow;

- (void)windowDidResize:(NSNotification *)notification
{
    // Update the Ogre window
    OSXCocoaWindow * curWindow = static_cast<OSXCocoaWindow *>(ogreWindow);
    WindowEventUtilities::WindowEventListeners::iterator
        start = WindowEventUtilities::_msListeners.lower_bound(curWindow),
        end = WindowEventUtilities::_msListeners.upper_bound(curWindow);

    curWindow->windowMovedOrResized();

    for( ; start != end; ++start )
        (start->second)->windowResized(curWindow);
}

- (void)windowWillMove:(NSNotification *)notification
{
    OSXCocoaWindow * curWindow = static_cast<OSXCocoaWindow *>(ogreWindow);

    WindowEventUtilities::WindowEventListeners::iterator
        start = WindowEventUtilities::_msListeners.lower_bound(curWindow),
        end = WindowEventUtilities::_msListeners.upper_bound(curWindow);
    
    curWindow->windowMovedOrResized();
    for( ; start != end; ++start )
        (start->second)->windowMoved(curWindow);
}

- (void)windowWillClose:(NSNotification *)notification
{
    OSXCocoaWindow * curWindow = static_cast<OSXCocoaWindow *>(ogreWindow);

    WindowEventUtilities::WindowEventListeners::iterator
    start = WindowEventUtilities::_msListeners.lower_bound(curWindow),
    end = WindowEventUtilities::_msListeners.upper_bound(curWindow);

    for( ; start != end; ++start )
    {
        (start->second)->windowClosing(curWindow);
    }
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    OSXCocoaWindow * curWindow = static_cast<OSXCocoaWindow *>(ogreWindow);
    
    WindowEventUtilities::WindowEventListeners::iterator
    start = WindowEventUtilities::_msListeners.lower_bound(curWindow),
    end = WindowEventUtilities::_msListeners.upper_bound(curWindow);
    
    curWindow->setActive( true );
    for( ; start != end; ++start )
        (start->second)->windowFocusChange(curWindow);
}

- (void)windowDidResignKey:(NSNotification *)notification
{
    OSXCocoaWindow * curWindow = static_cast<OSXCocoaWindow *>(ogreWindow);
    
    WindowEventUtilities::WindowEventListeners::iterator
    start = WindowEventUtilities::_msListeners.lower_bound(curWindow),
    end = WindowEventUtilities::_msListeners.upper_bound(curWindow);
    
    if( curWindow->isDeactivatedOnFocusChange() )
    {
        curWindow->setActive( false );
    }
    
    for( ; start != end; ++start )
        (start->second)->windowFocusChange(curWindow);
}

- (void)windowDidMiniaturize:(NSNotification *)notification
{
    OSXCocoaWindow * curWindow = static_cast<OSXCocoaWindow *>(ogreWindow);
    
    WindowEventUtilities::WindowEventListeners::iterator
    start = WindowEventUtilities::_msListeners.lower_bound(curWindow),
    end = WindowEventUtilities::_msListeners.upper_bound(curWindow);
    
    curWindow->setActive( false );
    curWindow->setVisible( false );
    for( ; start != end; ++start )
        (start->second)->windowFocusChange(curWindow);
}

- (void)windowDidDeminiaturize:(NSNotification *)notification
{
    OSXCocoaWindow * curWindow = static_cast<OSXCocoaWindow *>(ogreWindow);
    
    WindowEventUtilities::WindowEventListeners::iterator
    start = WindowEventUtilities::_msListeners.lower_bound(curWindow),
    end = WindowEventUtilities::_msListeners.upper_bound(curWindow);
    
    curWindow->setActive( true );
    curWindow->setVisible( true );
    for( ; start != end; ++start )
        (start->second)->windowFocusChange(curWindow);
}
@end
