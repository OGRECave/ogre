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

#import "macUtils.h"

#import "OgreString.h"
#import <Foundation/Foundation.h>
#import <dlfcn.h>

namespace Ogre {

    CFBundleRef mac_loadExeBundle(const char *name) {
        CFBundleRef baseBundle = CFBundleGetBundleWithIdentifier(CFSTR("org.ogre3d.Ogre"));
        CFBundleRef mainBundle = CFBundleGetMainBundle();
        CFStringRef nameRef = CFStringCreateWithCString(NULL, name, kCFStringEncodingASCII);
        CFURLRef bundleURL = 0; //URL of bundle to load
        CFBundleRef bundle = 0; //bundle to load
        
        //cut off .bundle if present
        if(CFStringHasSuffix(nameRef, CFSTR(".bundle"))) {
            CFStringRef nameTempRef = nameRef;
            long end = CFStringGetLength(nameTempRef) - CFStringGetLength(CFSTR(".bundle"));
            nameRef = CFStringCreateWithSubstring(NULL, nameTempRef, CFRangeMake(0, end));
            CFRelease(nameTempRef);
        }
                
        //assume relative to Resources/ directory of Main bundle
        bundleURL = CFBundleCopyResourceURL(mainBundle, nameRef, CFSTR("bundle"), NULL);
        if(bundleURL) {
            bundle = CFBundleCreate(NULL, bundleURL);
            CFRelease(bundleURL);
        }
        
        //otherwise, try Resources/ directory of Ogre Framework bundle
        if(!bundle) {
            bundleURL = CFBundleCopyResourceURL(baseBundle, nameRef, CFSTR("bundle"), NULL);
            if(bundleURL) {
               bundle = CFBundleCreate(NULL, bundleURL);
               CFRelease(bundleURL);
            }
        }
        CFRelease(nameRef);
       
        if(bundle) {
            if(CFBundleLoadExecutable(bundle)) {
                return bundle;
            }
            else {
                CFRelease(bundle);
            }
        }
        
        return 0;
    }
    
    void *mac_getBundleSym(CFBundleRef bundle, const char *name) {
        CFStringRef nameRef = CFStringCreateWithCString(NULL, name, kCFStringEncodingASCII);
        void *sym = CFBundleGetFunctionPointerForName(bundle, nameRef);
        CFRelease(nameRef);
        return sym;
    }
    
    //returns 1 on error, 0 otherwise
    bool mac_unloadExeBundle(CFBundleRef bundle) {
        if(bundle) {
            //no-op, can't unload Obj-C bundles without crashing
            return 0;
        }
        return 1;
    }

    void* mac_loadFramework(String name)
	{
        size_t lastSlashPos = name.find_last_of('/');
        size_t extensionPos = name.rfind(".framework");

        if (lastSlashPos != String::npos && extensionPos != String::npos && extensionPos > lastSlashPos)
        {
            // path, like "/Library/Frameworks/OgreTerrain.framework", append /OgreTerrain
            String realName = name.substr(lastSlashPos + 1, extensionPos - lastSlashPos - 1);

            name += "/" + realName; 
        }

        return dlopen(name.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    }

    void* mac_loadDylib(const char* name)
    {
        return dlopen(name, RTLD_LAZY | RTLD_GLOBAL);
    }
	
    String macBundlePath()
    {
        char path[1024];
        CFBundleRef mainBundle = CFBundleGetMainBundle();
        assert(mainBundle);
        
        CFURLRef mainBundleURL = CFBundleCopyBundleURL(mainBundle);
        assert(mainBundleURL);
        
        CFStringRef cfStringRef = CFURLCopyFileSystemPath( mainBundleURL, kCFURLPOSIXPathStyle);
        assert(cfStringRef);
        
        CFStringGetFileSystemRepresentation(cfStringRef, path, 1024);
        
        CFRelease(mainBundleURL);
        CFRelease(cfStringRef);
        
        return String(path);
    }
    
    String macCachePath()
    {
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
        NSString *cachesDirectory = [paths objectAtIndex:0];
        
        return [cachesDirectory cStringUsingEncoding:NSASCIIStringEncoding];
    }

    String macTempFileName()
    {
        NSString *tempFilePath;
        NSFileManager *fileManager = [NSFileManager defaultManager];
        for (;;) {
            NSString *baseName = [NSString stringWithFormat:@"tmp-%x", arc4random()];
            tempFilePath = [NSTemporaryDirectory() stringByAppendingPathComponent:baseName];
            if (![fileManager fileExistsAtPath:tempFilePath])
                break;
        }
        return String([tempFilePath cStringUsingEncoding:NSASCIIStringEncoding]);
    }
}
