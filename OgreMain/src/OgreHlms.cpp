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

#include "Compositor/OgreCompositorShadowNode.h"

#include "OgreLight.h"
#include "OgreSceneManager.h"
//#include "OgreMovableObject.h"
//#include "OgreRenderable.h"

namespace Ogre
{
    //Change per mesh (hash can be cached on the renderable)
    const IdString Hlms::HlmsPropertySkeleton           = IdString( "hlms_skeleton" );
    const IdString Hlms::HlmsPropertyBonesPerVertex     = IdString( "hlms_bones_per_vertex" );
    const IdString Hlms::HlmsPropertyPose               = IdString( "hlms_pose" );

    const IdString Hlms::HlmsPropertyNormal             = IdString( "hlms_normal" );
    const IdString Hlms::HlmsPropertyQTangent           = IdString( "hlms_qtangent" );

    const IdString Hlms::HlmsPropertyColour             = IdString( "hlms_colour" );

    const IdString Hlms::HlmsPropertyUvCount            = IdString( "hlms_uv_count" );
    const IdString Hlms::HlmsPropertyUvCount0           = IdString( "hlms_uv_count0" );
    const IdString Hlms::HlmsPropertyUvCount1           = IdString( "hlms_uv_count1" );
    const IdString Hlms::HlmsPropertyUvCount2           = IdString( "hlms_uv_count2" );
    const IdString Hlms::HlmsPropertyUvCount3           = IdString( "hlms_uv_count3" );
    const IdString Hlms::HlmsPropertyUvCount4           = IdString( "hlms_uv_count4" );
    const IdString Hlms::HlmsPropertyUvCount5           = IdString( "hlms_uv_count5" );
    const IdString Hlms::HlmsPropertyUvCount6           = IdString( "hlms_uv_count6" );
    const IdString Hlms::HlmsPropertyUvCount7           = IdString( "hlms_uv_count7" );

    //Change per frame (grouped together with scene pass)
    const IdString Hlms::HlmsPropertyLightsDirectional  = IdString( "hlms_lights_directional" );
    const IdString Hlms::HlmsPropertyLightsPoint        = IdString( "hlms_lights_point" );
    const IdString Hlms::HlmsPropertyLightsSpot         = IdString( "hlms_lights_spot" );
    const IdString Hlms::HlmsPropertyLightsAttenuation  = IdString( "hlms_lights_attenuation" );
    const IdString Hlms::HlmsPropertyLightsSpotParams   = IdString( "hlms_lights_spotparams" );

    //Change per scene pass
    const IdString Hlms::HlmsPropertyDualParaboloidMapping= IdString( "hlms_dual_paraboloid_mapping" );
    const IdString Hlms::HlmsPropertyNumShadowMaps      = IdString( "hlms_num_shadow_maps" );
    const IdString Hlms::HlmsPropertyPssmSplits         = IdString( "hlms_pssm_splits" );
    const IdString Hlms::HlmsPropertyShadowCaster       = IdString( "hlms_shadowcaster" );

    //Change per material (hash can be cached on the renderable)
    const IdString Hlms::PropertyDiffuseMap   = IdString( "diffuse_map" );
    const IdString Hlms::PropertyNormalMap    = IdString( "normal_map" );
    const IdString Hlms::PropertySpecularMap  = IdString( "specular_map" );
    const IdString Hlms::PropertyEnvProbeMap  = IdString( "envprobe_map" );
    const IdString Hlms::PropertyAlphaTest    = IdString( "alpha_test" );

    const IdString *Hlms::UvCountPtrs[8] =
    {
        &Hlms::HlmsPropertyUvCount0,
        &Hlms::HlmsPropertyUvCount1,
        &Hlms::HlmsPropertyUvCount2,
        &Hlms::HlmsPropertyUvCount3,
        &Hlms::HlmsPropertyUvCount4,
        &Hlms::HlmsPropertyUvCount5,
        &Hlms::HlmsPropertyUvCount6,
        &Hlms::HlmsPropertyUvCount7
    };

    const String ShaderFiles[] = { "VertexShader_vs", "PixelShader_ps", "GeometryShader_gs",
                                   "HullShader_hs", "DomainShader_ds" };
    const String PieceFilePatterns[] = { "piece_vs", "piece_ps", "piece_gs", "piece_hs", "piece_ds" };

    Hlms::Hlms( HlmsTypes type, IdString typeName, Archive *dataFolder ) :
        mDataFolder( dataFolder ),
        mType( type ),
        mTypeName( typeName ),
        mRenderSystem( 0 ),
        mShaderProfile( "unset!" ),
        mDebugOutput( true )
    {
        enumeratePieceFiles();
    }
    //-----------------------------------------------------------------------------------
    Hlms::~Hlms()
    {
        destroyAllDatablocks();

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

        setProperty( HlmsPropertySkeleton, numWorldTransforms > 1 );
        setProperty( HlmsPropertyUvCount, 2 );
        setProperty( "true", 1 );
        setProperty( "false", 0 );

        setProperty( HlmsPropertyDualParaboloidMapping, 0 );

        setProperty( HlmsPropertyNormal, 1 );

        setProperty( HlmsPropertyUvCount0, 2 );
        setProperty( HlmsPropertyUvCount1, 4 );
        setProperty( HlmsPropertyBonesPerVertex, 4 );

        setProperty( HlmsPropertyNumShadowMaps, 3 );
        setProperty( HlmsPropertyPssmSplits, 3 );
        setProperty( HlmsPropertyShadowCaster, 0 );

        setProperty( HlmsPropertyLightsDirectional, 1 );
        setProperty( HlmsPropertyLightsPoint, 2 );
        setProperty( HlmsPropertyLightsSpot, 3 );

        setProperty( PropertyDiffuseMap, 1 );
        setProperty( PropertyNormalMap, 1 );
        setProperty( PropertySpecularMap, 1 );
        setProperty( PropertyEnvProbeMap, 1 );
    }
    //-----------------------------------------------------------------------------------
    void Hlms::enumeratePieceFiles(void)
    {
        if( !mDataFolder )
            return; //Some Hlms implementations may not use template files at all

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
                }
                else
                {
                    for( size_t i=0; i<sizeof( blockNames ) / sizeof( char* ); ++i )
                    {
                        size_t idx = subString.find( blockNames[i] );
                        if( idx == 0 )
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
            outSubString.setEnd( it - outSubString.getOriginalBuffer().begin() - sizeof( "end" ) );
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
                    count = getProperty( argValues[0], -1 );
                }

                if( count < 0 )
                {
                    printf( "Invalid parameter at line %lu (@foreach)."
                            " '%s' is not a number nor a variable\n",
                            calculateLineCount( blockSubString ), argValues[0].c_str() );
                    syntaxError = true;
                    count = 0;
                }

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

        while( !syntaxError && outBuffer.find( "@insertpiece" ) != String::npos )
        {
            inBuffer.swap( outBuffer );
            syntaxError = insertPieces( inBuffer, outBuffer );
        }

        return syntaxError;
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::parseCounter( const String &inBuffer, String &outBuffer )
    {
        outBuffer.clear();
        outBuffer.reserve( inBuffer.size() );

        StringVector argValues;
        SubStringRef subString( &inBuffer, 0 );
        size_t _pos[2];
        _pos[0] = subString.find( "@counter" );
        _pos[1] = subString.find( "@value" );
        size_t pos;

        int keyword = 0;
        if( _pos[0] <= _pos[1] )
        {
            keyword = 0;
            pos = _pos[0];
        }
        else
        {
            keyword = 1;
            pos = _pos[1];
        }

        bool syntaxError = false;

        while( pos != String::npos && !syntaxError )
        {
            //Copy what comes before the block
            copy( outBuffer, subString, pos );

            subString.setStart( subString.getStart() + pos +
                                (keyword == 0 ? sizeof( "@counter" ) : sizeof( "@value" )) );
            evaluateParamArgs( subString, argValues, syntaxError );

            syntaxError |= argValues.size() != 1;

            if( !syntaxError )
            {
                const IdString propertyKey( argValues[0] );
                int32 count = getProperty( propertyKey );
                char tmp[16];
                sprintf( tmp, "%i", count );
                outBuffer += tmp;

                if( keyword == 0 )
                {
                    ++count;
                    setProperty( propertyKey, count );
                }
            }
            else
            {
                printf( "Syntax Error at line %lu: @counter/@value expect one parameter",
                        calculateLineCount( subString ) );
            }

            _pos[0] = subString.find( "@counter" );
            _pos[1] = subString.find( "@value" );
            if( _pos[0] <= _pos[1] )
            {
                keyword = 0;
                pos = _pos[0];
            }
            else
            {
                keyword = 1;
                pos = _pos[1];
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
        assert( mRenderableCache.size() < 64 );

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
    HlmsDatablock* Hlms::createDatablock( IdString name, const HlmsMacroblock &macroblockRef,
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

        mDatablocks[name] = DatablockEntry( retVal, visibleToManager );

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
    void Hlms::destroyAllDatablocks(void)
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
                                                      cache, OrderCacheByHash );

        assert( it == mShaderCache.end() || it->hash != hash &&
                "Can't add the same shader to the cache twice! (or a hash collision happened)" );

        cache.vertexShader              = vertexShader;
        cache.geometryShader            = geometryShader;
        cache.tesselationHullShader     = tesselationHullShader;
        cache.tesselationDomainShader   = tesselationDomainShader;
        cache.pixelShader               = pixelShader;

        return &(*mShaderCache.insert( it, cache ));
    }
    //-----------------------------------------------------------------------------------
    const HlmsCache* Hlms::getShaderCache( uint32 hash ) const
    {
        HlmsCache cache( hash, mType );
        HlmsCacheVec::const_iterator it = std::lower_bound( mShaderCache.begin(), mShaderCache.end(),
                                                            cache, OrderCacheByHash );

        if( it != mShaderCache.end() && it->hash == hash )
            return &(*it);

        return 0;
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

            StringVector::const_iterator itor = mPieceFiles[i].begin();
            StringVector::const_iterator end  = mPieceFiles[i].end();

            while( itor != end )
            {
                DataStreamPtr inFile = mDataFolder->open( *itor );

                String inString;
                String outString;

                inString.resize( inFile->size() );
                inFile->read( &inString[0], inFile->size() );

                this->parseForEach( inString, outString );
                this->parseProperties( outString, inString );
                this->collectPieces( inString, outString );
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

                this->parseForEach( inString, outString );
                this->parseProperties( outString, inString );
                this->collectPieces( inString, outString );
                this->insertPieces( outString, inString );
                this->parseCounter( inString, outString );

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

                gp->setSkeletalAnimationIncluded( getProperty( HlmsPropertySkeleton ) != 0 );
                gp->setMorphAnimationIncluded( false );
                gp->setPoseAnimationIncluded( getProperty( HlmsPropertyPose ) );
                gp->setVertexTextureFetchRequired( true );

                shaders[i] = gp;
            }
        }

        const HlmsCache* retVal = addShaderCache( finalHash, shaders[VertexShader],
                                                  shaders[GeometryShader], shaders[HullShader],
                                                  shaders[DomainShader], shaders[PixelShader] );
        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::calculateHashFor( Renderable *renderable, const HlmsParamVec &params,
                                 uint32 &outHash, uint32 &outCasterHash )
    {
        mSetProperties.clear();

        uint16 numWorldTransforms = renderable->getNumWorldTransforms();//TODO: Remove virtualness

        setProperty( HlmsPropertySkeleton, numWorldTransforms > 1 );

        RenderOperation op;
        renderable->getRenderOperation( op );
        VertexDeclaration *vertexDecl = op.vertexData->vertexDeclaration;
        const VertexDeclaration::VertexElementList &elementList = vertexDecl->getElements();
        VertexDeclaration::VertexElementList::const_iterator itor = elementList.begin();
        VertexDeclaration::VertexElementList::const_iterator end  = elementList.end();

        uint numTexCoords = 0;
        bool normalMappedCanBeSupported = false;
        while( itor != end )
        {
            const VertexElement &vertexElem = *itor;
            switch( vertexElem.getSemantic() )
            {
            case VES_NORMAL:
                if( VertexElement::getTypeCount( vertexElem.getType() ) < 4 )
                {
                    setProperty( HlmsPropertyNormal, 1 );
                }
                else
                {
                    normalMappedCanBeSupported = true;
                    setProperty( HlmsPropertyQTangent, 1 );
                }
                break;
            case VES_TANGENT:
                normalMappedCanBeSupported = true;
                break;
            case VES_DIFFUSE:
                setProperty( HlmsPropertyColour, 1 );
                break;
            case VES_TEXTURE_COORDINATES:
                numTexCoords = std::max<uint>( numTexCoords, vertexElem.getIndex() + 1 );
                setProperty( *UvCountPtrs[vertexElem.getIndex()],
                              VertexElement::getTypeCount( vertexElem.getType() ) );
                break;
            case VES_BLEND_WEIGHTS:
                setProperty( HlmsPropertyBonesPerVertex,
                             VertexElement::getTypeCount( vertexElem.getType() ) );
                break;
            default:
                break;
            }

            vertexElem.getType();
            ++itor;
        }

        setProperty( HlmsPropertyUvCount, numTexCoords );

        String paramVal;
        if( findParamInVec( params, PropertyDiffuseMap, paramVal ) )
            setProperty( PropertyDiffuseMap, 1 );
        if( normalMappedCanBeSupported && findParamInVec( params, PropertyNormalMap, paramVal ) )
            setProperty( PropertyNormalMap, 1 );
        if( findParamInVec( params, PropertySpecularMap, paramVal ) )
            setProperty( PropertySpecularMap, 1 );
        if( findParamInVec( params, PropertyEnvProbeMap, paramVal ) )
            setProperty( PropertyEnvProbeMap, 1 );
        bool alphaTest = findParamInVec( params, PropertyAlphaTest, paramVal );
        if( alphaTest )
            setProperty( PropertyAlphaTest, 1 );

        uint32 renderableHash = this->addRenderableCache( mSetProperties, 0 );

        //For shadow casters, turn normals off. UVs & diffuse also off unless there's alpha testing.
        setProperty( HlmsPropertyNormal, 0 );
        setProperty( HlmsPropertyQTangent, 0 );
        setProperty( PropertyNormalMap, 0 );
        setProperty( PropertySpecularMap, 0 );
        setProperty( PropertyEnvProbeMap, 0 );
        if( !alphaTest )
        {
            setProperty( HlmsPropertyUvCount, 0 );
            setProperty( PropertyDiffuseMap, 0 );
        }
        uint32 renderableCasterHash = this->addRenderableCache( mSetProperties, 0 );

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
                setProperty( HlmsPropertyPssmSplits, numPssmSplits );

                size_t numShadowMaps = shadowNode->getNumShadowCastingLights();
                if( numPssmSplits )
                    numShadowMaps += numPssmSplits - 1;
                setProperty( HlmsPropertyNumShadowMaps, numShadowMaps );

                uint numLightsPerType[3];
                memset( numLightsPerType, 0, sizeof( numLightsPerType ) );
                {
                    const LightClosestArray &lights = shadowNode->getShadowCastingLights();
                    LightClosestArray::const_iterator itor = lights.begin();
                    LightClosestArray::const_iterator end  = lights.end();
                    while( itor != end )
                    {
                        ++numLightsPerType[itor->light->getType()];
                        ++itor;
                    }
                }

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

                setProperty( HlmsPropertyLightsAttenuation, numLightsPerType[Light::LT_POINT] +
                                                            numLightsPerType[Light::LT_SPOTLIGHT] );
                setProperty( HlmsPropertyLightsSpotParams,  numLightsPerType[Light::LT_SPOTLIGHT] );


                numLightsPerType[Light::LT_POINT]       += numLightsPerType[Light::LT_DIRECTIONAL];
                numLightsPerType[Light::LT_SPOTLIGHT]   += numLightsPerType[Light::LT_POINT];

                //The value is cummulative for each type (order: Directional, point, spot)
                setProperty( HlmsPropertyLightsDirectional, numLightsPerType[Light::LT_DIRECTIONAL] );
                setProperty( HlmsPropertyLightsPoint,       numLightsPerType[Light::LT_POINT] );
                setProperty( HlmsPropertyLightsSpot,        numLightsPerType[Light::LT_SPOTLIGHT] );
            }
            else
            {
                const LightListInfo &globalLightList = sceneManager->getGlobalLightList();
                LightArray::const_iterator itor = globalLightList.lights.begin();
                LightArray::const_iterator end  = globalLightList.lights.end();

                uint numDirLights = 0;
                while( itor != end )
                {
                    if( (*itor)->getType() == Light::LT_DIRECTIONAL )
                        ++numDirLights;
                    ++itor;
                }

                setProperty( HlmsPropertyNumShadowMaps, 0 );
                setProperty( HlmsPropertyPssmSplits, 0 );
                setProperty( HlmsPropertyLightsAttenuation, 0 );
                setProperty( HlmsPropertyLightsSpotParams,  0 );
                setProperty( HlmsPropertyLightsDirectional, numDirLights );
                setProperty( HlmsPropertyLightsPoint,       numDirLights );
                setProperty( HlmsPropertyLightsSpot,        numDirLights );
            }
        }
        else
        {
            setProperty( HlmsPropertyShadowCaster, casterPass );
            setProperty( HlmsPropertyDualParaboloidMapping, dualParaboloid );

            setProperty( HlmsPropertyNumShadowMaps, 0 );
            setProperty( HlmsPropertyPssmSplits, 0 );
            setProperty( HlmsPropertyLightsAttenuation, 0 );
            setProperty( HlmsPropertyLightsSpotParams,  0 );
            setProperty( HlmsPropertyLightsDirectional, 0 );
            setProperty( HlmsPropertyLightsPoint,       0 );
            setProperty( HlmsPropertyLightsSpot,        0 );
        }

        uint32 hash = getProperty( HlmsPropertyDualParaboloidMapping ) |
                (getProperty( HlmsPropertyNumShadowMaps )           << 1 )|
                (getProperty( HlmsPropertyPssmSplits )              << 5 )|
                (getProperty( HlmsPropertyLightsDirectional )       << 8 )|
                (getProperty( HlmsPropertyLightsPoint )             << 12)|
                (getProperty( HlmsPropertyLightsSpot )              << 16)|
                (getProperty( HlmsPropertyShadowCaster )            << 20);

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

        MurmurHash3_x86_32( hash, sizeof( hash ), IdString::Seed, &finalHash );

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
    void Hlms::_changeRenderSystem( RenderSystem *newRs )
    {
		mShaderCache.clear();
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
