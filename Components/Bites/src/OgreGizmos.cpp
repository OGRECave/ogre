#include "../include/OgreGizmos.h"
#include "OgreManualObject.h"
#include "OgreMaterialManager.h"
#include "OgreSceneManager.h"
#include "OgreSubEntity.h"
#include "OgreTechnique.h"
#include <OgreEntity.h>
#include <iostream>

namespace OgreBites
{
Gizmo::Gizmo(Ogre::SceneManager* sceneManager, Ogre::SceneNode* sceneNode, GizmoMode mode) : mMode(G_NONE)
{
    Ogre::MaterialPtr gizmoMaterial =
        Ogre::MaterialManager::getSingletonPtr()
            ->createOrRetrieve("Gizmo_Material", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME)
            .first.staticCast<Ogre::Material>();

    gizmoMaterial->setReceiveShadows(false);
    gizmoMaterial->getTechnique(0)->getPass(0)->setLightingEnabled(false);
    gizmoMaterial->getTechnique(0)->getPass(0)->setDepthCheckEnabled(false);
    gizmoMaterial->getTechnique(0)->getPass(0)->setCullingMode(Ogre::CULL_NONE);
    gizmoMaterial->getTechnique(0)->getPass(0)->setVertexColourTracking(Ogre::TVC_DIFFUSE);

    createMesh(sceneManager, "AxisGizmosMesh");
    createPlaneMesh(sceneManager, "AxisPlaneMesh");

    attachTo(sceneNode);

    scaleToParent();

    mGizmoX = mGizmoNode->createChildSceneNode("scbnwx", Ogre::Vector3(0, 0, 0), Ogre::Quaternion::IDENTITY);
    mGizmoY = mGizmoNode->createChildSceneNode("scbnwy", Ogre::Vector3(0, 0, 0), Ogre::Quaternion::IDENTITY);
    mGizmoZ = mGizmoNode->createChildSceneNode("scbnwz", Ogre::Vector3(0, 0, 0), Ogre::Quaternion::IDENTITY);

    Ogre::Quaternion q1;
    Ogre::Quaternion q2;

    q1.FromAngleAxis(Ogre::Degree(90), Ogre::Vector3(0, 0, 1));
    q2.FromAngleAxis(Ogre::Degree(90), Ogre::Vector3(1, 0, 0));
    mGizmoY->setOrientation(q1 * q2);

    q1.FromAngleAxis(Ogre::Degree(-90), Ogre::Vector3(0, 1, 0));
    q2.FromAngleAxis(Ogre::Degree(-90), Ogre::Vector3(1, 0, 0));
    mGizmoZ->setOrientation(q1 * q2);

    // Entities
    mGizmoEntities[0] = sceneManager->createEntity("scbwx", "AxisGizmosMesh", Ogre::RGN_DEFAULT);
    mGizmoEntities[1] = sceneManager->createEntity("scbwy", "AxisGizmosMesh", Ogre::RGN_DEFAULT);
    mGizmoEntities[2] = sceneManager->createEntity("scbwz", "AxisGizmosMesh", Ogre::RGN_DEFAULT);
    mGizmoEntities[3] = sceneManager->createEntity("scbwt", "AxisPlaneMesh", Ogre::RGN_DEFAULT);
    mGizmoEntities[4] = sceneManager->createEntity("scbwu", "AxisPlaneMesh", Ogre::RGN_DEFAULT);
    mGizmoEntities[5] = sceneManager->createEntity("scbwv", "AxisPlaneMesh", Ogre::RGN_DEFAULT);

    // XX arrows
    mGizmoEntities[0]->setCastShadows(false);
    mGizmoEntities[0]->setMaterialName("MAT_GIZMO_X");
    mGizmoEntities[0]->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE);
    mGizmoEntities[0]->setQueryFlags(QUERYFLAG_GIZMO);
    mGizmoX->attachObject(mGizmoEntities[0]);

    // YY arrows
    mGizmoEntities[1]->setCastShadows(false);
    mGizmoEntities[1]->setMaterialName("MAT_GIZMO_Y");
    mGizmoEntities[1]->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE);
    mGizmoEntities[1]->setQueryFlags(QUERYFLAG_GIZMO);
    mGizmoY->attachObject(mGizmoEntities[1]);

    // ZZ arrows
    mGizmoEntities[2]->setCastShadows(false);
    mGizmoEntities[2]->setMaterialName("MAT_GIZMO_Z");
    mGizmoEntities[2]->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE);
    mGizmoEntities[2]->setQueryFlags(QUERYFLAG_GIZMO);
    mGizmoZ->attachObject(mGizmoEntities[2]);

    // XY Plane
    mGizmoEntities[3]->setCastShadows(false);
    mGizmoEntities[3]->setMaterialName("MAT_GIZMO_XY");
    mGizmoEntities[3]->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE);
    mGizmoEntities[3]->setQueryFlags(QUERYFLAG_GIZMO);
    mGizmoX->attachObject(mGizmoEntities[3]);

    // YZ Plane
    mGizmoEntities[4]->setCastShadows(false);
    mGizmoEntities[4]->setMaterialName("MAT_GIZMO_YZ");
    mGizmoEntities[4]->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE);
    mGizmoEntities[4]->setQueryFlags(QUERYFLAG_GIZMO);
    mGizmoY->attachObject(mGizmoEntities[4]);

    // ZX Plane
    mGizmoEntities[5]->setCastShadows(false);
    mGizmoEntities[5]->setMaterialName("MAT_GIZMO_ZX");
    mGizmoEntities[5]->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE);
    mGizmoEntities[5]->setQueryFlags(QUERYFLAG_GIZMO);
    mGizmoZ->attachObject(mGizmoEntities[5]);

    // Call once to derive bounding boxes
    mGizmoEntities[0]->getWorldBoundingBox(true);
    mGizmoEntities[1]->getWorldBoundingBox(true);
    mGizmoEntities[2]->getWorldBoundingBox(true);

    mEntityToAxis[mGizmoEntities[0]] = AXIS_X;
    mEntityToAxis[mGizmoEntities[1]] = AXIS_Y;
    mEntityToAxis[mGizmoEntities[2]] = AXIS_Z;

    mEntityToAxis[mGizmoEntities[3]] = AXIS_XY;
    mEntityToAxis[mGizmoEntities[4]] = AXIS_YZ;
    mEntityToAxis[mGizmoEntities[5]] = AXIS_XZ;

    mGizmoNode->setVisible(true);
    setMode(mode);

    mDragging = false;
}

void Gizmo::attachTo(Ogre::SceneNode* target)
{
    if (mGizmoNode && mGizmoNode->getParentSceneNode())
    {
        mGizmoNode->getParentSceneNode()->removeChild(mGizmoNode);
    }

    mParentNode = target;
    if (!mGizmoNode)
    {
        mGizmoNode = mParentNode->createChildSceneNode();
        mGizmoNode->setInheritScale(false);
        mGizmoNode->setInheritOrientation(mMode == G_SCALE);
    }
    else
    {
        mParentNode->addChild(mGizmoNode);
    }

    mGizmoNode->setPosition(Ogre::Vector3::ZERO);
    mGizmoNode->setOrientation(Ogre::Quaternion::IDENTITY);
    scaleToParent();
}

void Gizmo::setMode(GizmoMode mode)
{
    if (mMode == mode)
        return;
    mMode = mode;
    Ogre::Entity* wx = mGizmoEntities[0];
    Ogre::Entity* wy = mGizmoEntities[1];
    Ogre::Entity* wz = mGizmoEntities[2];

    mGizmoEntities[3]->getSubEntity(0)->setVisible(false);
    mGizmoEntities[4]->getSubEntity(0)->setVisible(false);
    mGizmoEntities[5]->getSubEntity(0)->setVisible(false);

    wx->getSubEntity(0)->setVisible(true);
    wx->getSubEntity(1)->setVisible(false);
    wx->getSubEntity(2)->setVisible(false);
    wx->getSubEntity(3)->setVisible(false);
    wx->getSubEntity(4)->setVisible(false);
    wy->getSubEntity(0)->setVisible(true);
    wy->getSubEntity(1)->setVisible(false);
    wy->getSubEntity(2)->setVisible(false);
    wy->getSubEntity(3)->setVisible(false);
    wy->getSubEntity(4)->setVisible(false);
    wz->getSubEntity(0)->setVisible(true);
    wz->getSubEntity(1)->setVisible(false);
    wz->getSubEntity(2)->setVisible(false);
    wz->getSubEntity(3)->setVisible(false);
    wz->getSubEntity(4)->setVisible(false);
    switch (mode)
    {
    case G_TRANSLATE:
    {
        wx->getSubEntity(2)->setVisible(true);
        wy->getSubEntity(2)->setVisible(true);
        wz->getSubEntity(2)->setVisible(true);
        mGizmoEntities[3]->getSubEntity(0)->setVisible(true);
        mGizmoEntities[4]->getSubEntity(0)->setVisible(true);
        mGizmoEntities[5]->getSubEntity(0)->setVisible(true);
        break;
    }
    case G_ROTATE:
    {
        wx->getSubEntity(0)->setVisible(false);
        wy->getSubEntity(0)->setVisible(false);
        wz->getSubEntity(0)->setVisible(false);
        wx->getSubEntity(1)->setVisible(true);
        wy->getSubEntity(1)->setVisible(true);
        wz->getSubEntity(1)->setVisible(true);
        wx->getSubEntity(3)->setVisible(true);
        wy->getSubEntity(3)->setVisible(true);
        wz->getSubEntity(3)->setVisible(true);
        break;
    }
    case G_SCALE:
    {
        wx->getSubEntity(4)->setVisible(true);
        wy->getSubEntity(4)->setVisible(true);
        wz->getSubEntity(4)->setVisible(true);
        mGizmoEntities[3]->getSubEntity(0)->setVisible(true);
        mGizmoEntities[4]->getSubEntity(0)->setVisible(true);
        mGizmoEntities[5]->getSubEntity(0)->setVisible(true);
        break;
    }
    case G_NONE:
    {
        break;
    }
    case G_AXIS:
    {
        break;
    }
    }
    mGizmoNode->setInheritOrientation(mMode == G_SCALE);
}

bool Gizmo::isGizmoEntity(Ogre::Entity* ent) const
{
    for (auto* g : mGizmoEntities)
    {
        if (g == ent)
            return true;
    }
    return false;
}

void Gizmo::startDrag(const Ogre::Ray& startRay,
                      const Ogre::Vector3& cameraDir)
{
    mDragging = true;

    mInitialObjectPos   = mParentNode->getPosition();
    mInitialObjectRot   = mParentNode->getOrientation();
    mInitialObjectScale = mParentNode->getScale();

    mDragAxisLocal = Ogre::Vector3::ZERO;

    if (mActiveAxis & AXIS_X) mDragAxisLocal += Ogre::Vector3::UNIT_X;
    if (mActiveAxis & AXIS_Y) mDragAxisLocal += Ogre::Vector3::UNIT_Y;
    if (mActiveAxis & AXIS_Z) mDragAxisLocal += Ogre::Vector3::UNIT_Z;

    if (mDragAxisLocal.isZeroLength())
        return;

    mDragAxisLocal.normalise();

    mDragAxisWorld =
        mGizmoNode->_getDerivedOrientation() * mDragAxisLocal;
    mDragAxisWorld.normalise();

    Ogre::Ray localRay = startRay;

    mDragStartHitLocal =
        computePlaneHit(localRay,
                        mDragAxisLocal,
                        cameraDir,
                        Ogre::Vector3::ZERO);
}

void Gizmo::computeDrag(Ogre::Ray& ray,
                        const Ogre::Vector3& cameraDir)
{
    if (!mDragging || !mActiveAxis)
        return;

    Ogre::Ray localRay = ray;

    if (mMode == G_TRANSLATE)
    {
        Ogre::Vector3 hitLocal =
            computePlaneHit(localRay,
                            mDragAxisLocal,
                            cameraDir,
                            Ogre::Vector3::ZERO);

        Ogre::Real delta =
            (hitLocal - mDragStartHitLocal).dotProduct(mDragAxisLocal);

        Ogre::Vector3 gizmoScale =
            mGizmoNode->_getDerivedScale();

        Ogre::Real axisScale =
            std::abs(mDragAxisLocal.dotProduct(gizmoScale));

        mParentNode->setPosition(
            mInitialObjectPos + mDragAxisWorld * (delta * axisScale)
        );

        return;
    }

    if (mMode == G_ROTATE)
    {
        // Plane is LOCAL
        Ogre::Plane plane(mDragAxisLocal, Ogre::Vector3::ZERO);
        auto r = localRay.intersects(plane);
        if (!r.first)
            return;

        Ogre::Vector3 hitLocal = localRay.getPoint(r.second);

        Ogre::Vector3 v0 =
            (mDragStartHitLocal).normalisedCopy();
        Ogre::Vector3 v1 =
            (hitLocal).normalisedCopy();

        Ogre::Real dot =
            Ogre::Math::Clamp(v0.dotProduct(v1), -1.0f, 1.0f);

        Ogre::Radian angle = Ogre::Math::ACos(dot);

        if (v0.crossProduct(v1).dotProduct(mDragAxisLocal) < 0)
            angle = -angle;

        // Rotate in WORLD space
        mParentNode->setOrientation(
            mInitialObjectRot *
            Ogre::Quaternion(angle, mDragAxisWorld)
        );
        return;
    }

    if (mMode == G_SCALE)
    {
        Ogre::Vector3 hitLocal =
            computePlaneHit(localRay,
                            mDragAxisLocal,
                            cameraDir,
                            Ogre::Vector3::ZERO);

        Ogre::Real startDist =
            mDragStartHitLocal.dotProduct(mDragAxisLocal);
        Ogre::Real currDist =
            hitLocal.dotProduct(mDragAxisLocal);

        Ogre::Real delta = currDist - startDist;
        Ogre::Real scaleFactor = 1.0f + delta * 0.01f;

        Ogre::Vector3 newScale = mInitialObjectScale;

        if (mActiveAxis & AXIS_X) newScale.x *= scaleFactor;
        if (mActiveAxis & AXIS_Y) newScale.y *= scaleFactor;
        if (mActiveAxis & AXIS_Z) newScale.z *= scaleFactor;

        mParentNode->setScale(newScale);
    }
}

Ogre::Vector3 Gizmo::computePlaneHit(const Ogre::Ray& ray, const Ogre::Vector3& mDragAxis,
                                     const Ogre::Vector3& cameraDir, const Ogre::Vector3& planePoint)
{
    Ogre::Vector3 v = mDragAxis.crossProduct(cameraDir);

    // handle degeneracy (camera looking along mDragAxis)
    if (v.squaredLength() < 1e-6f)
        v = mDragAxis.perpendicular();

    Ogre::Vector3 planeNormal = v.crossProduct(mDragAxis).normalisedCopy();
    Ogre::Plane plane(planeNormal, planePoint);

    auto r = ray.intersects(plane);
    if (!r.first)
        return planePoint; // fallback

    return ray.getPoint(r.second);
}

void Gizmo::stopDrag()
{
    mDragging = false;
    mDragAxisLocal = Ogre::Vector3::ZERO;
    mDragAxisWorld = Ogre::Vector3::ZERO;
}


bool Gizmo::isDragging() const { return mDragging; }

void Gizmo::createMesh(Ogre::SceneManager* manager, Ogre::String name)
{
    Ogre::ManualObject* mMesh = manager->createManualObject("AxisGizmoManualObject");

    mMesh->begin("AxisGizmo_Material", Ogre::RenderOperation::OT_LINE_LIST);
    mMesh->position(0, 0, 0);
    mMesh->position(3, 0, 0);

    mMesh->index(0);
    mMesh->index(1);
    mMesh->end();

    float const radius = 0.22f;
    float const accuracy = 8;
    float MPI = Ogre::Math::PI;

    float division = (MPI / 2.0f) / 16.0f;
    float start = division * 3;
    float end = division * 14;

    int index_pos = 0;

    mMesh->begin("AxisGizmo_Material", Ogre::RenderOperation::OT_LINE_STRIP);

    for (float theta = start; theta < end; theta += division)
    {
        mMesh->position(0, 3.0f * cos(theta), 3.0f * sin(theta));
        mMesh->index(index_pos++);
    }

    mMesh->end();

    mMesh->begin("AxisGizmo_Material", Ogre::RenderOperation::OT_TRIANGLE_LIST);

    mMesh->position(2.85f, 0, 0);

    for (float theta = 0; theta < 2 * MPI; theta += MPI / accuracy)
    {
        mMesh->position(2.95f, radius * cos(theta), radius * sin(theta));
    }
    mMesh->position(3.45f, 0, 0);

    for (int inside = 1; inside < 16; inside++)
    {
        mMesh->index(0);
        mMesh->index(inside);
        mMesh->index(inside + 1);
    }
    mMesh->index(0);
    mMesh->index(16);
    mMesh->index(1);

    for (int outside = 1; outside < 16; outside++)
    {
        mMesh->index(17);
        mMesh->index(outside);
        mMesh->index(outside + 1);
    }
    mMesh->index(17);
    mMesh->index(16);
    mMesh->index(1);

    mMesh->end();

    // ROTATE GIZMO

    mMesh->begin("AxisGizmo_Material", Ogre::RenderOperation::OT_TRIANGLE_LIST);

    Ogre::Quaternion q1;
    q1.FromAngleAxis(Ogre::Degree(-90), Ogre::Vector3(0, 0, 1));
    Ogre::Quaternion q2;
    q2.FromAngleAxis(Ogre::Degree(90), Ogre::Vector3(0, 1, 0));

    Ogre::Vector3 translate1(0, 3.0f * cos(end), 3.0f * sin(end));
    Ogre::Vector3 translate2(0, 3.0f * cos(start), 3.0f * sin(start) - 0.25f);

    Ogre::Vector3 pos(-0.3f, 0, 0);
    mMesh->position(q1 * pos + translate1);

    for (float theta = 0; theta < 2 * MPI; theta += MPI / accuracy)
    {
        pos = Ogre::Vector3(-0.3f, radius * cos(theta), radius * sin(theta));
        mMesh->position(q1 * pos + translate1);
    }
    pos = Ogre::Vector3(0.3f, 0, 0);
    mMesh->position(q1 * pos + translate1);

    pos = Ogre::Vector3(-0.3f, 0, 0);
    mMesh->position(q2 * pos + translate2);

    for (float theta = 0; theta < 2 * MPI; theta += MPI / accuracy)
    {
        pos = Ogre::Vector3(-0.3f, radius * cos(theta), radius * sin(theta));
        mMesh->position(q2 * pos + translate2);
    }
    pos = Ogre::Vector3(0.3f, 0, 0);
    mMesh->position(q2 * pos + translate2);

    for (int inside = 1; inside < 16; inside++)
    {
        mMesh->index(0);
        mMesh->index(inside);
        mMesh->index(inside + 1);
    }
    mMesh->index(0);
    mMesh->index(16);
    mMesh->index(1);

    for (int outside = 1; outside < 16; outside++)
    {
        mMesh->index(17);
        mMesh->index(outside);
        mMesh->index(outside + 1);
    }
    mMesh->index(17);
    mMesh->index(16);
    mMesh->index(1);

    for (int inside = 19; inside < 34; inside++)
    {
        mMesh->index(18);
        mMesh->index(inside);
        mMesh->index(inside + 1);
    }
    mMesh->index(18);
    mMesh->index(34);
    mMesh->index(19);

    for (int outside = 19; outside < 34; outside++)
    {
        mMesh->index(35);
        mMesh->index(outside);
        mMesh->index(outside + 1);
    }
    mMesh->index(35);
    mMesh->index(34);
    mMesh->index(19);

    mMesh->end();

    // SCALE GIZMO

    mMesh->begin("AxisGizmo_Material", Ogre::RenderOperation::OT_TRIANGLE_LIST);

    mMesh->position(2.85f, 0, 0);

    for (float theta = 0; theta < 2 * MPI; theta += MPI / accuracy)
    {
        mMesh->position(2.85f, radius * cos(theta), radius * sin(theta));
    }
    mMesh->position(3.45f, 0, 0);

    mMesh->position(3.40f, 0.20f, 0.20f);
    mMesh->position(3.40f, 0.20f, -0.20f);
    mMesh->position(3.40f, -0.20f, -0.20f);
    mMesh->position(3.40f, -0.20f, 0.20f);
    mMesh->position(3.50f, 0.20f, 0.20f);
    mMesh->position(3.50f, 0.20f, -0.20f);
    mMesh->position(3.50f, -0.20f, -0.20f);
    mMesh->position(3.50f, -0.20f, 0.20f);

    for (int inside = 1; inside < 16; inside++)
    {
        mMesh->index(0);
        mMesh->index(inside);
        mMesh->index(inside + 1);
    }
    mMesh->index(0);
    mMesh->index(16);
    mMesh->index(1);

    for (int outside = 1; outside < 16; outside++)
    {
        mMesh->index(17);
        mMesh->index(outside);
        mMesh->index(outside + 1);
    }
    mMesh->index(17);
    mMesh->index(16);
    mMesh->index(1);

    mMesh->index(18);
    mMesh->index(19);
    mMesh->index(20);
    mMesh->index(18);
    mMesh->index(20);
    mMesh->index(21);

    mMesh->index(22);
    mMesh->index(23);
    mMesh->index(24);
    mMesh->index(22);
    mMesh->index(24);
    mMesh->index(25);

    mMesh->index(18);
    mMesh->index(22);
    mMesh->index(25);
    mMesh->index(18);
    mMesh->index(25);
    mMesh->index(21);

    mMesh->index(19);
    mMesh->index(23);
    mMesh->index(24);
    mMesh->index(19);
    mMesh->index(24);
    mMesh->index(20);

    mMesh->index(18);
    mMesh->index(22);
    mMesh->index(23);
    mMesh->index(18);
    mMesh->index(23);
    mMesh->index(19);

    mMesh->index(21);
    mMesh->index(20);
    mMesh->index(24);
    mMesh->index(21);
    mMesh->index(24);
    mMesh->index(25);

    mMesh->end();

    mMesh->convertToMesh(name, Ogre::RGN_DEFAULT);

    manager->destroyManualObject(mMesh);
}

void Gizmo::createPlaneMesh(Ogre::SceneManager* manager, Ogre::String name)
{
    Ogre::ManualObject* mMesh = manager->createManualObject("OgitorAxisPlaneGizmoManualObject");

    mMesh->begin("AxisGizmo_Material", Ogre::RenderOperation::OT_TRIANGLE_LIST);

    mMesh->position(0, 1, 0);
    mMesh->position(1, 1, 0);
    mMesh->position(1, 0, 0);
    mMesh->position(0, 0, 0);

    mMesh->index(0);
    mMesh->index(1);
    mMesh->index(2);
    mMesh->index(0);
    mMesh->index(2);
    mMesh->index(3);

    mMesh->end();

    mMesh->convertToMesh(name, Ogre::RGN_DEFAULT);

    manager->destroyManualObject(mMesh);
}

void Gizmo::scaleToParent()
{
    if (!mParentNode)
        return;

    Ogre::AxisAlignedBox worldAABB;
    bool found = false;

    // Accumulate bounds of all attached movable objects
    for (unsigned i = 0; i < mParentNode->numAttachedObjects(); ++i)
    {
        Ogre::MovableObject* mo = mParentNode->getAttachedObject(i);

        if (!mo)
            continue;

        Ogre::AxisAlignedBox box = mo->getWorldBoundingBox(true);
        if (box.isNull())
            continue;

        if (!found)
        {
            worldAABB = box;
            found = true;
        }
        else
        {
            worldAABB.merge(box);
        }
    }

    if (!found)
        return;

    // Compute size
    Ogre::Vector3 size = worldAABB.getSize();

    // Choose a representative scale
    Ogre::Real maxExtent = std::max({ size.x, size.y, size.z });

    // Avoid degenerate scale
    if (maxExtent < Ogre::Real(1e-6))
        maxExtent = 1.0f;

    // Optional tuning factor (editor feel)
    constexpr Ogre::Real gizmoScaleFactor = 0.1f;

    Ogre::Real scale = maxExtent * gizmoScaleFactor;

    mGizmoNode->setScale(Ogre::Vector3(scale));
}

bool Gizmo::pickAxis(Ogre::Ray& ray)
{
    Ogre::Vector3 origin = mGizmoNode->_getDerivedPosition();
    Ogre::Quaternion q = mGizmoNode->_getDerivedOrientation();

    Ogre::Vector3 xAxis = q * Ogre::Vector3::UNIT_X;
    Ogre::Vector3 yAxis = q * Ogre::Vector3::UNIT_Y;
    Ogre::Vector3 zAxis = q * Ogre::Vector3::UNIT_Z;

    constexpr Ogre::Real axisLen = 3.0f;
    constexpr Ogre::Real axisRadius = 0.15f;

    switch (mMode)
    {
    case G_TRANSLATE:
    case G_SCALE:
    {


        Ogre::Real dx = rayLineDistance(ray, origin, xAxis, axisLen);
        Ogre::Real dy = rayLineDistance(ray, origin, yAxis, axisLen);
        Ogre::Real dz = rayLineDistance(ray, origin, zAxis, axisLen);

        Ogre::Real minD = std::min({ dx, dy, dz });

        if (minD > axisRadius)
        {mActiveAxis = AXIS_NONE; break;}

        if (minD == dx) {mActiveAxis = AXIS_X; break;}
        if (minD == dy) {mActiveAxis = AXIS_Y; break;}
        mActiveAxis = AXIS_Z;
        break;
    }
    case G_ROTATE:
    {
        constexpr Ogre::Real ringRadius = 3.0f;
        constexpr Ogre::Real tolerance  = 0.2f;

        if (pickRotateRing(ray, origin, xAxis, ringRadius, tolerance))
        {mActiveAxis = AXIS_X; break;}
        if (pickRotateRing(ray, origin, yAxis, ringRadius, tolerance))
        {mActiveAxis = AXIS_Y; break;}
        if (pickRotateRing(ray, origin, zAxis, ringRadius, tolerance))
        {mActiveAxis = AXIS_Z; break;}

        mActiveAxis = AXIS_NONE;
        break;
    }

    default:
        mActiveAxis = AXIS_NONE;
    }
    highlightAxis(mActiveAxis);
    return (mActiveAxis != AXIS_NONE);
}

void Gizmo::highlightAxis(AXIS axis)
{
    if (axis == mOldGizmoAxis)
        return;
    mOldGizmoAxis = axis;
    mGizmoEntities[0]->setMaterialName("MAT_GIZMO_X");
    mGizmoEntities[1]->setMaterialName("MAT_GIZMO_Y");
    mGizmoEntities[2]->setMaterialName("MAT_GIZMO_Z");
    mGizmoEntities[3]->setMaterialName("MAT_GIZMO_XY");
    mGizmoEntities[4]->setMaterialName("MAT_GIZMO_YZ");
    mGizmoEntities[5]->setMaterialName("MAT_GIZMO_ZX");
    if (axis == AXIS_NONE)
    {
        mOldGizmoAxis = AXIS_NONE;
        return;
    }
    mGizmoEntities[0]->setMaterialName((axis & AXIS_X) ? "MAT_GIZMO_X_L" : "MAT_GIZMO_X");
    mGizmoEntities[1]->setMaterialName((axis & AXIS_Y) ? "MAT_GIZMO_Y_L" : "MAT_GIZMO_Y");
    mGizmoEntities[2]->setMaterialName((axis & AXIS_Z) ? "MAT_GIZMO_Z_L" : "MAT_GIZMO_Z");
    mGizmoEntities[3]->setMaterialName((axis == AXIS_XY) ? "MAT_GIZMO_XY_L" : "MAT_GIZMO_XY");
    mGizmoEntities[4]->setMaterialName((axis == AXIS_YZ) ? "MAT_GIZMO_YZ_L" : "MAT_GIZMO_YZ");
    mGizmoEntities[5]->setMaterialName((axis == AXIS_XZ) ? "MAT_GIZMO_ZX_L" : "MAT_GIZMO_ZX");
}

Ogre::Real Gizmo::rayLineDistance(
    const Ogre::Ray& ray,
    const Ogre::Vector3& lineOrigin,
    const Ogre::Vector3& lineDir,
    Ogre::Real lineLength)
{
    const Ogre::Vector3 u = ray.getDirection();      // normalized
    const Ogre::Vector3 v = lineDir.normalisedCopy() * lineLength;
    const Ogre::Vector3 w0 = ray.getOrigin() - lineOrigin;

    Ogre::Real a = u.dotProduct(u);
    Ogre::Real b = u.dotProduct(v);
    Ogre::Real c = v.dotProduct(v);
    Ogre::Real d = u.dotProduct(w0);
    Ogre::Real e = v.dotProduct(w0);

    Ogre::Real denom = a * c - b * b;
    if (Ogre::Math::Abs(denom) < 1e-6f)
        return Ogre::Math::POS_INFINITY;

    Ogre::Real s = (b * e - c * d) / denom;
    Ogre::Real t = (a * e - b * d) / denom;

    t = Ogre::Math::Clamp(t, Ogre::Real(0), Ogre::Real(1));

    Ogre::Vector3 pr = ray.getOrigin() + u * s;
    Ogre::Vector3 pl = lineOrigin + v * t;

    return (pr - pl).length();
}

Ogre::Ray Gizmo::toLocalRay(const Ogre::Ray& worldRay) const
{
    Ogre::Matrix4 inv = mGizmoNode->_getFullTransform().inverse();

    Ogre::Vector3 localOrigin = inv * worldRay.getOrigin();
    Ogre::Vector3 localDir =
        inv.linear().inverse().transpose() * worldRay.getDirection();

    localDir.normalise();

    return Ogre::Ray(localOrigin, localDir);
}

bool Gizmo::pickRotateRing(
    const Ogre::Ray& ray,
    const Ogre::Vector3& center,
    const Ogre::Vector3& axis,
    Ogre::Real radius,
    Ogre::Real tolerance)
{
    Ogre::Plane plane(axis, center);
    auto hit = ray.intersects(plane);
    if (!hit.first)
        return false;

    Ogre::Vector3 p = ray.getPoint(hit.second);
    Ogre::Real d = (p - center).length();

    return Ogre::Math::Abs(d - radius) < tolerance;
}

} // namespace OgreBites