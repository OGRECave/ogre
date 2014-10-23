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

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */
    /** HLMS stands for "High Level Material System". */
    class _OgreExport Hlms : public PassAlloc
    {
        enum ShaderType
        {
            VertexShader,
            PixelShader,
            GeometryShader,
            HullShader,
            DomainShader,
            NumShaderTypes
        };

    protected:
        HlmsCacheVec    mRenderableCache;
        HlmsCacheVec    mShaderCache;

        typedef std::map<IdString, String> PiecesMap;
        HlmsPropertyVec mSetProperties;
        PiecesMap       mPieces;

        Archive         *mDataFolder;
        StringVector     mPieceFiles[5];
        //HlmsManager     *mHlmsManager;

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

            Expression() : result( false ), negated( false ), type( EXPR_VAR ) {}
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

        void addRenderableCache( uint32 hash, const HlmsPropertyVec &renderableSetProperties );
        const HlmsCache* getRenderableCache( uint32 hash ) const;
        const HlmsCache* addShaderCache( uint32 hash, GpuProgramPtr &vertexShader,
                                         GpuProgramPtr &geometryShader,
                                         GpuProgramPtr &tesselationHullShader,
                                         GpuProgramPtr &tesselationDomainShader,
                                         GpuProgramPtr &pixelShader );
        const HlmsCache* getShaderCache( uint32 hash ) const;
        const HlmsCache* createShaderCacheEntry( uint32 renderableHash, const HlmsCache &passCache,
                                                 uint32 finalHash );

        /** Finds the parameter with key 'key' in the given 'paramVec'. If found, outputs
            the value to 'inOut', otherwise leaves 'inOut' as is.
        @return
            True if the key was found (inOut was modified), false otherwise
        @remarks
            Assumes paramVec is sorted by key.
        */
        static bool findParamInVec( const HlmsParamVec &paramVec, IdString key, String &inOut );

        /// @See calculateHashFor
        virtual uint32 calculateRenderableHash(void) const;

    public:
        Hlms( Archive *dataFolder );
        virtual ~Hlms();

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
        const HlmsCache* getMaterial( const HlmsCache &passCache, Renderable *renderable,
                                      MovableObject *movableObject, bool casterPass );

        /// For debugging stuff. I.e. the Command line uses it for testing manually set properties
        void _setProperty( IdString key, int32 value )      { setProperty( key, value ); }
    };
    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
