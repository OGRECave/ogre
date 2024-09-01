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
#include "OgreCompositorManager.h"
#include "OgreCompositorChain.h"
#include "OgreCompositionPass.h"
#include "OgreCompositionTargetPass.h"
#include "OgreCompositionTechnique.h"

namespace Ogre {

template<> CompositorManager* Singleton<CompositorManager>::msSingleton = 0;
CompositorManager* CompositorManager::getSingletonPtr(void)
{
    return msSingleton;
}
CompositorManager& CompositorManager::getSingleton(void)
{  
    assert( msSingleton );  return ( *msSingleton );  
}//-----------------------------------------------------------------------
CompositorManager::CompositorManager():
    mRectangle(0)
{
    // Loading order (just after materials)
    mLoadOrder = 110.0f;

    // Resource type
    mResourceType = "Compositor";

    // Register with resource group manager
    ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);

}
//-----------------------------------------------------------------------
CompositorManager::~CompositorManager()
{
    freeChains();
    freePooledTextures(false);
    OGRE_DELETE mRectangle;

    // Resources cleared by superclass
    // Unregister with resource group manager
    ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);
    ResourceGroupManager::getSingleton()._unregisterScriptLoader(this);
}
//-----------------------------------------------------------------------
Resource* CompositorManager::createImpl(const String& name, ResourceHandle handle,
    const String& group, bool isManual, ManualResourceLoader* loader,
    const NameValuePairList* params)
{
    return OGRE_NEW Compositor(this, name, handle, group, isManual, loader);
}
//-----------------------------------------------------------------------
CompositorPtr CompositorManager::create (const String& name, const String& group,
                                bool isManual, ManualResourceLoader* loader,
                                const NameValuePairList* createParams)
{
    return static_pointer_cast<Compositor>(createResource(name,group,isManual,loader,createParams));
}
//-----------------------------------------------------------------------
CompositorPtr CompositorManager::getByName(const String& name, const String& groupName) const
{
    return static_pointer_cast<Compositor>(getResourceByName(name, groupName));
}
//-----------------------------------------------------------------------
CompositorChain *CompositorManager::getCompositorChain(Viewport *vp)
{
    Chains::iterator i=mChains.find(vp);
    if(i != mChains.end())
    {
        return i->second;
    }
    else
    {
        CompositorChain *chain = OGRE_NEW CompositorChain(vp);
        mChains[vp] = chain;
        return chain;
    }
}
//-----------------------------------------------------------------------
bool CompositorManager::hasCompositorChain(const Viewport *vp) const
{
    return mChains.find(vp) != mChains.end();
}
//-----------------------------------------------------------------------
void CompositorManager::removeCompositorChain(const Viewport *vp)
{
    Chains::iterator i = mChains.find(vp);
    if (i != mChains.end())
    {
        OGRE_DELETE  i->second;
        mChains.erase(i);
    }
}
//-----------------------------------------------------------------------
void CompositorManager::removeAll(void)
{
    freeChains();
    ResourceManager::removeAll();
}
//-----------------------------------------------------------------------
void CompositorManager::freeChains()
{
    for(auto& c : mChains)
    {
        OGRE_DELETE  c.second;
    }
    mChains.clear();
}
//-----------------------------------------------------------------------
Renderable *CompositorManager::_getTexturedRectangle2D()
{
    if(!mRectangle)
    {
        /// 2D rectangle, to use for render_quad passes
        mRectangle = OGRE_NEW Rectangle2D(true, HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
    }
    RenderSystem* rs = Root::getSingleton().getRenderSystem();
    Viewport* vp = rs->_getViewport();
    Real hOffset = rs->getHorizontalTexelOffset() / (0.5f * vp->getActualWidth());
    Real vOffset = rs->getVerticalTexelOffset() / (0.5f * vp->getActualHeight());
    mRectangle->setCorners(-1 + hOffset, 1 - vOffset, 1 + hOffset, -1 - vOffset);
    return mRectangle;
}
//-----------------------------------------------------------------------
CompositorInstance *CompositorManager::addCompositor(Viewport *vp, const String &compositor, int addPosition)
{
    CompositorPtr comp = getByName(compositor);
    if(!comp)
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Compositor '"+compositor+"' not found");
    CompositorChain *chain = getCompositorChain(vp);
    return chain->addCompositor(comp, addPosition==-1 ? CompositorChain::LAST : (size_t)addPosition);
}
//-----------------------------------------------------------------------
void CompositorManager::removeCompositor(Viewport *vp, const String &compositor)
{
    CompositorChain *chain = getCompositorChain(vp);
    size_t pos = chain->getCompositorPosition(compositor);

    if(pos == CompositorChain::NPOS)
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Compositor '"+compositor+"' not in chain");

    chain->removeCompositor(pos);
}
//-----------------------------------------------------------------------
void CompositorManager::setCompositorEnabled(Viewport *vp, const String &compositor, bool value)
{
    CompositorChain *chain = getCompositorChain(vp);
    size_t pos = chain->getCompositorPosition(compositor);

    if(pos == CompositorChain::NPOS)
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Compositor '"+compositor+"' not in chain");

    chain->setCompositorEnabled(pos, value);
}
//---------------------------------------------------------------------
void CompositorManager::_reconstructAllCompositorResources()
{
    // In order to deal with shared resources, we have to disable *all* compositors
    // first, that way shared resources will get freed
    typedef std::vector<CompositorInstance*> InstVec;
    InstVec instancesToReenable;
    for (auto & it : mChains)
    {
        CompositorChain* chain = it.second;
        for (CompositorInstance* inst : chain->getCompositorInstances())
        {
            if (inst->getEnabled())
            {
                inst->setEnabled(false);
                instancesToReenable.push_back(inst);
            }
        }
    }

    //UVs are lost, and will never be reconstructed unless we do them again, now
    if( mRectangle )
        mRectangle->setDefaultUVs();

    for (auto inst : instancesToReenable)
    {
        inst->setEnabled(true);
    }
}
//---------------------------------------------------------------------
TexturePtr CompositorManager::getPooledTexture(const String& name, 
    const String& localName,
    uint32 w, uint32 h, PixelFormat f, uint aa, const String& aaHint, bool srgb,
    CompositorManager::UniqueTextureSet& texturesAssigned, 
    CompositorInstance* inst, CompositionTechnique::TextureScope scope, TextureType type)
{
    OgreAssert(scope != CompositionTechnique::TS_GLOBAL, "Global scope texture can not be pooled");

    TextureDef def(w, h, type, f, aa, aaHint, srgb);

    if (scope == CompositionTechnique::TS_CHAIN)
    {
        StringPair pair = std::make_pair(inst->getCompositor()->getName(), localName);
        TextureDefMap& defMap = mChainTexturesByDef[pair];
        TextureDefMap::iterator it = defMap.find(def);
        if (it != defMap.end())
        {
            return it->second;
        }
        // ok, we need to create a new one
        TexturePtr newTex = TextureManager::getSingleton().createManual(
            name, 
            ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME, TEX_TYPE_2D, 
            (uint)w, (uint)h, 0, f, TU_RENDERTARGET, 0,
            srgb, aa, aaHint);
        defMap.emplace(def, newTex);
        return newTex;
    }

    TexturesByDef::iterator i = mTexturesByDef.emplace(def, TextureList()).first;

    CompositorInstance* previous = inst->getChain()->getPreviousInstance(inst);
    CompositorInstance* next = inst->getChain()->getNextInstance(inst);

    TexturePtr ret;
    TextureList& texList = i->second;
    // iterate over the existing textures and check if we can re-use
    for (auto & tex : texList)
    {
        // check not already used
        if (texturesAssigned.find(tex.get()) == texturesAssigned.end())
        {
            bool allowReuse = true;
            // ok, we didn't use this one already
            // however, there is an edge case where if we re-use a texture
            // which has an 'input previous' pass, and it is chained from another
            // compositor, we can end up trying to use the same texture for both
            // so, never allow a texture with an input previous pass to be 
            // shared with its immediate predecessor in the chain
            if (isInputPreviousTarget(inst, localName))
            {
                // Check whether this is also an input to the output target of previous
                // can't use CompositorInstance::mPreviousInstance, only set up
                // during compile
                if (previous && isInputToOutputTarget(previous, tex))
                    allowReuse = false;
            }
            // now check the other way around since we don't know what order they're bound in
            if (isInputToOutputTarget(inst, localName))
            {
                
                if (next && isInputPreviousTarget(next, tex))
                    allowReuse = false;
            }
            
            if (allowReuse)
            {
                ret = tex;
                break;
            }

        }
    }

    if (!ret)
    {
        // ok, we need to create a new one
        ret = TextureManager::getSingleton().createManual(
            name, 
            ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME, TEX_TYPE_2D, 
            w, h, 0, f, TU_RENDERTARGET, 0,
            srgb, aa, aaHint); 

        texList.push_back(ret);

    }

    // record that we used this one in the requester's list
    texturesAssigned.insert(ret.get());


    return ret;
}
//---------------------------------------------------------------------
bool CompositorManager::isInputPreviousTarget(CompositorInstance* inst, const Ogre::String& localName)
{
    const CompositionTechnique::TargetPasses& passes = inst->getTechnique()->getTargetPasses();
    for (auto *tp :  passes)
    {
        if (tp->getInputMode() == CompositionTargetPass::IM_PREVIOUS &&
            tp->getOutputName() == localName)
        {
            return true;
        }

    }

    return false;

}
//---------------------------------------------------------------------
bool CompositorManager::isInputPreviousTarget(CompositorInstance* inst, const TexturePtr& tex)
{
    const CompositionTechnique::TargetPasses& passes = inst->getTechnique()->getTargetPasses();
    for (auto *tp : passes)
    {
        if (tp->getInputMode() == CompositionTargetPass::IM_PREVIOUS)
        {
            // Don't have to worry about an MRT, because no MRT can be input previous
            TexturePtr t = inst->getTextureInstance(tp->getOutputName(), 0);
            if (t && t.get() == tex.get())
                return true;
        }

    }

    return false;

}
//---------------------------------------------------------------------
bool CompositorManager::isInputToOutputTarget(CompositorInstance* inst, const Ogre::String& localName)
{
    CompositionTargetPass* tp = inst->getTechnique()->getOutputTargetPass();
    for (auto *p : tp->getPasses())
    {
        for (size_t i = 0; i < p->getNumInputs(); ++i)
        {
            if (p->getInput(i).name == localName)
                return true;
        }
    }

    return false;

}
//---------------------------------------------------------------------()
bool CompositorManager::isInputToOutputTarget(CompositorInstance* inst, const TexturePtr& tex)
{
    CompositionTargetPass* tp = inst->getTechnique()->getOutputTargetPass();
    for (auto *p : tp->getPasses())
    {
        for (size_t i = 0; i < p->getNumInputs(); ++i)
        {
            TexturePtr t = inst->getTextureInstance(p->getInput(i).name, 0);
            if (t && t.get() == tex.get())
                return true;
        }
    }

    return false;

}
//---------------------------------------------------------------------
void CompositorManager::freePooledTextures(bool onlyIfUnreferenced)
{
    if (onlyIfUnreferenced)
    {
        for (auto & i : mTexturesByDef)
        {
            TextureList& texList = i.second;
            for (auto j = texList.begin(); j != texList.end();)
            {
                // if the resource system, plus this class, are the only ones to have a reference..
                // NOTE: any material references will stop this texture getting freed (e.g. compositor demo)
                // until this routine is called again after the material no longer references the texture
                if (j->use_count() == ResourceGroupManager::RESOURCE_SYSTEM_NUM_REFERENCE_COUNTS + 1)
                {
                    TextureManager::getSingleton().remove((*j)->getHandle());
                    j = texList.erase(j);
                }
                else
                    ++j;
            }
        }
        for (auto & i : mChainTexturesByDef)
        {
            TextureDefMap& texMap = i.second;
            for (TextureDefMap::iterator j = texMap.begin(); j != texMap.end();) 
            {
                const TexturePtr& tex = j->second;
                if (tex.use_count() == ResourceGroupManager::RESOURCE_SYSTEM_NUM_REFERENCE_COUNTS + 1)
                {
                    TextureManager::getSingleton().remove(tex->getHandle());
                    texMap.erase(j++);
                }
                else
                    ++j;
            }
        }
    }
    else
    {
        // destroy all
        mTexturesByDef.clear();
        mChainTexturesByDef.clear();
    }

}
//---------------------------------------------------------------------
void CompositorManager::registerCompositorLogic(const String& name, CompositorLogic* logic)
{   
    OgreAssert(!name.empty(), "Compositor logic name must not be empty");
    if (mCompositorLogics.find(name) != mCompositorLogics.end())
    {
        OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
            "Compositor logic '" + name + "' already exists.",
            "CompositorManager::registerCompositorLogic");
    }
    mCompositorLogics[name] = logic;
}
//---------------------------------------------------------------------
void CompositorManager::unregisterCompositorLogic(const String& name)
{
    CompositorLogicMap::iterator itor = mCompositorLogics.find(name);
    if( itor == mCompositorLogics.end() )
    {
        OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
            "Compositor logic '" + name + "' not registered.",
            "CompositorManager::unregisterCompositorLogic");
    }

    mCompositorLogics.erase( itor );
}
//---------------------------------------------------------------------
CompositorLogic* CompositorManager::getCompositorLogic(const String& name)
{
    CompositorLogicMap::iterator it = mCompositorLogics.find(name);
    if (it == mCompositorLogics.end())
    {
        OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
            "Compositor logic '" + name + "' not registered.",
            "CompositorManager::getCompositorLogic");
    }
    return it->second;
}
//---------------------------------------------------------------------
bool CompositorManager::hasCompositorLogic(const String& name)
{
	return mCompositorLogics.find(name) != mCompositorLogics.end();
}
//---------------------------------------------------------------------
void CompositorManager::registerCustomCompositionPass(const String& name, CustomCompositionPass* logic)
{   
    OgreAssert(!name.empty(), "Compositor pass name must not be empty");
    if (mCustomCompositionPasses.find(name) != mCustomCompositionPasses.end())
    {
        OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
            "Custom composition pass  '" + name + "' already exists.",
            "CompositorManager::registerCustomCompositionPass");
    }
    mCustomCompositionPasses[name] = logic;
}
//---------------------------------------------------------------------
void CompositorManager::unregisterCustomCompositionPass(const String& name)
{	
	CustomCompositionPassMap::iterator itor = mCustomCompositionPasses.find(name);
	if( itor == mCustomCompositionPasses.end() )
	{
		OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
			"Custom composition pass '" + name + "' not registered.",
			"CompositorManager::unRegisterCustomCompositionPass");
	}
	mCustomCompositionPasses.erase( itor );
}
//---------------------------------------------------------------------
bool CompositorManager::hasCustomCompositionPass(const String& name)
{
	return mCustomCompositionPasses.find(name) != mCustomCompositionPasses.end();
}
//---------------------------------------------------------------------
CustomCompositionPass* CompositorManager::getCustomCompositionPass(const String& name)
{
    CustomCompositionPassMap::iterator it = mCustomCompositionPasses.find(name);
    if (it == mCustomCompositionPasses.end())
    {
        OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
            "Custom composition pass '" + name + "' not registered.",
            "CompositorManager::getCustomCompositionPass");
    }
    return it->second;
}
//-----------------------------------------------------------------------
void CompositorManager::_relocateChain( Viewport* sourceVP, Viewport* destVP )
{
    if (sourceVP != destVP)
    {
        CompositorChain *chain = getCompositorChain(sourceVP);
        Ogre::RenderTarget *srcTarget = sourceVP->getTarget();
        Ogre::RenderTarget *dstTarget = destVP->getTarget();
        if (srcTarget != dstTarget)
        {
            srcTarget->removeListener(chain);
            dstTarget->addListener(chain);
        }
        chain->_notifyViewport(destVP);
        mChains.erase(sourceVP);
        mChains[destVP] = chain;
    }
}
//-----------------------------------------------------------------------
}
