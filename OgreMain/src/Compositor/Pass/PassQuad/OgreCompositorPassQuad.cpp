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

#include "Compositor/Pass/PassQuad/OgreCompositorPassQuad.h"
#include "Compositor/Pass/PassQuad/OgreCompositorPassQuadDef.h"
#include "Compositor/OgreCompositorNodeDef.h"
#include "Compositor/OgreCompositorNode.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorWorkspaceListener.h"

#include "OgreRenderTarget.h"
#include "OgreSceneManager.h"

namespace Ogre
{
    void CompositorPassQuadDef::addQuadTextureSource( size_t texUnitIdx, const String &textureName,
                                                        size_t mrtIndex )
    {
        if( textureName.find( "global_" ) == 0 )
        {
            mParentNodeDef->addTextureSourceName( textureName, 0,
                                                    TextureDefinitionBase::TEXTURE_GLOBAL );
        }
        mTextureSources.push_back( QuadTextureSource( texUnitIdx, textureName, mrtIndex ) );
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    CompositorPassQuad::CompositorPassQuad( const CompositorPassQuadDef *definition,
                                            Camera *defaultCamera, CompositorNode *parentNode,
                                            const CompositorChannel &target, Real horizonalTexelOffset,
                                            Real verticalTexelOffset ) :
                CompositorPass( definition, target, parentNode ),
                mDefinition( definition ),
                mFsRect( 0 ),
                mPass( 0 ),
                mCamera( 0 ),
                mHorizonalTexelOffset( horizonalTexelOffset ),
                mVerticalTexelOffset( verticalTexelOffset )
    {
        MaterialPtr material = MaterialManager::getSingleton().getByName( mDefinition->mMaterialName );
        if( material.isNull() )
        {
            OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND, "Cannot find material '" +
                         mDefinition->mMaterialName + "'", "CompositorPassQuad::CompositorPassQuad" );
        }
        material->load();

        if( !material->getBestTechnique() )
        {
            OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND, "Cannot find best technique for material '" +
                         mDefinition->mMaterialName + "'", "CompositorPassQuad::CompositorPassQuad" );
        }

        if( !material->getBestTechnique()->getPass( 0 ) )
        {
            OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND, "Best technique must have a Pass! Material '" +
                         mDefinition->mMaterialName + "'", "CompositorPassQuad::CompositorPassQuad" );
        }

        mPass = material->getBestTechnique()->getPass( 0 );

        const CompositorWorkspace *workspace = parentNode->getWorkspace();

        if( mDefinition->mUseQuad ||
            mDefinition->mFrustumCorners == CompositorPassQuadDef::WORLD_SPACE_CORNERS )
        {
            mFsRect = workspace->getCompositorManager()->getSharedFullscreenQuad();
        }
        else
        {
            mFsRect = workspace->getCompositorManager()->getSharedFullscreenTriangle();
        }

        if( mDefinition->mCameraName != IdString() )
            mCamera = workspace->findCamera( mDefinition->mCameraName );
        else
            mCamera = defaultCamera;
    }
    //-----------------------------------------------------------------------------------
    void CompositorPassQuad::execute( const Camera *lodCamera )
    {
        //Execute a limited number of times?
        if( mNumPassesLeft != std::numeric_limits<uint32>::max() )
        {
            if( !mNumPassesLeft )
                return;
            --mNumPassesLeft;
        }

        //Call beginUpdate if we're the first to use this RT
        if( mDefinition->mBeginRtUpdate )
            mTarget->_beginUpdate();

        //Set the material textures every frame (we don't clone the material)
        const CompositorPassQuadDef::TextureSources &textureSources = mDefinition->getTextureSources();
        CompositorPassQuadDef::TextureSources::const_iterator itor = textureSources.begin();
        CompositorPassQuadDef::TextureSources::const_iterator end  = textureSources.end();
        while( itor != end )
        {
            if( itor->texUnitIdx < mPass->getNumTextureUnitStates() )
            {
                TextureUnitState *tu = mPass->getTextureUnitState( itor->texUnitIdx );
                tu->setTexture( mParentNode->getDefinedTexture( itor->textureName, itor->mrtIndex ) );
            }

            ++itor;
        }

        const Real hOffset = 2.0f * mHorizonalTexelOffset / mTarget->getWidth();
        const Real vOffset = 2.0f * mVerticalTexelOffset / mTarget->getHeight();

        //The rectangle is shared, set the corners each time
        mFsRect->setCorners( mDefinition->mVpLeft + hOffset, mDefinition->mVpTop - vOffset,
                             mDefinition->mVpWidth, mDefinition->mVpHeight );

        if( mDefinition->mFrustumCorners == CompositorPassQuadDef::VIEW_SPACE_CORNERS )
        {
            const Ogre::Matrix4 &viewMat = mCamera->getViewMatrix(true);
            const Vector3 *corners = mCamera->getWorldSpaceCorners();

            mFsRect->setNormals( viewMat * corners[5], viewMat * corners[6],
                                 viewMat * corners[4], viewMat * corners[7] );
        }
        else if( mDefinition->mFrustumCorners == CompositorPassQuadDef::WORLD_SPACE_CORNERS )
        {
            const Vector3 *corners = mCamera->getWorldSpaceCorners();
            mFsRect->setNormals( corners[5], corners[6], corners[4], corners[7] );
        }

        SceneManager *sceneManager = mCamera->getSceneManager();
        sceneManager->_setViewport( mViewport );

        //Fire the listener in case it wants to change anything
        CompositorWorkspaceListener *listener = mParentNode->getWorkspace()->getListener();
        if( listener )
            listener->passPreExecute( this );

        sceneManager->_injectRenderWithPass( mPass, mFsRect, mCamera, false, false );

        //Call endUpdate if we're the last pass in a row to use this RT
        if( mDefinition->mEndRtUpdate )
            mTarget->_endUpdate();
    }
}
