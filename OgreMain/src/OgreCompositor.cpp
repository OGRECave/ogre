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
#include "OgreCompositor.h"
#include "OgreCompositionTechnique.h"

namespace Ogre {

//-----------------------------------------------------------------------
Compositor::Compositor(ResourceManager* creator, const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader):
    Resource(creator, name, handle, group, isManual, loader),
    mCompilationRequired(true)
{
}
//-----------------------------------------------------------------------

Compositor::~Compositor()
{
    removeAllTechniques();
    // have to call this here reather than in Resource destructor
    // since calling virtual methods in base destructors causes crash
    unload(); 
}
//-----------------------------------------------------------------------
CompositionTechnique *Compositor::createTechnique()
{
    CompositionTechnique *t = OGRE_NEW CompositionTechnique(this);
    mTechniques.push_back(t);
    mCompilationRequired = true;
    return t;
}
//-----------------------------------------------------------------------

void Compositor::removeTechnique(size_t index)
{
    assert (index < mTechniques.size() && "Index out of bounds.");
    Techniques::iterator i = mTechniques.begin() + index;
    OGRE_DELETE (*i);
    mTechniques.erase(i);
    mSupportedTechniques.clear();
    mCompilationRequired = true;
}
//-----------------------------------------------------------------------

CompositionTechnique *Compositor::getTechnique(size_t index)
{
    assert (index < mTechniques.size() && "Index out of bounds.");
    return mTechniques[index];
}
//-----------------------------------------------------------------------

size_t Compositor::getNumTechniques()
{
    return mTechniques.size();
}
//-----------------------------------------------------------------------
void Compositor::removeAllTechniques()
{
    Techniques::iterator i, iend;
    iend = mTechniques.end();
    for (i = mTechniques.begin(); i != iend; ++i)
    {
        OGRE_DELETE (*i);
    }
    mTechniques.clear();
    mSupportedTechniques.clear();
    mCompilationRequired = true;
}
//-----------------------------------------------------------------------
Compositor::TechniqueIterator Compositor::getTechniqueIterator(void)
{
    return TechniqueIterator(mTechniques.begin(), mTechniques.end());
}
//-----------------------------------------------------------------------

CompositionTechnique *Compositor::getSupportedTechnique(size_t index)
{
    assert (index < mSupportedTechniques.size() && "Index out of bounds.");
    return mSupportedTechniques[index];
}
//-----------------------------------------------------------------------

size_t Compositor::getNumSupportedTechniques()
{
    return mSupportedTechniques.size();
}
//-----------------------------------------------------------------------

Compositor::TechniqueIterator Compositor::getSupportedTechniqueIterator(void)
{
    return TechniqueIterator(mSupportedTechniques.begin(), mSupportedTechniques.end());
}
//-----------------------------------------------------------------------
void Compositor::loadImpl(void)
{
    // compile if required
    if (mCompilationRequired)
        compile();
}
//-----------------------------------------------------------------------
void Compositor::unloadImpl(void)
{
}
//-----------------------------------------------------------------------
size_t Compositor::calculateSize(void) const
{
    return 0;
}

//-----------------------------------------------------------------------
void Compositor::compile()
{
    /// Sift out supported techniques
    mSupportedTechniques.clear();
    Techniques::iterator i, iend;
    iend = mTechniques.end();

	// Try looking for exact technique support with no texture fallback
    for (i = mTechniques.begin(); i != iend; ++i)
    {
        // Look for exact texture support first
        if((*i)->isSupported(false))
        {
            mSupportedTechniques.push_back(*i);
        }
    }

	if (mSupportedTechniques.empty())
	{
		// Check again, being more lenient with textures
		for (i = mTechniques.begin(); i != iend; ++i)
		{
			// Allow texture support with degraded pixel format
			if((*i)->isSupported(true))
			{
				mSupportedTechniques.push_back(*i);
			}
		}
	}
    mCompilationRequired = false;
}

}
