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

#include "OgreSceneFormatExporter.h"
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

#include "OgreHlmsPbs.h"
#include "InstantRadiosity/OgreInstantRadiosity.h"
#include "OgreIrradianceVolume.h"

#include "Cubemaps/OgreParallaxCorrectedCubemap.h"
#include "Compositor/OgreCompositorWorkspaceDef.h"

#include "OgreForward3D.h"
#include "OgreForwardClustered.h"

namespace Ogre
{
    SceneFormatExporter::SceneFormatExporter( Root *root, SceneManager *sceneManager,
                                              InstantRadiosity *instantRadiosity ) :
        SceneFormatBase( root, sceneManager ),
        mInstantRadiosity( instantRadiosity )
    {
    }
    //-----------------------------------------------------------------------------------
    SceneFormatExporter::~SceneFormatExporter()
    {
    }
    //-----------------------------------------------------------------------------------
    const char* SceneFormatExporter::toQuotedStr( bool value )
    {
        return value ? "true" : "false";
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatExporter::toQuotedStr( LwString &jsonStr, Light::LightTypes lightType )
    {
        jsonStr.a( "\"", c_lightTypes[lightType], "\"" );
    }
    //-----------------------------------------------------------------------------------
    uint32 SceneFormatExporter::encodeFloat( float value )
    {
        union MyUnion
        {
            float   f32;
            uint32  u32;
        };

        MyUnion myUnion;
        myUnion.f32 = value;
        return myUnion.u32;
    }
    //-----------------------------------------------------------------------------------
    uint64 SceneFormatExporter::encodeDouble( double value )
    {
        union MyUnion
        {
            double  f64;
            uint64  u64;
        };

        MyUnion myUnion;
        myUnion.f64 = value;
        return myUnion.u64;
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatExporter::encodeVector( LwString &jsonStr, const Vector2 &value )
    {
        jsonStr.a( "[ ",
                   encodeFloat( value.x ), ", ",
                   encodeFloat( value.y ),
                   " ]" );
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatExporter::encodeVector( LwString &jsonStr, const Vector3 &value )
    {
        jsonStr.a( "[ ",
                   encodeFloat( value.x ), ", ",
                   encodeFloat( value.y ), ", ",
                   encodeFloat( value.z ),
                   " ]" );
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatExporter::encodeVector( LwString &jsonStr, const Vector4 &value )
    {
        jsonStr.a( "[ ",
                   encodeFloat( value.x ), ", ",
                   encodeFloat( value.y ), ", ",
                   encodeFloat( value.z ), ", " );
        jsonStr.a( encodeFloat( value.w ),
                   " ]" );
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatExporter::encodeQuaternion( LwString &jsonStr, const Quaternion &value )
    {
        jsonStr.a( "[ ",
                   encodeFloat( value.w ), ", ",
                   encodeFloat( value.x ), ", ",
                   encodeFloat( value.y ), ", " );
        jsonStr.a( encodeFloat( value.z ),
                   " ]" );
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatExporter::encodeColour( LwString &jsonStr, const ColourValue &value )
    {
        jsonStr.a( "[ ",
                   encodeFloat( value.r ), ", ",
                   encodeFloat( value.g ), ", ",
                   encodeFloat( value.b ), ", " );
        jsonStr.a( encodeFloat( value.a ),
                   " ]" );
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatExporter::encodeAabb( LwString &jsonStr, const Aabb &value )
    {
        jsonStr.a( "[" );
        encodeVector( jsonStr, value.mCenter );
        jsonStr.a( ", " );
        encodeVector( jsonStr, value.mHalfSize );
        jsonStr.a( "]" );
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatExporter::encodeMatrix( LwString &jsonStr, const Matrix3 &value )
    {
        jsonStr.a( "[ ",
                   encodeFloat( value[0][0] ), ", ",
                   encodeFloat( value[0][1] ), ", ",
                   encodeFloat( value[0][2] ), ", " );
        jsonStr.a( encodeFloat( value[1][0] ), ", ",
                   encodeFloat( value[1][1] ), ", ",
                   encodeFloat( value[1][2] ), ", " );
        jsonStr.a( encodeFloat( value[2][0] ), ", ",
                   encodeFloat( value[2][1] ), ", ",
                   encodeFloat( value[2][2] ), " ]" );
    }
    //-----------------------------------------------------------------------------------
    inline void SceneFormatExporter::flushLwString( LwString &jsonStr, String &outJson )
    {
        outJson += jsonStr.c_str();
        jsonStr.clear();
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatExporter::exportNode( LwString &jsonStr, String &outJson, Node *node )
    {
        outJson += "\n\t\t\t\"node\" :\n\t\t\t{";

        jsonStr.a( "\n\t\t\t\t\"position\" : " );
        encodeVector( jsonStr, node->getPosition() );
        jsonStr.a( ",\n\t\t\t\t\"rotation\" : " );
        encodeQuaternion( jsonStr, node->getOrientation() );
        jsonStr.a( ",\n\t\t\t\t\"scale\" : " );
        encodeVector( jsonStr, node->getScale() );

        jsonStr.a( ",\n\t\t\t\t\"inherit_orientation\" : ",
                   toQuotedStr( node->getInheritOrientation() ) );
        jsonStr.a( ",\n\t\t\t\t\"inherit_scale\" : ", toQuotedStr( node->getInheritScale() ) );
        jsonStr.a( ",\n\t\t\t\t\"is_static\" : ", toQuotedStr( node->isStatic() ) );

        Node *parentNode = node->getParent();
        if( parentNode )
        {
            NodeToIdxMap::const_iterator itor = mNodeToIdxMap.find( parentNode );
            if( itor != mNodeToIdxMap.end() )
                jsonStr.a( ",\n\t\t\t\t\"parent_id\" : ", itor->second );
        }

        flushLwString( jsonStr, outJson );

        outJson += "\n\t\t\t}";
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatExporter::exportSceneNode( LwString &jsonStr, String &outJson, SceneNode *sceneNode )
    {
        if( sceneNode == mSceneManager->getRootSceneNode( SCENE_DYNAMIC ) ||
            sceneNode == mSceneManager->getRootSceneNode( SCENE_STATIC ) )
        {
            outJson += "\n\t\t\t\"is_root_node\" : true,";
        }

        exportNode( jsonStr, outJson, sceneNode );
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatExporter::exportRenderable( LwString &jsonStr, String &outJson, Renderable *renderable )
    {
        outJson += "\n\t\t\t\t\t\"renderable\" :\n\t\t\t\t\t{";

        if( !renderable->getMaterial() )
        {
            HlmsDatablock *datablock = renderable->getDatablock();
            const String *datablockName = datablock->getNameStr();

            if( datablockName )
                jsonStr.a( "\n\t\t\t\t\t\t\"datablock\" : \"", datablockName->c_str(), "\"" );
            else
                jsonStr.a( "\n\t\t\t\t\t\t\"datablock\" : \"",
                           datablock->getName().getFriendlyText().c_str(), "\"" );
            jsonStr.a( ",\n\t\t\t\t\t\t\"is_v1_material\" : false" );
        }
        else
        {
            jsonStr.a( "\n\t\t\t\t\t\t\"datablock\" : \"",
                       renderable->getMaterial()->getName().c_str(), "\"" );
            jsonStr.a( ",\n\t\t\t\t\t\t\"is_v1_material\" : true" );
        }

        jsonStr.a( ",\n\t\t\t\t\t\t\"custom_parameter\" : ", renderable->mCustomParameter );
        jsonStr.a( ",\n\t\t\t\t\t\t\"render_queue_sub_group\" : ", renderable->getRenderQueueSubGroup() );
        jsonStr.a( ",\n\t\t\t\t\t\t\"polygon_mode_overrideable\" : ",
                   toQuotedStr( renderable->getPolygonModeOverrideable() ) );
        jsonStr.a( ",\n\t\t\t\t\t\t\"use_identity_view\" : ",
                   toQuotedStr( renderable->getUseIdentityView() ) );
        jsonStr.a( ",\n\t\t\t\t\t\t\"use_identity_projection\" : ",
                   toQuotedStr( renderable->getUseIdentityProjection() ) );

        flushLwString( jsonStr, outJson );

        const Renderable::CustomParameterMap &customParams = renderable->getCustomParameters();

        if( !customParams.empty() )
        {
            outJson += ",\n\t\t\t\t\t\t\"custom_parameters\" : { ";
            Renderable::CustomParameterMap::const_iterator begin = customParams.begin();
            Renderable::CustomParameterMap::const_iterator itor  = customParams.begin();
            Renderable::CustomParameterMap::const_iterator end   = customParams.end();
            while( itor != end )
            {
                if( itor != begin )
                    jsonStr.a( ", " );
                //, "127" : ["value_x", "value_y", "value_z", "value_w"]
                jsonStr.a( "\"", static_cast<uint32>( itor->first ), "\" : " );
                encodeVector( jsonStr, itor->second );
                ++itor;
            }

            flushLwString( jsonStr, outJson );
            outJson += " }";
        }

        outJson += "\n\t\t\t\t\t}\n";
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatExporter::exportMovableObject( LwString &jsonStr, String &outJson,
                                           MovableObject *movableObject )
    {
        outJson += "\t\t\t\"movable_object\" :\n\t\t\t{";
        if( !movableObject->getName().empty() )
        {
            outJson += "\n\t\t\t\t\"name\" : \"";
            outJson += movableObject->getName();
            outJson += "\",\n";
        }

        jsonStr.a( "\n\t\t\t\t\"render_queue\" : ", movableObject->getRenderQueueGroup() );

        {
            Aabb localAabb = movableObject->getLocalAabb();
            jsonStr.a( ",\n\t\t\t\t\"local_aabb\" : " );
            encodeAabb( jsonStr, localAabb );
        }

        const ObjectData &objData = movableObject->_getObjectData();

        jsonStr.a( ",\n\t\t\t\t\"local_radius\" : ",
                   encodeFloat( movableObject->getLocalRadius() ) );
        jsonStr.a( ",\n\t\t\t\t\"rendering_distance\" : ",
                   encodeFloat( movableObject->getRenderingDistance() ) );

        if( movableObject->isStatic() )
            jsonStr.a( ",\n\t\t\t\t\"is_static\" : ", toQuotedStr( movableObject->isStatic() ) );

        //Encode raw flag values
        jsonStr.a( ",\n\t\t\t\t\"visibility_flags\" : ", objData.mVisibilityFlags[objData.mIndex] );
        jsonStr.a( ",\n\t\t\t\t\"query_flags\" : ", objData.mQueryFlags[objData.mIndex] );
        jsonStr.a( ",\n\t\t\t\t\"light_mask\" : ", objData.mLightMask[objData.mIndex] );

        Node *parentNode = movableObject->getParentNode();
        if( parentNode )
        {
            NodeToIdxMap::const_iterator itor = mNodeToIdxMap.find( parentNode );
            if( itor != mNodeToIdxMap.end() )
                jsonStr.a( ",\n\t\t\t\t\"parent_node_id\" : ", itor->second );
        }

        flushLwString( jsonStr, outJson );

        outJson += "\n\t\t\t}";
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatExporter::exportItem( LwString &jsonStr, String &outJson, Item *item, bool exportMesh )
    {
        const Mesh *mesh = item->getMesh().get();

        outJson += "\n\t\t\t\"mesh\" : \"";
        outJson += mesh->getName();
        outJson += "\"";

        outJson += ",\n\t\t\t\"mesh_resource_group\" : \"";
        outJson += mesh->getGroup();
        outJson += "\"";

        outJson += ",\n";
        exportMovableObject( jsonStr, outJson, item );

        outJson += ",\n\t\t\t\"sub_items\" :\n\t\t\t[";
        const size_t numSubItems = item->getNumSubItems();
        for( size_t i=0; i<numSubItems; ++i )
        {
            if( i != 0 )
                outJson += ",\n";
            else
                outJson += "\n";
            outJson += "\t\t\t\t{";
            exportRenderable( jsonStr, outJson, item->getSubItem( i ) );
            outJson += "\t\t\t\t}";
        }
        outJson += "\n\t\t\t]";

        //Export the mesh, if we haven't done that already
        if( exportMesh &&
            mExportedMeshes.find( mesh ) == mExportedMeshes.end() &&
            mListener->exportMesh( mesh ) )
        {
            FileSystemLayer::createDirectory( mCurrentExportFolder + "/v2/" );

            Ogre::MeshSerializer meshSerializer( mRoot->getRenderSystem()->getVaoManager() );
            meshSerializer.exportMesh( mesh, mCurrentExportFolder + "/v2/" + mesh->getName(),
                                       MESH_VERSION_LATEST );
            mExportedMeshes.insert( mesh );
        }
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatExporter::exportLight( LwString &jsonStr, String &outJson, Light *light )
    {
        jsonStr.a( "\n\t\t\t\"diffuse\" : " );
        encodeColour( jsonStr, light->getDiffuseColour() );
        jsonStr.a( ",\n\t\t\t\"specular\" : " );
        encodeColour( jsonStr, light->getSpecularColour() );
        jsonStr.a( ",\n\t\t\t\"power\" : ", encodeFloat( light->getPowerScale() ) );

        jsonStr.a( ",\n\t\t\t\"type\" : " );
        toQuotedStr( jsonStr, light->getType() );

        jsonStr.a( ",\n\t\t\t\"attenuation\" : " );
        encodeVector( jsonStr, Vector4( light->getAttenuationRange(),
                                        light->getAttenuationConstant(),
                                        light->getAttenuationLinear(),
                                        light->getAttenuationQuadric() ) );

        jsonStr.a( ",\n\t\t\t\"spot\" : " );
        encodeVector( jsonStr, Vector4( light->getSpotlightInnerAngle().valueRadians(),
                                        light->getSpotlightOuterAngle().valueRadians(),
                                        light->getSpotlightFalloff(),
                                        light->getSpotlightNearClipDistance() ) );

        const Real ownShadowFarDistance = light->_getOwnShadowFarDistance();
        if( ownShadowFarDistance )
        {
            jsonStr.a( ",\n\t\t\t\"shadow_far_dist\" : ", encodeFloat( light->getShadowFarDistance() ) );
        }

        const Real nearClipDistance = light->getShadowNearClipDistance();
        const Real farClipDistance = light->getShadowFarClipDistance();
        if( nearClipDistance >= 0 || farClipDistance >= 0 )
        {
            jsonStr.a( ",\n\t\t\t\"shadow_clip_dist\" : " );
            encodeVector( jsonStr, Vector2( nearClipDistance, farClipDistance ) );
        }

        jsonStr.a( ",\n" );
        flushLwString( jsonStr, outJson );
        exportMovableObject( jsonStr, outJson, light );
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatExporter::exportEntity( LwString &jsonStr, String &outJson,
                                    v1::Entity *entity, bool exportMesh )
    {
        const v1::Mesh *mesh = entity->getMesh().get();

        outJson += "\n\t\t\t\"mesh\" : \"";
        outJson += mesh->getName();
        outJson += "\"";

        outJson += ",\n\t\t\t\"mesh_resource_group\" : \"";
        outJson += mesh->getGroup();
        outJson += "\"";

        outJson += ",\n";
        exportMovableObject( jsonStr, outJson, entity );

        outJson += ",\n\t\t\t\"sub_entities\" :\n\t\t\t[";
        const size_t numSubEntities = entity->getNumSubEntities();
        for( size_t i=0; i<numSubEntities; ++i )
        {
            if( i != 0 )
                outJson += ",\n";
            else
                outJson += "\n";
            outJson += "\t\t\t\t{";
            exportRenderable( jsonStr, outJson, entity->getSubEntity( i ) );
            outJson += "\t\t\t\t}";
        }
        outJson += "\n\t\t\t]";

        //Export the mesh, if we haven't done that already
        if( exportMesh &&
            mExportedMeshesV1.find( mesh ) == mExportedMeshesV1.end() &&
            mListener->exportMesh( mesh ) )
        {
            FileSystemLayer::createDirectory( mCurrentExportFolder + "/v1/" );

            Ogre::v1::MeshSerializer meshSerializer;
            meshSerializer.exportMesh( mesh, mCurrentExportFolder + "/v1/" + mesh->getName(),
                                       v1::MESH_VERSION_LATEST );
            mExportedMeshesV1.insert( mesh );
        }
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatExporter::exportInstantRadiosity( LwString &jsonStr, String &outJson )
    {
        if( !mInstantRadiosity )
            return;

        jsonStr.a( ",\n\t\t\"instant_radiosity\" :"
                   "\n\t\t{" );
        jsonStr.a( "\n\t\t\t\"first_rq\" : ", mInstantRadiosity->mFirstRq );
        jsonStr.a( ",\n\t\t\t\"last_rq\" : ", mInstantRadiosity->mLastRq );
        jsonStr.a( ",\n\t\t\t\"visibility_mask\" : ", mInstantRadiosity->mVisibilityMask );
        jsonStr.a( ",\n\t\t\t\"light_mask\" : ", mInstantRadiosity->mLightMask );
        jsonStr.a( ",\n\t\t\t\"num_rays\" : ", (uint64)mInstantRadiosity->mNumRays );
        jsonStr.a( ",\n\t\t\t\"num_ray_bounces\" : ", (uint64)mInstantRadiosity->mNumRayBounces );
        jsonStr.a( ",\n\t\t\t\"surviving_ray_fraction\" : ",
                   encodeFloat( mInstantRadiosity->mSurvivingRayFraction ) );
        jsonStr.a( ",\n\t\t\t\"cell_size\" : ", encodeFloat( mInstantRadiosity->mCellSize ) );
        jsonStr.a( ",\n\t\t\t\"bias\" : ", encodeFloat( mInstantRadiosity->mBias ) );
        jsonStr.a( ",\n\t\t\t\"num_spread_iterations\" : ", mInstantRadiosity->mNumSpreadIterations );
        jsonStr.a( ",\n\t\t\t\"spread_threshold\" : ",
                   encodeFloat( mInstantRadiosity->mSpreadThreshold ) );
        if( !mInstantRadiosity->mAoI.empty() )
        {
            flushLwString( jsonStr, outJson );
            jsonStr.a( ",\n\t\t\t\"areas_of_interest\" :\n\t\t\t[" );
            bool firstIteration = true;
            InstantRadiosity::AreaOfInterestVec::const_iterator itor = mInstantRadiosity->mAoI.begin();
            InstantRadiosity::AreaOfInterestVec::const_iterator end  = mInstantRadiosity->mAoI.end();

            while( itor != end )
            {
                if( !firstIteration )
                    jsonStr.a( "," );
                firstIteration = false;
                jsonStr.a( "\n\t\t\t\t[ " );
                encodeAabb( jsonStr, itor->aabb );
                jsonStr.a( ", ", encodeFloat( itor->sphereRadius ), " ]" );
                ++itor;
            }
            jsonStr.a( "\n\t\t\t]" );
            flushLwString( jsonStr, outJson );
        }
        jsonStr.a( ",\n\t\t\t\"vpl_max_range\" : ",
                   encodeFloat( mInstantRadiosity->mVplMaxRange ) );
        jsonStr.a( ",\n\t\t\t\"vpl_const_atten\" : ",
                   encodeFloat( mInstantRadiosity->mVplConstAtten ) );
        jsonStr.a( ",\n\t\t\t\"vpl_linear_atten\" : ",
                   encodeFloat( mInstantRadiosity->mVplLinearAtten ) );
        jsonStr.a( ",\n\t\t\t\"vpl_quad_atten\" : ",
                   encodeFloat( mInstantRadiosity->mVplQuadAtten ) );
        jsonStr.a( ",\n\t\t\t\"vpl_threshold\" : ",
                   encodeFloat( mInstantRadiosity->mVplThreshold ) );
        jsonStr.a( ",\n\t\t\t\"vpl_power_boost\" : ",
                   encodeFloat( mInstantRadiosity->mVplPowerBoost ) );
        jsonStr.a( ",\n\t\t\t\"vpl_use_intensity_for_max_range\" : ",
                   toQuotedStr( mInstantRadiosity->mVplUseIntensityForMaxRange ) );
        jsonStr.a( ",\n\t\t\t\"vpl_intensity_range_multiplier\" : ",
                   encodeDouble( mInstantRadiosity->mVplIntensityRangeMultiplier ) );
        jsonStr.a( ",\n\t\t\t\"mipmap_bias\" : ", mInstantRadiosity->mMipmapBias );
        jsonStr.a( ",\n\t\t\t\"use_textures\" : ",
                   toQuotedStr( mInstantRadiosity->getUseTextures() ) );
        jsonStr.a( ",\n\t\t\t\"use_irradiance_volume\" : ",
                   toQuotedStr( mInstantRadiosity->getUseIrradianceVolume() ) );

        HlmsManager *hlmsManager = mRoot->getHlmsManager();
        Hlms *hlms = hlmsManager->getHlms( "pbs" );
        HlmsPbs *hlmsPbs = dynamic_cast<HlmsPbs*>( hlms );

        if( hlmsPbs && hlmsPbs->getIrradianceVolume() )
        {
            IrradianceVolume *irradianceVolume = hlmsPbs->getIrradianceVolume();

            jsonStr.a( ",\n\t\t\t\"irradiance_volume\" :\n\t\t\t\t{" );
            jsonStr.a( ",\n\t\t\t\"num_blocks\" : [ ",
                       irradianceVolume->getNumBlocksX(), " ,",
                       irradianceVolume->getNumBlocksY(), " ,",
                       irradianceVolume->getNumBlocksZ(), " ]" );
            jsonStr.a( ",\n\t\t\t\"power_scale\" : ",
                       encodeFloat( irradianceVolume->getPowerScale() ) );
            jsonStr.a( ",\n\t\t\t\"fade_attenuation_over_distance\" : ",
                       toQuotedStr( irradianceVolume->getFadeAttenuationOverDistace() ) );
            jsonStr.a( ",\n\t\t\t\"irradiance_max_power\" : ",
                       encodeFloat( irradianceVolume->getIrradianceMaxPower() ) );
            jsonStr.a( ",\n\t\t\t\"irradiance_origin\" : " );
            encodeVector( jsonStr, irradianceVolume->getIrradianceOrigin() );
            jsonStr.a( ",\n\t\t\t\"irradiance_cell_size\" : " );
            encodeVector( jsonStr, irradianceVolume->getIrradianceCellSize() );
            jsonStr.a( "\n\t\t\t}" );
        }
        else if( mInstantRadiosity->getUseIrradianceVolume() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Instant Radiosity claims to be using Irradiance Volumes, "
                         "but we couldn't grab it from the Hlms PBS! "
                         "Make sure to export after you've called HlmsPbs::setIrradianceVolume",
                         "SceneFormatExporter::exportInstantRadiosity" );
        }

        jsonStr.a( "\n\t\t}" );
        flushLwString( jsonStr, outJson );
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatExporter::exportPcc( LwString &jsonStr, String &outJson )
    {
        HlmsPbs *hlmsPbs = getPbs();
        if( !hlmsPbs )
            return;

        ParallaxCorrectedCubemap *pcc = hlmsPbs->getParallaxCorrectedCubemap();

        if( !pcc )
            return;

        TexturePtr pccBlendTex = pcc->getBlendCubemap();

        jsonStr.a( ",\n\t\t\"parallax_corrected_cubemaps\" :"
                   "\n\t\t{" );
        jsonStr.a( "\n\t\t\t\"paused\" : ", pcc->mPaused );
        jsonStr.a( ",\n\t\t\t\"mask\" : ", pcc->mMask );
        jsonStr.a( ",\n\t\t\t\"reserved_rq_id\" : ", pcc->getProxyReservedRenderQueueId() );
        jsonStr.a( ",\n\t\t\t\"proxy_visibility_mask\" : ", pcc->getProxyReservedVisibilityMask() );
        if( pccBlendTex )
        {
            jsonStr.a( ",\n\t\t\t\"max_width\" : ", pccBlendTex->getWidth() );
            jsonStr.a( ",\n\t\t\t\"max_height\" : ", pccBlendTex->getHeight() );
            jsonStr.a( ",\n\t\t\t\"pixel_format\" : \"",
                       PixelUtil::getFormatName( pccBlendTex->getFormat() ).c_str(), "\"" );
        }

        const CompositorWorkspaceDef *workspaceDef = pcc->getDefaultWorkspaceDef();
        jsonStr.a( ",\n\t\t\t\"workspace\" : \"", workspaceDef->getNameStr().c_str(), "\"" );

        const CubemapProbeVec& probes = pcc->getProbes();

        if( !probes.empty() )
        {
            jsonStr.a( ",\n\t\t\t\"probes\" :"
                       "\n\t\t\t[" );

            CubemapProbeVec::const_iterator begin = probes.begin();
            CubemapProbeVec::const_iterator itor  = probes.begin();
            CubemapProbeVec::const_iterator end   = probes.end();
            while( itor != end )
            {
                if( itor != begin )
                    jsonStr.a( ", " );
                jsonStr.a( "\n\t\t\t\t{" );

                CubemapProbe *probe = *itor;

                jsonStr.a( "\n\t\t\t\t\t\"static\" : ", toQuotedStr( probe->getStatic() ) );

                TexturePtr probeTex = probe->getInternalTexture();

                if( probeTex )
                {
                    jsonStr.a( ",\n\t\t\t\t\t\"width\" : ", probeTex->getWidth() );
                    jsonStr.a( ",\n\t\t\t\t\t\"height\" : ", probeTex->getHeight() );
                    jsonStr.a( ",\n\t\t\t\t\t\"msaa\" : ", probeTex->getFSAA() );
                    jsonStr.a( ",\n\t\t\t\t\t\"pixel_format\" : \"",
                               PixelUtil::getFormatName( probeTex->getFormat() ).c_str(), "\"" );
                    jsonStr.a( ",\n\t\t\t\t\t\"use_manual\" : ",
                               toQuotedStr( (probeTex->getUsage() & TU_AUTOMIPMAP) != 0 ) );
                }

                jsonStr.a( ",\n\t\t\t\t\t\"camera_pos\" : " );
                encodeVector( jsonStr, probe->getProbeCameraPos() );

                jsonStr.a( ",\n\t\t\t\t\t\"area\" : " );
                encodeAabb( jsonStr, probe->getArea() );

                jsonStr.a( ",\n\t\t\t\t\t\"area_inner_region\" : " );
                encodeVector( jsonStr, probe->getAreaInnerRegion() );

                jsonStr.a( ",\n\t\t\t\t\t\"orientation\" : " );
                encodeMatrix( jsonStr, probe->getOrientation() );

                jsonStr.a( ",\n\t\t\t\t\t\"probe_shape\" : " );
                encodeAabb( jsonStr, probe->getProbeShape() );

                jsonStr.a( ",\n\t\t\t\t\t\"enabled\" : ", toQuotedStr( probe->mEnabled ) );
                jsonStr.a( ",\n\t\t\t\t\t\"num_iterations\" : ", probe->mNumIterations );
                jsonStr.a( ",\n\t\t\t\t\t\"mask\" : ", probe->mMask );

                jsonStr.a( "\n\t\t\t\t}" );
                ++itor;
            }

            jsonStr.a( "\n\t\t\t]" );
        }

        jsonStr.a( "\n\t\t}" );
        flushLwString( jsonStr, outJson );
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatExporter::exportSceneSettings( LwString &jsonStr, String &outJson,
                                                   uint32 exportFlags )
    {
        jsonStr.a( ",\n\t\"scene\" :\n\t{" );

        jsonStr.a( "\n\t\t\"ambient\" : [ " );
        encodeColour( jsonStr, mSceneManager->getAmbientLightUpperHemisphere() );
        jsonStr.a( ", " );
        encodeColour( jsonStr, mSceneManager->getAmbientLightLowerHemisphere() );
        jsonStr.a( ", " );
        encodeVector( jsonStr, mSceneManager->getAmbientLightHemisphereDir() );
        jsonStr.a( ", ", encodeFloat( mSceneManager->getAmbientLightUpperHemisphere().a ), " ]" );

        const ForwardPlusBase *forwardPlus = mSceneManager->getForwardPlus();

        if( forwardPlus )
        {
            jsonStr.a( ",\n\t\t\"forward_plus\" : \n\t\t{" );
            if( forwardPlus->getForwardPlusMethod() == ForwardPlusBase::MethodForward3D )
            {
                const Forward3D *forwardImpl =
                        static_cast<const Forward3D*>( forwardPlus );

                jsonStr.a( "\n\t\t\t\"mode\" : \"3d\"" );
                jsonStr.a( ",\n\t\t\t\"params\" : [" );
                jsonStr.a( forwardImpl->getWidth(), ", ", forwardImpl->getHeight(), ", ",
                           forwardImpl->getNumSlices(), ", ", forwardImpl->getLightsPerCell() );
                jsonStr.a( ", ", encodeFloat( forwardImpl->getMinDistance() ), ", ",
                           encodeFloat( forwardImpl->getMaxDistance() ), "]" );
            }
            else
            {
                const ForwardClustered *forwardImpl =
                        static_cast<const ForwardClustered*>( forwardPlus );

                jsonStr.a( "\n\t\t\t\"mode\" : \"clustered\"" );
                jsonStr.a( ",\n\t\t\t\"params\" : [" );
                jsonStr.a( forwardImpl->getWidth(), ", ", forwardImpl->getHeight(), ", ",
                           forwardImpl->getNumSlices(), ", ", forwardImpl->getLightsPerCell() );
                jsonStr.a( ", ", encodeFloat( forwardImpl->getMinDistance() ), ", ",
                           encodeFloat( forwardImpl->getMaxDistance() ), "]" );
            }

            jsonStr.a( "\n\t\t}" );
        }

        if( exportFlags & SceneFlags::ParallaxCorrectedCubemap )
            exportPcc( jsonStr, outJson );

        if( exportFlags & SceneFlags::InstantRadiosity )
            exportInstantRadiosity( jsonStr, outJson );

        jsonStr.a( "\n\t}" );

        flushLwString( jsonStr, outJson );
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatExporter::_exportScene( String &outJson, uint32 exportFlags )
    {
        mNodeToIdxMap.clear();
        mExportedMeshes.clear();
        mExportedMeshesV1.clear();

        mListener->setSceneFlags( exportFlags, this );

        char tmpBuffer[4096];
        LwString jsonStr( LwString::FromEmptyPointer( tmpBuffer, sizeof(tmpBuffer) ) );

        jsonStr.a( "{\n\t\"version\" : ", 0, "" );
        jsonStr.a( ",\n\t\"MovableObject_msDefaultVisibilityFlags\" : ",
                   MovableObject::getDefaultVisibilityFlags() );

        if( exportFlags & SceneFlags::TexturesOitd )
            jsonStr.a( ",\n\t\"saved_oitd_textures\" : \"true\"" );
        if( exportFlags & SceneFlags::TexturesOriginal )
            jsonStr.a( ",\n\t\"saved_original_textures\" : \"true\"" );

        flushLwString( jsonStr, outJson );

        if( exportFlags & SceneFlags::SceneNodes )
        {
            uint32 nodeCount = 0;

            outJson += ",\n\t\"scene_nodes\" :\n\t[";
            for( size_t i=0; i<NUM_SCENE_MEMORY_MANAGER_TYPES; ++i )
            {
                SceneNode *rootSceneNode =
                        mSceneManager->getRootSceneNode( static_cast<SceneMemoryMgrTypes>(i) );

                mNodeToIdxMap[rootSceneNode] = nodeCount++;
                if( i == 0 )
                    outJson += "\n\t\t{";
                else
                    outJson += ",\n\t\t{";
                exportSceneNode( jsonStr, outJson, rootSceneNode );
                outJson += "\n\t\t}";

                Node::NodeVecIterator nodeItor = rootSceneNode->getChildIterator();
                while( nodeItor.hasMoreElements() )
                {
                    Node *node = nodeItor.getNext();
                    SceneNode *sceneNode = dynamic_cast<SceneNode*>( node );

                    if( sceneNode && mListener->exportSceneNode( sceneNode ) )
                    {
                        mNodeToIdxMap[sceneNode] = nodeCount++;
                        outJson += ",\n\t\t{";
                        exportSceneNode( jsonStr, outJson, sceneNode );
                        outJson += "\n\t\t}";
                    }
                }
            }
            outJson += "\n\t]";
        }

        if( exportFlags & SceneFlags::Items )
        {
            SceneManager::MovableObjectIterator movableObjects =
                    mSceneManager->getMovableObjectIterator( ItemFactory::FACTORY_TYPE_NAME );

            if( movableObjects.hasMoreElements() )
            {
                outJson += ",\n\t\"items\" :\n\t[\n";

                bool firstObject = true;

                while( movableObjects.hasMoreElements() )
                {
                    MovableObject *mo = movableObjects.getNext();
                    Item *item = static_cast<Item*>( mo );
                    if( mListener->exportItem( item ) )
                    {
                        if( firstObject )
                        {
                            outJson += "\n\t\t{";
                            firstObject = false;
                        }
                        else
                            outJson += ",\n\t\t{";
                        exportItem( jsonStr, outJson, item, exportFlags & SceneFlags::Meshes );
                        outJson += "\n\t\t}";
                    }
                }

                outJson += "\n\t]";
            }
        }

        if( exportFlags & SceneFlags::Lights )
        {
            SceneManager::MovableObjectIterator movableObjects =
                    mSceneManager->getMovableObjectIterator( LightFactory::FACTORY_TYPE_NAME );

            if( movableObjects.hasMoreElements() )
            {
                outJson += ",\n\t\"lights\" :\n\t[\n";

                bool firstObject = true;

                while( movableObjects.hasMoreElements() )
                {
                    MovableObject *mo = movableObjects.getNext();
                    Light *light = static_cast<Light*>( mo );
                    if( mListener->exportLight( light ) )
                    {
                        if( firstObject )
                        {
                            outJson += "\n\t\t{";
                            firstObject = false;
                        }
                        else
                            outJson += ",\n\t\t{";
                        exportLight( jsonStr, outJson, light );
                        outJson += "\n\t\t}";
                    }
                }

                outJson += "\n\t]";
            }
        }

        if( exportFlags & SceneFlags::Entities )
        {
            SceneManager::MovableObjectIterator movableObjects =
                    mSceneManager->getMovableObjectIterator( v1::EntityFactory::FACTORY_TYPE_NAME );

            if( movableObjects.hasMoreElements() )
            {
                outJson += ",\n\t\"entities\" :\n\t[\n";

                bool firstObject = true;

                while( movableObjects.hasMoreElements() )
                {
                    MovableObject *mo = movableObjects.getNext();
                    v1::Entity *entity = static_cast<v1::Entity*>( mo );
                    if( mListener->exportEntity( entity ) )
                    {
                        if( firstObject )
                        {
                            outJson += "\n\t\t{";
                            firstObject = false;
                        }
                        else
                            outJson += ",\n\t\t{";
                        exportEntity( jsonStr, outJson, entity, exportFlags & SceneFlags::MeshesV1 );
                        outJson += "\n\t\t}";
                    }
                }

                outJson += "\n\t]";
            }
        }

        if( exportFlags & SceneFlags::SceneSettings )
            exportSceneSettings( jsonStr, outJson , exportFlags );

        outJson += "\n}\n";

        mNodeToIdxMap.clear();
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatExporter::exportScene( String &outJson, uint32 exportFlags )
    {
        mCurrentExportFolder.clear();
        _exportScene( outJson, exportFlags & ~(SceneFlags::Meshes | SceneFlags::MeshesV1) );
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatExporter::exportSceneToFile( const String &folderPath, uint32 exportFlags )
    {
        mCurrentExportFolder = folderPath;
        FileSystemLayer::createDirectory( mCurrentExportFolder );

        {
            String jsonString;
            _exportScene( jsonString, exportFlags );

            const String scenePath = folderPath + "/scene.json";
            std::ofstream file( scenePath.c_str(), std::ios::binary | std::ios::out );
            if( file.is_open() )
                file.write( jsonString.c_str(), jsonString.size() );
            file.close();
        }

        if( exportFlags & SceneFlags::Materials )
        {
            HlmsManager *hlmsManager = mRoot->getHlmsManager();
            for( size_t i=HLMS_LOW_LEVEL + 1u; i<HLMS_MAX; ++i )
            {
                if( hlmsManager->getHlms( static_cast<HlmsTypes>( i ) ) )
                {
                    const String materialPath = folderPath + "/material" +
                                                StringConverter::toString( i ) + ".material.json";
                    hlmsManager->saveMaterials( static_cast<HlmsTypes>( i ), materialPath.c_str(),
                                                mListener, BLANKSTRING );
                }
            }
        }

        if( exportFlags & (SceneFlags::TexturesOitd|SceneFlags::TexturesOriginal)  )
        {
            const String textureFolder = folderPath + "/textures/";
            FileSystemLayer::createDirectory( textureFolder );

            set<String>::type savedTextures;
            HlmsManager *hlmsManager = mRoot->getHlmsManager();
            for( size_t i=HLMS_LOW_LEVEL + 1u; i<HLMS_MAX; ++i )
            {
                Hlms *hlms = hlmsManager->getHlms( static_cast<HlmsTypes>( i ) );
                if( hlms )
                {
                    hlms->saveAllTexturesFromDatablocks(
                                textureFolder, savedTextures,
                                (exportFlags & SceneFlags::TexturesOitd) != 0,
                                (exportFlags & SceneFlags::TexturesOriginal) != 0,
                                mListener );
                }
            }
        }
    }
}
