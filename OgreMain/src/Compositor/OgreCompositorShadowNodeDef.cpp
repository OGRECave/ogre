/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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

		return CompositorNodeDef::addTextureSourceName( name, index, textureSource );
	}
	//-----------------------------------------------------------------------------------
	CompositorShadowNodeDef::ShadowTextureDefinition*
			CompositorShadowNodeDef::addShadowTextureDefinition( size_t lightIdx, size_t split,
																 const String &name, bool isAtlas )
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
			addTextureSourceName( name, mShadowMapTexDefinitions.size(), TEXTURE_LOCAL );
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

					//Set to only render casters
					passScene->mVisibilityMask |= MovableObject::LAYER_SHADOW_CASTER;

					//Nested shadow maps are not allowed. Sorry!
					passScene->mShadowNode				= IdString();
					passScene->mShadowNodeRecalculation	= SHADOW_NODE_CASTER_PASS;
				}

				//Now check all PASS_SCENE from the same shadow map # render to
				//the same viewport area (common mistake in case of UV atlas)
				CompositorPassDefVec::const_iterator it2 = it + 1;
				while( it2 != en )
				{
					CompositorPassDef *otherPass = *it2;
					if( pass->getType() == PASS_SCENE &&
						pass->mShadowMapIdx == otherPass->mShadowMapIdx &&
						Math::Abs( pass->mVpLeft - otherPass->mVpLeft )		< EPSILON &&
						Math::Abs( pass->mVpTop - otherPass->mVpTop )		< EPSILON &&
						Math::Abs( pass->mVpWidth - otherPass->mVpWidth )	< EPSILON &&
						Math::Abs( pass->mVpHeight - otherPass->mVpHeight ) < EPSILON )
					{
						LogManager::getSingleton().logMessage( "WARNING: Not all scene passes render to "
									"the same viewport! Attempting to fix. ShadowNode: '" +
									mName.getFriendlyText() + "' Shadow map index: " +
									StringConverter::toString( pass->mShadowMapIdx ) );
						pass->mVpLeft	= otherPass->mVpLeft;
						pass->mVpTop	= otherPass->mVpTop;
						pass->mVpWidth	= otherPass->mVpWidth;
						pass->mVpHeight	= otherPass->mVpHeight;
					}

					++it2;
				}
				++it;
			}

			++itor;
		}
	}
}
