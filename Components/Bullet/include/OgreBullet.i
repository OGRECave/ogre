
#ifdef SWIGPYTHON
%module(package="Ogre", directors="1") Bullet
#else
%module(directors="1") OgreBullet
#endif

%{
#include "Ogre.h"
#include "OgreBullet.h"
%}

%include std_string.i
%include exception.i
%include stdint.i
%include typemaps.i
%import "Ogre.i"

%feature("director") Ogre::Bullet::CollisionListener;
%feature("director") Ogre::Bullet::RayResultCallback;

#ifndef SWIGPYTHON
%rename(BulletDebugDrawer) Ogre::Bullet::DebugDrawer;
#endif

// avoid wrapping BtWorld for now..
%extend Ogre::Bullet::DynamicsWorld
{
  int stepSimulation(float timeStep)
  {
    return $self->getBtWorld()->stepSimulation(timeStep);
  }
}

#define _OgreBulletExport

%include "OgreBullet.h"

// bullet subset
#define SIMD_FORCE_INLINE
#define ATTRIBUTE_ALIGNED16(a) a
typedef float btScalar;
%include "LinearMath/btVector3.h"
%include "BulletCollision/NarrowPhaseCollision/btManifoldPoint.h"
%include "BulletCollision/CollisionDispatch/btCollisionObject.h"
%include "BulletDynamics/Dynamics/btRigidBody.h"

%include "BulletCollision/CollisionDispatch/btCollisionWorld.h"
%include "BulletDynamics/Dynamics/btDynamicsWorld.h"