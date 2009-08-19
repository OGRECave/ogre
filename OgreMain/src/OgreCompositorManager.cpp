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
#include "OgreCompositorManager.h"
#include "OgreCompositor.h"
#include "OgreCompositorChain.h"
#include "OgreCompositionPass.h"
#include "OgreCompositionTargetPass.h"
#include "OgreCompositionTechnique.h"
#include "OgreRoot.h"
#if OGRE_USE_NEW_COMPILERS == 1
#  include "OgreScriptCompiler.h"
#endif

namespace Ogre {

template<> CompositorManager* Singleton<CompositorManager>::ms_Singleton = 0;
CompositorManager* CompositorManager::getSingletonPtr(void)
{
	return ms_Singleton;
}
CompositorManager& CompositorManager::getSingleton(void)
{  
	assert( ms_Singleton );  return ( *ms_Singleton );  
}//-----------------------------------------------------------------------
CompositorManager::CompositorManager():
	mRectangle(0), OGRE_THREAD_POINTER_INIT(mSerializer)
{
	initialise();

	// Loading order (just after materials)
	mLoadOrder = 110.0f;
	// Scripting is supported by this manager
#if OGRE_USE_NEW_COMPILERS == 0
	mScriptPatterns.push_back("*.compositor");
	ResourceGroupManager::getSingleton()._registerScriptLoader(this);
#endif

	// Resource type
	mResourceType = "Compositor";

	// Create default thread serializer instance (also non-threaded)
	OGRE_THREAD_POINTER_SET(mSerializer, OGRE_NEW CompositorSerializer());

	// Register with resource group manager
	ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);

}
//-----------------------------------------------------------------------
CompositorManager::~CompositorManager()
{
    freeChains();
	freeSharedTextures(false);
	OGRE_DELETE mRectangle;

	OGRE_THREAD_POINTER_DELETE(mSerializer);

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
void CompositorManager::initialise(void)
{
    /// Create "default" compositor
    /** Compositor that is used to implicitly represent the original
        render in the chain. This is an identity compositor with only an output pass:
    compositor Ogre/Scene
    {
        technique
        {
            target_output
            {
				pass clear
				{
					/// Clear frame
				}
                pass render_scene
                {
					visibility_mask FFFFFFFF
					render_queues SKIES_EARLY SKIES_LATE
                }
            }
        }
    };
    */
    CompositorPtr scene = create("Ogre/Scene", ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
    CompositionTechnique *t = scene->createTechnique();
    CompositionTargetPass *tp = t->getOutputTargetPass();
    tp->setVisibilityMask(0xFFFFFFFF);
	{
		CompositionPass *pass = tp->createPass();
		pass->setType(CompositionPass::PT_CLEAR);
	}
	{
		CompositionPass *pass = tp->createPass();
		pass->setType(CompositionPass::PT_RENDERSCENE);
		/// Render everything, including skies
		pass->setFirstRenderQueue(RENDER_QUEUE_BACKGROUND);
		pass->setLastRenderQueue(RENDER_QUEUE_SKIES_LATE);
	}

}
//-----------------------------------------------------------------------
void CompositorManager::parseScript(DataStreamPtr& stream, const String& groupName)
{
#if OGRE_USE_NEW_COMPILERS == 1
	ScriptCompilerManager::getSingleton().parseScript(stream, groupName);
#else // OGRE_USE_NEW_COMPILERS
#  if OGRE_THREAD_SUPPORT
	// check we have an instance for this thread
	if (!OGRE_THREAD_POINTER_GET(mSerializer))
	{
		// create a new instance for this thread - will get deleted when
		// the thread dies
		OGRE_THREAD_POINTER_SET(mSerializer, OGRE_NEW CompositorSerializer());
	}
#  endif
    OGRE_THREAD_POINTER_GET(mSerializer)->parseScript(stream, groupName);
#endif // OGRE_USE_NEW_COMPILERS

}
//-----------------------------------------------------------------------
CompositorChain *CompositorManager::getCompositorChain(Viewport *vp)
{
    Chains::iterator i=mChains.find(vp);
    if(i != mChains.end())
    {
		// Make sure we have the right viewport
		// It's possible that this chain may have outlived a viewport and another
		// viewport was created at the same physical address, meaning we find it again but the viewport is gone
		i->second->_notifyViewport(vp);
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
bool CompositorManager::hasCompositorChain(Viewport *vp) const
{
    return mChains.find(vp) != mChains.end();
}
//-----------------------------------------------------------------------
void CompositorManager::removeCompositorChain(Viewport *vp)
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
    Chains::iterator i, iend=mChains.end();
    for(i=mChains.begin(); i!=iend;++i)
    {
        OGRE_DELETE  i->second;
    }
    mChains.clear();
}
//-----------------------------------------------------------------------
Renderable *CompositorManager::_getTexturedRectangle2D()
{
	if(!mRectangle)
	{
		/// 2D rectangle, to use for render_quad passes
		mRectangle = OGRE_NEW Rectangle2D(true);
	}
	RenderSystem* rs = Root::getSingleton().getRenderSystem();
	Viewport* vp = rs->_getViewport();
	Real hOffset = rs->getHorizontalTexelOffset() / (0.5 * vp->getActualWidth());
	Real vOffset = rs->getVerticalTexelOffset() / (0.5 * vp->getActualHeight());
	mRectangle->setCorners(-1 + hOffset, 1 - vOffset, 1 + hOffset, -1 - vOffset);
	return mRectangle;
}
//-----------------------------------------------------------------------
CompositorInstance *CompositorManager::addCompositor(Viewport *vp, const String &compositor, int addPosition)
{
	CompositorPtr comp = getByName(compositor);
	if(comp.isNull())
		return 0;
	CompositorChain *chain = getCompositorChain(vp);
	return chain->addCompositor(comp, addPosition==-1 ? CompositorChain::LAST : (size_t)addPosition);
}
//-----------------------------------------------------------------------
void CompositorManager::removeCompositor(Viewport *vp, const String &compositor)
{
	CompositorChain *chain = getCompositorChain(vp);
	CompositorChain::InstanceIterator it = chain->getCompositors();
	for(size_t pos=0; pos < chain->getNumCompositors(); ++pos)
	{
		CompositorInstance *instance = chain->getCompositor(pos);
		if(instance->getCompositor()->getName() == compositor)
		{
			chain->removeCompositor(pos);
			break;
		}
	}
}
//-----------------------------------------------------------------------
void CompositorManager::setCompositorEnabled(Viewport *vp, const String &compositor, bool value)
{
	CompositorChain *chain = getCompositorChain(vp);
	CompositorChain::InstanceIterator it = chain->getCompositors();
	for(size_t pos=0; pos < chain->getNumCompositors(); ++pos)
	{
		CompositorInstance *instance = chain->getCompositor(pos);
		if(instance->getCompositor()->getName() == compositor)
		{
			chain->setCompositorEnabled(pos, value);
			break;
		}
	}
}
//---------------------------------------------------------------------
void CompositorManager::_reconstructAllCompositorResources()
{
	// In order to deal with shared resources, we have to disable *all* compositors
	// first, that way shared resources will get freed
	typedef vector<CompositorInstance*>::type InstVec;
	InstVec instancesToReenable;
	for (Chains::iterator i = mChains.begin(); i != mChains.end(); ++i)
	{
		CompositorChain* chain = i->second;
		CompositorChain::InstanceIterator instIt = chain->getCompositors();
		while (instIt.hasMoreElements())
		{
			CompositorInstance* inst = instIt.getNext();
			if (inst->getEnabled())
			{
				inst->setEnabled(false);
				instancesToReenable.push_back(inst);
			}
		}
	}

	for (InstVec::iterator i = instancesToReenable.begin(); i != instancesToReenable.end(); ++i)
	{
		CompositorInstance* inst = *i;
		inst->setEnabled(true);
	}
}
//---------------------------------------------------------------------
TexturePtr CompositorManager::getSharedTexture(const String& name, 
	const String& localName,
	size_t w, size_t h, PixelFormat f, uint aa, const String& aaHint, bool srgb, 
	CompositorManager::UniqueTextureSet& texturesAssigned, 
	CompositorInstance* inst)
{
	TextureDef def(w, h, f, aa, aaHint, srgb);
	TexturesByDef::iterator i = mTexturesByDef.find(def);
	if (i == mTexturesByDef.end())
	{
		TextureList* texList = OGRE_NEW_T(TextureList, MEMCATEGORY_GENERAL);
		i = mTexturesByDef.insert(TexturesByDef::value_type(def, texList)).first;
	}
	CompositorInstance* previous = inst->getChain()->getPreviousInstance(inst);
	CompositorInstance* next = inst->getChain()->getNextInstance(inst);

	TexturePtr ret;
	TextureList* texList = i->second;
	// iterate over the existing textures and check if we can re-use
	for (TextureList::iterator t = texList->begin(); t != texList->end(); ++t)
	{
		TexturePtr& tex = *t;
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

	if (ret.isNull())
	{
		// ok, we need to create a new one
		ret = TextureManager::getSingleton().createManual(
			name, 
			ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME, TEX_TYPE_2D, 
			(uint)w, (uint)h, 0, f, TU_RENDERTARGET, 0,
			srgb, aa, aaHint); 

		texList->push_back(ret);

	}

	// record that we used this one in the requester's list
	texturesAssigned.insert(ret.get());


	return ret;
}
//---------------------------------------------------------------------
bool CompositorManager::isInputPreviousTarget(CompositorInstance* inst, const Ogre::String& localName)
{
	CompositionTechnique::TargetPassIterator tpit = inst->getTechnique()->getTargetPassIterator();
	while(tpit.hasMoreElements())
	{
		CompositionTargetPass* tp = tpit.getNext();
		if (tp->getInputMode() == CompositionTargetPass::IM_PREVIOUS &&
			tp->getOutputName() == localName)
		{
			return true;
		}

	}

	return false;

}
//---------------------------------------------------------------------
bool CompositorManager::isInputPreviousTarget(CompositorInstance* inst, TexturePtr tex)
{
	CompositionTechnique::TargetPassIterator tpit = inst->getTechnique()->getTargetPassIterator();
	while(tpit.hasMoreElements())
	{
		CompositionTargetPass* tp = tpit.getNext();
		if (tp->getInputMode() == CompositionTargetPass::IM_PREVIOUS)
		{
			// Don't have to worry about an MRT, because no MRT can be input previous
			TexturePtr t = inst->getTextureInstance(tp->getOutputName(), 0);
			if (!t.isNull() && t.get() == tex.get())
				return true;
		}

	}

	return false;

}
//---------------------------------------------------------------------
bool CompositorManager::isInputToOutputTarget(CompositorInstance* inst, const Ogre::String& localName)
{
	CompositionTargetPass* tp = inst->getTechnique()->getOutputTargetPass();
	CompositionTargetPass::PassIterator pit = tp->getPassIterator();

	while(pit.hasMoreElements())
	{
		CompositionPass* p = pit.getNext();
		for (size_t i = 0; i < p->getNumInputs(); ++i)
		{
			if (p->getInput(i).name == localName)
				return true;
		}
	}

	return false;

}
//---------------------------------------------------------------------()
bool CompositorManager::isInputToOutputTarget(CompositorInstance* inst, TexturePtr tex)
{
	CompositionTargetPass* tp = inst->getTechnique()->getOutputTargetPass();
	CompositionTargetPass::PassIterator pit = tp->getPassIterator();

	while(pit.hasMoreElements())
	{
		CompositionPass* p = pit.getNext();
		for (size_t i = 0; i < p->getNumInputs(); ++i)
		{
			TexturePtr t = inst->getTextureInstance(p->getInput(i).name, 0);
			if (!t.isNull() && t.get() == tex.get())
				return true;
		}
	}

	return false;

}
//---------------------------------------------------------------------
void CompositorManager::freeSharedTextures(bool onlyIfUnreferenced)
{
	if (onlyIfUnreferenced)
	{
		for (TexturesByDef::iterator i = mTexturesByDef.begin(); i != mTexturesByDef.end(); ++i)
		{
			TextureList* texList = i->second;
			for (TextureList::iterator j = texList->begin(); j != texList->end();)
			{
				// if the resource system, plus this class, are the only ones to have a reference..
				// NOTE: any material references will stop this texture getting freed (e.g. compositor demo)
				// until this routine is called again after the material no longer references the texture
				if (j->useCount() == ResourceGroupManager::RESOURCE_SYSTEM_NUM_REFERENCE_COUNTS + 1)
				{
					TextureManager::getSingleton().remove((*j)->getHandle());
					j = texList->erase(j);
				}
				else
					++j;
			}
		}
		
	}
	else
	{
		// destroy all
		for (TexturesByDef::iterator i = mTexturesByDef.begin(); i != mTexturesByDef.end(); ++i)
		{
			OGRE_DELETE_T(i->second, TextureList, MEMCATEGORY_GENERAL);
		}
		mTexturesByDef.clear();

	}

}


}
