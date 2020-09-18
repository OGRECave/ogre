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
    ManualObject mLines;
    ManualObject mAxes;
    int mDrawType;
    bool mStatic;
    void preFindVisibleObjects(SceneManager* source, SceneManager::IlluminationRenderStage irs, Viewport* v);
    void postFindVisibleObjects(SceneManager* source, SceneManager::IlluminationRenderStage irs, Viewport* v);
    void beginLines();
public:
    enum DrawType
    {
        DT_AXES = 1 << 0,
        DT_WIREBOX = 1 << 1
    };

    DefaultDebugDrawer();

    /// if static, the drawer contents are preserved across frames. They are cleared otherwise.
    void setStatic(bool enable) { mStatic = enable; }

    void drawBone(const Node* node);
    void drawSceneNode(const SceneNode* node);
    void drawFrustum(const Frustum* frust);
    /// Allows the rendering of a wireframe bounding box.
    void drawWireBox(const AxisAlignedBox& aabb, const ColourValue& colour = ColourValue::White);
    /// draw coordinate axes
    void drawAxes(const Affine3& pose, float size = 1.0f);
};

} /* namespace Ogre */

#endif /* OGREMAIN_INCLUDE_OGREDEFAULTDEBUGDRAWER_H_ */
