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
#include "OgreCompositorInstance.h"
#include "OgreCompositorChain.h"
#include "OgreCompositorManager.h"
#include "OgreCompositionTargetPass.h"
#include "OgreCompositionPass.h"
#include "OgreCompositionTechnique.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreTexture.h"
#include "OgreLogManager.h"
#include "OgreMaterialManager.h"
#include "OgreTextureManager.h"
#include "OgreSceneManager.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreCamera.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

namespace Ogre {
CompositorInstance::CompositorInstance(Compositor *filter, CompositionTechnique *technique,
    CompositorChain *chain):
    mCompositor(filter), mTechnique(technique), mChain(chain),
		mEnabled(false)
{
}
//-----------------------------------------------------------------------
CompositorInstance::~CompositorInstance()
{
    freeResources();
}
//-----------------------------------------------------------------------
void CompositorInstance::setEnabled(bool value)
{
    if (mEnabled != value)
    {
        mEnabled = value;

        // Create of free resource.
        if (value)
        {
            createResources();
        }
        else
        {
            freeResources();
        }

        /// Notify chain state needs recompile.
        mChain->_markDirty();
    }
}
//-----------------------------------------------------------------------
bool CompositorInstance::getEnabled()
{
    return mEnabled;
}
//-----------------------------------------------------------------------

/** Clear framebuffer RenderSystem operation
 */
class RSClearOperation: public CompositorInstance::RenderSystemOperation
{
public:
	RSClearOperation(uint32 buffers, ColourValue colour, Real depth, unsigned short stencil):
		buffers(buffers), colour(colour), depth(depth), stencil(stencil)
	{}
	/// Which buffers to clear (FrameBufferType)
	uint32 buffers;
	/// Colour to clear in case FBT_COLOUR is set
	ColourValue colour;
	/// Depth to set in case FBT_DEPTH is set
	Real depth;
	/// Stencil value to set in case FBT_STENCIL is set
	unsigned short stencil;

	virtual void execute(SceneManager *sm, RenderSystem *rs)
	{
		rs->clearFrameBuffer(buffers, colour, depth, stencil);
	}
};

/** "Set stencil state" RenderSystem operation
 */
class RSStencilOperation: public CompositorInstance::RenderSystemOperation
{
public:
	RSStencilOperation(bool stencilCheck,CompareFunction func,uint32 refValue,uint32 mask,
		StencilOperation stencilFailOp,StencilOperation depthFailOp,StencilOperation passOp,
		bool twoSidedOperation):
		stencilCheck(stencilCheck),func(func),refValue(refValue),mask(mask),
		stencilFailOp(stencilFailOp),depthFailOp(depthFailOp),passOp(passOp),
		twoSidedOperation(twoSidedOperation)
	{}
	bool stencilCheck;
	CompareFunction func; 
    uint32 refValue;
	uint32 mask;
    StencilOperation stencilFailOp;
    StencilOperation depthFailOp;
    StencilOperation passOp;
    bool twoSidedOperation;

	virtual void execute(SceneManager *sm, RenderSystem *rs)
	{
		rs->setStencilCheckEnabled(stencilCheck);
		rs->setStencilBufferParams(func, refValue, mask, stencilFailOp, depthFailOp, passOp, twoSidedOperation);
	}
};

/** "Render quad" RenderSystem operation
 */
class RSQuadOperation: public CompositorInstance::RenderSystemOperation
{
public:
	RSQuadOperation(CompositorInstance *instance, uint32 pass_id, MaterialPtr mat):
	  mat(mat),instance(instance), pass_id(pass_id),
      mQuadCornerModified(false),
      mQuadLeft(-1),
      mQuadTop(1),
      mQuadRight(1),
      mQuadBottom(-1)
	{
		mat->load();
		instance->_fireNotifyMaterialSetup(pass_id, mat);
		technique = mat->getTechnique(0);
		assert(technique);
        
       
	}
	MaterialPtr mat;
	Technique *technique;
	CompositorInstance *instance;
	uint32 pass_id;

    bool mQuadCornerModified;
    Real mQuadLeft;
    Real mQuadTop;
    Real mQuadRight;
    Real mQuadBottom;

    void setQuadCorners(Real left,Real top,Real right,Real bottom)
    {
        mQuadLeft = left;
        mQuadTop = top;
        mQuadRight = right;
        mQuadBottom = bottom;
        mQuadCornerModified=true;
    }
        
	virtual void execute(SceneManager *sm, RenderSystem *rs)
	{
		// Fire listener
		instance->_fireNotifyMaterialRender(pass_id, mat);

        Rectangle2D * mRectangle=static_cast<Rectangle2D *>(CompositorManager::getSingleton()._getTexturedRectangle2D());
        if (mQuadCornerModified)
        {
            // insure positions are using peculiar render system offsets 
            RenderSystem* rs = Root::getSingleton().getRenderSystem();
            Viewport* vp = rs->_getViewport();
            Real hOffset = rs->getHorizontalTexelOffset() / (0.5 * vp->getActualWidth());
            Real vOffset = rs->getVerticalTexelOffset() / (0.5 * vp->getActualHeight());
            mRectangle->setCorners(mQuadLeft + hOffset, mQuadTop - vOffset, mQuadRight + hOffset, mQuadBottom - vOffset);
        }
        
		// Queue passes from mat
		Technique::PassIterator i = technique->getPassIterator();
		while(i.hasMoreElements())
		{
			sm->_injectRenderWithPass(
				i.getNext(), 
				mRectangle,
				false // don't allow replacement of shadow passes
				);
		}
	}
};

void CompositorInstance::collectPasses(TargetOperation &finalState, CompositionTargetPass *target)
{
	/// Here, passes are converted into render target operations
    Pass *targetpass;
    Technique *srctech;
	MaterialPtr mat, srcmat;
	
    CompositionTargetPass::PassIterator it = target->getPassIterator();
    while(it.hasMoreElements())
    {
        CompositionPass *pass = it.getNext();
        switch(pass->getType())
        {
        case CompositionPass::PT_CLEAR:
			queueRenderSystemOp(finalState, OGRE_NEW RSClearOperation(
				pass->getClearBuffers(),
				pass->getClearColour(),
				pass->getClearDepth(),
				pass->getClearStencil()
				));
            break;
		case CompositionPass::PT_STENCIL:
			queueRenderSystemOp(finalState, OGRE_NEW RSStencilOperation(
				pass->getStencilCheck(),pass->getStencilFunc(), pass->getStencilRefValue(),
				pass->getStencilMask(), pass->getStencilFailOp(), pass->getStencilDepthFailOp(),
				pass->getStencilPassOp(), pass->getStencilTwoSidedOperation()
				));
            break;
        case CompositionPass::PT_RENDERSCENE:
			if(pass->getFirstRenderQueue() < finalState.currentQueueGroupID)
			{
				/// Mismatch -- warn user
				/// XXX We could support repeating the last queue, with some effort
				LogManager::getSingleton().logMessage("Warning in compilation of Compositor "
					+mCompositor->getName()+": Attempt to render queue "+
					StringConverter::toString(pass->getFirstRenderQueue())+" before "+
					StringConverter::toString(finalState.currentQueueGroupID));
			}
			/// Add render queues
			for(int x=pass->getFirstRenderQueue(); x<=pass->getLastRenderQueue(); ++x)
			{
				assert(x>=0);
				finalState.renderQueues.set(x);
			}
			finalState.currentQueueGroupID = pass->getLastRenderQueue()+1;
			finalState.findVisibleObjects = true;
			finalState.materialScheme = target->getMaterialScheme();
			finalState.shadowsEnabled = target->getShadowsEnabled();

            break;
        case CompositionPass::PT_RENDERQUAD:
            srcmat = pass->getMaterial();
			if(srcmat.isNull())
            {
                /// No material -- warn user
				LogManager::getSingleton().logMessage("Warning in compilation of Compositor "
					+mCompositor->getName()+": No material defined for composition pass");
                break;
            }
			srcmat->load();
			if(srcmat->getNumSupportedTechniques()==0)  
			{
				/// No supported techniques -- warn user
				LogManager::getSingleton().logMessage("Warning in compilation of Compositor "
					+mCompositor->getName()+": material "+srcmat->getName()+" has no supported techniques");
                break;
			}
			srctech = srcmat->getBestTechnique(0);
			/// Create local material
			MaterialPtr mat = createLocalMaterial(srcmat->getName());
			/// Copy and adapt passes from source material
			Technique::PassIterator i = srctech->getPassIterator();
			while(i.hasMoreElements())
			{
				Pass *srcpass = i.getNext();
				/// Create new target pass
				targetpass = mat->getTechnique(0)->createPass();
				(*targetpass) = (*srcpass);
				/// Set up inputs
				for(size_t x=0; x<pass->getNumInputs(); ++x)
				{
					const CompositionPass::InputTex& inp = pass->getInput(x);
					if(!inp.name.empty())
					{
						if(x < targetpass->getNumTextureUnitStates())
						{
							targetpass->getTextureUnitState((ushort)x)->setTextureName(getSourceForTex(inp.name, inp.mrtIndex));
						} 
						else
						{
							/// Texture unit not there
							LogManager::getSingleton().logMessage("Warning in compilation of Compositor "
								+mCompositor->getName()+": material "+srcmat->getName()+" texture unit "
								+StringConverter::toString(x)+" out of bounds");
						}
					}
				}
			}

            RSQuadOperation * rsQuadOperation = OGRE_NEW RSQuadOperation(this,pass->getIdentifier(),mat);
            Real left,top,right,bottom;
            if (pass->getQuadCorners(left,top,right,bottom))
                rsQuadOperation->setQuadCorners(left,top,right,bottom);
                
			queueRenderSystemOp(finalState,rsQuadOperation);
            break;
        }
    }
}
//-----------------------------------------------------------------------
void CompositorInstance::_compileTargetOperations(CompiledState &compiledState)
{
    /// Collect targets of previous state
	if(mPreviousInstance)
	    mPreviousInstance->_compileTargetOperations(compiledState);
    /// Texture targets
    CompositionTechnique::TargetPassIterator it = mTechnique->getTargetPassIterator();
    while(it.hasMoreElements())
    {
        CompositionTargetPass *target = it.getNext();
        
        TargetOperation ts(getTargetForTex(target->getOutputName()));
        /// Set "only initial" flag, visibilityMask and lodBias according to CompositionTargetPass.
        ts.onlyInitial = target->getOnlyInitial();
        ts.visibilityMask = target->getVisibilityMask();
        ts.lodBias = target->getLodBias();
		ts.shadowsEnabled = target->getShadowsEnabled();
        /// Check for input mode previous
        if(target->getInputMode() == CompositionTargetPass::IM_PREVIOUS)
        {
            /// Collect target state for previous compositor
            /// The TargetOperation for the final target is collected seperately as it is merged
            /// with later operations
            mPreviousInstance->_compileOutputOperation(ts);
        }
        /// Collect passes of our own target
        collectPasses(ts, target);
        compiledState.push_back(ts);
    }
}
//-----------------------------------------------------------------------
void CompositorInstance::_compileOutputOperation(TargetOperation &finalState)
{
    /// Final target
    CompositionTargetPass *tpass = mTechnique->getOutputTargetPass();
    
    /// Logical-and together the visibilityMask, and multiply the lodBias
    finalState.visibilityMask &= tpass->getVisibilityMask();
    finalState.lodBias *= tpass->getLodBias();
    
    if(tpass->getInputMode() == CompositionTargetPass::IM_PREVIOUS)
    {
        /// Collect target state for previous compositor
        /// The TargetOperation for the final target is collected seperately as it is merged
        /// with later operations
        mPreviousInstance->_compileOutputOperation(finalState);
    }
    /// Collect passes
    collectPasses(finalState, tpass);
}
//-----------------------------------------------------------------------
Compositor *CompositorInstance::getCompositor()
{
    return mCompositor;
}
//-----------------------------------------------------------------------
CompositionTechnique *CompositorInstance::getTechnique()
{
    return mTechnique;
}
//-----------------------------------------------------------------------
CompositorChain *CompositorInstance::getChain()
{
	return mChain;
}
//-----------------------------------------------------------------------
const String& CompositorInstance::getTextureInstanceName(const String& name, 
														 size_t mrtIndex)
{
	return getSourceForTex(name, mrtIndex);
}
//-----------------------------------------------------------------------
MaterialPtr CompositorInstance::createLocalMaterial(const String& srcName)
{
static size_t dummyCounter = 0;
    MaterialPtr mat = 
        MaterialManager::getSingleton().create(
            "c" + StringConverter::toString(dummyCounter) + "/" + srcName,
            ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME
        );
    ++dummyCounter;
    /// This is safe, as we hold a private reference
    /// XXX does not compile due to ResourcePtr conversion :
    ///     MaterialManager::getSingleton().remove(mat);
    MaterialManager::getSingleton().remove(mat->getName());
    /// Remove all passes from first technique
    mat->getTechnique(0)->removeAllPasses();
    return mat;
}
//-----------------------------------------------------------------------
void CompositorInstance::createResources()
{
static size_t dummyCounter = 0;
    freeResources();
    /// Create temporary textures
    /// In principle, temporary textures could be shared between multiple viewports
    /// (CompositorChains). This will save a lot of memory in case more viewports
    /// are composited.
    CompositionTechnique::TextureDefinitionIterator it = mTechnique->getTextureDefinitionIterator();
    while(it.hasMoreElements())
    {
        CompositionTechnique::TextureDefinition *def = it.getNext();
        /// Determine width and height
        size_t width = def->width;
        size_t height = def->height;
		uint fsaa = 0;
		bool hwGammaWrite = false;

		deriveTextureRenderTargetOptions(def->name, &hwGammaWrite, &fsaa);

        if(width == 0)
            width = static_cast<size_t>(
				static_cast<float>(mChain->getViewport()->getActualWidth()) * def->widthFactor);
        if(height == 0)
			height = static_cast<size_t>(
				static_cast<float>(mChain->getViewport()->getActualHeight()) * def->heightFactor);
        /// Make the tetxure
		RenderTarget* rendTarget;
		if (def->formatList.size() > 1)
		{
			String MRTbaseName = "c" + StringConverter::toString(dummyCounter++) + 
				"/" + def->name + "/" + mChain->getViewport()->getTarget()->getName();
			MultiRenderTarget* mrt = 
				Root::getSingleton().getRenderSystem()->createMultiRenderTarget(MRTbaseName);
			mLocalMRTs[def->name] = mrt;

			// create and bind individual surfaces
			size_t atch = 0;
			for (PixelFormatList::iterator p = def->formatList.begin(); 
				p != def->formatList.end(); ++p, ++atch)
			{

				TexturePtr tex = TextureManager::getSingleton().createManual(
					MRTbaseName + "/" + StringConverter::toString(atch),
					ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME, TEX_TYPE_2D, 
					(uint)width, (uint)height, 0, *p, TU_RENDERTARGET ); 
				
				RenderTexture* rt = tex->getBuffer()->getRenderTarget();
				rt->setAutoUpdated(false);
				mrt->bindSurface(atch, rt);

				// Also add to local textures so we can look up
				String mrtLocalName = getMRTTexLocalName(def->name, atch);
				mLocalTextures[mrtLocalName] = tex;
				
			}

			rendTarget = mrt;
		}
		else
		{
			String texName =  "c" + StringConverter::toString(dummyCounter++) + 
				"/" + def->name + "/" + mChain->getViewport()->getTarget()->getName();
			TexturePtr tex = TextureManager::getSingleton().createManual(
				texName, 
				ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME, TEX_TYPE_2D, 
				(uint)width, (uint)height, 0, def->formatList[0], TU_RENDERTARGET, 0,
				hwGammaWrite, fsaa); 

			rendTarget = tex->getBuffer()->getRenderTarget();
			mLocalTextures[def->name] = tex;
		}
        
        
        /// Set up viewport over entire texture
        rendTarget->setAutoUpdated( false );

        Camera* camera = mChain->getViewport()->getCamera();

        // Save last viewport and current aspect ratio
        Viewport* oldViewport = camera->getViewport();
        Real aspectRatio = camera->getAspectRatio();

        Viewport* v = rendTarget->addViewport( camera );
        v->setClearEveryFrame( false );
        v->setOverlaysEnabled( false );
        v->setBackgroundColour( ColourValue( 0, 0, 0, 0 ) );

        // Should restore aspect ratio, in case of auto aspect ratio
        // enabled, it'll changed when add new viewport.
        camera->setAspectRatio(aspectRatio);
        // Should restore last viewport, i.e. never disturb user code
        // which might based on that.
        camera->_notifyViewport(oldViewport);
    }
    
}
//---------------------------------------------------------------------
void CompositorInstance::deriveTextureRenderTargetOptions(
	const String& texname, bool *hwGammaWrite, uint *fsaa)
{
	// search for passes on this texture def that either include a render_scene
	// or use input previous
	bool renderingScene = false;

	CompositionTechnique::TargetPassIterator it = mTechnique->getTargetPassIterator();
	while (it.hasMoreElements())
	{
		CompositionTargetPass* tp = it.getNext();
		if (tp->getOutputName() == texname)
		{
			if (tp->getInputMode() == CompositionTargetPass::IM_PREVIOUS)
			{
				// this may be rendering the scene implicitly
				// Can't check mPreviousInstance against mChain->_getOriginalSceneCompositor()
				// at this time, so check the position
				CompositorChain::InstanceIterator instit = mChain->getCompositors();
				renderingScene = true;
				while(instit.hasMoreElements())
				{
					CompositorInstance* inst = instit.getNext();
					if (inst == this)
						break;
					else if (inst->getEnabled())
					{
						// nope, we have another compositor before us, this will
						// be doing the AA
						renderingScene = false;
					}
				}
				if (renderingScene)
					break;
			}
			else
			{
				// look for a render_scene pass
				CompositionTargetPass::PassIterator pit = tp->getPassIterator();
				while(pit.hasMoreElements())
				{
					CompositionPass* pass = pit.getNext();
					if (pass->getType() == CompositionPass::PT_RENDERSCENE)
					{
						renderingScene = true;
						break;
					}
				}
			}

		}
	}

	if (renderingScene)
	{
		// Ok, inherit settings from target
		RenderTarget* target = mChain->getViewport()->getTarget();
		*hwGammaWrite = target->isHardwareGammaEnabled();
		*fsaa = target->getFSAA();
	}
	else
	{
		*hwGammaWrite = false;
		*fsaa = 0;
	}

}
//---------------------------------------------------------------------
String CompositorInstance::getMRTTexLocalName(const String& baseName, size_t attachment)
{
	return baseName + "/" + StringConverter::toString(attachment);
}
//-----------------------------------------------------------------------
void CompositorInstance::freeResources()
{
    /// Remove temporary textures
    /// LocalTextureMap mLocalTextures;
    LocalTextureMap::iterator i, iend=mLocalTextures.end();
    for(i=mLocalTextures.begin(); i!=iend; ++i)
    {
        TextureManager::getSingleton().remove(i->second->getName());
    }
    mLocalTextures.clear();

	// Remove MRTs
	LocalMRTMap::iterator mrti, mrtend = mLocalMRTs.end();
	for (mrti = mLocalMRTs.begin(); mrti != mrtend; ++mrti)
	{
		// remove MRT
		Root::getSingleton().getRenderSystem()->destroyRenderTarget(mrti->second->getName());
	}
	mLocalMRTs.clear();
}
//---------------------------------------------------------------------
RenderTarget* CompositorInstance::getRenderTarget(const String& name)
{
	return getTargetForTex(name);
}
//-----------------------------------------------------------------------
RenderTarget *CompositorInstance::getTargetForTex(const String &name)
{
	// try simple texture
	LocalTextureMap::iterator i = mLocalTextures.find(name);
    if(i != mLocalTextures.end())
		return i->second->getBuffer()->getRenderTarget();

	// try MRTs
	LocalMRTMap::iterator mi = mLocalMRTs.find(name);
	if (mi != mLocalMRTs.end())
		return mi->second;
	else
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Non-existent local texture name", 
			"CompositorInstance::getTargetForTex");

}
//-----------------------------------------------------------------------
const String &CompositorInstance::getSourceForTex(const String &name, size_t mrtIndex)
{
	// try simple textures first
    LocalTextureMap::iterator i = mLocalTextures.find(name);
    if(i != mLocalTextures.end())
    {
		return i->second->getName();
    }

	// try MRTs - texture (rather than target)
	i = mLocalTextures.find(getMRTTexLocalName(name, mrtIndex));
	if (i != mLocalTextures.end())
	{
		return i->second->getName();
	}
	else
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Non-existent local texture name", 
			"CompositorInstance::getSourceForTex");
}
//-----------------------------------------------------------------------
void CompositorInstance::queueRenderSystemOp(TargetOperation &finalState, RenderSystemOperation *op)
{
	/// Store operation for current QueueGroup ID
	finalState.renderSystemOperations.push_back(RenderSystemOpPair(finalState.currentQueueGroupID, op));
	/// Tell parent for deletion
	mChain->_queuedOperation(op);
}
//-----------------------------------------------------------------------
void CompositorInstance::addListener(Listener *l)
{
	mListeners.push_back(l);
}
//-----------------------------------------------------------------------
void CompositorInstance::removeListener(Listener *l)
{
	mListeners.erase(std::find(mListeners.begin(), mListeners.end(), l));
}
//-----------------------------------------------------------------------
void CompositorInstance::_fireNotifyMaterialSetup(uint32 pass_id, MaterialPtr &mat)
{
	Listeners::iterator i, iend=mListeners.end();
	for(i=mListeners.begin(); i!=iend; ++i)
		(*i)->notifyMaterialSetup(pass_id, mat);
}
//-----------------------------------------------------------------------
void CompositorInstance::_fireNotifyMaterialRender(uint32 pass_id, MaterialPtr &mat)
{
	Listeners::iterator i, iend=mListeners.end();
	for(i=mListeners.begin(); i!=iend; ++i)
		(*i)->notifyMaterialRender(pass_id, mat);
}
//-----------------------------------------------------------------------
CompositorInstance::RenderSystemOperation::~RenderSystemOperation()
{
}
//-----------------------------------------------------------------------
CompositorInstance::Listener::~Listener()
{
}
void CompositorInstance::Listener::notifyMaterialSetup(uint32 pass_id, MaterialPtr &mat)
{
}
void CompositorInstance::Listener::notifyMaterialRender(uint32 pass_id, MaterialPtr &mat)
{
}
//-----------------------------------------------------------------------

}
