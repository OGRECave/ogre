/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2019 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#include <gtest/gtest.h>

#include "ResourceLocationPriorityTest.h"
#include "RootWithoutRenderSystemFixture.h"

#include "OgreArchiveManager.h"

TEST(ResourceGroupLocationTest, ResourceLocationPriority)
{
    std::unique_ptr<DummyArchiveFactory> fact = std::unique_ptr<DummyArchiveFactory>(new DummyArchiveFactory);
    Ogre::Root root("");
    Ogre::ArchiveManager& archiveMgr = Ogre::ArchiveManager::getSingleton();
    archiveMgr.addArchiveFactory(fact.get());

    Ogre::ResourceGroupManager& resGrpMgr = Ogre::ResourceGroupManager::getSingleton();
    resGrpMgr.addResourceLocation("ResourceLocationPriority0", "DummyArchive");
    resGrpMgr.addResourceLocation("ResourceLocationPriority1", "DummyArchive");

    Ogre::DataStreamPtr filePtr = resGrpMgr.openResource("dummyArchiveTest");
    EXPECT_TRUE(filePtr);

    unsigned char contents;
    filePtr->read(&contents, 1);
    // If the archive added first is returned then the file will contain 0x1,
    // otherwise it will contain 0x2.
    // Expect 0x1, as locations added first are preferred over locations added later.
    EXPECT_EQ(contents, 1);

    resGrpMgr.removeResourceLocation("ResourceLocationPriority0");
    resGrpMgr.removeResourceLocation("ResourceLocationPriority1");
}