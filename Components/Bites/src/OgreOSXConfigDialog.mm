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

#import "OgreLogManager.h"
#import "OgreRoot.h"
#import "OgreRenderSystem.h"
#import "OgreConfigOptionMap.h"

#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>
#include "OgreConfigDialogImp.h"

using namespace Ogre;

#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
@interface OgreConfigWindowDelegate : NSObject <NSWindowDelegate, NSTableViewDelegate, NSTableViewDataSource>
#else
@interface OgreConfigWindowDelegate : NSObject
#endif
{
    NSWindow *mConfigWindow;
    NSImageView *mOgreLogo;
    NSPopUpButton *mRenderSystemsPopUp;
    NSPopUpButton *mOptionsPopUp;
    NSTableView *mOptionsTable;
    NSButton *mOkButton;
    NSButton *mCancelButton;
    NSTextField *mOptionLabel;

    NSDictionary *mOptions;
}

- (void)cancelButtonPressed:(id)sender;
- (void)okButtonPressed:(id)sender;
- (void)popUpValueChanged:(id)sender;

// Getters and setters
- (void)setOptions:(NSDictionary *)dict;
- (NSDictionary *)getOptions;
- (void)setRenderSystemsPopUp:(NSPopUpButton *)button;
- (NSPopUpButton *)getRenderSystemsPopUp;
- (void)setOgreLogo:(NSImageView *)image;
- (NSImageView *)getOgreLogo;
- (void)setConfigWindow:(NSWindow *)window;
- (NSWindow *)getConfigWindow;
- (void)setOptionsTable:(NSTableView *)table;
- (NSTableView *)getOptionsTable;
- (void)setOptionsPopUp:(NSPopUpButton *)button;
- (NSPopUpButton *)getOptionsPopUp;

@end

namespace OgreBites {

    struct ConfigDialog::PrivateData {
        OgreConfigWindowDelegate *mWindowDelegate;
        RenderSystem *mSelectedRenderSystem;
    };

	static ConfigDialog* dlg = NULL;

	ConfigDialog::ConfigDialog() : mImpl(new ConfigDialog::PrivateData())
	{
		dlg = this;
	}
	
	ConfigDialog::~ConfigDialog()
	{
        [mImpl->mWindowDelegate release]; mImpl->mWindowDelegate = nil;
        delete mImpl;
	}
	
	static void initialise(OgreConfigWindowDelegate*& mWindowDelegate)
	{
	    mWindowDelegate = [[OgreConfigWindowDelegate alloc] init];

        if (!mWindowDelegate)
            OGRE_EXCEPT (Exception::ERR_INTERNAL_ERROR, "Could not load config dialog",
                         "ConfigDialog::initialise");

        NSMutableArray *videoModeOptions = [NSMutableArray arrayWithCapacity:1];
        NSMutableArray *fsaaOptions = [NSMutableArray arrayWithCapacity:1];
		const RenderSystemList& renderers = Root::getSingleton().getAvailableRenderers();

        // Add renderers and options that are detected per RenderSystem
        for (RenderSystemList::const_iterator pRend = renderers.begin(); pRend != renderers.end(); ++pRend)
        {
            RenderSystem* rs = *pRend;

            // Set defaults per RenderSystem
			rs->setConfigOption("Video Mode", "800 x 600");
			rs->setConfigOption("Colour Depth", "32");
			rs->setConfigOption("FSAA", "0");
			rs->setConfigOption("Full Screen", "No");
			rs->setConfigOption("RTT Preferred Mode", "FBO");
			rs->setConfigOption("sRGB Gamma Conversion", "No");
			rs->setConfigOption("Content Scaling Factor", "1.0");
            
#if OGRE_NO_QUAD_BUFFER_STEREO == 0
			rs->setConfigOption("Stereo Mode", "None");
#endif

            // Add to the drop down
            NSString *renderSystemName = [[NSString alloc] initWithCString:rs->getName().c_str() encoding:NSASCIIStringEncoding];
            [[mWindowDelegate getRenderSystemsPopUp] addItemWithTitle:renderSystemName];
            [renderSystemName release];
            
            // Get detected option values and add them to our config dictionary
            const ConfigOptionMap& opts = rs->getConfigOptions();
            for (ConfigOptionMap::const_iterator pOpt = opts.begin(); pOpt != opts.end(); ++pOpt)
            {
                if(pOpt->first == "FSAA")
                {
                    for(uint i = 0; i < pOpt->second.possibleValues.size(); i++)
                    {
                        NSString *optionString = [NSString stringWithCString:pOpt->second.possibleValues[i].c_str()
                                                                    encoding:NSASCIIStringEncoding];

                        if(![fsaaOptions containsObject:optionString])
                             [fsaaOptions addObject:optionString];
                    }
                }
                else if(pOpt->first == "Video Mode")
                {
                    for(uint i = 0; i < pOpt->second.possibleValues.size(); i++)
                    {
                        NSString *optionString = [NSString stringWithCString:pOpt->second.possibleValues[i].c_str()
                                                                    encoding:NSASCIIStringEncoding];
                        
                        if(![videoModeOptions containsObject:optionString])
                            [videoModeOptions addObject:optionString];
                    }
                }
            }
        }

        NSDictionary* options = [NSDictionary dictionaryWithObjectsAndKeys:
                                 videoModeOptions, @"Video Mode",
                                 fsaaOptions, @"FSAA",
                                 [NSArray arrayWithObjects:@"Yes", @"No", nil], @"Full Screen",
                                 [NSArray arrayWithObjects:@"32", @"16", nil], @"Colour Depth",
                                 [NSArray arrayWithObjects:@"FBO", @"PBuffer", @"Copy", nil], @"RTT Preferred Mode",
                                 [NSArray arrayWithObjects:@"Yes", @"No", nil], @"sRGB Gamma Conversion",
                                 [NSArray arrayWithObjects:@"2.0", @"1.5", @"1.33", @"1.0", nil], @"Content Scaling Factor",
#if OGRE_NO_QUAD_BUFFER_STEREO == 0
                                 [NSArray arrayWithObjects:@"None", @"Frame Sequential", nil], @"Stereo Mode",
#endif
                                 nil];
		[mWindowDelegate setOptions:options];

        // Reload table data
        [[mWindowDelegate getOptionsTable] reloadData];
	}

	bool ConfigDialog::display()
	{
        // Select previously selected rendersystem
        mImpl->mSelectedRenderSystem = Root::getSingleton().getRenderSystem();
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        initialise(mImpl->mWindowDelegate);

        // Run a modal dialog, Abort means cancel, Stop means Ok
        long retVal = 0;
        NSModalSession modalSession = [NSApp beginModalSessionForWindow:[mImpl->mWindowDelegate getConfigWindow]];
        for (;;) {
            retVal = [NSApp runModalSession:modalSession];

            // User pressed a button
            if (retVal != NSRunContinuesResponse)
                break;
        }
        [NSApp endModalSession:modalSession];

        // Set the rendersystem
        String selectedRenderSystemName = String([[[[mImpl->mWindowDelegate getRenderSystemsPopUp] selectedItem] title] UTF8String]);
        RenderSystem *rs = Root::getSingleton().getRenderSystemByName(selectedRenderSystemName);
        Root::getSingleton().setRenderSystem(rs);
        
        // Relinquish control of the table
        [[mImpl->mWindowDelegate getOptionsTable] setDataSource:nil];
        [[mImpl->mWindowDelegate getOptionsTable] setDelegate:nil];
        
        // Drain the auto release pool
        [pool drain];

        return (retVal == NSRunStoppedResponse) ? true : false;
	}

}

@implementation OgreConfigWindowDelegate

- (id)init
{
    if((self = [super init]))
    {
        // This needs to be called in order to use Cocoa from a Carbon app
        NSApplicationLoad();

        // Construct the window manually
        mConfigWindow = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 512, 512)
                                                    styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask)
                                                      backing:NSBackingStoreBuffered
                                                        defer:NO];

        // Make ourselves the delegate
        [mConfigWindow setDelegate:self];

        // First do the buttons
        mOkButton = [[NSButton alloc] initWithFrame:NSMakeRect(414, 12, 84, 32)];
        [mOkButton setButtonType:NSMomentaryPushInButton];
        [mOkButton setBezelStyle:NSRoundedBezelStyle];
        [mOkButton setTitle:NSLocalizedString(@"OK", @"okButtonString")];
        [mOkButton setAction:@selector(okButtonPressed:)];
        [mOkButton setTarget:self];
        [mOkButton setKeyEquivalent:@"\r"];
        [[mConfigWindow contentView] addSubview:mOkButton];

        mCancelButton = [[NSButton alloc] initWithFrame:NSMakeRect(330, 12, 84, 32)];
        [mCancelButton setButtonType:NSMomentaryPushInButton];
        [mCancelButton setBezelStyle:NSRoundedBezelStyle];
        [mCancelButton setAction:@selector(cancelButtonPressed:)];
        [mCancelButton setTarget:self];
        [mCancelButton setKeyEquivalent:@"\e"];
        [mCancelButton setTitle:NSLocalizedString(@"Cancel", @"cancelButtonString")];
        [[mConfigWindow contentView] addSubview:mCancelButton];

        // Then the Ogre logo out of the framework bundle
        mOgreLogo = [[NSImageView alloc] initWithFrame:NSMakeRect(0, 295, 512, 220)];
        NSMutableString *logoPath = [[[NSBundle bundleForClass:[self class]] resourcePath] mutableCopy];
        [logoPath appendString:@"/ogrelogo.png"];

        NSImage *image = [[NSImage alloc] initWithContentsOfFile:logoPath];
        [logoPath release];
        [mOgreLogo setImage:image];
        [mOgreLogo setImageScaling:NSScaleToFit];
        [mOgreLogo setEditable:NO];
        [image release];
        [[mConfigWindow contentView] addSubview:mOgreLogo];

        // Popup menu for rendersystems.  On OS X this is always OpenGL
        mRenderSystemsPopUp = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(168, 259, 327, 26) pullsDown:NO];
        [[mConfigWindow contentView] addSubview:mRenderSystemsPopUp];

        NSTextField *renderSystemLabel = [[NSTextField alloc] initWithFrame:NSMakeRect(18, 265, 148, 17)];
        [renderSystemLabel setStringValue:NSLocalizedString(@"Rendering Subsystem", @"renderingSubsystemString")];
        [renderSystemLabel setEditable:NO];
        [renderSystemLabel setSelectable:NO];
        [renderSystemLabel setDrawsBackground:NO];
        [renderSystemLabel setAlignment:NSNaturalTextAlignment];
        [renderSystemLabel setBezeled:NO];
        [[mConfigWindow contentView] addSubview:renderSystemLabel];
        [renderSystemLabel release];

        // The pretty box to contain the table and options
        NSBox *tableBox = [[NSBox alloc] initWithFrame:NSMakeRect(19, 54, 477, 203)];
        [tableBox setTitle:NSLocalizedString(@"Rendering System Options", @"optionsBoxString")];
        [tableBox setContentViewMargins:NSMakeSize(0, 0)];
        [tableBox setFocusRingType:NSFocusRingTypeNone];
        [tableBox setBorderType:NSLineBorder];

        // Set up the tableview
        mOptionsTable = [[NSTableView alloc] init];
        [mOptionsTable setDelegate:self];
        [mOptionsTable setDataSource:self];
        [mOptionsTable setHeaderView:nil];
        [mOptionsTable setUsesAlternatingRowBackgroundColors:YES];
        [mOptionsTable sizeToFit];
        
        // Table column to hold option names
        NSTableColumn *column = [[NSTableColumn alloc] initWithIdentifier: @"optionName"];
        [column setEditable:NO];
        [column setMinWidth:437];
        [mOptionsTable addTableColumn:column];
        [column release];

        // Scroll view to hold the table in case the list grows some day
        NSScrollView *scrollView = [[NSScrollView alloc] initWithFrame:NSMakeRect(22, 42, 439, 135)];
        [scrollView setBorderType:NSBezelBorder];
        [scrollView setAutoresizesSubviews:YES];
        [scrollView setAutohidesScrollers:YES];
        [scrollView setDocumentView:mOptionsTable];
        
        [[tableBox contentView] addSubview:scrollView];
        [scrollView release];

        mOptionLabel = [[NSTextField alloc] initWithFrame:NSMakeRect(15, 15, 173, 17)];
        [mOptionLabel setStringValue:NSLocalizedString(@"Select an Option", @"optionLabelString")];
        [mOptionLabel setEditable:NO];
        [mOptionLabel setSelectable:NO];
        [mOptionLabel setDrawsBackground:NO];
        [mOptionLabel setAlignment:NSRightTextAlignment];
        [mOptionLabel setBezeled:NO];
        [[tableBox contentView] addSubview:mOptionLabel];

        mOptionsPopUp = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(190, 10, 270, 26) pullsDown:NO];
        [[tableBox contentView] addSubview:mOptionsPopUp];
        [mOptionsPopUp setAction:@selector(popUpValueChanged:)];
        [mOptionsPopUp setTarget:self];

        [[mConfigWindow contentView] addSubview:tableBox];
        [tableBox release];
    }
    return self;
}

- (void)dealloc
{
    [mOptions release]; mOptions = nil;
    [mConfigWindow release]; mConfigWindow = nil;
    [mOptionsPopUp release]; mOptionsPopUp = nil;
    [mOptionLabel release]; mOptionLabel = nil;
    [mOptionsTable release]; mOptionsTable = nil;
    [mRenderSystemsPopUp release]; mRenderSystemsPopUp = nil;
    [mOgreLogo release]; mOgreLogo = nil;
    [mCancelButton release]; mCancelButton = nil;
    [mOkButton release]; mOkButton = nil;

    [super dealloc];
}

#pragma mark Window and Control delegate methods

- (void)popUpValueChanged:(id)sender
{
#pragma unused(sender)
    // Grab a copy of the selected RenderSystem name in String format
    String selectedRenderSystemName = String([[[mRenderSystemsPopUp selectedItem] title] UTF8String]);
    
    // Save the current config value
    if((0 <= [mOptionsTable selectedRow]) && [mOptionsPopUp selectedItem])
    {
        String value = String([[[mOptionsPopUp selectedItem] title] UTF8String]);
        String name = String([[[[mOptions keyEnumerator] allObjects] objectAtIndex:[mOptionsTable selectedRow]] UTF8String]);
        
        Root::getSingleton().getRenderSystemByName(selectedRenderSystemName)->setConfigOption(name, value);
    }
}

- (BOOL)windowShouldClose:(id)sender
{
#pragma unused(sender)
    // Hide the window
    [mConfigWindow orderOut:nil];
    
    [NSApp abortModal];

    return true;
}

- (void)cancelButtonPressed:(id)sender
{
#pragma unused(sender)
    // Hide the window
    [mConfigWindow orderOut:nil];

    [NSApp abortModal];
    [NSApp terminate:nil];
}

- (void)okButtonPressed:(id)sender
{
#pragma unused(sender)
    // Hide the window
    [mConfigWindow orderOut:nil];

    [NSApp stopModal];
}

#pragma mark NSTableView delegate and datasource methods
#if defined(MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
#else
- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex
#endif
{
#pragma unused(aTableView)
    return [[[mOptions keyEnumerator] allObjects] objectAtIndex:rowIndex];
}

#if defined(MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
#else
- (int)numberOfRowsInTableView:(NSTableView *)aTableView
#endif
{
#pragma unused(aTableView)
    return [mOptions count];
}

// Intercept the request to select a new row.  Update the popup's values.
#if defined(MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
- (BOOL)tableView:(NSTableView *)aTableView shouldSelectRow:(NSInteger)rowIndex
#else
- (BOOL)tableView:(NSTableView *)aTableView shouldSelectRow:(int)rowIndex
#endif
{
#pragma unused(aTableView)
    // Clear out the options popup menu
    [mOptionsPopUp removeAllItems];
    
    // Get the key for the selected table row
    NSString *key = [[[mOptions keyEnumerator] allObjects] objectAtIndex:rowIndex];
    
    // Add the available options
    [mOptionsPopUp addItemsWithTitles:[mOptions objectForKey:key]];
    
    // Grab a copy of the selected RenderSystem name in String format
    if([mRenderSystemsPopUp numberOfItems] > 0)
    {
        String selectedRenderSystemName = String([[[mRenderSystemsPopUp selectedItem] title] UTF8String]);
        const ConfigOptionMap& opts = Root::getSingleton().getRenderSystemByName(selectedRenderSystemName)->getConfigOptions();

        // Select the item that is the current config option, if there is no current setting, just pick the top of the list
        ConfigOptionMap::const_iterator it = opts.find([key UTF8String]);
        if (it != opts.end())
            [mOptionsPopUp selectItemWithTitle:[NSString stringWithCString:it->second.currentValue.c_str()
                                     encoding:NSASCIIStringEncoding]];

        if([mOptionsPopUp indexOfSelectedItem] < 0)
            [mOptionsPopUp selectItemAtIndex:0];

        // Always allow the new selection
        return YES;
    }
    else
    {
        return NO;
    }
}

#pragma mark Getters and Setters
- (NSWindow *)getConfigWindow
{
    return mConfigWindow;
}

- (void)setConfigWindow:(NSWindow *)window
{
    mConfigWindow = window;
}

- (NSDictionary *)getOptions
{
    return mOptions;
}

- (void)setOptions:(NSDictionary *)dict
{
    [dict retain];
    [mOptions release];
    mOptions = dict;
}

- (NSPopUpButton *)getRenderSystemsPopUp
{
    return mRenderSystemsPopUp;
}

- (void)setRenderSystemsPopUp:(NSPopUpButton *)button
{
    mRenderSystemsPopUp = button;
}

- (void)setOgreLogo:(NSImageView *)image
{
    mOgreLogo = image;
}

- (NSImageView *)getOgreLogo
{
    return mOgreLogo;
}

- (void)setOptionsTable:(NSTableView *)table
{
    mOptionsTable = table;
}

- (NSTableView *)getOptionsTable
{
    return mOptionsTable;
}

- (void)setOptionsPopUp:(NSPopUpButton *)button
{
    mOptionsPopUp = button;
}

- (NSPopUpButton *)getOptionsPopUp
{
    return mOptionsPopUp;
}

@end
