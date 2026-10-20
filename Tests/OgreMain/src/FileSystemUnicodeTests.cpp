// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include "OgreFileSystemLayer.h"
#include "OgreFileSystem.h"
#include "OgreArchiveManager.h"
#include "OgreRoot.h"
#include "OgreException.h"

using namespace Ogre;

// Non-ASCII path components that are not representable in most ANSI code pages.
// They exercise the UTF-8 to UTF-16 conversion on Windows.
static const char* UNICODE_DIR = "OgreUnicodeTest_\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E"; // 日本語
static const char* UNICODE_FILE = "\xE3\x83\x86\xE3\x82\xB9\xE3\x83\x88.txt";            // テスト.txt

class FileSystemUnicodeTests : public ::testing::Test
{
protected:
    String mDir;

    void SetUp() override
    {
        mDir = UNICODE_DIR;
        FileSystemLayer::createDirectory(mDir);
    }

    void TearDown() override
    {
        FileSystemLayer::removeFile(mDir + "/" + UNICODE_FILE);
        FileSystemLayer::removeDirectory(mDir);
    }
};

TEST_F(FileSystemUnicodeTests, CreateWriteReadRemove)
{
    const String path = mDir + "/" + UNICODE_FILE;
    const String contents = "unicode file contents";

    ASSERT_FALSE(FileSystemLayer::fileExists(path));

    DataStreamPtr out = Root::createFileStream(path, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true);
    out->write(contents.c_str(), contents.size());
    out->close();

    ASSERT_TRUE(FileSystemLayer::fileExists(path));

    DataStreamPtr in = Root::openFileStream(path);
    EXPECT_EQ(contents, in->getAsString());
    in->close();

    EXPECT_TRUE(FileSystemLayer::removeFile(path));
    EXPECT_FALSE(FileSystemLayer::fileExists(path));
}

TEST_F(FileSystemUnicodeTests, Rename)
{
    const String path = mDir + "/" + UNICODE_FILE;
    const String renamed = path + ".renamed";

    DataStreamPtr out = Root::createFileStream(path, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true);
    out->write("x", 1);
    out->close();

    ASSERT_TRUE(FileSystemLayer::renameFile(path, renamed));
    EXPECT_FALSE(FileSystemLayer::fileExists(path));
    EXPECT_TRUE(FileSystemLayer::fileExists(renamed));

    EXPECT_TRUE(FileSystemLayer::removeFile(renamed));
}

TEST_F(FileSystemUnicodeTests, ArchiveEnumeration)
{
    const String path = mDir + "/" + UNICODE_FILE;

    DataStreamPtr out = Root::createFileStream(path, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true);
    out->write("x", 1);
    out->close();

    FileSystemArchiveFactory factory;
    Archive* arch = factory.createInstance(mDir, false);
    arch->load();

    EXPECT_TRUE(arch->exists(UNICODE_FILE));

    StringVectorPtr files = arch->list(false);
    ASSERT_EQ((size_t)1, files->size());
    EXPECT_EQ(String(UNICODE_FILE), files->at(0));

    EXPECT_NE((time_t)0, arch->getModifiedTime(UNICODE_FILE));

    factory.destroyInstance(arch);
}
