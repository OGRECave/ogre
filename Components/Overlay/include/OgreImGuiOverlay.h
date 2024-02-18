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
/** Draws a renderer configuration menu using ImGui
  *
  * Wrap this in a ImGui::Begin() and ImGui::End() call to create the dialog.
  * @param renderSystemName The name of the render system that the user selected
  */
_OgreOverlayExport void DrawRenderingSettings(String& renderSystemName);

///Ogre's integrated support for [Dear ImGui](https://github.com/ocornut/imgui)
class _OgreOverlayExport ImGuiOverlay : public Overlay
{
public:
    ImGuiOverlay();
    ~ImGuiOverlay();

    /// add font from ogre .fontdef file.
    /// Must be called before first show()
    /// for the new font to be used.
    ImFont* addFont(const String& name, const String& group OGRE_RESOURCE_GROUP_INIT);

    /// generally should be called before every frame,
    /// before you render things with ImGui
    /// for example, at the start of a RenderTargetListener::preViewportUpdate
    ///
    /// Must be called before the first frame
    /// with a visible ImGuiOverlay is rendered
    /// or else ImgGUI will SegFault.
    ///
    /// Must be called before many ImgGUI functions
    /// or else they will SegFault.
    static void NewFrame();

    void _findVisibleObjects(Camera* cam, RenderQueue* queue, Viewport* vp) override;

private:
    void initialise() override;

    typedef std::vector<ImWchar> CodePointRange;
    std::vector<CodePointRange> mCodePointRanges;

    class ImGUIRenderable : public Ogre::Renderable
    {
    public:
        ImGUIRenderable();
        ~ImGUIRenderable();

        void initialise();

        void updateVertexData(ImDrawData* draw_data);

        bool preRender(SceneManager* sm, RenderSystem* rsys) override;

        void getWorldTransforms(Matrix4* xform) const override { *xform = mXform; }
        void getRenderOperation(RenderOperation& op) override { op = mRenderOp; }

        const LightList& getLights(void) const override;

        void createMaterial();
        void createFontTexture();

        const MaterialPtr& getMaterial() const override { return mMaterial; }

        Real getSquaredViewDepth(const Camera*) const override { return 0; }

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
