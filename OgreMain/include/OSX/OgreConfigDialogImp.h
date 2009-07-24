/*
 -----------------------------------------------------------------------------
 This source file is part of OGRE
 (Object-oriented Graphics Rendering Engine)
 For the latest info, see http://www.ogre3d.org/
 
 Copyright (c) 2000-2006 Torus Knot Software Ltd
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
 
 You may alternatively use this source under the terms of a specific version of
 the OGRE Unrestricted License provided you have obtained such a license from
 Torus Knot Software Ltd.
 -----------------------------------------------------------------------------
 */

#ifndef __OSXConfigDialog_H__
#define __OSXConfigDialog_H__

#include "OgrePrerequisites.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreConfigOptionMap.h"

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>

#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
@interface OgreConfigWindowController : NSWindowController <NSTableViewDelegate, NSTableViewDataSource>
#else
@interface OgreConfigWindowController : NSWindowController
#endif
{
    IBOutlet NSWindow *configWindow;
    IBOutlet NSImageView *ogreLogo;
    IBOutlet NSPopUpButton *renderSystemsPopUp;
    IBOutlet NSPopUpButton *optionsPopUp;
    IBOutlet NSTableView *optionsTable;
    IBOutlet NSButton *okButton;
    IBOutlet NSButton *cancelButton;
    IBOutlet NSTextField *optionLabel;
    
    NSDictionary *options;
}

- (IBAction)cancelButtonPressed:(id)sender;
- (IBAction)okButtonPressed:(id)sender;

// Getters and setters
- (void)setOptions:(NSDictionary *)dict;
- (NSDictionary *)getOptions;
- (void)setRenderSystemsPopUp:(NSPopUpButton *)button;
- (NSPopUpButton *)getRenderSystemsPopUp;
- (void)setConfigWindow:(NSWindow *)window;
- (NSWindow *)getConfigWindow;
- (void)setOgreLogo:(NSImageView *)image;
- (NSImageView *)getOgreLogo;
- (void)setOptionsTable:(NSTableView *)table;
- (NSTableView *)getOptionsTable;
- (void)setOptionsPopUp:(NSPopUpButton *)button;
- (NSPopUpButton *)getOptionsPopUp;

@end
#endif

namespace Ogre
{
	class _OgreExport ConfigDialog : public UtilityAlloc
	{
	public:
		ConfigDialog();
		~ConfigDialog();
	
		void initialise();
		bool display();

	protected:
#ifdef __OBJC__
        OgreConfigWindowController *mWindowController;
#endif
		RenderSystem *mSelectedRenderSystem;
	};
}

#endif // __OSX_CONFIG_DIALOG_H__
