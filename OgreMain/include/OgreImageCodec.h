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
#ifndef _ImageCodec_H__
#define _ImageCodec_H__

#include "OgreCodec.h"
#include "OgrePixelFormat.h"

namespace Ogre {

    /** Codec specialized in images.
        @remarks
            The users implementing subclasses of ImageCodec are required to return
            a valid pointer to a ImageData class from the decode(...) function.
    */
    class _OgreExport ImageCodec : public Codec
    {
    public:
        virtual ~ImageCodec();
        /** Codec return class for images. Has information about the size and the
            pixel format of the image. */
        class _OgrePrivate ImageData : public Codec::CodecData
        {
        public:
			ImageData():
				height(0), width(0), depth(1), size(0),
				num_mipmaps(0), flags(0), format(PF_UNKNOWN)
			{
			}
            size_t height;
            size_t width;
			size_t depth;
            size_t size;
            
            ushort num_mipmaps;
            uint flags;

            PixelFormat format;

        public:
            String dataType() const
            {
                return "ImageData";
            }
        };

    public:
        String getDataType() const
        {
            return "ImageData";
        }
    };

} // namespace

#endif
