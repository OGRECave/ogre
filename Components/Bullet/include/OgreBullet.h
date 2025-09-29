// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#ifndef COMPONENTS_BULLET_H_
#define COMPONENTS_BULLET_H_

#include "OgreBulletExports.h"

#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "Ogre.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

namespace Ogre
{
class TerrainGroup;
class Terrain;
namespace Bullet
{

/** \addtogroup Optional
*  @{
*/
/** \defgroup Bullet Bullet
 * [Bullet-Physics](https://pybullet.org) to %Ogre connection
 * @{
 */

enum ColliderType
{
    CT_BOX,
    CT_SPHERE,
    CT_CYLINDER,
    CT_CAPSULE,
    CT_TRIMESH,
    CT_HULL,
    CT_COMPOUND
};

inline btQuaternion convert(const Quaternion& q) { return btQuaternion(q.x, q.y, q.z, q.w); }
inline btVector3 convert(const Vector3& v) { return btVector3(v.x, v.y, v.z); }

inline Quaternion convert(const btQuaternion& q) { return Quaternion(q.w(), q.x(), q.y(), q.z()); }
inline Vector3 convert(const btVector3& v) { return Vector3(v.x(), v.y(), v.z()); }

/** A MotionState is Bullet's way of informing you about updates to an object.
 * Pass this MotionState to a btRigidBody to have your SceneNode updated automaticaly.
 */
class _OgreBulletExport RigidBodyState : public btMotionState
{
    Node* mNode;

public:
    RigidBodyState(Node* node) : mNode(node) {}

    void getWorldTransform(btTransform& ret) const override
    {
        ret = btTransform(convert(mNode->getOrientation()), convert(mNode->getPosition()));
    }

    void setWorldTransform(const btTransform& in) override
    {
        btQuaternion rot = in.getRotation();
        btVector3 pos = in.getOrigin();
        mNode->setOrientation(rot.w(), rot.x(), rot.y(), rot.z());
        mNode->setPosition(pos.x(), pos.y(), pos.z());
    }
};

/// height field data
struct _OgreBulletExport HeightFieldData {
    /** the position for a center of the shape, i.e. where to place btRigidBody
     *  or a child of btCompoundShape */
    Vector3 bodyPosition;
    /** a heightfield pointer to be freed when
     * btHeightfieldTerrainShape is freed */
    float *terrainHeights;
};

/// create sphere collider using ogre provided data
_OgreBulletExport btSphereShape* createSphereCollider(const MovableObject* mo);
/// create box collider using ogre provided data
_OgreBulletExport btBoxShape* createBoxCollider(const MovableObject* mo);
/// create capsule collider using ogre provided data
_OgreBulletExport btCapsuleShape* createCapsuleCollider(const MovableObject* mo);
/// create capsule collider using ogre provided data
_OgreBulletExport btCylinderShape* createCylinderCollider(const MovableObject* mo);
/// create triMesh collider
_OgreBulletExport btBvhTriangleMeshShape* createTrimeshCollider(const Entity* ent);
/// create convex hull collider
_OgreBulletExport btConvexHullShape* createConvexHullCollider(const Entity* ent);
/// create compound shape
_OgreBulletExport btCompoundShape* createCompoundShape();
/// create height field collider
_OgreBulletExport btHeightfieldTerrainShape* createHeightfieldTerrainShape(const Terrain* terrain, struct HeightFieldData *data);

struct _OgreBulletExport CollisionListener
{
    virtual ~CollisionListener() {}
    /** Called when two objects collide
    * @param other the other object
    * @param manifoldPoint the collision point
    */
    virtual void contact(const MovableObject* other, const btManifoldPoint& manifoldPoint) = 0;
};

struct _OgreBulletExport RayResultCallback
{
    virtual ~RayResultCallback() {}
    /** Called for each object hit by the ray
    * @param other the other object
    * @param distance the distance from ray origin to hit point
    */
    virtual void addSingleResult(const MovableObject* other, float distance) = 0;
};

/// simplified wrapper with automatic memory management
class _OgreBulletExport CollisionWorld
{
protected:
    std::unique_ptr<btCollisionConfiguration> mCollisionConfig;
    std::unique_ptr<btCollisionDispatcher> mDispatcher;
    std::unique_ptr<btBroadphaseInterface> mBroadphase;

    btCollisionWorld* mBtWorld;
    btGhostPairCallback* mGhostPairCallback;

public:
    CollisionWorld(btCollisionWorld* btWorld) : mBtWorld(btWorld), mGhostPairCallback(nullptr) {}
    virtual ~CollisionWorld();

    btCollisionObject* addCollisionObject(Entity* ent, ColliderType ct, int group = 1, int mask = -1);

    void rayTest(const Ray& ray, RayResultCallback* callback, float maxDist = 1000);
    void attachCollisionObject(btCollisionObject *collisionObject, Entity *ent, int group = 1, int mask = -1);
};

/// helper class for kinematic body motion
class _OgreBulletExport KinematicMotionSimple : public btActionInterface
{
    std::vector<btCollisionShape*> mCollisionShapes;
    std::vector<btTransform> mCollisionTransforms;
    btPairCachingGhostObject* mGhostObject;
    btVector3 mCurrentPosition;
    btQuaternion mCurrentOrientation;
    btManifoldArray mManifoldArray;
    btScalar mMaxPenetrationDepth;
    Node* mNode;
    bool mIsOnFloor;
    bool mIsPenetrating;
    int mManifolds;
    bool mAllowManualNarrowPhase;
    virtual bool needsCollision(const btCollisionObject* body0, const btCollisionObject* body1);
    void preStep(btCollisionWorld* collisionWorld);
    void playerStep(btCollisionWorld* collisionWorld, btScalar dt);
    void setupCollisionShapes(btCollisionObject* body);

public:
    KinematicMotionSimple(btPairCachingGhostObject* ghostObject, Node* node);
    ~KinematicMotionSimple();
    bool recoverFromPenetration(btCollisionWorld* collisionWorld);
    virtual void updateAction(btCollisionWorld* collisionWorld, btScalar deltaTimeStep) override;
    virtual void debugDraw(btIDebugDraw* debugDrawer) override;
    bool isOnFloor() const { return mIsOnFloor; }
    bool isPenetrating() const { return mIsPenetrating; }
    int getManifolds() const { return mManifolds; }
    /** Enable manual narrow phase
     * @param enable if enabled
    */
    void enableManualNarrowPhase(bool enable)
    {
        mAllowManualNarrowPhase = enable;
    }
    /** Report manual narrow phase enabled status */
    bool isManualNarrowPhaseEnabled() const { return mAllowManualNarrowPhase; }
};
/// simplified wrapper with automatic memory management
class _OgreBulletExport DynamicsWorld : public CollisionWorld
{
    std::unique_ptr<btConstraintSolver> mSolver;

public:
    explicit DynamicsWorld(const Vector3& gravity);
    DynamicsWorld(btDynamicsWorld* btWorld) : CollisionWorld(btWorld) {}

    /** Add an Entity as a rigid body to the DynamicsWorld
    * @param mass the mass of the object
    * @param ent the entity to control
    * @param ct the collider type
    * @param listener a listener to call on collision with other objects
    * @param group the collision group
    * @param mask the collision mask
    */
    btRigidBody* addRigidBody(float mass, Entity* ent, ColliderType ct, CollisionListener* listener = nullptr,
                              int group = 1, int mask = -1);
    btRigidBody* addKinematicRigidBody(Entity* ent, ColliderType ct, int group = 1, int mask = -1);

    /** Add static body for Ogre terrain
     * @param terrainGroup the TerrainGroup of the terrain
     * @param x x coordinate of the terrain slot
     * @param y y coordinate of the terrain slot
     * @param group the collision group
     * @param mask the collision mask
     * @param debugDraw allow debug drawing
     */
    btRigidBody* addTerrainRigidBody(TerrainGroup* terrainGroup, long x, long y, int group = 1, int mask = -1, bool debugDraw = false);
    /** Add static body for Ogre terrain
     * @param terrain the terrain
     * @param group the collision group
     * @param mask the collision mask
     * @param debugDraw allow debug drawing
     */
    btRigidBody* addTerrainRigidBody(Terrain* terrain, int group = 1, int mask = -1, bool debugDraw = false);

    void attachRigidBody(btRigidBody *rigidBody, Entity *ent, CollisionListener* listener = nullptr,
                              int group = 1, int mask = -1);
    btDynamicsWorld* getBtWorld() const { return static_cast<btDynamicsWorld*>(mBtWorld); }
};

class _OgreBulletExport DebugDrawer : public btIDebugDraw
{
    SceneNode* mNode;
    btCollisionWorld* mWorld;

    ManualObject mLines;
    int mDebugMode;

public:
    DebugDrawer(SceneNode* node, btCollisionWorld* world)
        : mNode(node), mWorld(world), mLines(""), mDebugMode(DBG_DrawWireframe)
    {
        mLines.setCastShadows(false);
        mNode->attachObject(&mLines);
        mWorld->setDebugDrawer(this);
    }

    void update()
    {
        mWorld->debugDrawWorld();
        if (!mLines.getSections().empty()) // begin was called
            mLines.end();
    }

    void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;

    void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime,
                          const btVector3& color) override
    {
        drawLine(PointOnB, PointOnB + normalOnB * distance * 20, color);
    }

    void reportErrorWarning(const char* warningString) override { LogManager::getSingleton().logWarning(warningString); }

    void draw3dText(const btVector3& location, const char* textString) override {}

    void setDebugMode(int mode) override
    {
        mDebugMode = mode;

        if (mDebugMode == DBG_NoDebug)
            clear();
    }

    void clear() { mLines.clear(); }

    int getDebugMode() const override { return mDebugMode; }
};
/** @} */
/** @} */
} // namespace Bullet
} // namespace Ogre

#endif
