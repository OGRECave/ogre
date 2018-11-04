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
#ifndef _OgreHlms_H_
#define _OgreHlms_H_

#include "OgreStringVector.h"
#include "OgreHlmsCommon.h"
#include "OgreHlmsPso.h"
#if !OGRE_NO_JSON
    #include "OgreHlmsJson.h"
#endif
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    class CompositorShadowNode;
    struct QueuedRenderable;
    typedef vector<Archive*>::type ArchiveVec;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

    /** HLMS stands for "High Level Material System".

        The Hlms has multiple caches:

        mRenderableCache
            This cache contains all the properties set to a Renderable class and can be evaluated
            early, when a Renderable is assigned a datablock i.e. inside Renderable::setDatablock.
            Contains properties such as whether the material has normal mapping, if the mesh
            has UV sets, evaluates if the material requires tangents for normal mapping, etc.
            The main function in charge of filling this cache is Hlms::calculateHashFor

        mPassCache
            This cache contains per-pass information, such as how many lights are in the scene,
            whether this is a shadow mapping pass, etc.
            The main function in charge of filling this cache is Hlms::preparePassHash

        mShaderCodeCache
            Contains a cache of unique shaders (from Hlms templates -> actual valid shader code)
            based on the properties merged from mRenderableCache & mPassCache.
            However it is possible that two shaders are exactly the same and thus be duplicated,
            this can happen if two combinations of properties end up producing the exact same code.
            The Microcode cache (GpuProgramManager::setSaveMicrocodesToCache) can help with that issue.

        mShaderCache
            Contains a cache of the PSOs. The difference between this and mShaderCodeCache is
            that PSOs require additional information, such as HlmsMacroblock. HlmsBlendblock.
            For more information of all that is required, see HlmsPso
    */
    class _OgreExport Hlms : public HlmsAlloc
    {
    public:
        friend class HlmsDiskCache;

        enum LightGatheringMode
        {
            LightGatherForward,
            LightGatherForwardPlus,
            LightGatherDeferred,
            LightGatherNone,
        };

        struct DatablockEntry
        {
            HlmsDatablock   *datablock;
            bool            visibleToManager;
            String          name;
            String          srcFile;            ///Filename in which it was defined, if any
            String          srcResourceGroup;   ///ResourceGroup in which it was defined, if any
            DatablockEntry() : datablock( 0 ), visibleToManager( false ) {}
            DatablockEntry( HlmsDatablock *_datablock, bool _visibleToManager, const String &_name,
                            const String &_srcFile, const String &_srcGroup ) :
                datablock( _datablock ), visibleToManager( _visibleToManager ), name( _name ),
                srcFile( _srcFile ), srcResourceGroup( _srcGroup  ) {}
        };

        typedef std::map<IdString, DatablockEntry> HlmsDatablockMap;

    protected:
        struct RenderableCache
        {
            HlmsPropertyVec setProperties;
            PiecesMap       pieces[NumShaderTypes];

            RenderableCache( const HlmsPropertyVec &properties,
                             const PiecesMap *_pieces ) :
                setProperties( properties )
            {
                if( _pieces )
                {
                    for( size_t i=0; i<NumShaderTypes; ++i )
                        pieces[i] = _pieces[i];
                }
            }

            bool operator == ( const RenderableCache &_r ) const
            {
                bool piecesEqual = true;
                for( size_t i=0; i<NumShaderTypes; ++i )
                    piecesEqual &= pieces[i] == _r.pieces[i];

                return setProperties == _r.setProperties && piecesEqual;
            }
        };

        struct PassCache
        {
            HlmsPropertyVec properties;
            HlmsPassPso     passPso;

            bool operator == ( const PassCache &_r ) const
            {
                return properties == _r.properties && passPso == _r.passPso;
            }
        };

        struct ShaderCodeCache
        {
            /// Contains merged properties (pass and renderable's)
            RenderableCache mergedCache;
            GpuProgramPtr   shaders[NumShaderTypes];

            ShaderCodeCache( const PiecesMap *_pieces ) :
                mergedCache( HlmsPropertyVec(), _pieces )
            {
            }

            bool operator == ( const ShaderCodeCache &_r ) const
            {
                return this->mergedCache == _r.mergedCache;
            }
        };

        typedef vector<PassCache>::type PassCacheVec;
        typedef vector<RenderableCache>::type RenderableCacheVec;
        typedef vector<ShaderCodeCache>::type ShaderCodeCacheVec;

        PassCacheVec        mPassCache;
        RenderableCacheVec  mRenderableCache;
        ShaderCodeCacheVec  mShaderCodeCache;
        HlmsCacheVec        mShaderCache;

        HlmsPropertyVec mSetProperties;
        PiecesMap       mPieces;

    public:
        struct Library
        {
            Archive         *dataFolder;
            StringVector    pieceFiles[NumShaderTypes];
        };

        typedef vector<Library>::type LibraryVec;
    protected:
        LibraryVec      mLibrary;
        Archive         *mDataFolder;
        StringVector    mPieceFiles[NumShaderTypes];
        HlmsManager     *mHlmsManager;

        LightGatheringMode  mLightGatheringMode;
        uint16              mNumLightsLimit;
        uint16              mNumAreaLightsLimit;
        uint8               mAreaLightsRoundMultiple;
        uint32              mAreaLightsGlobalLightListStart;
        uint32              mRealNumAreaApproxLights;
        uint32              mRealNumAreaLtcLights;

        /// Listener for adding extensions. @see setListener.
        /// Pointer is [b]never[/b] null.
        HlmsListener    *mListener;
        RenderSystem    *mRenderSystem;

        HlmsDatablockMap mDatablocks;

        String          mShaderProfile; /// "glsl", "glsles", "hlsl"
        IdString        mShaderSyntax;
        IdStringVec     mRsSpecificExtensions;
        String const    *mShaderTargets[NumShaderTypes]; ///[0] = "vs_4_0", etc. Only used by D3D
        String          mShaderFileExt; /// Either glsl or hlsl
        String          mOutputPath;
        bool            mDebugOutput;
        bool            mDebugOutputProperties;
        bool            mHighQuality;
        bool            mFastShaderBuildHack;

        /// The default datablock occupies the name IdString(); which is not the same as IdString("")
        HlmsDatablock   *mDefaultDatablock;

        HlmsTypes       mType;
        IdString        mTypeName;
        String          mTypeNameStr;

        /** Inserts common properties about the current Renderable,
            such as hlms_skeleton hlms_uv_count, etc
        */
        void setCommonProperties();

        /** Populates all mPieceFiles with all files in mDataFolder with suffix ending in
                piece_vs    - Vertex Shader
                piece_ps    - Pixel Shader
                piece_gs    - Geometry Shader
                piece_hs    - Hull Shader
                piece_ds    - Domain Shader
            Case insensitive.
        */
        void enumeratePieceFiles(void);
        /// Populates pieceFiles, returns true if found at least one piece file.
        static bool enumeratePieceFiles( Archive *dataFolder, StringVector *pieceFiles );

        void setProperty( IdString key, int32 value );
        int32 getProperty( IdString key, int32 defaultVal=0 ) const;

        void unsetProperty( IdString key );

        enum ExpressionType
        {
            EXPR_OPERATOR_OR,        //||
            EXPR_OPERATOR_AND,       //&&
            EXPR_OPERATOR_LE,        //<
            EXPR_OPERATOR_LEEQ,      //<=
            EXPR_OPERATOR_EQ,        //==
            EXPR_OPERATOR_NEQ,       //!=
            EXPR_OPERATOR_GR,        //>
            EXPR_OPERATOR_GREQ,      //>=
            EXPR_OBJECT,             //(...)
            EXPR_VAR
        };

        struct Expression
        {
            int32                   result;
            bool                    negated;
            ExpressionType          type;
            std::vector<Expression> children;
            String                  value;

            Expression() : result( false ), negated( false ), type( EXPR_VAR ) {}

            bool isOperator(void) const
                { return type >= EXPR_OPERATOR_OR && type <= EXPR_OPERATOR_GREQ; }
            inline void swap( Expression &other );
        };

        typedef std::vector<Expression> ExpressionVec;

        inline int interpretAsNumberThenAsProperty( const String &argValue ) const;

        static void copy( String &outBuffer, const SubStringRef &inSubString, size_t length );
        static void repeat( String &outBuffer, const SubStringRef &inSubString, size_t length,
                            size_t passNum, const String &counterVar );
        bool parseMath( const String &inBuffer, String &outBuffer );
        bool parseForEach( const String &inBuffer, String &outBuffer ) const;
        bool parseProperties( String &inBuffer, String &outBuffer ) const;
        bool parseUndefPieces( String &inBuffer, String &outBuffer );
        bool collectPieces( const String &inBuffer, String &outBuffer );
        bool insertPieces( String &inBuffer, String &outBuffer ) const;
        bool parseCounter( const String &inBuffer, String &outBuffer );
        bool parse( const String &inBuffer, String &outBuffer ) const;

        /** Goes through 'buffer', starting from startPos (inclusive) looking for the given
            character while skipping whitespace. If any character other than whitespace or
            EOLs if found returns String::npos
        @return
            String::npos if not found or wasn't the next character. If found, the position
            in the buffer, from start
        */
        static size_t findNextCharacter( const String &buffer, size_t startPos, char character );

        /// Returns true if we found an @end, false if we found
        /// \@else instead (can only happen if allowsElse=true).
        static bool findBlockEnd( SubStringRef &outSubString, bool &syntaxError, bool allowsElse=false );

        bool evaluateExpression( SubStringRef &outSubString, bool &outSyntaxError ) const;
        int32 evaluateExpressionRecursive( ExpressionVec &expression, bool &outSyntaxError ) const;
        static size_t evaluateExpressionEnd( const SubStringRef &outSubString );

        static void evaluateParamArgs( SubStringRef &outSubString, StringVector &outArgs,
                                       bool &outSyntaxError );

        static size_t calculateLineCount(const String &buffer, size_t idx );
        static size_t calculateLineCount( const SubStringRef &subString );

        /** Caches a set of properties (i.e. key-value pairs) & snippets of shaders. If an
            exact entry exists in the cache, its index is returned. Otherwise a new entry
            will be created.
        @param renderableSetProperties
            A vector containing key-value pairs of data
        @param pieces
            Shader snippets for each type of shader. Can be null. When not null, must hold
            NumShaderTypes entries, i.e. String val = pieces[NumShaderTypes-1][IdString]
        @return
            The index to the cache entry.
        */
        size_t addRenderableCache( const HlmsPropertyVec &renderableSetProperties,
                                   const PiecesMap *pieces );

        /// Retrieves a cache entry using the returned value from @addRenderableCache
        const RenderableCache& getRenderableCache( uint32 hash ) const;

        const HlmsCache* addShaderCache( uint32 hash, const HlmsPso &pso );
        const HlmsCache* getShaderCache( uint32 hash ) const;
        virtual void clearShaderCache(void);

        void processPieces( Archive *archive, const StringVector &pieceFiles );
        void hashPieceFiles( Archive *archive, const StringVector &pieceFiles,
                             FastArray<uint8> &fileContents ) const;

        void dumpProperties( std::ofstream &outFile );

        /** Modifies the PSO's macroblock if there are reasons to do that, and creates
            a strong reference to the macroblock that the PSO will own.
        @param pso [in/out]
            PSO to (potentially) modify.
        */
        void applyStrongMacroblockRules( HlmsPso &pso );

        HighLevelGpuProgramPtr compileShaderCode( const String &source,
                                                  const String &debugFilenameOutput,
                                                  uint32 finalHash, ShaderType shaderType );

    public:
        void _compileShaderFromPreprocessedSource( const RenderableCache &mergedCache,
                                                   const String source[NumShaderTypes] );

        /** Compiles input properties and adds it to the shader code cache
        @param codeCache [in/out]
            All variables must be filled except for ShaderCodeCache::shaders which is the output
        */
        void compileShaderCode( ShaderCodeCache &codeCache );

        const ShaderCodeCacheVec& getShaderCodeCache(void) const    { return mShaderCodeCache; }

    protected:
        /** Creates a shader based on input parameters. Caller is responsible for ensuring
            this shader hasn't already been created.
            Shader template files will be processed and then compiled.
        @param renderableHash
            The hash calculated in from Hlms::calculateHashFor that lives in Renderable
        @param passCache
            The return value of Hlms::preparePassHash
        @param finalHash
            A hash calculated on the pass' & renderable' hash. Must be unique. Caller is
            responsible for ensuring this hash stays unique.
        @param queuedRenderable
            The renderable who owns the renderableHash. Not used by the base class, but
            derived implementations may overload this function and take advantage of
            some of the direct access it provides.
        @return
            The newly created shader.
        */
        virtual const HlmsCache* createShaderCacheEntry( uint32 renderableHash,
                                                         const HlmsCache &passCache,
                                                         uint32 finalHash,
                                                         const QueuedRenderable &queuedRenderable );

        /// This function gets called right before starting parsing all templates, and after
        /// the renderable properties have been merged with the pass properties.
        virtual void notifyPropertiesMergedPreGenerationStep(void);

        virtual HlmsDatablock* createDatablockImpl( IdString datablockName,
                                                    const HlmsMacroblock *macroblock,
                                                    const HlmsBlendblock *blendblock,
                                                    const HlmsParamVec &paramVec );

        virtual HlmsDatablock* createDefaultDatablock(void);
        void _destroyAllDatablocks(void);

        inline void calculateHashForSemantic( VertexElementSemantic semantic, VertexElementType type,
                                              uint16 index, uint &inOutNumTexCoords );
        uint16 calculateHashForV1( Renderable *renderable );
        uint16 calculateHashForV2( Renderable *renderable );

        virtual void calculateHashForPreCreate( Renderable *renderable, PiecesMap *inOutPieces ) {}
        virtual void calculateHashForPreCaster( Renderable *renderable, PiecesMap *inOutPieces ) {}

        HlmsCache preparePassHashBase( const Ogre::CompositorShadowNode *shadowNode,
                                       bool casterPass, bool dualParaboloid,
                                       SceneManager *sceneManager );

        HlmsPassPso getPassPsoForScene( SceneManager *sceneManager );

    public:
        /**
        @param libraryFolders
            Path to folders to be processed first for collecting pieces. Will be processed in order.
            Pointer can be null.
        */
        Hlms(HlmsTypes type, const String &typeName, Archive *dataFolder,
              ArchiveVec *libraryFolders );
        virtual ~Hlms();

        HlmsTypes getType(void) const                       { return mType; }
        IdString getTypeName(void) const                    { return mTypeName; }
        const String& getTypeNameStr(void) const            { return mTypeNameStr; }
        void _notifyManager( HlmsManager *manager )         { mHlmsManager = manager; }
        HlmsManager* getHlmsManager(void) const             { return mHlmsManager; }
        const String& getShaderProfile(void) const          { return mShaderProfile; }

        void getTemplateChecksum( uint64 outHash[2] ) const;

        /** Sets the quality of the Hlms. This function is most relevant for mobile and
            almost or completely ignored by Desktop.
            The default value is false.
        @par
            On mobile, high quality will use "highp" quality precision qualifier for
            all its variables and functions.
            When not in HQ, mobile users may see aliasing artifacts, gradients; but
            the performance impact can be quite high. Some GPU drivers might even
            refuse to execute the shader as they cannot handle it.
        @par
            Unless you absolutely require high quality rendering on Mobile devices
            and/or to get it to look as closely as possible as it looks in a Desktop
            device, the recommended option is to have this off.
        */
        void setHighQuality( bool highQuality );
        bool getHighQuality(void) const                     { return mHighQuality; }

        /** Area lights use regular Forward.
        @param areaLightsLimit
            Maximum number of area lights that will be considered by the shader.
            Default value is 1.
            Use 0 to disable area lights.
        @param areaLightsRoundMultiple
            To prevent frequent shader recompiles, you can round the number of area lights
            to the next multiple.

            For example when areaLightsRoundMultiple = 1, if there are two area lights
            in the frustum, shader 'A' will be used. If the camera moves and now only
            one are light is in the frustum, shader 'B' will be used.

            This maximizes GPU performance, but if the number of area lights is constantly
            jumping, you may see a lot of recompiles until all variations are cached, which
            can be very slow.

            By setting for example, areaLightsRoundMultiple = 2, we will always generate
            shader variations that use 2 area lights, even if there's only 1 area light in
            the camera (if there's none, we use a different variation). The unused slot
            will just output black.
            If there's 3 area lights, the shader variation will be compiled to use 4.
            This sacrifices some pixel shader GPU performance, but prevents permutation
            explosion.

            By setting areaLightsLimit = areaLightsRoundMultiple, you will minimize the number
            of permutations and stabilize frame rates; but average framerate may be lower if
            there are less area lights.

            Default value is 1.
            This value cannot be 0.
            This value must be <= areaLightsLimit, unless areaLightsLimit is 0.
        */
        void setAreaLightForwardSettings( uint16 areaLightsLimit, uint8 areaLightsRoundMultiple );
        uint16 getAreaLightsLimit(void) const               { return mNumAreaLightsLimit; }
        uint8 getAreaLightsRoundMultiple(void) const        { return mAreaLightsRoundMultiple; }

#if !OGRE_NO_JSON
        /** Loads datablock values from a JSON value. @see HlmsJson.
        @param jsonValue
            JSON Object containing the definition of this datablock.
        @param blocks
            All the loaded Macro-, Blend- & Samplerblocks the JSON has
            defined and may be referenced by the datablock declaration.
        @param datablock
            Datablock to fill the values.
        */
        virtual void _loadJson( const rapidjson::Value &jsonValue, const HlmsJson::NamedBlocks &blocks,
                                HlmsDatablock *datablock, HlmsJsonListener *listener,
                                const String &additionalTextureExtension ) const {}
        virtual void _saveJson( const HlmsDatablock *datablock, String &outString,
                                HlmsJsonListener *listener,
                                const String &additionalTextureExtension ) const {}

        virtual void _collectSamplerblocks( set<const HlmsSamplerblock*>::type &outSamplerblocks,
                                            const HlmsDatablock *datablock ) const {}
#endif

        void saveAllTexturesFromDatablocks( const String &folderPath, set<String>::type &savedTextures,
                                            bool saveOitd, bool saveOriginal,
                                            HlmsTextureExportListener *listener );

        /** Destroys all the cached shaders and in the next opportunity will recreate them
            from the new location. This is very useful for fast iteration and real-time
            editing of Hlms shader templates.
        @remarks
            Calling with null pointer is possible and will only invalidate existing shaders
            but you should provide a valid pointer before we start generating the first
            shader (or else crash).
        @par
            Existing datablock materials won't be reloaded from files, so their properties
            won't change (i.e. changed from blue to red), but the shaders will.
        @param libraryFolders
            When null pointer, the library folders paths won't be changed at all
            (but still will be reloaded).
            When non-null pointer, the library folders will be overwriten.
            Pass an empty container if you want to stop using libraries.
        */
        virtual void reloadFrom( Archive *newDataFolder, ArchiveVec *libraryFolders=0 );

        Archive* getDataFolder(void)                        { return mDataFolder; }
        const LibraryVec& getPiecesLibrary(void) const      { return mLibrary; }
        ArchiveVec getPiecesLibraryAsArchiveVec(void) const;

        /** Creates a unique datablock that can be shared by multiple renderables.
        @remarks
            The name of the datablock must be in paramVec["name"] and must be unique
            Throws if a datablock with the same name paramVec["name"] already exists
        @param name
            Name of the Datablock, must be unique within all Hlms types, not just this one.
            99% you want this to be IdString( refName ); however this is not enforced.
        @param refName
            Name of the Datablock. The engine doesn't use this value at all. It is only
            useful for UI editors which want to enumerate all existing datablocks and
            display its name to the user.
        @param macroblockRef
            @See HlmsManager::getMacroblock
        @param blendblockRef
            @See HlmsManager::getBlendblock
        @param paramVec
            Key - String Value list of paramters. MUST BE SORTED.
        @param visibleToManager
            When false, HlmsManager::getDatablock won't find this datablock. True by default
        @param filename
            Filename in which it was defined, so that this information can be retrieved
            later by the user if needed. This is only for informational purposes.
        @param resourceGroup
            ResourceGroup. See filename param.
        @return
            Pointer to created Datablock
        */
        HlmsDatablock* createDatablock( IdString name, const String &refName,
                                        const HlmsMacroblock &macroblockRef,
                                        const HlmsBlendblock &blendblockRef,
                                        const HlmsParamVec &paramVec,
                                        bool visibleToManager=true,
                                        const String &filename=BLANKSTRING,
                                        const String &resourceGroup=BLANKSTRING );

        /** Finds an existing datablock based on its name (@see createDatablock)
        @return
            The datablock associated with that name. Null pointer if not found. Doesn't throw.
        */
        HlmsDatablock* getDatablock( IdString name ) const;

        /// Returns the string name associated with its hashed name (this was
        /// passed as refName in @createDatablock). Returns null ptr if
        /// not found.
        /// The reason this String doesn't live in HlmsDatablock is to prevent
        /// cache trashing (datablocks are hot iterated every frame, and the
        /// full name is rarely ever used)
        const String* getNameStr(IdString name) const;

        /// Returns the filaname & resource group a datablock was created from, and
        /// is associated with its hashed name (this was passed as in @createDatablock).
        /// Returns null ptr if not found. Note that it may also be a valid pointer but
        /// contain an empty string.
        /// The reason this String doesn't live in HlmsDatablock is to prevent
        /// cache trashing (datablocks are hot iterated every frame, and the
        /// filename & resource groups are rarely ever used)
        /// Usage:
        ///     String const *filename;
        ///     String const *resourceGroup;
        ///     datablock->getFilenameAndResourceGroup( &filename, &resourceGroup );
        ///     if( filename && resourceGroup && !filename->empty() && !resourceGroup->empty() )
        ///     {
        ///         //Valid filename & resource group.
        ///     }
        void getFilenameAndResourceGroup(IdString name, String const * *outFilename,
                                          String const * *outResourceGroup ) const;

        /** Destroys a datablocks given its name. Caller is responsible for ensuring
            those pointers aren't still in use (i.e. dangling pointers)
        @remarks
            Throws if no datablock with the given name exists.
        */
        void destroyDatablock( IdString name );

        /// Destroys all datablocks created with @createDatablock. Caller is responsible
        /// for ensuring those pointers aren't still in use (i.e. dangling pointers)
        /// The default datablock will be recreated.
        void destroyAllDatablocks(void);

        /// @copydoc HlmsManager::getDefaultDatablock
        HlmsDatablock* getDefaultDatablock(void) const;

        /// Returns all datablocks owned by this Hlms, including the default one.
        const HlmsDatablockMap& getDatablockMap(void) const { return mDatablocks; }

        /** Finds the parameter with key 'key' in the given 'paramVec'. If found, outputs
            the value to 'inOut', otherwise leaves 'inOut' as is.
        @return
            True if the key was found (inOut was modified), false otherwise
        @remarks
            Assumes paramVec is sorted by key.
        */
        static bool findParamInVec( const HlmsParamVec &paramVec, IdString key, String &inOut );

        /** Called by the renderable when either it changes the material,
            or its properties change (e.g. the mesh' uvs are stripped)
        @param renderable
            The renderable the material will be used on.
        @param movableObject
            The MovableObject the material will be used on (usually the parent of renderable)
        @return
            A hash. This hash references property parameters that are already cached.
        */
        virtual void calculateHashFor( Renderable *renderable, uint32 &outHash, uint32 &outCasterHash );

        /** Called every frame by the Render Queue to cache the properties needed by this
            pass. i.e. Number of PSSM splits, number of shadow casting lights, etc
        @param shadowNode
            The shadow node currently in effect. Can be null.
        @return
            A hash and cached property parameters. Unlike @calculateHashFor, the cache
            must be kept by the caller and not by us (because it may change every frame
            and is one for the whole pass, but Mesh' properties usually stay consistent
            through its lifetime but may differ per mesh)
        */
        virtual HlmsCache preparePassHash( const Ogre::CompositorShadowNode *shadowNode,
                                           bool casterPass, bool dualParaboloid,
                                           SceneManager *sceneManager );

        /** Retrieves an HlmsCache filled with the GPU programs to be used by the given
            renderable. If the shaders have already been created (i.e. whether for this
            renderable, or another one) it gets them from a cache. Otherwise we create it.
            It assumes that renderable->setHlms( this, parameters ) has already called.
        @param lastReturnedValue
            The last value returned by getMaterial.
        @param passCache
            The cache returned by @preparePassHash.
        @param renderable
            The renderable the caller wants us to give the shaders.
        @param movableObject
            The MovableObject owner of the renderable (we need it to know if renderable
            should cast shadows)
        @param casterPass
            True if this pass is the shadow mapping caster pass, false otherwise
        @return
            Structure containing all necessary shaders
        */
        const HlmsCache* getMaterial( HlmsCache const *lastReturnedValue, const HlmsCache &passCache,
                                      const QueuedRenderable &queuedRenderable, bool casterPass );

        /** Fills the constant buffers. Gets executed right before drawing the mesh.
        @param cache
            Current cache of Shaders to be used.
        @param queuedRenderable
            The Renderable-MovableObject pair about to be rendered.
        @param casterPass
            Whether this is a shadow mapping caster pass.
        @param lastCacheHash
            The hash of the cache of shaders that was the used by the previous renderable.
        @param lastTextureHash
            Last Texture Hash, used to let the Hlms know whether the textures should be changed again
        @return
            New Texture hash (may be equal or different to lastTextureHash).
        */
        virtual uint32 fillBuffersFor( const HlmsCache *cache, const QueuedRenderable &queuedRenderable,
                                       bool casterPass, uint32 lastCacheHash,
                                       uint32 lastTextureHash ) = 0;

        virtual uint32 fillBuffersForV1( const HlmsCache *cache,
                                         const QueuedRenderable &queuedRenderable,
                                         bool casterPass, uint32 lastCacheHash,
                                         CommandBuffer *commandBuffer ) = 0;

        virtual uint32 fillBuffersForV2( const HlmsCache *cache,
                                         const QueuedRenderable &queuedRenderable,
                                         bool casterPass, uint32 lastCacheHash,
                                         CommandBuffer *commandBuffer ) = 0;

        /// This gets called right before executing the command buffer.
        virtual void preCommandBufferExecution( CommandBuffer *commandBuffer ) {}
        /// This gets called after executing the command buffer.
        virtual void postCommandBufferExecution( CommandBuffer *commandBuffer ) {}

        /// Called when the frame has fully ended (ALL passes have been executed to all RTTs)
        virtual void frameEnded(void) {}

        /** Call to output the automatically generated shaders (which are usually made from templates)
            on the given folder for inspection, analyzing, debugging, etc.
        @remarks
            The shader will be dumped when it is generated, not when this function gets called.
            You should call this function at start up
        @param enableDebugOutput
            Whether to enable or disable dumping the shaders into a folder
        @param outputProperties
            Whether to dump properties and pieces at the beginning of the shader file.
            This is very useful for determining what caused Ogre to compile a new variation.
            Note that this setting may not always produce valid shader code in the dumped files
            (but it we'll still produce valid shader code while at runtime)
            If you want to compile the dumped file and it is invalid, just strip this info.
        @param path
            Path location on where to dump it. Should end with slash for proper concatenation
            (i.e. C:/path/ instead of C:/path; or /home/user/ instead of /home/user)
        */
        void setDebugOutputPath( bool enableDebugOutput, bool outputProperties,
                                 const String &path = BLANKSTRING );

        /** Sets a listener to extend an existing Hlms implementation's with custom code,
            without having to rewrite it or modify the source code directly.
        @remarks
            Other alternatives for extending an existing implementation is to derive
            from the class and override particular virtual functions.
            For performance reasons, listeners are never called on a per-object basis.
            Consult the section "Customizing an existing implementation" from the
            manual in the Docs/2.0 folder.
        @param listener
            Listener pointer. Use null to disable.
        */
        void setListener( HlmsListener *listener );

        /** Returns the current listener. @see setListener();
        @remarks
            If the default listener is being used (that does nothing) then null is returned.
        */
        HlmsListener* getListener(void) const;

        /// For debugging stuff. I.e. the Command line uses it for testing manually set properties
        void _setProperty( IdString key, int32 value )      { setProperty( key, value ); }
        int32 _getProperty( IdString key, int32 defaultVal=0 ) const
                                                { return getProperty( key, defaultVal ); }

        /// Utility helper, mostly useful to HlmsListener implementations.
        static void setProperty( HlmsPropertyVec &properties, IdString key, int32 value );
        /// Utility helper, mostly useful to HlmsListener implementations.
        static int32 getProperty( const HlmsPropertyVec &properties,
                                  IdString key, int32 defaultVal=0 );

        /// Internal use. @see HlmsManager::setShadowMappingUseBackFaces
        void _notifyShadowMappingBackFaceSetting(void);

        void _clearShaderCache(void);

        virtual void _changeRenderSystem( RenderSystem *newRs );

        RenderSystem* getRenderSystem(void) const           { return mRenderSystem; }
    };

    /// These are "default" or "Base" properties common to many implementations and thus defined here.
    /// Most of them start with the suffix hlms_
    struct _OgreExport HlmsBaseProp
    {
        static const IdString Skeleton;
        static const IdString BonesPerVertex;
        static const IdString Pose;

        static const IdString Normal;
        static const IdString QTangent;
        static const IdString Tangent;

        static const IdString Colour;

        static const IdString IdentityWorld;
        static const IdString IdentityViewProj;
        /// When this is set, the value of IdentityViewProj is meaningless.
        static const IdString IdentityViewProjDynamic;

        static const IdString UvCount;
        static const IdString UvCount0;
        static const IdString UvCount1;
        static const IdString UvCount2;
        static const IdString UvCount3;
        static const IdString UvCount4;
        static const IdString UvCount5;
        static const IdString UvCount6;
        static const IdString UvCount7;
        
        //Change per frame (grouped together with scene pass)
        static const IdString LightsDirectional;
        static const IdString LightsDirNonCaster;
        static const IdString LightsPoint;
        static const IdString LightsSpot;
        static const IdString LightsAreaApprox;
        static const IdString LightsAreaLtc;
        static const IdString LightsAreaTexMask;
        static const IdString LightsAttenuation;
        static const IdString LightsSpotParams;
        static const IdString LightsAreaTexColour;

        //Change per scene pass
        static const IdString PsoClipDistances;
        static const IdString GlobalClipPlanes;
        static const IdString DualParaboloidMapping;
        static const IdString NumShadowMapLights;
        static const IdString NumShadowMapTextures;
        static const IdString PssmSplits;
        static const IdString PssmBlend;
        static const IdString PssmFade;
        static const IdString ShadowCaster;
        static const IdString ShadowCasterDirectional;
        static const IdString ShadowCasterPoint;
        static const IdString ShadowUsesDepthTexture;
        static const IdString RenderDepthOnly;
        static const IdString FineLightMask;
        static const IdString PrePass;
        static const IdString UsePrePass;
        static const IdString UsePrePassMsaa;
        static const IdString UseSsr;
        static const IdString EnableVpls;
        static const IdString ForwardPlus;
        static const IdString ForwardPlusFlipY;
        static const IdString ForwardPlusDebug;
        static const IdString ForwardPlusFadeAttenRange;
        static const IdString ForwardPlusFineLightMask;
        static const IdString ForwardPlusCoversEntireTarget;
        static const IdString Forward3DNumSlices;
        static const IdString FwdClusteredWidthxHeight;
        static const IdString FwdClusteredWidth;
        static const IdString FwdClusteredLightsPerCell;
        static const IdString EnableDecals;
        static const IdString FwdPlusDecalsSlotOffset;
        static const IdString DecalsDiffuse;
        static const IdString DecalsNormals;
        static const IdString DecalsEmissive;

        static const IdString Forward3D;
        static const IdString ForwardClustered;
        static const IdString VPos;

        //Change per material (hash can be cached on the renderable)
        static const IdString AlphaTest;
        static const IdString AlphaTestShadowCasterOnly;
        static const IdString AlphaBlend;

        static const IdString Syntax;
        static const IdString Hlsl;
        static const IdString Glsl;
        static const IdString Glsles;
        static const IdString Metal;
        static const IdString GL3Plus;
        static const IdString iOS;
        static const IdString macOS;
        static const IdString GLVersion;
        static const IdString HighQuality;
        static const IdString FastShaderBuildHack;
        static const IdString TexGather;
        static const IdString DisableStage;

        //Useful GL Extensions
        static const IdString GlAmdTrinaryMinMax;

        static const IdString *UvCountPtrs[8];
    };

    struct _OgreExport HlmsPsoProp
    {
        static const IdString Macroblock;
        static const IdString Blendblock;
        static const IdString InputLayoutId;
    };

    struct _OgreExport HlmsBasePieces
    {
        static const IdString AlphaTestCmpFunc;
    };

    struct _OgreExport HlmsBits
    {
        static const int HlmsTypeBits;
        static const int RenderableBits;
        static const int PassBits;

        static const int HlmsTypeShift;
        static const int RenderableShift;
        static const int PassShift;
        static const int InputLayoutShift;

        static const int RendarebleHlmsTypeMask;
        static const int HlmsTypeMask;
        static const int RenderableMask;
        static const int PassMask;
        static const int InputLayoutMask;
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
