/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2016 Torus Knot Software Ltd

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

#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>
#import <UIKit/UIKit.h>

@interface OgreMetalView : UIView

@property (readwrite, nonatomic) BOOL layerSizeDidUpdate;

/// When true (default), we will try to set the contentScaleFactor to the native's.
/// You can use 'nativeScaleFactor' for further control.
/// Note: Changing contentScaleFactor directly will force this value to false.
@property (nonatomic) bool scaleToNative;

/// When scaleToNative = true, instead of setting self.contentScaleFactor, you
/// should change this setting. It's expressed in fractions of the nativeScale.
///
/// For example on an iPad Mini 3 the native factor is 2.0; thus if you set
/// nativeScaleFactor = 1; then contentScaleFactor = 2.0
/// If you set nativeScaleFactor = 0.5; we'll set contentScaleFactor = 1.0
@property (nonatomic) CGFloat nativeScaleFactor;

@end
