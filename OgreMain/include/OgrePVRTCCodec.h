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
#ifndef __OgrePVRTCCodec_H__
#define __OgrePVRTCCodec_H__

#include "OgreImageCodec.h"

namespace Ogre {
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Image
	*  @{
	*/

    /** Codec specialized in loading PVRTC (PowerVR) images.
	@remarks
		We implement our own codec here since we need to be able to keep PVRTC
		data compressed if the card supports it.
    */
    class _OgreExport PVRTCCodec : public ImageCodec
    {
    protected:
        String mType;

    	void flipEndian(void * pData, size_t size, size_t count) const;
	    void flipEndian(void * pData, size_t size) const;

		/// Single registered codec instance
		static PVRTCCodec* msInstance;

	public:
        PVRTCCodec();
        virtual ~PVRTCCodec() { }

        /// @copydoc Codec::code
        DataStreamPtr code(MemoryDataStreamPtr& input, CodecDataPtr& pData) const;
        /// @copydoc Codec::codeToFile
        void codeToFile(MemoryDataStreamPtr& input, const String& outFileName, CodecDataPtr& pData) const;
        /// @copydoc Codec::decode
        DecodeResult decode(DataStreamPtr& input) const;
		/// @copydoc Codec::magicNumberToFileExt
		String magicNumberToFileExt(const char *magicNumberPtr, size_t maxbytes) const;
        
        virtual String getType() const;        

		/// Static method to startup and register the PVRTC codec
		static void startup(void);
		/// Static method to shutdown and unregister the PVRTC codec
		static void shutdown(void);

    };
	/** @} */
	/** @} */

} // namespace

#endif

