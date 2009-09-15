/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
//---------------------------------------------------------------------
CompositionTechnique* Compositor::getSupportedTechnique(const String& schemeName)
{
	for(Techniques::iterator i = mSupportedTechniques.begin(); i != mSupportedTechniques.end(); ++i)
	{
		if ((*i)->getSchemeName() == schemeName)
		{
			return *i;
		}
	}

	// didn't find a matching one
	for(Techniques::iterator i = mSupportedTechniques.begin(); i != mSupportedTechniques.end(); ++i)
	{
		if ((*i)->getSchemeName() == StringUtil::BLANK)
		{
			return *i;
		}
	}

	return 0;

}

}
