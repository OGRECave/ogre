// Ogre SWIG bindings: materials, texture units, passes, compositors, render system, render targets, render queue.
// Part of Ogre.i. The python bindings build each part as a separate module, see Components/Python.

%feature("director") Ogre::RenderObjectListener;
%include "OgreRenderObjectListener.h"
%feature("director") Ogre::RenderQueueListener;
%include "OgreRenderQueueListener.h"
%feature("director") Ogre::RenderTargetListener;
%include "OgreRenderTargetListener.h"
#ifdef SWIGCSHARP
%extend Ogre::PixelBox
{
    void* getData() { return $self->data; }
}
#endif
%include "OgreBlendMode.h"
%ignore Ogre::TextureUnitState::setCubicTexture;
%ignore Ogre::TextureUnitState::setCubicTextureName;
%ignore Ogre::TextureUnitState::isCubic;
%ignore Ogre::TextureUnitState::is3D;
%ignore Ogre::TextureUnitState::setBindingType;
%ignore Ogre::TextureUnitState::getBindingType;
%ignore Ogre::TextureUnitState::setIsAlpha;
%ignore Ogre::TextureUnitState::setTextureNameAlias;
%ignore Ogre::TextureUnitState::getTextureNameAlias;
%ignore Ogre::TextureUnitState::setAnimatedTextureName( const String* const, size_t, Real = 0 );
%template(UIntPair) std::pair<Ogre::uint32, Ogre::uint32>;
%template(UIntList) std::vector<Ogre::uint32>;
%include "OgreTextureUnitState.h"
SHARED_PTR(Compositor);
%ignore Ogre::Compositor::getTechniqueIterator;
%ignore Ogre::Compositor::getSupportedTechniqueIterator;
%include "OgreCompositor.h"
%ignore Ogre::CompositionTechnique::getNumTextureDefinitions;
%ignore Ogre::CompositionTechnique::getTextureDefinitionIterator;
%ignore Ogre::CompositionTechnique::getTargetPass;
%ignore Ogre::CompositionTechnique::getNumTargetPasses;
%ignore Ogre::CompositionTechnique::getTargetPassIterator;
%template(CompositionTargetPassList) std::vector<Ogre::CompositionTargetPass*>;
%include "OgreCompositionTechnique.h"
%ignore Ogre::CompositionTargetPass::getPass;
%ignore Ogre::CompositionTargetPass::getNumPasses;
%ignore Ogre::CompositionTargetPass::getPassIterator;
%template(CompositionPassList) std::vector<Ogre::CompositionPass*>;
%include "OgreCompositionTargetPass.h"
    // ambiguity as enums are just ints
    %ignore Ogre::TextureManager::createManual(const String&, const String&,TextureType,uint,uint,int,PixelFormat,int);
    %ignore Ogre::TextureManager::createManual(const String&, const String&,TextureType,uint,uint,int,PixelFormat);
    %ignore Ogre::TextureManager::load(const String&, const String&,TextureType,int,float,bool,PixelFormat=PF_UNKNOWN,bool=false);
    %include "OgreTextureManager.h"
%include "OgreRenderQueue.h"
%ignore Ogre::Material::getLodValueIterator;
%ignore Ogre::Material::getUserLodValueIterator;
%ignore Ogre::Material::getSupportedTechniqueIterator;
%ignore Ogre::Material::getTechniqueIterator;
%ignore Ogre::Material::getSupportedTechnique;
%ignore Ogre::Material::getNumSupportedTechniques;
%ignore Ogre::Material::clone(const String&, bool) const;
%ignore Ogre::Material::clone(const String&, bool, const String&) const;
SHARED_PTR(Material);
%template(TechniqueList) std::vector<Ogre::Technique*>;
%include "OgreMaterial.h"
%ignore Ogre::RenderSystem::_setSceneBlending;
%ignore Ogre::RenderSystem::_setSeparateSceneBlending;
%ignore Ogre::RenderSystem::_setColourBufferWriteEnabled;
%ignore Ogre::RenderSystem::_setFog;
%ignore Ogre::RenderSystem::_setWorldMatrix;
%ignore Ogre::RenderSystem::_setViewMatrix;
%ignore Ogre::RenderSystem::_setProjectionMatrix;
%ignore Ogre::RenderSystem::getRenderTargetIterator;
%ignore Ogre::RenderSystem::getColourVertexElementType;
%ignore Ogre::RenderSystem::setStencilCheckEnabled;
%ignore Ogre::RenderSystem::setStencilBufferParams;
%ignore Ogre::RenderSystem::setGlobalInstanceCount;
%ignore Ogre::RenderSystem::getGlobalInstanceCount;
%ignore Ogre::RenderSystem::setGlobalInstanceVertexDeclaration;
%ignore Ogre::RenderSystem::getGlobalInstanceVertexDeclaration;
%ignore Ogre::RenderSystem::setGlobalInstanceVertexBuffer;
%ignore Ogre::RenderSystem::getGlobalInstanceVertexBuffer;
%ignore Ogre::RenderSystem::setScissorTest(bool, uint32, uint32 = 0, uint32 = 800, uint32 = 600);
%include "OgreRenderSystem.h"
PTR_TRAITS(Ogre::Texture)
%template(UniqueTextureSet) std::set<Ogre::Texture*>;
%include "OgreCompositorManager.h"
%include "OgreCompositorLogic.h"
#ifdef SWIGJAVA
%ignore Ogre::CompositorInstance::Listener; // issue with converting shared_ptr<Material>
#endif
%include "OgreCompositorInstance.h"
%include "OgreCompositionPass.h"
%include "OgreMaterialSerializer.h"
%rename(MaterialManager_Listener) Ogre::MaterialManager::Listener;
%include "OgreMaterialManager.h"
%ignore Ogre::Pass::getTextureUnitStateIterator; // deprecated
%template(TextureUnitStateList) std::vector<Ogre::TextureUnitState*>;
%template(PassSet) std::set<Ogre::Pass*>;
%include "OgrePass.h"
    %ignore Ogre::Technique::getGPUVendorRuleIterator;
    %ignore Ogre::Technique::getGPUDeviceNameRuleIterator;
    %ignore Ogre::Technique::getIlluminationPassIterator;
    %ignore Ogre::Technique::getPassIterator();
    %template(PassList) std::vector<Ogre::Pass*>;
    %template(IlluminationPassList) std::vector<Ogre::IlluminationPass*>;
    %include "OgreTechnique.h"
%ignore Ogre::RenderTarget::copyContentsToMemory(const PixelBox&);
%ignore Ogre::RenderTarget::copyContentsToMemory(const PixelBox&, FrameBuffer); // deprecated
%feature("flatnested") Ogre::RenderTarget::FrameStats;
%include "OgreRenderTarget.h"
#ifdef __ANDROID__
    %ignore Ogre::RenderWindow::_notifySurfaceCreated(void*);
    %ignore Ogre::RenderWindow::_notifySurfaceCreated(void*, void*);
    %extend Ogre::RenderWindow {
        void _notifySurfaceCreated(jobject surface) {
            ANativeWindow* nativeWnd = ANativeWindow_fromSurface(OgreJNIGetEnv(), surface);
            $self->_notifySurfaceCreated(nativeWnd, NULL);
        }
    }
#endif
    %include "OgreRenderWindow.h"
    %template(BoundSurfaceList) std::vector<Ogre::RenderTexture*>;
    %include "OgreRenderTexture.h"
%ignore Ogre::Viewport::getActualDimensions(int&, int& ,int& ,int&) const;
%include "OgreViewport.h"
%ignore Ogre::CompositorChain::getNumCompositors;
%ignore Ogre::CompositorChain::getCompositor(size_t) const;
%ignore Ogre::CompositorChain::getCompositors;
%template(CompositorInstanceList) std::vector<Ogre::CompositorInstance*>;
%include "OgreCompositorChain.h"
%ignore Ogre::RenderQueueGroup::getIterator;
%template(RenderableList) std::vector<Ogre::Renderable*>;
PTR_TRAITS(Ogre::RenderPriorityGroup)
%template(PriorityMap) std::map<unsigned short, Ogre::RenderPriorityGroup*>;
%include "OgreRenderQueueSortingGrouping.h"
