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
#ifndef _OgreHlmsManager_H_
#define _OgreHlmsManager_H_

#include "OgreHlmsCommon.h"
#include "OgreHlmsDatablock.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

#define OGRE_HLMS_NUM_MACROBLOCKS 32
#define OGRE_HLMS_NUM_BLENDBLOCKS 32

    /** HLMS stands for "High Level Material System". */
    class _OgreExport HlmsManager : public PassAlloc
    {
        Hlms* mRegisteredHlms[HLMS_MAX];

        typedef vector<uint8>::type BlockIdxVec;
        HlmsMacroblock      mMacroblocks[OGRE_HLMS_NUM_MACROBLOCKS];
        HlmsBlendblock      mBlendblocks[OGRE_HLMS_NUM_BLENDBLOCKS];
        BlockIdxVec         mActiveMacroblocks;
        BlockIdxVec         mActiveBlendblocks;
        BlockIdxVec         mFreeMacroblockIds;
        BlockIdxVec         mFreeBlendblockIds;

        RenderSystem        *mRenderSystem;

        void renderSystemDestroyAllBlocks(void);

    public:
        HlmsManager();
        virtual ~HlmsManager();

        /// Returns a registered HLMS based on type. May be null.
        Hlms* getHlms( HlmsTypes type )                 { return mRegisteredHlms[type]; }

        /** Creates a macroblock that matches the same parameter as the input. If it already exists,
            returns the existing one.
        @par
            Macroblocks are destroyed by the HlmsManager. Don't try to delete them manually.
        @par
            Up to 32 different macroblocks are supported at the same time.
        @param baseParams
            A macroblock reference to base the parameters. This reference may live on the stack,
            on the heap, etc; it's RS-specific data does not have to be filled.
            e.g. this is fine:
                HlmsMacroblock myRef;
                myRef.mDepthCheck = false;
                HlmsMacroblock *finalBlock = manager->getMacroblock( myRef );
                //myRef.mRsData == finalBlock.mRsData not necessarily true
        @return
            Created or cached datablock with same parameters as baseParams
        */
        const HlmsMacroblock* getMacroblock( const HlmsMacroblock &baseParams );

        /// Destroys a macroblock created by @getMacroblock. Note it performs an O(N) search,
        /// but N <= OGRE_HLMS_NUM_MACROBLOCKS
        void destroyMacroblock( const HlmsMacroblock *macroblock );

        /// @See getMacroblock. This is the same for blend states
        const HlmsBlendblock* getBlendblock( const HlmsBlendblock &baseParams );

        /// Destroys a macroblock created by @getBlendblock. Note it performs an O(N) search,
        /// but N <= OGRE_HLMS_NUM_BLENDBLOCKS
        void destroyBlendblock( const HlmsBlendblock *Blendblock );

        void registerHlms( HlmsTypes type, Hlms *provider );
        void unregisterHlms( HlmsTypes type );

        void _changeRenderSystem( RenderSystem *newRs );
    };
    /** @} */
    /** @} */

}

//#include "OgreHeaderSuffix.h"

#endif
