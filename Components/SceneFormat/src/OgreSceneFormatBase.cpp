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

#include "OgreSceneFormatBase.h"

#include "OgreSceneNode.h"
#include "OgreItem.h"
#include "OgreEntity.h"
#include "OgreLight.h"

#include "OgreHlmsPbs.h"
#include "Cubemaps/OgreParallaxCorrectedCubemap.h"

namespace Ogre
{
    const char* SceneFormatBase::c_lightTypes[Light::NUM_LIGHT_TYPES+1u] =
    {
        "directional",
        "point",
        "spotlight",
        "vpl",
        "area_approx",
        "area_ltc",
        "NUM_LIGHT_TYPES"
    };

    static DefaultSceneFormatListener sDefaultSceneFormatListener;

    SceneFormatBase::SceneFormatBase( Root *root, SceneManager *sceneManager ) :
        mRoot( root ),
        mSceneManager( sceneManager ),
        mListener( &sDefaultSceneFormatListener )
    {
    }
    //-----------------------------------------------------------------------------------
    SceneFormatBase::~SceneFormatBase()
    {
    }
    //-----------------------------------------------------------------------------------
    HlmsPbs* SceneFormatBase::getPbs(void) const
    {
        HlmsManager *hlmsManager = mRoot->getHlmsManager();
        Hlms *hlms = hlmsManager->getHlms( "pbs" );
        return dynamic_cast<HlmsPbs*>( hlms );
    }
    //-----------------------------------------------------------------------------------
    void SceneFormatBase::setListener( SceneFormatListener *listener )
    {
        if( listener )
            mListener = listener;
        else
            mListener = &sDefaultSceneFormatListener;
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    DefaultSceneFormatListener::DefaultSceneFormatListener() :
        mSceneFlags( 0 ),
        mParallaxCorrectedCubemap( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    void DefaultSceneFormatListener::setSceneFlags( uint32 sceneFlags, SceneFormatBase *parent )
    {
        mSceneFlags = sceneFlags;

        HlmsPbs *hlmsPbs = parent->getPbs();
        if( hlmsPbs )
            mParallaxCorrectedCubemap = hlmsPbs->getParallaxCorrectedCubemap();
    }
    //-----------------------------------------------------------------------------------
    bool DefaultSceneFormatListener::hasNoAttachedObjectsOfType( const SceneNode *sceneNode )
    {
        bool hasNoValidAttachedObject = true;

        SceneNode::ConstObjectIterator objItor = sceneNode->getAttachedObjectIterator();

        while( objItor.hasMoreElements() && hasNoValidAttachedObject )
        {
            MovableObject *movableObject =  objItor.getNext();

            if( mSceneFlags & SceneFlags::Items )
            {
                Item *asItem = dynamic_cast<Item*>( movableObject );
                if( asItem )
                    hasNoValidAttachedObject = false;
            }
            if( mSceneFlags & SceneFlags::Entities )
            {
                v1::Entity *asEntity = dynamic_cast<v1::Entity*>( movableObject );
                if( asEntity )
                    hasNoValidAttachedObject = false;
            }
            if( mSceneFlags & SceneFlags::Lights )
            {
                Light *asLight = dynamic_cast<Light*>( movableObject );
                if( asLight )
                    hasNoValidAttachedObject = false;
            }
        }

        Node::ConstNodeVecIterator nodeItor = sceneNode->getChildIterator();
        while( nodeItor.hasMoreElements() && hasNoValidAttachedObject )
        {
            Node *node = nodeItor.getNext();
            SceneNode *childNode = dynamic_cast<SceneNode*>( node );

            if( childNode )
                hasNoAttachedObjectsOfType( childNode );
        }

        return hasNoValidAttachedObject;
    }
    //-----------------------------------------------------------------------------------
    bool DefaultSceneFormatListener::exportSceneNode( const SceneNode *sceneNode )
    {
        if( mSceneFlags & SceneFlags::ForceAllSceneNodes )
            return true;

        const uint32 allObjsMask = SceneFlags::Items | SceneFlags::Entities | SceneFlags::Lights;
        if( !(mSceneFlags & allObjsMask) )
            return false; //Nothing is being exported, this node has no need (early out)

        if( mParallaxCorrectedCubemap )
        {
            SceneNode * const *proxySceneNodes = mParallaxCorrectedCubemap->getProxySceneNodes();
            if( proxySceneNodes )
            {
                for( size_t i=0; i<OGRE_MAX_CUBE_PROBES; ++i )
                {
                    if( proxySceneNodes[i] == sceneNode )
                        return false;
                }
            }
        }

        if( (mSceneFlags & allObjsMask) == allObjsMask )
            return true; //Everything is being exported. No need to keep digging (early out)

        return hasNoAttachedObjectsOfType( sceneNode );
    }
    //-----------------------------------------------------------------------------------
    bool DefaultSceneFormatListener::exportItem( const Item *item )
    {
        if( mParallaxCorrectedCubemap )
        {
            Item * const *proxyItems = mParallaxCorrectedCubemap->getProxyItems();
            if( proxyItems )
            {
                for( size_t i=0; i<OGRE_MAX_CUBE_PROBES; ++i )
                {
                    if( proxyItems[i] == item )
                        return false;
                }
            }
        }

        return true;
    }
}
