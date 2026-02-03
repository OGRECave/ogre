// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.

#ifndef OGREMAIN_INCLUDE_OGREDEFAULTDEBUGDRAWER_H_
#define OGREMAIN_INCLUDE_OGREDEFAULTDEBUGDRAWER_H_

#include "OgreSceneManager.h"

namespace Ogre
{

class _OgreExport DefaultDebugDrawer : public DebugDrawer
{
    SceneNode* mCameraNode;
    ManualObject mLines;
    ManualObject mAxes;
    int mDrawType;
    bool mStatic;
    float mBoneAxesSize;
    void preFindVisibleObjects(SceneManager* source, SceneManager::IlluminationRenderStage irs, Viewport* v) override;
    void postFindVisibleObjects(SceneManager* source, SceneManager::IlluminationRenderStage irs, Viewport* v) override;
    void beginLines();
    void beginAxes();
    void drawAxis2D(const Affine3& pose, const Matrix3& rot, float scale, const ColourValue& col);
public:
    enum DrawType
    {
        DT_AXES = 1 << 0,
        DT_WIREBOX = 1 << 1
    };

    DefaultDebugDrawer();

    /// if static, the drawer contents are preserved across frames. They are cleared otherwise.
    void setStatic(bool enable) { mStatic = enable; }

    void drawBone(const Node* node, const Affine3 & transform = Affine3::IDENTITY) override;
    void drawSceneNode(const SceneNode* node) override;
    void drawFrustum(const Frustum* frust) override;
    /// Allows the rendering of a wireframe bounding box.
    void drawWireBox(const AxisAlignedBox& aabb, const ColourValue& colour = ColourValue::White);
    void drawSphere(const Sphere & sphere) override;
    /// draw coordinate axes
    void drawAxes(const Affine3& pose, float size = 1.0f);
    /// Specifes the size of the axes drawn by drawBone()
    void setBoneAxesSize(float size);
};

} /* namespace Ogre */

#endif /* OGREMAIN_INCLUDE_OGREDEFAULTDEBUGDRAWER_H_ */
