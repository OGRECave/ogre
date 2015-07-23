/*
 * -----------------------------------------------------------------------------
 * This source file is part of OGRE
 * (Object-oriented Graphics Rendering Engine)
 * For the latest info, see http://www.ogre3d.org/
 *
 * Copyright (c) 2000-2014 Torus Knot Software Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * -----------------------------------------------------------------------------
 */

#ifndef __LogConfigSerializer_H_
#define __LogConfigSerializer_H_

#include "OgreLodPrerequisites.h"

#include "OgreDataStream.h"
#include "OgreSerializer.h"

namespace Ogre
{

    class _OgreLodExport LodConfigSerializer :
        public Ogre::Serializer
    {

    public:

        LodConfigSerializer();

        void exportLodConfig(Ogre::LodConfig& config, const Ogre::String& filename,
                             Endian endianMode = ENDIAN_NATIVE);
        void exportLodConfig(Ogre::LodConfig& config, Ogre::DataStreamPtr stream,
                             Endian endianMode = ENDIAN_NATIVE);

        void importLodConfig(Ogre::LodConfig* config, const Ogre::String& filename);
        void importLodConfig(Ogre::LodConfig* config, DataStreamPtr& stream);

    protected:

        enum LodConfigChunkID
        {
            LCCID_FILE_HEADER = 0x300,
            LCCID_LOD_CONFIG = 0x400,
            LCCID_BASIC_INFO = 0x500,
            LCCID_LOD_LEVELS = 0x600,
            LCCID_ADVANCED_INFO = 0x700,
            LCCID_PROFILE = 0x800,
        };

        void readLodConfig();
        void readLodBasicInfo();
        void readLodLevels();
        void readLodAdvancedInfo();
        void readLodProfile();

        void writeLodConfig();
        size_t calcLodConfigSize();
        void writeLodBasicInfo();
        size_t calcLodBasicInfoSize();
        void writeLodLevels();
        size_t calcLodLevelsSize();
        void writeLodAdvancedInfo();
        size_t calcLodAdvancedInfoSize();
        void writeLodProfile();
        size_t calcLodProfileSize();

        LodConfig* mLodConfig;
    };
}
#endif