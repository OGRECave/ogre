/* OgreController */

#import <Cocoa/Cocoa.h>
#import <Ogre/OgreOSXCocoaView.h>

@interface OgreController : NSObject
{
    IBOutlet OgreGLView *ogreView;
    NSColor *diffuseLight;
    NSColor *specularLight;
}
@end
