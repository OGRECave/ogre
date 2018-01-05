/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2017 Torus Knot Software Ltd

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

#include "OgreSceneFormatImporter.h"
#include "OgreSceneManager.h"
#include "OgreRoot.h"

#include "OgreLwString.h"

#include "OgreItem.h"
#include "OgreMesh2.h"
#include "OgreEntity.h"
#include "OgreHlms.h"

#include "OgreMeshSerializer.h"
#include "OgreMesh2Serializer.h"
#include "OgreFileSystemLayer.h"

#include "rapidjson/document.h"

namespace Ogre
{
    SceneFormatImporter::SceneFormatImporter( Root *root, SceneManager *sceneManager ) :
        SceneFormatBase( root, sceneManager )
    {
    }
    //-----------------------------------------------------------------------------------
    SceneFormatImporter::~SceneFormatImporter()
    {
    }
    //-----------------------------------------------------------------------------------
    inline float SceneFormatImporter::decodeFloat( const rapidjson::Value &jsonValue )
    {
        union MyUnion
        {
            float   f32;
            uint32  u32;
        };

        MyUnion myUnion;
        myUnion.u32 = jsonValue.GetUint();
        return myUnion.f32;
    }
    //-----------------------------------------------------------------------------------
    inline Vector3 SceneFormatImporter::decodeVector3Array( const rapidjson::Value &jsonArray )
    {
        Vector3 retVal( Vector3::ZERO );

        const rapidjson::SizeType arraySize = std::min( 3u, jsonArray.Size() );
        for( rapidjson::SizeType i=0; i<arraySize; ++i )
        {
            if( jsonArray[i].IsUint() )
                retVal[i] = decodeFloat( jsonArray[i] );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    inline Quaternion SceneFormatImporter::decodeQuaternionArray( const rapidjson::Value &jsonArray )
    {
        Quaternion retVal( Quaternion::IDENTITY );

        const rapidjson::SizeType arraySize = std::min( 4u, jsonArray.Size() );
        for( rapidjson::SizeType i=0; i<arraySize; ++i )
        {
            if( jsonArray[i].IsUint() )
                retVal[i] = decodeFloat( jsonArray[i] );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::importNode( const rapidjson::Value &nodeValue, Node *node )
    {
        rapidjson::Value::ConstMemberIterator  itor;

        itor = nodeValue.FindMember( "position" );
        if( itor != nodeValue.MemberEnd() && itor->value.IsArray() )
            node->setPosition( decodeVector3Array( itor->value ) );

        itor = nodeValue.FindMember( "rotation" );
        if( itor != nodeValue.MemberEnd() && itor->value.IsArray() )
            node->setOrientation( decodeQuaternionArray( itor->value ) );

        itor = nodeValue.FindMember( "scale" );
        if( itor != nodeValue.MemberEnd() && itor->value.IsArray() )
            node->setScale( decodeVector3Array( itor->value ) );

        itor = nodeValue.FindMember( "inherit_orientation" );
        if( itor != nodeValue.MemberEnd() && itor->value.IsBool() )
            node->setInheritOrientation( itor->value.GetBool() );

        itor = nodeValue.FindMember( "inherit_scale" );
        if( itor != nodeValue.MemberEnd() && itor->value.IsBool() )
            node->setInheritScale( itor->value.GetBool() );
    }
    //-----------------------------------------------------------------------------------
    SceneNode* SceneFormatImporter::importSceneNode( const rapidjson::Value &sceneNodeValue,
                                                     uint32 nodeIdx,
                                                     const rapidjson::Value &sceneNodesJson )
    {
        SceneNode *sceneNode = 0;

        rapidjson::Value::ConstMemberIterator itTmp = sceneNodeValue.FindMember( "node" );
        if( itTmp != sceneNodeValue.MemberEnd() && itTmp->value.IsObject() )
        {
            const rapidjson::Value &nodeValue = itTmp->value;

            bool isStatic = false;
            uint32 parentIdx = nodeIdx;

            itTmp = nodeValue.FindMember( "parent_id" );
            if( itTmp != nodeValue.MemberEnd() && itTmp->value.IsUint() )
                parentIdx = itTmp->value.GetUint();

            itTmp = nodeValue.FindMember( "is_static" );
            if( itTmp != nodeValue.MemberEnd() && itTmp->value.IsBool() )
                isStatic = itTmp->value.GetBool();

            const SceneMemoryMgrTypes sceneNodeType = isStatic ? SCENE_STATIC : SCENE_DYNAMIC;

            if( parentIdx != nodeIdx )
            {
                SceneNode *parentNode = 0;
                IndexToSceneNodeMap::const_iterator parentNodeIt = mCreatedSceneNodes.find( parentIdx );
                if( parentNodeIt == mCreatedSceneNodes.end() )
                {
                    //Our parent node will be created after us. Initialize it now.
                    if( parentIdx < sceneNodesJson.MemberCount() &&
                        sceneNodesJson[parentIdx].IsObject() )
                    {
                        parentNode = importSceneNode( sceneNodesJson[parentIdx], parentIdx,
                                                      sceneNodesJson );
                    }

                    if( !parentNode )
                    {
                        OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND,
                                     "Node " + StringConverter::toString( nodeIdx ) + " is child of " +
                                     StringConverter::toString( parentIdx ) +
                                     " but we could not find it or create it. This file is malformed.",
                                     "SceneFormatImporter::importSceneNode" );
                    }
                }
                else
                {
                    //Parent was already created
                    parentNode = parentNodeIt->second;
                }

                sceneNode = parentNode->createChildSceneNode( sceneNodeType );
            }
            else
            {
                //Has no parent. Could be root scene node,
                //or a loose node whose parent wasn't exported.
                sceneNode = mSceneManager->createSceneNode( sceneNodeType );
            }

            importNode( nodeValue, sceneNode );

            mCreatedSceneNodes[nodeIdx] = sceneNode;
        }
        else
        {
            OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND,
                         "Object 'node' must be present in a scene_node. SceneNode: " +
                         StringConverter::toString( nodeIdx ) + " File: " + mFilename,
                         "SceneFormatImporter::importSceneNodes" );
        }

        return sceneNode;
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::importSceneNodes( const rapidjson::Value &json )
    {
        rapidjson::Value::ConstMemberIterator begin = json.MemberBegin();
        rapidjson::Value::ConstMemberIterator itor = begin;
        rapidjson::Value::ConstMemberIterator end  = json.MemberEnd();

        while( itor != end )
        {
            const size_t nodeIdx = itor - begin;
            if( itor->value.IsObject() &&
                mCreatedSceneNodes.find( nodeIdx ) == mCreatedSceneNodes.end() )
            {
                importSceneNode( itor->value, nodeIdx, json );
            }

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::importScene( const String &filename, const char *jsonString )
    {
        mFilename = filename;

        rapidjson::Document d;
        d.Parse( jsonString );

        if( d.HasParseError() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "SceneFormatImporter::importScene",
                         "Invalid JSON string in file " + filename );
        }

        rapidjson::Value::ConstMemberIterator itor = d.FindMember( "scene_nodes" );
        if( itor != d.MemberEnd() && itor->value.IsArray() )
            importSceneNodes( itor->value );
    }
}
