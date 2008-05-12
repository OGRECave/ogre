/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

#include "OgreOSXCocoaView.h"

@implementation OgreWindow

@end

@implementation OgreView

- (id)initWithFrame:(NSRect)f
{
	[super initWithFrame:f];
	NSApplicationLoad();
	
	window = 0;

	return self;
}

- (id)initWithGLOSXWindow:(Ogre::RenderWindow*)w
{
	[super init];
	window = w;
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
	window->windowMovedOrResized();
}

- (void)drawRect:(NSRect)r
{
	if(window)
		window->update();
}

@end