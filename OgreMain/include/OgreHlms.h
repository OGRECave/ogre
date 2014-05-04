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
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    class CompositorShadowNode;
    struct QueuedRenderable;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

    /** HLMS stands for "High Level Material System". */
    class _OgreExport Hlms : public PassAlloc
    {
    protected:
        enum ShaderType
        {
            VertexShader,
            PixelShader,
            GeometryShader,
            HullShader,
            DomainShader,
            NumShaderTypes
        };

        typedef std::map<IdString, String> PiecesMap;

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

        typedef vector<RenderableCache>::type RenderableCacheVec;

        RenderableCacheVec  mRenderableCache;
        HlmsCacheVec        mShaderCache;

        HlmsPropertyVec mSetProperties;
        PiecesMap       mPieces;

        Archive         *mDataFolder;
        StringVector    mPieceFiles[5];
        HlmsManager     *mHlmsManager;

        RenderSystem    *mRenderSystem;

        typedef std::map<IdString, HlmsDatablock*> HlmsDatablockMap;
        HlmsDatablockMap mDatablocks;

        String          mOutputPath;
        bool            mDebugOutput;

        HlmsTypes       mType;

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

        void setProperty( IdString key, int32 value );
        int32 getProperty( IdString key, int32 defaultVal=0 ) const;

        enum ExpressionType
        {
            EXPR_OPERATOR_OR,        //||
            EXPR_OPERATOR_AND,       //&&
            EXPR_OBJECT,             //(...)
            EXPR_VAR
        };

        struct Expression
        {
            bool                    result;
            bool                    negated;
            ExpressionType          type;
            std::vector<Expression> children;
            String                  value;

            Expression() : type( EXPR_VAR ), result( false ), negated( false ) {}
        };

        typedef std::vector<Expression> ExpressionVec;

        static void copy( String &outBuffer, const SubStringRef &inSubString, size_t length );
        static void repeat( String &outBuffer, const SubStringRef &inSubString, size_t length,
                            size_t passNum, const String &counterVar );
        bool parseForEach( const String &inBuffer, String &outBuffer ) const;
        bool parseProperties( String &inBuffer, String &outBuffer ) const;
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

        static void findBlockEnd( SubStringRef &outSubString , bool &syntaxError );

        bool evaluateExpression( SubStringRef &outSubString, bool &outSyntaxError ) const;
        bool evaluateExpressionRecursive( ExpressionVec &expression, bool &outSyntaxError ) const;
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

        const HlmsCache* addShaderCache( uint32 hash, GpuProgramPtr &vertexShader,
                                         GpuProgramPtr &geometryShader,
                                         GpuProgramPtr &tesselationHullShader,
                                         GpuProgramPtr &tesselationDomainShader,
                                         GpuProgramPtr &pixelShader );
        const HlmsCache* getShaderCache( uint32 hash ) const;

        /** Creates a shader based on input parameters. Caller is responsible for ensuring
            this shader hasn't already been created.
            Shader template files will be processed and then compiled.
        @param renderableHash
            The hash calculated in from @calculateHashFor that lives in @Renderable
        @param passCache
            The return value of @preparePassHash
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

        virtual HlmsDatablock* createDatablockImpl( const HlmsParamVec &paramVec,
                                                    const HlmsMacroblock *macroblock,
                                                    const HlmsBlendblock *blendblock,
                                                    IdString datablockName );

    public:
        Hlms( HlmsTypes type, Archive *dataFolder );
        virtual ~Hlms();

        HlmsTypes getType(void) const                       { return mType; }
        void _notifyManager( HlmsManager *manager )         { mHlmsManager = manager; }
        HlmsManager* getHlmsManager(void) const             { return mHlmsManager; }

        /** Creates a unique datablock that can be shared by multiple renderables.
        @remarks
            The name of the datablock must be in paramVec["name"] and must be unique
            Throws if a datablock with the same name paramVec["name"] already exists
        @param paramVec
            Key - String Value list of paramters. MUST BE SORTED.
        @param macroblockRef
            @See HlmsManager::getMacroblock
        @param blendblockRef
            @See HlmsManager::getBlendblock
        @return
            Pointer to created Datablock
        */
        HlmsDatablock* createDatablock( const HlmsParamVec &paramVec,
                                        const HlmsMacroblock &macroblockRef,
                                        const HlmsBlendblock &blendblockRef );

        /** Finds an existing datablock based on its name (@see createDatablock)
        @return
            The datablock associated with that name. Null pointer if not found. Doesn't throw.
        */
        HlmsDatablock* getDatablock( IdString name ) const;

        /** Destroys a datablocks given its name. Caller is responsible for ensuring
            those pointers aren't still in use (i.e. dangling pointers)
        @remarks
            Throws if no datablock with the given name exists.
        */
        void destroyDatablock( IdString name );

        /// Destroys all datablocks created with @createDatablock. Caller is responsible
        /// for ensuring those pointers aren't still in use (i.e. dangling pointers)
        void destroyAllDatablocks(void);

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
        virtual void calculateHashFor( Renderable *renderable, const HlmsParamVec &params,
                                       uint32 &outHash, uint32 &outCasterHash );

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

        virtual void fillBuffersFor( const HlmsCache *cache, const QueuedRenderable &queuedRenderable,
                                     bool casterPass, const HlmsCache *lastCache,
                                     uint32 lastTextureHash ) = 0;

        /// For debugging stuff. I.e. the Command line uses it for testing manually set properties
        void _setProperty( IdString key, int32 value )      { setProperty( key, value ); }

        void _changeRenderSystem( RenderSystem *newRs );

        RenderSystem* getRenderSystem(void) const           { return mRenderSystem; }

        static const IdString HlmsPropertySkeleton;
        static const IdString HlmsPropertyBonesPerVertex;
        static const IdString HlmsPropertyPose;

        static const IdString HlmsPropertyNormal;
        static const IdString HlmsPropertyQTangent;

        static const IdString HlmsPropertyUvCount;
        static const IdString HlmsPropertyUvCount0;
        static const IdString HlmsPropertyUvCount1;
        static const IdString HlmsPropertyUvCount2;
        static const IdString HlmsPropertyUvCount3;
        static const IdString HlmsPropertyUvCount4;
        static const IdString HlmsPropertyUvCount5;
        static const IdString HlmsPropertyUvCount6;
        static const IdString HlmsPropertyUvCount7;

        //Change per frame (grouped together with scene pass)
        static const IdString HlmsPropertyLightsDirectional;
        static const IdString HlmsPropertyLightsPoint;
        static const IdString HlmsPropertyLightsSpot;
        static const IdString HlmsPropertyLightsAttenuation;
        static const IdString HlmsPropertyLightsSpotParams;

        //Change per scene pass
        static const IdString HlmsPropertyDualParaboloidMapping;
        static const IdString HlmsPropertyNumShadowMaps;
        static const IdString HlmsPropertyPssmSplits;
        static const IdString HlmsPropertyShadowCaster;

        //Change per material (hash can be cached on the renderable)
        static const IdString PropertyDiffuseMap;
        static const IdString PropertyNormalMap;
        static const IdString PropertySpecularMap;
        static const IdString PropertyEnvProbeMap;
        static const IdString PropertyAlphaTest;

        static const IdString *UvCountPtrs[8];
    };
    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
