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

#import "OgreOSXCocoaView.h"
#import <AppKit/AppKit.h>

@implementation OgreGL3PlusView

- (id)initWithFrame:(NSRect)f
{
	if((self = [super initWithFrame:f]))
    {
        NSApplicationLoad();
        
        window = 0;
    }
	return self;
}

- (id)initWithGLOSXWindow:(Ogre::RenderWindow*)w
{
	if((self = [super initWithFrame:NSMakeRect(0, 0, w->getWidth(), w->getHeight())]))
    {
        window = w;
    }
	return self;
}

- (void)setOgreWindow:(Ogre::RenderWindow*)w
{
	window = w;
}

- (Ogre::RenderWindow*)ogreWindow
{
	return window;
}

- (void)setFrameSize:(NSSize)s
{
	[super setFrameSize:s];
    if (window)
        window->windowMovedOrResized();
}

- (void)drawRect:(NSRect)r
{
	if(window)
		window->update();
}

- (BOOL)acceptsFirstResponder
{
    return NO;
}

- (BOOL)canBecomeKeyView
{
    return NO;
}

@end
