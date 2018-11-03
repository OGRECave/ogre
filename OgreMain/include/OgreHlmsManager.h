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
#if !OGRE_NO_JSON
    #include "OgreScriptLoader.h"
#endif
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

    class HlmsJsonListener;

#define OGRE_HLMS_MAX_LIFETIME_MACROBLOCKS 4096
#define OGRE_HLMS_MAX_LIFETIME_BLENDBLOCKS 4096

#define OGRE_HLMS_NUM_MACROBLOCKS 32
#define OGRE_HLMS_NUM_BLENDBLOCKS 32
#define OGRE_HLMS_NUM_SAMPLERBLOCKS 64

//Biggest value between all three (Input Layouts is not a block)
#define OGRE_HLMS_MAX_BASIC_BLOCKS OGRE_HLMS_NUM_SAMPLERBLOCKS

    /** HLMS stands for "High Level Material System".

        HlmsMacroblock & HlmsBlendblock pointers are never recycled when their reference counts reach 0.
        This allows us to better cache PSOs instead of destroying them every time materials are
        destroyed.

        A user can only create up to OGRE_HLMS_MAX_LIFETIME_MACROBLOCKS &
        OGRE_HLMS_MAX_LIFETIME_BLENDBLOCKS.
        If for some reason you need more, increase these values (they're macros) and recompile Ogre
    */
    class _OgreExport HlmsManager :
        #if !OGRE_NO_JSON
            public ScriptLoader,
        #endif
            public HlmsAlloc
    {
    public:
        typedef vector<uint16>::type BlockIdxVec;
    protected:
        typedef vector<HlmsMacroblock>::type HlmsMacroblockVec;
        typedef vector<HlmsBlendblock>::type HlmsBlendblockVec;

        Hlms    *mRegisteredHlms[HLMS_MAX];
        bool    mDeleteRegisteredOnExit[HLMS_MAX];
        HlmsCompute *mComputeHlms;

        HlmsMacroblockVec   mMacroblocks;
        HlmsBlendblockVec   mBlendblocks;
        HlmsSamplerblock    mSamplerblocks[OGRE_HLMS_NUM_SAMPLERBLOCKS];
        BlockIdxVec         mActiveBlocks[NUM_BASIC_BLOCKS];
        BlockIdxVec         mFreeBlockIds[NUM_BASIC_BLOCKS];
        BasicBlock          *mBlocks[NUM_BASIC_BLOCKS][OGRE_HLMS_MAX_BASIC_BLOCKS];

        struct InputLayouts
        {
            //OperationType           opType;
            VertexElement2VecVec    vertexElements;
        };

        typedef vector<InputLayouts>::type InputLayoutsVec;

        InputLayoutsVec     mInputLayouts;

        RenderSystem        *mRenderSystem;
        bool                mShadowMappingUseBackFaces;

        HlmsTextureManager  *mTextureManager;

        public: typedef std::map<IdString, HlmsDatablock*> HlmsDatablockMap;
    protected:
        HlmsDatablockMap mRegisteredDatablocks;

        HlmsTypes           mDefaultHlmsType;

#if !OGRE_NO_JSON
        StringVector mScriptPatterns;

    public:
        typedef map<String, String>::type ResourceToTexExtensionMap;
        ResourceToTexExtensionMap   mAdditionalTextureExtensionsPerGroup;
        HlmsJsonListener            *mJsonListener;
    protected:
#endif

        void renderSystemDestroyAllBlocks(void);
        uint16 getFreeBasicBlock( uint8 type, BasicBlock *basicBlock );
        void destroyBasicBlock( BasicBlock *block );

        template <typename T, HlmsBasicBlock type, size_t maxLimit>
        T* getBasicBlock( typename vector<T>::type &container, const T &baseParams );

    public:
        HlmsManager();
        virtual ~HlmsManager();

        /// Increments the reference count for the block, despite being const.
        void addReference( const BasicBlock *block );

        /// Returns a registered HLMS based on type. May be null.
        Hlms* getHlms( HlmsTypes type )                 { return mRegisteredHlms[type]; }

        /// Returns a registered HLMS based on name. May be null.
        Hlms* getHlms( IdString name );

        HlmsCompute* getComputeHlms(void)               { return mComputeHlms; }

        /** Creates a macroblock that matches the same parameter as the input.
            If it already exists, returns the existing one.

            Macroblocks must be destroyed with destroyMacroblock.
            Don't try to delete the pointer directly.

            Calling this function will increase the reference count of the block.
            Make sure to call destroyMacroblock after you're done using it; which will
            decrease the reference count (it won't be actually destroyed until the
            reference is 0).

            Up to 32 different macroblocks are supported at the same time.

            VERY IMPORTANT:

            You can only create up to OGRE_HLMS_MAX_LIFETIME_MACROBLOCKS different macroblocks
            throghout the entire lifetime of HlmsManager, EVEN if you call destroyMacroblock
            on them, as we need to ensure caches of HlmsPso remain valid.

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

        /// Destroys a macroblock created by HlmsManager::getMacroblock.
        /// Blocks are manually reference counted and calling this function will decrease the count.
        ///
        /// The internal object (BasicBlock::mRsData) will actually be destroyed when the count
        /// reaches 0; however the actual pointer is never deallocated throghout the lifetime of
        /// HlmsManager, it is only deactivated.
        ///
        /// This guarantees caches of HlmsPso that once a Macroblock is created,
        /// its pointer always valid.
        ///
        /// When count reaches 0, it will perform an O(N) search but N <= OGRE_HLMS_NUM_MACROBLOCKS
        void destroyMacroblock( const HlmsMacroblock *macroblock );

        /// See HlmsManager::getMacroblock. This is the same for blend states
        /// The block's reference count will be increased. Use destroyBlendblock to decrease it.
        const HlmsBlendblock* getBlendblock( const HlmsBlendblock &baseParams );

        /// @see    destroyMacroblock
        void destroyBlendblock( const HlmsBlendblock *Blendblock );

        /** @See getMacroblock. This is the same for Sampler states
        @remarks
            The block's reference count will be increased. Use destroySamplerblock to decrease it.
            The input is a hard copy because it may be modified if invalid parameters are detected
            (i.e. specifying anisotropic level higher than 1, but no anisotropic filter)
            A warning on the log will be generated in such cases.
        */
        const HlmsSamplerblock* getSamplerblock( HlmsSamplerblock baseParams );

        /// @See destroyMacroblock
        void destroySamplerblock( const HlmsSamplerblock *Samplerblock );

        /** Always returns a unique ID for the given vertexElement / OperationType combination,
            necessary by Hlms to generate a unique PSO.

            We store the OperationType in the last 6 bits because of v1 reasons,
            since v1::VertexDeclaration does not store OperationType, thus it cannot cache
            beforehand, and calling this function from Hlms::calculateHashForV1 would make
            material asignment to SubEntity more expensive than it is.

            That means there 10 bits left for vertexElements, and that's how many different
            vertex elements you can create (1024).

            @see    v1::VertexDeclaration::_getInputLayoutId
        @param vertexElements
        @param opType
        @return
            The returned value is deterministic, however it depends on the order in which
            _getInputLayoutId is called.
        */
        uint16 _getInputLayoutId( const VertexElement2VecVec &vertexElements, OperationType opType );

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

        /// Returns all registered datablocks. @see getDatablock,
        /// @see _datablockAdded, @see _datablockDestroyed
        const HlmsDatablockMap& getDatablocks(void) const   { return mRegisteredDatablocks; }

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

        void registerComputeHlms( HlmsCompute *provider );
        void unregisterComputeHlms(void);

        /** Sets whether or not shadow casters should be rendered into shadow
            textures using their back faces rather than their front faces.
        @remarks
            Rendering back faces rather than front faces into a shadow texture
            can help minimise depth comparison issues, if you're using depth
            shadowmapping. You will probably still need some biasing but you
            won't need as much. For solid objects the result is the same anyway,
            if you have objects with holes you may want to turn this option off.
            The default is to enable this option.
        */
        void setShadowMappingUseBackFaces( bool useBackFaces );

        bool getShadowMappingUseBackFaces(void)             { return mShadowMappingUseBackFaces; }

        void _changeRenderSystem( RenderSystem *newRs );

#if !OGRE_NO_JSON
        /** Opens a file containing a JSON string to load all Hlms materials from.
        @remarks
            You can do:
                HlmsJson hlmsJson( this );
                const char *string = ...;
                hlmsJson.loadMaterials( "Filename for debug purposes", string );
            To load materials from an arbitrary JSON string.
            See HlmsJson::loadMaterials
        @param filename
        @param groupName
        */
        void loadMaterials( const String &filename, const String &groupName,
                            HlmsJsonListener *listener,
                            const String &additionalTextureExtension );

        /** Saves all materials of the registered Hlms at the given file location.
        @param hlmsType
            Hlms type. The type must be registered, otherwise it may crash.
        @param filename
            Valid file path.
        */
        void saveMaterials( HlmsTypes hlmsType,const String &filename,
                            HlmsJsonListener *listener,
                            const String &additionalTextureExtension );

        /** Saves a specific Hlms material at the given file location.
        @param datablock
            Datablock/Material to save
        @param filename
            Valid file path.
        */
        void saveMaterial( const HlmsDatablock *datablock, const String &filename,
                           HlmsJsonListener *listener,
                           const String &additionalTextureExtension );

        //ScriptLoader overloads
        virtual void parseScript(DataStreamPtr& stream, const String& groupName);
        virtual const StringVector& getScriptPatterns(void) const       { return mScriptPatterns; }
        virtual Real getLoadingOrder(void) const;
#endif

        /// Gets the indices of active blocks
        /// @see    HlmsManager::_getBlocks
        const BlockIdxVec& _getActiveBlocksIndices( const HlmsBasicBlock &blockType ) const;

        /// Gets all blocks of a given type. This is an advanced function useful in retrieving
        /// all the Macroblocks, all the Blendblocks, and all the Samplerblocks currently in use.
        /// Example:
        ///     Get all macroblocks:
        ///         const BlockIdxVec &activeMacroblockIdx = mgr->_getActiveBlocksIndices( BLOCK_MACRO );
        ///         BasicBlock const * const *macroblocks = mgr->_getBlocks( BLOCK_MACRO );
        ///         BlockIdxVec::const_iterator itor = activeMacroblockIdx.begin();
        ///         BlockIdxVec::const_iterator end  = activeMacroblockIdx.end();
        ///         while( itor != end )
        ///         {
        ///             const HlmsMacroblock *macroblock = static_cast<const HlmsMacroblock*>(
        ///                                                                  macroblocks[*itor] );
        ///             ++itor;
        ///         }
        BasicBlock const * const *  _getBlocks( const HlmsBasicBlock &blockType ) const;

        /// Gets a macroblock based on its index. @see _getActiveBlocksIndices
        /// to get how which indices are active. @see _getBlocks to retrieve
        /// all types of block in a generic way.
        const HlmsMacroblock* _getMacroblock( uint16 idx ) const;

        /// Gets a blendblock based on its index. @see _getActiveBlocksIndices
        /// to get how which indices are active. @see _getBlocks to retrieve
        /// all types of block in a generic way.
        const HlmsBlendblock* _getBlendblock( uint16 idx ) const;

        /// Gets a samplerblock based on its index. @see _getActiveBlocksIndices
        /// to get how which indices are active. @see _getBlocks to retrieve
        /// all types of block in a generic way.
        const HlmsSamplerblock* _getSamplerblock( uint16 idx ) const;
    };
    /** @} */
    /** @} */

}

//#include "OgreHeaderSuffix.h"

#endif
