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
