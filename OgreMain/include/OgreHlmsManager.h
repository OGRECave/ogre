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
#include "OgreHlmsSamplerblock.h"
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
#define OGRE_HLMS_NUM_SAMPLERBLOCKS 64

//Biggest value between all three
#define OGRE_HLMS_MAX_BASIC_BLOCKS OGRE_HLMS_NUM_SAMPLERBLOCKS

    /** HLMS stands for "High Level Material System". */
    class _OgreExport HlmsManager : public PassAlloc
    {
        Hlms    *mRegisteredHlms[HLMS_MAX];
        bool    mDeleteRegisteredOnExit[HLMS_MAX];

        typedef vector<uint16>::type BlockIdxVec;
        HlmsMacroblock      mMacroblocks[OGRE_HLMS_NUM_MACROBLOCKS];
        HlmsBlendblock      mBlendblocks[OGRE_HLMS_NUM_BLENDBLOCKS];
        HlmsSamplerblock    mSamplerblocks[OGRE_HLMS_NUM_SAMPLERBLOCKS];
        BlockIdxVec         mActiveBlocks[NUM_BASIC_BLOCKS];
        BlockIdxVec         mFreeBlockIds[NUM_BASIC_BLOCKS];
        BasicBlock          *mBlocks[NUM_BASIC_BLOCKS][OGRE_HLMS_MAX_BASIC_BLOCKS];

        RenderSystem        *mRenderSystem;

        HlmsTextureManager  *mTextureManager;

        typedef std::map<IdString, HlmsDatablock*> HlmsDatablockMap;
        HlmsDatablockMap mRegisteredDatablocks;

        HlmsTypes           mDefaultHlmsType;

        void renderSystemDestroyAllBlocks(void);
        uint16 getFreeBasicBlock( uint8 type );
        void destroyBasicBlock( BasicBlock *block );

    public:
        HlmsManager();
        virtual ~HlmsManager();

        /// Increments the reference count for the block, despite being const.
        void addReference( const BasicBlock *block );

        /// Returns a registered HLMS based on type. May be null.
        Hlms* getHlms( HlmsTypes type )                 { return mRegisteredHlms[type]; }

        /** Creates a macroblock that matches the same parameter as the input. If it already exists,
            returns the existing one.
        @par
            Macroblocks are destroyed by the HlmsManager. Don't try to delete them manually.
        @par
            Macroblocks are manually reference counted. They increment with each getMacroblock call.
            Make sure to call destroyMacroblock after you're done using it.
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

        /// Destroys a macroblock created by @getMacroblock. Blocks are manually reference counted
        /// and calling this function will decrease the count.
        /// The object will actually be destroyed when the count reaches 0.
        /// When count reaches 0, it will perform an O(N) search but N <= OGRE_HLMS_NUM_MACROBLOCKS
        void destroyMacroblock( const HlmsMacroblock *macroblock );

        /// @See getMacroblock. This is the same for blend states
        const HlmsBlendblock* getBlendblock( const HlmsBlendblock &baseParams );

        /// @See destroyMacroblock
        void destroyBlendblock( const HlmsBlendblock *Blendblock );

        /** @See getMacroblock. This is the same for Sampler states
        @remarks
            The input is a hard copy because it may be modified if invalid parameters are detected
            (i.e. specifying anisotropic level higher than 1, but no anisotropic filter)
            A warning on the log will be generated in such cases.
        */
        const HlmsSamplerblock* getSamplerblock( HlmsSamplerblock baseParams );

        /// @See destroyMacroblock
        void destroySamplerblock( const HlmsSamplerblock *Samplerblock );

        /** Internal function used by Hlms types to tell us a datablock has been created
            so that we can return it when the user calls @getDatablock.
        @remarks
            Throws if a datablock with the same name has already been registered.
            Don't call this function directly unless you know what you're doing.
        */
        void _datablockAdded( HlmsDatablock *datablock );

        /// Internal function to inform us that the datablock with the input name has been destroyed
        void _datablockDestroyed( IdString name );

        /** Retrieves an exisiting datablock (i.e. material) based on its name, regardless of
            which HLMS type it belongs to.
        @remarks
            If the datablock was created with the flag visibleByManager = false; you can't
            retrieve it using this function. If that's the case, get the appropiate Hlms
            using @getHlms and then call @Hlms::getDatablock on it
        @par
            If the material/datablock with that name wasn't found, returns a default one
            (note that Hlms::getDatablock doesn't do this!!!)
        @param name
            Unique name of the datablock. Datablock names are unique within the same Hlms
            type. If two types create a datablock with the same name and both attempt to
            make it globally visible to this manager, we will throw on creation.
        @return
            Pointer to the datablock
        */
        HlmsDatablock* getDatablock( IdString name ) const;

        /// @See getDatablock. Exactly the same, but returns null pointer if it wasn't found,
        /// instead of going fallback to default.
        HlmsDatablock* getDatablockNoDefault( IdString name ) const;

        /// Alias function. @See getDatablock, as many beginners will probably think of the word
        /// "Material" first. Datablock is a more technical (and accurate) name of what it does
        /// (it's a block.. of data). Prefer calling getDatablock directly.
        HlmsDatablock* getMaterial( IdString name ) const   { return getDatablock( name ); }

        HlmsTextureManager* getTextureManager(void) const   { return mTextureManager; }

        void useDefaultDatablockFrom( HlmsTypes type )      { mDefaultHlmsType = type; }

        /// Datablock to use when another datablock failed or none was specified.
        HlmsDatablock* getDefaultDatablock(void) const;

        /** Registers an HLMS provider. The type is retrieved from the provider. Two providers of
            the same type cannot be registered at the same time (@see HlmsTypes) and will throw
            an exception.
        @param provider
            The HLMS provider being registered.
        @param deleteOnExit
            True if we should delete the pointer using OGRE_DELETE when the provider is
            unregistered or when this manager is destroyed. Otherwise it's caller's
            responsability to free the pointer.
        */
        void registerHlms( Hlms *provider, bool deleteOnExit=true );

        /// Unregisters an HLMS provider of the given type. Does nothing if no provider was registered.
        /// @See registerHlms for details.
        void unregisterHlms( HlmsTypes type );

        void _changeRenderSystem( RenderSystem *newRs );
    };
    /** @} */
    /** @} */

}

//#include "OgreHeaderSuffix.h"

#endif
