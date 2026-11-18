// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include "RootWithoutRenderSystemFixture.h"
#include "OgreTextureManager.h"

using namespace Ogre;

namespace
{
class MockTexture : public Texture
{
public:
    explicit MockTexture(ResourceManager* creator)
        : Texture(creator, "MockTexture", 0, "General")
    {
    }

    static int mGenerations;
    bool mFail = false;
    int mAttempts = 0;

protected:
    void createInternalResourcesImpl() override {}
    void freeInternalResourcesImpl() override {}
    void loadImpl() override {}
    void generateMipmapsImpl() override
    {
        ++mAttempts;
        if (mFail)
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "generation failed");
        ++mGenerations;
    }
};

int MockTexture::mGenerations = 0;

class DeferredMipmapTests : public RootWithoutRenderSystemFixture
{
public:
    std::unique_ptr<DefaultTextureManager> mTexMgr;

    void SetUp() override
    {
        RootWithoutRenderSystemFixture::SetUp();
        mTexMgr = std::make_unique<DefaultTextureManager>();
        MockTexture::mGenerations = 0;
    }

    void TearDown() override
    {
        mTexMgr.reset();
        RootWithoutRenderSystemFixture::TearDown();
    }
};

MockTexture* createTexture(TextureManager* mgr, int usage = TU_AUTOMIPMAP, uint32 numMipmaps = 3)
{
    auto tex = new MockTexture(mgr);
    tex->setNumMipmaps(numMipmaps);
    tex->setUsage(usage);
    tex->createInternalResources();
    return tex;
}
} // namespace

TEST_F(DeferredMipmapTests, FlushEmptyIsNoOp)
{
    mTexMgr->_flushMipmaps();
    EXPECT_EQ(MockTexture::mGenerations, 0);
}

TEST_F(DeferredMipmapTests, MarkTwiceGeneratesOnce)
{
    Texture* tex = createTexture(mTexMgr.get());

    tex->_markMipmapsDirty();
    tex->_markMipmapsDirty();
    EXPECT_TRUE(tex->_isMipmapsDirty());

    mTexMgr->_flushMipmaps();
    EXPECT_EQ(MockTexture::mGenerations, 1);
    EXPECT_FALSE(tex->_isMipmapsDirty());

    // nothing was marked since, so the next flush is a no-op
    mTexMgr->_flushMipmaps();
    EXPECT_EQ(MockTexture::mGenerations, 1);

    delete tex;
}

TEST_F(DeferredMipmapTests, ExplicitGenerationClearsDirty)
{
    Texture* tex = createTexture(mTexMgr.get());

    tex->_markMipmapsDirty();
    tex->generateMipmaps();
    EXPECT_EQ(MockTexture::mGenerations, 1);
    EXPECT_FALSE(tex->_isMipmapsDirty());

    // the queued entry is skipped because the flag was cleared
    mTexMgr->_flushMipmaps();
    EXPECT_EQ(MockTexture::mGenerations, 1);

    delete tex;
}

TEST_F(DeferredMipmapTests, DestroyedTextureIsRemoved)
{
    Texture* tex = createTexture(mTexMgr.get());
    tex->_markMipmapsDirty();

    delete tex;

    // if the destructor left a dangling pointer in the queue this touches freed
    // memory; the mock texture also has no generation to report
    mTexMgr->_flushMipmaps();
    EXPECT_EQ(MockTexture::mGenerations, 0);
}

TEST_F(DeferredMipmapTests, DestroyedAfterExplicitGenerationIsRemoved)
{
    Texture* tex = createTexture(mTexMgr.get());
    tex->_markMipmapsDirty();
    tex->generateMipmaps();
    EXPECT_EQ(MockTexture::mGenerations, 1);

    // the flag is clear but the texture is still queued; destroying it must
    // not leave a dangling pointer behind
    delete tex;

    mTexMgr->_flushMipmaps();
    EXPECT_EQ(MockTexture::mGenerations, 1);
}

TEST_F(DeferredMipmapTests, NoOpWithoutAutomipmap)
{
    Texture* tex = createTexture(mTexMgr.get(), HBU_GPU_ONLY);

    tex->_markMipmapsDirty();
    EXPECT_FALSE(tex->_isMipmapsDirty());

    mTexMgr->_flushMipmaps();
    EXPECT_EQ(MockTexture::mGenerations, 0);

    // explicit call also does nothing
    tex->generateMipmaps();
    EXPECT_EQ(MockTexture::mGenerations, 0);

    delete tex;
}

TEST_F(DeferredMipmapTests, NoOpWithoutMipLevels)
{
    Texture* tex = createTexture(mTexMgr.get(), TU_AUTOMIPMAP, 0);

    tex->_markMipmapsDirty();
    EXPECT_FALSE(tex->_isMipmapsDirty());

    mTexMgr->_flushMipmaps();
    EXPECT_EQ(MockTexture::mGenerations, 0);

    delete tex;
}

TEST_F(DeferredMipmapTests, FailedGenerationKeepsOthersQueued)
{
    MockTexture* tex[3];
    for (auto& t : tex)
        t = createTexture(mTexMgr.get());

    // whatever the flush order, the failing texture is in the middle
    tex[1]->mFail = true;
    for (auto t : tex)
        t->_markMipmapsDirty();

    EXPECT_THROW(mTexMgr->_flushMipmaps(), RenderingAPIException);
    EXPECT_FALSE(tex[1]->_isMipmapsDirty());

    // the textures behind the failing one are still generated, and the failing
    // one is not retried on every flush
    mTexMgr->_flushMipmaps();
    EXPECT_EQ(MockTexture::mGenerations, 2);
    EXPECT_EQ(tex[1]->mAttempts, 1);
    for (auto t : tex)
        EXPECT_FALSE(t->_isMipmapsDirty());

    // a new upload marks it again
    tex[1]->mFail = false;
    tex[1]->_markMipmapsDirty();
    mTexMgr->_flushMipmaps();
    EXPECT_EQ(MockTexture::mGenerations, 3);

    for (auto t : tex)
        delete t;
}
