/*
 -----------------------------------------------------------------------------
 This source file is part of OGRE
 (Object-oriented Graphics Rendering Engine)
 For the latest info, see http://www.ogre3d.org
 
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

#import "OgreOSXCocoaWindow.h"

#import "OgreOSXCocoaWindowDelegate.h"
#import "OgreWindowEventUtilities.h"

using namespace Ogre;

@implementation CocoaWindowDelegate


-(id)initWithNSWindow:(NSWindow*)nswin ogreWindow:(RenderWindow*)ogrewin
{
    if ((self = [super init]))
    {
		window = nswin;
		ogreWindow = ogrewin;
		
        // Register ourselves for several window event notifications
		// Note that
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(windowDidResize:)
                                                     name:@"NSWindowDidResizeNotification" 
												   object:window];

        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(windowWillMove:)
                                                     name:@"NSWindowWillMoveNotification"
												   object:window];

        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(windowWillClose:)
                                                     name:@"NSWindowWillCloseNotification"
												   object:window];

        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(windowDidBecomeKey:)
                                                     name:@"NSWindowDidBecomeKeyNotification" 
												   object:window];

        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(windowDidResignKey:)
                                                     name:@"NSWindowDidResignKeyNotification"
												   object:window];
        
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(windowDidMiniaturize:)
                                                     name:@"NSWindowDidMiniaturizeNotification" 
												   object:window];

        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(windowDidDeminiaturize:)
                                                     name:@"NSWindowDidDeminiaturizeNotification" 
												   object:window];
    }
    return self;
}

- (void)dealloc {
    // Stop observing notifications
    [[NSNotificationCenter defaultCenter] removeObserver:self];

    [super dealloc];
}

- (void)windowDidResize:(NSNotification *)notification
{
	// Update the Ogre window
	CocoaWindow * curWindow = static_cast<CocoaWindow *>(ogreWindow);
	WindowEventUtilities::WindowEventListeners::iterator
	start = WindowEventUtilities::_msListeners.lower_bound(curWindow),
	end = WindowEventUtilities::_msListeners.upper_bound(curWindow);
	
	curWindow->windowMovedOrResized();
	
	for( ; start != end; ++start )
		(start->second)->windowResized(curWindow);
		
}

- (void)windowWillMove:(NSNotification *)notification
{
    CocoaWindow * curWindow = static_cast<CocoaWindow *>(ogreWindow);

    WindowEventUtilities::WindowEventListeners::iterator
        start = WindowEventUtilities::_msListeners.lower_bound(curWindow),
        end = WindowEventUtilities::_msListeners.upper_bound(curWindow);
    
    curWindow->windowMovedOrResized();
    for( ; start != end; ++start )
        (start->second)->windowMoved(curWindow);
}

- (void)windowWillClose:(NSNotification *)notification
{
    CocoaWindow * curWindow = static_cast<CocoaWindow *>(ogreWindow);

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
    CocoaWindow * curWindow = static_cast<CocoaWindow *>(ogreWindow);
    
    WindowEventUtilities::WindowEventListeners::iterator
    start = WindowEventUtilities::_msListeners.lower_bound(curWindow),
    end = WindowEventUtilities::_msListeners.upper_bound(curWindow);
    
    curWindow->setActive( true );
    for( ; start != end; ++start )
        (start->second)->windowFocusChange(curWindow);
}

- (void)windowDidResignKey:(NSNotification *)notification
{
    CocoaWindow * curWindow = static_cast<CocoaWindow *>(ogreWindow);
    
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
    CocoaWindow * curWindow = static_cast<CocoaWindow *>(ogreWindow);
    
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
    CocoaWindow * curWindow = static_cast<CocoaWindow *>(ogreWindow);
    
    WindowEventUtilities::WindowEventListeners::iterator
    start = WindowEventUtilities::_msListeners.lower_bound(curWindow),
    end = WindowEventUtilities::_msListeners.upper_bound(curWindow);
    
    curWindow->setActive( true );
    curWindow->setVisible( true );
    for( ; start != end; ++start )
        (start->second)->windowFocusChange(curWindow);
}
@end
