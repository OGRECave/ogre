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
#ifndef _Ogre_ILUtil_H__
#define _Ogre_ILUtil_H__

#include <OgrePrerequisites.h>
#include <OgreCommon.h>
#include <OgrePixelFormat.h>

namespace Ogre {

    /* 
     * DevIL specific utility class
     **/    
    class _OgrePrivate ILUtil {
    public:
    	/// Structure that encapsulates a devIL image format definition
		struct ILFormat {
			/// Construct an invalidated ILFormat structure
			ILFormat():
				numberOfChannels(0), format(-1), type(-1) {};

			/// Construct a ILFormat from parameters
			ILFormat(int channels, int format, int type=-1):
				numberOfChannels(channels), format(format), type(type) {}

			/// Return wether this structure represents a valid DevIL format
			bool isValid() { return format!=-1; }

			/// Number of channels, usually 3 or 4
			int numberOfChannels;
			/// IL_RGBA,IL_BGRA,IL_DXTx, ...
  			int format;
			/// IL_UNSIGNED_BYTE, IL_UNSIGNED_SHORT, ... may be -1 for compressed formats
  			int type;
		};

        /** Get OGRE format to which a given IL format can be most optimally converted.
         */
        static PixelFormat ilFormat2OgreFormat( int ImageFormat, int ImageType );
        /**	Get IL format that matches a given OGRE format exactly in memory.
        	@remarks	Returns an invalid ILFormat (.isValid()==false) when
        		there is no IL format that matches this.
         */
        static ILFormat OgreFormat2ilFormat( PixelFormat format );      
        /** Convert current IL image to an OGRE format. The size of the target will be
          	PixelUtil::getNumElemBytes(fmt) * ilGetInteger( IL_IMAGE_WIDTH ) * ilGetInteger( IL_IMAGE_HEIGHT ) * ilGetInteger( IL_IMAGE_DEPTH )
          	The IL image type must be IL(_UNSIGNED_)BYTE or IL_FLOAT.
        	The IL image format must be IL_RGBA, IL_BGRA, IL_RGB, IL_BGR, IL_LUMINANCE or IL_LUMINANCE_ALPHA
         
         	@param tar       Target pointer
         	@param ogrefmt   Ogre pixel format to employ
        */
        static void toOgre(const PixelBox &dst);

        /** Convert an OGRE format image to current IL image.
         	@param src       Pixelbox; encapsulates source pointer, width, height, 
         					 depth and format
        */
        static void fromOgre(const PixelBox &src);
    };

}

#endif
