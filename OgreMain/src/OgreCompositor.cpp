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
#include "OgreCompositor.h"
#include "OgreCompositionTechnique.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreCompositorInstance.h"

namespace Ogre {

//-----------------------------------------------------------------------
Compositor::Compositor(ResourceManager* creator, const String& name, ResourceHandle handle, const String& group, bool isManual, ManualResourceLoader* loader):
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
void Compositor::removeAllTechniques()
{
    for (auto *t : mTechniques)
    {
        OGRE_DELETE t;
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

    createGlobalTextures();
}
//-----------------------------------------------------------------------
void Compositor::unloadImpl(void)
{
    freeGlobalTextures();
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

    for (auto t : mTechniques)
    {
        // Allow texture support with degraded pixel format
        if (t->isSupported(true))
        {
            mSupportedTechniques.push_back(t);
        }
    }

    if (mSupportedTechniques.empty())
        LogManager::getSingleton().logError("Compositor '" + getName() + "' has no supported techniques");

    mCompilationRequired = false;
}
//---------------------------------------------------------------------
CompositionTechnique* Compositor::getSupportedTechnique(const String& schemeName)
{
    for(auto & t : mSupportedTechniques)
    {
        if (t->getSchemeName() == schemeName)
        {
            return t;
        }
    }

    // didn't find a matching one
    for(auto & t : mSupportedTechniques)
    {
        if (t->getSchemeName().empty())
        {
            return t;
        }
    }

    return 0;

}
//-----------------------------------------------------------------------
void Compositor::createGlobalTextures()
{
    if (mSupportedTechniques.empty())
        return;

    //To make sure that we are consistent, it is demanded that all composition
    //techniques define the same set of global textures.
    typedef std::set<String> StringSet;
    StringSet globalTextureNames;

    //Initialize global textures from first supported technique
    CompositionTechnique* firstTechnique = mSupportedTechniques[0];

    for (auto *def : firstTechnique->getTextureDefinitions())
    {
        if (def->scope == CompositionTechnique::TS_GLOBAL)
        {
            //Check that this is a legit global texture
            OgreAssert(def->refCompName.empty(), "Global compositor texture definition can not be a reference");
            OgreAssert(def->width && def->height, "Global compositor texture definition must have absolute size");
            if (def->pooled)
            {
                LogManager::getSingleton().logWarning("Pooling global compositor textures has no effect");
            }
            globalTextureNames.insert(def->name);

            //TODO GSOC : Heavy copy-pasting from CompositorInstance. How to we solve it?

            // unique, even without dummyCounter, as these are global
            String baseName = StringUtil::format("%s.%s", def->name.c_str(), mName.c_str());

            /// Make the tetxure
            RenderTarget* rendTarget;
            if (def->formatList.size() > 1)
            {
                MultiRenderTarget* mrt = Root::getSingleton().getRenderSystem()->createMultiRenderTarget(baseName);
                mGlobalMRTs[def->name] = mrt;

                // create and bind individual surfaces
                size_t atch = 0;
                for (PixelFormatList::iterator p = def->formatList.begin();
                    p != def->formatList.end(); ++p, ++atch)
                {

                    String texname = StringUtil::format("mrt%zu.%s", atch, baseName.c_str());
                    TexturePtr tex;

                    tex = TextureManager::getSingleton().createManual(
                            texname,
                            ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME, TEX_TYPE_2D,
                            (uint)def->width, (uint)def->height, 0, *p, TU_RENDERTARGET, 0,
                            def->hwGammaWrite && !PixelUtil::isFloatingPoint(*p), def->fsaa);

                    RenderTexture* rt = tex->getBuffer()->getRenderTarget();
                    rt->setAutoUpdated(false);
                    mrt->bindSurface(atch, rt);

                    // Also add to local textures so we can look up
                    String mrtLocalName = CompositorInstance::getMRTTexLocalName(def->name, atch);
                    mGlobalTextures[mrtLocalName] = tex;

                }

                rendTarget = mrt;
            }
            else
            {
                String texName = baseName;

                // space in the name mixup the cegui in the compositor demo
                // this is an auto generated name - so no spaces can't hart us.
                std::replace( texName.begin(), texName.end(), ' ', '_' );

                TexturePtr tex;
                tex = TextureManager::getSingleton().createManual(
                    texName,
                    ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME, TEX_TYPE_2D,
                    (uint)def->width, (uint)def->height, 0, def->formatList[0], TU_RENDERTARGET, 0,
                    def->hwGammaWrite && !PixelUtil::isFloatingPoint(def->formatList[0]), def->fsaa);


                rendTarget = tex->getBuffer()->getRenderTarget();
                mGlobalTextures[def->name] = tex;
            }

            rendTarget->setAutoUpdated( false );

            //Set DepthBuffer pool for sharing
            if(rendTarget->getDepthBufferPool() != DepthBuffer::POOL_NO_DEPTH)
                rendTarget->setDepthBufferPool( def->depthBufferId );
        }
    }

    //Validate that all other supported techniques expose the same set of global textures.
    for (size_t i = 1; i < mSupportedTechniques.size(); i++)
    {
        bool isConsistent = true;
        size_t numGlobals = 0;
        CompositionTechnique* technique = mSupportedTechniques[i];
        for (auto *texDef : technique->getTextureDefinitions())
        {
            if (texDef->scope == CompositionTechnique::TS_GLOBAL)
            {
                if (globalTextureNames.find(texDef->name) == globalTextureNames.end())
                {
                    isConsistent = false;
                    break;
                }
                numGlobals++;
            }
        }
        if (numGlobals != globalTextureNames.size())
            isConsistent = false;

        OgreAssert(isConsistent, "Different composition techniques define different global textures");
    }

}
//-----------------------------------------------------------------------
void Compositor::freeGlobalTextures()
{
    for (const auto& t : mGlobalTextures)
    {
        TextureManager::getSingleton().remove(t.second);
    }
    mGlobalTextures.clear();

    for (const auto& m : mGlobalMRTs)
    {
        // remove MRT
        Root::getSingleton().getRenderSystem()->destroyRenderTarget(m.second->getName());
    }
    mGlobalMRTs.clear();

}
//-----------------------------------------------------------------------
const String& Compositor::getTextureInstanceName(const String& name, size_t mrtIndex)
{
    return getTextureInstance(name, mrtIndex)->getName();
}
//-----------------------------------------------------------------------
const TexturePtr& Compositor::getTextureInstance(const String& name, size_t mrtIndex)
{
    //Try simple texture
    GlobalTextureMap::iterator i = mGlobalTextures.find(name);
    if(i != mGlobalTextures.end())
    {
        return i->second;
    }
    //Try MRT
    String mrtName = CompositorInstance::getMRTTexLocalName(name, mrtIndex);
    i = mGlobalTextures.find(mrtName);
    if(i != mGlobalTextures.end())
    {
        return i->second;
    }

    OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Non-existent global texture name",
        "Compositor::getTextureInstance");

}
//---------------------------------------------------------------------
RenderTarget* Compositor::getRenderTarget(const String& name, int slice)
{
    // try simple texture
    GlobalTextureMap::iterator i = mGlobalTextures.find(name);
    if(i != mGlobalTextures.end())
        return i->second->getBuffer(slice)->getRenderTarget();

    // try MRTs
    GlobalMRTMap::iterator mi = mGlobalMRTs.find(name);
    if (mi != mGlobalMRTs.end())
        return mi->second;
    else
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Non-existent global texture name",
            "Compositor::getRenderTarget");
}
//---------------------------------------------------------------------
}
