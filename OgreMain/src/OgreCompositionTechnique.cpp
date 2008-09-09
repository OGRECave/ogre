/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"
#include "OgreCompositionTechnique.h"
#include "OgreCompositionTargetPass.h"
#include "OgreCompositorInstance.h"
#include "OgreCompositorChain.h"
#include "OgreCompositionPass.h"
#include "OgreTextureManager.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

namespace Ogre {

CompositionTechnique::CompositionTechnique(Compositor *parent):
    mParent(parent)
{
    mOutputTarget = OGRE_NEW CompositionTargetPass(this);
}
//-----------------------------------------------------------------------
CompositionTechnique::~CompositionTechnique()
{
    removeAllTextureDefinitions();
    removeAllTargetPasses();
    OGRE_DELETE  mOutputTarget;
}
//-----------------------------------------------------------------------
CompositionTechnique::TextureDefinition *CompositionTechnique::createTextureDefinition(const String &name)
{
    TextureDefinition *t = OGRE_NEW TextureDefinition();
    t->name = name;
    mTextureDefinitions.push_back(t);
    return t;
}
//-----------------------------------------------------------------------

void CompositionTechnique::removeTextureDefinition(size_t index)
{
    assert (index < mTextureDefinitions.size() && "Index out of bounds.");
    TextureDefinitions::iterator i = mTextureDefinitions.begin() + index;
    OGRE_DELETE (*i);
    mTextureDefinitions.erase(i);
}
//-----------------------------------------------------------------------

CompositionTechnique::TextureDefinition *CompositionTechnique::getTextureDefinition(size_t index)
{
    assert (index < mTextureDefinitions.size() && "Index out of bounds.");
    return mTextureDefinitions[index];
}
//-----------------------------------------------------------------------

size_t CompositionTechnique::getNumTextureDefinitions()
{
    return mTextureDefinitions.size();
}
//-----------------------------------------------------------------------
void CompositionTechnique::removeAllTextureDefinitions()
{
    TextureDefinitions::iterator i, iend;
    iend = mTextureDefinitions.end();
    for (i = mTextureDefinitions.begin(); i != iend; ++i)
    {
        OGRE_DELETE (*i);
    }
    mTextureDefinitions.clear();
}
//-----------------------------------------------------------------------
CompositionTechnique::TextureDefinitionIterator CompositionTechnique::getTextureDefinitionIterator(void)
{
    return TextureDefinitionIterator(mTextureDefinitions.begin(), mTextureDefinitions.end());
}

//-----------------------------------------------------------------------
CompositionTargetPass *CompositionTechnique::createTargetPass()
{
    CompositionTargetPass *t = OGRE_NEW CompositionTargetPass(this);
    mTargetPasses.push_back(t);
    return t;
}
//-----------------------------------------------------------------------

void CompositionTechnique::removeTargetPass(size_t index)
{
    assert (index < mTargetPasses.size() && "Index out of bounds.");
    TargetPasses::iterator i = mTargetPasses.begin() + index;
    OGRE_DELETE (*i);
    mTargetPasses.erase(i);
}
//-----------------------------------------------------------------------

CompositionTargetPass *CompositionTechnique::getTargetPass(size_t index)
{
    assert (index < mTargetPasses.size() && "Index out of bounds.");
    return mTargetPasses[index];
}
//-----------------------------------------------------------------------

size_t CompositionTechnique::getNumTargetPasses()
{
    return mTargetPasses.size();
}
//-----------------------------------------------------------------------
void CompositionTechnique::removeAllTargetPasses()
{
    TargetPasses::iterator i, iend;
    iend = mTargetPasses.end();
    for (i = mTargetPasses.begin(); i != iend; ++i)
    {
        OGRE_DELETE (*i);
    }
    mTargetPasses.clear();
}
//-----------------------------------------------------------------------
CompositionTechnique::TargetPassIterator CompositionTechnique::getTargetPassIterator(void)
{
    return TargetPassIterator(mTargetPasses.begin(), mTargetPasses.end());
}
//-----------------------------------------------------------------------
CompositionTargetPass *CompositionTechnique::getOutputTargetPass()
{
    return mOutputTarget;
}
//-----------------------------------------------------------------------
bool CompositionTechnique::isSupported(bool acceptTextureDegradation)
{
	// A technique is supported if all materials referenced have a supported
	// technique, and the intermediate texture formats requested are supported
	// Material support is a cast-iron requirement, but if no texture formats 
	// are directly supported we can let the rendersystem create the closest 
	// match for the least demanding technique
	

    // Check output target pass is supported
    if (!mOutputTarget->_isSupported())
    {
        return false;
    }

    // Check all target passes is supported
    TargetPasses::iterator pi, piend;
    piend = mTargetPasses.end();
    for (pi = mTargetPasses.begin(); pi != piend; ++pi)
    {
        CompositionTargetPass* targetPass = *pi;
        if (!targetPass->_isSupported())
        {
            return false;
        }
    }

    TextureDefinitions::iterator i, iend;
    iend = mTextureDefinitions.end();
	TextureManager& texMgr = TextureManager::getSingleton();
    for (i = mTextureDefinitions.begin(); i != iend; ++i)
    {
		TextureDefinition* td = *i;

		// Firstly check MRTs
		if (td->formatList.size() > 
			Root::getSingleton().getRenderSystem()->getCapabilities()->getNumMultiRenderTargets())
		{
			return false;
		}


		for (PixelFormatList::iterator pfi = td->formatList.begin(); pfi != td->formatList.end(); ++pfi)
		{

			// Check whether equivalent supported
			if(acceptTextureDegradation)
			{
				// Don't care about exact format so long as something is supported
				if(texMgr.getNativeFormat(TEX_TYPE_2D, *pfi, TU_RENDERTARGET) == PF_UNKNOWN)
				{
					return false;
				}
			}
			else
			{
				// Need a format which is the same number of bits to pass
				if (!texMgr.isEquivalentFormatSupported(TEX_TYPE_2D, *pfi, TU_RENDERTARGET))
				{
					return false;
				}	
			}
		}
	}
	
	// Must be ok
	return true;
}
//-----------------------------------------------------------------------
Compositor *CompositionTechnique::getParent()
{
    return mParent;
}

}
