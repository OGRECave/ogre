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
#import "Select_Ogre_Install_LocationPane.h"


@implementation Select_Ogre_Install_LocationPane

- (id)initWithSection:(id)parent
{
    if((self = [super initWithSection:parent]))
    {
        validOSXSDKChosen = NO;
        validiPhoneSDKChosen = NO;
        isChoosingiPhoneSDK = NO;
    }
    
    return self;
}

- (NSString *)title
{
	return [[NSBundle bundleForClass:[self class]] localizedStringForKey:@"InstallerPaneTitle" value:nil table:nil];
}

- (BOOL)shouldExitPane:(InstallerSectionDirection)dir
{
    if(!validOSXSDKChosen && !validiPhoneSDKChosen)
    {
        NSAlert *alert = [[[NSAlert alloc] init] autorelease];
        [alert addButtonWithTitle:@"OK"];
        [alert setMessageText:@"No SDK chosen!"];
        [alert setInformativeText:@"Please choose the location of your OGRE SDK."];
        [alert setAlertStyle:NSWarningAlertStyle];
        [alert beginSheetModalForWindow:[ogreOSXLocationLabel window]
                          modalDelegate:self
                         didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
                            contextInfo:nil];
        return NO;
    }
    return YES;
}

- (void)alertDidEnd:(NSAlert *)alert returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
    [[alert window] orderOut:nil];
}

- (IBAction)chooseOSXOgreSDKLocation:(id)sender
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    [openPanel setDelegate:self];
    [openPanel setCanChooseFiles:NO];
    [openPanel setCanChooseDirectories:YES];
    [openPanel setAllowsMultipleSelection:NO];
    [openPanel setMessage:@"Select where the Mac OS X OGRE SDK is installed."];
    isChoosingiPhoneSDK = NO;

    [openPanel beginSheetForDirectory:nil
                                 file:nil
                                types:nil
                       modalForWindow:[ogreOSXLocationLabel window]
                        modalDelegate:self
                       didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:)
                          contextInfo:nil];
}

- (IBAction)chooseiPhoneOgreSDKLocation:(id)sender
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    [openPanel setDelegate:self];
    [openPanel setCanChooseFiles:NO];
    [openPanel setCanChooseDirectories:YES];
    [openPanel setAllowsMultipleSelection:NO];
    [openPanel setMessage:@"Select where the iPhone OGRE SDK is installed."];
    isChoosingiPhoneSDK = YES;

    [openPanel beginSheetForDirectory:nil
                                 file:nil
                                types:nil
                       modalForWindow:[ogreOSXLocationLabel window]
                        modalDelegate:self
                       didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:)
                          contextInfo:nil];
}

- (void)openPanelDidEnd:(NSOpenPanel *)panel returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
    if(NSOKButton == returnCode) {
        NSString *ogreDirectory = [[panel filenames] objectAtIndex:0];

        // Pull the version info out of config.h
        NSMutableString *configFilePath = [ogreDirectory mutableCopy];
        NSString *fileContents = nil;
        NSArray *lines = nil;

        // Validate that this folder contains a valid SDK

        // Using contextInfo to determine whether we're searching for iPhone or Mac
        // YES means iPhone
        if(isChoosingiPhoneSDK)
        {
            // iPhone:
            // lib/release/libOgreMainStatic.a
            // include/OGRE/Ogre.h
            NSString *iPhoneLibPath = [ogreDirectory stringByAppendingString:@"/lib/release/libOgreMainStatic.a"];
            NSString *iPhoneHeaderPath = [ogreDirectory stringByAppendingString:@"/include/OGRE/Ogre.h"];

            if(([[NSFileManager defaultManager] fileExistsAtPath:iPhoneLibPath]) &&
                ([[NSFileManager defaultManager] fileExistsAtPath:iPhoneHeaderPath]))
            {
                // Set the flag to show that will be ok to move on to the next installer section
                validiPhoneSDKChosen = YES;
                
                [configFilePath appendString:@"/include/OGRE/OgrePrerequisites.h"];
                fileContents = [NSString stringWithContentsOfFile:configFilePath];
                lines = [fileContents componentsSeparatedByString:@"\n"];

                NSString *version = [self extractOGREVersionFromLines:lines];
                
                // Update the GUI
                // TODO: Localize this
                [ogreiPhoneLocationLabel setStringValue:[NSString stringWithFormat:@"%@%@ @ %@", @"Found Ogre version ", version, ogreDirectory]];

                // Replace the placeholder string in the project files with the SDK root chosen by the user
                NSMutableString *projectFileContents = [NSMutableString stringWithContentsOfFile:@"/Library/Application Support/Developer/Shared/Xcode/Project Templates/Ogre/Mac OS X/___PROJECTNAME___.xcodeproj/project.pbxproj"];
                projectFileContents = [NSMutableString stringWithContentsOfFile:@"/Library/Application Support/Developer/Shared/Xcode/Project Templates/Ogre/iPhone OS/___PROJECTNAME___.xcodeproj/project.pbxproj"];
                [projectFileContents replaceOccurrencesOfString:@"_OGRESDK_ROOT_"
                                                     withString:ogreDirectory
                                                        options:NSLiteralSearch
                                                          range:NSMakeRange(0, [projectFileContents length])];
                [projectFileContents writeToFile:@"/Library/Application Support/Developer/Shared/Xcode/Project Templates/Ogre/iPhone OS/___PROJECTNAME___.xcodeproj/project.pbxproj" atomically:YES];
            }
            else
            {
                validiPhoneSDKChosen = NO;
            }
        }
        else
        {
            BOOL isDir = NO;
            // Files/folders to look for:
            //
            // OS X:
            // lib/release/Ogre.framework
            NSString *frameworkPath = [ogreDirectory stringByAppendingString:@"/lib/release/Ogre.framework"];
            if(([[NSFileManager defaultManager] fileExistsAtPath:frameworkPath isDirectory:&isDir] && isDir))
            {
                // Set the flag to show that will be ok to move on to the next installer section
                validOSXSDKChosen = YES;

                [configFilePath appendString:@"/lib/release/Ogre.framework/Headers/OgrePrerequisites.h"];
                fileContents = [NSString stringWithContentsOfFile:configFilePath];
                lines = [fileContents componentsSeparatedByString:@"\n"];

                NSString *version = [self extractOGREVersionFromLines:lines];

                // Update the GUI
                // TODO: Localize this
                [ogreOSXLocationLabel setStringValue:[NSString stringWithFormat:@"%@%@ @ %@", @"Found Ogre version ", version, ogreDirectory]];

                // Replace the placeholder string in the project files with the SDK root chosen by the user
                NSMutableString *projectFileContents = [NSMutableString stringWithContentsOfFile:@"/Library/Application Support/Developer/Shared/Xcode/Project Templates/Ogre/Mac OS X/___PROJECTNAME___.xcodeproj/project.pbxproj"];
                [projectFileContents replaceOccurrencesOfString:@"_OGRESDK_ROOT_"
                                                     withString:ogreDirectory
                                                        options:NSLiteralSearch
                                                          range:NSMakeRange(0, [projectFileContents length])];
                [projectFileContents writeToFile:@"/Library/Application Support/Developer/Shared/Xcode/Project Templates/Ogre/Mac OS X/___PROJECTNAME___.xcodeproj/project.pbxproj" atomically:YES];
            }
            else
            {
                validOSXSDKChosen = NO;
            }
        }

        if(validOSXSDKChosen || validiPhoneSDKChosen)
        {
            // Register the file type from extension.  Create a Uniform Type Identifier for each one
            CFStringRef particleUTI = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, CFSTR("particle"), kUTTypePlainText);
            CFStringRef materialUTI = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, CFSTR("material"), kUTTypePlainText);
            CFStringRef compositorUTI = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, CFSTR("compositor"), kUTTypePlainText);
            CFStringRef fontdefUTI = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, CFSTR("fontdef"), kUTTypePlainText);
            CFStringRef overlayUTI = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, CFSTR("overlay"), kUTTypePlainText);
            CFStringRef programUTI = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, CFSTR("program"), kUTTypePlainText);
            CFStringRef genericScriptUTI = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, CFSTR("os"), kUTTypePlainText);

            // Register Xcode as the default editor
            LSSetDefaultRoleHandlerForContentType(particleUTI, (kLSRolesEditor | kLSRolesViewer), CFSTR("com.apple.Xcode"));
            LSSetDefaultRoleHandlerForContentType(materialUTI, (kLSRolesEditor | kLSRolesViewer), CFSTR("com.apple.Xcode"));
            LSSetDefaultRoleHandlerForContentType(compositorUTI, (kLSRolesEditor | kLSRolesViewer), CFSTR("com.apple.Xcode"));
            LSSetDefaultRoleHandlerForContentType(fontdefUTI, (kLSRolesEditor | kLSRolesViewer), CFSTR("com.apple.Xcode"));
            LSSetDefaultRoleHandlerForContentType(overlayUTI, (kLSRolesEditor | kLSRolesViewer), CFSTR("com.apple.Xcode"));
            LSSetDefaultRoleHandlerForContentType(programUTI, (kLSRolesEditor | kLSRolesViewer), CFSTR("com.apple.Xcode"));
            LSSetDefaultRoleHandlerForContentType(genericScriptUTI, (kLSRolesEditor | kLSRolesViewer), CFSTR("com.apple.Xcode"));
        }
        [panel close];
    }
}

- (NSString *)extractOGREVersionFromLines:(NSArray *)lines
{
    NSString *version = @"";
    int ogreMajor = 0;
    int ogreMinor = 0;
    int ogrePatch = 0;
    for(int i = 0; i < [lines count]; i++)
    {
        if([[lines objectAtIndex:i] rangeOfString:@"#define OGRE_VERSION_MAJOR"].length > 0)
        {
            ogreMajor = [[[[[lines objectAtIndex:i] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]]
                           componentsSeparatedByString:@" "] objectAtIndex:2] intValue];
        }
        if([[lines objectAtIndex:i] rangeOfString:@"define OGRE_VERSION_MINOR"].length > 0)
        {
            ogreMinor = [[[[[lines objectAtIndex:i] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]]
                           componentsSeparatedByString:@" "] objectAtIndex:2] intValue];
        }
        if([[lines objectAtIndex:i] rangeOfString:@"define OGRE_VERSION_PATCH"].length > 0)
        {
            ogrePatch = [[[[[lines objectAtIndex:i] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]]
                           componentsSeparatedByString:@" "] objectAtIndex:2] intValue];
        }
    }
    version = [NSString stringWithFormat:@"%i.%i.%i", ogreMajor, ogreMinor, ogrePatch];

    return version;
}

@end
