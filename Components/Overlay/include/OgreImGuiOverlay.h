// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.

#ifndef COMPONENTS_OVERLAY_INCLUDE_OGREIMGUIOVERLAY_H_
#define COMPONENTS_OVERLAY_INCLUDE_OGREIMGUIOVERLAY_H_

#include "OgreOverlay.h"
#include "OgreOverlayPrerequisites.h"
#include <OgreResourceGroupManager.h>

#include <imgui.h>

namespace Ogre
{
class _OgreOverlayExport ImGuiOverlay : public Overlay
{
public:
    ImGuiOverlay();
    ~ImGuiOverlay();

    /// add font from ogre .fontdef file
    /// must be called before first show()
    ImFont* addFont(const String& name, const String& group OGRE_RESOURCE_GROUP_INIT);

    static void NewFrame();

    void _findVisibleObjects(Camera* cam, RenderQueue* queue, Viewport* vp);

private:
    void initialise();

    typedef std::vector<ImWchar> CodePointRange;
    std::vector<CodePointRange> mCodePointRanges;

    class ImGUIRenderable : public Ogre::Renderable
    {
    public:
        ImGUIRenderable();
        ~ImGUIRenderable();

        void initialise();

        void updateVertexData(ImDrawData* draw_data);

        bool preRender(SceneManager* sm, RenderSystem* rsys);

        virtual void getWorldTransforms(Matrix4* xform) const { *xform = mXform; }
        virtual void getRenderOperation(RenderOperation& op) { op = mRenderOp; }

        const LightList& getLights(void) const;

        void createMaterial();
        void createFontTexture();

        const MaterialPtr& getMaterial() const { return mMaterial; }

        Real getSquaredViewDepth(const Camera*) const { return 0; }

        void _update();

        Matrix4 mXform;
        RenderOperation mRenderOp;
        TexturePtr mFontTex;
        MaterialPtr mMaterial;
    };

    ImGUIRenderable mRenderable;
};
} // namespace Ogre

#endif /* COMPONENTS_OVERLAY_INCLUDE_OGREIMGUIOVERLAY_H_ */
