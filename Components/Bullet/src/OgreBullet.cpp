// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include "OgreBullet.h"

namespace Ogre
{
namespace Bullet
{

typedef std::vector<Vector3> Vector3Array;
typedef std::pair<unsigned short, Vector3Array*> BoneKeyIndex;

btSphereShape* createSphereCollider(const MovableObject* mo)
{
    OgreAssert(mo->getParentSceneNode(), "MovableObject must be attached");

    auto shape = new btSphereShape(mo->getBoundingRadius());
    shape->setLocalScaling(convert(mo->getParentSceneNode()->getScale()));

    return shape;
}
btBoxShape* createBoxCollider(const MovableObject* mo)
{
    OgreAssert(mo->getParentSceneNode(), "MovableObject must be attached");

    auto shape = new btBoxShape(convert(mo->getBoundingBox().getHalfSize()));
    shape->setLocalScaling(convert(mo->getParentSceneNode()->getScale()));

    return shape;
}

btCapsuleShape* createCapsuleCollider(const MovableObject* mo)
{
    OgreAssert(mo->getParentSceneNode(), "MovableObject must be attached");

    auto sz = mo->getBoundingBox().getHalfSize();

    btScalar height = std::max(sz.x, std::max(sz.y, sz.z));
    btScalar radius;
    btCapsuleShape* shape;
    // Orient the capsule such that its height is aligned with the largest dimension.
    if (height == sz.y)
    {
        radius = std::max(sz.x, sz.z);
        shape = new btCapsuleShape(radius, 2 * height - 2 * radius);
    }
    else if (height == sz.x)
    {
        radius = std::max(sz.y, sz.z);
        shape = new btCapsuleShapeX(radius, 2 * height - 2 * radius);
    }
    else
    {
        radius = std::max(sz.x, sz.y);
        shape = new btCapsuleShapeZ(radius, 2 * height - 2 * radius);
    }

    shape->setLocalScaling(convert(mo->getParentSceneNode()->getScale()));

    return shape;
}

/// create capsule collider using ogre provided data
btCylinderShape* createCylinderCollider(const MovableObject* mo)
{
    OgreAssert(mo->getParentSceneNode(), "MovableObject must be attached");

    auto sz = convert(mo->getBoundingBox().getHalfSize());

    btScalar height = std::max(sz.x(), std::max(sz.y(), sz.z()));
    btCylinderShape* shape;
    // Orient the capsule such that its height is aligned with the largest dimension.
    if (height == sz.y())
    {
        shape = new btCylinderShape(sz);
    }
    else if (height == sz.x())
    {
        shape = new btCylinderShapeX(sz);
    }
    else
    {
        shape = new btCylinderShapeZ(sz);
    }

    shape->setLocalScaling(convert(mo->getParentSceneNode()->getScale()));

    return shape;
}

struct EntityCollisionListener
{
    const MovableObject* entity;
    CollisionListener* listener;
};

static void onTick(btDynamicsWorld* world, btScalar timeStep)
{
    int numManifolds = world->getDispatcher()->getNumManifolds();
    auto manifolds = world->getDispatcher()->getInternalManifoldPointer();
    for (int i = 0; i < numManifolds; i++)
    {
        btPersistentManifold* manifold = manifolds[i];

        for (int j = 0; j < manifold->getNumContacts(); j++)
        {
            const btManifoldPoint& mp = manifold->getContactPoint(i);
            auto body0 = static_cast<EntityCollisionListener*>(manifold->getBody0()->getUserPointer());
            auto body1 = static_cast<EntityCollisionListener*>(manifold->getBody1()->getUserPointer());
            if (body0->listener)
                body0->listener->contact(body1->entity, mp);
            if (body1->listener)
                body1->listener->contact(body0->entity, mp);
        }
    }
}

class VertexIndexToShape
{
public:
    VertexIndexToShape(const Affine3& transform = Affine3::IDENTITY);
    VertexIndexToShape(Renderable* rend, const Affine3& transform = Affine3::IDENTITY);
    VertexIndexToShape(const Entity* entity, const Affine3& transform = Affine3::IDENTITY);
    ~VertexIndexToShape();

    Real getRadius();
    Vector3 getSize();

    btBvhTriangleMeshShape* createTrimesh();
    btConvexHullShape* createConvex();

    void addEntity(const Entity* entity, const Affine3& transform = Affine3::IDENTITY);
    void addMesh(const MeshPtr& mesh, const Affine3& transform = Affine3::IDENTITY);

    const Vector3* getVertices() { return mVertexBuffer; }
    unsigned int getVertexCount() { return mVertexCount; };

private:
    void addStaticVertexData(const VertexData* vertex_data);

    void addAnimatedVertexData(const VertexData* vertex_data, const VertexData* blended_data,
                               const Mesh::IndexMap* indexMap);

    void addIndexData(IndexData* data, const unsigned int offset = 0);

    Vector3* mVertexBuffer;
    unsigned int* mIndexBuffer;
    unsigned int mVertexCount;
    unsigned int mIndexCount;

    Vector3 mBounds;
    Real mBoundRadius;

    typedef std::map<unsigned char, std::vector<Vector3>*> BoneIndex;
    BoneIndex* mBoneIndex;

    Affine3 mTransform;

    Vector3 mScale;
};

/// wrapper with automatic memory management
class CollisionObject
{
protected:
    btCollisionObject* mBtBody;
    btCollisionWorld* mBtWorld;

public:
    CollisionObject(btCollisionObject* btBody, btCollisionWorld* btWorld) : mBtBody(btBody), mBtWorld(btWorld) {}
    virtual ~CollisionObject()
    {
        mBtWorld->removeCollisionObject(mBtBody);
        delete mBtBody->getCollisionShape();
        delete mBtBody;
    }
};
class RigidBody : public CollisionObject
{
public:
    using CollisionObject::CollisionObject;

    ~RigidBody()
    {
        delete (EntityCollisionListener*)(mBtBody)->getUserPointer();
        delete ((btRigidBody*)mBtBody)->getMotionState();
    }
};

DynamicsWorld::DynamicsWorld(const Vector3& gravity) : CollisionWorld(NULL) // prevent CollisionWorld from creating a world
{
    // Bullet initialisation.
    mCollisionConfig.reset(new btDefaultCollisionConfiguration());
    mDispatcher.reset(new btCollisionDispatcher(mCollisionConfig.get()));
    mSolver.reset(new btSequentialImpulseConstraintSolver());
    mBroadphase.reset(new btDbvtBroadphase());

    auto btworld = new btDiscreteDynamicsWorld(mDispatcher.get(), mBroadphase.get(), mSolver.get(), mCollisionConfig.get());
    btworld->setGravity(convert(gravity));
    btworld->setInternalTickCallback(onTick);
    mBtWorld = btworld;
}

static btCollisionShape* getCollisionShape(Entity* ent, ColliderType ct)
{
    if (ent->hasSkeleton())
    {
        ent->addSoftwareAnimationRequest(false);
        ent->_updateAnimation();
        ent->setUpdateBoundingBoxFromSkeleton(true);
    }

    btCollisionShape* cs = NULL;
    switch (ct)
    {
    case CT_BOX:
        cs = createBoxCollider(ent);
        break;
    case CT_SPHERE:
        cs = createSphereCollider(ent);
        break;
    case CT_CYLINDER:
        cs = createCylinderCollider(ent);
        break;
    case CT_CAPSULE:
        cs = createCapsuleCollider(ent);
        break;
    case CT_TRIMESH:
        cs = VertexIndexToShape(ent).createTrimesh();
        break;
    case CT_HULL:
        cs = VertexIndexToShape(ent).createConvex();
        break;
    }

    if (ent->hasSkeleton())
        ent->removeSoftwareAnimationRequest(false);

    return cs;
}

btRigidBody* DynamicsWorld::addRigidBody(float mass, Entity* ent, ColliderType ct, CollisionListener* listener,
                                         int group, int mask)
{
    auto node = ent->getParentSceneNode();
    OgreAssert(node, "entity must be attached");
    RigidBodyState* state = new RigidBodyState(node);

    btCollisionShape* cs = getCollisionShape(ent, ct);

    btVector3 inertia(0, 0, 0);
    if (mass != 0) // mass = 0 -> static
        cs->calculateLocalInertia(mass, inertia);

    auto rb = new btRigidBody(mass, state, cs, inertia);
    getBtWorld()->addRigidBody(rb, group, mask);
    rb->setUserPointer(new EntityCollisionListener{ent, listener});

    // transfer ownership to node
    auto objWrapper = std::make_shared<RigidBody>(rb, mBtWorld);
    node->getUserObjectBindings().setUserAny("BtCollisionObject", objWrapper);

    return rb;
}

btCollisionObject* CollisionWorld::addCollisionObject(Entity* ent, ColliderType ct, int group, int mask)
{
    auto node = ent->getParentSceneNode();
    OgreAssert(node, "entity must be attached");

    btCollisionShape* cs = getCollisionShape(ent, ct);

    auto co = new btCollisionObject();
    co->setCollisionShape(cs);
    mBtWorld->addCollisionObject(co, group, mask);

    // transfer ownership to node
    auto objWrapper = std::make_shared<CollisionObject>(co, mBtWorld);
    node->getUserObjectBindings().setUserAny("BtCollisionObject", objWrapper);

    return co;
}

struct RayResultCallbackWrapper : public btCollisionWorld::RayResultCallback
{
    Bullet::RayResultCallback* mCallback;
    float mMaxDistance;
    RayResultCallbackWrapper(Bullet::RayResultCallback* callback, float maxDist)
        : mCallback(callback), mMaxDistance(maxDist)
    {
    }
    btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) override
    {
        auto body0 = static_cast<const EntityCollisionListener*>(rayResult.m_collisionObject->getUserPointer());
        mCallback->addSingleResult(body0->entity, rayResult.m_hitFraction * mMaxDistance);
        return rayResult.m_hitFraction;
    }
};

void CollisionWorld::rayTest(const Ray& ray, RayResultCallback* callback, float maxDist)
{
    RayResultCallbackWrapper wrapper(callback, maxDist);
    btVector3 from = convert(ray.getOrigin());
    btVector3 to = convert(ray.getPoint(maxDist));
    mBtWorld->rayTest(from, to, wrapper);
}

CollisionWorld::~CollisionWorld() { delete mBtWorld; }

/*
 * =============================================================================================
 * BtVertexIndexToShape
 * =============================================================================================
 */

void VertexIndexToShape::addStaticVertexData(const VertexData* vertex_data)
{
    if (!vertex_data)
        return;

    const VertexData* data = vertex_data;

    const unsigned int prev_size = mVertexCount;
    mVertexCount += (unsigned int)data->vertexCount;

    Vector3* tmp_vert = new Vector3[mVertexCount];
    if (mVertexBuffer)
    {
        memcpy(tmp_vert, mVertexBuffer, sizeof(Vector3) * prev_size);
        delete[] mVertexBuffer;
    }
    mVertexBuffer = tmp_vert;

    // Get the positional buffer element
    {
        const VertexElement* posElem = data->vertexDeclaration->findElementBySemantic(VES_POSITION);
        HardwareVertexBufferSharedPtr vbuf = data->vertexBufferBinding->getBuffer(posElem->getSource());
        const unsigned int vSize = (unsigned int)vbuf->getVertexSize();

        unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(HardwareBuffer::HBL_READ_ONLY));
        float* pReal;
        Vector3* curVertices = &mVertexBuffer[prev_size];
        const unsigned int vertexCount = (unsigned int)data->vertexCount;
        for (unsigned int j = 0; j < vertexCount; ++j)
        {
            posElem->baseVertexPointerToElement(vertex, &pReal);
            vertex += vSize;

            curVertices->x = (*pReal++);
            curVertices->y = (*pReal++);
            curVertices->z = (*pReal++);

            *curVertices = mTransform * (*curVertices);

            curVertices++;
        }
        vbuf->unlock();
    }
}
//------------------------------------------------------------------------------------------------
void VertexIndexToShape::addAnimatedVertexData(const VertexData* vertex_data, const VertexData* blend_data,
                                               const Mesh::IndexMap* indexMap)
{
    // Get the bone index element
    assert(vertex_data);

    const VertexData* data = blend_data;
    const unsigned int prev_size = mVertexCount;
    mVertexCount += (unsigned int)data->vertexCount;
    Vector3* tmp_vert = new Vector3[mVertexCount];
    if (mVertexBuffer)
    {
        memcpy(tmp_vert, mVertexBuffer, sizeof(Vector3) * prev_size);
        delete[] mVertexBuffer;
    }
    mVertexBuffer = tmp_vert;

    // Get the positional buffer element
    {
        const VertexElement* posElem = data->vertexDeclaration->findElementBySemantic(VES_POSITION);
        assert(posElem);
        HardwareVertexBufferSharedPtr vbuf = data->vertexBufferBinding->getBuffer(posElem->getSource());
        const unsigned int vSize = (unsigned int)vbuf->getVertexSize();

        unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(HardwareBuffer::HBL_READ_ONLY));
        float* pReal;
        Vector3* curVertices = &mVertexBuffer[prev_size];
        const unsigned int vertexCount = (unsigned int)data->vertexCount;
        for (unsigned int j = 0; j < vertexCount; ++j)
        {
            posElem->baseVertexPointerToElement(vertex, &pReal);
            vertex += vSize;

            curVertices->x = (*pReal++);
            curVertices->y = (*pReal++);
            curVertices->z = (*pReal++);

            *curVertices = mTransform * (*curVertices);

            curVertices++;
        }
        vbuf->unlock();
    }
    {
        const VertexElement* bneElem = vertex_data->vertexDeclaration->findElementBySemantic(VES_BLEND_INDICES);
        assert(bneElem);

        HardwareVertexBufferSharedPtr vbuf = vertex_data->vertexBufferBinding->getBuffer(bneElem->getSource());
        const unsigned int vSize = (unsigned int)vbuf->getVertexSize();
        unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(HardwareBuffer::HBL_READ_ONLY));

        unsigned char* pBone;

        if (!mBoneIndex)
            mBoneIndex = new BoneIndex();
        BoneIndex::iterator i;

        Vector3* curVertices = &mVertexBuffer[prev_size];

        const unsigned int vertexCount = (unsigned int)vertex_data->vertexCount;
        for (unsigned int j = 0; j < vertexCount; ++j)
        {
            bneElem->baseVertexPointerToElement(vertex, &pBone);
            vertex += vSize;

            const unsigned char currBone = (indexMap) ? (*indexMap)[*pBone] : *pBone;
            i = mBoneIndex->find(currBone);
            Vector3Array* l = 0;
            if (i == mBoneIndex->end())
            {
                l = new Vector3Array;
                mBoneIndex->emplace(currBone, l);
            }
            else
            {
                l = i->second;
            }

            l->push_back(*curVertices);

            curVertices++;
        }
        vbuf->unlock();
    }
}
//------------------------------------------------------------------------------------------------
void VertexIndexToShape::addIndexData(IndexData* data, const unsigned int offset)
{
    const unsigned int prev_size = mIndexCount;
    mIndexCount += (unsigned int)data->indexCount;

    unsigned int* tmp_ind = new unsigned int[mIndexCount];
    if (mIndexBuffer)
    {
        memcpy(tmp_ind, mIndexBuffer, sizeof(unsigned int) * prev_size);
        delete[] mIndexBuffer;
    }
    mIndexBuffer = tmp_ind;

    const unsigned int numTris = (unsigned int)data->indexCount / 3;
    HardwareIndexBufferSharedPtr ibuf = data->indexBuffer;
    const bool use32bitindexes = (ibuf->getType() == HardwareIndexBuffer::IT_32BIT);
    unsigned int index_offset = prev_size;

    if (use32bitindexes)
    {
        const unsigned int* pInt = static_cast<unsigned int*>(ibuf->lock(HardwareBuffer::HBL_READ_ONLY));
        for (unsigned int k = 0; k < numTris; ++k)
        {
            mIndexBuffer[index_offset++] = offset + *pInt++;
            mIndexBuffer[index_offset++] = offset + *pInt++;
            mIndexBuffer[index_offset++] = offset + *pInt++;
        }
        ibuf->unlock();
    }
    else
    {
        const unsigned short* pShort = static_cast<unsigned short*>(ibuf->lock(HardwareBuffer::HBL_READ_ONLY));
        for (unsigned int k = 0; k < numTris; ++k)
        {
            mIndexBuffer[index_offset++] = offset + static_cast<unsigned int>(*pShort++);
            mIndexBuffer[index_offset++] = offset + static_cast<unsigned int>(*pShort++);
            mIndexBuffer[index_offset++] = offset + static_cast<unsigned int>(*pShort++);
        }
        ibuf->unlock();
    }
}
//------------------------------------------------------------------------------------------------
Real VertexIndexToShape::getRadius()
{
    if (mBoundRadius == (-1))
    {
        getSize();
        mBoundRadius = (std::max(mBounds.x, std::max(mBounds.y, mBounds.z)) * 0.5);
    }
    return mBoundRadius;
}
//------------------------------------------------------------------------------------------------
Vector3 VertexIndexToShape::getSize()
{
    const unsigned int vCount = getVertexCount();
    if (mBounds == Vector3(-1, -1, -1) && vCount > 0)
    {

        const Vector3* const v = getVertices();

        Vector3 vmin(v[0]);
        Vector3 vmax(v[0]);

        for (unsigned int j = 1; j < vCount; j++)
        {
            vmin.x = std::min(vmin.x, v[j].x);
            vmin.y = std::min(vmin.y, v[j].y);
            vmin.z = std::min(vmin.z, v[j].z);

            vmax.x = std::max(vmax.x, v[j].x);
            vmax.y = std::max(vmax.y, v[j].y);
            vmax.z = std::max(vmax.z, v[j].z);
        }

        mBounds.x = vmax.x - vmin.x;
        mBounds.y = vmax.y - vmin.y;
        mBounds.z = vmax.z - vmin.z;
    }

    return mBounds;
}
//------------------------------------------------------------------------------------------------
btConvexHullShape* VertexIndexToShape::createConvex()
{
    assert(mVertexCount && (mIndexCount >= 6) && ("Mesh must have some vertices and at least 6 indices (2 triangles)"));

    btConvexHullShape* shape = new btConvexHullShape((btScalar*)&mVertexBuffer[0].x, mVertexCount, sizeof(Vector3));

    shape->setLocalScaling(convert(mScale));

    return shape;
}
//------------------------------------------------------------------------------------------------
btBvhTriangleMeshShape* VertexIndexToShape::createTrimesh()
{
    assert(mVertexCount && (mIndexCount >= 6) && ("Mesh must have some vertices and at least 6 indices (2 triangles)"));

    unsigned int numFaces = mIndexCount / 3;

    btTriangleMesh* trimesh = new btTriangleMesh();
    unsigned int* indices = mIndexBuffer;
    Vector3* vertices = mVertexBuffer;

    btVector3 vertexPos[3];
    for (unsigned int n = 0; n < numFaces; ++n)
    {
        {
            const Vector3& vec = vertices[*indices];
            vertexPos[0][0] = vec.x;
            vertexPos[0][1] = vec.y;
            vertexPos[0][2] = vec.z;
        }
        {
            const Vector3& vec = vertices[*(indices + 1)];
            vertexPos[1][0] = vec.x;
            vertexPos[1][1] = vec.y;
            vertexPos[1][2] = vec.z;
        }
        {
            const Vector3& vec = vertices[*(indices + 2)];
            vertexPos[2][0] = vec.x;
            vertexPos[2][1] = vec.y;
            vertexPos[2][2] = vec.z;
        }

        indices += 3;

        trimesh->addTriangle(vertexPos[0], vertexPos[1], vertexPos[2]);
    }

    const bool useQuantizedAABB = true;
    btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape(trimesh, useQuantizedAABB);

    shape->setLocalScaling(convert(mScale));

    return shape;
}
//------------------------------------------------------------------------------------------------
VertexIndexToShape::~VertexIndexToShape()
{
    delete[] mVertexBuffer;
    delete[] mIndexBuffer;

    if (mBoneIndex)
    {
        for (auto & i : *mBoneIndex)
        {
            delete i.second;
        }
        delete mBoneIndex;
    }
}
//------------------------------------------------------------------------------------------------
VertexIndexToShape::VertexIndexToShape(const Affine3& transform)
    : mVertexBuffer(0), mIndexBuffer(0), mVertexCount(0), mIndexCount(0), mBounds(Vector3(-1, -1, -1)),
      mBoundRadius(-1), mBoneIndex(0), mTransform(transform), mScale(1)
{
}
//------------------------------------------------------------------------------------------------
VertexIndexToShape::VertexIndexToShape(const Entity* entity, const Affine3& transform) : VertexIndexToShape(transform)
{
    addEntity(entity, transform);
}
//------------------------------------------------------------------------------------------------
VertexIndexToShape::VertexIndexToShape(Renderable* rend, const Affine3& transform) : VertexIndexToShape(transform)
{
    RenderOperation op;
    rend->getRenderOperation(op);
    addStaticVertexData(op.vertexData);
    if (op.useIndexes)
        addIndexData(op.indexData);
}
//------------------------------------------------------------------------------------------------
void VertexIndexToShape::addEntity(const Entity* entity, const Affine3& transform)
{
    // Each entity added need to reset size and radius
    // next time getRadius and getSize are asked, they're computed.
    mBounds = Vector3(-1, -1, -1);
    mBoundRadius = -1;

    auto node = entity->getParentSceneNode();
    mTransform = transform;
    mScale = node ? node->getScale() : Vector3(1, 1, 1);

    bool hasSkeleton = entity->hasSkeleton();

    if (entity->getMesh()->sharedVertexData)
    {
        if (hasSkeleton)
            addAnimatedVertexData(entity->getMesh()->sharedVertexData, entity->_getSkelAnimVertexData(),
                                  &entity->getMesh()->sharedBlendIndexToBoneIndexMap);
        else
            addStaticVertexData(entity->getMesh()->sharedVertexData);
    }

    for (unsigned int i = 0; i < entity->getNumSubEntities(); ++i)
    {
        SubMesh* sub_mesh = entity->getSubEntity(i)->getSubMesh();

        if (!sub_mesh->useSharedVertices)
        {
            addIndexData(sub_mesh->indexData, mVertexCount);

            if (hasSkeleton)
                addAnimatedVertexData(sub_mesh->vertexData, entity->getSubEntity(i)->_getSkelAnimVertexData(),
                                      &sub_mesh->blendIndexToBoneIndexMap);
            else
                addStaticVertexData(sub_mesh->vertexData);
        }
        else
        {
            addIndexData(sub_mesh->indexData);
        }
    }
}
//------------------------------------------------------------------------------------------------
void VertexIndexToShape::addMesh(const MeshPtr& mesh, const Affine3& transform)
{
    // Each entity added need to reset size and radius
    // next time getRadius and getSize are asked, they're computed.
    mBounds = Vector3(-1, -1, -1);
    mBoundRadius = -1;

    mTransform = transform;

    if (mesh->hasSkeleton())
        LogManager::getSingleton().logWarning("Mesh " + mesh->getName() + " has a skeleton but added non animated");

    if (mesh->sharedVertexData)
    {
        VertexIndexToShape::addStaticVertexData(mesh->sharedVertexData);
    }

    for (unsigned int i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        SubMesh* sub_mesh = mesh->getSubMesh(i);

        if (!sub_mesh->useSharedVertices)
        {
            VertexIndexToShape::addIndexData(sub_mesh->indexData, mVertexCount);
            VertexIndexToShape::addStaticVertexData(sub_mesh->vertexData);
        }
        else
        {
            VertexIndexToShape::addIndexData(sub_mesh->indexData);
        }
    }
}

/*
 * =============================================================================================
 * BtDebugDrawer
 * =============================================================================================
 */
//------------------------------------------------------------------------------------------------
void DebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
    if (mLines.getSections().empty())
    {
        const char* matName = "Ogre/Debug/LinesMat";
        auto mat = MaterialManager::getSingleton().getByName(matName, RGN_INTERNAL);
        if (!mat)
        {
            mat = MaterialManager::getSingleton().create(matName, RGN_INTERNAL);
            auto p = mat->getTechnique(0)->getPass(0);
            p->setLightingEnabled(false);
            p->setVertexColourTracking(TVC_AMBIENT);
        }
        mLines.setBufferUsage(HBU_CPU_TO_GPU);
        mLines.begin(mat, RenderOperation::OT_LINE_LIST);
    }
    else if (mLines.getCurrentVertexCount() == 0)
        mLines.beginUpdate(0);

    ColourValue col(color.x(), color.x(), color.z());
    mLines.position(convert(from));
    mLines.colour(col);
    mLines.position(convert(to));
    mLines.colour(col);
}
} // namespace Bullet
} // namespace Ogre
