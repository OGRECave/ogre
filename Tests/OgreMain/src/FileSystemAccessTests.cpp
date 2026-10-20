// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include "RootWithoutRenderSystemFixture.h"
#include "OgreDataStream.h"
#include "OgreFileSystemLayer.h"
#include "OgreResourceGroupManager.h"

using namespace Ogre;

namespace {

const char* TEST_DIR = "OgreFileSystemAccessTest";
const char* TEST_GROUP = "OgreFileSystemAccessTest";

void writeFile(const String& path, const String& contents)
{
    DataStreamPtr s = Root::createFileStream(path, "", true);
    s->write(contents.c_str(), contents.size());
}

} // namespace

class FileSystemAccessTest : public RootWithoutRenderSystemFixture
{
protected:
    void SetUp() override
    {
        RootWithoutRenderSystemFixture::SetUp();
        FileSystemLayer::createDirectory(TEST_DIR);
        writeFile(String(TEST_DIR) + "/shadow.txt", "archived");

        auto& rgm = ResourceGroupManager::getSingleton();
        rgm.createResourceGroup(TEST_GROUP, false);
        rgm.addResourceLocation(TEST_DIR, "FileSystem", TEST_GROUP, false, false);
    }

    void TearDown() override
    {
        auto& rgm = ResourceGroupManager::getSingleton();
        rgm.destroyResourceGroup(TEST_GROUP);

        FileSystemLayer::removeFile(String(TEST_DIR) + "/shadow.txt");
        FileSystemLayer::removeFile(String(TEST_DIR) + "/created.txt");
        FileSystemLayer::removeDirectory(TEST_DIR);
        FileSystemLayer::removeFile("shadow.txt");
        FileSystemLayer::removeFile("created.txt");
        FileSystemLayer::removeFile("direct.txt");
    }
};

TEST_F(FileSystemAccessTest, OpenReadsFromGroup)
{
    writeFile("shadow.txt", "filesystem");

    EXPECT_EQ("archived", Root::openFileStream("shadow.txt", TEST_GROUP)->getAsString());
}

TEST_F(FileSystemAccessTest, OpenEmptyGroupBypassesResourceSystem)
{
    writeFile("shadow.txt", "filesystem");

    EXPECT_EQ("filesystem", Root::openFileStream("shadow.txt", "")->getAsString());
}

TEST_F(FileSystemAccessTest, CreateWritesToGroup)
{
    Root::createFileStream("created.txt", TEST_GROUP, true)->close();

    EXPECT_TRUE(FileSystemLayer::fileExists(String(TEST_DIR) + "/created.txt"));
    EXPECT_FALSE(FileSystemLayer::fileExists("created.txt"));
}

TEST_F(FileSystemAccessTest, CreateEmptyGroupBypassesResourceSystem)
{
    Root::createFileStream("direct.txt", "", true)->close();

    EXPECT_TRUE(FileSystemLayer::fileExists("direct.txt"));
    EXPECT_FALSE(FileSystemLayer::fileExists(String(TEST_DIR) + "/direct.txt"));
}
