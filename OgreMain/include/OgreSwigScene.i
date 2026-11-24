// Ogre SWIG bindings: movable objects, nodes, lights, cameras, entities, particles, level of detail, scene manager, root.
// Part of Ogre.i. The python bindings build each part as a separate module, see Components/Python.

#ifdef OGRE_SWIG_PART
// built as a separate module: these traits are generated with the templates in OgreSwigAnimation.i
PTR_TRAITS(Ogre::Animation)
PTR_TRAITS(Ogre::AnimationState)
PTR_TRAITS(Ogre::StaticGeometry)
#endif

%include "OgreBillboard.h"
%ignore Ogre::Particle::hasOwnDimensions ; // deprecated
%include "OgreParticle.h"
    SHARED_PTR(ParticleAffector);
    %include "OgreParticleAffector.h"
        %include "OgreParticleAffectorFactory.h"
//    SHARED_PTR(ParticleEmitter);
    %include "OgreParticleEmitter.h"
        %include "OgreParticleEmitterFactory.h"
%include "OgreUserObjectBindings.h"
%include "OgreRenderable.h"
%ignore Ogre::ShadowCaster::getShadowVolumeRenderableIterator;
%template(ShadowRenderableList) std::vector<Ogre::ShadowRenderable*>;
%include "OgreShadowCaster.h"
%extend Ogre::MovableObject {
  Entity* castEntity()
  {
    return dynamic_cast<Ogre::Entity*>($self);
  }
}
%extend Ogre::Node {
  SceneNode* castSceneNode()
  {
    return dynamic_cast<Ogre::SceneNode*>($self);
  }
}
%include "OgreMovableObject.h"
    %feature("flatnested") Ogre::BillboardChain::Element;
    %rename(BillboardChain_Element) Ogre::BillboardChain::Element;
    %include "OgreBillboardChain.h"
        %ignore Ogre::RibbonTrail::getNodeIterator;
        %include "OgreRibbonTrail.h"
    %ignore Ogre::BillboardSet::setTextureCoords(const FloatRect*, uint16);
    %template(TextureCoordSets) std::vector<Ogre::FloatRect>;
    %include "OgreBillboardSet.h"
    %include "OgreMovablePlane.h"
    %ignore Ogre::Light::setPosition;
    %ignore Ogre::Light::getPosition;
    %ignore Ogre::Light::getDirection;
    %ignore Ogre::Light::_deriveShadowFarClipDistance(const Camera*) const;
    %include "OgreLight.h"
    %ignore Ogre::Node::getChildIterator;
    %template(NodeList) std::vector<Ogre::Node*>;
    %include "OgreNode.h"
        %include "OgreBone.h"
        %include "OgreTagPoint.h"
        %ignore Ogre::SceneNode::getAttachedObjectIterator;
        %template(MovableObjectList) std::vector<Ogre::MovableObject*>;
        %include "OgreSceneNode.h"
    SHARED_PTR(ShadowCameraSetup);
    SHARED_PTR(DefaultShadowCameraSetup);
    %include "OgreShadowCameraSetup.h"
        SHARED_PTR(FocusedShadowCameraSetup);
        %include "OgreShadowCameraSetupFocused.h"
        SHARED_PTR(LiSPSMShadowCameraSetup);
        %include "OgreShadowCameraSetupLiSPSM.h"
        SHARED_PTR(PlaneOptimalShadowCameraSetup);
        %include "OgreShadowCameraSetupPlaneOptimal.h"
        SHARED_PTR(PSSMShadowCameraSetup);
        %include "OgreShadowCameraSetupPSSM.h"
            %template(SplitPointList) std::vector<Ogre::Real>;
    %ignore Ogre::Frustum::getFrustumExtents(Real&, Real& ,Real& ,Real&) const;
    %include "OgreFrustum.h"
        %ignore Ogre::Camera::setPosition;
        %ignore Ogre::Camera::getPosition;
        %ignore Ogre::Camera::setDirection;
        %ignore Ogre::Camera::getDirection;
        %ignore Ogre::Camera::setOrientation;
        %ignore Ogre::Camera::getOrientation;
        %ignore Ogre::Camera::rotate;
        %ignore Ogre::Camera::getUp;
        %ignore Ogre::Camera::getRight;
        %ignore Ogre::Camera::lookAt;
        %ignore Ogre::Camera::roll;
        %ignore Ogre::Camera::yaw;
        %ignore Ogre::Camera::pitch;
        %ignore Ogre::Camera::setFixedYawAxis;
        %ignore Ogre::Camera::setAutoTracking;
        %ignore Ogre::Camera::move;
        %ignore Ogre::Camera::moveRelative;
        %ignore Ogre::Camera::_renderScene(Viewport*, bool);
        %include "OgreCamera.h"
        ADD_REPR(Camera)
    %include "OgreManualObject.h"
    %template(SubEntityList) std::vector<Ogre::SubEntity*>;
    PTR_TRAITS(Ogre::Entity)
%template(EntitySet) std::set<Ogre::Entity*>;
    %include "OgreEntity.h"
    %ignore Ogre::SubEntity::getAttachedObjectIterator;
    %ignore Ogre::SubEntity::setIndexDataStartIndex;
    %ignore Ogre::SubEntity::getIndexDataStartIndex;
    %ignore Ogre::SubEntity::setIndexDataEndIndex;
    %ignore Ogre::SubEntity::getIndexDataEndIndex;
    %include "OgreSubEntity.h"
    %ignore Ogre::ParticleSystemRenderer::_createVisualData;
    %ignore Ogre::ParticleSystemRenderer::_destroyVisualData;
    %template(ParticleList) std::vector<Ogre::Particle*>;
    %include "OgreParticleSystemRenderer.h"
    SHARED_PTR(ParticleSystem);
    %ignore Ogre::ParticleSystem::_getIterator;
    %include "OgreParticleSystem.h"
    %template(InstancedEntityVec) std::vector<Ogre::InstancedEntity*>;
    %include "OgreInstancedEntity.h"
    %include "OgreInstanceBatch.h"
    %ignore Ogre::SimpleRenderable::setMaterial(const String&);
    %include "OgreSimpleRenderable.h"
        %include "OgreRectangle2D.h"
        %include "OgreWireBoundingBox.h"
%ignore Ogre::ParticleSystemManager::getTemplateIterator;
%ignore Ogre::ParticleSystemManager::getEmitterFactoryIterator;
%ignore Ogre::ParticleSystemManager::getAffectorFactoryIterator;
%ignore Ogre::ParticleSystemManager::getRendererFactoryIterator;
%include "OgreParticleSystemManager.h"
%ignore Ogre::InstanceManager::getInstanceBatchIterator;
%ignore Ogre::InstanceManager::getInstanceBatchMapIterator;
%include "OgreInstanceManager.h" 
%ignore Ogre::StaticGeometry::getRegionIterator;
%ignore Ogre::StaticGeometry::Region::getLODIterator;
%ignore Ogre::StaticGeometry::MaterialBucket::getGeometryIterator;
%ignore Ogre::StaticGeometry::LODBucket::getMaterialIterator;
#ifdef SWIGPYTHON
%{
    // this is a workaround for the following map
    namespace swig {
    template<> struct traits<Ogre::StaticGeometry::Region> {
        typedef pointer_category category;
        static const char* type_name() { return "Ogre::StaticGeometry::Region"; }
    };
    }
%}
#endif
#ifndef SWIGJAVA
%template(RegionMap) std::map<uint32_t, Ogre::StaticGeometry::Region*>;
#endif
#ifdef SWIGPYTHON
%{
    // this is a workaround for the following map
    namespace swig {
    template<> struct traits<Ogre::StaticGeometry::MaterialBucket> {
        typedef pointer_category category;
        static const char* type_name() { return "Ogre::StaticGeometry::MaterialBucket"; }
    };
    }
%}
#endif
%template(MaterialBucketMap) std::map<std::string, Ogre::StaticGeometry::MaterialBucket*>;
%template(LODBucketList) std::vector<Ogre::StaticGeometry::LODBucket*>;
%template(GeometryBucketList) std::vector<Ogre::StaticGeometry::GeometryBucket*>;
%include "OgreStaticGeometry.h"
%template(MeshLodUsageList) std::vector<Ogre::MeshLodUsage>;
%include "OgreLodStrategy.h"
%include "OgrePixelCountLodStrategy.h"
%include "OgreDistanceLodStrategy.h"
%ignore Ogre::LodStrategyManager::getIterator;
%include "OgreLodStrategyManager.h"
%ignore Ogre::SceneManager::getCameraIterator; // deprecated
%ignore Ogre::SceneManager::getAnimationIterator;
%ignore Ogre::SceneManager::getAnimationStateIterator;
%ignore Ogre::SceneManager::getMovableObjectIterator;
%ignore Ogre::SceneManager::getShadowTextureCount;
%ignore Ogre::SceneManager::getShadowTextureConfigIterator;
%ignore Ogre::SceneManager::getShadowCasterBoundsInfo;
%ignore Ogre::SceneManager::getSkyDomeGenParameters;
%ignore Ogre::SceneManager::getSkyDomeNode;
%ignore Ogre::SceneManager::isSkyDomeEnabled;
%ignore Ogre::SceneManager::setSkyDomeEnabled;
%ignore Ogre::SceneManager::getSkyBoxGenParameters;
%ignore Ogre::SceneManager::getSkyBoxNode;
%ignore Ogre::SceneManager::isSkyBoxEnabled;
%ignore Ogre::SceneManager::setSkyBoxEnabled;
%ignore Ogre::SceneManager::getSkyPlaneGenParameters;
%ignore Ogre::SceneManager::getSkyPlaneNode;
%ignore Ogre::SceneManager::isSkyPlaneEnabled;
%ignore Ogre::SceneManager::setSkyPlaneEnabled;
%newobject Ogre::SceneManager::createRayQuery(const Ray&, uint32 mask);
%newobject Ogre::SceneManager::createRayQuery(const Ray&);
%rename(SceneManager_Listener) Ogre::SceneManager::Listener;
%template(MovableObjectMap) std::map<std::string, Ogre::MovableObject*>;
%template(StaticGeometryMap) std::map<std::string, Ogre::StaticGeometry*>;
%template(ConstCameraList) std::vector<const Ogre::Camera*>;
%template(CameraMap) std::map<std::string, Ogre::Camera*>;
namespace Ogre { struct ShadowTextureConfig; }
%template(ShadowTextureConfigList) std::vector<Ogre::ShadowTextureConfig>;
%include "OgreSceneManager.h"
%include "OgreDefaultDebugDrawer.h"
%include "OgreConfigDialog.h"
%template(RenderSystemList) std::vector<Ogre::RenderSystem*>;
%ignore Ogre::Root::createSceneManager(uint16);
%ignore Ogre::Root::createSceneManager(uint16, const String&);
%ignore Ogre::Root::getMovableObjectFactoryIterator;
#ifdef SWIGPYTHON
%{
class ThreadAllowFrameListener : public Ogre::FrameListener {
    PyThreadState* _save = 0;
public:
    bool frameRenderingQueued(const Ogre::FrameEvent& evt)
    {
        if(!_save)
            _save = PyEval_SaveThread();
        return true;
    }
    bool frameEnded(const Ogre::FrameEvent& evt)
    {
        if(_save) {
            PyEval_RestoreThread(_save);
            _save = 0;
        }
        return true;
    }
};
%}
%extend Ogre::Root {
    void allowPyThread()
    {
        static ThreadAllowFrameListener listener;
        $self->addFrameListener(&listener);
    }
}
#endif
PTR_TRAITS(Ogre::SceneManager)
PTR_TRAITS(Ogre::MovableObjectFactory)
%template(SceneManagerInstanceMap) std::map<std::string, Ogre::SceneManager*>;
%template(MovableObjectFactoryMap) std::map<std::string, Ogre::MovableObjectFactory*>;
%include "OgreRoot.h"
