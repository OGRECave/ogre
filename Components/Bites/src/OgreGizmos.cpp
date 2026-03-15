#include "OgreGizmos.h"
#include "Ogre.h"

namespace OgreBites

{
using namespace Ogre;
namespace
{
// exclude from queries
constexpr uint32_t QUERYFLAG_GIZMO = 0;

enum AXIS
{
    AXIS_NONE = 0,
    AXIS_X = 1 << 0, // 1
    AXIS_Y = 1 << 1, // 2
    AXIS_Z = 1 << 2, // 4

    AXIS_XY = AXIS_X | AXIS_Y, // 3
    AXIS_YZ = AXIS_Y | AXIS_Z, // 6
    AXIS_XZ = AXIS_X | AXIS_Z  // 5
};

// --- shared geometry helpers ---

void addPosition(const Vector3& p, const Quaternion& rot, ManualObject* mesh, const ColourValue& color)
{
    mesh->position(rot * p);
    mesh->colour(color);
}

void addQuadVertices(ManualObject* mo, const ColourValue& color)
{
    for (auto& v : {Vector3(-0.5f, -0.5f, 0), Vector3(0.5f, -0.5f, 0), Vector3(0.5f, 0.5f, 0), Vector3(-0.5f, 0.5f, 0)})
    {
        mo->position(v);
        mo->colour(color);
    }

    mo->quad(0, 1, 2, 3);
}

void addFan(int center, int start, int count, ManualObject* mesh)
{
    for (int i = 0; i < count - 1; ++i)
        mesh->triangle(center, start + i, start + i + 1);
    mesh->triangle(center, start + count - 1, start);
}

// --- axis gizmo mesh building ---

constexpr Real CIRCLE_RADIUS = 0.22f;

void addCircleVertices(Real x, const Quaternion& rot, ManualObject* mesh, const ColourValue& color,
                       const Quaternion& orient = Quaternion::IDENTITY, const Vector3& offset = Vector3::ZERO)
{
    for (Real t = 0; t < 2 * Math::PI; t += Math::PI / 8.0f)
        addPosition(orient * Vector3(x, CIRCLE_RADIUS * cos(t), CIRCLE_RADIUS * sin(t)) + offset, rot, mesh, color);
}

void addRotatedCircle(const Quaternion& q, const Vector3& t, Real x, const Quaternion& rot, ManualObject* mesh,
                      const ColourValue& color)
{
    addPosition(q * Vector3(x, 0, 0) + t, rot, mesh, color);
    addCircleVertices(x, rot, mesh, color, q, t);
    addPosition(q * Vector3(-x, 0, 0) + t, rot, mesh, color);
}

void addLine(const Vector3& a, const Vector3& b, const Quaternion& rot, ManualObject* mesh, const ColourValue& color)
{
    addPosition(a, rot, mesh, color);
    addPosition(b, rot, mesh, color);
    mesh->index(0);
    mesh->index(1);
}

constexpr int AXIS_PARTS = 5;
constexpr int PLANE_BASE = AXIS_PARTS * 3;

void addAxisMesh(ManualObject* mesh, const Quaternion& rot, const ColourValue& color, Real arcStart, Real arcEnd,
                 Real division)
{
    auto mat = MaterialManager::getSingleton().getByName("Ogre/AxisGizmo", RGN_INTERNAL);
    mesh->begin(mat, RenderOperation::OT_LINE_LIST);
    addLine(Vector3::ZERO, Vector3(3, 0, 0), rot, mesh, color);
    mesh->end();

    mesh->begin(mat, RenderOperation::OT_LINE_STRIP);
    int arcBase = 0;
    for (Real t = arcStart; t < arcEnd; t += division)
    {
        addPosition(Vector3(0, 3 * cos(t), 3 * sin(t)), rot, mesh, color);
        mesh->index(arcBase++);
    }
    mesh->end();

    // Translate
    mesh->begin(mat, RenderOperation::OT_TRIANGLE_LIST);
    int base = 0;
    addPosition(Vector3(2.85f, 0, 0), rot, mesh, color);
    addCircleVertices(2.95f, rot, mesh, color);
    addPosition(Vector3(3.45f, 0, 0), rot, mesh, color);
    addFan(base, base + 1, 16, mesh);
    addFan(base + 17, base + 1, 16, mesh);
    mesh->end();

    // Rotate
    mesh->begin(mat, RenderOperation::OT_TRIANGLE_LIST);
    Quaternion q1(Degree(-90), Vector3::UNIT_Z);
    Quaternion q2(Degree(90), Vector3::UNIT_Y);
    Vector3 t1(0, 3 * cos(arcEnd), 3 * sin(arcEnd));
    Vector3 t2(0, 3 * cos(arcStart), 3 * sin(arcStart) - 0.25f);
    int rotBase = 0;
    addRotatedCircle(q1, t1, -0.3f, rot, mesh, color);
    addRotatedCircle(q2, t2, -0.3f, rot, mesh, color);
    addFan(rotBase, rotBase + 1, 16, mesh);
    addFan(rotBase + 17, rotBase + 1, 16, mesh);
    addFan(rotBase + 18, rotBase + 19, 16, mesh);
    addFan(rotBase + 35, rotBase + 19, 16, mesh);
    mesh->end();

    // Scale
    mesh->begin(mat, RenderOperation::OT_TRIANGLE_LIST);
    int scaleBase = 0;
    addPosition(Vector3(2.85f, 0, 0), rot, mesh, color);
    addCircleVertices(2.85f, rot, mesh, color);
    addPosition(Vector3(3.45f, 0, 0), rot, mesh, color);

    static const Vector3 cube[] = {{3.40f, 0.20f, 0.20f},   {3.40f, 0.20f, -0.20f}, {3.40f, -0.20f, -0.20f},
                                    {3.40f, -0.20f, 0.20f},  {3.50f, 0.20f, 0.20f},  {3.50f, 0.20f, -0.20f},
                                    {3.50f, -0.20f, -0.20f}, {3.50f, -0.20f, 0.20f}};

    for (auto& v : cube)
        addPosition(v, rot, mesh, color);

    addFan(scaleBase, scaleBase + 1, 16, mesh);
    addFan(scaleBase + 17, scaleBase + 1, 16, mesh);

    mesh->quad(18, 19, 20, 21); // -X face
    mesh->quad(22, 23, 24, 25); // +X face
    mesh->quad(18, 22, 25, 21); // +Z face
    mesh->quad(19, 23, 24, 20); // -Z face
    mesh->quad(18, 22, 23, 19); // +Y face
    mesh->quad(21, 20, 24, 25); // -Y face
    mesh->end();
}

void addPlaneMesh(ManualObject* mesh, const Quaternion& rot, const ColourValue& color)
{
    mesh->begin("Ogre/AxisGizmo", RenderOperation::OT_TRIANGLE_LIST, RGN_INTERNAL);
    for (auto& v : {Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(1, 1, 0), Vector3(0, 1, 0)})
        addPosition(v, rot, mesh, color);

    mesh->quad(0, 1, 2, 3);
    mesh->end();
}

// --- axis gizmo interaction ---

void showPerAxis(Entity* entity, int offset, bool visible = true)
{
    for (int axis = 0; axis < 3; ++axis)
        entity->getSubEntity(axis * AXIS_PARTS + offset)->setVisible(visible);
}

void showPlanes(Entity* entity)
{
    for (int i = 0; i < 3; ++i)
        entity->getSubEntity(PLANE_BASE + i)->setVisible(true);
}

PlaneBoundedVolume makeBox(const Vector3& lo, const Vector3& hi)
{
    PlaneBoundedVolume vol(Plane::POSITIVE_SIDE);
    const Vector3 axes[] = {Vector3::UNIT_X, Vector3::UNIT_Y, Vector3::UNIT_Z};
    for (int i = 0; i < 3; ++i)
    {
        vol.planes.emplace_back(axes[i], axes[i] * hi[i]);
        vol.planes.emplace_back(-axes[i], axes[i] * lo[i]);
    }
    return vol;
}

void highlightAxisSubmeshes(Entity* entity, int axisIndex)
{
    const int base = axisIndex * AXIS_PARTS;
    for (int i = 0; i < AXIS_PARTS; ++i)
        entity->getSubEntity(base + i)->setMaterialName("Ogre/AxisGizmo_Highlight");
}

// --- camera gizmo ---

void updateCameraQuad(ManualObject* mo, const ColourValue& color)
{
    mo->beginUpdate(0);
    addQuadVertices(mo, color);
    mo->end();
}

bool tryQuadHit(const Ray& ray, const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d, Real& closest)
{
    auto hit = Math::intersects(ray, a, b, c, true, false);
    if(!hit.first)
        hit = Math::intersects(ray, a, c, d, true, false);

    if (!hit.first || hit.second < 0.0f)
        return false;

    if (hit.second < closest)
    {
        closest = hit.second;
        return true;
    }
    return false;
}

SceneNode* createCameraFace(SceneNode* parentNode, SceneManager* manager, const String& n,
                            const ColourValue& color, ManualObject*& outQuad)
{
    SceneNode* faceNode = parentNode->createChildSceneNode(n);

    ManualObject* quad = manager->createManualObject(n + "_Face");
    quad->setDynamic(true);
    quad->begin("Ogre/CameraGizmo", RenderOperation::OT_TRIANGLE_LIST, RGN_INTERNAL);
    addQuadVertices(quad, color);
    quad->end();

    quad->setQueryFlags(QUERYFLAG_GIZMO);
    quad->setCastShadows(false);
    quad->setRenderQueueGroup(RENDER_QUEUE_OVERLAY);
    faceNode->attachObject(quad);
    outQuad = quad;

    return faceNode;
}

Vector3 computePlaneHit(const Ray& ray, const Vector3& mDragAxis, const Vector3& cameraDir, const Vector3& planePoint)
{
    Vector3 v = mDragAxis.crossProduct(cameraDir);

    // handle degeneracy (camera looking along mDragAxis)
    if (v.squaredLength() < 1e-6f)
        v = mDragAxis.perpendicular();

    Vector3 planeNormal = v.crossProduct(mDragAxis).normalisedCopy();
    Plane plane(planeNormal, planePoint);

    auto r = ray.intersects(plane);
    if (!r.first)
        return planePoint; // fallback

    return ray.getPoint(r.second);
}

} // namespace

Gizmo::Gizmo(SceneNode* sceneNode, GizmoOperation mode, Viewport* viewport) : mOperation(GO_ROTATE)
{
    mViewport = viewport;

    auto manager = sceneNode->getCreator();
    if(!MeshManager::getSingleton().resourceExists("AxisGizmosMesh"))
    {
        const float division = Math::HALF_PI / 16.0f;
        const Real arcStart = division * 3;
        const Real arcEnd = division * 14;

        const Quaternion rotY = Quaternion(Degree(90), Vector3::UNIT_Z) * Quaternion(Degree(90), Vector3::UNIT_X);
        const Quaternion rotZ = Quaternion(Degree(-90), Vector3::UNIT_Y) * Quaternion(Degree(-90), Vector3::UNIT_X);

        const ColourValue xColor(1.0f, 0.0f, 0.0f, 0.50f);
        const ColourValue yColor(0.0f, 1.0f, 0.0f, 0.50f);
        const ColourValue zColor(0.0f, 0.0f, 1.0f, 0.50f);
        const ColourValue xyColor(1.0f, 1.0f, 0.0f, 0.50f);
        const ColourValue yzColor(0.0f, 1.0f, 1.0f, 0.50f);
        const ColourValue zxColor(1.0f, 0.0f, 1.0f, 0.50f);

        ManualObject* mo = manager->createManualObject("AxisGizmosMesh_ManualObject");
        mo->setBufferUsage(HBU_CPU_TO_GPU); // allows vulkan to copy buffers in spite of incomplete impl
        addAxisMesh(mo, Quaternion::IDENTITY, xColor, arcStart, arcEnd, division);
        addAxisMesh(mo, rotY, yColor, arcStart, arcEnd, division);
        addAxisMesh(mo, rotZ, zColor, arcStart, arcEnd, division);

        addPlaneMesh(mo, Quaternion::IDENTITY, xyColor);
        addPlaneMesh(mo, rotY, yzColor);
        addPlaneMesh(mo, rotZ, zxColor);

        mo->convertToMesh("AxisGizmosMesh", RGN_INTERNAL);
        manager->destroyManualObject(mo);
    }

    setTargetNode(sceneNode);

    scaleToParent();

    mGizmoEntity = sceneNode->getCreator()->createEntity("scbw", "AxisGizmosMesh", RGN_INTERNAL);
    mGizmoEntity->setCastShadows(false);
    mGizmoEntity->setRenderQueueGroup(RENDER_QUEUE_OVERLAY);
    mGizmoEntity->setQueryFlags(QUERYFLAG_GIZMO);

    mGizmoNode->attachObject(mGizmoEntity);
    setOperation(mode);

    mDragging = false;
}

void Gizmo::setTargetNode(SceneNode* target)
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
        mGizmoNode->setInheritOrientation(mOperation == GO_SCALE || mOperation == GO_ROTATE);
    }
    else
    {
        mParentNode->addChild(mGizmoNode);
    }

    mGizmoNode->setPosition(Vector3::ZERO);
    mGizmoNode->setOrientation(Quaternion::IDENTITY);
    scaleToParent();
}

void Gizmo::setOperation(GizmoOperation mode)
{
    if (mOperation == mode)
        return;
    mOperation = mode;

    for (int axis = 0; axis < 3; ++axis)
    {
        const int base = axis * AXIS_PARTS;
        mGizmoEntity->getSubEntity(base + 0)->setVisible(true);
        for (int i = 1; i < AXIS_PARTS; ++i)
            mGizmoEntity->getSubEntity(base + i)->setVisible(false);
    }

    for (int i = 0; i < 3; ++i)
        mGizmoEntity->getSubEntity(PLANE_BASE + i)->setVisible(false);

    switch (mode)
    {
    case GO_TRANSLATE:
        showPerAxis(mGizmoEntity, 2);
        showPlanes(mGizmoEntity);
        break;
    case GO_ROTATE:
        showPerAxis(mGizmoEntity, 0, false);
        showPerAxis(mGizmoEntity, 1);
        showPerAxis(mGizmoEntity, 3);
        break;
    case GO_SCALE:
        showPerAxis(mGizmoEntity, 4);
        showPlanes(mGizmoEntity);
        break;
    }
    mGizmoNode->setInheritOrientation(mOperation == GO_SCALE || mOperation == GO_ROTATE);
}

bool Gizmo::mouseMoved(const MouseMotionEvent& evt)
{
    auto* cam = mViewport->getCamera();
    const float nx = static_cast<float>(evt.x) / static_cast<float>(mViewport->getActualWidth());
    const float ny = static_cast<float>(evt.y) / static_cast<float>(mViewport->getActualHeight());
    if (mDragging)
    {
        Ray ray = cam->getCameraToViewportRay(nx, ny);
        computeDrag(ray, cam->getDerivedDirection());
        return true;
    }
    Ray ray = cam->getCameraToViewportRay(nx, ny);
    pickAxis(ray);
    return InputListener::mouseMoved(evt);
}

bool Gizmo::mousePressed(const MouseButtonEvent& evt)
{
    if (evt.button == BUTTON_LEFT)
    {
        auto* cam = mViewport->getCamera();
        const float nx = static_cast<float>(evt.x) / static_cast<float>(mViewport->getActualWidth());
        const float ny = static_cast<float>(evt.y) / static_cast<float>(mViewport->getActualHeight());

        Ray ray = cam->getCameraToViewportRay(nx, ny);
        if (pickAxis(ray))
        {
            startDrag(cam->getCameraToViewportRay(nx, ny), cam->getDerivedDirection());
            return true;
        }
    }
    return InputListener::mousePressed(evt);
}

bool Gizmo::mouseReleased(const MouseButtonEvent& evt)
{
    if (evt.button == BUTTON_LEFT)
    {
        if (mDragging)
        {
            stopDrag();
            return true;
        }
    }
    return InputListener::mouseReleased(evt);
}

void Gizmo::startDrag(const Ray& startRay, const Vector3& cameraDir)
{
    mDragging = true;

    mInitialObjectPos = mParentNode->getPosition();
    mInitialObjectRot = mParentNode->getOrientation();
    mInitialObjectScale = mParentNode->getScale();

    mDragAxisLocal = Vector3::ZERO;

    if (mActiveAxis & AXIS_X)
        mDragAxisLocal += Vector3::UNIT_X;
    if (mActiveAxis & AXIS_Y)
        mDragAxisLocal += Vector3::UNIT_Y;
    if (mActiveAxis & AXIS_Z)
        mDragAxisLocal += Vector3::UNIT_Z;

    if (mDragAxisLocal.isZeroLength())
        return;

    const int axisCount =
        ((mActiveAxis & AXIS_X) ? 1 : 0) + ((mActiveAxis & AXIS_Y) ? 1 : 0) + ((mActiveAxis & AXIS_Z) ? 1 : 0);

    mDragUsePlane = axisCount >= 2;
    mDragPlaneNormalLocal = Vector3::ZERO;

    if (mDragUsePlane)
    {
        if (mOperation == GO_SCALE)
        {
            mDragGizmoOrientation = mGizmoNode->_getDerivedOrientation();
            mDragGizmoCenter = mGizmoNode->_getDerivedPosition();
            const Quaternion invOri = mDragGizmoOrientation.Inverse();
            Ray localRay(invOri * (startRay.getOrigin() - mDragGizmoCenter), invOri * startRay.getDirection());

            if (axisCount == 2)
            {
                Vector3 a = (mActiveAxis & AXIS_X) ? Vector3::UNIT_X : Vector3::ZERO;
                Vector3 b = (mActiveAxis & AXIS_Y) ? Vector3::UNIT_Y : Vector3::ZERO;
                if (mActiveAxis & AXIS_Z)
                    b = Vector3::UNIT_Z;
                if (a.isZeroLength())
                    a = Vector3::UNIT_Y;
                mDragPlaneNormalLocal = a.crossProduct(b).normalisedCopy();
            }
            else
            {
                mDragPlaneNormalLocal = (invOri * cameraDir).normalisedCopy();
            }

            Plane plane(mDragPlaneNormalLocal, Vector3::ZERO);
            auto r = localRay.intersects(plane);
            if (!r.first)
                return;
            mDragStartHitLocal = localRay.getPoint(r.second);
            return;
        }

        if (axisCount == 2)
        {
            Vector3 a = (mActiveAxis & AXIS_X) ? Vector3::UNIT_X : Vector3::ZERO;
            Vector3 b = (mActiveAxis & AXIS_Y) ? Vector3::UNIT_Y : Vector3::ZERO;
            if (mActiveAxis & AXIS_Z)
                b = Vector3::UNIT_Z;
            if (a.isZeroLength())
                a = Vector3::UNIT_Y;
            mDragPlaneNormalLocal = a.crossProduct(b).normalisedCopy();
        }
        else
        {
            mDragPlaneNormalLocal = cameraDir.normalisedCopy();
        }

        Plane plane(mDragPlaneNormalLocal, Vector3::ZERO);
        auto r = startRay.intersects(plane);
        if (!r.first)
            return;
        mDragStartHitLocal = startRay.getPoint(r.second);
        return;
    }

    mDragAxisLocal.normalise();

    mDragAxisWorld = mGizmoNode->_getDerivedOrientation() * mDragAxisLocal;
    mDragAxisWorld.normalise();

    if (mOperation == GO_ROTATE)
    {
        mDragGizmoOrientation = mGizmoNode->_getDerivedOrientation();
        mDragGizmoCenter = mGizmoNode->_getDerivedPosition();
        const Quaternion invOri = mDragGizmoOrientation.Inverse();
        Ray localRay(invOri * (startRay.getOrigin() - mDragGizmoCenter), invOri * startRay.getDirection());

        Plane plane(mDragAxisLocal, Vector3::ZERO);
        auto r = localRay.intersects(plane);
        if (!r.first)
            return;
        mDragStartHitLocal = localRay.getPoint(r.second);
        mDragLastHitLocal = mDragStartHitLocal;
        mDragAccumulatedAngle = Radian(0);
    }
    else
    {
        if (mOperation == GO_SCALE)
        {
            mDragGizmoOrientation = mGizmoNode->_getDerivedOrientation();
            mDragGizmoCenter = mGizmoNode->_getDerivedPosition();
            const Quaternion invOri = mDragGizmoOrientation.Inverse();
            Ray localRay(invOri * (startRay.getOrigin() - mDragGizmoCenter), invOri * startRay.getDirection());
            Vector3 localCameraDir = invOri * cameraDir;
            mDragStartHitLocal = computePlaneHit(localRay, mDragAxisLocal, localCameraDir, Vector3::ZERO);
            return;
        }
        Ray localRay = startRay;
        mDragStartHitLocal = computePlaneHit(localRay, mDragAxisLocal, cameraDir, Vector3::ZERO);
    }
}

void Gizmo::computeDrag(Ray& ray, const Vector3& cameraDir)
{
    if (!mDragging || !mActiveAxis)
        return;

    if (mOperation == GO_TRANSLATE)
    {
        Ray localRay = ray;
        if (mDragUsePlane)
        {
            Plane plane(mDragPlaneNormalLocal, Vector3::ZERO);
            auto r = localRay.intersects(plane);
            if (!r.first)
                return;
            Vector3 hitLocal = localRay.getPoint(r.second);
            Vector3 deltaLocal = hitLocal - mDragStartHitLocal;

            if (!(mActiveAxis & AXIS_X))
                deltaLocal.x = 0;
            if (!(mActiveAxis & AXIS_Y))
                deltaLocal.y = 0;
            if (!(mActiveAxis & AXIS_Z))
                deltaLocal.z = 0;

            Vector3 deltaWorld = mGizmoNode->_getDerivedOrientation() * deltaLocal;
            mParentNode->setPosition(mInitialObjectPos + deltaWorld);
        }
        else
        {
            Vector3 hitLocal = computePlaneHit(localRay, mDragAxisLocal, cameraDir, Vector3::ZERO);
            Real delta = (hitLocal - mDragStartHitLocal).dotProduct(mDragAxisLocal);
            mParentNode->setPosition(mInitialObjectPos + mDragAxisWorld * delta);
        }

        return;
    }

    if (mOperation == GO_ROTATE)
    {
        const Quaternion invOri = mDragGizmoOrientation.Inverse();
        Ray localRay(invOri * (ray.getOrigin() - mDragGizmoCenter), invOri * ray.getDirection());

        Plane plane(mDragAxisLocal, Vector3::ZERO);
        auto r = localRay.intersects(plane);
        if (!r.first)
            return;

        Vector3 hitLocal = localRay.getPoint(r.second);
        const Real radius = mDragStartHitLocal.length();
        if (radius > 1e-4f && hitLocal.squaredLength() > 1e-8f)
            hitLocal = hitLocal.normalisedCopy() * radius;

        Vector3 v0 = mDragLastHitLocal;
        Vector3 v1 = hitLocal;
        v0 -= mDragAxisLocal * v0.dotProduct(mDragAxisLocal);
        v1 -= mDragAxisLocal * v1.dotProduct(mDragAxisLocal);
        if (v0.isZeroLength() || v1.isZeroLength())
            return;
        v0.normalise();
        v1.normalise();

        Real cosTheta = Math::Clamp(v0.dotProduct(v1), Real(-1.0), Real(1.0));
        Real sinTheta = v0.crossProduct(v1).dotProduct(mDragAxisLocal);

        mDragAccumulatedAngle += Math::ATan2(sinTheta, cosTheta);
        mDragLastHitLocal = hitLocal;

        // Rotate in LOCAL space
        mParentNode->setOrientation(mInitialObjectRot * Quaternion(mDragAccumulatedAngle, mDragAxisLocal));
        return;
    }

    if (mOperation == GO_SCALE)
    {
        const Quaternion invOri = mDragGizmoOrientation.Inverse();
        Ray localRay(invOri * (ray.getOrigin() - mDragGizmoCenter), invOri * ray.getDirection());
        Vector3 localCameraDir = invOri * cameraDir;
        if (mDragUsePlane)
        {
            Plane plane(mDragPlaneNormalLocal, Vector3::ZERO);
            auto r = localRay.intersects(plane);
            if (!r.first)
                return;
            Vector3 hitLocal = localRay.getPoint(r.second);
            Vector3 deltaLocal = hitLocal - mDragStartHitLocal;

            Vector3 newScale = mInitialObjectScale;
            if (mActiveAxis & AXIS_X)
                newScale.x *= std::max(Real(1.0) + (deltaLocal.x * Real(0.5)), Real(0.1));
            if (mActiveAxis & AXIS_Y)
                newScale.y *= std::max(Real(1.0) + (deltaLocal.y * Real(0.5)), Real(0.1));
            if (mActiveAxis & AXIS_Z)
                newScale.z *= std::max(Real(1.0) + (deltaLocal.z * Real(0.5)), Real(0.1));

            mParentNode->setScale(newScale);
        }
        else
        {
            Vector3 hitLocal = computePlaneHit(localRay, mDragAxisLocal, localCameraDir, Vector3::ZERO);
            Real startDist = mDragStartHitLocal.dotProduct(mDragAxisLocal);
            Real currDist = hitLocal.dotProduct(mDragAxisLocal);
            Real delta = currDist - startDist;
            Real scaleFactor = Real(1.0) + delta * Real(0.5);

            Vector3 newScale = mInitialObjectScale;
            if (mActiveAxis & AXIS_X)
                newScale.x *= std::max(scaleFactor, Real(0.1));
            if (mActiveAxis & AXIS_Y)
                newScale.y *= std::max(scaleFactor, Real(0.1));
            if (mActiveAxis & AXIS_Z)
                newScale.z *= std::max(scaleFactor, Real(0.1));

            mParentNode->setScale(newScale);
        }
    }
}

void Gizmo::stopDrag()
{
    mDragging = false;
    mDragAxisLocal = Vector3::ZERO;
    mDragAxisWorld = Vector3::ZERO;
    mDragPlaneNormalLocal = Vector3::ZERO;
    mDragUsePlane = false;
    mDragLastHitLocal = Vector3::ZERO;
    mDragAccumulatedAngle = Radian(0);
    mDragGizmoOrientation = Quaternion::IDENTITY;
    mDragGizmoCenter = Vector3::ZERO;
}

void Gizmo::scaleToParent()
{
    if (!mParentNode)
        return;

    AxisAlignedBox worldAABB;

    // Accumulate bounds of all attached movable objects
    for (auto* mo : mParentNode->getAttachedObjects())
    {
        worldAABB.merge(mo->getWorldBoundingBox(true));
    }

    // Compute size
    Vector3 size = worldAABB.getSize();
    Real maxExtent = std::max({size.x, size.y, size.z});

    // Avoid degenerate scale
    if (maxExtent < Real(1e-6))
        return;

    constexpr Real gizmoSize = 3.5f * 2.0f; // from mesh construction
    mGizmoNode->setScale(Vector3(maxExtent / gizmoSize));
}

bool Gizmo::pickAxis(Ray& ray)
{
    const Ray localRay = toLocalRay(ray);
    constexpr Real axisLen = 4.0f;
    constexpr Real r = 0.3f;  // axis radius
    constexpr Real s = 1.0f;  // plane size
    constexpr Real t = 0.25f; // plane/ring thickness
    constexpr Real R = 3.0f;  // ring radius

    struct PickVolume
    {
        PlaneBoundedVolume vol;
        AXIS axis;
        GizmoOperation mode;
    };

    const PickVolume volumes[] = {
        {makeBox({0, -r, -r}, {axisLen, r, r}), AXIS_X, GO_TRANSLATE},
        {makeBox({-r, 0, -r}, {r, axisLen, r}), AXIS_Y, GO_TRANSLATE},
        {makeBox({-r, -r, 0}, {r, r, axisLen}), AXIS_Z, GO_TRANSLATE},
        {makeBox({0, 0, -t}, {s, s, t}), AXIS_XY, GO_TRANSLATE},
        {makeBox({-t, 0, 0}, {t, s, s}), AXIS_YZ, GO_TRANSLATE},
        {makeBox({0, -t, 0}, {s, t, s}), AXIS_XZ, GO_TRANSLATE},
        {makeBox({-t, -(R + t), -(R + t)}, {t, R + t, R + t}), AXIS_X, GO_ROTATE},
        {makeBox({-(R + t), -t, -(R + t)}, {R + t, t, R + t}), AXIS_Y, GO_ROTATE},
        {makeBox({-(R + t), -(R + t), -t}, {R + t, R + t, t}), AXIS_Z, GO_ROTATE},
    };

    Real bestT = Math::POS_INFINITY;
    AXIS bestAxis = AXIS_NONE;

    for (const auto& v : volumes)
    {
        if (mOperation == GO_ROTATE)
        {
            if (v.mode != GO_ROTATE)
                continue;
        }
        else if (mOperation == GO_TRANSLATE || mOperation == GO_SCALE)
        {
            if (v.mode == GO_ROTATE)
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

void Gizmo::highlightAxis(int axis)
{
    if (axis == mOldGizmoAxis)
        return;
    mOldGizmoAxis = axis;

    const int totalSubMeshes = PLANE_BASE + 3;

    for (int i = 0; i < totalSubMeshes; ++i)
        mGizmoEntity->getSubEntity(i)->setMaterialName("Ogre/AxisGizmo");

    if (axis == AXIS_NONE)
        return;

    if (axis & AXIS_X)
        highlightAxisSubmeshes(mGizmoEntity, 0);
    if (axis & AXIS_Y)
        highlightAxisSubmeshes(mGizmoEntity, 1);
    if (axis & AXIS_Z)
        highlightAxisSubmeshes(mGizmoEntity, 2);

    if (axis == AXIS_XY)
        mGizmoEntity->getSubEntity(PLANE_BASE + 0)->setMaterialName("Ogre/AxisGizmo_Highlight");
    else if (axis == AXIS_YZ)
        mGizmoEntity->getSubEntity(PLANE_BASE + 1)->setMaterialName("Ogre/AxisGizmo_Highlight");
    else if (axis == AXIS_XZ)
        mGizmoEntity->getSubEntity(PLANE_BASE + 2)->setMaterialName("Ogre/AxisGizmo_Highlight");
}

Ray Gizmo::toLocalRay(const Ray& worldRay) const
{
    Matrix4 inv = mGizmoNode->_getFullTransform().inverse();

    Vector3 localOrigin = inv * worldRay.getOrigin();
    Vector3 localDir = inv.linear().inverse().transpose() * worldRay.getDirection();

    localDir.normalise();

    return Ray(localOrigin, localDir);
}

CameraGizmo::CameraGizmo(Viewport* viewport)
{
    mViewport = viewport;
    mCamera = mViewport->getCamera();
    OgreAssert(mCamera, "CameraGizmo requires a camera in the viewport");

    auto* cameraNode = mCamera->getParentSceneNode();
    mGizmoNode = cameraNode->createChildSceneNode();
    // No rotation with camera
    mGizmoNode->setInheritOrientation(false);
    mGizmoNode->setOrientation(Ogre::Quaternion::IDENTITY);
    mGizmoNode->setInheritScale(false);

    createMesh(cameraNode->getCreator(), "AxisGizmosMesh");

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
    mGizmoNode->setPosition(Vector3(ndcX * halfWidth, ndcY * halfHeight, -depth));
    mGizmoNode->setScale(Vector3(size));
}

bool CameraGizmo::mouseMoved(const MouseMotionEvent& evt)
{
    const float nx = static_cast<float>(evt.x) / static_cast<float>(mViewport->getActualWidth());
    const float ny = static_cast<float>(evt.y) / static_cast<float>(mViewport->getActualHeight());

    if (pickFace(nx, ny) != -1)
    {
        return true;
    }
    return InputListener::mouseMoved(evt);
}

bool CameraGizmo::mousePressed(const MouseButtonEvent& evt)
{
    if (evt.button == BUTTON_LEFT)
    {
        const float nx = static_cast<float>(evt.x) / static_cast<float>(mViewport->getActualWidth());
        const float ny = static_cast<float>(evt.y) / static_cast<float>(mViewport->getActualHeight());

        if (snapCamera(pickFace(nx, ny)))
        {
            highlightFace(-1);
            return true;
        }
    }
    return InputListener::mousePressed(evt);
}

int CameraGizmo::pickFace(float nx, float ny)
{
    const Ray worldRay = mCamera->getCameraToViewportRay(nx, ny);
    Real closest = std::numeric_limits<Real>::max();

    int faceIndex = -1;
    for (int i = 0; i < 6; ++i)
    {
        const Matrix4 xf = mFaceNodes[i]->_getFullTransform();
        const Vector3 v0 = xf * Vector3(-0.5f, -0.5f, 0.0f);
        const Vector3 v1 = xf * Vector3(0.5f, -0.5f, 0.0f);
        const Vector3 v2 = xf * Vector3(0.5f, 0.5f, 0.0f);
        const Vector3 v3 = xf * Vector3(-0.5f, 0.5f, 0.0f);

        if(tryQuadHit(worldRay, v0, v1, v2, v3, closest))
            faceIndex = i;
    }

    highlightFace(faceIndex);
    return faceIndex;
}

bool CameraGizmo::snapCamera(int faceIndex) const
{
    using namespace Ogre;

    if (faceIndex == -1)
        return false;

    static const Vector3 directions[] = {
        Vector3::NEGATIVE_UNIT_X,
        Vector3::UNIT_X,
        Vector3::NEGATIVE_UNIT_Y,
        Vector3::UNIT_Y,
        Vector3::NEGATIVE_UNIT_Z,
        Vector3::UNIT_Z,
    };

    auto* cameraNode = mCamera->getParentSceneNode();
    Vector3 camDir = cameraNode->_getDerivedOrientation() * Vector3::NEGATIVE_UNIT_Z;
    Vector3 camPos = cameraNode->_getDerivedPosition();
    Real dist = -camDir.dotProduct(camPos);
    if (dist < 0.01f)
        dist = 1.0f;
    Vector3 target = camPos + camDir * dist;

    cameraNode->setPosition(target);
    cameraNode->setDirection(directions[faceIndex], Node::TS_WORLD);
    cameraNode->translate(Vector3(0, 0, dist), Node::TS_LOCAL);
    return true;
}

void CameraGizmo::highlightFace(int faceIndex)
{
    static const ColourValue base[] = {
        ColourValue(0.1f, 0.1f, 0.8f),
        ColourValue(0.1f, 0.8f, 0.2f),
        ColourValue(0.8f, 0.1f, 0.1f),
    };
    static const ColourValue hl[] = {
        ColourValue(0.0f, 0.0f, 1.0f),
        ColourValue(0.0f, 1.0f, 0.0f),
        ColourValue(1.0f, 0.0f, 0.0f),
    };

    if (faceIndex == mOldFaceIndex)
        return;
    mOldFaceIndex = faceIndex;

    for (int i = 0; i < 6; ++i)
    {
        updateCameraQuad(mGizmoObjects[i], (i == faceIndex) ? hl[i/2] : base[i/2]);
    }
}

void CameraGizmo::createMesh(SceneManager* manager, String name)
{
    using namespace Ogre;

    struct FaceData
    {
        const char* suffix;
        ColourValue color;
        Vector3 position;
        Quaternion orientation;
    };

    const FaceData faces[] = {
        {"_PX", ColourValue(0.2f, 0.2f, 0.9f, 1.0f), Vector3(0.5f, 0, 0), Quaternion(Degree(90), Vector3::UNIT_Y)},
        {"_NX", ColourValue(0.2f, 0.2f, 0.9f, 1.0f), Vector3(-0.5f, 0, 0), Quaternion(Degree(-90), Vector3::UNIT_Y)},
        {"_PY", ColourValue(0.2f, 0.9f, 0.2f, 1.0f), Vector3(0, 0.5f, 0), Quaternion(Degree(-90), Vector3::UNIT_X)},
        {"_NY", ColourValue(0.2f, 0.9f, 0.2f, 1.0f), Vector3(0, -0.5f, 0), Quaternion(Degree(90), Vector3::UNIT_X)},
        {"_PZ", ColourValue(0.9f, 0.2f, 0.2f, 1.0f), Vector3(0, 0, 0.5f), Quaternion::IDENTITY},
        {"_NZ", ColourValue(0.9f, 0.2f, 0.2f, 1.0f), Vector3(0, 0, -0.5f), Quaternion(Degree(180), Vector3::UNIT_Y)},
    };

    for (int i = 0; i < 6; ++i)
    {
        mFaceNodes[i] = createCameraFace(mGizmoNode, manager, name + faces[i].suffix, faces[i].color, mGizmoObjects[i]);
        mFaceNodes[i]->setPosition(faces[i].position);
        mFaceNodes[i]->setOrientation(faces[i].orientation);
    }
}

} // namespace OgreBites
