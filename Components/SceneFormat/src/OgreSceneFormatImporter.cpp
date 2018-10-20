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
#include "OgreDecal.h"
#include "OgreHlms.h"
#include "OgreHlmsTextureManager.h"

#include "OgreHlmsPbs.h"
#include "InstantRadiosity/OgreInstantRadiosity.h"
#include "OgreIrradianceVolume.h"

#include "Cubemaps/OgreParallaxCorrectedCubemap.h"
#include "Compositor/OgreCompositorManager2.h"

#include "OgreTextureManager.h"

#include "OgreMeshSerializer.h"
#include "OgreMesh2Serializer.h"
#include "OgreFileSystemLayer.h"

#include "OgreLogManager.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

namespace Ogre
{
    SceneFormatImporter::SceneFormatImporter( Root *root, SceneManager *sceneManager,
                                              const String &defaultPccWorkspaceName ) :
        SceneFormatBase( root, sceneManager ),
        mInstantRadiosity( 0 ),
        mIrradianceVolume( 0 ),
        mParallaxCorrectedCubemap( 0 ),
        mSceneComponentTransform( Matrix4::IDENTITY ),
        mDefaultPccWorkspaceName( defaultPccWorkspaceName ),
        mUseBinaryFloatingPoint( true ),
        mUsingOitd( false )
    {
        memset( mRootNodes, 0, sizeof(mRootNodes) );
        memset( mParentlessRootNodes, 0, sizeof(mParentlessRootNodes) );
    }
    //-----------------------------------------------------------------------------------
    SceneFormatImporter::~SceneFormatImporter()
    {
        destroyInstantRadiosity();
        destroyParallaxCorrectedCubemap();
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::destroyInstantRadiosity(void)
    {
        if( mIrradianceVolume )
        {
            HlmsPbs *hlmsPbs = getPbs();
            if( hlmsPbs && hlmsPbs->getIrradianceVolume() == mIrradianceVolume )
                hlmsPbs->setIrradianceVolume( 0 );

            delete mIrradianceVolume;
            mIrradianceVolume = 0;
        }

        delete mInstantRadiosity;
        mInstantRadiosity = 0;
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::destroyParallaxCorrectedCubemap(void)
    {
        if( mParallaxCorrectedCubemap )
        {
            HlmsPbs *hlmsPbs = getPbs();
            if( hlmsPbs && hlmsPbs->getParallaxCorrectedCubemap() == mParallaxCorrectedCubemap )
                hlmsPbs->setParallaxCorrectedCubemap( 0 );

            delete mParallaxCorrectedCubemap;
            mParallaxCorrectedCubemap = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    Light::LightTypes SceneFormatImporter::parseLightType( const char *value )
    {
        for( size_t i=0; i<Light::NUM_LIGHT_TYPES+1u; ++i )
        {
            if( !strcmp( value, c_lightTypes[i] ) )
                return static_cast<Light::LightTypes>( i );
        }

        return Light::LT_DIRECTIONAL;
    }
    //-----------------------------------------------------------------------------------
    inline bool SceneFormatImporter::isFloat( const rapidjson::Value &jsonValue ) const
    {
        if( mUseBinaryFloatingPoint )
        {
            return jsonValue.IsUint();
        }
        else
        {
            if( jsonValue.IsDouble() )
                return true;
            else
            {
                if( !strcmp( jsonValue.GetString(), "nan" ) ||
                    !strcmp( jsonValue.GetString(), "inf" ) ||
                    !strcmp( jsonValue.GetString(), "-inf" ) )
                {
                    return true;
                }
            }

            return false;
        }
    }
    //-----------------------------------------------------------------------------------
    inline bool SceneFormatImporter::isDouble( const rapidjson::Value &jsonValue ) const
    {
        if( mUseBinaryFloatingPoint )
        {
            return jsonValue.IsUint();
        }
        else
        {
            if( jsonValue.IsDouble() )
                return true;
            else
            {
                if( !strcmp( jsonValue.GetString(), "nan" ) ||
                    !strcmp( jsonValue.GetString(), "inf" ) ||
                    !strcmp( jsonValue.GetString(), "-inf" ) )
                {
                    return true;
                }
            }

            return false;
        }
    }
    //-----------------------------------------------------------------------------------
    inline float SceneFormatImporter::decodeFloat( const rapidjson::Value &jsonValue )
    {
        if( mUseBinaryFloatingPoint )
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
        else
        {
            if( jsonValue.IsString() )
            {
                if( !strcmp( jsonValue.GetString(), "nan" ) )
                    return std::numeric_limits<float>::quiet_NaN();
                else if( !strcmp( jsonValue.GetString(), "inf" ) )
                    return std::numeric_limits<float>::infinity();
                else if( !strcmp( jsonValue.GetString(), "-inf" ) )
                    return -std::numeric_limits<float>::infinity();
            }
            else if( jsonValue.IsDouble() )
            {
                return static_cast<float>( jsonValue.GetDouble() );
            }
        }

        return 0;
    }
    //-----------------------------------------------------------------------------------
    inline double SceneFormatImporter::decodeDouble( const rapidjson::Value &jsonValue )
    {
        if( mUseBinaryFloatingPoint )
        {
            union MyUnion
            {
                double  f64;
                uint64  u64;
            };

            MyUnion myUnion;
            myUnion.u64 = jsonValue.GetUint64();
            return myUnion.f64;
        }
        else
        {
            if( jsonValue.IsString() )
            {
                if( !strcmp( jsonValue.GetString(), "nan" ) )
                    return std::numeric_limits<double>::quiet_NaN();
                else if( !strcmp( jsonValue.GetString(), "inf" ) )
                    return std::numeric_limits<double>::infinity();
                else if( !strcmp( jsonValue.GetString(), "-inf" ) )
                    return -std::numeric_limits<double>::infinity();
            }
            else if( jsonValue.IsDouble() )
            {
                return jsonValue.GetDouble();
            }
        }

        return 0;
    }
    //-----------------------------------------------------------------------------------
    inline Vector2 SceneFormatImporter::decodeVector2Array( const rapidjson::Value &jsonArray )
    {
        Vector2 retVal( Vector2::ZERO );

        const rapidjson::SizeType arraySize = std::min( 2u, jsonArray.Size() );
        for( rapidjson::SizeType i=0; i<arraySize; ++i )
        {
            if( isFloat( jsonArray[i] ) )
                retVal[i] = decodeFloat( jsonArray[i] );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    inline Vector3 SceneFormatImporter::decodeVector3Array( const rapidjson::Value &jsonArray )
    {
        Vector3 retVal( Vector3::ZERO );

        const rapidjson::SizeType arraySize = std::min( 3u, jsonArray.Size() );
        for( rapidjson::SizeType i=0; i<arraySize; ++i )
        {
            if( isFloat( jsonArray[i] ) )
                retVal[i] = decodeFloat( jsonArray[i] );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    inline Vector4 SceneFormatImporter::decodeVector4Array( const rapidjson::Value &jsonArray )
    {
        Vector4 retVal( Vector4::ZERO );

        const rapidjson::SizeType arraySize = std::min( 4u, jsonArray.Size() );
        for( rapidjson::SizeType i=0; i<arraySize; ++i )
        {
            if( isFloat( jsonArray[i] ) )
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
            if( isFloat( jsonArray[i] ) )
                retVal[i] = decodeFloat( jsonArray[i] );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    inline ColourValue SceneFormatImporter::decodeColourValueArray( const rapidjson::Value &jsonArray )
    {
        ColourValue retVal( ColourValue::Black );

        const rapidjson::SizeType arraySize = std::min( 4u, jsonArray.Size() );
        for( rapidjson::SizeType i=0; i<arraySize; ++i )
        {
            if( isFloat( jsonArray[i] ) )
                retVal[i] = decodeFloat( jsonArray[i] );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    inline Aabb SceneFormatImporter::decodeAabbArray( const rapidjson::Value &jsonArray,
                                                      const Aabb &defaultValue )
    {
        Aabb retVal( defaultValue );

        if( jsonArray.Size() == 2u )
        {
            retVal.mCenter = decodeVector3Array( jsonArray[0] );
            retVal.mHalfSize = decodeVector3Array( jsonArray[1] );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    inline Matrix3 SceneFormatImporter::decodeMatrix3Array( const rapidjson::Value &jsonArray )
    {
        Matrix3 retVal( Matrix3::IDENTITY );

        const rapidjson::SizeType arraySize = std::min( 12u, jsonArray.Size() );
        for( rapidjson::SizeType i=0; i<arraySize; ++i )
        {
            if( isFloat( jsonArray[i] ) )
                retVal[0][i] = decodeFloat( jsonArray[i] );
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

        itor = nodeValue.FindMember( "name" );
        if( itor != nodeValue.MemberEnd() && itor->value.IsString() )
            node->setName( itor->value.GetString() );
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
                    if( parentIdx < sceneNodesJson.Size() &&
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
                bool isRootNode = false;
                itTmp = sceneNodeValue.FindMember( "is_root_node" );
                if( itTmp != sceneNodeValue.MemberEnd() && itTmp->value.IsBool() )
                    isRootNode = itTmp->value.GetBool();

                if( isRootNode )
                    sceneNode = mRootNodes[sceneNodeType];
                else
                {
                    if( mParentlessRootNodes[sceneNodeType] )
                        sceneNode = mParentlessRootNodes[sceneNodeType]->createChildSceneNode();
                    else
                        sceneNode = mSceneManager->createSceneNode( sceneNodeType );
                }
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
        rapidjson::Value::ConstValueIterator begin = json.Begin();
        rapidjson::Value::ConstValueIterator itor = begin;
        rapidjson::Value::ConstValueIterator end  = json.End();

        while( itor != end )
        {
            const size_t nodeIdx = itor - begin;
            if( itor->IsObject() &&
                mCreatedSceneNodes.find( nodeIdx ) == mCreatedSceneNodes.end() )
            {
                importSceneNode( *itor, nodeIdx, json );
            }

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::importMovableObject( const rapidjson::Value &movableObjectValue,
                                                   MovableObject *movableObject )
    {
        rapidjson::Value::ConstMemberIterator tmpIt;

        tmpIt = movableObjectValue.FindMember( "name" );
        if( tmpIt != movableObjectValue.MemberEnd() && tmpIt->value.IsString() )
            movableObject->setName( tmpIt->value.GetString() );

        tmpIt = movableObjectValue.FindMember( "parent_node_id" );
        if( tmpIt != movableObjectValue.MemberEnd() && tmpIt->value.IsUint() )
        {
            uint32 nodeId = tmpIt->value.GetUint();
            IndexToSceneNodeMap::const_iterator itNode = mCreatedSceneNodes.find( nodeId );
            if( itNode != mCreatedSceneNodes.end() )
                itNode->second->attachObject( movableObject );
            else
            {
                LogManager::getSingleton().logMessage( "WARNING: MovableObject references SceneNode " +
                                                       StringConverter::toString( nodeId ) +
                                                       " which does not exist or couldn't be created" );
            }
        }

        tmpIt = movableObjectValue.FindMember( "render_queue" );
        if( tmpIt != movableObjectValue.MemberEnd() && tmpIt->value.IsUint() )
        {
            uint32 rqId = tmpIt->value.GetUint();
            movableObject->setRenderQueueGroup( rqId );
        }

        tmpIt = movableObjectValue.FindMember( "local_aabb" );
        if( tmpIt != movableObjectValue.MemberEnd() && tmpIt->value.IsArray() )
        {
            movableObject->setLocalAabb( decodeAabbArray( tmpIt->value,
                                                          movableObject->getLocalAabb() ) );
        }

        ObjectData &objData = movableObject->_getObjectData();

        tmpIt = movableObjectValue.FindMember( "local_radius" );
        if( tmpIt != movableObjectValue.MemberEnd() && isFloat( tmpIt->value ) )
            objData.mLocalRadius[objData.mIndex] = decodeFloat( tmpIt->value );

        tmpIt = movableObjectValue.FindMember( "rendering_distance" );
        if( tmpIt != movableObjectValue.MemberEnd() && isFloat( tmpIt->value ) )
            movableObject->setRenderingDistance( decodeFloat( tmpIt->value ) );

        //Decode raw flag values
        tmpIt = movableObjectValue.FindMember( "visibility_flags" );
        if( tmpIt != movableObjectValue.MemberEnd() && tmpIt->value.IsUint() )
            objData.mVisibilityFlags[objData.mIndex] = tmpIt->value.GetUint();
        tmpIt = movableObjectValue.FindMember( "query_flags" );
        if( tmpIt != movableObjectValue.MemberEnd() && tmpIt->value.IsUint() )
            objData.mQueryFlags[objData.mIndex] = tmpIt->value.GetUint();
        tmpIt = movableObjectValue.FindMember( "light_mask" );
        if( tmpIt != movableObjectValue.MemberEnd() && tmpIt->value.IsUint() )
            objData.mLightMask[objData.mIndex] = tmpIt->value.GetUint();
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::importRenderable( const rapidjson::Value &renderableValue,
                                                Renderable *renderable )
    {
        rapidjson::Value::ConstMemberIterator tmpIt;

        tmpIt = renderableValue.FindMember( "custom_parameters" );
        if( tmpIt != renderableValue.MemberEnd() && tmpIt->value.IsObject() )
        {
            rapidjson::Value::ConstMemberIterator itor = tmpIt->value.MemberBegin();
            rapidjson::Value::ConstMemberIterator end  = tmpIt->value.MemberEnd();

            while( itor != end )
            {
                if( itor->name.IsUint() && itor->value.IsArray() )
                {
                    const uint32 idxCustomParam = itor->name.GetUint();
                    renderable->setCustomParameter( idxCustomParam, decodeVector4Array( itor->value ) );
                }

                ++itor;
            }
        }

        bool isV1Material = false;
        tmpIt = renderableValue.FindMember( "is_v1_material" );
        if( tmpIt != renderableValue.MemberEnd() && tmpIt->value.IsBool() )
            isV1Material = tmpIt->value.GetBool();

        tmpIt = renderableValue.FindMember( "datablock" );
        if( tmpIt != renderableValue.MemberEnd() && tmpIt->value.IsString() )
        {
            if( !isV1Material )
                renderable->setDatablock( tmpIt->value.GetString() );
            else
            {
                renderable->setDatablockOrMaterialName(
                            tmpIt->value.GetString(),
                            ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME );
            }
        }

        tmpIt = renderableValue.FindMember( "custom_parameter" );
        if( tmpIt != renderableValue.MemberEnd() && tmpIt->value.IsUint() )
            renderable->mCustomParameter = static_cast<uint8>( tmpIt->value.GetUint() );

        tmpIt = renderableValue.FindMember( "render_queue_sub_group" );
        if( tmpIt != renderableValue.MemberEnd() && tmpIt->value.IsUint() )
            renderable->setRenderQueueSubGroup( static_cast<uint8>( tmpIt->value.GetUint() ) );

        tmpIt = renderableValue.FindMember( "polygon_mode_overrideable" );
        if( tmpIt != renderableValue.MemberEnd() && tmpIt->value.IsBool() )
            renderable->setPolygonModeOverrideable( tmpIt->value.GetBool() );

        tmpIt = renderableValue.FindMember( "use_identity_view" );
        if( tmpIt != renderableValue.MemberEnd() && tmpIt->value.IsBool() )
            renderable->setUseIdentityView( tmpIt->value.GetBool() );

        tmpIt = renderableValue.FindMember( "use_identity_projection" );
        if( tmpIt != renderableValue.MemberEnd() && tmpIt->value.IsBool() )
            renderable->setUseIdentityProjection( tmpIt->value.GetBool() );
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::importSubItem( const rapidjson::Value &subentityValue, SubItem *subItem )
    {
        rapidjson::Value::ConstMemberIterator tmpIt;
        tmpIt = subentityValue.FindMember( "renderable" );
        if( tmpIt != subentityValue.MemberEnd() && tmpIt->value.IsObject() )
            importRenderable( tmpIt->value, subItem );
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::importSubEntity( const rapidjson::Value &subEntityValue,
                                               v1::SubEntity *subEntity )
    {
        rapidjson::Value::ConstMemberIterator tmpIt;
        tmpIt = subEntityValue.FindMember( "renderable" );
        if( tmpIt != subEntityValue.MemberEnd() && tmpIt->value.IsObject() )
            importRenderable( tmpIt->value, subEntity );
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::importItem( const rapidjson::Value &entityValue )
    {
        String meshName, resourceGroup;

        rapidjson::Value::ConstMemberIterator tmpIt;

        tmpIt = entityValue.FindMember( "mesh" );
        if( tmpIt != entityValue.MemberEnd() && tmpIt->value.IsString() )
            meshName = tmpIt->value.GetString();

        resourceGroup = "SceneFormatImporter";
//        tmpIt = entityValue.FindMember( "mesh_resource_group" );
//        if( tmpIt != entityValue.MemberEnd() && tmpIt->value.IsString() )
//            resourceGroup = tmpIt->value.GetString();

//        if( resourceGroup.empty() )
//            resourceGroup = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME;

        bool isStatic = false;
        rapidjson::Value const *movableObjectValue = 0;

        tmpIt = entityValue.FindMember( "movable_object" );
        if( tmpIt != entityValue.MemberEnd() && tmpIt->value.IsObject() )
        {
            movableObjectValue = &tmpIt->value;

            tmpIt = movableObjectValue->FindMember( "is_static" );
            if( tmpIt != movableObjectValue->MemberEnd() && tmpIt->value.IsBool() )
                isStatic = tmpIt->value.GetBool();
        }

        const SceneMemoryMgrTypes sceneNodeType = isStatic ? SCENE_STATIC : SCENE_DYNAMIC;

        Item *item = mSceneManager->createItem( meshName, resourceGroup, sceneNodeType );

        if( movableObjectValue )
            importMovableObject( *movableObjectValue, item );

        tmpIt = entityValue.FindMember( "sub_items" );
        if( tmpIt != entityValue.MemberEnd() && tmpIt->value.IsArray() )
        {
            const rapidjson::Value &subItemsArray = tmpIt->value;
            const size_t numSubItems = std::min<size_t>( item->getNumSubItems(),
                                                         subItemsArray.Size() );
            for( size_t i=0; i<numSubItems; ++i )
            {
                const rapidjson::Value &subentityValue = subItemsArray[i];

                if( subentityValue.IsObject() )
                    importSubItem( subentityValue, item->getSubItem( i ) );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::importItems( const rapidjson::Value &json )
    {
        rapidjson::Value::ConstValueIterator itor = json.Begin();
        rapidjson::Value::ConstValueIterator end  = json.End();

        while( itor != end )
        {
            if( itor->IsObject() )
                importItem( *itor );

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::importEntity( const rapidjson::Value &entityValue )
    {
        String meshName, resourceGroup;

        rapidjson::Value::ConstMemberIterator tmpIt;

        tmpIt = entityValue.FindMember( "mesh" );
        if( tmpIt != entityValue.MemberEnd() && tmpIt->value.IsString() )
            meshName = tmpIt->value.GetString();

        resourceGroup = "SceneFormatImporter";
//        tmpIt = entityValue.FindMember( "mesh_resource_group" );
//        if( tmpIt != entityValue.MemberEnd() && tmpIt->value.IsString() )
//            resourceGroup = tmpIt->value.GetString();

//        if( resourceGroup.empty() )
//            resourceGroup = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME;

        bool isStatic = false;
        rapidjson::Value const *movableObjectValue = 0;

        tmpIt = entityValue.FindMember( "movable_object" );
        if( tmpIt != entityValue.MemberEnd() && tmpIt->value.IsObject() )
        {
            movableObjectValue = &tmpIt->value;

            tmpIt = movableObjectValue->FindMember( "is_static" );
            if( tmpIt != movableObjectValue->MemberEnd() && tmpIt->value.IsBool() )
                isStatic = tmpIt->value.GetBool();
        }

        const SceneMemoryMgrTypes sceneNodeType = isStatic ? SCENE_STATIC : SCENE_DYNAMIC;

        v1::Entity *entity = mSceneManager->createEntity( meshName, resourceGroup, sceneNodeType );

        if( movableObjectValue )
            importMovableObject( *movableObjectValue, entity );

        tmpIt = entityValue.FindMember( "sub_entities" );
        if( tmpIt != entityValue.MemberEnd() && tmpIt->value.IsArray() )
        {
            const rapidjson::Value &subEntitiesArray = tmpIt->value;
            const size_t numSubEntities = std::min<size_t>( entity->getNumSubEntities(),
                                                            subEntitiesArray.Size() );
            for( size_t i=0; i<numSubEntities; ++i )
            {
                const rapidjson::Value &subEntityValue = subEntitiesArray[i];

                if( subEntityValue.IsObject() )
                    importSubEntity( subEntityValue, entity->getSubEntity( i ) );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::importEntities( const rapidjson::Value &json )
    {
        rapidjson::Value::ConstValueIterator itor = json.Begin();
        rapidjson::Value::ConstValueIterator end  = json.End();

        while( itor != end )
        {
            if( itor->IsObject() )
                importEntity( *itor );

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::importLight( const rapidjson::Value &lightValue )
    {
        rapidjson::Value::ConstMemberIterator tmpIt;

        Light *light = mSceneManager->createLight();

        tmpIt = lightValue.FindMember( "movable_object" );
        if( tmpIt != lightValue.MemberEnd() && tmpIt->value.IsObject() )
        {
            const rapidjson::Value &movableObjectValue = tmpIt->value;
            importMovableObject( movableObjectValue, light );
        }

        tmpIt = lightValue.FindMember( "diffuse" );
        if( tmpIt != lightValue.MemberEnd() && tmpIt->value.IsArray() )
            light->setDiffuseColour( decodeColourValueArray( tmpIt->value ) );

        tmpIt = lightValue.FindMember( "specular" );
        if( tmpIt != lightValue.MemberEnd() && tmpIt->value.IsArray() )
            light->setSpecularColour( decodeColourValueArray( tmpIt->value ) );

        tmpIt = lightValue.FindMember( "power" );
        if( tmpIt != lightValue.MemberEnd() && isFloat( tmpIt->value ) )
            light->setPowerScale( decodeFloat( tmpIt->value ) );

        tmpIt = lightValue.FindMember( "type" );
        if( tmpIt != lightValue.MemberEnd() && tmpIt->value.IsString() )
            light->setType( parseLightType( tmpIt->value.GetString() ) );

        tmpIt = lightValue.FindMember( "attenuation" );
        if( tmpIt != lightValue.MemberEnd() && tmpIt->value.IsArray() )
        {
            const Vector4 rangeConstLinQuad = decodeVector4Array( tmpIt->value );
            light->setAttenuation( rangeConstLinQuad.x, rangeConstLinQuad.y,
                                   rangeConstLinQuad.z, rangeConstLinQuad.w );
        }

        tmpIt = lightValue.FindMember( "spot" );
        if( tmpIt != lightValue.MemberEnd() && tmpIt->value.IsArray() )
        {
            const Vector4 innerOuterFalloffNearClip = decodeVector4Array( tmpIt->value );
            light->setSpotlightInnerAngle( Radian( innerOuterFalloffNearClip.x ) );
            light->setSpotlightOuterAngle( Radian( innerOuterFalloffNearClip.y ) );
            light->setSpotlightFalloff( innerOuterFalloffNearClip.z );
            light->setSpotlightNearClipDistance( innerOuterFalloffNearClip.w );
        }

        tmpIt = lightValue.FindMember( "affect_parent_node" );
        if( tmpIt != lightValue.MemberEnd() && tmpIt->value.IsBool() )
            light->setAffectParentNode( tmpIt->value.GetBool() );

        tmpIt = lightValue.FindMember( "shadow_far_dist" );
        if( tmpIt != lightValue.MemberEnd() && isFloat( tmpIt->value ) )
            light->setShadowFarDistance( decodeFloat( tmpIt->value ) );

        tmpIt = lightValue.FindMember( "shadow_clip_dist" );
        if( tmpIt != lightValue.MemberEnd() && isFloat( tmpIt->value ) )
        {
            const Vector2 nearFar = decodeVector2Array( tmpIt->value );
            light->setShadowNearClipDistance( nearFar.x );
            light->setShadowFarClipDistance( nearFar.y );
        }

        tmpIt = lightValue.FindMember( "rect_size" );
        if( tmpIt != lightValue.MemberEnd() && tmpIt->value.IsArray() )
            light->setRectSize( decodeVector2Array( tmpIt->value ) );

        tmpIt = lightValue.FindMember( "texture_light_mask_idx" );
        if( tmpIt != lightValue.MemberEnd() && tmpIt->value.IsUint() )
            light->mTextureLightMaskIdx = static_cast<uint16>( tmpIt->value.GetUint() );

        if( light->getType() == Light::LT_VPL )
            mVplLights.push_back( light );
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::importLights( const rapidjson::Value &json )
    {
        rapidjson::Value::ConstValueIterator itor = json.Begin();
        rapidjson::Value::ConstValueIterator end  = json.End();

        while( itor != end )
        {
            if( itor->IsObject() )
                importLight( *itor );

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::importInstantRadiosity( const rapidjson::Value &json )
    {
        mInstantRadiosity = new InstantRadiosity( mSceneManager, mRoot->getHlmsManager() );

        rapidjson::Value::ConstMemberIterator tmpIt;
        tmpIt = json.FindMember( "first_rq" );
        if( tmpIt != json.MemberEnd() && tmpIt->value.IsUint() )
            mInstantRadiosity->mFirstRq = static_cast<uint8>( tmpIt->value.GetUint() );

        tmpIt = json.FindMember( "last_rq" );
        if( tmpIt != json.MemberEnd() && tmpIt->value.IsUint() )
            mInstantRadiosity->mLastRq = static_cast<uint8>( tmpIt->value.GetUint() );

        tmpIt = json.FindMember( "visibility_mask" );
        if( tmpIt != json.MemberEnd() && tmpIt->value.IsUint() )
            mInstantRadiosity->mVisibilityMask = static_cast<uint32>( tmpIt->value.GetUint() );

        tmpIt = json.FindMember( "light_mask" );
        if( tmpIt != json.MemberEnd() && tmpIt->value.IsUint() )
            mInstantRadiosity->mLightMask = static_cast<uint32>( tmpIt->value.GetUint() );

        tmpIt = json.FindMember( "num_rays" );
        if( tmpIt != json.MemberEnd() && tmpIt->value.IsUint() )
            mInstantRadiosity->mNumRays = static_cast<size_t>( tmpIt->value.GetUint() );

        tmpIt = json.FindMember( "num_ray_bounces" );
        if( tmpIt != json.MemberEnd() && tmpIt->value.IsUint() )
            mInstantRadiosity->mNumRayBounces = static_cast<size_t>( tmpIt->value.GetUint() );

        tmpIt = json.FindMember( "surviving_ray_fraction" );
        if( tmpIt != json.MemberEnd() && isFloat( tmpIt->value ) )
            mInstantRadiosity->mSurvivingRayFraction = decodeFloat( tmpIt->value );

        tmpIt = json.FindMember( "cell_size" );
        if( tmpIt != json.MemberEnd() && isFloat( tmpIt->value ) )
            mInstantRadiosity->mCellSize = decodeFloat( tmpIt->value );

        tmpIt = json.FindMember( "bias" );
        if( tmpIt != json.MemberEnd() && isFloat( tmpIt->value ) )
            mInstantRadiosity->mBias = decodeFloat( tmpIt->value );

        tmpIt = json.FindMember( "num_spread_iterations" );
        if( tmpIt != json.MemberEnd() && tmpIt->value.IsUint() )
            mInstantRadiosity->mNumSpreadIterations = static_cast<uint32>( tmpIt->value.GetUint() );

        tmpIt = json.FindMember( "spread_threshold" );
        if( tmpIt != json.MemberEnd() && isFloat( tmpIt->value ) )
            mInstantRadiosity->mSpreadThreshold = decodeFloat( tmpIt->value );

        tmpIt = json.FindMember( "areas_of_interest" );
        if( tmpIt != json.MemberEnd() && tmpIt->value.IsArray() )
        {
            const size_t numAoIs = tmpIt->value.Size();

            for( size_t i=0; i<numAoIs; ++i )
            {
                const rapidjson::Value &aoi = tmpIt->value[i];

                if( aoi.IsArray() && aoi.Size() == 2u &&
                    aoi[0].IsArray() &&
                    isFloat( aoi[1] ) )
                {
                    Aabb aabb = decodeAabbArray( aoi[0], Aabb::BOX_ZERO );
                    const float sphereRadius = decodeFloat( aoi[1] );
                    aabb.transformAffine( mSceneComponentTransform );
                    InstantRadiosity::AreaOfInterest areaOfInterest( aabb, sphereRadius );
                    mInstantRadiosity->mAoI.push_back( areaOfInterest );
                }
            }
        }

        tmpIt = json.FindMember( "vpl_max_range" );
        if( tmpIt != json.MemberEnd() && isFloat( tmpIt->value ) )
            mInstantRadiosity->mVplMaxRange = decodeFloat( tmpIt->value );

        tmpIt = json.FindMember( "vpl_const_atten" );
        if( tmpIt != json.MemberEnd() && isFloat( tmpIt->value ) )
            mInstantRadiosity->mVplConstAtten = decodeFloat( tmpIt->value );

        tmpIt = json.FindMember( "vpl_linear_atten" );
        if( tmpIt != json.MemberEnd() && isFloat( tmpIt->value ) )
            mInstantRadiosity->mVplLinearAtten = decodeFloat( tmpIt->value );

        tmpIt = json.FindMember( "vpl_quad_atten" );
        if( tmpIt != json.MemberEnd() && isFloat( tmpIt->value ) )
            mInstantRadiosity->mVplQuadAtten = decodeFloat( tmpIt->value );

        tmpIt = json.FindMember( "vpl_threshold" );
        if( tmpIt != json.MemberEnd() && isFloat( tmpIt->value ) )
            mInstantRadiosity->mVplThreshold = decodeFloat( tmpIt->value );

        tmpIt = json.FindMember( "vpl_power_boost" );
        if( tmpIt != json.MemberEnd() && isFloat( tmpIt->value ) )
            mInstantRadiosity->mVplPowerBoost = decodeFloat( tmpIt->value );

        tmpIt = json.FindMember( "vpl_use_intensity_for_max_range" );
        if( tmpIt != json.MemberEnd() && tmpIt->value.IsBool() )
            mInstantRadiosity->mVplUseIntensityForMaxRange = tmpIt->value.GetBool();

        tmpIt = json.FindMember( "vpl_intensity_range_multiplier" );
        if( tmpIt != json.MemberEnd() && isDouble( tmpIt->value ) )
            mInstantRadiosity->mVplIntensityRangeMultiplier = decodeDouble( tmpIt->value );

        tmpIt = json.FindMember( "mipmap_bias" );
        if( tmpIt != json.MemberEnd() && tmpIt->value.IsUint() )
            mInstantRadiosity->mMipmapBias = static_cast<uint32>( tmpIt->value.GetUint() );

        tmpIt = json.FindMember( "use_textures" );
        if( tmpIt != json.MemberEnd() && tmpIt->value.IsBool() )
            mInstantRadiosity->setUseTextures( tmpIt->value.GetBool() );

        tmpIt = json.FindMember( "use_irradiance_volume" );
        if( tmpIt != json.MemberEnd() && tmpIt->value.IsBool() )
            mInstantRadiosity->setUseIrradianceVolume( tmpIt->value.GetBool() );

        tmpIt = json.FindMember( "irradiance_volume" );
        if( tmpIt != json.MemberEnd() && tmpIt->value.IsObject() )
        {
            mIrradianceVolume = new IrradianceVolume( mRoot->getHlmsManager() );

            tmpIt = json.FindMember( "num_blocks" );
            if( tmpIt != json.MemberEnd() && tmpIt->value.IsArray() &&
                tmpIt->value.Size() == 3u &&
                tmpIt->value[0].IsUint() &&
                tmpIt->value[1].IsUint() &&
                tmpIt->value[2].IsUint() )
            {
                mIrradianceVolume->createIrradianceVolumeTexture(
                            tmpIt->value[0].GetUint(),
                            tmpIt->value[1].GetUint(),
                            tmpIt->value[2].GetUint() );
            }

            tmpIt = json.FindMember( "power_scale" );
            if( tmpIt != json.MemberEnd() && isFloat( tmpIt->value ) )
                mIrradianceVolume->setPowerScale( decodeFloat( tmpIt->value ) );

            tmpIt = json.FindMember( "fade_attenuation_over_distance" );
            if( tmpIt != json.MemberEnd() && tmpIt->value.IsBool() )
                mIrradianceVolume->setFadeAttenuationOverDistace( tmpIt->value.GetBool() );

            tmpIt = json.FindMember( "irradiance_max_power" );
            if( tmpIt != json.MemberEnd() && isFloat( tmpIt->value ) )
                mIrradianceVolume->setIrradianceMaxPower( decodeFloat( tmpIt->value ) );

            tmpIt = json.FindMember( "irradiance_origin" );
            if( tmpIt != json.MemberEnd() && tmpIt->value.IsArray() )
                mIrradianceVolume->setIrradianceOrigin( decodeVector3Array( tmpIt->value ) );

            tmpIt = json.FindMember( "irradiance_cell_size" );
            if( tmpIt != json.MemberEnd() && tmpIt->value.IsArray() )
                mIrradianceVolume->setIrradianceCellSize( decodeVector3Array( tmpIt->value ) );
        }
        else
        {
            mInstantRadiosity->setUseIrradianceVolume( false );
        }
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::importDecal( const rapidjson::Value &decalValue )
    {
        rapidjson::Value::ConstMemberIterator tmpIt;

        Decal *decal = mSceneManager->createDecal();

        tmpIt = decalValue.FindMember( "movable_object" );
        if( tmpIt != decalValue.MemberEnd() && tmpIt->value.IsObject() )
        {
            const rapidjson::Value &movableObjectValue = tmpIt->value;
            importMovableObject( movableObjectValue, decal );
        }

        String additionalExtension;
        if( mUsingOitd )
            additionalExtension = ".oitd";

        DecalTex decalTex[3] =
        {
            DecalTex( TexturePtr(), 0, "diffuse" ),
            DecalTex( TexturePtr(), 0, "normal" ),
            DecalTex( TexturePtr(), 0, "emissive" ),
        };

        char tmpBuffer[32];
        LwString texName( LwString::FromEmptyPointer( tmpBuffer, sizeof(tmpBuffer) ) );
        HlmsManager *hlmsManager = mRoot->getHlmsManager();
        HlmsTextureManager *hlmsTextureManager = hlmsManager->getTextureManager();

        for( int i=0; i<3; ++i )
        {
            texName.clear();
            texName.a( decalTex[i].texTypeName, "_managed" );
            tmpIt = decalValue.FindMember( texName.c_str() );
            if( tmpIt != decalValue.MemberEnd() && tmpIt->value.IsArray() &&
                tmpIt->value.Size() == 3u &&
                tmpIt->value[0].IsString() && tmpIt->value[1].IsString() && tmpIt->value[2].IsUint() )
            {
                const char *aliasName = tmpIt->value[0].GetString();
                //Real texture name is lost in 2.1. Needs 2.2
                //const char *textureName = tmpIt->value[1].GetString();
                const uint32 poolId = tmpIt->value[2].GetUint();

                HlmsTextureManager::TextureLocation texLocation =
                        hlmsTextureManager->createOrRetrieveTexture(
                            //aliasName, textureName + additionalExtension,
                            aliasName, aliasName + additionalExtension,
                            i != 1 ? HlmsTextureManager::TEXTURE_TYPE_DIFFUSE :
                                     HlmsTextureManager::TEXTURE_TYPE_NORMALS, poolId );
                decalTex[i].texture = texLocation.texture;
                decalTex[i].xIdx    = texLocation.xIdx;
            }

            texName.clear();
            texName.a( decalTex[i].texTypeName, "_raw" );
            tmpIt = decalValue.FindMember( texName.c_str() );
            if( tmpIt != decalValue.MemberEnd() && tmpIt->value.IsArray() &&
                tmpIt->value.Size() == 2u &&
                tmpIt->value[0].IsString() && tmpIt->value[1].IsUint() )
            {
                const char *textureName = tmpIt->value[0].GetString();
                const uint32 arrayIdx = tmpIt->value[1].GetUint();

                TexturePtr texture =
                        TextureManager::getSingleton().load( textureName, "SceneFormatImporter",
                                                             TEX_TYPE_2D_ARRAY, MIP_DEFAULT, 1.0f,
                                                             false, PF_UNKNOWN, i != 1 );
                decalTex[i].texture = texture;
                decalTex[i].xIdx    = static_cast<uint16>( arrayIdx );
            }
        }

        if( decalTex[0].texture )
            decal->setDiffuseTexture( decalTex[0].texture, decalTex[0].xIdx );
        if( decalTex[1].texture )
            decal->setNormalTexture( decalTex[1].texture, decalTex[1].xIdx );
        if( decalTex[2].texture )
            decal->setEmissiveTexture( decalTex[2].texture, decalTex[2].xIdx );
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::importDecals( const rapidjson::Value &json )
    {
        rapidjson::Value::ConstValueIterator itor = json.Begin();
        rapidjson::Value::ConstValueIterator end  = json.End();

        while( itor != end )
        {
            if( itor->IsObject() )
                importDecal( *itor );

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::importPcc( const rapidjson::Value &pccValue )
    {
        uint8 reservedRqId = 250;
        uint32 reservedProxyMask = 1u << 25u;

        rapidjson::Value::ConstMemberIterator tmpIt;
        tmpIt = pccValue.FindMember( "reserved_rq_id" );
        if( tmpIt != pccValue.MemberEnd() && tmpIt->value.IsUint() )
            reservedRqId = static_cast<uint8>( tmpIt->value.GetUint() );

        tmpIt = pccValue.FindMember( "proxy_visibility_mask" );
        if( tmpIt != pccValue.MemberEnd() && tmpIt->value.IsUint() )
            reservedProxyMask = tmpIt->value.GetUint();

        Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();

        String workspaceName = mDefaultPccWorkspaceName;
        tmpIt = pccValue.FindMember( "workspace" );
        if( tmpIt != pccValue.MemberEnd() && tmpIt->value.IsString() )
        {
            workspaceName = tmpIt->value.GetString();
            if( !compositorManager->hasWorkspaceDefinition( workspaceName ) )
            {
                LogManager::getSingleton().logMessage(
                            "INFO: Parallax Corrected Cubemaps workspace definition '" +
                            workspaceName + "not found, using default one." );
                workspaceName = mDefaultPccWorkspaceName;
            }
        }

        if( workspaceName.empty() )
        {
            LogManager::getSingleton().logMessage(
                        "WARNING: Cannot import Parallax Corrected Cubemaps." );
            return;
        }

        Ogre::CompositorWorkspaceDef *workspaceDef =
                compositorManager->getWorkspaceDefinition( workspaceName );

        mParallaxCorrectedCubemap = new ParallaxCorrectedCubemap(
                                        Ogre::Id::generateNewId<Ogre::ParallaxCorrectedCubemap>(),
                                        mRoot, mSceneManager, workspaceDef,
                                        reservedRqId, reservedProxyMask );

        uint32 maxWidth = 0, maxHeight = 0;
        PixelFormat blendPixelFormat = PF_UNKNOWN;
        tmpIt = pccValue.FindMember( "max_width" );
        if( tmpIt != pccValue.MemberEnd() && tmpIt->value.IsUint() )
            maxWidth = tmpIt->value.GetUint();
        tmpIt = pccValue.FindMember( "max_height" );
        if( tmpIt != pccValue.MemberEnd() && tmpIt->value.IsUint() )
            maxHeight = tmpIt->value.GetUint();
        tmpIt = pccValue.FindMember( "pixel_format" );
        if( tmpIt != pccValue.MemberEnd() && tmpIt->value.IsString() )
            blendPixelFormat = PixelUtil::getFormatFromName( tmpIt->value.GetString(), false, true );

        if( maxWidth != 0 && maxHeight != 0 && blendPixelFormat != PF_UNKNOWN )
            mParallaxCorrectedCubemap->setEnabled( true, maxWidth, maxHeight, blendPixelFormat );

        tmpIt = pccValue.FindMember( "paused" );
        if( tmpIt != pccValue.MemberEnd() && tmpIt->value.IsBool() )
            mParallaxCorrectedCubemap->mPaused = tmpIt->value.GetBool();

        tmpIt = pccValue.FindMember( "mask" );
        if( tmpIt != pccValue.MemberEnd() && tmpIt->value.IsUint() )
            mParallaxCorrectedCubemap->mMask = tmpIt->value.GetUint();
        tmpIt = pccValue.FindMember( "probes" );
        if( tmpIt != pccValue.MemberEnd() && tmpIt->value.IsArray() )
        {
            const rapidjson::Value &jsonProbeArray = tmpIt->value;
            const size_t numProbes = jsonProbeArray.Size();

            for( size_t i=0; i<numProbes; ++i )
            {
                const rapidjson::Value &jsonProbe = jsonProbeArray[i];

                if( !jsonProbe.IsObject() )
                    continue;

                CubemapProbe *probe = mParallaxCorrectedCubemap->createProbe();

                uint32 width = 0, height = 0;
                PixelFormat pixelFormat = PF_UNKNOWN;
                uint8 msaa = 0;
                bool useManual = true;
                bool isStatic = false;

                tmpIt = jsonProbe.FindMember( "width" );
                if( tmpIt != jsonProbe.MemberEnd() && tmpIt->value.IsUint() )
                    width = tmpIt->value.GetUint();
                tmpIt = jsonProbe.FindMember( "height" );
                if( tmpIt != jsonProbe.MemberEnd() && tmpIt->value.IsUint() )
                    height = tmpIt->value.GetUint();
                tmpIt = jsonProbe.FindMember( "msaa" );
                if( tmpIt != jsonProbe.MemberEnd() && tmpIt->value.IsUint() )
                    msaa = static_cast<uint8>( tmpIt->value.GetUint() );
                tmpIt = jsonProbe.FindMember( "pixel_format" );
                if( tmpIt != jsonProbe.MemberEnd() && tmpIt->value.IsString() )
                    pixelFormat = PixelUtil::getFormatFromName( tmpIt->value.GetString(), false, true );
                tmpIt = jsonProbe.FindMember( "use_manual" );
                if( tmpIt != jsonProbe.MemberEnd() && tmpIt->value.IsBool() )
                    useManual = tmpIt->value.GetBool();
                tmpIt = jsonProbe.FindMember( "static" );
                if( tmpIt != jsonProbe.MemberEnd() && tmpIt->value.IsBool() )
                    isStatic = tmpIt->value.GetBool();

                if( width != 0 && height != 0 && pixelFormat != PF_UNKNOWN )
                {
                    probe->setTextureParams( width, height, useManual, pixelFormat, isStatic, msaa );
                    probe->initWorkspace();
                }

                Aabb probeArea, probeShape;
                Vector3 cameraPos( Vector3::ZERO );
                Vector3 areaInnerRegion( Vector3::UNIT_SCALE );
                Matrix3 orientation( Matrix3::IDENTITY );

                tmpIt = jsonProbe.FindMember( "camera_pos" );
                if( tmpIt != jsonProbe.MemberEnd() && tmpIt->value.IsArray() )
                    cameraPos = decodeVector3Array( tmpIt->value );

                tmpIt = jsonProbe.FindMember( "area" );
                if( tmpIt != jsonProbe.MemberEnd() && tmpIt->value.IsArray() )
                    probeArea = decodeAabbArray( tmpIt->value, Aabb::BOX_ZERO );

                tmpIt = jsonProbe.FindMember( "area_inner_region" );
                if( tmpIt != jsonProbe.MemberEnd() && tmpIt->value.IsArray() )
                    areaInnerRegion = decodeVector3Array( tmpIt->value );

                tmpIt = jsonProbe.FindMember( "orientation" );
                if( tmpIt != jsonProbe.MemberEnd() && tmpIt->value.IsArray() )
                    orientation = decodeMatrix3Array( tmpIt->value );

                tmpIt = jsonProbe.FindMember( "probe_shape" );
                if( tmpIt != jsonProbe.MemberEnd() && tmpIt->value.IsArray() )
                    probeShape = decodeAabbArray( tmpIt->value, Aabb::BOX_ZERO );

                cameraPos = mSceneComponentTransform * cameraPos;
                probeArea.transformAffine( mSceneComponentTransform );
                areaInnerRegion = mSceneComponentTransform * areaInnerRegion;
                //orientation = pccTransform3x3 * orientation;
                probeShape.transformAffine( mSceneComponentTransform );

                probe->set( cameraPos, probeArea, areaInnerRegion, orientation, probeShape );

                tmpIt = jsonProbe.FindMember( "enabled" );
                if( tmpIt != jsonProbe.MemberEnd() && tmpIt->value.IsBool() )
                    probe->mEnabled = tmpIt->value.GetBool();

                tmpIt = jsonProbe.FindMember( "num_iterations" );
                if( tmpIt != jsonProbe.MemberEnd() && tmpIt->value.IsUint() )
                    probe->mNumIterations = tmpIt->value.GetUint();

                tmpIt = jsonProbe.FindMember( "mask" );
                if( tmpIt != jsonProbe.MemberEnd() && tmpIt->value.IsUint() )
                    probe->mMask = tmpIt->value.GetUint();
            }
        }

        HlmsPbs *hlmsPbs = getPbs();
        if( hlmsPbs )
            hlmsPbs->setParallaxCorrectedCubemap( mParallaxCorrectedCubemap );
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::importSceneSettings( const rapidjson::Value &json, uint32 importFlags )
    {
        rapidjson::Value::ConstMemberIterator tmpIt;
        tmpIt = json.FindMember( "ambient" );
        if( tmpIt != json.MemberEnd() && tmpIt->value.IsArray() && tmpIt->value.Size() >= 4u &&
            tmpIt->value[0].IsArray() &&
            tmpIt->value[1].IsArray() &&
            tmpIt->value[2].IsArray() &&
            isFloat( tmpIt->value[3] ) )
        {
            const ColourValue upperHemisphere = decodeColourValueArray( tmpIt->value[0] );
            const ColourValue lowerHemisphere = decodeColourValueArray( tmpIt->value[1] );
            const Vector3 hemiDir = decodeVector3Array( tmpIt->value[2] );
            const float envmapScale = decodeFloat( tmpIt->value[3] );
            mSceneManager->setAmbientLight( upperHemisphere, lowerHemisphere, hemiDir, envmapScale );
        }

        if( importFlags & SceneFlags::InstantRadiosity )
        {
            tmpIt = json.FindMember( "instant_radiosity" );
            if( tmpIt != json.MemberEnd() && tmpIt->value.IsObject() )
                importInstantRadiosity( tmpIt->value );
        }

        if( importFlags & SceneFlags::ParallaxCorrectedCubemap )
        {
            tmpIt = json.FindMember( "parallax_corrected_cubemaps" );
            if( tmpIt != json.MemberEnd() && tmpIt->value.IsObject() )
                importPcc( tmpIt->value );
        }

        if( importFlags & SceneFlags::AreaLightMasks )
        {
            tmpIt = json.FindMember( "area_light_masks" );
            if( tmpIt != json.MemberEnd() && tmpIt->value.IsString() )
            {
                TexturePtr areaLightMask = TextureManager::getSingleton().load(
                                               String( tmpIt->value.GetString() ) + ".oitd",
                                               "SceneFormatImporter", TEX_TYPE_2D_ARRAY );
                HlmsPbs *hlmsPbs = getPbs();
                hlmsPbs->setAreaLightMasks( areaLightMask );
            }
        }

        if( importFlags & SceneFlags::Decals )
        {
            HlmsManager *hlmsManager = mRoot->getHlmsManager();
            HlmsTextureManager *hlmsTextureManager = hlmsManager->getTextureManager();

            char tmpBuffer[32];
            LwString keyName( LwString::FromEmptyPointer( tmpBuffer, sizeof( tmpBuffer ) ) );
            const char *texTypes[3] = { "diffuse", "normals", "emissive" };
            TexturePtr textures[3];

            for( int i=0; i<3; ++i )
            {
                keyName.clear();
                keyName.a( "decals_", texTypes[i],"_managed" );
                tmpIt = json.FindMember( keyName.c_str() );
                if( tmpIt != json.MemberEnd() && tmpIt->value.IsString() )
                {
                    //TextureType shouldn't matter because that alias should've been loaded by now
                    HlmsTextureManager::TextureLocation texLocation =
                            hlmsTextureManager->createOrRetrieveTexture(
                                tmpIt->value.GetString(), HlmsTextureManager::TEXTURE_TYPE_DIFFUSE );
                    textures[i] = texLocation.texture;
                }

                keyName.clear();
                keyName.a( "decals_", texTypes[i],"_raw" );
                tmpIt = json.FindMember( keyName.c_str() );
                if( tmpIt != json.MemberEnd() && tmpIt->value.IsString() )
                {
                    textures[i] = TextureManager::getSingleton().load(
                                      tmpIt->value.GetString(),
                                      "SceneFormatImporter", TEX_TYPE_2D_ARRAY );
                }
            }

            mSceneManager->setDecalsDiffuse( textures[0] );
            mSceneManager->setDecalsNormals( textures[1] );
            mSceneManager->setDecalsEmissive( textures[2] );
        }
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::importScene( const String &filename, const rapidjson::Document &d,
                                           uint32 importFlags )
    {
        mUseBinaryFloatingPoint = true; //The default when setting is not present

        mFilename = filename;
        destroyInstantRadiosity();
        destroyParallaxCorrectedCubemap();

        //Set null pointers to valid root scene nodes. We'll restore the nullptrs at the end.
        SceneNode *oldRootNodes[NUM_SCENE_MEMORY_MANAGER_TYPES];
        for( size_t i=0; i<NUM_SCENE_MEMORY_MANAGER_TYPES; ++i )
        {
            oldRootNodes[i] = mRootNodes[i];
            if( !mRootNodes[i] )
                mRootNodes[i] = mSceneManager->getRootSceneNode( static_cast<SceneMemoryMgrTypes>(i) );
        }

        rapidjson::Value::ConstMemberIterator itor;

        itor = d.FindMember( "version" );
        if( itor == d.MemberEnd() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "SceneFormatImporter::importScene",
                         "JSON file " + filename + " does not contain version key. "
                         "Probably this is not a valid Ogre scene" );
        }
        else
        {
            if( itor->value.IsUint() )
            {
                const uint32 version = itor->value.GetUint();
                if( version > LATEST_VERSION )
                {
                    LogManager::getSingleton().logMessage(
                                "WARNING: SceneFormatImporter::importScene "
                                "JSON file " + filename + " is a newer version(" +
                                StringConverter::toString( version ) +") than what we support (" +
                                StringConverter::toString( LATEST_VERSION ) + "). "
                                "Imported scene may not be complete or have graphical corruption. "
                                "Or crash.", LML_CRITICAL );
                }
            }
        }

        itor = d.FindMember( "use_binary_floating_point" );
        if( itor != d.MemberEnd() && itor->value.IsBool() )
            mUseBinaryFloatingPoint = itor->value.GetBool();

        if( importFlags & SceneFlags::SceneNodes )
        {
            itor = d.FindMember( "scene_nodes" );
            if( itor != d.MemberEnd() && itor->value.IsArray() )
                importSceneNodes( itor->value );
        }

        if( importFlags & SceneFlags::Items )
        {
            itor = d.FindMember( "items" );
            if( itor != d.MemberEnd() && itor->value.IsArray() )
                importItems( itor->value );
        }

        if( importFlags & SceneFlags::Entities )
        {
            itor = d.FindMember( "entities" );
            if( itor != d.MemberEnd() && itor->value.IsArray() )
                importEntities( itor->value );
        }

        if( importFlags & SceneFlags::Lights )
        {
            itor = d.FindMember( "lights" );
            if( itor != d.MemberEnd() && itor->value.IsArray() )
                importLights( itor->value );
        }

        if( importFlags & SceneFlags::Decals )
        {
            itor = d.FindMember( "decals" );
            if( itor != d.MemberEnd() && itor->value.IsArray() )
                importDecals( itor->value );
        }

        itor = d.FindMember( "scene" );
        if( itor != d.MemberEnd() && itor->value.IsObject() )
            importSceneSettings( itor->value, importFlags );

        if( !(importFlags & SceneFlags::LightsVpl) )
        {
            LightArray::const_iterator itLight = mVplLights.begin();
            LightArray::const_iterator enLight = mVplLights.end();

            while( itLight != enLight )
            {
                Light *vplLight = *itLight;
                SceneNode *sceneNode = vplLight->getParentSceneNode();
                mSceneManager->destroySceneNode( sceneNode );
                mSceneManager->destroyLight( vplLight );
                ++itLight;
            }

            mVplLights.clear();
        }

        if( mInstantRadiosity && importFlags & SceneFlags::BuildInstantRadiosity )
        {
            mInstantRadiosity->build();

            HlmsPbs *hlmsPbs = getPbs();
            if( hlmsPbs && mInstantRadiosity->getUseIrradianceVolume() )
                hlmsPbs->setIrradianceVolume( mIrradianceVolume );

            if( mIrradianceVolume )
            {
                mInstantRadiosity->fillIrradianceVolume(
                            mIrradianceVolume,
                            mIrradianceVolume->getIrradianceCellSize(),
                            mIrradianceVolume->getIrradianceOrigin(),
                            mIrradianceVolume->getIrradianceMaxPower(),
                            mIrradianceVolume->getFadeAttenuationOverDistace() );
            }
        }

        for( size_t i=0; i<NUM_SCENE_MEMORY_MANAGER_TYPES; ++i )
            mRootNodes[i] = oldRootNodes[i];
    }

    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::setRootNodes( SceneNode *dynamicRoot, SceneNode *staticRoot )
    {
        mRootNodes[SCENE_DYNAMIC] = dynamicRoot;
        mRootNodes[SCENE_STATIC] = staticRoot;
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::setParentlessRootNodes( SceneNode *dynamicRoot, SceneNode *staticRoot )
    {
        mParentlessRootNodes[SCENE_DYNAMIC] = dynamicRoot;
        mParentlessRootNodes[SCENE_STATIC] = staticRoot;
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::setSceneComponentTransform( const Matrix4 &transform )
    {
        mSceneComponentTransform = transform;
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::importScene( const String &filename, const char *jsonString,
                                           uint32 importFlags )
    {
        rapidjson::Document d;
        d.Parse( jsonString );

        if( d.HasParseError() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "SceneFormatImporter::importScene",
                         "Invalid JSON string in file " + filename + " at line " +
                         StringConverter::toString( d.GetErrorOffset() ) + " Reason: " +
                         rapidjson::GetParseError_En( d.GetParseError() ) );
        }

        importScene( filename, d, importFlags );
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::importSceneFromFile( const String &folderPath, uint32 importFlags )
    {
        ResourceGroupManager &resourceGroupManager = ResourceGroupManager::getSingleton();
        resourceGroupManager.addResourceLocation( folderPath, "FileSystem", "SceneFormatImporter" );
        resourceGroupManager.addResourceLocation( folderPath + "/v2/",
                                                  "FileSystem", "SceneFormatImporter" );
        resourceGroupManager.addResourceLocation( folderPath + "/v1/",
                                                  "FileSystem", "SceneFormatImporter" );
        resourceGroupManager.addResourceLocation( folderPath + "/textures/",
                                                  "FileSystem", "SceneFormatImporter" );

        {
            DataStreamPtr stream = resourceGroupManager.openResource( "textureMetadataCache.json",
                                                                      "SceneFormatImporter" );
            vector<char>::type fileData;
            fileData.resize( stream->size() + 1 );
            if( !fileData.empty() )
            {
                stream->read( &fileData[0], stream->size() );
                //Add null terminator just in case (to prevent bad input)
                fileData.back() = '\0';

                HlmsManager *hlmsManager = mRoot->getHlmsManager();
                HlmsTextureManager *hlmsTextureManager = hlmsManager->getTextureManager();
                hlmsTextureManager->importTextureMetadataCache( stream->getName(), &fileData[0] );
            }
        }

        DataStreamPtr stream = resourceGroupManager.openResource( "scene.json", "SceneFormatImporter" );
        vector<char>::type fileData;
        fileData.resize( stream->size() + 1 );
        if( !fileData.empty() )
        {
            stream->read( &fileData[0], stream->size() );

            //Add null terminator just in case (to prevent bad input)
            fileData.back() = '\0';

            rapidjson::Document d;
            d.Parse( &fileData[0] );

            if( d.HasParseError() )
            {
                OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                             "SceneFormatImporter::importScene",
                             "Invalid JSON string in file " + stream->getName() + " at line " +
                             StringConverter::toString( d.GetErrorOffset() ) + " Reason: " +
                             rapidjson::GetParseError_En( d.GetParseError() ) );
            }

            rapidjson::Value::ConstMemberIterator  itor;

            mUsingOitd = false;
            itor = d.FindMember( "saved_oitd_textures" );
            if( itor != d.MemberEnd() && itor->value.IsBool() )
                mUsingOitd = itor->value.GetBool();

            HlmsManager *hlmsManager = mRoot->getHlmsManager();
            if( mUsingOitd )
                hlmsManager->mAdditionalTextureExtensionsPerGroup["SceneFormatImporter"] = ".oitd";
            resourceGroupManager.initialiseResourceGroup( "SceneFormatImporter", true );
            if( mUsingOitd )
                hlmsManager->mAdditionalTextureExtensionsPerGroup.erase( "SceneFormatImporter" );

            importScene( stream->getName(), d, importFlags );

            resourceGroupManager.removeResourceLocation( folderPath + "/textures/", "SceneFormatImporter" );
            resourceGroupManager.removeResourceLocation( folderPath + "/v2/", "SceneFormatImporter" );
            resourceGroupManager.removeResourceLocation( folderPath + "/v1/", "SceneFormatImporter" );
            resourceGroupManager.removeResourceLocation( folderPath, "SceneFormatImporter" );
        }
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatImporter::getInstantRadiosity( bool releaseOwnership,
                                                   InstantRadiosity **outInstantRadiosity,
                                                   IrradianceVolume **outIrradianceVolume )
    {
        *outInstantRadiosity = mInstantRadiosity;
        *outIrradianceVolume = mIrradianceVolume;
        if( releaseOwnership )
        {
            mInstantRadiosity = 0;
            mIrradianceVolume = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    ParallaxCorrectedCubemap* SceneFormatImporter::getParallaxCorrectedCubemap( bool releaseOwnership )
    {
        ParallaxCorrectedCubemap *retVal = mParallaxCorrectedCubemap;
        if( releaseOwnership )
            mParallaxCorrectedCubemap = 0;

        return retVal;
    }
}
