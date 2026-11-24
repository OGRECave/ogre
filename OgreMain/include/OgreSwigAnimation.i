// Ogre SWIG bindings: animations, skeletons, meshes.
// Part of Ogre.i. The python bindings build each part as a separate module, see Components/Python.

%feature("director") Ogre::LodListener;
%include "OgreLodListener.h"
SHARED_PTR(AnimableValue);
%include "OgreAnimable.h"
// animations
%ignore Ogre::VertexPoseKeyFrame::getPoseReferenceIterator;
%include "OgreKeyFrame.h"
%ignore Ogre::Pose::getNormalsIterator;
%ignore Ogre::Pose::getVertexOffsetIterator;
#ifndef SWIGJAVA // Java std_map iterator typemaps fail for this one
%template(VertexOffsetMap) std::map<Ogre::uint32, Ogre::Vector3>;
#endif
%include "OgrePose.h"
%include "OgreAnimationTrack.h"
%ignore Ogre::AnimationStateSet::getAnimationStateIterator;
%ignore Ogre::AnimationStateSet::getEnabledAnimationStateIterator;
#ifndef SWIGJAVA // older std_list.i clashes with JDK 21 List.removeFirst
%template(EnabledAnimationStateList) std::list<Ogre::AnimationState*>;
#endif
%include "OgreAnimationState.h"

#ifdef SWIGPYTHON
%{
    // this is a workaround for the following maps with swig 3.0.12
    // AnimationState and Camera traits come from the list/vector templates
    namespace swig {
    template<> struct traits<Ogre::StaticGeometry> {
        typedef pointer_category category;
        static const char* type_name() { return "Ogre::StaticGeometry"; }
    };
    }
%}
#endif
%template(AnimationStateMap) std::map<std::string, Ogre::AnimationState*>;
%ignore Ogre::Animation::getVertexTrackIterator;
%ignore Ogre::Animation::getNodeTrackIterator;
%ignore Ogre::Animation::getNumericTrackIterator;
PTR_TRAITS(Ogre::Animation)
PTR_TRAITS(Ogre::NodeAnimationTrack)
PTR_TRAITS(Ogre::NumericAnimationTrack)
PTR_TRAITS(Ogre::VertexAnimationTrack)
%template(AnimationList) std::map<std::string, Ogre::Animation*>;
%template(NodeTrackList) std::map<unsigned short, Ogre::NodeAnimationTrack*>;
%template(NumericTrackList) std::map<unsigned short, Ogre::NumericAnimationTrack*>;
%template(VertexTrackList) std::map<unsigned short, Ogre::VertexAnimationTrack*>;
%template(TrackHandleList) std::set<unsigned short>;
%include "OgreAnimation.h"
SHARED_PTR(Skeleton);
// deprecated
%ignore Ogre::Skeleton::getRootBone;
%ignore Ogre::Skeleton::getRootBoneIterator;
%ignore Ogre::Skeleton::getBoneIterator;
%ignore Ogre::Skeleton::getLinkedSkeletonAnimationSourceIterator;
%template(BoneList) std::vector<Ogre::Bone*>;
namespace Ogre { struct LinkedSkeletonAnimationSource; }
%ignore std::vector<Ogre::LinkedSkeletonAnimationSource>::resize; // non default constructible
%ignore std::vector<Ogre::LinkedSkeletonAnimationSource>::vector(size_type);
%template(LinkedSkeletonAnimSourceList) std::vector<Ogre::LinkedSkeletonAnimationSource>;
%include "OgreSkeleton.h"
    %ignore Ogre::SkeletonInstance::getLinkedSkeletonAnimationSourceIterator;
    SHARED_PTR(SkeletonInstance);
    %include "OgreSkeletonInstance.h"
%include "OgreSkeletonManager.h"
%include "OgreSkeletonSerializer.h"
%include "OgreVertexBoneAssignment.h"
#ifdef SWIGPYTHON
%template(VertexBoneAssignmentList) std::multimap<size_t, Ogre::VertexBoneAssignment>;
#endif
// deprecated
%ignore Ogre::Mesh::getSubMeshIterator;
%ignore Ogre::Mesh::getPoseCount;
%ignore Ogre::Mesh::getPose;
%ignore Ogre::Mesh::getPoseIterator;
%ignore Ogre::Mesh::getBoneAssignmentIterator;
%template(PoseList) std::vector<Ogre::Pose*>;
%template(SubMeshList) std::vector<Ogre::SubMesh*>;
%template(SubMeshNameMap) std::unordered_map<std::string, unsigned short>;
%apply unsigned short& OUTPUT { unsigned short& outSourceCoordSet, unsigned short& outIndex };
SHARED_PTR(Mesh);
%ignore Ogre::Mesh::buildTangentVectors(VertexElementSemantic,unsigned short=0,unsigned short=0,bool=false,bool=false,bool=false);
%ignore Ogre::Mesh::suggestTangentVectorBuildParams(VertexElementSemantic,unsigned short&,unsigned short&);
%include "OgreMesh.h"
%ignore Ogre::SubMesh::getBoneAssignmentIterator;
%template(LODFaceList) std::vector<Ogre::IndexData*>;
%include "OgreSubMesh.h"
%include "OgrePatchSurface.h"
    SHARED_PTR(PatchMesh);
    %include "OgrePatchMesh.h"
%include "OgreMeshSerializer.h"
%include "OgreMeshManager.h"
