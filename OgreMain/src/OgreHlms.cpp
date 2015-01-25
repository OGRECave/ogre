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

#include "OgreStableHeaders.h"

#include "OgreHlms.h"
#include "OgreHlmsManager.h"

#include "OgreHighLevelGpuProgramManager.h"
#include "OgreHighLevelGpuProgram.h"

#include "Vao/OgreVertexArrayObject.h"

#include "Compositor/OgreCompositorShadowNode.h"

#include "OgreLight.h"
#include "OgreSceneManager.h"
#include "OgreLogManager.h"
//#include "OgreMovableObject.h"
//#include "OgreRenderable.h"

namespace Ogre
{
    //Change per mesh (hash can be cached on the renderable)
    const IdString HlmsBaseProp::Skeleton           = IdString( "hlms_skeleton" );
    const IdString HlmsBaseProp::BonesPerVertex     = IdString( "hlms_bones_per_vertex" );
    const IdString HlmsBaseProp::Pose               = IdString( "hlms_pose" );

    const IdString HlmsBaseProp::Normal             = IdString( "hlms_normal" );
    const IdString HlmsBaseProp::QTangent           = IdString( "hlms_qtangent" );
    const IdString HlmsBaseProp::Tangent            = IdString( "hlms_tangent" );

    const IdString HlmsBaseProp::Colour             = IdString( "hlms_colour" );

    const IdString HlmsBaseProp::UvCount            = IdString( "hlms_uv_count" );
    const IdString HlmsBaseProp::UvCount0           = IdString( "hlms_uv_count0" );
    const IdString HlmsBaseProp::UvCount1           = IdString( "hlms_uv_count1" );
    const IdString HlmsBaseProp::UvCount2           = IdString( "hlms_uv_count2" );
    const IdString HlmsBaseProp::UvCount3           = IdString( "hlms_uv_count3" );
    const IdString HlmsBaseProp::UvCount4           = IdString( "hlms_uv_count4" );
    const IdString HlmsBaseProp::UvCount5           = IdString( "hlms_uv_count5" );
    const IdString HlmsBaseProp::UvCount6           = IdString( "hlms_uv_count6" );
    const IdString HlmsBaseProp::UvCount7           = IdString( "hlms_uv_count7" );

    //Change per frame (grouped together with scene pass)
    const IdString HlmsBaseProp::LightsDirectional  = IdString( "hlms_lights_directional" );
    const IdString HlmsBaseProp::LightsPoint        = IdString( "hlms_lights_point" );
    const IdString HlmsBaseProp::LightsSpot         = IdString( "hlms_lights_spot" );
    const IdString HlmsBaseProp::LightsAttenuation  = IdString( "hlms_lights_attenuation" );
    const IdString HlmsBaseProp::LightsSpotParams   = IdString( "hlms_lights_spotparams" );

    //Change per scene pass
    const IdString HlmsBaseProp::DualParaboloidMapping= IdString( "hlms_dual_paraboloid_mapping" );
    const IdString HlmsBaseProp::NumShadowMaps      = IdString( "hlms_num_shadow_maps" );
    const IdString HlmsBaseProp::PssmSplits         = IdString( "hlms_pssm_splits" );
    const IdString HlmsBaseProp::ShadowCaster       = IdString( "hlms_shadowcaster" );

    //Change per material (hash can be cached on the renderable)
    const IdString HlmsBaseProp::AlphaTest      = IdString( "alpha_test" );

    const IdString HlmsBaseProp::GL3Plus        = IdString( "GL3+" );
    const IdString HlmsBaseProp::HighQuality    = IdString( "hlms_high_quality" );

    const IdString HlmsBasePieces::AlphaTestCmpFunc = IdString( "alpha_test_cmp_func" );

    const IdString *HlmsBaseProp::UvCountPtrs[8] =
    {
        &HlmsBaseProp::UvCount0,
        &HlmsBaseProp::UvCount1,
        &HlmsBaseProp::UvCount2,
        &HlmsBaseProp::UvCount3,
        &HlmsBaseProp::UvCount4,
        &HlmsBaseProp::UvCount5,
        &HlmsBaseProp::UvCount6,
        &HlmsBaseProp::UvCount7
    };

    const String ShaderFiles[] = { "VertexShader_vs", "PixelShader_ps", "GeometryShader_gs",
                                   "HullShader_hs", "DomainShader_ds" };
    const String PieceFilePatterns[] = { "piece_vs", "piece_ps", "piece_gs", "piece_hs", "piece_ds" };

    Hlms::Hlms( HlmsTypes type, IdString typeName, Archive *dataFolder ) :
        mDataFolder( dataFolder ),
        mHlmsManager( 0 ),
        mLightGatheringMode( LightGatherForward ),
        mNumLightsLimit( 8 ),
        mRenderSystem( 0 ),
        mShaderProfile( "unset!" ),
        mDebugOutput( true ),
        mHighQuality( false ),
        mDefaultDatablock( 0 ),
        mType( type ),
        mTypeName( typeName )
    {
        enumeratePieceFiles();
    }
    //-----------------------------------------------------------------------------------
    Hlms::~Hlms()
    {
        clearShaderCache();

        _destroyAllDatablocks();

        if( mHlmsManager )
        {
            mHlmsManager->unregisterHlms( mType );
            mHlmsManager = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    void Hlms::setCommonProperties(void)
    {
        uint16 numWorldTransforms = 2;
        //bool castShadows          = true;

        setProperty( HlmsBaseProp::Skeleton, numWorldTransforms > 1 );
        setProperty( HlmsBaseProp::UvCount, 2 );
        setProperty( "true", 1 );
        setProperty( "false", 0 );

        setProperty( HlmsBaseProp::DualParaboloidMapping, 0 );

        setProperty( HlmsBaseProp::Normal, 1 );

        setProperty( HlmsBaseProp::UvCount0, 2 );
        setProperty( HlmsBaseProp::UvCount1, 4 );
        setProperty( HlmsBaseProp::BonesPerVertex, 4 );

        setProperty( HlmsBaseProp::NumShadowMaps, 3 );
        setProperty( HlmsBaseProp::PssmSplits, 3 );
        setProperty( HlmsBaseProp::ShadowCaster, 0 );

        setProperty( HlmsBaseProp::LightsDirectional, 1 );
        setProperty( HlmsBaseProp::LightsPoint, 2 );
        setProperty( HlmsBaseProp::LightsSpot, 3 );
    }
    //-----------------------------------------------------------------------------------
    void Hlms::enumeratePieceFiles(void)
    {
        if( !mDataFolder )
            return; //Some Hlms implementations may not use template files at all

        bool hasValidFile = false;

        //Check this folder can at least generate one valid type of shader.
        for( size_t i=0; i<NumShaderTypes; ++i )
        {
             //Generate the shader file. TODO: Identify the file extension at runtime
            const String filename = ShaderFiles[i] + ".glsl";
            hasValidFile |= mDataFolder->exists( filename );
        }

        if( !hasValidFile )
        {
            OGRE_EXCEPT( Exception::ERR_FILE_NOT_FOUND,
                         "Data folder provided contains no valid template shader files. "
                         "Did you provide the right folder location? Check you have the "
                         "right read pemissions. Folder: " + mDataFolder->getName(),
                         "Hlms::Hlms" );
        }

        StringVectorPtr stringVectorPtr = mDataFolder->list( false, false );

        StringVector stringVectorLowerCase( *stringVectorPtr );

        {
            StringVector::iterator itor = stringVectorLowerCase.begin();
            StringVector::iterator end  = stringVectorLowerCase.end();
            while( itor != end )
            {
                std::transform( itor->begin(), itor->end(), itor->begin(), ::tolower );
                ++itor;
            }
        }

        for( size_t i=0; i<NumShaderTypes; ++i )
        {
            StringVector::const_iterator itLowerCase = stringVectorLowerCase.begin();
            StringVector::const_iterator itor = stringVectorPtr->begin();
            StringVector::const_iterator end  = stringVectorPtr->end();

            while( itor != end )
            {
                if( itLowerCase->find( PieceFilePatterns[i] ) != String::npos )
                    mPieceFiles[i].push_back( *itor );

                ++itLowerCase;
                ++itor;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void Hlms::setProperty( IdString key, int32 value )
    {
        HlmsProperty p( key, value );
        HlmsPropertyVec::iterator it = std::lower_bound( mSetProperties.begin(), mSetProperties.end(),
                                                         p, OrderPropertyByIdString );
        if( it == mSetProperties.end() || it->keyName != p.keyName )
            mSetProperties.insert( it, p );
        else
            *it = p;
    }
    //-----------------------------------------------------------------------------------
    int32 Hlms::getProperty(IdString key, int32 defaultVal ) const
    {
        HlmsProperty p( key, 0 );
        HlmsPropertyVec::const_iterator it = std::lower_bound( mSetProperties.begin(),
                                                               mSetProperties.end(),
                                                               p, OrderPropertyByIdString );
        if( it != mSetProperties.end() && it->keyName == p.keyName )
            defaultVal = it->value;

        return defaultVal;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::findBlockEnd( SubStringRef &outSubString, bool &syntaxError )
    {
        const char *blockNames[] =
        {
            "foreach",
            "property",
            "piece"
        };

        String::const_iterator it = outSubString.begin();
        String::const_iterator en = outSubString.end();

        int nesting = 0;

        while( it != en && nesting >= 0 )
        {
            if( *it == '@' )
            {
                SubStringRef subString( &outSubString.getOriginalBuffer(), it + 1 );

                size_t idx = subString.find( "end" );
                if( idx == 0 )
                {
                    --nesting;
                    it += sizeof( "end" ) - 1;
                    continue;
                }
                else
                {
                    for( size_t i=0; i<sizeof( blockNames ) / sizeof( char* ); ++i )
                    {
                        size_t idxBlock = subString.find( blockNames[i] );
                        if( idxBlock == 0 )
                        {
                            it = subString.begin() + strlen( blockNames[i] );
                            ++nesting;
                            break;
                        }
                    }
                }
            }

            ++it;
        }

        assert( nesting >= -1 );

        if( it != en && nesting < 0 )
            outSubString.setEnd( it - outSubString.getOriginalBuffer().begin() - (sizeof( "end" ) - 1) );
        else
        {
            syntaxError = false;
            printf( "Syntax Error at line %lu: start block (e.g. @foreach; @property) "
                    "without matching @end\n", calculateLineCount( outSubString ) );
        }
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::evaluateExpression( SubStringRef &outSubString, bool &outSyntaxError ) const
    {
        size_t expEnd = evaluateExpressionEnd( outSubString );

        if( expEnd == String::npos )
        {
            outSyntaxError = true;
            return false;
        }

        SubStringRef subString( &outSubString.getOriginalBuffer(), outSubString.getStart(),
                                 outSubString.getStart() + expEnd );

        outSubString = SubStringRef( &outSubString.getOriginalBuffer(),
                                     outSubString.getStart() + expEnd + 1 );

        bool textStarted = false;
        bool syntaxError = false;
        bool nextExpressionNegates = false;

        std::vector<Expression*> expressionParents;
        ExpressionVec outExpressions;
        outExpressions.clear();
        outExpressions.resize( 1 );

        Expression *currentExpression = &outExpressions.back();

        String::const_iterator it = subString.begin();
        String::const_iterator en = subString.end();

        while( it != en && !syntaxError )
        {
            char c = *it;

            if( c == '(' )
            {
                currentExpression->children.push_back( Expression() );
                expressionParents.push_back( currentExpression );

                currentExpression->children.back().negated = nextExpressionNegates;

                textStarted = false;
                nextExpressionNegates = false;

                currentExpression = &currentExpression->children.back();
            }
            else if( c == ')' )
            {
                if( expressionParents.empty() )
                    syntaxError = true;
                else
                {
                    currentExpression = expressionParents.back();
                    expressionParents.pop_back();
                }

                textStarted = false;
            }
            else if( c == ' ' || c == '\t' || c == '\n' || c == '\r' )
            {
                textStarted = false;
            }
            else if( c == '!' )
            {
                nextExpressionNegates = true;
            }
            else
            {
                if( !textStarted )
                {
                    textStarted = true;
                    currentExpression->children.push_back( Expression() );
                    currentExpression->children.back().negated = nextExpressionNegates;
                }

                if( c == '&' || c == '|' )
                {
                    if( currentExpression->children.empty() || nextExpressionNegates )
                    {
                        syntaxError = true;
                    }
                    else if( !currentExpression->children.back().value.empty() &&
                             c != *(currentExpression->children.back().value.end()-1) )
                    {
                        currentExpression->children.push_back( Expression() );
                    }
                }

                currentExpression->children.back().value.push_back( c );
                nextExpressionNegates = false;
            }

            ++it;
        }

        bool retVal = false;

        if( !expressionParents.empty() )
            syntaxError = true;

        if( !syntaxError )
            retVal = evaluateExpressionRecursive( outExpressions, syntaxError );

        if( syntaxError )
            printf( "Syntax Error at line %lu\n", calculateLineCount( subString ) );

        outSyntaxError = syntaxError;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::evaluateExpressionRecursive( ExpressionVec &expression, bool &outSyntaxError ) const
    {
        ExpressionVec::iterator itor = expression.begin();
        ExpressionVec::iterator end  = expression.end();

        while( itor != end )
        {
            if( itor->value == "&&" )
                itor->type = EXPR_OPERATOR_AND;
            else if( itor->value == "||" )
                itor->type = EXPR_OPERATOR_OR;
            else if( !itor->children.empty() )
                itor->type = EXPR_OBJECT;
            else
                itor->type = EXPR_VAR;

            ++itor;
        }

        bool syntaxError = outSyntaxError;
        bool lastExpWasOperator = true;

        itor = expression.begin();

        while( itor != end && !syntaxError )
        {
            Expression &exp = *itor;
            if( ((exp.type == EXPR_OPERATOR_OR || exp.type == EXPR_OPERATOR_AND) && lastExpWasOperator) ||
                ((exp.type == EXPR_VAR || exp.type == EXPR_OBJECT) && !lastExpWasOperator ) )
            {
                syntaxError = true;
                printf( "Unrecognized token '%s'", exp.value.c_str() );
            }
            else if( exp.type == EXPR_OPERATOR_OR || exp.type == EXPR_OPERATOR_AND )
            {
                lastExpWasOperator = true;
            }
            else if( exp.type == EXPR_VAR )
            {
                exp.result = getProperty( exp.value ) != 0;
                lastExpWasOperator = false;
            }
            else
            {
                exp.result = evaluateExpressionRecursive( exp.children, syntaxError );
                lastExpWasOperator = false;
            }

            ++itor;
        }

        bool retVal = true;

        if( !syntaxError )
        {
            itor = expression.begin();
            bool andMode = true;

            while( itor != end )
            {
                if( itor->type == EXPR_OPERATOR_OR )
                    andMode = false;
                else if( itor->type == EXPR_OPERATOR_AND )
                    andMode = true;
                else
                {
                    if( andMode )
                        retVal &= itor->negated ? !itor->result : itor->result;
                    else
                        retVal |= itor->negated ? !itor->result : itor->result;
                }

                ++itor;
            }
        }

        outSyntaxError = syntaxError;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    size_t Hlms::evaluateExpressionEnd( const SubStringRef &outSubString )
    {
        String::const_iterator it = outSubString.begin();
        String::const_iterator en = outSubString.end();

        int nesting = 0;

        while( it != en && nesting >= 0 )
        {
            if( *it == '(' )
                ++nesting;
            else if( *it == ')' )
                --nesting;
            ++it;
        }

        assert( nesting >= -1 );

        size_t retVal = String::npos;
        if( it != en && nesting < 0 )
        {
            retVal = it - outSubString.begin() - 1;
        }
        else
        {
            printf( "Syntax Error at line %lu: opening parenthesis without matching closure\n",
                    calculateLineCount( outSubString ) );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::evaluateParamArgs( SubStringRef &outSubString, StringVector &outArgs,
                                  bool &outSyntaxError )
    {
        size_t expEnd = evaluateExpressionEnd( outSubString );

        if( expEnd == String::npos )
        {
            outSyntaxError = true;
            return;
        }

        SubStringRef subString( &outSubString.getOriginalBuffer(), outSubString.getStart(),
                                 outSubString.getStart() + expEnd );

        outSubString = SubStringRef( &outSubString.getOriginalBuffer(),
                                     outSubString.getStart() + expEnd + 1 );

        int expressionState = 0;
        bool syntaxError = false;

        outArgs.clear();
        outArgs.push_back( String() );

        String::const_iterator it = subString.begin();
        String::const_iterator en = subString.end();

        while( it != en && !syntaxError )
        {
            char c = *it;

            if( c == '(' || c == ')' || c == '@' || c == '&' || c == '|' )
            {
                syntaxError = true;
            }
            else if( c == ' ' || c == '\t' || c == '\n' || c == '\r' )
            {
                if( expressionState == 1 )
                    expressionState = 2;
            }
            else if( c == ',' )
            {
                expressionState = 0;
                outArgs.push_back( String() );
            }
            else
            {
                if( expressionState == 2 )
                {
                    printf( "Syntax Error at line %lu: ',' or ')' expected\n",
                            calculateLineCount( subString ) );
                    syntaxError = true;
                }
                else
                {
                    outArgs.back().push_back( *it );
                    expressionState = 1;
                }
            }

            ++it;
        }

        if( syntaxError )
            printf( "Syntax Error at line %lu\n", calculateLineCount( subString ) );

        outSyntaxError = syntaxError;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::copy( String &outBuffer, const SubStringRef &inSubString, size_t length )
    {
        String::const_iterator itor = inSubString.begin();
        String::const_iterator end  = inSubString.begin() + length;

        while( itor != end )
            outBuffer.push_back( *itor++ );
    }
    //-----------------------------------------------------------------------------------
    void Hlms::repeat( String &outBuffer, const SubStringRef &inSubString, size_t length,
                       size_t passNum, const String &counterVar )
    {
        String::const_iterator itor = inSubString.begin();
        String::const_iterator end  = inSubString.begin() + length;

        while( itor != end )
        {
            if( *itor == '@' && !counterVar.empty() )
            {
                SubStringRef subString( &inSubString.getOriginalBuffer(), itor + 1 );
                if( subString.find( counterVar ) == 0 )
                {
                    char tmp[16];
                    sprintf( tmp, "%lu", passNum );
                    outBuffer += tmp;
                    itor += counterVar.size() + 1;
                }
                else
                {
                    outBuffer.push_back( *itor++ );
                }
            }
            else
            {
               outBuffer.push_back( *itor++ );
            }
        }
    }
    //-----------------------------------------------------------------------------------
        int setOp( int op1, int op2 ) { return op2; }
        int addOp( int op1, int op2 ) { return op1 + op2; }
        int subOp( int op1, int op2 ) { return op1 - op2; }
        int mulOp( int op1, int op2 ) { return op1 * op2; }
        int divOp( int op1, int op2 ) { return op1 / op2; }
        int modOp( int op1, int op2 ) { return op1 % op2; }

        struct Operation
        {
            const char *opName;
            size_t length;
            int (*opFunc)(int, int);
            Operation( const char *_name, size_t len, int (*_opFunc)(int, int) ) :
                opName( _name ), length( len ), opFunc( _opFunc ) {}
        };

        const Operation c_operations[6] =
        {
            Operation( "pset", sizeof( "@pset" ), &setOp ),
            Operation( "padd", sizeof( "@padd" ), &addOp ),
            Operation( "psub", sizeof( "@psub" ), &subOp ),
            Operation( "pmul", sizeof( "@pmul" ), &mulOp ),
            Operation( "pdiv", sizeof( "@pdiv" ), &divOp ),
            Operation( "pmod", sizeof( "@pmod" ), &modOp )
        };
    //-----------------------------------------------------------------------------------
    bool Hlms::parseMath( const String &inBuffer, String &outBuffer )
    {
        outBuffer.clear();
        outBuffer.reserve( inBuffer.size() );

        StringVector argValues;
        SubStringRef subString( &inBuffer, 0 );

        size_t pos;
        pos = subString.find( "@" );
        size_t keyword = ~0;

        while( pos != String::npos && keyword == (size_t)~0 )
        {
            size_t maxSize = subString.findFirstOf( " \t(", pos + 1 );
            maxSize = maxSize == String::npos ? subString.getSize() : maxSize;
            SubStringRef keywordStr( &inBuffer, subString.getStart() + pos + 1,
                                                subString.getStart() + maxSize );

            for( size_t i=0; i<6 && keyword == (size_t)~0; ++i )
            {
                if( keywordStr.matchEqual( c_operations[i].opName ) )
                    keyword = i;
            }

            if( keyword == (size_t)~0 )
                pos = subString.find( "@", pos + 1 );
        }

        bool syntaxError = false;

        while( pos != String::npos && !syntaxError )
        {
            //Copy what comes before the block
            copy( outBuffer, subString, pos );

            subString.setStart( subString.getStart() + pos + c_operations[keyword].length );
            evaluateParamArgs( subString, argValues, syntaxError );

            syntaxError |= argValues.size() < 2 || argValues.size() > 3;

            if( !syntaxError )
            {
                IdString dstProperty;
                IdString srcProperty;
                int op1Value;
                int op2Value;

                dstProperty = argValues[0];
                size_t idx  = 1;
                srcProperty = dstProperty;
                if( argValues.size() == 3 )
                    srcProperty = argValues[idx++];
                op1Value    = getProperty( srcProperty );
                op2Value    = StringConverter::parseInt( argValues[idx],
                                                         -std::numeric_limits<int>::max() );

                if( op2Value == -std::numeric_limits<int>::max() )
                {
                    //Not a number, interpret as property
                    op2Value = getProperty( argValues[idx] );
                }

                int result = c_operations[keyword].opFunc( op1Value, op2Value );
                setProperty( dstProperty, result );
            }
            else
            {
                size_t lineCount = calculateLineCount( subString );
                if( keyword <= 1 )
                {
                    printf( "Syntax Error at line %lu: @%s expects one parameter",
                            lineCount, c_operations[keyword].opName );
                }
                else
                {
                    printf( "Syntax Error at line %lu: @%s expects two or three parameters",
                            lineCount, c_operations[keyword].opName );
                }
            }

            pos = subString.find( "@" );
            keyword = ~0;

            while( pos != String::npos && keyword == (size_t)~0 )
            {
                size_t maxSize = subString.findFirstOf( " \t(", pos + 1 );
                maxSize = maxSize == String::npos ? subString.getSize() : maxSize;
                SubStringRef keywordStr( &inBuffer, subString.getStart() + pos + 1,
                                                    subString.getStart() + maxSize );

                for( size_t i=0; i<6 && keyword == (size_t)~0; ++i )
                {
                    if( keywordStr.matchEqual( c_operations[i].opName ) )
                        keyword = i;
                }

                if( keyword == (size_t)~0 )
                    pos = subString.find( "@", pos + 1 );
            }
        }

        copy( outBuffer, subString, subString.getSize() );

        return syntaxError;
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::parseForEach( const String &inBuffer, String &outBuffer ) const
    {
        outBuffer.clear();
        outBuffer.reserve( inBuffer.size() );

        StringVector argValues;
        SubStringRef subString( &inBuffer, 0 );
        size_t pos = subString.find( "@foreach" );

        bool syntaxError = false;

        while( pos != String::npos && !syntaxError )
        {
            //Copy what comes before the block
            copy( outBuffer, subString, pos );

            subString.setStart( subString.getStart() + pos + sizeof( "@foreach" ) );
            evaluateParamArgs( subString, argValues, syntaxError );

            SubStringRef blockSubString = subString;
            findBlockEnd( blockSubString, syntaxError );

            if( !syntaxError )
            {
                char *endPtr;
                int count = strtol( argValues[0].c_str(), &endPtr, 10 );
                if( argValues[0].c_str() == endPtr )
                {
                    //This isn't a number. Let's try if it's a variable
                    //count = getProperty( argValues[0], -1 );
					count = getProperty( argValues[0], 0 );
                }

                /*if( count < 0 )
                {
                    printf( "Invalid parameter at line %lu (@foreach)."
                            " '%s' is not a number nor a variable\n",
                            calculateLineCount( blockSubString ), argValues[0].c_str() );
                    syntaxError = true;
                    count = 0;
                }*/

                String counterVar;
                if( argValues.size() > 1 )
                    counterVar = argValues[1];

                int start = 0;
                if( argValues.size() > 2 )
                {
                    start = strtol( argValues[2].c_str(), &endPtr, 10 );
                    if( argValues[2].c_str() == endPtr )
                    {
                        //This isn't a number. Let's try if it's a variable
                        start = getProperty( argValues[2], -1 );
                    }

                    if( start < 0 )
                    {
                        printf( "Invalid parameter at line %lu (@foreach)."
                                " '%s' is not a number nor a variable\n",
                                calculateLineCount( blockSubString ), argValues[2].c_str() );
                        syntaxError = true;
                        start = 0;
                        count = 0;
                    }
                }

                for( int i=start; i<count; ++i )
                    repeat( outBuffer, blockSubString, blockSubString.getSize(), i, counterVar );

            }

            subString.setStart( blockSubString.getEnd() + sizeof( "@end" ) );
            pos = subString.find( "@foreach" );
        }

        copy( outBuffer, subString, subString.getSize() );

        return syntaxError;
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::parseProperties( String &inBuffer, String &outBuffer ) const
    {
        outBuffer.clear();
        outBuffer.reserve( inBuffer.size() );

        SubStringRef subString( &inBuffer, 0 );
        size_t pos = subString.find( "@property" );

        bool syntaxError = false;

        while( pos != String::npos && !syntaxError )
        {
            //Copy what comes before the block
            copy( outBuffer, subString, pos );

            subString.setStart( subString.getStart() + pos + sizeof( "@property" ) );
            bool result = evaluateExpression( subString, syntaxError );

            SubStringRef blockSubString = subString;
            findBlockEnd( blockSubString, syntaxError );

            if( result && !syntaxError )
                copy( outBuffer, blockSubString, blockSubString.getSize() );

            subString.setStart( blockSubString.getEnd() + sizeof( "@end" ) );
            pos = subString.find( "@property" );
        }

        copy( outBuffer, subString, subString.getSize() );

        while( !syntaxError && outBuffer.find( "@property" ) != String::npos )
        {
            inBuffer.swap( outBuffer );
            syntaxError = parseProperties( inBuffer, outBuffer );
        }

        return syntaxError;
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::collectPieces( const String &inBuffer, String &outBuffer )
    {
        outBuffer.clear();
        outBuffer.reserve( inBuffer.size() );

        StringVector argValues;
        SubStringRef subString( &inBuffer, 0 );
        size_t pos = subString.find( "@piece" );

        bool syntaxError = false;

        while( pos != String::npos && !syntaxError )
        {
            //Copy what comes before the block
            copy( outBuffer, subString, pos );

            subString.setStart( subString.getStart() + pos + sizeof( "@piece" ) );
            evaluateParamArgs( subString, argValues, syntaxError );

            syntaxError |= argValues.size() != 1;

            if( !syntaxError )
            {
                const IdString pieceName( argValues[0] );
                PiecesMap::const_iterator it = mPieces.find( pieceName );
                if( it != mPieces.end() )
                {
                    syntaxError = true;
                    printf( "Error at line %lu: @piece '%s' already defined",
                            calculateLineCount( subString ), argValues[0].c_str() );
                }
                else
                {
                    SubStringRef blockSubString = subString;
                    findBlockEnd( blockSubString, syntaxError );

                    String tmpBuffer;
                    copy( tmpBuffer, blockSubString, blockSubString.getSize() );
                    mPieces[pieceName] = tmpBuffer;

                    subString.setStart( blockSubString.getEnd() + sizeof( "@end" ) );
                }
            }
            else
            {
                printf( "Syntax Error at line %lu: @piece expects one parameter",
                        calculateLineCount( subString ) );
            }

            pos = subString.find( "@piece" );
        }

        copy( outBuffer, subString, subString.getSize() );

        return syntaxError;
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::insertPieces( String &inBuffer, String &outBuffer ) const
    {
        outBuffer.clear();
        outBuffer.reserve( inBuffer.size() );

        StringVector argValues;
        SubStringRef subString( &inBuffer, 0 );
        size_t pos = subString.find( "@insertpiece" );

        bool syntaxError = false;

        while( pos != String::npos && !syntaxError )
        {
            //Copy what comes before the block
            copy( outBuffer, subString, pos );

            subString.setStart( subString.getStart() + pos + sizeof( "@insertpiece" ) );
            evaluateParamArgs( subString, argValues, syntaxError );

            syntaxError |= argValues.size() != 1;

            if( !syntaxError )
            {
                const IdString pieceName( argValues[0] );
                PiecesMap::const_iterator it = mPieces.find( pieceName );
                if( it != mPieces.end() )
                    outBuffer += it->second;
            }
            else
            {
                printf( "Syntax Error at line %lu: @insertpiece expects one parameter",
                        calculateLineCount( subString ) );
            }

            pos = subString.find( "@insertpiece" );
        }

        copy( outBuffer, subString, subString.getSize() );

        return syntaxError;
    }
    //-----------------------------------------------------------------------------------
        const Operation c_counterOperations[8] =
        {
            Operation( "counter", sizeof( "@counter" ), 0 ),
            Operation( "value", sizeof( "@value" ), 0 ),
            Operation( "set", sizeof( "@set" ), &setOp ),
            Operation( "add", sizeof( "@add" ), &addOp ),
            Operation( "sub", sizeof( "@sub" ), &subOp ),
            Operation( "mul", sizeof( "@mul" ), &mulOp ),
            Operation( "div", sizeof( "@div" ), &divOp ),
            Operation( "mod", sizeof( "@mod" ), &modOp )
        };
    //-----------------------------------------------------------------------------------
    bool Hlms::parseCounter( const String &inBuffer, String &outBuffer )
    {
        outBuffer.clear();
        outBuffer.reserve( inBuffer.size() );

        StringVector argValues;
        SubStringRef subString( &inBuffer, 0 );

        size_t pos;
        pos = subString.find( "@" );
        size_t keyword = ~0;

        if( pos != String::npos )
        {
            size_t maxSize = subString.findFirstOf( " \t(", pos + 1 );
            maxSize = maxSize == String::npos ? subString.getSize() : maxSize;
            SubStringRef keywordStr( &inBuffer, subString.getStart() + pos + 1,
                                                subString.getStart() + maxSize );

            for( size_t i=0; i<8 && keyword == (size_t)~0; ++i )
            {
                if( keywordStr.matchEqual( c_counterOperations[i].opName ) )
                    keyword = i;
            }

            if( keyword == (size_t)~0 )
                pos = String::npos;
        }

        bool syntaxError = false;

        while( pos != String::npos && !syntaxError )
        {
            //Copy what comes before the block
            copy( outBuffer, subString, pos );

            subString.setStart( subString.getStart() + pos + c_counterOperations[keyword].length );
            evaluateParamArgs( subString, argValues, syntaxError );

            if( keyword <= 1 )
                syntaxError |= argValues.size() != 1;
            else
                syntaxError |= argValues.size() < 2 || argValues.size() > 3;

            if( !syntaxError )
            {
                IdString dstProperty;
                IdString srcProperty;
                int op1Value;
                int op2Value;

                if( argValues.size() == 1 )
                {
                    dstProperty = argValues[0];
                    srcProperty = dstProperty;
                    op1Value    = getProperty( srcProperty );
                    op2Value    = op1Value;

                    //@value & @counter write, the others are invisible
                    char tmp[16];
                    sprintf( tmp, "%i", op1Value );
                    outBuffer += tmp;

                    if( keyword == 0 )
                    {
                        ++op1Value;
                        setProperty( dstProperty, op1Value );
                    }
                }
                else
                {
                    dstProperty = argValues[0];
                    size_t idx  = 1;
                    srcProperty = dstProperty;
                    if( argValues.size() == 3 )
                        srcProperty = argValues[idx++];
                    op1Value    = getProperty( srcProperty );
                    op2Value    = StringConverter::parseInt( argValues[idx],
                                                             -std::numeric_limits<int>::max() );

                    if( op2Value == -std::numeric_limits<int>::max() )
                    {
                        //Not a number, interpret as property
                        op2Value = getProperty( argValues[idx] );
                    }

                    int result = c_counterOperations[keyword].opFunc( op1Value, op2Value );
                    setProperty( dstProperty, result );
                }
            }
            else
            {
                size_t lineCount = calculateLineCount( subString );
                if( keyword <= 1 )
                {
                    printf( "Syntax Error at line %lu: @%s expects one parameter",
                            lineCount, c_counterOperations[keyword].opName );
                }
                else
                {
                    printf( "Syntax Error at line %lu: @%s expects two or three parameters",
                            lineCount, c_counterOperations[keyword].opName );
                }
            }

            pos = subString.find( "@" );
            keyword = ~0;

            if( pos != String::npos )
            {
                size_t maxSize = subString.findFirstOf( " \t(", pos + 1 );
                maxSize = maxSize == String::npos ? subString.getSize() : maxSize;
                SubStringRef keywordStr( &inBuffer, subString.getStart() + pos + 1,
                                                    subString.getStart() + maxSize );

                for( size_t i=0; i<8 && keyword == (size_t)~0; ++i )
                {
                    if( keywordStr.matchEqual( c_counterOperations[i].opName ) )
                        keyword = i;
                }

                if( keyword == (size_t)~0 )
                    pos = String::npos;
            }
        }

        copy( outBuffer, subString, subString.getSize() );

        return syntaxError;
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::parse( const String &inBuffer, String &outBuffer ) const
    {
        outBuffer.clear();
        outBuffer.reserve( inBuffer.size() );

        return parseForEach( inBuffer, outBuffer );
        //return parseProperties( inBuffer, outBuffer );
    }
    //-----------------------------------------------------------------------------------
    size_t Hlms::addRenderableCache( const HlmsPropertyVec &renderableSetProperties,
                                     const PiecesMap *pieces )
    {
        assert( mRenderableCache.size() < 128 );

        RenderableCache cacheEntry( renderableSetProperties, pieces );

        RenderableCacheVec::iterator it = std::find( mRenderableCache.begin(), mRenderableCache.end(),
                                                     cacheEntry );
        if( it == mRenderableCache.end() )
        {
            mRenderableCache.push_back( cacheEntry );
            it = mRenderableCache.end() - 1;
        }

        return (mType << 7) | (it - mRenderableCache.begin());
    }
    //-----------------------------------------------------------------------------------
    const Hlms::RenderableCache &Hlms::getRenderableCache( uint32 hash ) const
    {
        return mRenderableCache[hash & 0x7f];
    }
    //-----------------------------------------------------------------------------------
    HlmsDatablock* Hlms::createDatablockImpl( IdString datablockName,
                                              const HlmsMacroblock *macroblock,
                                              const HlmsBlendblock *blendblock,
                                              const HlmsParamVec &paramVec )
    {
        return OGRE_NEW HlmsDatablock( datablockName, this, macroblock, blendblock, paramVec );
    }
    //-----------------------------------------------------------------------------------
    HlmsDatablock* Hlms::createDefaultDatablock(void)
    {
        return createDatablock( IdString(), "[Default]",
                                HlmsMacroblock(), HlmsBlendblock(), HlmsParamVec(), false );
    }
    //-----------------------------------------------------------------------------------
    void Hlms::setHighQuality( bool highQuality )
    {
        clearShaderCache();
        mHighQuality = highQuality;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::reloadFrom( Archive *newDataFolder )
    {
        clearShaderCache();
        mDataFolder = newDataFolder;
        enumeratePieceFiles();
    }
    //-----------------------------------------------------------------------------------
    HlmsDatablock* Hlms::createDatablock( IdString name, const String &refName,
                                          const HlmsMacroblock &macroblockRef,
                                          const HlmsBlendblock &blendblockRef,
                                          const HlmsParamVec &paramVec, bool visibleToManager )
    {
        if( mDatablocks.find( name ) != mDatablocks.end() )
        {
            OGRE_EXCEPT( Exception::ERR_DUPLICATE_ITEM, "A material datablock with name '" +
                         name.getFriendlyText() + "' already exists.", "Hlms::createDatablock" );
        }

        const HlmsMacroblock *macroblock = mHlmsManager->getMacroblock( macroblockRef );
        const HlmsBlendblock *blendblock = mHlmsManager->getBlendblock( blendblockRef );

        HlmsDatablock *retVal = createDatablockImpl( name, macroblock, blendblock, paramVec );

        mDatablocks[name] = DatablockEntry( retVal, visibleToManager, refName );

        retVal->calculateHash();

        if( visibleToManager )
            mHlmsManager->_datablockAdded( retVal );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    HlmsDatablock* Hlms::getDatablock( IdString name ) const
    {
        HlmsDatablock *retVal = 0;
        HlmsDatablockMap::const_iterator itor = mDatablocks.find( name );
        if( itor != mDatablocks.end() )
            retVal = itor->second.datablock;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    const String* Hlms::getFullNameString( IdString name ) const
    {
        String const *retVal = 0;
        HlmsDatablockMap::const_iterator itor = mDatablocks.find( name );
        if( itor != mDatablocks.end() )
            retVal = &itor->second.name;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::destroyDatablock( IdString name )
    {
        HlmsDatablockMap::iterator itor = mDatablocks.find( name );
        if( itor == mDatablocks.end() )
        {
            OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND,
                         "Can't find datablock with name '" + name.getFriendlyText() + "'",
                         "Hlms::destroyDatablock" );
        }

        if( itor->second.visibleToManager )
            mHlmsManager->_datablockDestroyed( name );

        OGRE_DELETE itor->second.datablock;
        mDatablocks.erase( itor );
    }
    //-----------------------------------------------------------------------------------
    void Hlms::_destroyAllDatablocks(void)
    {
        HlmsDatablockMap::const_iterator itor = mDatablocks.begin();
        HlmsDatablockMap::const_iterator end  = mDatablocks.end();

        while( itor != end )
        {
            OGRE_DELETE itor->second.datablock;
            ++itor;
        }

        mDatablocks.clear();
    }
    //-----------------------------------------------------------------------------------
    void Hlms::destroyAllDatablocks(void)
    {
        _destroyAllDatablocks();
        mDefaultDatablock = createDefaultDatablock();
    }
    //-----------------------------------------------------------------------------------
    HlmsDatablock* Hlms::getDefaultDatablock(void) const
    {
        return mDefaultDatablock;
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::findParamInVec( const HlmsParamVec &paramVec, IdString key, String &inOut )
    {
        bool retVal = false;
        HlmsParamVec::const_iterator it = std::lower_bound( paramVec.begin(), paramVec.end(),
                                                            std::pair<IdString, String>( key, String() ),
                                                            OrderParamVecByKey );
        if( it != paramVec.end() && it->first == key )
        {
            inOut = it->second;
            retVal = true;
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    const HlmsCache* Hlms::addShaderCache( uint32 hash, GpuProgramPtr &vertexShader,
                                           GpuProgramPtr &geometryShader,
                                           GpuProgramPtr &tesselationHullShader,
                                           GpuProgramPtr &tesselationDomainShader,
                                           GpuProgramPtr &pixelShader )
    {
        HlmsCache cache( hash, mType );
        HlmsCacheVec::iterator it = std::lower_bound( mShaderCache.begin(), mShaderCache.end(),
                                                      &cache, OrderCacheByHash );

        assert( (it == mShaderCache.end() || (*it)->hash != hash) &&
                "Can't add the same shader to the cache twice! (or a hash collision happened)" );

        cache.vertexShader              = vertexShader;
        cache.geometryShader            = geometryShader;
        cache.tesselationHullShader     = tesselationHullShader;
        cache.tesselationDomainShader   = tesselationDomainShader;
        cache.pixelShader               = pixelShader;

        HlmsCache *retVal = new HlmsCache( cache );
        mShaderCache.insert( it, retVal );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    const HlmsCache* Hlms::getShaderCache( uint32 hash ) const
    {
        HlmsCache cache( hash, mType );
        HlmsCacheVec::const_iterator it = std::lower_bound( mShaderCache.begin(), mShaderCache.end(),
                                                            &cache, OrderCacheByHash );

        if( it != mShaderCache.end() && (*it)->hash == hash )
            return *it;

        return 0;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::clearShaderCache(void)
    {
        HlmsCacheVec::const_iterator itor = mShaderCache.begin();
        HlmsCacheVec::const_iterator end  = mShaderCache.end();

        while( itor != end )
        {
            delete *itor;
            ++itor;
        }

        mShaderCache.clear();
    }
    //-----------------------------------------------------------------------------------
    const HlmsCache* Hlms::createShaderCacheEntry( uint32 renderableHash, const HlmsCache &passCache,
                                                   uint32 finalHash,
                                                   const QueuedRenderable &queuedRenderable )
    {
        //Set the properties by merging the cache from the pass, with the cache from renderable
        mSetProperties.clear();
        //If retVal is null, we did something wrong earlier
        //(the cache should've been generated by now)
        const RenderableCache &renderableCache = getRenderableCache( renderableHash );
        mSetProperties.reserve( passCache.setProperties.size() + renderableCache.setProperties.size() );
        //Copy the properties from the renderable
        mSetProperties.insert( mSetProperties.end(), renderableCache.setProperties.begin(),
                                                     renderableCache.setProperties.end() );
        {
            //Now copy the properties from the pass (one by one, since be must maintain the order)
            HlmsPropertyVec::const_iterator itor = passCache.setProperties.begin();
            HlmsPropertyVec::const_iterator end  = passCache.setProperties.end();

            while( itor != end )
            {
                setProperty( itor->keyName, itor->value );
                ++itor;
            }
        }

        GpuProgramPtr shaders[NumShaderTypes];
        //Generate the shaders
        for( size_t i=0; i<NumShaderTypes; ++i )
        {
            //Collect pieces
            mPieces = renderableCache.pieces[i];

            if( mShaderProfile == "glsl" ) //TODO: String comparision
                setProperty( HlmsBaseProp::GL3Plus, 330 );

            setProperty( HlmsBaseProp::HighQuality, mHighQuality );

            StringVector::const_iterator itor = mPieceFiles[i].begin();
            StringVector::const_iterator end  = mPieceFiles[i].end();

            while( itor != end )
            {
                DataStreamPtr inFile = mDataFolder->open( *itor );

                String inString;
                String outString;

                inString.resize( inFile->size() );
                inFile->read( &inString[0], inFile->size() );

                this->parseMath( inString, outString );
                this->parseForEach( outString, inString );
                this->parseProperties( inString, outString );
                this->collectPieces( outString, inString );
                this->parseCounter( inString, outString );
                ++itor;
            }

            //Generate the shader file. TODO: Identify the file extension at runtime
            const String filename = ShaderFiles[i] + ".glsl";
            if( mDataFolder->exists( filename ) )
            {
                DataStreamPtr inFile = mDataFolder->open( filename );

                String inString;
                String outString;

                inString.resize( inFile->size() );
                inFile->read( &inString[0], inFile->size() );

                bool syntaxError = false;

                syntaxError |= this->parseMath( inString, outString );
                syntaxError |= this->parseForEach( outString, inString );
                syntaxError |= this->parseProperties( inString, outString );
                while( !syntaxError  && (outString.find( "@piece" ) != String::npos ||
                                         outString.find( "@insertpiece" ) != String::npos) )
                {
                    syntaxError |= this->collectPieces( outString, inString );
                    syntaxError |= this->insertPieces( inString, outString );
                }
                syntaxError |= this->parseCounter( outString, inString );

                outString.swap( inString );

                if( syntaxError )
                {
                    LogManager::getSingleton().logMessage( "There were HLMS syntax errors while parsing "
                                                           + StringConverter::toString( finalHash ) +
                                                           ShaderFiles[i] );
                }

                if( mDebugOutput )
                {
                    std::ofstream outFile( (mOutputPath + "./" +
                                           StringConverter::toString( finalHash ) +
                                           ShaderFiles[i]).c_str(),
                                           std::ios::out | std::ios::binary );
                    outFile.write( &outString[0], outString.size() );
                }

                HighLevelGpuProgramManager *gpuProgramManager =
                                                        HighLevelGpuProgramManager::getSingletonPtr();

                HighLevelGpuProgramPtr gp = gpuProgramManager->createProgram(
                                    StringConverter::toString( finalHash ) + ShaderFiles[i],
                                    ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME,
                                    mShaderProfile, static_cast<GpuProgramType>(i) );
                gp->setSource( outString );

                gp->setSkeletalAnimationIncluded( getProperty( HlmsBaseProp::Skeleton ) != 0 );
                gp->setMorphAnimationIncluded( false );
                gp->setPoseAnimationIncluded( getProperty( HlmsBaseProp::Pose ) );
                gp->setVertexTextureFetchRequired( false );

                gp->load();

                shaders[i] = gp;
            }
        }

        const HlmsCache* retVal = addShaderCache( finalHash, shaders[VertexShader],
                                                  shaders[GeometryShader], shaders[HullShader],
                                                  shaders[DomainShader], shaders[PixelShader] );
        return retVal;
    }
    //-----------------------------------------------------------------------------------
    uint16 Hlms::calculateHashForV1( Renderable *renderable )
    {
        v1::RenderOperation op;
        renderable->getRenderOperation( op );
        v1::VertexDeclaration *vertexDecl = op.vertexData->vertexDeclaration;
        const v1::VertexDeclaration::VertexElementList &elementList = vertexDecl->getElements();
        v1::VertexDeclaration::VertexElementList::const_iterator itor = elementList.begin();
        v1::VertexDeclaration::VertexElementList::const_iterator end  = elementList.end();

        uint numTexCoords = 0;
        while( itor != end )
        {
            const v1::VertexElement &vertexElem = *itor;
            calculateHashForSemantic( vertexElem.getSemantic(), vertexElem.getType(),
                                      vertexElem.getIndex(), numTexCoords );
            ++itor;
        }

        return numTexCoords;
    }
    //-----------------------------------------------------------------------------------
    uint16 Hlms::calculateHashForV2( Renderable *renderable )
    {
        //TODO: Account LOD
        VertexArrayObject *vao = renderable->getVaos()[0];
        const VertexBufferPackedVec &vertexBuffers = vao->getVertexBuffers();

        uint numTexCoords = 0;
        uint16 semIndex[VES_COUNT];
        memset( semIndex, 0, sizeof( semIndex ) );
        VertexBufferPackedVec::const_iterator itor = vertexBuffers.begin();
        VertexBufferPackedVec::const_iterator end  = vertexBuffers.end();

        while( itor != end )
        {
            const VertexElement2Vec &vertexElements = (*itor)->getVertexElements();
            VertexElement2Vec::const_iterator itElements = vertexElements.begin();
            VertexElement2Vec::const_iterator enElements = vertexElements.end();

            while( itElements != enElements )
            {
                calculateHashForSemantic( itElements->mSemantic, itElements->mType,
                                          semIndex[itElements->mSemantic-1]++, numTexCoords );
                ++itElements;
            }

            ++itor;
        }

        return numTexCoords;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::calculateHashForSemantic( VertexElementSemantic semantic, VertexElementType type,
                                         uint16 index, uint &inOutNumTexCoords )
    {
        switch( semantic )
        {
        case VES_NORMAL:
            if( v1::VertexElement::getTypeCount( type ) < 4 )
            {
                setProperty( HlmsBaseProp::Normal, 1 );
            }
            else
            {
                setProperty( HlmsBaseProp::QTangent, 1 );
            }
            break;
        case VES_TANGENT:
            setProperty( HlmsBaseProp::Tangent, 1 );
            break;
        case VES_DIFFUSE:
            setProperty( HlmsBaseProp::Colour, 1 );
            break;
        case VES_TEXTURE_COORDINATES:
            inOutNumTexCoords = std::max<uint>( inOutNumTexCoords, index + 1 );
            setProperty( *HlmsBaseProp::UvCountPtrs[index],
                          v1::VertexElement::getTypeCount( type ) );
            break;
        case VES_BLEND_WEIGHTS:
            setProperty( HlmsBaseProp::BonesPerVertex,
                         v1::VertexElement::getTypeCount( type ) );
            break;
        default:
            break;
        }
    }
    //-----------------------------------------------------------------------------------
    void Hlms::calculateHashFor( Renderable *renderable, uint32 &outHash, uint32 &outCasterHash )
    {
        mSetProperties.clear();

        setProperty( HlmsBaseProp::Skeleton, renderable->hasSkeletonAnimation() );

        uint16 numTexCoords = 0;
        if( renderable->getVaos().empty() )
            numTexCoords = calculateHashForV1( renderable );
        else
            numTexCoords = calculateHashForV2( renderable );

        setProperty( HlmsBaseProp::UvCount, numTexCoords );

        HlmsDatablock *datablock = renderable->getDatablock();

        setProperty( HlmsBaseProp::AlphaTest, datablock->getAlphaTest() != CMPF_ALWAYS_PASS );

        PiecesMap pieces[NumShaderTypes];
        if( datablock->getAlphaTest() != CMPF_ALWAYS_PASS )
        {
            pieces[PixelShader][HlmsBasePieces::AlphaTestCmpFunc] =
                    HlmsDatablock::getCmpString( datablock->getAlphaTest() );
        }
        calculateHashForPreCreate( renderable, pieces );

        uint32 renderableHash = this->addRenderableCache( mSetProperties, pieces );

        //For shadow casters, turn normals off. UVs & diffuse also off unless there's alpha testing.
        setProperty( HlmsBaseProp::Normal, 0 );
        setProperty( HlmsBaseProp::QTangent, 0 );
        if( datablock->getAlphaTest() != CMPF_ALWAYS_PASS )
        {
            setProperty( HlmsBaseProp::UvCount, 0 );
        }
        PiecesMap piecesCaster[NumShaderTypes];
        calculateHashForPreCaster( renderable, piecesCaster );
        uint32 renderableCasterHash = this->addRenderableCache( mSetProperties, piecesCaster );

        outHash         = renderableHash;
        outCasterHash   = renderableCasterHash;
    }
    //-----------------------------------------------------------------------------------
    HlmsCache Hlms::preparePassHash( const CompositorShadowNode *shadowNode, bool casterPass,
                                     bool dualParaboloid, SceneManager *sceneManager )
    {
        mSetProperties.clear();

        if( !casterPass )
        {
            if( shadowNode )
            {
                int32 numPssmSplits = 0;
                const vector<Real>::type *pssmSplits = shadowNode->getPssmSplits( 0 );
                if( pssmSplits )
                    numPssmSplits = static_cast<int32>( pssmSplits->size() - 1 );
                setProperty( HlmsBaseProp::PssmSplits, numPssmSplits );

                size_t numShadowMaps = shadowNode->getNumShadowCastingLights();
                if( numPssmSplits )
                    numShadowMaps += numPssmSplits - 1;
                setProperty( HlmsBaseProp::NumShadowMaps, numShadowMaps );
            }

            uint numLightsPerType[Light::NUM_LIGHT_TYPES];
            memset( numLightsPerType, 0, sizeof( numLightsPerType ) );

            if( mLightGatheringMode == LightGatherForwardPlus )
            {
                if( shadowNode )
                {
                    //Gather shadow casting lights, regardless of their type.
                    const LightClosestArray &lights = shadowNode->getShadowCastingLights();
                    LightClosestArray::const_iterator itor = lights.begin();
                    LightClosestArray::const_iterator end  = lights.end();
                    while( itor != end )
                    {
                        ++numLightsPerType[itor->light->getType()];
                        ++itor;
                    }
                }

                //Always gather directional lights.
                numLightsPerType[Light::LT_DIRECTIONAL] = 0;
                {
                    const LightListInfo &globalLightList = sceneManager->getGlobalLightList();
                    LightArray::const_iterator itor = globalLightList.lights.begin();
                    LightArray::const_iterator end  = globalLightList.lights.end();

                    while( itor != end )
                    {
                        if( (*itor)->getType() == Light::LT_DIRECTIONAL )
                            ++numLightsPerType[Light::LT_DIRECTIONAL];
                        ++itor;
                    }
                }
            }
            else if( mLightGatheringMode == LightGatherForward )
            {
                //Gather all lights.
                const LightListInfo &globalLightList = sceneManager->getGlobalLightList();
                LightArray::const_iterator itor = globalLightList.lights.begin();
                LightArray::const_iterator end  = globalLightList.lights.end();

                size_t numTotalLights = 0;

                while( itor != end && numTotalLights < mNumLightsLimit )
                {
                    ++numLightsPerType[(*itor)->getType()];
                    ++numTotalLights;
                    ++itor;
                }
            }

            setProperty( HlmsBaseProp::LightsAttenuation, numLightsPerType[Light::LT_POINT] +
                                                          numLightsPerType[Light::LT_SPOTLIGHT] );
            setProperty( HlmsBaseProp::LightsSpotParams,  numLightsPerType[Light::LT_SPOTLIGHT] );


            numLightsPerType[Light::LT_POINT]       += numLightsPerType[Light::LT_DIRECTIONAL];
            numLightsPerType[Light::LT_SPOTLIGHT]   += numLightsPerType[Light::LT_POINT];

            //The value is cummulative for each type (order: Directional, point, spot)
            setProperty( HlmsBaseProp::LightsDirectional, numLightsPerType[Light::LT_DIRECTIONAL] );
            setProperty( HlmsBaseProp::LightsPoint,       numLightsPerType[Light::LT_POINT] );
            setProperty( HlmsBaseProp::LightsSpot,        numLightsPerType[Light::LT_SPOTLIGHT] );
        }
        else
        {
            setProperty( HlmsBaseProp::ShadowCaster, casterPass );
            setProperty( HlmsBaseProp::DualParaboloidMapping, dualParaboloid );

            setProperty( HlmsBaseProp::NumShadowMaps, 0 );
            setProperty( HlmsBaseProp::PssmSplits, 0 );
            setProperty( HlmsBaseProp::LightsAttenuation, 0 );
            setProperty( HlmsBaseProp::LightsSpotParams,  0 );
            setProperty( HlmsBaseProp::LightsDirectional, 0 );
            setProperty( HlmsBaseProp::LightsPoint,       0 );
            setProperty( HlmsBaseProp::LightsSpot,        0 );
        }

        uint32 hash = getProperty( HlmsBaseProp::DualParaboloidMapping ) |
                (getProperty( HlmsBaseProp::NumShadowMaps )         << 1 )|
                (getProperty( HlmsBaseProp::PssmSplits )            << 5 )|
                (getProperty( HlmsBaseProp::LightsDirectional )     << 8 )|
                (getProperty( HlmsBaseProp::LightsPoint )           << 12)|
                (getProperty( HlmsBaseProp::LightsSpot )            << 16)|
                (getProperty( HlmsBaseProp::ShadowCaster )          << 20);

        HlmsCache retVal( hash, mType );
        retVal.setProperties = mSetProperties;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    const HlmsCache* Hlms::getMaterial( HlmsCache const *lastReturnedValue,
                                        const HlmsCache &passCache,
                                        const QueuedRenderable &queuedRenderable, bool casterPass )
    {
        uint32 finalHash;
        uint32 hash[2];
        hash[0] = casterPass ? queuedRenderable.renderable->getHlmsCasterHash() :
                               queuedRenderable.renderable->getHlmsHash();
        hash[1] = passCache.hash &
                        (queuedRenderable.movableObject->getCastShadows() ? 0xffffffff : 0xffffffe1 );

        //MurmurHash3_x86_32( hash, sizeof( hash ), IdString::Seed, &finalHash );

        assert( !(hash[0] & ~((1 << 10) - 1)) );
        assert( !(hash[1] & ~((1 << 21) - 1)) );

        finalHash = (hash[0] << 22) | hash[1];

        if( lastReturnedValue->hash != finalHash )
        {
            lastReturnedValue = this->getShaderCache( finalHash );

            if( !lastReturnedValue )
            {
                lastReturnedValue = createShaderCacheEntry( hash[0], passCache, finalHash,
                                                            queuedRenderable );
            }
        }

        return lastReturnedValue;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::setDebugOutputPath( bool enableDebugOutput, const String &path )
    {
        mDebugOutput	= enableDebugOutput;
        mOutputPath		= path;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::_notifyShadowMappingBackFaceSetting(void)
    {
        HlmsDatablockMap::const_iterator itor = mDatablocks.begin();
        HlmsDatablockMap::const_iterator end  = mDatablocks.end();

        while( itor != end )
        {
            HlmsDatablock *datablock = itor->second.datablock;
            datablock->setMacroblock( datablock->getMacroblock( false ), false );

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void Hlms::_changeRenderSystem( RenderSystem *newRs )
    {
        clearShaderCache();
        mRenderSystem = newRs;

        mShaderProfile = "unset!";

        if( mRenderSystem )
        {
            //Prefer glsl over glsles
            const String shaderProfiles[3] = { "hlsl", "glsles", "glsl" };
            const RenderSystemCapabilities *capabilities = mRenderSystem->getCapabilities();

            for( size_t i=0; i<3; ++i )
            {
                if( capabilities->isShaderProfileSupported( shaderProfiles[i] ) )
                    mShaderProfile = shaderProfiles[i];
            }

            if( !mDefaultDatablock )
                mDefaultDatablock = createDefaultDatablock();
        }
    }
    //-----------------------------------------------------------------------------------
    /*void Hlms::generateFor()
    {
        uint16 numWorldTransforms = 1;
        bool castShadows          = true;

        *//*std::ifstream inFile( "E:/Projects/Hlms/bin/Hlms/PBS/GLSL/VertexShader_vs.glsl",
                              std::ios::in | std::ios::binary );
        std::ofstream outFile( "E:/Projects/Hlms/bin/Hlms/PBS/GLSL/Output_vs.glsl",
                               std::ios::out | std::ios::binary );*//*
        std::ifstream inFile( "E:/Projects/Hlms/bin/Hlms/PBS/GLSL/PixelShader_ps.glsl",
                                      std::ios::in | std::ios::binary );
        std::ofstream outFile( "E:/Projects/Hlms/bin/Hlms/PBS/GLSL/Output_ps.glsl",
                               std::ios::out | std::ios::binary );

        String inString;
        String outString;

        inFile.seekg( 0, std::ios::end );
        inString.resize( inFile.tellg() );
        inFile.seekg( 0, std::ios::beg );

        inFile.read( &inString[0], inString.size() );

        setCommonProperties();
        //this->parse( inString, outString );
        this->parseForEach( inString, outString );
        this->parseProperties( outString, inString );
        this->collectPieces( inString, outString );
        this->insertPieces( outString, inString );
        this->parseCounter( inString, outString );

        outFile.write( &outString[0], outString.size() );
    }*/
    //-----------------------------------------------------------------------------------
    size_t Hlms::calculateLineCount( const String &buffer, size_t idx )
    {
        String::const_iterator itor = buffer.begin();
        String::const_iterator end  = buffer.begin() + idx;

        size_t lineCount = 0;

        while( itor != end )
        {
            if( *itor == '\n' )
                ++lineCount;
            ++itor;
        }

        return lineCount + 1;
    }
    //-----------------------------------------------------------------------------------
    size_t Hlms::calculateLineCount( const SubStringRef &subString )
    {
        return calculateLineCount( subString.getOriginalBuffer(), subString.getStart() );
    }
}
