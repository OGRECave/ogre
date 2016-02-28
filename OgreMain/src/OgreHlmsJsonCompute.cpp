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
    void HlmsJsonCompute::loadJob( const rapidjson::Value &json, const HlmsJson::NamedBlocks &blocks,
                                   HlmsComputeJob *job, const String &jobName )
    {
        assert( dynamic_cast<HlmsCompute*>( job->getCreator() ) );
        HlmsCompute *hlmsCompute = static_cast<HlmsCompute*>( job->getCreator() );

        rapidjson::Value::ConstMemberIterator itor = json.FindMember("threads_per_group");
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

        itor = json.FindMember("thread_groups");
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

        itor = json.FindMember("shaders");
        if( itor != json.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &subobj = itor->value;

//            itor = subobj.FindMember( "source" );
//            if( itor != subobj.MemberEnd() && itor->value.IsString() )
//                job->getName() = static_cast<float>( itor->value.GetDouble() );
        }

        itor = json.FindMember("properties");
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

        itor = json.FindMember("textures");
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

                    itor = subobj.FindMember("sampler");
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

//                    itor = subobj.FindMember( "texture_hlms" );
//                    if( itor != subobj.MemberEnd() && itor->value.IsString() )
//                    {
//                        const char *textureName = itor->value.GetString();
//                        HlmsTextureManager *hlmsTextureManager = mHlmsManager->getTextureManager();
//                        HlmsTextureManager::TextureLocation texLocation = hlmsTextureManager->
//                                createOrRetrieveTexture( textureName,
//                                                         HlmsTextureManager::TEXTURE_TYPE_DIFFUSE );
//                        //job->setProperty( i,  );
//                        //TODO: Parameter
//                        job->setTexture( i, texLocation.texture );
//                    }
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
