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

#include "OgreRenderable.h"
#include "OgreHlmsLowLevelDatablock.h"
#include "OgreHlms.h"
#include "OgreHlmsManager.h"
#include "OgreMaterialManager.h"
#include "OgreLogManager.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreRoot.h"

namespace Ogre
{
    Renderable::Renderable() :
        mHlmsHash( 0 ),
        mHlmsCasterHash( 0 ),
        mHlmsDatablock( 0 ),
        mHasSkeletonAnimation( false ),
        mCurrentMaterialLod( 0 ),
        mLodMaterial( &MovableObject::c_DefaultLodMesh ),
        mHlmsGlobalIndex( ~0 ),
        mPolygonModeOverrideable( true ),
        mUseIdentityProjection( false ),
        mUseIdentityView( false ),
        mRenderSystemData( NULL )
    {
    }
    //-----------------------------------------------------------------------------------
    Renderable::~Renderable()
    {
        if( mHlmsDatablock )
        {
            mHlmsDatablock->_unlinkRenderable( this );
            mHlmsDatablock = 0;
        }

        if (mRenderSystemData)
        {
            delete mRenderSystemData;
            mRenderSystemData = NULL;
        }
    }
    //-----------------------------------------------------------------------------------
    void Renderable::setDatablock( IdString datablockName )
    {
        HlmsManager *hlmsManager = Root::getSingleton().getHlmsManager();
        setDatablock( hlmsManager->getDatablock( datablockName ) );
    }
    //-----------------------------------------------------------------------------------
    void Renderable::setDatablock( HlmsDatablock *datablock )
    {
        if( mHlmsDatablock != datablock )
        {
            if( mHlmsDatablock )
                mHlmsDatablock->_unlinkRenderable( this );

            mHlmsDatablock = datablock;
            try
            {
                mHlmsDatablock->getCreator()->calculateHashFor( this, mHlmsDatablock->getOriginalParams(),
                                                                mHlmsHash, mHlmsCasterHash );
            }
            catch( Exception &e )
            {
                LogManager::getSingleton().logMessage( e.getFullDescription() );
                LogManager::getSingleton().logMessage( "Couldn't apply datablock '" +
                                                       datablock->getName().getFriendlyText() + "' to "
                                                       "this renderable. Using default one. Check "
                                                       "previous log messages to see if there's more "
                                                       "information.", LML_CRITICAL );

                //Use the default datablock from the same HLMS as the one the user wanted us to apply
                mHlmsDatablock = mHlmsDatablock->getCreator()->getDefaultDatablock();
                mHlmsDatablock->getCreator()->calculateHashFor( this, mHlmsDatablock->getOriginalParams(),
                                                                mHlmsHash, mHlmsCasterHash );
            }

            mHlmsDatablock->_linkRenderable( this );
        }
    }
    //-----------------------------------------------------------------------------------
    void Renderable::_setHlmsHashes( uint32 hash, uint32 casterHash )
    {
        mHlmsHash       = hash;
        mHlmsCasterHash = casterHash;
    }
    //-----------------------------------------------------------------------------------
    void Renderable::setMaterialName( const String& name, const String& groupName )
    {
        MaterialPtr material = MaterialManager::getSingleton().getByName( name, groupName );

        if( material.isNull() )
        {
            LogManager::getSingleton().logMessage( "Can't assign material " + name +
                " because this Material does not exist. Have you forgotten to define it in a "
                ".material script?", LML_CRITICAL );

            HlmsManager *hlmsManager = Root::getSingleton().getHlmsManager();
            setDatablock( hlmsManager->getDefaultDatablock() );
        }
        else
        {
            setMaterial( material );
        }
    }
    //-----------------------------------------------------------------------------------
    void Renderable::setMaterial( const MaterialPtr& material )
    {
        // Ensure new material loaded (will not load again if already loaded)
        material->load();
        setDatablock( material->getTechnique(0)->getPass(0)->_getDatablock() );
        mLodMaterial = material->_getLodValues();
    }
    //-----------------------------------------------------------------------------------
    MaterialPtr Renderable::getMaterial(void) const
    {
        if( mHlmsDatablock->mType == HLMS_LOW_LEVEL )
        {
            assert( dynamic_cast<HlmsLowLevelDatablock*>( mHlmsDatablock ) );
            return static_cast<HlmsLowLevelDatablock*>( mHlmsDatablock )->mProxyMaterial;
        }

        return MaterialPtr();
    }
}
