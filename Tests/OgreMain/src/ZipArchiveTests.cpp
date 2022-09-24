/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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
#include "ZipArchiveTests.h"
#include "Threading/OgreThreadHeaders.h"
#include "OgreCommon.h"
#include "OgreConfigFile.h"
#include "OgreFileSystemLayer.h"

using namespace Ogre;

static String fileId(const String& path) {
#if !OGRE_RESOURCEMANAGER_STRICT
    String file;
    String base;
    StringUtil::splitFilename(path, file, base);
    return file;
#endif
    return path;
}

//--------------------------------------------------------------------------
void ZipArchiveTests::SetUp()
{
    Ogre::ConfigFile cf;
    cf.load(Ogre::FileSystemLayer(OGRE_VERSION_NAME).getConfigFilePath("resources.cfg"));
    Ogre::String testPath = cf.getSettings("Tests").begin()->second+"/misc/ArchiveTest.zip";

    arch = Ogre::ZipArchiveFactory().createInstance(testPath, true);
    arch->load();
}
//--------------------------------------------------------------------------
void ZipArchiveTests::TearDown()
{
    OGRE_DELETE arch;
}
//--------------------------------------------------------------------------
TEST_F(ZipArchiveTests,ListNonRecursive)
{
    StringVectorPtr vec = arch->list(false);

    EXPECT_EQ((size_t)2, vec->size());
    EXPECT_EQ(String("rootfile.txt"), vec->at(0));
    EXPECT_EQ(String("rootfile2.txt"), vec->at(1));

    EXPECT_EQ("rootfile.txt", arch->open("rootfile.txt")->getName());
}
//--------------------------------------------------------------------------
TEST_F(ZipArchiveTests,ListRecursive)
{
    StringVectorPtr vec = arch->list(true);

    EXPECT_EQ((size_t)6, vec->size());
    EXPECT_EQ(fileId("level1/materials/scripts/file.material"), vec->at(0));
    EXPECT_EQ(fileId("level1/materials/scripts/file2.material"), vec->at(1));
    EXPECT_EQ(fileId("level2/materials/scripts/file3.material"), vec->at(2));
    EXPECT_EQ(fileId("level2/materials/scripts/file4.material"), vec->at(3));
    EXPECT_EQ(String("rootfile.txt"), vec->at(4));
    EXPECT_EQ(String("rootfile2.txt"), vec->at(5));
}
//--------------------------------------------------------------------------
TEST_F(ZipArchiveTests,ListFileInfoNonRecursive)
{
    FileInfoListPtr vec = arch->listFileInfo(false);

    EXPECT_EQ((size_t)2, vec->size());
    FileInfo& fi1 = vec->at(0);
    EXPECT_EQ(String("rootfile.txt"), fi1.filename);
    EXPECT_EQ(BLANKSTRING, fi1.path);
    EXPECT_EQ((size_t)40, fi1.compressedSize);
    EXPECT_EQ((size_t)130, fi1.uncompressedSize);

    FileInfo& fi2 = vec->at(1);
    EXPECT_EQ(String("rootfile2.txt"), fi2.filename);
    EXPECT_EQ(BLANKSTRING, fi2.path);
    EXPECT_EQ((size_t)45, fi2.compressedSize);
    EXPECT_EQ((size_t)156, fi2.uncompressedSize);
}
//--------------------------------------------------------------------------
TEST_F(ZipArchiveTests,ListFileInfoRecursive)
{
    FileInfoListPtr vec = arch->listFileInfo(true);

    EXPECT_EQ((size_t)6, vec->size());
    FileInfo& fi3 = vec->at(0);
    EXPECT_EQ(fileId("level1/materials/scripts/file.material"), fi3.filename);
    EXPECT_EQ(String("level1/materials/scripts/"), fi3.path);
    EXPECT_EQ((size_t)0, fi3.compressedSize);
    EXPECT_EQ((size_t)0, fi3.uncompressedSize);

    FileInfo& fi4 = vec->at(1);
    EXPECT_EQ(fileId("level1/materials/scripts/file2.material"), fi4.filename);
    EXPECT_EQ(String("level1/materials/scripts/"), fi4.path);
    EXPECT_EQ((size_t)0, fi4.compressedSize);
    EXPECT_EQ((size_t)0, fi4.uncompressedSize);

    FileInfo& fi5 = vec->at(2);
    EXPECT_EQ(fileId("level2/materials/scripts/file3.material"), fi5.filename);
    EXPECT_EQ(String("level2/materials/scripts/"), fi5.path);
    EXPECT_EQ((size_t)0, fi5.compressedSize);
    EXPECT_EQ((size_t)0, fi5.uncompressedSize);

    FileInfo& fi6 = vec->at(3);
    EXPECT_EQ(fileId("level2/materials/scripts/file4.material"), fi6.filename);
    EXPECT_EQ(String("level2/materials/scripts/"), fi6.path);
    EXPECT_EQ((size_t)0, fi6.compressedSize);
    EXPECT_EQ((size_t)0, fi6.uncompressedSize);

    FileInfo& fi1 = vec->at(4);
    EXPECT_EQ(String("rootfile.txt"), fi1.filename);
    EXPECT_EQ(BLANKSTRING, fi1.path);
    EXPECT_EQ((size_t)40, fi1.compressedSize);
    EXPECT_EQ((size_t)130, fi1.uncompressedSize);

    FileInfo& fi2 = vec->at(5);
    EXPECT_EQ(String("rootfile2.txt"), fi2.filename);
    EXPECT_EQ(BLANKSTRING, fi2.path);
    EXPECT_EQ((size_t)45, fi2.compressedSize);
    EXPECT_EQ((size_t)156, fi2.uncompressedSize);
}
//--------------------------------------------------------------------------
TEST_F(ZipArchiveTests,FindNonRecursive)
{
    StringVectorPtr vec = arch->find("*.txt", false);

    EXPECT_EQ((size_t)2, vec->size());
    EXPECT_EQ(String("rootfile.txt"), vec->at(0));
    EXPECT_EQ(String("rootfile2.txt"), vec->at(1));
}
//--------------------------------------------------------------------------
TEST_F(ZipArchiveTests,FindRecursive)
{
    StringVectorPtr vec = arch->find("*.material", true);

    EXPECT_EQ((size_t)4, vec->size());
    EXPECT_EQ(fileId("level1/materials/scripts/file.material"), vec->at(0));
    EXPECT_EQ(fileId("level1/materials/scripts/file2.material"), vec->at(1));
    EXPECT_EQ(fileId("level2/materials/scripts/file3.material"), vec->at(2));
    EXPECT_EQ(fileId("level2/materials/scripts/file4.material"), vec->at(3));
}
//--------------------------------------------------------------------------
TEST_F(ZipArchiveTests,FindFileInfoNonRecursive)
{
    FileInfoListPtr vec = arch->findFileInfo("*.txt", false);

    EXPECT_EQ((size_t)2, vec->size());
    FileInfo& fi1 = vec->at(0);
    EXPECT_EQ(String("rootfile.txt"), fi1.filename);
    EXPECT_EQ(BLANKSTRING, fi1.path);
    EXPECT_EQ((size_t)40, fi1.compressedSize);
    EXPECT_EQ((size_t)130, fi1.uncompressedSize);

    FileInfo& fi2 = vec->at(1);
    EXPECT_EQ(String("rootfile2.txt"), fi2.filename);
    EXPECT_EQ(BLANKSTRING, fi2.path);
    EXPECT_EQ((size_t)45, fi2.compressedSize);
    EXPECT_EQ((size_t)156, fi2.uncompressedSize);
}
//--------------------------------------------------------------------------
TEST_F(ZipArchiveTests,FindFileInfoRecursive)
{
    FileInfoListPtr vec = arch->findFileInfo("*.material", true);

    EXPECT_EQ((size_t)4, vec->size());

    FileInfo& fi3 = vec->at(0);
    EXPECT_EQ(fileId("level1/materials/scripts/file.material"), fi3.filename);
    EXPECT_EQ(String("level1/materials/scripts/"), fi3.path);
    EXPECT_EQ((size_t)0, fi3.compressedSize);
    EXPECT_EQ((size_t)0, fi3.uncompressedSize);

    FileInfo& fi4 = vec->at(1);
    EXPECT_EQ(fileId("level1/materials/scripts/file2.material"), fi4.filename);
    EXPECT_EQ(String("level1/materials/scripts/"), fi4.path);
    EXPECT_EQ((size_t)0, fi4.compressedSize);
    EXPECT_EQ((size_t)0, fi4.uncompressedSize);

    FileInfo& fi5 = vec->at(2);
    EXPECT_EQ(fileId("level2/materials/scripts/file3.material"), fi5.filename);
    EXPECT_EQ(String("level2/materials/scripts/"), fi5.path);
    EXPECT_EQ((size_t)0, fi5.compressedSize);
    EXPECT_EQ((size_t)0, fi5.uncompressedSize);

    FileInfo& fi6 = vec->at(3);
    EXPECT_EQ(fileId("level2/materials/scripts/file4.material"), fi6.filename);
    EXPECT_EQ(String("level2/materials/scripts/"), fi6.path);
    EXPECT_EQ((size_t)0, fi6.compressedSize);
    EXPECT_EQ((size_t)0, fi6.uncompressedSize);
}
//--------------------------------------------------------------------------
TEST_F(ZipArchiveTests,FileRead)
{
    DataStreamPtr stream = arch->open("rootfile.txt");
    EXPECT_EQ(String("this is line 1 in file 1"), stream->getLine());
    EXPECT_EQ(String("this is line 2 in file 1"), stream->getLine());
    EXPECT_EQ(String("this is line 3 in file 1"), stream->getLine());
    EXPECT_EQ(String("this is line 4 in file 1"), stream->getLine());
    EXPECT_EQ(String("this is line 5 in file 1"), stream->getLine());
    EXPECT_TRUE(stream->eof());
}
//--------------------------------------------------------------------------
TEST_F(ZipArchiveTests,ReadInterleave)
{
    // Test overlapping reads from same archive
    // File 1
    DataStreamPtr stream1 = arch->open("rootfile.txt");
    EXPECT_EQ(String("this is line 1 in file 1"), stream1->getLine());
    EXPECT_EQ(String("this is line 2 in file 1"), stream1->getLine());

    // File 2
    DataStreamPtr stream2 = arch->open("rootfile2.txt");
    EXPECT_EQ(String("this is line 1 in file 2"), stream2->getLine());
    EXPECT_EQ(String("this is line 2 in file 2"), stream2->getLine());
    EXPECT_EQ(String("this is line 3 in file 2"), stream2->getLine());

    // File 1
    EXPECT_EQ(String("this is line 3 in file 1"), stream1->getLine());
    EXPECT_EQ(String("this is line 4 in file 1"), stream1->getLine());
    EXPECT_EQ(String("this is line 5 in file 1"), stream1->getLine());
    EXPECT_TRUE(stream1->eof());

    // File 2
    EXPECT_EQ(String("this is line 4 in file 2"), stream2->getLine());
    EXPECT_EQ(String("this is line 5 in file 2"), stream2->getLine());
    EXPECT_EQ(String("this is line 6 in file 2"), stream2->getLine());
    EXPECT_TRUE(stream2->eof());
}
//--------------------------------------------------------------------------
