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
#include "FileSystemArchiveTests.h"
#include "OgreFileSystem.h"
#include "OgreException.h"
#include "OgreCommon.h"
#include "OgreConfigFile.h"
#include "OgreFileSystemLayer.h"


namespace Ogre {
static bool operator<(const FileInfo& a, const FileInfo& b) {
    return a.basename < b.basename;
}
}
// Register the test suite

//--------------------------------------------------------------------------
void FileSystemArchiveTests::SetUp()
{    
    mFileSizeRoot1 = 125;
    mFileSizeRoot2 = 150;

    Ogre::ConfigFile cf;
    cf.load(Ogre::FileSystemLayer(OGRE_VERSION_NAME).getConfigFilePath("resources.cfg"));
    mTestPath = cf.getSettings("Tests").begin()->second+"/misc/ArchiveTest";

    mArch = mFactory.createInstance(mTestPath, false);
    mArch->load();
}
//--------------------------------------------------------------------------
void FileSystemArchiveTests::TearDown()
{
    if(mArch)
        mFactory.destroyInstance(mArch);
}
//--------------------------------------------------------------------------
TEST_F(FileSystemArchiveTests,ListNonRecursive)
{
    StringVectorPtr vec = mArch->list(false);

    EXPECT_EQ((unsigned int)2, (unsigned int)vec->size());
    sort(vec->begin(), vec->end());
    EXPECT_EQ(String("rootfile.txt"), vec->at(0));
    EXPECT_EQ(String("rootfile2.txt"), vec->at(1));

    EXPECT_EQ("rootfile.txt", mArch->open("rootfile.txt")->getName());
}

TEST_F(FileSystemArchiveTests,Exists)
{
    EXPECT_FALSE(mArch->exists(""));
}
//--------------------------------------------------------------------------
TEST_F(FileSystemArchiveTests,ListRecursive)
{
    StringVectorPtr vec = mArch->list(true);

    EXPECT_EQ((size_t)6, vec->size());
    sort(vec->begin(), vec->end());
    EXPECT_EQ(String("level1/materials/scripts/file.material"), vec->at(0));
    EXPECT_EQ(String("level1/materials/scripts/file2.material"), vec->at(1));
    EXPECT_EQ(String("level2/materials/scripts/file3.material"), vec->at(2));
    EXPECT_EQ(String("level2/materials/scripts/file4.material"), vec->at(3));
    EXPECT_EQ(String("rootfile.txt"), vec->at(4));
    EXPECT_EQ(String("rootfile2.txt"), vec->at(5));
}
//--------------------------------------------------------------------------
TEST_F(FileSystemArchiveTests,ListFileInfoNonRecursive)
{
    FileInfoListPtr vec = mArch->listFileInfo(false);

    // Only execute size checks, if the values have been set for the current platform
    if(mFileSizeRoot1 >0 && mFileSizeRoot2 > 0) 
    {
        EXPECT_EQ((size_t)2, vec->size());
        sort(vec->begin(), vec->end());
        FileInfo& fi1 = vec->at(0);
        EXPECT_EQ(String("rootfile.txt"), fi1.filename);
        EXPECT_EQ(String("rootfile.txt"), fi1.basename);
        EXPECT_EQ(BLANKSTRING, fi1.path);
        EXPECT_EQ((size_t)mFileSizeRoot1, fi1.compressedSize);
        EXPECT_EQ((size_t)mFileSizeRoot1, fi1.uncompressedSize);

        FileInfo& fi2 = vec->at(1);
        EXPECT_EQ(String("rootfile2.txt"), fi2.filename);
        EXPECT_EQ(String("rootfile2.txt"), fi2.basename);
        EXPECT_EQ(BLANKSTRING, fi2.path);
        EXPECT_EQ((size_t)mFileSizeRoot2, fi2.compressedSize);
        EXPECT_EQ((size_t)mFileSizeRoot2, fi2.uncompressedSize);
    }
}
//--------------------------------------------------------------------------
TEST_F(FileSystemArchiveTests,ListFileInfoRecursive)
{
    FileInfoListPtr vec = mArch->listFileInfo(true);
    sort(vec->begin(), vec->end());

    // Only execute size checks, if the values have been set for the current platform
    if(mFileSizeRoot1 >0 && mFileSizeRoot2 > 0) 
    {
        EXPECT_EQ((size_t)6, vec->size()); 
        FileInfo& fi1 = vec->at(4);
        EXPECT_EQ(String("rootfile.txt"), fi1.filename);
        EXPECT_EQ(String("rootfile.txt"), fi1.basename);
        EXPECT_EQ(BLANKSTRING, fi1.path);
        EXPECT_EQ((size_t)mFileSizeRoot1, fi1.compressedSize);
        EXPECT_EQ((size_t)mFileSizeRoot1, fi1.uncompressedSize);

        FileInfo& fi2 = vec->at(5);
        EXPECT_EQ(String("rootfile2.txt"), fi2.filename);
        EXPECT_EQ(String("rootfile2.txt"), fi2.basename);
        EXPECT_EQ(BLANKSTRING, fi2.path);
        EXPECT_EQ((size_t)mFileSizeRoot2, fi2.compressedSize);
        EXPECT_EQ((size_t)mFileSizeRoot2, fi2.uncompressedSize);
    }

    FileInfo& fi3 = vec->at(0);
    EXPECT_EQ(String("level1/materials/scripts/file.material"), fi3.filename);
    EXPECT_EQ(String("file.material"), fi3.basename);
    EXPECT_EQ(String("level1/materials/scripts/"), fi3.path);
    EXPECT_EQ((size_t)0, fi3.compressedSize);
    EXPECT_EQ((size_t)0, fi3.uncompressedSize);

    FileInfo& fi4 = vec->at(1);
    EXPECT_EQ(String("level1/materials/scripts/file2.material"), fi4.filename);
    EXPECT_EQ(String("file2.material"), fi4.basename);
    EXPECT_EQ(String("level1/materials/scripts/"), fi4.path);
    EXPECT_EQ((size_t)0, fi4.compressedSize);
    EXPECT_EQ((size_t)0, fi4.uncompressedSize);

    FileInfo& fi5 = vec->at(2);
    EXPECT_EQ(String("level2/materials/scripts/file3.material"), fi5.filename);
    EXPECT_EQ(String("file3.material"), fi5.basename);
    EXPECT_EQ(String("level2/materials/scripts/"), fi5.path);
    EXPECT_EQ((size_t)0, fi5.compressedSize);
    EXPECT_EQ((size_t)0, fi5.uncompressedSize);

    FileInfo& fi6 = vec->at(3);
    EXPECT_EQ(String("level2/materials/scripts/file4.material"), fi6.filename);
    EXPECT_EQ(String("file4.material"), fi6.basename);
    EXPECT_EQ(String("level2/materials/scripts/"), fi6.path);
    EXPECT_EQ((size_t)0, fi6.compressedSize);
    EXPECT_EQ((size_t)0, fi6.uncompressedSize);
}
//--------------------------------------------------------------------------
TEST_F(FileSystemArchiveTests,FindNonRecursive)
{
    StringVectorPtr vec = mArch->find("*.txt", false);

    EXPECT_EQ((size_t)2, vec->size());
    sort(vec->begin(), vec->end());
    EXPECT_EQ(String("rootfile.txt"), vec->at(0));
    EXPECT_EQ(String("rootfile2.txt"), vec->at(1));
}
//--------------------------------------------------------------------------
TEST_F(FileSystemArchiveTests,FindRecursive)
{
    StringVectorPtr vec = mArch->find("*.material", true);

    EXPECT_EQ((size_t)4, vec->size());
    sort(vec->begin(), vec->end());
    EXPECT_EQ(String("level1/materials/scripts/file.material"), vec->at(0));
    EXPECT_EQ(String("level1/materials/scripts/file2.material"), vec->at(1));
    EXPECT_EQ(String("level2/materials/scripts/file3.material"), vec->at(2));
    EXPECT_EQ(String("level2/materials/scripts/file4.material"), vec->at(3));
}
//--------------------------------------------------------------------------
TEST_F(FileSystemArchiveTests,FindFileInfoNonRecursive)
{
    FileInfoListPtr vec = mArch->findFileInfo("*.txt", false);

    // Only execute size checks, if the values have been set for the current platform
    if(mFileSizeRoot1 >0 && mFileSizeRoot2 > 0) 
    {
        EXPECT_EQ((size_t)2, vec->size());
        sort(vec->begin(), vec->end());
        FileInfo& fi1 = vec->at(0);
        EXPECT_EQ(String("rootfile.txt"), fi1.filename);
        EXPECT_EQ(String("rootfile.txt"), fi1.basename);
        EXPECT_EQ(BLANKSTRING, fi1.path);
        EXPECT_EQ((size_t)mFileSizeRoot1, fi1.compressedSize);
        EXPECT_EQ((size_t)mFileSizeRoot1, fi1.uncompressedSize);

        FileInfo& fi2 = vec->at(1);
        EXPECT_EQ(String("rootfile2.txt"), fi2.filename);
        EXPECT_EQ(String("rootfile2.txt"), fi2.basename);
        EXPECT_EQ(BLANKSTRING, fi2.path);
        EXPECT_EQ((size_t)mFileSizeRoot2, fi2.compressedSize);
        EXPECT_EQ((size_t)mFileSizeRoot2, fi2.uncompressedSize);
    }
}
//--------------------------------------------------------------------------
TEST_F(FileSystemArchiveTests,FindFileInfoRecursive)
{
    FileInfoListPtr vec = mArch->findFileInfo("*.material", true);

    EXPECT_EQ((size_t)4, vec->size());
    sort(vec->begin(), vec->end());

    FileInfo& fi3 = vec->at(0);
    EXPECT_EQ(String("level1/materials/scripts/file.material"), fi3.filename);
    EXPECT_EQ(String("file.material"), fi3.basename);
    EXPECT_EQ(String("level1/materials/scripts/"), fi3.path);
    EXPECT_EQ((size_t)0, fi3.compressedSize);
    EXPECT_EQ((size_t)0, fi3.uncompressedSize);

    FileInfo& fi4 = vec->at(1);
    EXPECT_EQ(String("level1/materials/scripts/file2.material"), fi4.filename);
    EXPECT_EQ(String("file2.material"), fi4.basename);
    EXPECT_EQ(String("level1/materials/scripts/"), fi4.path);
    EXPECT_EQ((size_t)0, fi4.compressedSize);
    EXPECT_EQ((size_t)0, fi4.uncompressedSize);

    FileInfo& fi5 = vec->at(2);
    EXPECT_EQ(String("level2/materials/scripts/file3.material"), fi5.filename);
    EXPECT_EQ(String("file3.material"), fi5.basename);
    EXPECT_EQ(String("level2/materials/scripts/"), fi5.path);
    EXPECT_EQ((size_t)0, fi5.compressedSize);
    EXPECT_EQ((size_t)0, fi5.uncompressedSize);

    FileInfo& fi6 = vec->at(3);
    EXPECT_EQ(String("level2/materials/scripts/file4.material"), fi6.filename);
    EXPECT_EQ(String("file4.material"), fi6.basename);
    EXPECT_EQ(String("level2/materials/scripts/"), fi6.path);
    EXPECT_EQ((size_t)0, fi6.compressedSize);
    EXPECT_EQ((size_t)0, fi6.uncompressedSize);
}
//--------------------------------------------------------------------------
TEST_F(FileSystemArchiveTests,FileRead)
{
    DataStreamPtr stream = mArch->open("rootfile.txt");
    EXPECT_EQ(String("this is line 1 in file 1"), stream->getLine());
    EXPECT_EQ(String("this is line 2 in file 1"), stream->getLine());
    EXPECT_EQ(String("this is line 3 in file 1"), stream->getLine());
    EXPECT_EQ(String("this is line 4 in file 1"), stream->getLine());
    EXPECT_EQ(String("this is line 5 in file 1"), stream->getLine());
    EXPECT_EQ(BLANKSTRING, stream->getLine()); // blank at end of file
    EXPECT_TRUE(stream->eof());
}
//--------------------------------------------------------------------------
TEST_F(FileSystemArchiveTests,ReadInterleave)
{
    // Test overlapping reads from same archive

    // File 1
    DataStreamPtr stream1 = mArch->open("rootfile.txt");
    EXPECT_EQ(String("this is line 1 in file 1"), stream1->getLine());
    EXPECT_EQ(String("this is line 2 in file 1"), stream1->getLine());

    // File 2
    DataStreamPtr stream2 = mArch->open("rootfile2.txt");
    EXPECT_EQ(String("this is line 1 in file 2"), stream2->getLine());
    EXPECT_EQ(String("this is line 2 in file 2"), stream2->getLine());
    EXPECT_EQ(String("this is line 3 in file 2"), stream2->getLine());
    
    // File 1
    EXPECT_EQ(String("this is line 3 in file 1"), stream1->getLine());
    EXPECT_EQ(String("this is line 4 in file 1"), stream1->getLine());
    EXPECT_EQ(String("this is line 5 in file 1"), stream1->getLine());
    EXPECT_EQ(BLANKSTRING, stream1->getLine()); // blank at end of file
    EXPECT_TRUE(stream1->eof());

    // File 2
    EXPECT_EQ(String("this is line 4 in file 2"), stream2->getLine());
    EXPECT_EQ(String("this is line 5 in file 2"), stream2->getLine());
    EXPECT_EQ(String("this is line 6 in file 2"), stream2->getLine());
    EXPECT_EQ(BLANKSTRING, stream2->getLine()); // blank at end of file
    EXPECT_TRUE(stream2->eof());
}
//--------------------------------------------------------------------------
TEST_F(FileSystemArchiveTests,CreateAndRemoveFile)
{
    EXPECT_TRUE(!mArch->isReadOnly());

    String fileName = "a_test_file.txt";
    DataStreamPtr stream = mArch->create(fileName);

    String testString = "Some text here";
    size_t written = stream->write((const void*)testString.c_str(), testString.size());
    EXPECT_EQ(testString.size(), written);

    stream->close();

    mArch->remove(fileName);

    EXPECT_TRUE(!mArch->exists(fileName));
}
//--------------------------------------------------------------------------
