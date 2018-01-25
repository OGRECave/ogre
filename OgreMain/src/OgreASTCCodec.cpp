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

#include "OgreStableHeaders.h"
#include "OgreASTCCodec.h"

#include <cmath>
#include "OgreImage.h"

namespace Ogre {

    const uint32 ASTC_MAGIC = 0x5CA1AB13;

    typedef struct
    {
        uint8 magic[4];
        uint8 blockdim_x;
        uint8 blockdim_y;
        uint8 blockdim_z;
        uint8 xsize[3];			// x-size = xsize[0] + xsize[1] + xsize[2]
        uint8 ysize[3];			// x-size, y-size and z-size are given in texels;
        uint8 zsize[3];			// block count is inferred
    } ASTCHeader;

    float ASTCCodec::getBitrateForPixelFormat(PixelFormat fmt)
    {
        switch (fmt)
        {
        case PF_ASTC_RGBA_4X4_LDR:
            return 8.00;
        case PF_ASTC_RGBA_5X4_LDR:
            return 6.40;
        case PF_ASTC_RGBA_5X5_LDR:
            return 5.12;
        case PF_ASTC_RGBA_6X5_LDR:
            return 4.27;
        case PF_ASTC_RGBA_6X6_LDR:
            return 3.56;
        case PF_ASTC_RGBA_8X5_LDR:
            return 3.20;
        case PF_ASTC_RGBA_8X6_LDR:
            return 2.67;
        case PF_ASTC_RGBA_8X8_LDR:
            return 2.00;
        case PF_ASTC_RGBA_10X5_LDR:
            return 2.56;
        case PF_ASTC_RGBA_10X6_LDR:
            return 2.13;
        case PF_ASTC_RGBA_10X8_LDR:
            return 1.60;
        case PF_ASTC_RGBA_10X10_LDR:
            return 1.28;
        case PF_ASTC_RGBA_12X10_LDR:
            return 1.07;
        case PF_ASTC_RGBA_12X12_LDR:
            return 0.89;

        default:
            return 0;
        }
    }

    // Utility function to determine 2D block dimensions from a target bitrate. Used for 3D textures.
    // Taken from astc_toplevel.cpp in ARM's ASTC Evaluation Codec
    void ASTCCodec::getClosestBlockDim2d(float targetBitrate, int *x, int *y) const
    {
        int blockdims[6] = { 4, 5, 6, 8, 10, 12 };

        float best_error = 1000;
        float aspect_of_best = 1;
        int i, j;

        // Y dimension
        for (i = 0; i < 6; i++)
        {
            // X dimension
            for (j = i; j < 6; j++)
            {
                //              NxN       MxN         8x5               10x5              10x6
                int is_legal = (j==i) || (j==i+1) || (j==3 && j==1) || (j==4 && j==1) || (j==4 && j==2);

                if(is_legal)
                {
                    float bitrate = 128.0f / (blockdims[i] * blockdims[j]);
                    float bitrate_error = std::fabs(bitrate - targetBitrate);
                    float aspect = (float)blockdims[j] / blockdims[i];
                    if (bitrate_error < best_error || (bitrate_error == best_error && aspect < aspect_of_best))
                    {
                        *x = blockdims[j];
                        *y = blockdims[i];
                        best_error = bitrate_error;
                        aspect_of_best = aspect;
                    }
                }
            }
        }
    }

    // Taken from astc_toplevel.cpp in ARM's ASTC Evaluation Codec
    void ASTCCodec::getClosestBlockDim3d(float targetBitrate, int *x, int *y, int *z)
    {
        int blockdims[4] = { 3, 4, 5, 6 };

        float best_error = 1000;
        float aspect_of_best = 1;
        int i, j, k;

        for (i = 0; i < 4; i++)	// Z
        {
            for (j = i; j < 4; j++) // Y
            {
                for (k = j; k < 4; k++) // X
                {
                    //              NxNxN              MxNxN                  MxMxN
                    int is_legal = ((k==j)&&(j==i)) || ((k==j+1)&&(j==i)) || ((k==j)&&(j==i+1));

                    if(is_legal)
                    {
                        float bitrate = 128.0f / (blockdims[i] * blockdims[j] * blockdims[k]);
                        float bitrate_error = std::fabs(bitrate - targetBitrate);
                        float aspect = (float)blockdims[k] / blockdims[j] + (float)blockdims[j] / blockdims[i] + (float)blockdims[k] / blockdims[i];

                        if (bitrate_error < best_error || (bitrate_error == best_error && aspect < aspect_of_best))
                        {
                            *x = blockdims[k];
                            *y = blockdims[j];
                            *z = blockdims[i];
                            best_error = bitrate_error;
                            aspect_of_best = aspect;
                        }
                    }
                }
            }
        }
    }

    size_t ASTCCodec::getMemorySize( uint32 width, uint32 height, uint32 depth,
                                     int32 xdim, int32 ydim, PixelFormat fmt )
    {
        float bitrate = getBitrateForPixelFormat(fmt);
        int32 zdim = 1;
        if(depth > 1)
        {
            getClosestBlockDim3d(bitrate, &xdim, &ydim, &zdim);
        }
        int xblocks = (width + xdim - 1) / xdim;
        int yblocks = (height + ydim - 1) / ydim;
        int zblocks = (depth + zdim - 1) / zdim;
        return xblocks * yblocks * zblocks * 16;
    }

	//---------------------------------------------------------------------
	ASTCCodec* ASTCCodec::msInstance = 0;
	//---------------------------------------------------------------------
	void ASTCCodec::startup(void)
	{
		if (!msInstance)
		{
			msInstance = OGRE_NEW ASTCCodec();
			Codec::registerCodec(msInstance);
		}

        LogManager::getSingleton().logMessage(LML_NORMAL,
                                              "ASTC codec registering");
	}
	//---------------------------------------------------------------------
	void ASTCCodec::shutdown(void)
	{
		if(msInstance)
		{
			Codec::unregisterCodec(msInstance);
			OGRE_DELETE msInstance;
			msInstance = 0;
		}
	}
	//---------------------------------------------------------------------
    ASTCCodec::ASTCCodec():
        mType("astc")
    { 
    }
    //---------------------------------------------------------------------
    DataStreamPtr ASTCCodec::encode(const MemoryDataStreamPtr& input, const Codec::CodecDataPtr& pData) const
    {        
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                    "ASTC encoding not supported",
                    "ASTCCodec::encode" ) ;
    }
    //---------------------------------------------------------------------
    void ASTCCodec::encodeToFile(const MemoryDataStreamPtr& input, const String& outFileName,
                                 const Codec::CodecDataPtr& pData) const
    {
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                    "ASTC encoding not supported",
                    "ASTCCodec::encodeToFile" ) ;
	}

    //---------------------------------------------------------------------
    Codec::DecodeResult ASTCCodec::decode(const DataStreamPtr& stream) const
    {
        DecodeResult ret;
        ASTCHeader header;

        // Read the ASTC header
        stream->read(&header, sizeof(ASTCHeader));

		if (memcmp(&ASTC_MAGIC, &header.magic, sizeof(uint32)) != 0 )
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "This is not a valid ASTC file!", "ASTCCodec::decode");

        int xdim = header.blockdim_x;
        int ydim = header.blockdim_y;
        int zdim = header.blockdim_z;

        int xsize = header.xsize[0] + 256 * header.xsize[1] + 65536 * header.xsize[2];
        int ysize = header.ysize[0] + 256 * header.ysize[1] + 65536 * header.ysize[2];
        int zsize = header.zsize[0] + 256 * header.zsize[1] + 65536 * header.zsize[2];

        ImageData *imgData = OGRE_NEW ImageData();
        imgData->width = xsize;
        imgData->height = ysize;
        imgData->depth = zsize;
		imgData->num_mipmaps = 0; // Always 1 mip level per file

        // For 3D we calculate the bitrate then find the nearest 2D block size.
        if(zdim > 1)
        {
            float bitrate = 128.0f / (xdim * ydim * zdim);
            getClosestBlockDim2d(bitrate, &xdim, &ydim);
        }

        if(xdim == 4)
        {
            imgData->format = PF_ASTC_RGBA_4X4_LDR;
        }
        else if(xdim == 5)
        {
            if(ydim == 4)
                imgData->format = PF_ASTC_RGBA_5X4_LDR;
            else if(ydim == 5)
                imgData->format = PF_ASTC_RGBA_5X5_LDR;
        }
        else if(xdim == 6)
        {
            if(ydim == 5)
                imgData->format = PF_ASTC_RGBA_6X5_LDR;
            else if(ydim == 6)
                imgData->format = PF_ASTC_RGBA_6X6_LDR;
        }
        else if(xdim == 8)
        {
            if(ydim == 5)
                imgData->format = PF_ASTC_RGBA_8X5_LDR;
            else if(ydim == 6)
                imgData->format = PF_ASTC_RGBA_8X6_LDR;
            else if(ydim == 8)
                imgData->format = PF_ASTC_RGBA_8X8_LDR;
        }
        else if(xdim == 10)
        {
            if(ydim == 5)
                imgData->format = PF_ASTC_RGBA_10X5_LDR;
            else if(ydim == 6)
                imgData->format = PF_ASTC_RGBA_10X6_LDR;
            else if(ydim == 8)
                imgData->format = PF_ASTC_RGBA_10X8_LDR;
            else if(ydim == 10)
                imgData->format = PF_ASTC_RGBA_10X10_LDR;
        }
        else if(xdim == 12)
        {
            if(ydim == 10)
                imgData->format = PF_ASTC_RGBA_12X10_LDR;
            else if(ydim == 12)
                imgData->format = PF_ASTC_RGBA_12X12_LDR;
        }

        imgData->flags = IF_COMPRESSED;

		size_t numFaces = 1; // Always one face, cubemaps are not currently supported
                             // Calculate total size from number of mipmaps, faces and size
		imgData->size = Image::calculateSize(imgData->num_mipmaps, numFaces,
                                             imgData->width, imgData->height, imgData->depth, imgData->format);

		// Bind output buffer
		MemoryDataStreamPtr output(OGRE_NEW MemoryDataStream(imgData->size));

		// Now deal with the data
		uchar* destPtr = output->getPtr();
        stream->read(destPtr, imgData->size);

		ret.first = output;
		ret.second = CodecDataPtr(imgData);
        
		return ret;
    }
    //---------------------------------------------------------------------    
    String ASTCCodec::getType() const 
    {
        return mType;
    }
    //---------------------------------------------------------------------    
    void ASTCCodec::flipEndian(void * pData, size_t size, size_t count) const
    {
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
		for(unsigned int index = 0; index < count; index++)
        {
            flipEndian((void *)((long)pData + (index * size)), size);
        }
#endif
    }
    //---------------------------------------------------------------------    
    void ASTCCodec::flipEndian(void * pData, size_t size) const
    {
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
        char swapByte;
        for(unsigned int byteIndex = 0; byteIndex < size/2; byteIndex++)
        {
            swapByte = *(char *)((long)pData + byteIndex);
            *(char *)((long)pData + byteIndex) = *(char *)((long)pData + size - byteIndex - 1);
            *(char *)((long)pData + size - byteIndex - 1) = swapByte;
        }
#endif
    }
    //---------------------------------------------------------------------    
	String ASTCCodec::magicNumberToFileExt(const char *magicNumberPtr, size_t maxbytes) const
	{
		if (maxbytes >= sizeof(uint32))
		{
			uint32 fileType;
			memcpy(&fileType, magicNumberPtr, sizeof(uint32));
			flipEndian(&fileType, sizeof(uint32), 1);

			if (ASTC_MAGIC == fileType)
				return String("astc");
		}

        return BLANKSTRING;
	}
}
