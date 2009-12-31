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

#include "OgreLogManager.h"
#include "OgreConfigDialog.h"

namespace Ogre {

	ConfigDialog* dlg = NULL;

	ConfigDialog::ConfigDialog()
	{
		dlg = this;
	}
	
	ConfigDialog::~ConfigDialog()
	{
        [mWindowDelegate release]; mWindowDelegate = nil;
	}
	
	void ConfigDialog::initialise()
	{
        mWindowDelegate = [[OgreConfigWindowDelegate alloc] init];

        if (!mWindowDelegate)
            OGRE_EXCEPT (Exception::ERR_INTERNAL_ERROR, "Could not load config dialog",
                         "ConfigDialog::initialise");

        NSArray *keys = [[NSArray alloc] initWithObjects:@"Full Screen", @"FSAA", @"Colour Depth", @"RTT Preferred Mode", @"Video Mode", nil];
        NSArray *fullScreenOptions = [[NSArray alloc] initWithObjects:@"Yes", @"No", nil];
        NSArray *colourDepthOptions = [[NSArray alloc] initWithObjects:@"32", @"16", nil];
        NSArray *rttOptions = [[NSArray alloc] initWithObjects:@"FBO", @"PBuffer", @"Copy", nil];
        NSMutableArray *videoModeOptions = [[NSMutableArray alloc] initWithCapacity:1];
        NSMutableArray *fsaaOptions = [[NSMutableArray alloc] initWithCapacity:1];

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
                        NSString *optionString = [[NSString alloc] initWithCString:pOpt->second.possibleValues[i].c_str()
                                                                    encoding:NSASCIIStringEncoding];

                        if(![fsaaOptions containsObject:optionString])
                             [fsaaOptions addObject:optionString];

                        [optionString release];
                    }
                }
                else if(pOpt->first == "Video Mode")
                {
                    for(uint i = 0; i < pOpt->second.possibleValues.size(); i++)
                    {
                        NSString *optionString = [[NSString alloc] initWithCString:pOpt->second.possibleValues[i].c_str()
                                                                    encoding:NSASCIIStringEncoding];
                        
                        if(![videoModeOptions containsObject:optionString])
                            [videoModeOptions addObject:optionString];
                        
                        [optionString release];
                    }
                }
            }
        }

        NSArray *objects = [[NSArray alloc] initWithObjects:fullScreenOptions, fsaaOptions,
                            colourDepthOptions, rttOptions, videoModeOptions, nil];
        [mWindowDelegate setOptions:[[NSDictionary alloc] initWithObjects:objects forKeys:keys]];

        // Clean up all those arrays
        [fullScreenOptions release];
        [fsaaOptions release];
        [colourDepthOptions release];
        [rttOptions release];
        [videoModeOptions release];
        [keys release];
        [objects release];

        // Reload table data
        [[mWindowDelegate getOptionsTable] reloadData];
	}

	bool ConfigDialog::display()
	{
        // Select previously selected rendersystem
        mSelectedRenderSystem = Root::getSingleton().getRenderSystem();
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        initialise();

        // Run a modal dialog, Abort means cancel, Stop means Ok
        long retVal = 0;
        NSModalSession modalSession = [NSApp beginModalSessionForWindow:[mWindowDelegate getConfigWindow]];
        for (;;) {
            retVal = [NSApp runModalSession:modalSession];

            // User pressed a button
            if (retVal != NSRunContinuesResponse)
                break;
        }
        [NSApp endModalSession:modalSession];

        // Set the rendersystem
        Ogre::String selectedRenderSystemName = Ogre::String([[[[mWindowDelegate getRenderSystemsPopUp] selectedItem] title] UTF8String]);
        RenderSystem *rs = Ogre::Root::getSingleton().getRenderSystemByName(selectedRenderSystemName);
        Root::getSingleton().setRenderSystem(rs);
        
        // Relinquish control of the table
        [[mWindowDelegate getOptionsTable] setDataSource:nil];
        [[mWindowDelegate getOptionsTable] setDelegate:nil];
        
        // Drain the auto release pool
        [pool drain];

        return (retVal == NSRunStoppedResponse) ? true : false;
	}

};

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
        [mCancelButton setTitle:NSLocalizedString(@"Cancel", @"cancelButtonString")];
        [[mConfigWindow contentView] addSubview:mCancelButton];

        // Then the Ogre logo out of the framework bundle
        mOgreLogo = [[NSImageView alloc] initWithFrame:NSMakeRect(0, 295, 512, 220)];
        NSMutableString *logoPath = [[[NSBundle mainBundle] bundlePath] mutableCopy];
        [logoPath appendString:@"/Contents/Frameworks/Ogre.framework/Resources/ogrelogo.png"];

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
        mOptionsTable = [[NSTableView alloc] initWithFrame:NSMakeRect(0, 0, 437, 133)];
        [mOptionsTable setDelegate:self];
        [mOptionsTable setDataSource:self];
        [mOptionsTable setUsesAlternatingRowBackgroundColors:YES];
        [mOptionsTable sizeToFit];
        
        // Table column to hold option names
        NSTableColumn *column = [[NSTableColumn alloc] initWithIdentifier: @"optionName"];
        [column setEditable:NO];
        [column setMinWidth:437];
        [mOptionsTable addTableColumn:column];
        [column release];

        // Scroll view to hold the table in case the list grows some day
        NSScrollView *scrollView = [[NSScrollView alloc] initWithFrame:NSMakeRect(18, 42, 439, 135)];
        [scrollView setBorderType:NSBezelBorder];
        [scrollView setAutoresizesSubviews:YES];
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
    // Grab a copy of the selected RenderSystem name in Ogre::String format
    Ogre::String selectedRenderSystemName = Ogre::String([[[mRenderSystemsPopUp selectedItem] title] UTF8String]);
    
    // Save the current config value
    if((0 <= [mOptionsTable selectedRow]) && [mOptionsPopUp selectedItem])
    {
        Ogre::String value = Ogre::String([[[mOptionsPopUp selectedItem] title] UTF8String]);
        Ogre::String name = Ogre::String([[[[mOptions keyEnumerator] allObjects] objectAtIndex:[mOptionsTable selectedRow]] UTF8String]);
        
        Ogre::Root::getSingleton().getRenderSystemByName(selectedRenderSystemName)->setConfigOption(name, value);
    }
}

- (BOOL)windowShouldClose:(id)sender
{
    // Hide the window
    [mConfigWindow orderOut:nil];
    
    [NSApp abortModal];

    return true;
}

- (void)cancelButtonPressed:(id)sender
{
    // Hide the window
    [mConfigWindow orderOut:nil];

    [NSApp abortModal];
}

- (void)okButtonPressed:(id)sender
{
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
    return [[[mOptions keyEnumerator] allObjects] objectAtIndex:rowIndex];
}

- (int)numberOfRowsInTableView:(NSTableView *)aTableView
{
    return [mOptions count];
}

// Intercept the request to select a new row.  Update the popup's values.
#if defined(MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
- (BOOL)tableView:(NSTableView *)aTableView shouldSelectRow:(NSInteger)rowIndex
#else
- (BOOL)tableView:(NSTableView *)aTableView shouldSelectRow:(int)rowIndex
#endif
{
    // Clear out the options popup menu
    [mOptionsPopUp removeAllItems];
    
    // Get the key for the selected table row
    NSString *key = [[[mOptions keyEnumerator] allObjects] objectAtIndex:rowIndex];
    
    // Add the available options
    [mOptionsPopUp addItemsWithTitles:[mOptions objectForKey:key]];
    
    // Grab a copy of the selected RenderSystem name in Ogre::String format
    Ogre::String selectedRenderSystemName = Ogre::String([[[mRenderSystemsPopUp selectedItem] title] UTF8String]);
    const Ogre::ConfigOptionMap& opts = Ogre::Root::getSingleton().getRenderSystemByName(selectedRenderSystemName)->getConfigOptions();

    // Select the item that is the current config option, if there is no current setting, just pick the top of the list
    Ogre::ConfigOptionMap::const_iterator it = opts.find([key UTF8String]);
    if (it != opts.end())
        [mOptionsPopUp selectItemWithTitle:[NSString stringWithCString:it->second.currentValue.c_str()
                                 encoding:NSASCIIStringEncoding]];

    if([mOptionsPopUp indexOfSelectedItem] < 0)
        [mOptionsPopUp selectItemAtIndex:0];

    // Always allow the new selection
    return YES;
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
