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
    uint8 HlmsJsonCompute::parseAccess( const char *value )
    {
        if( !strcmp( value, "read" ) )
            return ResourceAccess::Read;
        if( !strcmp( value, "write" ) )
            return ResourceAccess::Write;
        if( !strcmp( value, "readwrite" ) )
            return ResourceAccess::ReadWrite;

        return 0;
    }
    //-----------------------------------------------------------------------------------
    ResourceAccess::ResourceAccess HlmsJsonCompute::parseAccess( const rapidjson::Value &json )
    {
        uint8 access = 0;
        if( json.IsArray() )
        {
            for( rapidjson::SizeType i=0; i<json.Size(); ++i )
            {
                if( json[i].IsString() )
                    access |= parseAccess( json[i].GetString() );
            }
        }
        else if( json.IsString() )
        {
            access = parseAccess( json.GetString() );
        }

        return static_cast<ResourceAccess::ResourceAccess>( access );
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

            if( !subArray.IsArray() )
            {
                //Not an array. Could be a comment. Skip it.
                continue;
            }

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
                param.isAutomatic   = true;
                param.isDirty       = true;
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
                param.isAutomatic   = false;
                param.isDirty       = true;
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

                shaderParams.mParams.push_back( param );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonCompute::loadTexture( const rapidjson::Value &json, const HlmsJson::NamedBlocks &blocks,
                                       HlmsComputeJob *job, uint8 slotIdx )
    {
        rapidjson::Value::ConstMemberIterator itor = json.FindMember( "sampler" );
        if( itor != json.MemberEnd() && itor->value.IsString() )
        {
            map<LwConstString, const HlmsSamplerblock*>::type::const_iterator it =
                    blocks.samplerblocks.find(
                        LwConstString( itor->value.GetString(),
                                       itor->value.GetStringLength() + 1u ) );
            if( it != blocks.samplerblocks.end() )
            {
                job->_setSamplerblock( slotIdx, it->second );
                mHlmsManager->addReference( it->second );
            }
        }

        itor = json.FindMember( "texture" );
        if( itor != json.MemberEnd() && itor->value.IsString() )
        {
            const char *textureName = itor->value.GetString();

            TexturePtr texture = TextureManager::getSingleton().getByName(
                        textureName, ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME );
            job->setTexture( slotIdx, texture );
        }

        itor = json.FindMember( "texture_hlms" );
        if( itor != json.MemberEnd() && itor->value.IsArray() &&
            itor->value.Size() >= 1u && itor->value.Size() <= 2u &&
            itor->value[0].IsString() )
        {
            const rapidjson::Value &jsonArray = itor->value;

            const char *textureName = jsonArray[0].GetString();
            HlmsTextureManager *hlmsTextureManager = mHlmsManager->getTextureManager();
            HlmsTextureManager::TextureLocation texLocation = hlmsTextureManager->
                    createOrRetrieveTexture( textureName,
                                             HlmsTextureManager::TEXTURE_TYPE_DIFFUSE );
            job->setTexture( slotIdx, texLocation.texture );

            if( jsonArray.Size() >= 2u && jsonArray[1].IsString() )
            {
                ShaderParams &shaderParams = job->getShaderParams( "default" );
                ShaderParams::Param param;
                param.name = jsonArray[1].GetString();
                param.isAutomatic = false;
                param.isDirty     = true;
                param.mp.elementType    = ShaderParams::ElementInt;
                param.mp.dataSizeBytes  = sizeof(int32);
                memcpy( param.mp.dataBytes, &texLocation.xIdx, sizeof(int32) );
                shaderParams.mParams.push_back( param );
            }
        }

        //TODO: Implement named buffers
//        itor = json.FindMember( "buffer" );
//        if( itor != json.MemberEnd() && itor->value.IsString() )
//        {
//            const char *bufferName = itor->value.GetString();

//            size_t bufferOffset = 0, bufferSize = 0;
//            itor = json.FindMember( "offset" );
//            if( itor != json.MemberEnd() && itor->value.IsUint() )
//                bufferOffset = itor->value.GetUint();

//            itor = json.FindMember( "size" );
//            if( itor != json.MemberEnd() && itor->value.IsUint() ){}
//                bufferSize = itor->value.GetUint();
//        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonCompute::loadBasedOnTextureOrUav( const rapidjson::Value &objValue,
                                                   const String &jobName, HlmsComputeJob *job,
                                                   int _threadGroupsBasedOn )
    {
        HlmsComputeJob::ThreadGroupsBasedOn threadGroupsBasedOn =
                static_cast<HlmsComputeJob::ThreadGroupsBasedOn>( _threadGroupsBasedOn );

        if( objValue.IsUint() )
        {
            job->setNumThreadGroupsBasedOn( threadGroupsBasedOn,
                                            static_cast<uint8>( objValue.GetUint() ),
                                            1u, 1u, 1u );
        }
        else if( objValue.IsObject() )
        {
            uint8 slot = 0;
            uint8 divisors[3] = { 1u, 1u, 1u };

            bool hasError = false;

            const rapidjson::Value &subobj = objValue;
            rapidjson::Value::ConstMemberIterator itor = subobj.FindMember( "slot" );

            if( itor != subobj.MemberEnd() && itor->value.IsUint() )
                slot = static_cast<uint8>( itor->value.GetUint() );
            else
                hasError = true;

            itor = subobj.FindMember( "divisor" );

            if( itor != subobj.MemberEnd() && itor->value.IsArray() )
            {
                const rapidjson::Value &divArray = itor->value;
                const rapidjson::SizeType arraySize = std::min( 3u, divArray.Size() );
                for( rapidjson::SizeType i=0; i<arraySize; ++i )
                {
                    if( divArray[i].IsUint() )
                    {
                        divisors[i] = divArray[i].GetUint();
                    }
                    else
                    {
                        hasError = true;
                        LogManager::getSingleton().logMessage(
                                    "Array with 3 integers expected in " + jobName + ". "
                                    "Syntax is thread_groups_based_on_texture : { \"slot\" "
                                    ": 0, \"divisor\" : [1, 1, 1] } or the short form: "
                                    "thread_groups_based_on_texture : 0 (with no divisors)" );
                    }
                }
            }

            if( !hasError )
            {
                job->setNumThreadGroupsBasedOn( threadGroupsBasedOn,
                                                slot, divisors[0], divisors[1], divisors[2] );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonCompute::loadJob( const rapidjson::Value &json, const HlmsJson::NamedBlocks &blocks,
                                   HlmsComputeJob *job, const String &jobName )
    {
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

        itor = json.FindMember( "inform_shader_of_texture_data_change" );
        if( itor != json.MemberEnd() && itor->value.IsBool() )
            job->setInformHlmsOfTextureData( itor->value.GetBool() );

        itor = json.FindMember( "thread_groups_based_on_texture" );
        if( itor != json.MemberEnd() )
        {
            loadBasedOnTextureOrUav( itor->value, jobName, job,
                                     HlmsComputeJob::ThreadGroupsBasedOnTexture );
        }

        itor = json.FindMember( "thread_groups_based_on_uav" );
        if( itor != json.MemberEnd() )
        {
            loadBasedOnTextureOrUav( itor->value, jobName, job,
                                     HlmsComputeJob::ThreadGroupsBasedOnUav );
        }

        itor = json.FindMember( "properties" );
        if( itor != json.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &subobj = itor->value;

            rapidjson::Value::ConstMemberIterator itSubObj = subobj.MemberBegin();
            rapidjson::Value::ConstMemberIterator enSubObj = subobj.MemberEnd();

            while( itSubObj != enSubObj )
            {
                if( itSubObj->value.IsInt() )
                {
                    job->setProperty( itSubObj->name.GetString(),
                                      static_cast<int32>(itSubObj->value.GetInt()) );
                }

                ++itSubObj;
            }
        }

        itor = json.FindMember( "params" );
        if( itor != json.MemberEnd() && itor->value.IsArray() )
            loadParams( itor->value, job->getShaderParams( "default" ), jobName );

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
                    loadTexture( jsonArray[i], blocks, job, i );
            }
        }

        itor = json.FindMember( "uav_units" );
        if( itor != json.MemberEnd() && itor->value.IsUint() )
        {
            assert( itor->value.GetUint() < 256u && "Exceeding max limit!" );
            job->setNumUavUnits( std::min( itor->value.GetUint(), 255u ) );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonCompute::loadJobs( const rapidjson::Value &json, const HlmsJson::NamedBlocks &blocks )
    {
        HlmsCompute *hlmsCompute = mHlmsManager->getComputeHlms();

        StringVector pieceFiles;

        rapidjson::Value::ConstMemberIterator itJob = json.MemberBegin();
        rapidjson::Value::ConstMemberIterator enJob = json.MemberEnd();

        while( itJob != enJob )
        {
            if( itJob->value.IsObject() )
            {
                const String jobName( itJob->name.GetString(), itJob->name.GetStringLength() );
                pieceFiles.clear();

                rapidjson::Value::ConstMemberIterator itor = itJob->value.FindMember( "pieces" );
                if( itor != itJob->value.MemberEnd() && itor->value.IsString() )
                    pieceFiles.push_back( itor->value.GetString() );
                else if( itor != itJob->value.MemberEnd() && itor->value.IsArray() )
                {
                    const rapidjson::Value &jsonArray = itor->value;
                    for( rapidjson::SizeType i=0; i<jsonArray.Size(); ++i )
                    {
                        if( jsonArray[i].IsString() )
                            pieceFiles.push_back( jsonArray[i].GetString() );
                    }
                }

                itor = itJob->value.FindMember( "source" );
                if( itor != itJob->value.MemberEnd() && itor->value.IsString() )
                {
                    HlmsComputeJob *job = hlmsCompute->createComputeJob( jobName, jobName,
                                                                         itor->value.GetString(),
                                                                         pieceFiles );
                    loadJob( itJob->value, blocks, job, jobName );
                }
            }

            ++itJob;
        }
    }
}
#endif
