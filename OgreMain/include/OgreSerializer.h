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

#ifndef __Serializer_H__
#define __Serializer_H__

#include "OgrePrerequisites.h"
#include "OgreString.h"
#include "OgreDataStream.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup General
	*  @{
	*/
	/** Generic class for serialising data to / from binary stream-based files.
    @remarks
        This class provides a number of useful methods for exporting / importing data
        from stream-oriented binary files (e.g. .mesh and .skeleton).
    */
	class _OgreExport Serializer : public SerializerAlloc
    {
    public:
        Serializer();
        virtual ~Serializer();

		/// The endianness of written files
		enum Endian
		{
			/// Use the platform native endian
			ENDIAN_NATIVE,
			/// Use big endian (0x1000 is serialised as 0x10 0x00)
			ENDIAN_BIG,
			/// Use little endian (0x1000 is serialised as 0x00 0x10)
			ENDIAN_LITTLE
		};


    protected:

        uint32 mCurrentstreamLen;
        FILE* mpfFile;
        String mVersion;
		bool mFlipEndian; // default to native endian, derive from header

        // Internal methods
        virtual void writeFileHeader(void);
        virtual void writeChunkHeader(uint16 id, size_t size);
        
        void writeFloats(const float* const pfloat, size_t count);
        void writeFloats(const double* const pfloat, size_t count);
        void writeShorts(const uint16* const pShort, size_t count);
        void writeInts(const uint32* const pInt, size_t count); 
        void writeBools(const bool* const pLong, size_t count);
        void writeObject(const Vector3& vec);
        void writeObject(const Quaternion& q);
        
        void writeString(const String& string);
        void writeData(const void* const buf, size_t size, size_t count);
        
        virtual void readFileHeader(DataStreamPtr& stream);
        virtual unsigned short readChunk(DataStreamPtr& stream);
        
        void readBools(DataStreamPtr& stream, bool* pDest, size_t count);
        void readFloats(DataStreamPtr& stream, float* pDest, size_t count);
        void readFloats(DataStreamPtr& stream, double* pDest, size_t count);
        void readShorts(DataStreamPtr& stream, uint16* pDest, size_t count);
        void readInts(DataStreamPtr& stream, uint32* pDest, size_t count);
        void readObject(DataStreamPtr& stream, Vector3& pDest);
        void readObject(DataStreamPtr& stream, Quaternion& pDest);

        String readString(DataStreamPtr& stream);
        String readString(DataStreamPtr& stream, size_t numChars);
        
        virtual void flipToLittleEndian(void* pData, size_t size, size_t count = 1);
        virtual void flipFromLittleEndian(void* pData, size_t size, size_t count = 1);
        
        virtual void flipEndian(void * pData, size_t size, size_t count);
        virtual void flipEndian(void * pData, size_t size);

		/// Determine the endianness of the incoming stream compared to native
		virtual void determineEndianness(DataStreamPtr& stream);
		/// Determine the endianness to write with based on option
		virtual void determineEndianness(Endian requestedEndian);
    };
	/** @} */
	/** @} */

}


#endif
