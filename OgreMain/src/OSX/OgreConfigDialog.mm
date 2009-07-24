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
        [mWindowController release]; mWindowController = nil;
	}
	
	void ConfigDialog::initialise()
	{
        // Load the view controller from the nib and set up all the options
        mWindowController = [[OgreConfigWindowController alloc] initWithWindowNibName:@"config"];

        if (!mWindowController)
            OGRE_EXCEPT (Exception::ERR_INTERNAL_ERROR, "Could not load config dialog",
                         "ConfigDialog::initialise");

        [mWindowController loadWindow];

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
            [[mWindowController getRenderSystemsPopUp] addItemWithTitle:renderSystemName];
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
        [mWindowController setOptions:[[NSDictionary alloc] initWithObjects:objects forKeys:keys]];

        // Clean up all those arrays
        [fullScreenOptions release];
        [fsaaOptions release];
        [colourDepthOptions release];
        [rttOptions release];
        [videoModeOptions release];
        [keys release];
        [objects release];
        
        // Load and set the logo
        NSString *logoFilePath = [[NSBundle mainBundle] pathForResource:@"ogrelogo" ofType:@"png"];
        NSImage *logo = [[NSImage alloc] initWithContentsOfFile:logoFilePath];
        [logo setSize:NSMakeSize([mWindowController getOgreLogo].bounds.size.width, [mWindowController getOgreLogo].bounds.size.height)];
        [[mWindowController getOgreLogo] setImage:logo];
        [logo release];
        
        // Set an action for when an option is changed
        [[mWindowController getOptionsPopUp] setAction:@selector(popUpValueChanged:)];
        [[mWindowController getOptionsPopUp] setTarget:mWindowController];

        // Set ourselves as the delegate and datasource for the table
        [[mWindowController getOptionsTable] setDataSource:mWindowController];
        [[mWindowController getOptionsTable] setDelegate:mWindowController];
	}

	bool ConfigDialog::display()
	{
        // Select previously selected rendersystem
        mSelectedRenderSystem = Root::getSingleton().getRenderSystem();

        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        initialise();

        // Run a modal dialog, Abort means cancel, Stop means Ok
        long retVal = 0;
        NSModalSession modalSession = [NSApp beginModalSessionForWindow:[mWindowController getConfigWindow]];
        for (;;) {
            retVal = [NSApp runModalSession:modalSession];

            // User pressed a button
            if (retVal != NSRunContinuesResponse)
                break;
        }
        [NSApp endModalSession:modalSession];

        // Set the rendersystem
        Ogre::String selectedRenderSystemName = Ogre::String([[[[mWindowController getRenderSystemsPopUp] selectedItem] title] UTF8String]);
        RenderSystem *rs = Ogre::Root::getSingleton().getRenderSystemByName(selectedRenderSystemName);
        Root::getSingleton().setRenderSystem(rs);
        
        // Relinquish control of the table
        [[mWindowController getOptionsTable] setDataSource:nil];
        [[mWindowController getOptionsTable] setDelegate:nil];
        
        // Drain the auto release pool
        [pool release];

        return (retVal == NSRunStoppedResponse) ? true : false;
	}

};

@implementation OgreConfigWindowController

- (void)dealloc
{
    [options release]; options = nil;
    [super dealloc];
}

- (void)popUpValueChanged:(id)sender
{
    // Grab a copy of the selected RenderSystem name in Ogre::String format
    Ogre::String selectedRenderSystemName = Ogre::String([[[renderSystemsPopUp selectedItem] title] UTF8String]);
    
    // Save the current config value
    if((0 < [optionsTable selectedRow]) && [optionsPopUp selectedItem])
    {
        Ogre::String value = Ogre::String([[[optionsPopUp selectedItem] title] UTF8String]);
        Ogre::String name = Ogre::String([[[[options keyEnumerator] allObjects] objectAtIndex:[optionsTable selectedRow]] UTF8String]);
        
        Ogre::Root::getSingleton().getRenderSystemByName(selectedRenderSystemName)->setConfigOption(name, value);
    }
}

- (void)cancelButtonPressed:(id)sender
{
    // Hide the window
    [configWindow orderOut:nil];

    [NSApp abortModal];
}

- (void)okButtonPressed:(id)sender
{
    // Hide the window
    [configWindow orderOut:nil];

    [NSApp stopModal];
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex
{
    return [[[options keyEnumerator] allObjects] objectAtIndex:rowIndex];
}

- (int)numberOfRowsInTableView:(NSTableView *)aTableView
{
    return [options count];
}

// Intercept the request to select a new row.  Update the popup's values.
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
- (BOOL)tableView:(NSTableView *)aTableView shouldSelectRow:(NSInteger)rowIndex
#else
- (BOOL)tableView:(NSTableView *)aTableView shouldSelectRow:(long)rowIndex
#endif
{
    // Clear out the options popup menu
    [optionsPopUp removeAllItems];
    
    // Get the key for the selected table row
    NSString *key = [[[options keyEnumerator] allObjects] objectAtIndex:rowIndex];
    
    // Add the available options
    [optionsPopUp addItemsWithTitles:[options objectForKey:key]];
    
    // Grab a copy of the selected RenderSystem name in Ogre::String format
    Ogre::String selectedRenderSystemName = Ogre::String([[[renderSystemsPopUp selectedItem] title] UTF8String]);
    const Ogre::ConfigOptionMap& opts = Ogre::Root::getSingleton().getRenderSystemByName(selectedRenderSystemName)->getConfigOptions();

    // Select the item that is the current config option, if there is no current setting, just pick the top of the list
    Ogre::ConfigOptionMap::const_iterator it = opts.find([key UTF8String]);
    if (it != opts.end())
        [optionsPopUp selectItemWithTitle:[NSString stringWithCString:it->second.currentValue.c_str()
                                 encoding:NSASCIIStringEncoding]];

    if([optionsPopUp indexOfSelectedItem] < 0)
        [optionsPopUp selectItemAtIndex:0];

    // Always allow the new selection
    return YES;
}

- (NSDictionary *)getOptions
{
    return options;
}

- (void)setOptions:(NSDictionary *)dict
{
    options = dict;
}

- (NSPopUpButton *)getRenderSystemsPopUp
{
    return renderSystemsPopUp;
}

- (void)setRenderSystemsPopUp:(NSPopUpButton *)button
{
    renderSystemsPopUp = button;
}

- (void)setConfigWindow:(NSWindow *)window
{
    configWindow = window;
}

- (NSWindow *)getConfigWindow
{
    return configWindow;
}

- (void)setOgreLogo:(NSImageView *)image
{
    ogreLogo = image;
}

- (NSImageView *)getOgreLogo
{
    return ogreLogo;
}

- (void)setOptionsTable:(NSTableView *)table
{
    optionsTable = table;
}

- (NSTableView *)getOptionsTable
{
    return optionsTable;
}

- (void)setOptionsPopUp:(NSPopUpButton *)button
{
    optionsPopUp = button;
}

- (NSPopUpButton *)getOptionsPopUp
{
    return optionsPopUp;
}

@end
