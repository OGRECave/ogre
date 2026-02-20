#include "OgreGizmos.h"
#include "OgreManualObject.h"
#include "OgreMaterial.h"
#include "OgreMaterialManager.h"
#include "OgreMesh.h"
#include "OgrePass.h"
#include "OgrePlaneBoundedVolume.h"
#include "OgreSceneManager.h"
#include "OgreSubEntity.h"
#include "OgreTechnique.h"
#include "OgreViewport.h"
#include <OgreEntity.h>
#include <algorithm>

namespace OgreBites

{
using namespace Ogre;
namespace
{

void addPosition(const Vector3f& p, const Quaternion& rot, ManualObject* mesh, const ColourValue& color)
{
    mesh->position(rot * p);
    mesh->colour(color);
}

void addCircle(Real x, const Quaternion& rot, ManualObject* mesh, const ColourValue& color)
{
    constexpr Real radius = 0.22f;
    constexpr Real accuracy = 8.0f;

    using namespace Ogre;
    for (Real t = 0; t < 2 * Math::PI; t += Math::PI / accuracy)
    {
        addPosition(Vector3f(x, radius * cos(t), radius * sin(t)), rot, mesh, color);
    }
}

void addLine(const Vector3f& a, const Vector3f& b, const Quaternion& rot, ManualObject* mesh, const ColourValue& color)
{
    addPosition(a, rot, mesh, color);
    addPosition(b, rot, mesh, color);
    mesh->index(0);
    mesh->index(1);
};

void addFan(int center, int start, int count, ManualObject* mesh)
{
    for (int i = 0; i < count - 1; ++i)
    {
        mesh->triangle(center, start + i, start + i + 1);
    }
    mesh->index(center);
    mesh->index(start + count - 1);
    mesh->index(start);
}

void addRotatedCircle(const Quaternion& q, const Vector3f& t, Real x, const Quaternion& rot, ManualObject* mesh,
                      const ColourValue& color)
{
    constexpr Real radius = 0.22f;
    constexpr Real accuracy = 8.0f;

    using namespace Ogre;
    Vector3f p(x, 0, 0);
    addPosition(q * p + t, rot, mesh, color);

    for (Real a = 0; a < 2 * Math::PI; a += Math::PI / accuracy)
    {
        p = Vector3f(x, radius * cos(a), radius * sin(a));
        addPosition(q * p + t, rot, mesh, color);
    }

    p = Vector3f(-x, 0, 0);
    addPosition(q * p + t, rot, mesh, color);
}

void updateCameraQuad(ManualObject* mo, const ColourValue& color)
{
    mo->beginUpdate(0);
    mo->position(-0.5f, -0.5f, 0.0f); mo->colour(color);
    mo->position(0.5f, -0.5f, 0.0f);  mo->colour(color);
    mo->position(0.5f, 0.5f, 0.0f);   mo->colour(color);
    mo->position(-0.5f, 0.5f, 0.0f);  mo->colour(color);
    mo->quad(0, 1, 2, 3);
    mo->end();
}

} // namespace

Gizmo::Gizmo(SceneManager* sceneManager, SceneNode* sceneNode, GizmoMode mode) : mMode(G_NONE)
{

    createMesh(sceneManager, "AxisGizmosMesh", ColourValue(1.0f, 0.0f, 0.0f, 0.50f),
               ColourValue(0.0f, 1.0f, 0.0f, 0.50f), ColourValue(0.0f, 0.0f, 1.0f, 0.50f),
               ColourValue(1.0f, 1.0f, 0.0f, 0.35f), ColourValue(0.0f, 1.0f, 1.0f, 0.35f),
               ColourValue(1.0f, 0.0f, 1.0f, 0.35f));

    attachTo(sceneNode);

    scaleToParent();

    mGizmoEntity = sceneManager->createEntity("scbw", "AxisGizmosMesh", RGN_INTERNAL);
    mGizmoEntity->setCastShadows(false);
    mGizmoEntity->setMaterialName("AxisGizmo_Material");
    mGizmoEntity->setRenderQueueGroup(RENDER_QUEUE_SKIES_LATE);
    mGizmoEntity->setQueryFlags(QUERYFLAG_GIZMO);

    mGizmoNode->attachObject(mGizmoEntity);

    // Call once to derive bounding boxes
    mGizmoEntity->getWorldBoundingBox(true);

    mGizmoNode->setVisible(true);
    setMode(mode);

    mDragging = false;
}

void Gizmo::attachTo(SceneNode* target)
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

    mGizmoNode->setPosition(Vector3f::ZERO);
    mGizmoNode->setOrientation(Quaternion::IDENTITY);
    scaleToParent();
}

void Gizmo::setMode(GizmoMode mode)
{
    if (mMode == mode)
        return;
    mMode = mode;
    constexpr int axisParts = 5;
    constexpr int planeBase = axisParts * 3;

    for (int axis = 0; axis < 3; ++axis)
    {
        const int base = axis * axisParts;
        mGizmoEntity->getSubEntity(base + 0)->setVisible(true);
        mGizmoEntity->getSubEntity(base + 1)->setVisible(false);
        mGizmoEntity->getSubEntity(base + 2)->setVisible(false);
        mGizmoEntity->getSubEntity(base + 3)->setVisible(false);
        mGizmoEntity->getSubEntity(base + 4)->setVisible(false);
    }

    for (int i = 0; i < 3; ++i)
        mGizmoEntity->getSubEntity(planeBase + i)->setVisible(false);
    switch (mode)
    {
    case G_TRANSLATE:
    {
        mGizmoEntity->getSubEntity(0 + 2)->setVisible(true);
        mGizmoEntity->getSubEntity(5 + 2)->setVisible(true);
        mGizmoEntity->getSubEntity(10 + 2)->setVisible(true);
        mGizmoEntity->getSubEntity(planeBase + 0)->setVisible(true);
        mGizmoEntity->getSubEntity(planeBase + 1)->setVisible(true);
        mGizmoEntity->getSubEntity(planeBase + 2)->setVisible(true);
        break;
    }
    case G_ROTATE:
    {
        mGizmoEntity->getSubEntity(0)->setVisible(false);
        mGizmoEntity->getSubEntity(5)->setVisible(false);
        mGizmoEntity->getSubEntity(10)->setVisible(false);
        mGizmoEntity->getSubEntity(0 + 1)->setVisible(true);
        mGizmoEntity->getSubEntity(5 + 1)->setVisible(true);
        mGizmoEntity->getSubEntity(10 + 1)->setVisible(true);
        mGizmoEntity->getSubEntity(0 + 3)->setVisible(true);
        mGizmoEntity->getSubEntity(5 + 3)->setVisible(true);
        mGizmoEntity->getSubEntity(10 + 3)->setVisible(true);
        break;
    }
    case G_SCALE:
    {
        mGizmoEntity->getSubEntity(0 + 4)->setVisible(true);
        mGizmoEntity->getSubEntity(5 + 4)->setVisible(true);
        mGizmoEntity->getSubEntity(10 + 4)->setVisible(true);
        mGizmoEntity->getSubEntity(planeBase + 0)->setVisible(true);
        mGizmoEntity->getSubEntity(planeBase + 1)->setVisible(true);
        mGizmoEntity->getSubEntity(planeBase + 2)->setVisible(true);
        break;
    }
    case G_NONE:
    {
        break;
    }
    }
    mGizmoNode->setInheritOrientation(mMode == G_SCALE || mMode == G_ROTATE);
}

bool Gizmo::isGizmoEntity(Entity* ent) const { return ent == mGizmoEntity; }

void Gizmo::startDrag(const Ray& startRay, const Vector3f& cameraDir)
{
    mDragging = true;

    mInitialObjectPos = mParentNode->getPosition();
    mInitialObjectRot = mParentNode->getOrientation();
    mInitialObjectScale = mParentNode->getScale();

    mDragAxisLocal = Vector3f::ZERO;

    if (mActiveAxis & AXIS_X)
        mDragAxisLocal += Vector3f::UNIT_X;
    if (mActiveAxis & AXIS_Y)
        mDragAxisLocal += Vector3f::UNIT_Y;
    if (mActiveAxis & AXIS_Z)
        mDragAxisLocal += Vector3f::UNIT_Z;

    if (mDragAxisLocal.isZeroLength())
        return;

    const int axisCount =
        ((mActiveAxis & AXIS_X) ? 1 : 0) + ((mActiveAxis & AXIS_Y) ? 1 : 0) + ((mActiveAxis & AXIS_Z) ? 1 : 0);

    mDragUsePlane = axisCount >= 2;
    mDragPlaneNormalLocal = Vector3f::ZERO;

    if (mDragUsePlane)
    {
        if (mMode == G_SCALE)
        {
            mDragGizmoOrientation = mGizmoNode->_getDerivedOrientation();
            mDragGizmoCenter = mGizmoNode->_getDerivedPosition();
            const Quaternion invOri = mDragGizmoOrientation.Inverse();
            Ray localRay(invOri * (startRay.getOrigin() - mDragGizmoCenter), invOri * startRay.getDirection());

            if (axisCount == 2)
            {
                Vector3f a = (mActiveAxis & AXIS_X) ? Vector3f::UNIT_X : Vector3f::ZERO;
                Vector3f b = (mActiveAxis & AXIS_Y) ? Vector3f::UNIT_Y : Vector3f::ZERO;
                if (mActiveAxis & AXIS_Z)
                    b = Vector3f::UNIT_Z;
                if (a.isZeroLength())
                    a = Vector3f::UNIT_Y;
                mDragPlaneNormalLocal = a.crossProduct(b).normalisedCopy();
            }
            else
            {
                mDragPlaneNormalLocal = (invOri * cameraDir).normalisedCopy();
            }

            Plane plane(mDragPlaneNormalLocal, Vector3f::ZERO);
            auto r = localRay.intersects(plane);
            if (!r.first)
                return;
            mDragStartHitLocal = localRay.getPoint(r.second);
            return;
        }

        if (axisCount == 2)
        {
            Vector3f a = (mActiveAxis & AXIS_X) ? Vector3f::UNIT_X : Vector3f::ZERO;
            Vector3f b = (mActiveAxis & AXIS_Y) ? Vector3f::UNIT_Y : Vector3f::ZERO;
            if (mActiveAxis & AXIS_Z)
                b = Vector3f::UNIT_Z;
            if (a.isZeroLength())
                a = Vector3f::UNIT_Y;
            mDragPlaneNormalLocal = a.crossProduct(b).normalisedCopy();
        }
        else
        {
            mDragPlaneNormalLocal = cameraDir.normalisedCopy();
        }

        Plane plane(mDragPlaneNormalLocal, Vector3f::ZERO);
        auto r = startRay.intersects(plane);
        if (!r.first)
            return;
        mDragStartHitLocal = startRay.getPoint(r.second);
        return;
    }

    mDragAxisLocal.normalise();

    mDragAxisWorld = mGizmoNode->_getDerivedOrientation() * mDragAxisLocal;
    mDragAxisWorld.normalise();

    if (mMode == G_ROTATE)
    {
        mDragGizmoOrientation = mGizmoNode->_getDerivedOrientation();
        mDragGizmoCenter = mGizmoNode->_getDerivedPosition();
        const Quaternion invOri = mDragGizmoOrientation.Inverse();
        Ray localRay(invOri * (startRay.getOrigin() - mDragGizmoCenter), invOri * startRay.getDirection());

        Plane plane(mDragAxisLocal, Vector3f::ZERO);
        auto r = localRay.intersects(plane);
        if (!r.first)
            return;
        mDragStartHitLocal = localRay.getPoint(r.second);
        mDragLastHitLocal = mDragStartHitLocal;
        mDragAccumulatedAngle = Radian(0);
    }
    else
    {
        if (mMode == G_SCALE)
        {
            mDragGizmoOrientation = mGizmoNode->_getDerivedOrientation();
            mDragGizmoCenter = mGizmoNode->_getDerivedPosition();
            const Quaternion invOri = mDragGizmoOrientation.Inverse();
            Ray localRay(invOri * (startRay.getOrigin() - mDragGizmoCenter), invOri * startRay.getDirection());
            Vector3f localCameraDir = invOri * cameraDir;
            mDragStartHitLocal = computePlaneHit(localRay, mDragAxisLocal, localCameraDir, Vector3f::ZERO);
            return;
        }
        Ray localRay = startRay;
        mDragStartHitLocal = computePlaneHit(localRay, mDragAxisLocal, cameraDir, Vector3f::ZERO);
    }
}

void Gizmo::computeDrag(Ray& ray, const Vector3f& cameraDir)
{
    if (!mDragging || !mActiveAxis)
        return;

    if (mMode == G_TRANSLATE)
    {
        Ray localRay = ray;
        if (mDragUsePlane)
        {
            Plane plane(mDragPlaneNormalLocal, Vector3f::ZERO);
            auto r = localRay.intersects(plane);
            if (!r.first)
                return;
            Vector3f hitLocal = localRay.getPoint(r.second);
            Vector3f deltaLocal = hitLocal - mDragStartHitLocal;

            if (!(mActiveAxis & AXIS_X))
                deltaLocal.x = 0;
            if (!(mActiveAxis & AXIS_Y))
                deltaLocal.y = 0;
            if (!(mActiveAxis & AXIS_Z))
                deltaLocal.z = 0;

            Vector3f deltaWorld = mGizmoNode->_getDerivedOrientation() * deltaLocal;
            mParentNode->setPosition(mInitialObjectPos + deltaWorld);
        }
        else
        {
            Vector3f hitLocal = computePlaneHit(localRay, mDragAxisLocal, cameraDir, Vector3f::ZERO);
            Real delta = (hitLocal - mDragStartHitLocal).dotProduct(mDragAxisLocal);
            mParentNode->setPosition(mInitialObjectPos + mDragAxisWorld * delta);
        }

        return;
    }

    if (mMode == G_ROTATE)
    {
        const Quaternion invOri = mDragGizmoOrientation.Inverse();
        Ray localRay(invOri * (ray.getOrigin() - mDragGizmoCenter), invOri * ray.getDirection());

        Plane plane(mDragAxisLocal, Vector3f::ZERO);
        auto r = localRay.intersects(plane);
        if (!r.first)
            return;

        Vector3f hitLocal = localRay.getPoint(r.second);
        const Real radius = mDragStartHitLocal.length();
        if (radius > 1e-4f && hitLocal.squaredLength() > 1e-8f)
            hitLocal = hitLocal.normalisedCopy() * radius;

        Vector3f v0 = mDragLastHitLocal;
        Vector3f v1 = hitLocal;
        v0 -= mDragAxisLocal * v0.dotProduct(mDragAxisLocal);
        v1 -= mDragAxisLocal * v1.dotProduct(mDragAxisLocal);
        if (v0.isZeroLength() || v1.isZeroLength())
            return;
        v0.normalise();
        v1.normalise();

        Real cosTheta = Math::Clamp(v0.dotProduct(v1), -1.0f, 1.0f);
        Real sinTheta = v0.crossProduct(v1).dotProduct(mDragAxisLocal);

        mDragAccumulatedAngle += Math::ATan2(sinTheta, cosTheta);
        mDragLastHitLocal = hitLocal;

        // Rotate in LOCAL space
        mParentNode->setOrientation(mInitialObjectRot * Quaternion(mDragAccumulatedAngle, mDragAxisLocal));
        return;
    }

    if (mMode == G_SCALE)
    {
        const Quaternion invOri = mDragGizmoOrientation.Inverse();
        Ray localRay(invOri * (ray.getOrigin() - mDragGizmoCenter), invOri * ray.getDirection());
        Vector3f localCameraDir = invOri * cameraDir;
        if (mDragUsePlane)
        {
            Plane plane(mDragPlaneNormalLocal, Vector3f::ZERO);
            auto r = localRay.intersects(plane);
            if (!r.first)
                return;
            Vector3f hitLocal = localRay.getPoint(r.second);
            Vector3f deltaLocal = hitLocal - mDragStartHitLocal;

            Vector3f newScale = mInitialObjectScale;
            if (mActiveAxis & AXIS_X)
                newScale.x *= std::max(1.0f + (deltaLocal.x * 0.5f), 0.1f);
            if (mActiveAxis & AXIS_Y)
                newScale.y *= std::max(1.0f + (deltaLocal.y * 0.5f), 0.1f);
            if (mActiveAxis & AXIS_Z)
                newScale.z *= std::max(1.0f + (deltaLocal.z * 0.5f), 0.1f);

            mParentNode->setScale(newScale);
        }
        else
        {
            Vector3f hitLocal = computePlaneHit(localRay, mDragAxisLocal, localCameraDir, Vector3f::ZERO);
            Real startDist = mDragStartHitLocal.dotProduct(mDragAxisLocal);
            Real currDist = hitLocal.dotProduct(mDragAxisLocal);
            Real delta = currDist - startDist;
            Real scaleFactor = 1.0f + (delta * 0.5f);

            Vector3f newScale = mInitialObjectScale;
            if (mActiveAxis & AXIS_X)
                newScale.x *= std::max(scaleFactor, 0.1f);
            if (mActiveAxis & AXIS_Y)
                newScale.y *= std::max(scaleFactor, 0.1f);
            if (mActiveAxis & AXIS_Z)
                newScale.z *= std::max(scaleFactor, 0.1f);

            mParentNode->setScale(newScale);
        }
    }
}

Vector3f Gizmo::computePlaneHit(const Ray& ray, const Vector3f& mDragAxis, const Vector3f& cameraDir,
                               const Vector3f& planePoint)
{
    Vector3f v = mDragAxis.crossProduct(cameraDir);

    // handle degeneracy (camera looking along mDragAxis)
    if (v.squaredLength() < 1e-6f)
        v = mDragAxis.perpendicular();

    Vector3f planeNormal = v.crossProduct(mDragAxis).normalisedCopy();
    Plane plane(planeNormal, planePoint);

    auto r = ray.intersects(plane);
    if (!r.first)
        return planePoint; // fallback

    return ray.getPoint(r.second);
}

void Gizmo::stopDrag()
{
    mDragging = false;
    mDragAxisLocal = Vector3f::ZERO;
    mDragAxisWorld = Vector3f::ZERO;
    mDragPlaneNormalLocal = Vector3f::ZERO;
    mDragUsePlane = false;
    mDragLastHitLocal = Vector3f::ZERO;
    mDragAccumulatedAngle = Radian(0);
    mDragGizmoOrientation = Quaternion::IDENTITY;
    mDragGizmoCenter = Vector3f::ZERO;
}

bool Gizmo::isDragging() const { return mDragging; }

void Gizmo::createMesh(SceneManager* manager, const String& name, const ColourValue& xColor, const ColourValue& yColor,
                       const ColourValue& zColor, const ColourValue& xyColor, const ColourValue& yzColor,
                       const ColourValue& zxColor)
{
    using namespace Ogre;

    ManualObject* mMesh = manager->createManualObject(name + "ManualObject");

    constexpr Real PI = Math::PI;
    const float division = (PI / 2.0f) / 16.0f;
    const Real start = division * 3;
    const Real end = division * 14;

    auto addAxis = [&](const Quaternion& rot, const ColourValue& color)
    {
        mMesh->begin("AxisGizmo_Material", RenderOperation::OT_LINE_LIST);
        addLine(Vector3f::ZERO, Vector3f(3, 0, 0), rot, mMesh, color);
        mMesh->end();

        mMesh->begin("AxisGizmo_Material", RenderOperation::OT_LINE_STRIP);
        int arcBase = 0;
        for (Real t = start; t < end; t += division)
        {
            addPosition(Vector3f(0, 3 * cos(t), 3 * sin(t)), rot, mMesh, color);
            mMesh->index(arcBase++);
        }
        mMesh->end();

        // Translate
        mMesh->begin("AxisGizmo_Material", RenderOperation::OT_TRIANGLE_LIST);
        int base = 0;
        addPosition(Vector3f(2.85f, 0, 0), rot, mMesh, color);
        addCircle(2.95f, rot, mMesh, color);
        addPosition(Vector3f(3.45f, 0, 0), rot, mMesh, color);
        addFan(base, base + 1, 16, mMesh);
        addFan(base + 17, base + 1, 16, mMesh);
        mMesh->end();

        // Rotate
        mMesh->begin("AxisGizmo_Material", RenderOperation::OT_TRIANGLE_LIST);
        Quaternion q1(Degree(-90), Vector3f::UNIT_Z);
        Quaternion q2(Degree(90), Vector3f::UNIT_Y);
        Vector3f t1(0, 3 * cos(end), 3 * sin(end));
        Vector3f t2(0, 3 * cos(start), 3 * sin(start) - 0.25f);
        int rotBase = 0;
        addRotatedCircle(q1, t1, -0.3f, rot, mMesh, color);
        addRotatedCircle(q2, t2, -0.3f, rot, mMesh, color);
        addFan(rotBase, rotBase + 1, 16, mMesh);
        addFan(rotBase + 17, rotBase + 1, 16, mMesh);
        addFan(rotBase + 18, rotBase + 19, 16, mMesh);
        addFan(rotBase + 35, rotBase + 19, 16, mMesh);
        mMesh->end();

        // Scale
        mMesh->begin("AxisGizmo_Material", RenderOperation::OT_TRIANGLE_LIST);
        int scaleBase = 0;
        addPosition(Vector3f(2.85f, 0, 0), rot, mMesh, color);
        addCircle(2.85f, rot, mMesh, color);
        addPosition(Vector3f(3.45f, 0, 0), rot, mMesh, color);

        static const Vector3f cube[] = {{3.40f, 0.20f, 0.20f},   {3.40f, 0.20f, -0.20f}, {3.40f, -0.20f, -0.20f},
                                       {3.40f, -0.20f, 0.20f},  {3.50f, 0.20f, 0.20f},  {3.50f, 0.20f, -0.20f},
                                       {3.50f, -0.20f, -0.20f}, {3.50f, -0.20f, 0.20f}};

        for (auto& v : cube)
            addPosition(v, rot, mMesh, color);

        addFan(scaleBase, scaleBase + 1, 16, mMesh);
        addFan(scaleBase + 17, scaleBase + 1, 16, mMesh);

        const int faces[][6] = {{18, 19, 20, 18, 20, 21}, {22, 23, 24, 22, 24, 25}, {18, 22, 25, 18, 25, 21},
                                {19, 23, 24, 19, 24, 20}, {18, 22, 23, 18, 23, 19}, {21, 20, 24, 21, 24, 25}};

        for (auto& f : faces)
            for (int i : f)
                mMesh->index(i);
        mMesh->end();
    };

    Quaternion qy1(Degree(90), Vector3f::UNIT_Z);
    Quaternion qy2(Degree(90), Vector3f::UNIT_X);
    Quaternion qz1(Degree(-90), Vector3f::UNIT_Y);
    Quaternion qz2(Degree(-90), Vector3f::UNIT_X);
    Quaternion rotY = qy1 * qy2;
    Quaternion rotZ = qz1 * qz2;

    addAxis(Quaternion::IDENTITY, xColor);
    addAxis(rotY, yColor);
    addAxis(rotZ, zColor);

    auto addPlane = [&](const Quaternion& rot, const ColourValue& color)
    {
        mMesh->begin("AxisGizmo_Material", RenderOperation::OT_TRIANGLE_LIST);
        for (auto& v : {Vector3f(0, 0, 0), Vector3f(1, 0, 0), Vector3f(1, 1, 0), Vector3f(0, 1, 0)})
        {
            addPosition(v, rot, mMesh, color);
        }
        for (int i : {0, 1, 2, 0, 2, 3})
            mMesh->index(i);
        mMesh->end();
    };

    addPlane(Quaternion::IDENTITY, xyColor);
    addPlane(rotY, yzColor);
    addPlane(rotZ, zxColor);

    mMesh->convertToMesh(name, RGN_INTERNAL);
    manager->destroyManualObject(mMesh);
}

void Gizmo::scaleToParent()
{
    if (!mParentNode)
        return;

    AxisAlignedBox worldAABB;
    bool found = false;

    // Accumulate bounds of all attached movable objects
    for (unsigned i = 0; i < mParentNode->numAttachedObjects(); ++i)
    {
        MovableObject* mo = mParentNode->getAttachedObject(i);

        if (!mo)
            continue;

        AxisAlignedBox box = mo->getWorldBoundingBox(true);
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
    Vector3f size = worldAABB.getSize();

    Real maxExtent = std::max({size.x, size.y, size.z});

    // Avoid degenerate scale
    if (maxExtent < Real(1e-6))
        maxExtent = 1.0f;

    constexpr Real gizmoScaleFactor = 0.1f;

    Real scale = maxExtent * gizmoScaleFactor;

    mGizmoNode->setScale(Vector3f(scale));
}

bool Gizmo::pickAxis(Ray& ray)
{
    const Ray localRay = toLocalRay(ray);
    constexpr Real axisLen = 4.0f;
    constexpr Real axisRadius = 0.3f;
    constexpr Real planeSize = 1.0f;
    constexpr Real planeThickness = 0.25f;
    constexpr Real ringRadius = 3.0f;
    constexpr Real ringThickness = 0.25f;

    PlaneBoundedVolume axisX(Plane::POSITIVE_SIDE);
    axisX.planes.emplace_back(Vector3f::UNIT_X, Vector3f(axisLen, 0, 0));
    axisX.planes.emplace_back(-Vector3f::UNIT_X, Vector3f(0, 0, 0));
    axisX.planes.emplace_back(Vector3f::UNIT_Y, Vector3f(0, axisRadius, 0));
    axisX.planes.emplace_back(-Vector3f::UNIT_Y, Vector3f(0, -axisRadius, 0));
    axisX.planes.emplace_back(Vector3f::UNIT_Z, Vector3f(0, 0, axisRadius));
    axisX.planes.emplace_back(-Vector3f::UNIT_Z, Vector3f(0, 0, -axisRadius));

    PlaneBoundedVolume axisY(Plane::POSITIVE_SIDE);
    axisY.planes.emplace_back(Vector3f::UNIT_Y, Vector3f(0, axisLen, 0));
    axisY.planes.emplace_back(-Vector3f::UNIT_Y, Vector3f(0, 0, 0));
    axisY.planes.emplace_back(Vector3f::UNIT_X, Vector3f(axisRadius, 0, 0));
    axisY.planes.emplace_back(-Vector3f::UNIT_X, Vector3f(-axisRadius, 0, 0));
    axisY.planes.emplace_back(Vector3f::UNIT_Z, Vector3f(0, 0, axisRadius));
    axisY.planes.emplace_back(-Vector3f::UNIT_Z, Vector3f(0, 0, -axisRadius));

    PlaneBoundedVolume axisZ(Plane::POSITIVE_SIDE);
    axisZ.planes.emplace_back(Vector3f::UNIT_Z, Vector3f(0, 0, axisLen));
    axisZ.planes.emplace_back(-Vector3f::UNIT_Z, Vector3f(0, 0, 0));
    axisZ.planes.emplace_back(Vector3f::UNIT_X, Vector3f(axisRadius, 0, 0));
    axisZ.planes.emplace_back(-Vector3f::UNIT_X, Vector3f(-axisRadius, 0, 0));
    axisZ.planes.emplace_back(Vector3f::UNIT_Y, Vector3f(0, axisRadius, 0));
    axisZ.planes.emplace_back(-Vector3f::UNIT_Y, Vector3f(0, -axisRadius, 0));

    PlaneBoundedVolume planeXY(Plane::POSITIVE_SIDE);
    planeXY.planes.emplace_back(Vector3f::UNIT_X, Vector3f(planeSize, 0, 0));
    planeXY.planes.emplace_back(-Vector3f::UNIT_X, Vector3f(0, 0, 0));
    planeXY.planes.emplace_back(Vector3f::UNIT_Y, Vector3f(0, planeSize, 0));
    planeXY.planes.emplace_back(-Vector3f::UNIT_Y, Vector3f(0, 0, 0));
    planeXY.planes.emplace_back(Vector3f::UNIT_Z, Vector3f(0, 0, planeThickness));
    planeXY.planes.emplace_back(-Vector3f::UNIT_Z, Vector3f(0, 0, -planeThickness));

    PlaneBoundedVolume planeYZ(Plane::POSITIVE_SIDE);
    planeYZ.planes.emplace_back(Vector3f::UNIT_Y, Vector3f(0, planeSize, 0));
    planeYZ.planes.emplace_back(-Vector3f::UNIT_Y, Vector3f(0, 0, 0));
    planeYZ.planes.emplace_back(Vector3f::UNIT_Z, Vector3f(0, 0, planeSize));
    planeYZ.planes.emplace_back(-Vector3f::UNIT_Z, Vector3f(0, 0, 0));
    planeYZ.planes.emplace_back(Vector3f::UNIT_X, Vector3f(planeThickness, 0, 0));
    planeYZ.planes.emplace_back(-Vector3f::UNIT_X, Vector3f(-planeThickness, 0, 0));

    PlaneBoundedVolume planeXZ(Plane::POSITIVE_SIDE);
    planeXZ.planes.emplace_back(Vector3f::UNIT_X, Vector3f(planeSize, 0, 0));
    planeXZ.planes.emplace_back(-Vector3f::UNIT_X, Vector3f(0, 0, 0));
    planeXZ.planes.emplace_back(Vector3f::UNIT_Z, Vector3f(0, 0, planeSize));
    planeXZ.planes.emplace_back(-Vector3f::UNIT_Z, Vector3f(0, 0, 0));
    planeXZ.planes.emplace_back(Vector3f::UNIT_Y, Vector3f(0, planeThickness, 0));
    planeXZ.planes.emplace_back(-Vector3f::UNIT_Y, Vector3f(0, -planeThickness, 0));

    PlaneBoundedVolume ringX(Plane::POSITIVE_SIDE);
    ringX.planes.emplace_back(Vector3f::UNIT_X, Vector3f(ringThickness, 0, 0));
    ringX.planes.emplace_back(-Vector3f::UNIT_X, Vector3f(-ringThickness, 0, 0));
    ringX.planes.emplace_back(Vector3f::UNIT_Y, Vector3f(0, ringRadius + ringThickness, 0));
    ringX.planes.emplace_back(-Vector3f::UNIT_Y, Vector3f(0, -ringRadius - ringThickness, 0));
    ringX.planes.emplace_back(Vector3f::UNIT_Z, Vector3f(0, 0, ringRadius + ringThickness));
    ringX.planes.emplace_back(-Vector3f::UNIT_Z, Vector3f(0, 0, -ringRadius - ringThickness));

    PlaneBoundedVolume ringY(Plane::POSITIVE_SIDE);
    ringY.planes.emplace_back(Vector3f::UNIT_Y, Vector3f(0, ringThickness, 0));
    ringY.planes.emplace_back(-Vector3f::UNIT_Y, Vector3f(0, -ringThickness, 0));
    ringY.planes.emplace_back(Vector3f::UNIT_X, Vector3f(ringRadius + ringThickness, 0, 0));
    ringY.planes.emplace_back(-Vector3f::UNIT_X, Vector3f(-ringRadius - ringThickness, 0, 0));
    ringY.planes.emplace_back(Vector3f::UNIT_Z, Vector3f(0, 0, ringRadius + ringThickness));
    ringY.planes.emplace_back(-Vector3f::UNIT_Z, Vector3f(0, 0, -ringRadius - ringThickness));

    PlaneBoundedVolume ringZ(Plane::POSITIVE_SIDE);
    ringZ.planes.emplace_back(Vector3f::UNIT_Z, Vector3f(0, 0, ringThickness));
    ringZ.planes.emplace_back(-Vector3f::UNIT_Z, Vector3f(0, 0, -ringThickness));
    ringZ.planes.emplace_back(Vector3f::UNIT_X, Vector3f(ringRadius + ringThickness, 0, 0));
    ringZ.planes.emplace_back(-Vector3f::UNIT_X, Vector3f(-ringRadius - ringThickness, 0, 0));
    ringZ.planes.emplace_back(Vector3f::UNIT_Y, Vector3f(0, ringRadius + ringThickness, 0));
    ringZ.planes.emplace_back(-Vector3f::UNIT_Y, Vector3f(0, -ringRadius - ringThickness, 0));

    struct PickVolume
    {
        PlaneBoundedVolume vol;
        AXIS axis;
        GizmoMode mode;
    };

    const PickVolume volumes[] = {
        {axisX, AXIS_X, G_TRANSLATE},    {axisY, AXIS_Y, G_TRANSLATE},    {axisZ, AXIS_Z, G_TRANSLATE},
        {planeXY, AXIS_XY, G_TRANSLATE}, {planeYZ, AXIS_YZ, G_TRANSLATE}, {planeXZ, AXIS_XZ, G_TRANSLATE},
        {ringX, AXIS_X, G_ROTATE},       {ringY, AXIS_Y, G_ROTATE},       {ringZ, AXIS_Z, G_ROTATE},
    };

    Real bestT = Math::POS_INFINITY;
    AXIS bestAxis = AXIS_NONE;

    for (const auto& v : volumes)
    {
        if (mMode == G_ROTATE)
        {
            if (v.mode != G_ROTATE)
                continue;
        }
        else if (mMode == G_TRANSLATE || mMode == G_SCALE)
        {
            if (v.mode == G_ROTATE)
                continue;
        }
        else
        {
            continue;
        }

        std::pair<bool, Real> hit = localRay.intersects(v.vol);
        if (hit.first && hit.second < bestT)
        {
            bestT = hit.second;
            bestAxis = v.axis;
        }
    }

    mActiveAxis = bestAxis;
    highlightAxis(mActiveAxis);
    return (mActiveAxis != AXIS_NONE);
}

void Gizmo::highlightAxis(AXIS axis)
{
    if (axis == mOldGizmoAxis)
        return;
    mOldGizmoAxis = axis;

    constexpr int axisParts = 5;
    constexpr int planeBase = axisParts * 3;
    const int totalSubMeshes = planeBase + 3;

    for (int i = 0; i < totalSubMeshes; ++i)
        mGizmoEntity->getSubEntity(i)->setMaterialName("AxisGizmo_Material");

    if (axis == AXIS_NONE)
        return;

    auto highlightAxis = [&](int axisIndex)
    {
        const int base = axisIndex * axisParts;
        for (int i = 0; i < axisParts; ++i)
            mGizmoEntity->getSubEntity(base + i)->setMaterialName("AxisGizmo_Material_Hl");
    };

    if (axis & AXIS_X)
        highlightAxis(0);
    if (axis & AXIS_Y)
        highlightAxis(1);
    if (axis & AXIS_Z)
        highlightAxis(2);

    if (axis == AXIS_XY)
        mGizmoEntity->getSubEntity(planeBase + 0)->setMaterialName("AxisGizmo_Material_Hl");
    else if (axis == AXIS_YZ)
        mGizmoEntity->getSubEntity(planeBase + 1)->setMaterialName("AxisGizmo_Material_Hl");
    else if (axis == AXIS_XZ)
        mGizmoEntity->getSubEntity(planeBase + 2)->setMaterialName("AxisGizmo_Material_Hl");
}

Ray Gizmo::toLocalRay(const Ray& worldRay) const
{
    Matrix4 inv = mGizmoNode->_getFullTransform().inverse();

    Vector3f localOrigin = inv * worldRay.getOrigin();
    Vector3f localDir = inv.linear().inverse().transpose() * worldRay.getDirection();

    localDir.normalise();

    return Ray(localOrigin, localDir);
}

CameraGizmo::CameraGizmo(SceneNode* cameraNode, CameraMan* cameraMan)
{
    auto* gizmoSm = cameraNode->getCreator();

    mCameraMan = cameraMan;
    mCameraNode = cameraNode;
    mCamera = dynamic_cast<Camera*>(cameraMan->getCamera()->getAttachedObject(0));
    mGizmoNode = mCameraNode->createChildSceneNode("GizmoRoot");
    mGizmoNode->setInheritOrientation(true);
    mGizmoNode->setInheritScale(false);

    createMesh(gizmoSm, "AxisGizmosMesh");

    if (mCamera)
    {
        const float nx = mOverlayLeft + mOverlayWidth * 0.5f;
        const float ny = mOverlayTop + mOverlayHeight * 0.5f;
        const float depth = std::max(mCamera->getNearClipDistance() * 10.0f, 1.0f);
        const float halfHeight = Math::Tan(mCamera->getFOVy() * 0.5f) * depth;
        const float halfWidth = halfHeight * mCamera->getAspectRatio();
        const float ndcX = nx * 2.0f - 1.0f;
        const float ndcY = 1.0f - ny * 2.0f;
        const float regionWidth = mOverlayWidth * 2.0f * halfWidth;
        const float regionHeight = mOverlayHeight * 2.0f * halfHeight;
        const float size = std::max(std::min(regionWidth, regionHeight) * 0.9f * 0.75f, 0.01f);

        // Camera-local placement; parent orientation rotates this into world space.
        mGizmoNode->setPosition(Vector3f(ndcX * halfWidth, ndcY * halfHeight, -depth));
        mGizmoNode->setScale(Vector3f(size));
    }

    updateOrientation();
}

void CameraGizmo::updateOrientation()
{
    if (!mCamera || !mGizmoNode)
        return;

    // Counter-rotate so the gizmo stays aligned to camera axes.
    mGizmoNode->setOrientation(mCamera->getDerivedOrientation().Inverse());
}

ManualObject* CameraGizmo::pickFace(float nx, float ny)
{
    if (!mCamera)
        return nullptr;
    const Ray worldRay = mCamera->getCameraToViewportRay(nx, ny);

    ManualObject* face = nullptr;
    Real closest = std::numeric_limits<Real>::max();

    for (int i = 0; i < 6; ++i)
    {
        if (!mFaceNodes[i] || !mGizmoObjects[i])
            continue;

        const Vector3f worldNormal = mFaceNodes[i]->_getDerivedOrientation() * Vector3f::UNIT_Z;
        if (worldNormal.dotProduct(worldRay.getDirection()) >= 0.0f)
            continue;

        const Matrix4 xf = mFaceNodes[i]->_getFullTransform();
        const Vector3f v0 = xf * Vector3f(-0.5f, -0.5f, 0.0f);
        const Vector3f v1 = xf * Vector3f(0.5f, -0.5f, 0.0f);
        const Vector3f v2 = xf * Vector3f(0.5f, 0.5f, 0.0f);
        const Vector3f v3 = xf * Vector3f(-0.5f, 0.5f, 0.0f);

        auto tryTriangle = [&](const Vector3f& a, const Vector3f& b, const Vector3f& c)
        {
            auto hit = Math::intersects(worldRay, a, b, c, true, true);
            if (!hit.first || hit.second < 0.0f)
                return;
            if (hit.second < closest)
            {
                closest = hit.second;
                face = mGizmoObjects[i];
            }
        };

        tryTriangle(v0, v1, v2);
        tryTriangle(v0, v2, v3);
    }
    highlightFace(face);
    return face;
}

void CameraGizmo::snapCamera(ManualObject* face) const
{
    using namespace Ogre;

    if (!face || !mCamera)
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

    Vector3f camPos = mCameraNode->getPosition();
    Vector3f target = mCameraMan->getTarget()->getPosition();

    Real dist = camPos.distance(target);

    mCameraMan->setYawPitchDist(yaw, pitch, dist);
}

void CameraGizmo::highlightFace(ManualObject* face)
{
    static const ColourValue baseX(0.1f, 0.1f, 0.8f, 1.0f);
    static const ColourValue baseY(0.1f, 0.8f, 0.2f, 1.0f);
    static const ColourValue baseZ(0.8f, 0.1f, 0.1f, 1.0f);
    static const ColourValue hlX(0.0f, 0.0f, 1.0f, 1.0f);
    static const ColourValue hlY(0.0f, 1.0f, 0.0f, 1.0f);
    static const ColourValue hlZ(1.0f, 0.0f, 0.0f, 1.0f);

    if (!face)
    {
        for (int i = 0; i < 6; ++i)
            mGizmoObjects[i]->setMaterialName(0, "AxisGizmo_Material");

        updateCameraQuad(mGizmoObjects[0], baseX);
        updateCameraQuad(mGizmoObjects[1], baseX);
        updateCameraQuad(mGizmoObjects[2], baseY);
        updateCameraQuad(mGizmoObjects[3], baseY);
        updateCameraQuad(mGizmoObjects[4], baseZ);
        updateCameraQuad(mGizmoObjects[5], baseZ);
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
    for (int i = 0; i < 6; ++i)
        mGizmoObjects[i]->setMaterialName(0, "AxisGizmo_Material");

    updateCameraQuad(mGizmoObjects[0], baseX);
    updateCameraQuad(mGizmoObjects[1], baseX);
    updateCameraQuad(mGizmoObjects[2], baseY);
    updateCameraQuad(mGizmoObjects[3], baseY);
    updateCameraQuad(mGizmoObjects[4], baseZ);
    updateCameraQuad(mGizmoObjects[5], baseZ);
    if (faceIndex == -1)
    {
        mOldFaceIndex = -1;
        return;
    }
    switch (faceIndex)
    {
    case 0:
    case 1:
        updateCameraQuad(mGizmoObjects[faceIndex], hlX);
        break;
    case 2:
    case 3:
        updateCameraQuad(mGizmoObjects[faceIndex], hlY);
        break;
    case 4:
    case 5:
        updateCameraQuad(mGizmoObjects[faceIndex], hlZ);
        break;
    default:
        break;
    }
}

void CameraGizmo::createMesh(SceneManager* manager, String name)
{
    using namespace Ogre;

    auto createFace = [&](const String& n, const ColourValue& color, const int index)
    {
        SceneNode* faceNode = mGizmoNode->createChildSceneNode(n);

        // Face quad
        ManualObject* quad = manager->createManualObject(n + "_Face");
        quad->setDynamic(true);
        quad->begin("AxisGizmo_Material", RenderOperation::OT_TRIANGLE_LIST);
        quad->position(-0.5f, -0.5f, 0.0f);
        quad->colour(color);
        quad->position(0.5f, -0.5f, 0.0f);
        quad->colour(color);
        quad->position(0.5f, 0.5f, 0.0f);
        quad->colour(color);
        quad->position(-0.5f, 0.5f, 0.0f);
        quad->colour(color);
        quad->triangle(0, 1, 2);
        quad->triangle(0, 2, 3);
        quad->end();

        quad->setQueryFlags(QUERYFLAG_GIZMO); // pickable
        quad->setCastShadows(false);
        quad->setMaterialName(0, "AxisGizmo_Material");
        quad->setRenderQueueGroup(RENDER_QUEUE_OVERLAY);
        faceNode->attachObject(quad);
        mGizmoObjects[index] = quad;
        mFaceNodes[index] = faceNode;

        return faceNode;
    };
    SceneNode* px = createFace(name + "_PX", ColourValue(0.2f, 0.2f, 0.9f, 1.0f), 0);
    px->setPosition(0.5f, 0, 0);
    px->yaw(Degree(90));
    SceneNode* nx = createFace(name + "_NX", ColourValue(0.2f, 0.2f, 0.9f, 1.0f), 1);
    nx->setPosition(-0.5f, 0, 0);
    nx->yaw(Degree(-90));
    SceneNode* py = createFace(name + "_PY", ColourValue(0.2f, 0.9f, 0.2f, 1.0f), 2);
    py->setPosition(0, 0.5f, 0);
    py->pitch(Degree(-90));
    SceneNode* ny = createFace(name + "_NY", ColourValue(0.2f, 0.9f, 0.2f, 1.0f), 3);
    ny->setPosition(0, -0.5f, 0);
    ny->pitch(Degree(90));
    SceneNode* pz = createFace(name + "_PZ", ColourValue(0.9f, 0.2f, 0.2f, 1.0f), 4);
    pz->setPosition(0, 0, 0.5f);
    SceneNode* nz = createFace(name + "_NZ", ColourValue(0.9f, 0.2f, 0.2f, 1.0f), 5);
    nz->setPosition(0, 0, -0.5f);
    nz->yaw(Degree(180));
}

} // namespace OgreBites
