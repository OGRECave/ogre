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

#include "OgreSceneFormat.h"
#include "OgreSceneManager.h"

#include "OgreLwString.h"

#include "OgreItem.h"
#include "OgreMesh2.h"
#include "OgreHlms.h"

namespace Ogre
{
    SceneFormat::SceneFormat( SceneManager *sceneManager, CompositorManager2 *compositorManager ) :
        mSceneManager( sceneManager ),
        mCompositorManager( compositorManager )
    {
    }
    //-----------------------------------------------------------------------------------
    SceneFormat::~SceneFormat()
    {
    }
    //-----------------------------------------------------------------------------------
    const char* SceneFormat::toStr( bool value )
    {
        return value ? "true" : "false";
    }
    //-----------------------------------------------------------------------------------
    uint32 SceneFormat::encodeFloat( float value )
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
    void SceneFormat::encodeVector( LwString &jsonStr, Vector3 value )
    {
        jsonStr.a( "[ ",
                   encodeFloat( value.x ), ", ",
                   encodeFloat( value.y ), ", ",
                   encodeFloat( value.z ),
                   " ]" );
    }
    //-----------------------------------------------------------------------------------
    void SceneFormat::encodeVector( LwString &jsonStr, Vector4 value )
    {
        jsonStr.a( "[ ",
                   encodeFloat( value.x ), ", ",
                   encodeFloat( value.y ), ", ",
                   encodeFloat( value.z ), ", " );
        jsonStr.a( encodeFloat( value.w ),
                   " ]" );
    }
    //-----------------------------------------------------------------------------------
    inline void SceneFormat::flushLwString( LwString &jsonStr, String &outJson )
    {
        outJson += jsonStr.c_str();
        jsonStr.clear();
    }
    //-----------------------------------------------------------------------------------
    void SceneFormat::exportRenderable( LwString &jsonStr, String &outJson, Renderable *renderable )
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
            jsonStr.a( "\n,\t\t\t\t\t\t\"is_v1_material\" : false" );
        }
        else
        {
            jsonStr.a( "\n\t\t\t\t\t\t\"datablock\" : \"",
                       renderable->getMaterial()->getName().c_str(), "\"" );
            jsonStr.a( "\n,\t\t\t\t\t\t\"is_v1_material\" : true" );
        }

        jsonStr.a( ",\n\t\t\t\t\t\t\"custom_parameter\" : ", renderable->mCustomParameter );
        jsonStr.a( ",\n\t\t\t\t\t\t\"render_queue_sub_group\" : ", renderable->getRenderQueueSubGroup() );
        jsonStr.a( ",\n\t\t\t\t\t\t\"polygon_mode_overrideable\" : ",
                   toStr( renderable->getPolygonModeOverrideable() ) );
        jsonStr.a( ",\n\t\t\t\t\t\t\"use_identity_view\" : ",
                   toStr( renderable->getUseIdentityView() ) );
        jsonStr.a( ",\n\t\t\t\t\t\t\"use_identity_projection\" : ",
                   toStr( renderable->getUseIdentityProjection() ) );

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
    void SceneFormat::exportMovableObject( LwString &jsonStr, String &outJson,
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
            jsonStr.a( ",\n\t\t\t\t\"local_aabb\" : [" );
            encodeVector( jsonStr, localAabb.mCenter );
            jsonStr.a( ", " );
            encodeVector( jsonStr, localAabb.mHalfSize );
            jsonStr.a( "]" );
        }

        const ObjectData &objData = movableObject->_getObjectData();

        jsonStr.a( ",\n\t\t\t\t\"local_radius\" : ",
                   encodeFloat( movableObject->getLocalRadius() ) );
        jsonStr.a( ",\n\t\t\t\t\"rendering_distance\" : ",
                   encodeFloat( movableObject->getRenderingDistance() ) );

        //Encode raw flag values
        jsonStr.a( ",\n\t\t\t\t\"visibility_flags\" : ", objData.mVisibilityFlags[objData.mIndex] );
        jsonStr.a( ",\n\t\t\t\t\"query_flags\" : ", objData.mQueryFlags[objData.mIndex] );
        jsonStr.a( ",\n\t\t\t\t\"light_mask\" : ", objData.mLightMask[objData.mIndex] );

        flushLwString( jsonStr, outJson );

        outJson += "\n\t\t\t}\n";
    }
    //-----------------------------------------------------------------------------------
    void SceneFormat::exportItem( LwString &jsonStr, String &outJson, Item *item )
    {
        const MeshPtr &mesh = item->getMesh();

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
    }
    //-----------------------------------------------------------------------------------
    void SceneFormat::exportScene( String &outJson )
    {
        char tmpBuffer[4096];
        LwString jsonStr( LwString::FromEmptyPointer( tmpBuffer, sizeof(tmpBuffer) ) );

        jsonStr.a( "{\n\t\"version\" : ", 0, "" );
        jsonStr.a( ",\n\t\"MovableObject_msDefaultVisibilityFlags\" : ",
                   MovableObject::getDefaultVisibilityFlags() );

        flushLwString( jsonStr, outJson );

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
                if( firstObject )
                {
                    outJson += "\n\t\t{";
                    firstObject = false;
                }
                else
                    outJson += ",\n\t\t{";
                exportItem( jsonStr, outJson, item );
                outJson += "\n\t\t}";
            }

            outJson += "\n\t]";
        }

        outJson += "\n}\n";
    }
}
