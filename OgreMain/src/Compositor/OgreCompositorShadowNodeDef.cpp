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

#include "Compositor/OgreCompositorShadowNodeDef.h"
#include "Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h"

#include "OgreStringConverter.h"
#include "OgreLogManager.h"

namespace Ogre
{
    IdString CompositorShadowNodeDef::addTextureSourceName( const String &name, size_t index,
                                                            TextureSource textureSource )
    {
        if( textureSource == TEXTURE_INPUT )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Shadow Nodes don't support input channels!"
                            " Shadow Node: '" + mName.getFriendlyText() + "'",
                            "OgreCompositorShadowNodeDef::addTextureSourceName" );
        }

        return CompositorNodeDef::addTextureSourceName( name, mShadowMapTexDefinitions.size() + index,
                                                        textureSource );
    }
    //-----------------------------------------------------------------------------------
    IdString CompositorShadowNodeDef::addShadowTextureSourceName( const String &name, size_t index,
                                                                  TextureSource textureSource )
    {
        if( textureSource == TEXTURE_INPUT )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Shadow Nodes don't support input channels!"
                            " Shadow Node: '" + mName.getFriendlyText() + "'",
                            "OgreCompositorShadowNodeDef::addTextureSourceName" );
        }

        return CompositorNodeDef::addTextureSourceName( name, index, textureSource );
    }
    //-----------------------------------------------------------------------------------
    ShadowTextureDefinition* CompositorShadowNodeDef::addShadowTextureDefinition( size_t lightIdx,
                                                    size_t split, const String &name, bool isAtlas )
    {
        if( name.empty() && isAtlas )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                            "Shadow maps used as atlas can't have empty names."
                            " Light index #" + StringConverter::toString( lightIdx ),
                            "OgreCompositorShadowNodeDef::addShadowTextureDefinition" );
        }

        if( !mLocalTextureDefs.empty() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                            "Shadow maps need to be defined before normal textures in a Shadow Node.",
                            "OgreCompositorShadowNodeDef::addShadowTextureDefinition" );
        }

        ShadowMapTexDefVec::const_iterator itor = mShadowMapTexDefinitions.begin();
        ShadowMapTexDefVec::const_iterator end  = mShadowMapTexDefinitions.end();

        size_t newLight = 1;

        while( itor != end )
        {
            if( itor->light == lightIdx )
                newLight = 0;

            if( itor->light == lightIdx && itor->split == split )
            {
                OGRE_EXCEPT( Exception::ERR_DUPLICATE_ITEM, "There's already a texture for light index #"
                                + StringConverter::toString( lightIdx ),
                                "OgreCompositorShadowNodeDef::addShadowTextureDefinition" );
            }
            ++itor;
        }

        mNumLights += newLight;

        if( !isAtlas )
            addShadowTextureSourceName( name, mShadowMapTexDefinitions.size(), TEXTURE_LOCAL );
        mShadowMapTexDefinitions.push_back( ShadowTextureDefinition( mDefaultTechnique, name,
                                                                     lightIdx, split ) );
        return &mShadowMapTexDefinitions.back();
    }
    //-----------------------------------------------------------------------------------
    void CompositorShadowNodeDef::_validateAndFinish(void)
    {
        const Real EPSILON = 1e-6f;

        CompositorTargetDefVec::iterator itor = mTargetPasses.begin();
        CompositorTargetDefVec::iterator end  = mTargetPasses.end();

        while( itor != end )
        {
            const CompositorPassDefVec &passDefVec = itor->getCompositorPasses();
            CompositorPassDefVec::const_iterator it = passDefVec.begin();
            CompositorPassDefVec::const_iterator en = passDefVec.end();

            while( it != en )
            {
                CompositorPassDef *pass = *it;

                //All passes in shadow nodes can't include overlays.
                if( pass->mIncludeOverlays )
                {
                    LogManager::getSingleton().logMessage( "WARNING: All Passes in a Shadow Node "
                                    "can't include overlays. Turning them off. ShadowNode: '" +
                                    mName.getFriendlyText() + "'" );
                }

                pass->mIncludeOverlays = false;

                if( pass->getType() == PASS_SCENE )
                {
                    //assert( dynamic_cast<CompositorPassSceneDef*>( pass ) );
                    CompositorPassSceneDef *passScene = static_cast<CompositorPassSceneDef*>( pass );
                    mMinRq = std::min<size_t>( mMinRq, passScene->mFirstRQ );
                    mMaxRq = std::max<size_t>( mMaxRq, passScene->mLastRQ );

                    //Regular nodes calculate the LOD values, we just use them.
                    if( passScene->mLodCameraName == IdString() )
                        passScene->mUpdateLodLists = false;

                    //Set to only render casters
                    passScene->mVisibilityMask |= VisibilityFlags::LAYER_SHADOW_CASTER;

                    //Nested shadow maps are not allowed. Sorry!
                    passScene->mShadowNode              = IdString();
                    passScene->mShadowNodeRecalculation = SHADOW_NODE_CASTER_PASS;
                }

                //Now check all PASS_SCENE from the same shadow map # render to
                //the same viewport area (common mistake in case of UV atlas)
                CompositorPassDefVec::const_iterator it2 = it + 1;
                while( it2 != en )
                {
                    CompositorPassDef *otherPass = *it2;
                    if( pass->getType() == PASS_SCENE &&
                        pass->mShadowMapIdx == otherPass->mShadowMapIdx &&
                        Math::Abs( pass->mVpLeft - otherPass->mVpLeft )     < EPSILON &&
                        Math::Abs( pass->mVpTop - otherPass->mVpTop )       < EPSILON &&
                        Math::Abs( pass->mVpWidth - otherPass->mVpWidth )   < EPSILON &&
                        Math::Abs( pass->mVpHeight - otherPass->mVpHeight ) < EPSILON &&
                        Math::Abs( pass->mVpScissorLeft - otherPass->mVpScissorLeft )     < EPSILON &&
                        Math::Abs( pass->mVpScissorTop - otherPass->mVpScissorTop )       < EPSILON &&
                        Math::Abs( pass->mVpScissorWidth - otherPass->mVpScissorWidth )   < EPSILON &&
                        Math::Abs( pass->mVpScissorHeight - otherPass->mVpScissorHeight ) < EPSILON )
                    {
                        LogManager::getSingleton().logMessage( "WARNING: Not all scene passes render to "
                                    "the same viewport! Attempting to fix. ShadowNode: '" +
                                    mName.getFriendlyText() + "' Shadow map index: " +
                                    StringConverter::toString( pass->mShadowMapIdx ) );
                        pass->mVpLeft   = otherPass->mVpLeft;
                        pass->mVpTop    = otherPass->mVpTop;
                        pass->mVpWidth  = otherPass->mVpWidth;
                        pass->mVpHeight = otherPass->mVpHeight;
                        pass->mVpScissorLeft   = otherPass->mVpScissorLeft;
                        pass->mVpScissorTop    = otherPass->mVpScissorTop;
                        pass->mVpScissorWidth  = otherPass->mVpScissorWidth;
                        pass->mVpScissorHeight = otherPass->mVpScissorHeight;
                    }

                    ++it2;
                }
                ++it;
            }

            ++itor;
        }

        //See which shadow maps can share the camera setup
        ShadowMapTexDefVec::iterator it1 = mShadowMapTexDefinitions.begin();
        ShadowMapTexDefVec::iterator en1 = mShadowMapTexDefinitions.end();

        while( it1 != en1 )
        {
            if( it1->split != 0 && it1->shadowMapTechnique != SHADOWMAP_PSSM )
            {
                OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                    "Trying to use a split with non-PSSM shadow map techniques.",
                    "CompositorShadowNodeDef::_validateAndFinish");
            }

#ifndef OGRE_REMOVE_PSSM_SPLIT_LIMIT
            if( it1->numSplits > 5 )
            {
                //The risk is that, because of how Ogre deals with constant params, we have no way
                //to tell whether the shader has enough room to hold all the floats we'll sent.
                //At 5 splits, we send 4 floats (i.e. a float4). If there were 6 splits, we would
                //send 5 floats, which means the variable needs to be declared as float [5] or
                //float4 [2], float4x2 etc. If you're sure the shader has enough room, you
                //can define this macro to remove the limit.
                it1->numSplits = 5;
                LogManager::getSingleton().logMessage( "WARNING: Limiting the number of PSSM splits per "
                            "light to 5. If you wish to use more & understand the risks, recompile Ogre "
                            "defining the macro OGRE_REMOVE_PSSM_SPLIT_LIMIT." );
            }
#endif

            bool bShared = false;

            ShadowMapTexDefVec::const_iterator it2 = mShadowMapTexDefinitions.begin();
            ShadowMapTexDefVec::const_iterator en2 = it1;

            while( it2 != en2 && !bShared )
            {
                if( it2->light == it1->light )
                {
                    if( it2->split == it1->split )
                    {
                        //Do not share the setups, the user may be trying to do tricky stuff
                        //(like comparing two shadow mapping techniques on the same light)
                        LogManager::getSingleton().logMessage( "WARNING: Two shadow maps refer to the "
                            "same light & split. Ignore this if it is intentional" );
                    }
                    else if( it2->split != it1->split )
                    {
                        if( it2->numSplits != it1->numSplits )
                        {
                            LogManager::getSingleton().logMessage( "WARNING: All pssm shadow maps with "
                                    "the same light but different split must have the same number of "
                                    "splits. Attempting to fix. ShadowNode: '" +
                                    mName.getFriendlyText() + "'." );
                            it1->numSplits = it2->numSplits;
                        }

                        it1->_setSharesSetupWithIdx( it2 - mShadowMapTexDefinitions.begin() );
                        bShared = true;
                    }
                }
                else if( it2->shadowMapTechnique == it1->shadowMapTechnique )
                {
                    it1->_setSharesSetupWithIdx( it2 - mShadowMapTexDefinitions.begin() );
                    bShared = true;
                }
                ++it2;
            }

            ++it1;
        }
    }
}
