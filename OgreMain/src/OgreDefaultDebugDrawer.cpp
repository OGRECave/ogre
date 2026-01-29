// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.

#include "OgreMatrix3.h"
#include "OgreStableHeaders.h"
#include "OgreDefaultDebugDrawer.h"
#include "OgreTagPoint.h"
#include "OgreViewport.h"

namespace Ogre
{

DefaultDebugDrawer::DefaultDebugDrawer() : mCamera(nullptr), mLines(""), mAxes(""), mDrawType(0), mStatic(false), mBoneAxesSize(1.0f) {}

void DefaultDebugDrawer::preFindVisibleObjects(SceneManager* source,
                                               SceneManager::IlluminationRenderStage irs, Viewport* v)
{
    mDrawType = 0;

    if (source->getDisplaySceneNodes())
        mDrawType |= DT_AXES;
    if (source->getShowBoundingBoxes())
        mDrawType |= DT_WIREBOX;

    mCamera = v->getCamera();
}
void DefaultDebugDrawer::beginLines()
{
    if (mLines.getSections().empty())
    {
        const char* matName = "Ogre/Debug/LinesMat";
        auto mat = MaterialManager::getSingleton().getByName(matName, RGN_INTERNAL);
        if (!mat)
        {
            mat = MaterialManager::getSingleton().create(matName, RGN_INTERNAL);
            Pass* p = mat->getTechnique(0)->getPass(0);
            p->setLightingEnabled(false);
            p->setVertexColourTracking(TVC_AMBIENT);
        }
        mLines.setBufferUsage(HBU_CPU_TO_GPU);
        mLines.begin(mat, RenderOperation::OT_LINE_LIST);
    }
    else if (mLines.getCurrentVertexCount() == 0)
        mLines.beginUpdate(0);
}
void DefaultDebugDrawer::beginAxes()
{
    if (mAxes.getSections().empty())
    {
        const char* matName = "Ogre/Debug/AxesMat";
        auto mat = MaterialManager::getSingleton().getByName(matName, RGN_INTERNAL);
        if (!mat)
        {
            mat = MaterialManager::getSingleton().create(matName, RGN_INTERNAL);
            Pass* p = mat->getTechnique(0)->getPass(0);
            p->setLightingEnabled(false);
            p->setPolygonModeOverrideable(false);
            p->setVertexColourTracking(TVC_AMBIENT);
            p->setSceneBlending(SBT_TRANSPARENT_ALPHA);
            p->setCullingMode(CULL_NONE);
            p->setDepthWriteEnabled(false);
            p->setDepthCheckEnabled(false);
        }

        mAxes.setBufferUsage(HBU_CPU_TO_GPU);
        mAxes.begin(mat);
    }
    else if (mAxes.getCurrentVertexCount() == 0)
        mAxes.beginUpdate(0);
}
void DefaultDebugDrawer::drawWireBox(const AxisAlignedBox& aabb, const ColourValue& colour)
{
    beginLines();

    int base = mLines.getCurrentVertexCount();
    for (const auto& corner : aabb.getAllCorners())
    {
        mLines.position(corner);
        mLines.colour(colour);
    }
    // see AxisAlignedBox::getAllCorners
    int idx[] = {0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 6, 1, 5, 2, 4, 3, 7};
    for (int i : idx)
        mLines.index(base + i);
}
void DefaultDebugDrawer::drawFrustum(const Frustum* frust)
{
    beginLines();

    int base = mLines.getCurrentVertexCount();
    for (const auto& corner : frust->getWorldSpaceCorners())
    {
        mLines.position(corner);
        mLines.colour(frust->getDebugColour());
    }
    // see ConvexBody::define
    int idx[] = {0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 2, 6, 1, 5, 0, 4, 3, 7};
    for (int i : idx)
        mLines.index(base + i);
}
void DefaultDebugDrawer::drawBoundingSphere(const Sphere & sphere)
{
    if (!mCamera)
    {
        return;
    }

    const Vector3 & center = sphere.getCenter();
    float radius = sphere.getRadius();

    const Vector3 & camPos = mCamera->getParentSceneNode()->_getDerivedPosition();
    Vector3 axis = camPos - center;
    float distance = axis.normalise();

    // Unlike std's, Ogre's ACos tolerates domain errors
    const Radian facetAngle = 2 * Math::ACos(1.0f - (0.001f * distance / radius));
    int facetCount = (int)std::ceil(Math::TWO_PI / facetAngle.valueRadians());

    // Calculate an arbitrary radial orthogonal to the axis
    // TODO: replace this with something smarter
    Vector3 radial = radius * (axis + Vector3::UNIT_X).crossProduct(axis).normalisedCopy();

    beginLines();
    int base = mLines.getCurrentVertexCount();

    for (int i = 0; i < facetCount; ++i)
    {
        Radian angle(Math::TWO_PI * i / (float)facetCount);
        mLines.position(center + Quaternion(angle , axis) * radial);
        mLines.colour(ColourValue::White);
        mLines.index(base + i);
        mLines.index(base + ((i + 1) % facetCount));
    }
}
void DefaultDebugDrawer::drawAxis2D(const Affine3& pose, const Matrix3& rot, float scale, const ColourValue& col)
{
    /* each made up of 2 of these (base plane = XY)
     *   .------------|\
     *   '------------|/
     */
     static Vector3 basepos[7] =
     {
         // stalk
         Vector3(0, 0.05, 0),
         Vector3(0, -0.05, 0),
         Vector3(0.7, -0.05, 0),
         Vector3(0.7, 0.05, 0),
         // head
         Vector3(0.7, -0.15, 0),
         Vector3(1, 0, 0),
         Vector3(0.7, 0.15, 0)
     };

     uint32 base = mAxes.getCurrentVertexCount();
     // vertices
     for (const auto& p : basepos)
     {
         mAxes.position(pose * (rot * p * scale));
         mAxes.colour(col);
     }

     // indices
     mAxes.quad(base + 0, base + 1, base + 2, base + 3);
     mAxes.triangle(base + 4, base + 5, base + 6);
}
void DefaultDebugDrawer::drawAxes(const Affine3& pose, float size)
{
    beginAxes();

    ColourValue col[3] = {ColourValue(1, 0, 0, 0.8), ColourValue(0, 1, 0, 0.8), ColourValue(0, 0, 1, 0.8)};

    Matrix3 rot[6];

    // x-axis
    rot[0] = Matrix3::IDENTITY;
    rot[1].FromAxes(Vector3::UNIT_X, Vector3::NEGATIVE_UNIT_Z, Vector3::UNIT_Y);
    // y-axis
    rot[2].FromAxes(Vector3::UNIT_Y, Vector3::NEGATIVE_UNIT_X, Vector3::UNIT_Z);
    rot[3].FromAxes(Vector3::UNIT_Y, Vector3::UNIT_Z, Vector3::UNIT_X);
    // z-axis
    rot[4].FromAxes(Vector3::UNIT_Z, Vector3::UNIT_Y, Vector3::NEGATIVE_UNIT_X);
    rot[5].FromAxes(Vector3::UNIT_Z, Vector3::UNIT_X, Vector3::UNIT_Y);

    // 6 arrows
    for (size_t i = 0; i < 6; ++i)
    {
        drawAxis2D(pose, rot[i], size, col[i / 2]);
    }
}
void DefaultDebugDrawer::setBoneAxesSize(float size)
{
    mBoneAxesSize = size;
}
void DefaultDebugDrawer::drawBone(const Node* node, const Affine3& transform)
{
    beginAxes();

    ColourValue col(0.5, 0, 1, 0.9);
    Affine3 pose = transform * node->_getFullTransform();

    Matrix3 rot[2];
    // x-axis
    rot[0] = Matrix3::IDENTITY;
    rot[1].FromAxes(Vector3::UNIT_X, Vector3::NEGATIVE_UNIT_Z, Vector3::UNIT_Y);

    for(const auto* child : node->getChildren())
    {
        if(dynamic_cast<const TagPoint*>(child))
            continue;

        float size = child->getPosition().length(); // we can assume relative to parent position

        Matrix3 crot;
        Vector3::UNIT_X.getRotationTo(child->getPosition()).ToRotationMatrix(crot);

        for (const auto& r : rot)
        {
            drawAxis2D(pose, crot * r, size, col);
        }
    }

    if(node->getChildren().empty())
    {
        Matrix3 crot;
        Vector3::UNIT_X.getRotationTo(node->getPosition()).ToRotationMatrix(crot);
        // draw the axes of the node itself
        for (const auto& r : rot)
        {
            drawAxis2D(pose, crot * r, mBoneAxesSize, col);
        }
    }
}
void DefaultDebugDrawer::drawSceneNode(const SceneNode* node)
{
    // skip drawing root scene node as it contains the camera frustum
    if(!node->getParent())
        return;

    const auto& aabb = node->_getWorldAABB();
    //Skip all bounding boxes that are infinite.
    if (aabb.isInfinite()) {
        return;
    }
    if (node->getDisplaySceneNode() || (mDrawType & DT_AXES))
    {
        // remove scale here as it will be in full transform below too
        Vector3f hs(aabb.getHalfSize() / node->_getDerivedScale());
        float sz = std::min(hs[0], hs[1]);
        sz = std::min(sz, hs[2]);
        sz = std::max(sz, 1.0f);
        drawAxes(node->_getFullTransform(), sz);
    }

    if (node->getShowBoundingBox() || (mDrawType & DT_WIREBOX))
    {
        drawWireBox(aabb);
    }
}
void DefaultDebugDrawer::postFindVisibleObjects(SceneManager* source,
                                                SceneManager::IlluminationRenderStage irs, Viewport* v)
{
    auto queue = source->getRenderQueue();
    if (mLines.getCurrentVertexCount())
    {
        mLines.end();
        mLines._updateRenderQueue(queue);
    }
    if (mAxes.getCurrentVertexCount())
    {
        mAxes.end();
        mAxes._updateRenderQueue(queue);
    }

    if (mStatic)
    {
        mLines._updateRenderQueue(queue);
        mAxes._updateRenderQueue(queue);
    }
}

} /* namespace Ogre */
