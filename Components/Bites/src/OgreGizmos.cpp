#include "OgreGizmos.h"
#include "OgreManualObject.h"
#include "OgreMesh.h"
#include "OgreRenderWindow.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "OgreSubEntity.h"
#include "OgreTechnique.h"
#include "OgreViewport.h"
#include <OgreEntity.h>

namespace OgreBites

{
using namespace Ogre;
Gizmo::Gizmo(Ogre::SceneManager* sceneManager, Ogre::SceneNode* sceneNode, GizmoMode mode) : mMode(G_NONE)
{
    // If one mesh shared by multiple entities then we use multiple materials
    // Alternatively, have to have different meshes (submeshes?) for different entities so that the vertex colours aren't shared

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
    mGizmoEntities[0] = sceneManager->createEntity("scbwx", "AxisGizmosMesh", Ogre::RGN_INTERNAL);
    mGizmoEntities[1] = sceneManager->createEntity("scbwy", "AxisGizmosMesh", Ogre::RGN_INTERNAL);
    mGizmoEntities[2] = sceneManager->createEntity("scbwz", "AxisGizmosMesh", Ogre::RGN_INTERNAL);
    mGizmoEntities[3] = sceneManager->createEntity("scbwt", "AxisPlaneMesh", Ogre::RGN_INTERNAL);
    mGizmoEntities[4] = sceneManager->createEntity("scbwu", "AxisPlaneMesh", Ogre::RGN_INTERNAL);
    mGizmoEntities[5] = sceneManager->createEntity("scbwv", "AxisPlaneMesh", Ogre::RGN_INTERNAL);

    int i = 0;
    for (auto matName : {"MAT_GIZMO_X", "MAT_GIZMO_Y", "MAT_GIZMO_Z", "MAT_GIZMO_XY", "MAT_GIZMO_YZ", "MAT_GIZMO_ZX"})
    {
        mGizmoEntities[i]->setCastShadows(false);
        mGizmoEntities[i]->setMaterialName(matName);
        mGizmoEntities[i]->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE);
        mGizmoEntities[i]->setQueryFlags(QUERYFLAG_GIZMO);
        i++;
    }

    mGizmoX->attachObject(mGizmoEntities[0]);
    mGizmoY->attachObject(mGizmoEntities[1]);
    mGizmoZ->attachObject(mGizmoEntities[2]);
    mGizmoX->attachObject(mGizmoEntities[3]);
    mGizmoY->attachObject(mGizmoEntities[4]);
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
        mGizmoNode->setInheritOrientation(mMode == G_SCALE || mMode == G_ROTATE);
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
    }
    mGizmoNode->setInheritOrientation(mMode == G_SCALE || mMode == G_ROTATE);
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

void Gizmo::startDrag(const Ogre::Ray& startRay, const Ogre::Vector3& cameraDir)
{
    mDragging = true;

    mInitialObjectPos = mParentNode->getPosition();
    mInitialObjectRot = mParentNode->getOrientation();
    mInitialObjectScale = mParentNode->getScale();

    mDragAxisLocal = Ogre::Vector3::ZERO;

    if (mActiveAxis & AXIS_X)
        mDragAxisLocal += Ogre::Vector3::UNIT_X;
    if (mActiveAxis & AXIS_Y)
        mDragAxisLocal += Ogre::Vector3::UNIT_Y;
    if (mActiveAxis & AXIS_Z)
        mDragAxisLocal += Ogre::Vector3::UNIT_Z;

    if (mDragAxisLocal.isZeroLength())
        return;

    mDragAxisLocal.normalise();

    mDragAxisWorld = mGizmoNode->_getDerivedOrientation() * mDragAxisLocal;
    mDragAxisWorld.normalise();

    Ogre::Ray localRay = startRay;

    mDragStartHitLocal = computePlaneHit(localRay, mDragAxisLocal, cameraDir, Ogre::Vector3::ZERO);
}

void Gizmo::computeDrag(Ogre::Ray& ray, const Ogre::Vector3& cameraDir)
{
    if (!mDragging || !mActiveAxis)
        return;

    Ogre::Ray localRay = ray;

    if (mMode == G_TRANSLATE)
    {
        Ogre::Vector3 hitLocal = computePlaneHit(localRay, mDragAxisLocal, cameraDir, Ogre::Vector3::ZERO);

        Ogre::Real delta = (hitLocal - mDragStartHitLocal).dotProduct(mDragAxisLocal);

        mParentNode->setPosition(mInitialObjectPos + mDragAxisWorld * (delta * 1));

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

        Ogre::Vector3 v0 = (mDragStartHitLocal).normalisedCopy();
        Ogre::Vector3 v1 = (hitLocal).normalisedCopy();

        Ogre::Real cosTheta = Ogre::Math::Clamp(v0.dotProduct(v1), -1.0f, 1.0f);

        Ogre::Real sinTheta = v0.crossProduct(v1).dotProduct(mDragAxisLocal);

        Ogre::Radian angle = Ogre::Math::ATan2(sinTheta, cosTheta);

        // Rotate in WORLD space
        mParentNode->setOrientation(mInitialObjectRot * Ogre::Quaternion(angle, mDragAxisWorld));
        return;
    }

    if (mMode == G_SCALE)
    {
        Ogre::Vector3 hitLocal = computePlaneHit(localRay, mDragAxisLocal, cameraDir, Ogre::Vector3::ZERO);

        Ogre::Real startDist = mDragStartHitLocal.dotProduct(mDragAxisLocal);
        Ogre::Real currDist = hitLocal.dotProduct(mDragAxisLocal);

        Ogre::Real delta = currDist - startDist;
        Ogre::Real scaleFactor = 1.0f + delta;

        Ogre::Vector3 newScale = mInitialObjectScale;

        if (mActiveAxis & AXIS_X)
            newScale.x *= std::max(scaleFactor, 0.1f);
        if (mActiveAxis & AXIS_Y)
            newScale.y *= std::max(scaleFactor, 0.1f);
        if (mActiveAxis & AXIS_Z)
            newScale.z *= std::max(scaleFactor, 0.1f);

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
    using namespace Ogre;

    ManualObject* mMesh = manager->createManualObject("AxisGizmoManualObject");

    constexpr Real radius   = 0.22f;
    constexpr Real accuracy = 8.0f;
    constexpr Real PI       = Math::PI;

    auto addLine = [&](const Vector3& a, const Vector3& b)
    {
        mMesh->position(a); mMesh->position(b);
        mMesh->index(0);    mMesh->index(1);
    };

    auto addFan = [&](int center, int start, int count)
    {
        for (int i = 0; i < count - 1; ++i)
        {
            mMesh->index(center);
            mMesh->index(start + i);
            mMesh->index(start + i + 1);
        }
        mMesh->index(center);
        mMesh->index(start + count - 1);
        mMesh->index(start);
    };

    auto addRotatedCircle = [&](const Quaternion& q, const Vector3& t, Real x)
    {
        Vector3 p(x, 0, 0);
        mMesh->position(q * p + t);

        for (Real a = 0; a < 2 * PI; a += PI / accuracy)
        {
            p = Vector3(x, radius * cos(a), radius * sin(a));
            mMesh->position(q * p + t);
        }

        p = Vector3(-x, 0, 0);
        mMesh->position(q * p + t);
    };

    mMesh->begin("MAT_GIZMO_VERTEX", RenderOperation::OT_LINE_LIST);
    addLine(Vector3::ZERO, Vector3(3, 0, 0));
    mMesh->end();

    const float division = (PI / 2.0f) / 16.0f;
    const Real start    = division * 3;
    const Real end      = division * 14;

    mMesh->begin("MAT_GIZMO_VERTEX", RenderOperation::OT_LINE_STRIP);
    int arcBase = 0;
    for (Real t = start; t < end; t += division)
    {
        mMesh->position(0, 3 * cos(t), 3 * sin(t));
        mMesh->index(arcBase++);
    }
    mMesh->end();

    // ----------------------------------------------------
    // TRANSLATE GIZMO
    // ----------------------------------------------------
    mMesh->begin("MAT_GIZMO_VERTEX", RenderOperation::OT_TRIANGLE_LIST);

    int base = 0;
    mMesh->position(2.85f, 0, 0);      // center
    addCircle(2.95f);                  // ring
    mMesh->position(3.45f, 0, 0);      // tip

    addFan(base, base + 1, 16);
    addFan(base + 17, base + 1, 16);

    mMesh->end();

    // ----------------------------------------------------
    // ROTATE GIZMO
    // ----------------------------------------------------
    mMesh->begin("MAT_GIZMO_VERTEX", RenderOperation::OT_TRIANGLE_LIST);

    Quaternion q1(Degree(-90), Vector3::UNIT_Z);
    Quaternion q2(Degree(90),  Vector3::UNIT_Y);

    Vector3 t1(0, 3 * cos(end),   3 * sin(end));
    Vector3 t2(0, 3 * cos(start), 3 * sin(start) - 0.25f);

    int rotBase = 0;
    addRotatedCircle(q1, t1, -0.3f);
    addRotatedCircle(q2, t2, -0.3f);

    addFan(rotBase,      rotBase + 1, 16);
    addFan(rotBase + 17, rotBase + 1, 16);
    addFan(rotBase + 18, rotBase + 19, 16);
    addFan(rotBase + 35, rotBase + 19, 16);

    mMesh->end();

    // ----------------------------------------------------
    // SCALE GIZMO
    // ----------------------------------------------------
    mMesh->begin("MAT_GIZMO_VERTEX", RenderOperation::OT_TRIANGLE_LIST);

    int scaleBase = 0;
    mMesh->position(2.85f, 0, 0);
    addCircle(2.85f);
    mMesh->position(3.45f, 0, 0);

    static const Vector3 cube[] = {
        {3.40f,  0.20f,  0.20f}, {3.40f,  0.20f, -0.20f},
        {3.40f, -0.20f, -0.20f}, {3.40f, -0.20f,  0.20f},
        {3.50f,  0.20f,  0.20f}, {3.50f,  0.20f, -0.20f},
        {3.50f, -0.20f, -0.20f}, {3.50f, -0.20f,  0.20f}
    };

    for (auto& v : cube) mMesh->position(v);

    addFan(scaleBase,      scaleBase + 1, 16);
    addFan(scaleBase + 17, scaleBase + 1, 16);

    const int faces[][6] = {
        {18,19,20, 18,20,21}, {22,23,24, 22,24,25},
        {18,22,25, 18,25,21}, {19,23,24, 19,24,20},
        {18,22,23, 18,23,19}, {21,20,24, 21,24,25}
    };

    for (auto& f : faces)
        for (int i : f) mMesh->index(i);

    mMesh->end();

    mMesh->convertToMesh(name, RGN_INTERNAL);
    manager->destroyManualObject(mMesh);
}

void Gizmo::createPlaneMesh(Ogre::SceneManager* manager, Ogre::String name)
{
    Ogre::ManualObject* mMesh = manager->createManualObject("OgitorAxisPlaneGizmoManualObject");

    mMesh->begin("MAT_GIZMO_VERTEX_ALPHA", Ogre::RenderOperation::OT_TRIANGLE_LIST);

    for (auto& v : { Ogre::Vector3(0,1,0), Ogre::Vector3(1,1,0),
                     Ogre::Vector3(1,0,0), Ogre::Vector3(0,0,0) })
        mMesh->position(v);

    for (int i : { 0,1,2, 0,2,3 })
        mMesh->index(i);

    mMesh->end();

    mMesh->convertToMesh(name, Ogre::RGN_INTERNAL);

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

    Ogre::Real maxExtent = std::max({size.x, size.y, size.z});

    // Avoid degenerate scale
    if (maxExtent < Ogre::Real(1e-6))
        maxExtent = 1.0f;

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

    Ogre::Real axisLen = 4.0f * mGizmoNode->getScale().x;
    Ogre::Real axisRadius = 0.3f * mGizmoNode->getScale().x;

    switch (mMode)
    {
    case G_TRANSLATE:
    case G_SCALE:
    {
        // Pick planes first

        // Pick axes
        Ogre::Real dx = rayLineDistance(ray, origin, xAxis, axisLen);
        Ogre::Real dy = rayLineDistance(ray, origin, yAxis, axisLen);
        Ogre::Real dz = rayLineDistance(ray, origin, zAxis, axisLen);

        mActiveAxis = AXIS_NONE;

        if (dx <= axisRadius)
            mActiveAxis |= AXIS_X;

        if (dy <= axisRadius)
            mActiveAxis |= AXIS_Y;

        if (dz <= axisRadius)
            mActiveAxis |= AXIS_Z;

        // mActiveAxis now may be AXIS_X, AXIS_XY, AXIS_XZ, etc.
        break;
    }

    case G_ROTATE:
    {
        constexpr Ogre::Real ringRadius = 3.0f;
        constexpr Ogre::Real tolerance = 0.2f;

        if (pickRotateRing(ray, origin, xAxis, ringRadius, tolerance))
        {
            mActiveAxis = AXIS_X;
            break;
        }
        if (pickRotateRing(ray, origin, yAxis, ringRadius, tolerance))
        {
            mActiveAxis = AXIS_Y;
            break;
        }
        if (pickRotateRing(ray, origin, zAxis, ringRadius, tolerance))
        {
            mActiveAxis = AXIS_Z;
            break;
        }

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

Ogre::Real Gizmo::rayLineDistance(const Ogre::Ray& ray, const Ogre::Vector3& lineOrigin, const Ogre::Vector3& lineDir,
                                  Ogre::Real lineLength)
{
    const Ogre::Vector3 u = ray.getDirection(); // normalized
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
    Ogre::Vector3 localDir = inv.linear().inverse().transpose() * worldRay.getDirection();

    localDir.normalise();

    return Ogre::Ray(localOrigin, localDir);
}

bool Gizmo::pickRotateRing(const Ogre::Ray& ray, const Ogre::Vector3& center,
                           const Ogre::Vector3& axis, // must be normalized
                           Ogre::Real radius, Ogre::Real tolerance)
{
    // 1. Reject near-parallel rays (huge stability win)
    Ogre::Real ndotd = Ogre::Math::Abs(axis.dotProduct(ray.getDirection()));
    if (ndotd > 0.95f)
        return false;

    // 2. Intersect ray with ring plane
    Ogre::Plane plane(axis, center);
    auto hit = ray.intersects(plane);
    if (!hit.first)
        return false;

    Ogre::Vector3 p = ray.getPoint(hit.second);

    // 3. Compute radial distance IN THE PLANE
    Ogre::Vector3 v = p - center;

    // remove axis component (critical fix)
    v -= axis * v.dotProduct(axis);

    Ogre::Real d = v.length();

    // 4. Scale tolerance by depth (simple perspective fix)
    Ogre::Real depth = (center - ray.getOrigin()).length();
    Ogre::Real scaledTolerance = tolerance * depth * 0.05f;

    return Ogre::Math::Abs(d - radius) < scaledTolerance;
}

CameraGizmo::CameraGizmo(Ogre::RenderWindow* window, Ogre::SceneNode* cameraNode,
                         CameraMan* cameraMan)
{
    auto gizmoSm = Ogre::Root::getSingleton().createSceneManager();

    mCameraMan = cameraMan;
    mCameraNode = cameraNode;
    mGizmoNode = gizmoSm->getRootSceneNode()->createChildSceneNode("GizmoRoot");

    mGizmoCamera = gizmoSm->createCamera("GizmoCamera");
    mGizmoCameraNode = gizmoSm->createSceneNode("GizmoCameraNode");

    mGizmoCameraNode->attachObject(mGizmoCamera);
    gizmoSm->getRootSceneNode()->addChild(mGizmoCameraNode);

    mGizmoCamera->setProjectionType(Ogre::PT_ORTHOGRAPHIC);
    mGizmoCamera->setNearClipDistance(0.01f);
    mGizmoCamera->setFarClipDistance(10.0f);
    Ogre::Viewport* vp = window->addViewport(mGizmoCamera,
                                             1, // higher Z-order than main viewport
                                             0.80f, 0.0f, 0.20f, 0.20f);
    float aspect = vp->getActualWidth() / float(vp->getActualHeight());

    float size = 2.0f;
    mGizmoCamera->setOrthoWindow(size * aspect, size);
    vp->setOverlaysEnabled(false);
    vp->setClearEveryFrame(false);

    // Always look down -Z
    mGizmoCameraNode->setPosition(0, 0, 2);
    createMesh(gizmoSm, "AxisGizmosMesh");

    mRayQuery = gizmoSm->createRayQuery(Ogre::Ray());
}

void CameraGizmo::updateOrientation()
{
    mGizmoNode->setOrientation(mCameraMan->getCamera()->getOrientation().Inverse());
}

Ogre::ManualObject* CameraGizmo::pickFace(float nx, float ny)
{
    float vx = (nx - 0.80f) / 0.20f;
    float vy = ny / 0.20f;
    const Ogre::Ray ray = mGizmoCamera->getCameraToViewportRay(vx, vy);
    mRayQuery->setRay(ray);
    mRayQuery->setQueryMask(QUERYFLAG_GIZMO);
    mRayQuery->setSortByDistance(true);

    Ogre::ManualObject* face = nullptr;
    Ogre::Real closest = std::numeric_limits<Ogre::Real>::max();

    auto& results = mRayQuery->execute();
    for (auto& hit : results)
    {
        if (!hit.movable)
            continue;

        if (!(hit.movable->getQueryFlags() & QUERYFLAG_GIZMO))
            continue;

        if (hit.distance < closest)
        {
            closest = hit.distance;
            face = dynamic_cast<Ogre::ManualObject*>(hit.movable);
        }
    }
    highlightFace(face);
    return face;
}

void CameraGizmo::snapCamera(Ogre::ManualObject* face) const
{
    using namespace Ogre;

    if (!face || !mCameraMan)
        return;

    int faceIndex = -1;

    for (int i = 0; i < 6; ++i)
    {
        if (mGizmoObjects[i] == face)
        {
            faceIndex = i;
            break;
        }
    }

    if (faceIndex == -1)
        return; // not one of our faces
    Radian yaw(0), pitch(0);

    switch (faceIndex)
    {
    case 0: // +X
        yaw = Degree(90);
        pitch = Degree(0);
        break;

    case 1: // -X
        yaw = Degree(-90);
        pitch = Degree(0);
        break;

    case 2: // +Y (top)
        yaw = Degree(0);
        pitch = Degree(90);
        break;

    case 3: // -Y (bottom)
        yaw = Degree(0);
        pitch = Degree(-90);
        break;

    case 4: // +Z (front)
        yaw = Degree(0);
        pitch = Degree(0);
        break;

    case 5: // -Z (back)
        yaw = Degree(180);
        pitch = Degree(0);
        break;
    default:;
    }

    Vector3 camPos = mCameraNode->getPosition();
    Vector3 target = mCameraMan->getTarget()->getPosition();

    Real dist = camPos.distance(target);

    mCameraMan->setYawPitchDist(yaw, pitch, dist);
}

void CameraGizmo::highlightFace(Ogre::ManualObject* face)
{
    if (!face)
    {
        mGizmoObjects[0]->setMaterialName(0, "MAT_CAMERA_GIZMO_X");
        mGizmoObjects[1]->setMaterialName(0, "MAT_CAMERA_GIZMO_X");
        mGizmoObjects[2]->setMaterialName(0, "MAT_CAMERA_GIZMO_Y");
        mGizmoObjects[3]->setMaterialName(0, "MAT_CAMERA_GIZMO_Y");
        mGizmoObjects[4]->setMaterialName(0, "MAT_CAMERA_GIZMO_Z");
        mGizmoObjects[5]->setMaterialName(0, "MAT_CAMERA_GIZMO_Z");
        return;
    }

    int faceIndex = -1;

    for (int i = 0; i < 6; ++i)
    {
        if (mGizmoObjects[i] == face)
        {
            faceIndex = i;
            break;
        }
    }

    if (faceIndex == mOldFaceIndex)
        return;
    mOldFaceIndex = faceIndex;
    mGizmoObjects[0]->setMaterialName(0, "MAT_CAMERA_GIZMO_X");
    mGizmoObjects[1]->setMaterialName(0, "MAT_CAMERA_GIZMO_X");
    mGizmoObjects[2]->setMaterialName(0, "MAT_CAMERA_GIZMO_Y");
    mGizmoObjects[3]->setMaterialName(0, "MAT_CAMERA_GIZMO_Y");
    mGizmoObjects[4]->setMaterialName(0, "MAT_CAMERA_GIZMO_Z");
    mGizmoObjects[5]->setMaterialName(0, "MAT_CAMERA_GIZMO_Z");
    if (faceIndex == -1)
    {
        mOldFaceIndex = -1;
        return;
    }
    mGizmoObjects[0]->setMaterialName(0, faceIndex == 0 ? "MAT_CAMERA_GIZMO_X_L" : "MAT_CAMERA_GIZMO_X");
    mGizmoObjects[1]->setMaterialName(0, faceIndex == 1 ? "MAT_CAMERA_GIZMO_X_L" : "MAT_CAMERA_GIZMO_X");
    mGizmoObjects[2]->setMaterialName(0, faceIndex == 2 ? "MAT_CAMERA_GIZMO_Y_L" : "MAT_CAMERA_GIZMO_Y");
    mGizmoObjects[3]->setMaterialName(0, faceIndex == 3 ? "MAT_CAMERA_GIZMO_Y_L" : "MAT_CAMERA_GIZMO_Y");
    mGizmoObjects[4]->setMaterialName(0, faceIndex == 4 ? "MAT_CAMERA_GIZMO_Z_L" : "MAT_CAMERA_GIZMO_Z");
    mGizmoObjects[5]->setMaterialName(0, faceIndex == 5 ? "MAT_CAMERA_GIZMO_Z_L" : "MAT_CAMERA_GIZMO_Z");
}

void CameraGizmo::createMesh(Ogre::SceneManager* manager, Ogre::String name)
{
    using namespace Ogre;

    auto addQuad = [](ManualObject* mo, float x0, float y0, float x1, float y1, float z = 0.0f)
    {
        mo->position(x0, y0, z);
        mo->position(x1, y0, z);
        mo->position(x1, y1, z);
        mo->position(x0, y1, z);

        int base = mo->getCurrentVertexCount() - 4;
        mo->triangle(base + 0, base + 1, base + 2);
        mo->triangle(base + 0, base + 2, base + 3);
    };

    auto createFace = [&](const String& n, const String& materialName, const int index)
    {
        SceneNode* faceNode = mGizmoNode->createChildSceneNode(n);

        // Face quad
        ManualObject* quad = manager->createManualObject(n + "_Face");
        quad->begin("Gizmo/Face", RenderOperation::OT_TRIANGLE_LIST);
        addQuad(quad, -0.5f, -0.5f, 0.5f, 0.5f);
        quad->end();

        quad->setQueryFlags(QUERYFLAG_GIZMO); // pickable
        quad->setCastShadows(false);
        quad->setMaterialName(0, materialName);
        quad->setRenderQueueGroup(RENDER_QUEUE_OVERLAY);
        faceNode->attachObject(quad);
        mGizmoObjects[index] = quad;

        return faceNode;
    };
    SceneNode* px = createFace(name + "_PX", "MAT_CAMERA_GIZMO_X", 0);
    px->setPosition(0.5f, 0, 0);
    px->yaw(Degree(-90));
    SceneNode* nx = createFace(name + "_NX", "MAT_CAMERA_GIZMO_X", 1);
    nx->setPosition(-0.5f, 0, 0);
    nx->yaw(Degree(90));
    SceneNode* py = createFace(name + "_PY", "MAT_CAMERA_GIZMO_Y", 2);
    py->setPosition(0, 0.5f, 0);
    py->pitch(Degree(90));
    SceneNode* ny = createFace(name + "_NY", "MAT_CAMERA_GIZMO_Y", 3);
    ny->setPosition(0, -0.5f, 0);
    ny->pitch(Degree(-90));
    SceneNode* pz = createFace(name + "_PZ", "MAT_CAMERA_GIZMO_Z", 4);
    pz->setPosition(0, 0, 0.5f);
    SceneNode* nz = createFace(name + "_NZ", "MAT_CAMERA_GIZMO_Z", 5);
    nz->setPosition(0, 0, -0.5f);
    nz->yaw(Degree(180));
}

static void addCircle(Ogre::Real x, Ogre::MeshPtr mesh, float radius, float accuracy)
{
    for (Real t = 0; t < 2 * Math::PI; t += Math::PI / accuracy)
        mesh->position(x, radius * cos(t), radius * sin(t));
};

} // namespace OgreBites