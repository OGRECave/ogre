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

#if !OGRE_NO_JSON

#include "OgreHlmsJsonCompute.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsCompute.h"
#include "OgreHlmsComputeJob.h"
#include "OgreHlmsTextureManager.h"
#include "OgreLwString.h"
#include "OgreLogManager.h"
#include "OgreTextureManager.h"

#include "rapidjson/document.h"

namespace Ogre
{
    HlmsJsonCompute::HlmsJsonCompute( HlmsManager *hlmsManager ) :
        mHlmsManager( hlmsManager )
    {
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonCompute::loadParams( const rapidjson::Value &jsonArray, ShaderParams &shaderParams,
                                      const String &jobName )
    {
        //JSON syntax has the following alternates:
        //  Manual Parameters. Syntax is:
        //      ["parameter_name", [array, with, int, values], "int" "float" "uint"]
        //The 3rd parameter is optional and specifies how the array values are going to be
        //interpreted. Default is float. The array with the parameters can have up to
        //16 elements.
        //
        //  Auto params. Syntax is:
        //      ["parameter_name", "camera_position"]
        //      ["parameter_name", "camera_position", extra_param_value]
        //The 3rd parameter is mandatory in some cases, optional in others. Its value
        //depends on the auto parameter.
        for( rapidjson::SizeType i=0; i<jsonArray.Size(); ++i )
        {
            const rapidjson::Value &subArray = jsonArray[i];
            if( subArray.Size() < 2u || subArray.Size() > 3u )
            {
                LogManager::getSingleton().logMessage( "Error parsing JSON '" + jobName +
                                                       "': params expecting an array with "
                                                       "2 to 3 elements." );
                continue;
            }

            if( !subArray[0].IsString() )
            {
                LogManager::getSingleton().logMessage( "Error parsing JSON '" + jobName +
                                                       "': params expecting parameter name"
                                                       " as first element in array." );
                continue;
            }

            const char *paramName = subArray[0].GetString();

            if( subArray[1].IsString() )
            {
                //Automatic param
                ShaderParams::Param param;
                param.isAutomatic = true;
                param.name = paramName;

                const GpuProgramParameters::AutoConstantDefinition *acDef = GpuProgramParameters::
                        getAutoConstantDefinition( subArray[1].GetString() );
                param.ap.acType = acDef->acType;

                param.ap.extraParamType = acDef->dataType == GpuProgramParameters::ACDT_REAL ?
                            ShaderParams::ElementFloat : ShaderParams::ElementUInt;

                if( subArray.Size() > 2u && acDef->dataType != GpuProgramParameters::ACDT_NONE )
                {
                    if( subArray[2].IsNumber() )
                        param.ap.extraParamValue = subArray[2].GetDouble();
                    else
                    {
                        LogManager::getSingleton().logMessage( "Error parsing JSON '" + jobName +
                                                               "': invalid 3rd parameter for "
                                                               + param.name );
                        continue;
                    }
                }
                else
                {
                    //No extra parameter given. Assign default (or raise
                    //error if extra parameter must be present)
                    param.ap.extraParamValue = 0;
                    if( acDef->dataType == GpuProgramParameters::ACDT_REAL )
                    {
                        if( acDef->acType == GpuProgramParameters::ACT_TIME ||
                            acDef->acType == GpuProgramParameters::ACT_FRAME_TIME )
                        {
                            //Set default
                            param.ap.extraParamValue = 1;
                        }
                        else
                        {
                            LogManager::getSingleton().logMessage( "Error parsing JSON '" + jobName +
                                                                   "': expecting 3rd parameter for "
                                                                   + param.name );
                            continue;
                        }
                    }
                    else if( acDef->dataType == GpuProgramParameters::ACDT_INT )
                    {
                        if( acDef->acType == GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX ||
                            acDef->acType == GpuProgramParameters::ACT_TEXTURE_WORLDVIEWPROJ_MATRIX ||
                            acDef->acType == GpuProgramParameters::ACT_SPOTLIGHT_VIEWPROJ_MATRIX ||
                            acDef->acType == GpuProgramParameters::ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX )
                        {
                            //Set default
                            param.ap.extraParamValue = 0;
                        }
                        else
                        {
                            LogManager::getSingleton().logMessage( "Error parsing JSON '" + jobName +
                                                                   "': expecting 3rd parameter for "
                                                                   + param.name );
                            continue;
                        }
                    }
                }

                shaderParams.mParams.push_back( param );
            }
            else
            {
                ShaderParams::Param param;
                param.isAutomatic = false;
                param.name = paramName;

                param.mp.elementType = ShaderParams::ElementFloat;

                if( subArray.Size() > 2u && subArray[2].IsString() )
                {
                    const char *typeName = subArray[2].GetString();
                    if( !strcmp( typeName, "int" ) )
                        param.mp.elementType = ShaderParams::ElementInt;
                    else if( !strcmp( typeName, "uint" ) )
                        param.mp.elementType = ShaderParams::ElementUInt;
                }

                if( !subArray[1].IsArray() )
                {
                    LogManager::getSingleton().logMessage( "Error parsing JSON '" + jobName +
                                                           "': expecting an array for "
                                                           + param.name );
                    continue;
                }

                const rapidjson::Value &paramArray = subArray[1];
                const rapidjson::SizeType paramArraySize = std::min( paramArray.Size(), 16u );

                param.mp.dataSizeBytes = paramArraySize * 4u;
                memset( param.mp.dataBytes, 0, param.mp.dataSizeBytes );

                float *dataFloat = reinterpret_cast<float*>( param.mp.dataBytes );
                int32 *dataInt32 = reinterpret_cast<int32*>( param.mp.dataBytes );
                uint32 *dataUint32 = reinterpret_cast<uint32*>( param.mp.dataBytes );

                for( rapidjson::SizeType j=0; j<paramArraySize; ++j )
                {
                    if( param.mp.elementType == ShaderParams::ElementFloat &&
                        paramArray[j].IsNumber() )
                    {
                       dataFloat[j] = paramArray[j].GetDouble();
                    }
                    else if( param.mp.elementType == ShaderParams::ElementInt &&
                             paramArray[j].IsInt() )
                    {
                        dataInt32[j] = paramArray[j].GetInt();
                    }
                    else if( param.mp.elementType == ShaderParams::ElementUInt &&
                             paramArray[j].IsUint() )
                    {
                        dataUint32[j] = paramArray[j].GetUint();
                    }
                }
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonCompute::loadJob( const rapidjson::Value &json, const HlmsJson::NamedBlocks &blocks,
                                   HlmsComputeJob *job, const String &jobName )
    {
        assert( dynamic_cast<HlmsCompute*>( job->getCreator() ) );
        HlmsCompute *hlmsCompute = static_cast<HlmsCompute*>( job->getCreator() );

        rapidjson::Value::ConstMemberIterator itor = json.FindMember( "threads_per_group" );
        if( itor != json.MemberEnd() && itor->value.IsArray() )
        {
            uint32 val[3];

            bool hasError = false;

            const rapidjson::Value &jsonArray = itor->value;
            if( jsonArray.Size() != 3u )
                hasError = true;
            for( rapidjson::SizeType i=0; i<3u && !hasError; ++i )
            {
                if( jsonArray[i].IsUint() )
                    val[i] = jsonArray[i].GetUint();
                else
                    hasError = true;
            }

            if( hasError )
            {
                LogManager::getSingleton().logMessage( "Error parsing JSON '" + jobName +
                                                       "': threads_per_group' expects an array "
                                                       "with three values. e.g. [16, 16, 2]" );
            }
            else
                job->setThreadsPerGroup( val[0], val[1], val[2] );
        }

        itor = json.FindMember( "thread_groups" );
        if( itor != json.MemberEnd() && itor->value.IsArray() )
        {
            uint32 val[3];

            bool hasError = false;

            const rapidjson::Value &jsonArray = itor->value;
            if( jsonArray.Size() != 3u )
                hasError = true;
            for( rapidjson::SizeType i=0; i<3u && !hasError; ++i )
            {
                if( jsonArray[i].IsUint() )
                    val[i] = jsonArray[i].GetUint();
                else
                    hasError = true;
            }

            if( hasError )
            {
                LogManager::getSingleton().logMessage( "Error parsing JSON '" + jobName +
                                                       "': thread_groups' expects an array "
                                                       "with three values. e.g. [16, 16, 2]" );
            }
            else
                job->setNumThreadGroups( val[0], val[1], val[2] );
        }

        itor = json.FindMember( "shaders" );
        if( itor != json.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &subobj = itor->value;

//            itor = subobj.FindMember( "source" );
//            if( itor != subobj.MemberEnd() && itor->value.IsString() )
//                job->getName() = static_cast<float>( itor->value.GetDouble() );
        }

        itor = json.FindMember( "properties" );
        if( itor != json.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &subobj = itor->value;

            rapidjson::Value::ConstMemberIterator itor = subobj.MemberBegin();
            rapidjson::Value::ConstMemberIterator end  = subobj.MemberEnd();

            while( itor != end )
            {
                if( itor->value.IsInt() )
                {
                    job->setProperty( itor->name.GetString(),
                                      static_cast<int32>(itor->value.GetInt()) );
                }

                ++itor;
            }
        }

        itor = json.FindMember( "params" );
        if( itor != json.MemberEnd() && itor->value.IsArray() )
            loadParams( itor->value, job->getShaderParams( "Default" ), jobName );

        itor = json.FindMember( "params_glsl" );
        if( itor != json.MemberEnd() && itor->value.IsArray() )
            loadParams( itor->value, job->getShaderParams( "glsl" ), jobName );

        itor = json.FindMember( "params_hlsl" );
        if( itor != json.MemberEnd() && itor->value.IsArray() )
            loadParams( itor->value, job->getShaderParams( "hlsl" ), jobName );

        itor = json.FindMember( "textures" );
        if( itor != json.MemberEnd() && itor->value.IsArray() )
        {
            const rapidjson::Value &jsonArray = itor->value;

            assert( jsonArray.Size() < 256u && "Exceeding max limit!" );

            const uint8 arraySize = std::min( jsonArray.Size(), 255u );
            job->setNumTexUnits( arraySize );

            for( uint8 i=0; i<arraySize; ++i )
            {
                if( jsonArray[i].IsObject() )
                {
                    const rapidjson::Value &subobj = jsonArray[i];

                    itor = subobj.FindMember( "sampler" );
                    if( itor != subobj.MemberEnd() && itor->value.IsString() )
                    {
                        map<LwConstString, const HlmsSamplerblock*>::type::const_iterator it =
                                blocks.samplerblocks.find(
                                    LwConstString::FromUnsafeCStr(itor->value.GetString()) );
                        if( it != blocks.samplerblocks.end() )
                        {
                            job->_setSamplerblock( i, it->second );
                            mHlmsManager->addReference( it->second );
                        }
                    }

                    itor = subobj.FindMember( "texture" );
                    if( itor != subobj.MemberEnd() && itor->value.IsString() )
                    {
                        const char *textureName = itor->value.GetString();

                        TexturePtr texture = TextureManager::getSingleton().getByName(
                                    textureName, ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME );
                        job->setTexture( i, texture );
                    }

                    itor = subobj.FindMember( "texture_hlms" );
                    if( itor != subobj.MemberEnd() && itor->value.IsArray() &&
                        itor->value.Size() >= 1u && itor->value.Size() <= 2u &&
                        itor->value[0].IsString() )
                    {
                        const rapidjson::Value &jsonArray = itor->value;

                        const char *textureName = jsonArray[0].GetString();
                        HlmsTextureManager *hlmsTextureManager = mHlmsManager->getTextureManager();
                        HlmsTextureManager::TextureLocation texLocation = hlmsTextureManager->
                                createOrRetrieveTexture( textureName,
                                                         HlmsTextureManager::TEXTURE_TYPE_DIFFUSE );
                        job->setTexture( i, texLocation.texture );

                        if( jsonArray.Size() >= 2u && jsonArray[1].IsString() )
                        {
                            ShaderParams &shaderParams = job->getShaderParams( "Default" );
                            ShaderParams::Param param;
                            param.name = jsonArray[1].GetString();
                            param.isAutomatic = false;
                            param.mp.elementType    = ShaderParams::ElementInt;
                            param.mp.dataSizeBytes  = sizeof(int32);
                            memcpy( param.mp.dataBytes, &texLocation.xIdx, sizeof(int32) );
                            shaderParams.mParams.push_back( param );
                        }
                    }

                    //TODO: Implement named buffers
//                    itor = subobj.FindMember( "buffer" );
//                    if( itor != subobj.MemberEnd() && itor->value.IsString() )
//                    {
//                        const char *bufferName = itor->value.GetString();

//                        size_t bufferOffset = 0, bufferSize = 0;
//                        itor = subobj.FindMember( "offset" );
//                        if( itor != subobj.MemberEnd() && itor->value.IsUint() )
//                            bufferOffset = itor->value.GetUint();

//                        itor = subobj.FindMember( "size" );
//                        if( itor != subobj.MemberEnd() && itor->value.IsUint() ){}
//                            bufferSize = itor->value.GetUint();
//                    }
                }
            }
        }
    }
}
#endif
