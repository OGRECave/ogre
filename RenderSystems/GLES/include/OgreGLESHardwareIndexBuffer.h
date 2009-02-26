/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#ifndef __GLESHardwareIndexBuffer_H__
#define __GLESHardwareIndexBuffer_H__

#include "OgreGLESPrerequisites.h"
#include "OgreHardwareIndexBuffer.h"

namespace Ogre {
    class _OgrePrivate GLESHardwareIndexBuffer : public HardwareIndexBuffer
    {
        private:
            GLuint mBufferId;
            // Scratch buffer handling
            void* mcopyPtr;
            bool mLockedToScratch;
            size_t mScratchOffset;
            size_t mScratchSize;
            void* mScratchPtr;
            bool mScratchUploadOnUnlock;

            void clearData(void);

        protected:
            /** See HardwareBuffer. */
            void* lockImpl(size_t offset, size_t length, LockOptions options);
            /** See HardwareBuffer. */
            void unlockImpl(void);

        public:
            GLESHardwareIndexBuffer(IndexType idxType, size_t numIndexes,
                                  HardwareBuffer::Usage usage,
                                  bool useShadowBuffer);
            virtual ~GLESHardwareIndexBuffer();
            /** See HardwareBuffer. */
            void readData(size_t offset, size_t length, void* pDest);
            /** See HardwareBuffer. */
            void writeData(size_t offset, size_t length, 
                const void* pSource, bool discardWholeBuffer = false);
            /** See HardwareBuffer. */
            void _updateFromShadow(void);

            GLuint getGLBufferId(void) const { return mBufferId; }
    };
}

#endif
