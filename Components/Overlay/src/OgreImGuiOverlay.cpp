// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.

#include <imgui.h>
#include <imgui_freetype.h>

#include <OgreImGuiOverlay.h>
#include <OgreHardwareBufferManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreRenderSystem.h>
#include <OgreTextureManager.h>
#include <OgreMaterialManager.h>
#include <OgreOverlayManager.h>
#include <OgreFontManager.h>
#include <OgreTechnique.h>
#include <OgreTextureUnitState.h>
#include <OgreFont.h>
#include <OgreRenderQueue.h>
#include <OgreFrameListener.h>
#include <OgreRoot.h>

namespace Ogre
{

ImGuiOverlay::ImGuiOverlay() : Overlay("ImGuiOverlay")
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.BackendPlatformName = "OGRE";
}
ImGuiOverlay::~ImGuiOverlay()
{
    ImGui::DestroyContext();
}

void ImGuiOverlay::initialise()
{
    if (!mInitialised)
    {
        mRenderable.initialise();
        mCodePointRanges.clear();
    }
    mInitialised = true;
}

//-----------------------------------------------------------------------------------
void ImGuiOverlay::_findVisibleObjects(Camera* cam, RenderQueue* queue, Viewport* vp)
{
    if (!mVisible)
        return;

    mRenderable._update();
    queue->addRenderable(&mRenderable, RENDER_QUEUE_OVERLAY, mZOrder * 100);
}
//-----------------------------------------------------------------------------------
void ImGuiOverlay::ImGUIRenderable::createMaterial()
{
    mMaterial = MaterialManager::getSingleton().create("ImGui/material", RGN_INTERNAL);
    Pass* mPass = mMaterial->getTechnique(0)->getPass(0);
    mPass->setCullingMode(CULL_NONE);
    mPass->setVertexColourTracking(TVC_DIFFUSE);
    mPass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
    mPass->setSeparateSceneBlendingOperation(SBO_ADD, SBO_ADD);
    mPass->setSeparateSceneBlending(SBF_SOURCE_ALPHA, SBF_ONE_MINUS_SOURCE_ALPHA,
                                    SBF_ONE_MINUS_SOURCE_ALPHA, SBF_ZERO);

    TextureUnitState* mTexUnit = mPass->createTextureUnitState();
    mTexUnit->setTexture(mFontTex);
    mTexUnit->setTextureFiltering(TFO_NONE);

    mMaterial->load();
    mMaterial->setLightingEnabled(false);
    mMaterial->setDepthCheckEnabled(false);
}

ImFont* ImGuiOverlay::addFont(const String& name, const String& group)
{
    FontPtr font = FontManager::getSingleton().getByName(name, group);
    OgreAssert(font, "font does not exist");
    OgreAssert(font->getType() == FT_TRUETYPE, "font must be of FT_TRUETYPE");
    DataStreamPtr dataStreamPtr =
        ResourceGroupManager::getSingleton().openResource(font->getSource(), font->getGroup());
    MemoryDataStream ttfchunk(dataStreamPtr, false); // transfer ownership to imgui

    // convert codepoint ranges for imgui
    CodePointRange cprange;
    for (const auto& r : font->getCodePointRangeList())
    {
        cprange.push_back(r.first);
        cprange.push_back(r.second);
    }

    ImGuiIO& io = ImGui::GetIO();
    const ImWchar* cprangePtr = io.Fonts->GetGlyphRangesDefault();
    if (!cprange.empty())
    {
        cprange.push_back(0); // terminate
        mCodePointRanges.push_back(cprange);
        // ptr must persist until createFontTexture
        cprangePtr = mCodePointRanges.back().data();
    }

    ImFontConfig cfg;
    strncpy(cfg.Name, name.c_str(), 40);
    return io.Fonts->AddFontFromMemoryTTF(ttfchunk.getPtr(), ttfchunk.size(), font->getTrueTypeSize(), &cfg,
                                          cprangePtr);
}

void ImGuiOverlay::ImGUIRenderable::createFontTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    if (io.Fonts->Fonts.empty())
        io.Fonts->AddFontDefault();
    ImGuiFreeType::BuildFontAtlas(io.Fonts, 0);

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    mFontTex = TextureManager::getSingleton().createManual("ImGui/FontTex", RGN_INTERNAL, TEX_TYPE_2D,
                                                           width, height, 1, 1, PF_BYTE_RGBA);

    mFontTex->getBuffer()->blitFromMemory(PixelBox(Box(0, 0, width, height), PF_BYTE_RGBA, pixels));
}
void ImGuiOverlay::NewFrame(const FrameEvent& evt)
{
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = std::max<float>(
        evt.timeSinceLastFrame,
        1e-4f); // see https://github.com/ocornut/imgui/commit/3c07ec6a6126fb6b98523a9685d1f0f78ca3c40c

    // Read keyboard modifiers inputs
    io.KeyAlt = false;
    io.KeySuper = false;

    OverlayManager& oMgr = OverlayManager::getSingleton();

    // Setup display size (every frame to accommodate for window resizing)
    io.DisplaySize = ImVec2(oMgr.getViewportWidth(), oMgr.getViewportHeight());

    // Start the frame
    ImGui::NewFrame();
}

void ImGuiOverlay::ImGUIRenderable::_update()
{
    if (mMaterial->getSupportedTechniques().empty())
    {
        mMaterial->load(); // Support for adding lights run time
    }

    RenderSystem* rSys = Root::getSingleton().getRenderSystem();
    OverlayManager& oMgr = OverlayManager::getSingleton();

    // Construct projection matrix, taking texel offset corrections in account (important for DirectX9)
    // See also:
    //     - OGRE-API specific hint: http://www.ogre3d.org/forums/viewtopic.php?f=5&p=536881#p536881
    //     - IMGUI Dx9 demo solution:
    //     https://github.com/ocornut/imgui/blob/master/examples/directx9_example/imgui_impl_dx9.cpp#L127-L138
    float texelOffsetX = rSys->getHorizontalTexelOffset();
    float texelOffsetY = rSys->getVerticalTexelOffset();
    float L = texelOffsetX;
    float R = oMgr.getViewportWidth() + texelOffsetX;
    float T = texelOffsetY;
    float B = oMgr.getViewportHeight() + texelOffsetY;

    mXform = Matrix4(2.0f / (R - L), 0.0f, 0.0f, (L + R) / (L - R), 0.0f, -2.0f / (B - T), 0.0f,
                     (T + B) / (B - T), 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}

bool ImGuiOverlay::ImGUIRenderable::preRender(SceneManager* sm, RenderSystem* rsys)
{
    Viewport* vp = rsys->_getViewport();

    // Instruct ImGui to Render() and process the resulting CmdList-s
    // Adopted from https://bitbucket.org/ChaosCreator/imgui-ogre2.1-binding
    // ... Commentary on OGRE forums: http://www.ogre3d.org/forums/viewtopic.php?f=5&t=89081#p531059
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    int vpWidth = vp->getActualWidth();
    int vpHeight = vp->getActualHeight();

    TextureUnitState* tu = mMaterial->getBestTechnique()->getPass(0)->getTextureUnitState(0);

    for (int i = 0; i < draw_data->CmdListsCount; ++i)
    {
        const ImDrawList* draw_list = draw_data->CmdLists[i];
        updateVertexData(draw_list->VtxBuffer, draw_list->IdxBuffer);

        unsigned int startIdx = 0;

        for (int j = 0; j < draw_list->CmdBuffer.Size; ++j)
        {
            // Create a renderable and fill it's buffers
            const ImDrawCmd* drawCmd = &draw_list->CmdBuffer[j];

            // Set scissoring
            int scLeft = static_cast<int>(drawCmd->ClipRect.x); // Obtain bounds
            int scTop = static_cast<int>(drawCmd->ClipRect.y);
            int scRight = static_cast<int>(drawCmd->ClipRect.z);
            int scBottom = static_cast<int>(drawCmd->ClipRect.w);

            // Clamp bounds to viewport dimensions
            scLeft = Math::Clamp(scLeft, 0, vpWidth);
            scRight = Math::Clamp(scRight, 0, vpWidth);
            scTop = Math::Clamp(scTop, 0, vpHeight);
            scBottom = Math::Clamp(scBottom, 0, vpHeight);

            if (drawCmd->TextureId)
            {
                auto handle = (ResourceHandle)drawCmd->TextureId;
                auto tex = static_pointer_cast<Texture>(TextureManager::getSingleton().getByHandle(handle));
                if (tex)
                {
                    rsys->_setTexture(0, true, tex);
                    rsys->_setSampler(0, *TextureManager::getSingleton().getDefaultSampler());
                }
            }

            rsys->setScissorTest(true, scLeft, scTop, scRight, scBottom);

            // Render!
            mRenderOp.indexData->indexStart = startIdx;
            mRenderOp.indexData->indexCount = drawCmd->ElemCount;

            rsys->_render(mRenderOp);

            if (drawCmd->TextureId)
            {
                // reset to pass state
                rsys->_setTexture(0, true, mFontTex);
                rsys->_setSampler(0, *tu->getSampler());
            }

            // Update counts
            startIdx += drawCmd->ElemCount;
        }
    }
    rsys->setScissorTest(false);
    return false;
}

const LightList& ImGuiOverlay::ImGUIRenderable::getLights() const
{
    // Overlayelements should not be lit by the scene, this will not get called
    static LightList ll;
    return ll;
}

ImGuiOverlay::ImGUIRenderable::ImGUIRenderable()
{
    // default overlays to preserve their own detail level
    mPolygonModeOverrideable = false;

    // use identity projection and view matrices
    mUseIdentityProjection = true;
    mUseIdentityView = true;

    mConvertToBGR = false;
}
//-----------------------------------------------------------------------------------
void ImGuiOverlay::ImGUIRenderable::initialise(void)
{
    createFontTexture();
    createMaterial();

    mRenderOp.vertexData = OGRE_NEW VertexData();
    mRenderOp.indexData = OGRE_NEW IndexData();

    mRenderOp.vertexData->vertexCount = 0;
    mRenderOp.vertexData->vertexStart = 0;

    mRenderOp.indexData->indexCount = 0;
    mRenderOp.indexData->indexStart = 0;
    mRenderOp.operationType = RenderOperation::OT_TRIANGLE_LIST;
    mRenderOp.useIndexes = true;
    mRenderOp.useGlobalInstancingVertexBufferIsAvailable = false;

    VertexDeclaration* decl = mRenderOp.vertexData->vertexDeclaration;

    // vertex declaration
    size_t offset = 0;
    decl->addElement(0, offset, VET_FLOAT2, VES_POSITION);
    offset += VertexElement::getTypeSize(VET_FLOAT2);
    decl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
    offset += VertexElement::getTypeSize(VET_FLOAT2);
    decl->addElement(0, offset, VET_COLOUR, VES_DIFFUSE);

    if (Root::getSingleton().getRenderSystem()->getName().find("Direct3D9") != String::npos)
        mConvertToBGR = true;
}
//-----------------------------------------------------------------------------------
ImGuiOverlay::ImGUIRenderable::~ImGUIRenderable()
{
    OGRE_DELETE mRenderOp.vertexData;
    OGRE_DELETE mRenderOp.indexData;
}
//-----------------------------------------------------------------------------------
void ImGuiOverlay::ImGUIRenderable::updateVertexData(const ImVector<ImDrawVert>& vtxBuf,
                                                     const ImVector<ImDrawIdx>& idxBuf)
{
    VertexBufferBinding* bind = mRenderOp.vertexData->vertexBufferBinding;

    if (bind->getBindings().empty() || bind->getBuffer(0)->getNumVertices() != size_t(vtxBuf.size()))
    {
        bind->setBinding(0, HardwareBufferManager::getSingleton().createVertexBuffer(
                                sizeof(ImDrawVert), vtxBuf.size(), HardwareBuffer::HBU_WRITE_ONLY));
    }
    if (!mRenderOp.indexData->indexBuffer ||
        mRenderOp.indexData->indexBuffer->getNumIndexes() != size_t(idxBuf.size()))
    {
        mRenderOp.indexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
            HardwareIndexBuffer::IT_16BIT, idxBuf.size(), HardwareBuffer::HBU_WRITE_ONLY);
    }

    if (mConvertToBGR)
    {
        // convert RGBA > BGRA
        PixelBox src(1, vtxBuf.size(), 1, PF_A8B8G8R8, (char*)vtxBuf.Data + offsetof(ImDrawVert, col));
        src.rowPitch = sizeof(ImDrawVert) / sizeof(ImU32);
        PixelBox dst = src;
        dst.format = PF_A8R8G8B8;
        PixelUtil::bulkPixelConversion(src, dst);
    }

    // Copy all vertices
    bind->getBuffer(0)->writeData(0, vtxBuf.size_in_bytes(), vtxBuf.Data, true);
    mRenderOp.indexData->indexBuffer->writeData(0, idxBuf.size_in_bytes(), idxBuf.Data, true);

    mRenderOp.vertexData->vertexStart = 0;
    mRenderOp.vertexData->vertexCount = vtxBuf.size();
}
} // namespace Ogre
