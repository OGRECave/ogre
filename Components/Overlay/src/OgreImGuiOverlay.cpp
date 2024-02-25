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
#include <OgreTimer.h>

namespace Ogre
{

static void DrawConfigOption(RenderSystem* rs, const ConfigOption& opt)
{
    if(ImGui::BeginCombo(opt.name.c_str(), opt.currentValue.c_str()))
    {
        for(const auto& it : opt.possibleValues)
        {
            if(ImGui::Selectable(it.c_str(), it == opt.currentValue))
                rs->setConfigOption(opt.name, it);
        }
        ImGui::EndCombo();
    }
}

void DrawRenderingSettings(String& rsName)
{
    auto root = Root::getSingletonPtr();
    OgreAssert(root, "Root must be created");

    if(rsName.empty())
        rsName = root->getRenderSystem()->getName();

    if(ImGui::BeginCombo("Render System", rsName.c_str()))
    {
        for(const auto& it : root->getAvailableRenderers())
        {
            if(ImGui::Selectable(it->getName().c_str(), it->getName() == rsName))
                rsName = it->getName();
        }
        ImGui::EndCombo();
    }

    ImGui::SeparatorText("Options");

    auto rs = root->getRenderSystemByName(rsName);
    for(const auto& it : rs->getConfigOptions())
        DrawConfigOption(rs, it.second);
}

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
    mMaterial->setReceiveShadows(false);
}

ImFont* ImGuiOverlay::addFont(const String& name, const String& group)
{
    FontPtr font = FontManager::getSingleton().getByName(name, group);
    if (!font)
        OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
                    StringUtil::format("Font '%s' not found in group '%s'", name.c_str(), group.c_str()));

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
    if (cprange.empty())
        cprange = {32, 126}; // ogre default

    cprange.push_back(0); // terminate
    mCodePointRanges.push_back(cprange);
    // ptr must persist until createFontTexture
    cprangePtr = mCodePointRanges.back().data();

    float vpScale = OverlayManager::getSingleton().getPixelRatio();

    ImFontConfig cfg;
    strncpy(cfg.Name, name.c_str(), IM_ARRAYSIZE(cfg.Name) - 1);
    return io.Fonts->AddFontFromMemoryTTF(ttfchunk.getPtr(), ttfchunk.size(), font->getTrueTypeSize() * vpScale, &cfg,
                                          cprangePtr);
}

void ImGuiOverlay::ImGUIRenderable::createFontTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    if (io.Fonts->Fonts.empty())
        io.Fonts->AddFontDefault();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    mFontTex = TextureManager::getSingleton().createManual("ImGui/FontTex", RGN_INTERNAL, TEX_TYPE_2D,
                                                           width, height, 1, 1, PF_BYTE_RGBA);

    mFontTex->getBuffer()->blitFromMemory(PixelBox(Box(0, 0, width, height), PF_BYTE_RGBA, pixels));
}
void ImGuiOverlay::NewFrame()
{
    static auto lastTime = Root::getSingleton().getTimer()->getMilliseconds();
    auto now = Root::getSingleton().getTimer()->getMilliseconds();

    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = std::max<float>(
        float(now - lastTime)/1000,
        1e-4f); // see https://github.com/ocornut/imgui/commit/3c07ec6a6126fb6b98523a9685d1f0f78ca3c40c

    lastTime = now;

    // Read keyboard modifiers inputs
    io.KeyAlt = false;
    io.KeySuper = false;

    OverlayManager& oMgr = OverlayManager::getSingleton();

    // Setup display size (every frame to accommodate for window resizing)
    auto vpScale = oMgr.getPixelRatio();
    io.DisplaySize = ImVec2(oMgr.getViewportWidth() * vpScale, oMgr.getViewportHeight() * vpScale);

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

    // Construct projection matrix, taking texel offset corrections in account (important for DirectX9)
    // See also:
    //     - OGRE-API specific hint: http://www.ogre3d.org/forums/viewtopic.php?f=5&p=536881#p536881
    //     - IMGUI Dx9 demo solution:
    //     https://github.com/ocornut/imgui/blob/v1.50/examples/directx9_example/imgui_impl_dx9.cpp#L127-L138
    ImGuiIO& io = ImGui::GetIO();
    float texelOffsetX = rSys->getHorizontalTexelOffset();
    float texelOffsetY = rSys->getVerticalTexelOffset();
    float L = texelOffsetX;
    float R = io.DisplaySize.x + texelOffsetX;
    float T = texelOffsetY;
    float B = io.DisplaySize.y + texelOffsetY;

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

    updateVertexData(draw_data);

    mRenderOp.indexData->indexStart = 0;
    mRenderOp.vertexData->vertexStart = 0;

    for (int i = 0; i < draw_data->CmdListsCount; ++i)
    {
        const ImDrawList* draw_list = draw_data->CmdLists[i];
        mRenderOp.vertexData->vertexCount = draw_list->VtxBuffer.size();

        for (int j = 0; j < draw_list->CmdBuffer.Size; ++j)
        {
            // Create a renderable and fill it's buffers
            const ImDrawCmd* drawCmd = &draw_list->CmdBuffer[j];

            // Set scissoring
            Rect scissor(drawCmd->ClipRect.x, drawCmd->ClipRect.y, drawCmd->ClipRect.z,
                          drawCmd->ClipRect.w);

            // Clamp bounds to viewport dimensions
            scissor = scissor.intersect(Rect(0, 0, vpWidth, vpHeight));

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

            rsys->setScissorTest(true, scissor);

            // Render!
            mRenderOp.indexData->indexCount = drawCmd->ElemCount;

            rsys->_render(mRenderOp);

            if (drawCmd->TextureId)
            {
                // reset to pass state
                rsys->_setTexture(0, true, mFontTex);
                rsys->_setSampler(0, *tu->getSampler());
            }

            // Update counts
            mRenderOp.indexData->indexStart += drawCmd->ElemCount;
        }
        mRenderOp.vertexData->vertexStart += draw_list->VtxBuffer.size();
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
    mRenderOp.useGlobalInstancing = false;

    VertexDeclaration* decl = mRenderOp.vertexData->vertexDeclaration;

    // vertex declaration
    size_t offset = 0;
    decl->addElement(0, offset, VET_FLOAT2, VES_POSITION);
    offset += VertexElement::getTypeSize(VET_FLOAT2);
    decl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
    offset += VertexElement::getTypeSize(VET_FLOAT2);
    decl->addElement(0, offset, VET_UBYTE4_NORM, VES_DIFFUSE);
}
//-----------------------------------------------------------------------------------
ImGuiOverlay::ImGUIRenderable::~ImGUIRenderable()
{
    if(mFontTex)
        TextureManager::getSingleton().remove(mFontTex);
    if(mMaterial)
        MaterialManager::getSingleton().remove(mMaterial);
    OGRE_DELETE mRenderOp.vertexData;
    OGRE_DELETE mRenderOp.indexData;
}
//-----------------------------------------------------------------------------------
void ImGuiOverlay::ImGUIRenderable::updateVertexData(ImDrawData* draw_data)
{
    if(!draw_data->TotalVtxCount)
        return;

    VertexBufferBinding* bind = mRenderOp.vertexData->vertexBufferBinding;

    if (bind->getBindings().empty() || bind->getBuffer(0)->getNumVertices() < size_t(draw_data->TotalVtxCount))
    {
        bind->setBinding(0, HardwareBufferManager::getSingleton().createVertexBuffer(
                                sizeof(ImDrawVert), draw_data->TotalVtxCount, HBU_CPU_TO_GPU));
    }
    if (!mRenderOp.indexData->indexBuffer ||
        mRenderOp.indexData->indexBuffer->getNumIndexes() < size_t(draw_data->TotalIdxCount))
    {
        mRenderOp.indexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
            HardwareIndexBuffer::IT_16BIT, draw_data->TotalIdxCount, HBU_CPU_TO_GPU);
    }

    // Copy all vertices
    size_t vtx_offset = 0;
    size_t idx_offset = 0;
    for (int i = 0; i < draw_data->CmdListsCount; ++i)
    {
        const ImDrawList* draw_list = draw_data->CmdLists[i];
        bind->getBuffer(0)->writeData(vtx_offset, draw_list->VtxBuffer.size_in_bytes(), draw_list->VtxBuffer.Data,
                                      i == 0); // discard on first write
        mRenderOp.indexData->indexBuffer->writeData(idx_offset, draw_list->IdxBuffer.size_in_bytes(),
                                                    draw_list->IdxBuffer.Data, i == 0);
        vtx_offset += draw_list->VtxBuffer.size_in_bytes();
        idx_offset += draw_list->IdxBuffer.size_in_bytes();
    }
}
} // namespace Ogre
