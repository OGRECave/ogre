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

#include "OgreHlmsCommon.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    class HlmsManager;
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
    protected:
        typedef std::map<IdString, String> PiecesMap;
        HlmsPropertyVec mSetProperties;
        PiecesMap       mPieces;

        HlmsManager     *mHlmsManager;

        /** Inserts common properties about the current Renderable,
            such as hlms_skeleton hlms_uv_count, etc
        */
        void setCommonProperties();

        void setProperty(IdString key, int32 value );
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

        virtual uint32 calculateRenderableHash(void) const;


        virtual MaterialPtr prepareFor( Renderable *renderable, MovableObject *movableObject );

    public:
		Hlms();
        virtual ~Hlms();

        /** Called every frame by the Render Queue to cache the properties needed by this
            pass. i.e. Number of PSSM splits, number of shadow casting lights, etc
        @param shadowNode
            The shadow node currently in effect. Can be null.
        @return
            A hash and cached property parameters. Unlike @calculateHashFor, the cache
            must be kept by the caller and not by the HlmsManager (because it may
            change every frame and is one for the whole pass, but Mesh' properties
            usually stay consistent through its lifetime but may differ per mesh)
        */
        virtual HlmsCache preparePassHash( const Ogre::CompositorShadowNode *shadowNode );

        /** Called by the renderable when either it changes the material,
            or its properties change (e.g. the mesh' uvs are stripped)
        @param renderable
            The renderable the material will be used on.
        @param movableObject
            The MovableObject the material will be used on (usually the parent of renderable)
        @return
            A hash. This hash references property parameters cached in the HlmsManager.
        */
        virtual uint32 calculateHashFor( Renderable *renderable );

        MaterialPtr getMaterial( const HlmsCache &passCache, Renderable *renderable,
                                 MovableObject *movableObject );
        //void generateFor();
    };
    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
