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

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#include "macUtils.h"
#endif

#include "UnitTestSuite.h"

// Register the test suite
CPPUNIT_TEST_SUITE_REGISTRATION(FileSystemArchiveTests);

//--------------------------------------------------------------------------
void FileSystemArchiveTests::setUp()
{
    UnitTestSuite::getSingletonPtr()->startTestSetup(__FUNCTION__);
    
    mFileSizeRoot1 = 0;
    mFileSizeRoot2 = 0;

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
    mTestPath = macBundlePath() + "/Contents/Resources/Media/misc/ArchiveTest";
    mFileSizeRoot1 = 125;
    mFileSizeRoot2 = 150;
#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX || OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
    mTestPath = "./Tests/OgreMain/misc/ArchiveTest";
    mFileSizeRoot1 = 130;
    mFileSizeRoot2 = 156;
#elif OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    mTestPath = "../../Tests/OgreMain/misc/ArchiveTest";
    mFileSizeRoot1 = 125;
    mFileSizeRoot2 = 150;
#endif
}
//--------------------------------------------------------------------------
void FileSystemArchiveTests::tearDown()
{
}
//--------------------------------------------------------------------------
void FileSystemArchiveTests::testListNonRecursive()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    try {
        FileSystemArchive arch(mTestPath, "FileSystem", true);
        arch.load();
        StringVectorPtr vec = arch.list(false);

        CPPUNIT_ASSERT_EQUAL((unsigned int)2, (unsigned int)vec->size());
        CPPUNIT_ASSERT_EQUAL(String("rootfile.txt"), vec->at(0));
        CPPUNIT_ASSERT_EQUAL(String("rootfile2.txt"), vec->at(1));
    }
    catch (Exception& e)
    {
        std::cout << e.getFullDescription();
    }
}
//--------------------------------------------------------------------------
void FileSystemArchiveTests::testListRecursive()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    FileSystemArchive arch(mTestPath, "FileSystem", true);
    arch.load();
    StringVectorPtr vec = arch.list(true);

    CPPUNIT_ASSERT_EQUAL((size_t)6, vec->size());
    CPPUNIT_ASSERT_EQUAL(String("rootfile.txt"), vec->at(0));
    CPPUNIT_ASSERT_EQUAL(String("rootfile2.txt"), vec->at(1));
    CPPUNIT_ASSERT_EQUAL(String("level1/materials/scripts/file.material"), vec->at(2));
    CPPUNIT_ASSERT_EQUAL(String("level1/materials/scripts/file2.material"), vec->at(3));
    CPPUNIT_ASSERT_EQUAL(String("level2/materials/scripts/file3.material"), vec->at(4));
    CPPUNIT_ASSERT_EQUAL(String("level2/materials/scripts/file4.material"), vec->at(5));
}
//--------------------------------------------------------------------------
void FileSystemArchiveTests::testListFileInfoNonRecursive()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    FileSystemArchive arch(mTestPath, "FileSystem", true);
    arch.load();
    FileInfoListPtr vec = arch.listFileInfo(false);

    // Only execute size checks, if the values have been set for the current platform
    if(mFileSizeRoot1 >0 && mFileSizeRoot2 > 0) 
    {
        CPPUNIT_ASSERT_EQUAL((size_t)2, vec->size());
        FileInfo& fi1 = vec->at(0);
        CPPUNIT_ASSERT_EQUAL(String("rootfile.txt"), fi1.filename);
        CPPUNIT_ASSERT_EQUAL(String("rootfile.txt"), fi1.basename);
        CPPUNIT_ASSERT_EQUAL(BLANKSTRING, fi1.path);
        CPPUNIT_ASSERT_EQUAL((size_t)mFileSizeRoot1, fi1.compressedSize);
        CPPUNIT_ASSERT_EQUAL((size_t)mFileSizeRoot1, fi1.uncompressedSize);

        FileInfo& fi2 = vec->at(1);
        CPPUNIT_ASSERT_EQUAL(String("rootfile2.txt"), fi2.filename);
        CPPUNIT_ASSERT_EQUAL(String("rootfile2.txt"), fi2.basename);
        CPPUNIT_ASSERT_EQUAL(BLANKSTRING, fi2.path);
        CPPUNIT_ASSERT_EQUAL((size_t)mFileSizeRoot2, fi2.compressedSize);
        CPPUNIT_ASSERT_EQUAL((size_t)mFileSizeRoot2, fi2.uncompressedSize);
    }
}
//--------------------------------------------------------------------------
void FileSystemArchiveTests::testListFileInfoRecursive()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    FileSystemArchive arch(mTestPath, "FileSystem", true);
    arch.load();
    FileInfoListPtr vec = arch.listFileInfo(true);

    // Only execute size checks, if the values have been set for the current platform
    if(mFileSizeRoot1 >0 && mFileSizeRoot2 > 0) 
    {
        CPPUNIT_ASSERT_EQUAL((size_t)6, vec->size()); 
        FileInfo& fi1 = vec->at(0);
        CPPUNIT_ASSERT_EQUAL(String("rootfile.txt"), fi1.filename);
        CPPUNIT_ASSERT_EQUAL(String("rootfile.txt"), fi1.basename);
        CPPUNIT_ASSERT_EQUAL(BLANKSTRING, fi1.path);
        CPPUNIT_ASSERT_EQUAL((size_t)mFileSizeRoot1, fi1.compressedSize);
        CPPUNIT_ASSERT_EQUAL((size_t)mFileSizeRoot1, fi1.uncompressedSize);

        FileInfo& fi2 = vec->at(1);
        CPPUNIT_ASSERT_EQUAL(String("rootfile2.txt"), fi2.filename);
        CPPUNIT_ASSERT_EQUAL(String("rootfile2.txt"), fi2.basename);
        CPPUNIT_ASSERT_EQUAL(BLANKSTRING, fi2.path);
        CPPUNIT_ASSERT_EQUAL((size_t)mFileSizeRoot2, fi2.compressedSize);
        CPPUNIT_ASSERT_EQUAL((size_t)mFileSizeRoot2, fi2.uncompressedSize);
    }

    FileInfo& fi3 = vec->at(2);
    CPPUNIT_ASSERT_EQUAL(String("level1/materials/scripts/file.material"), fi3.filename);
    CPPUNIT_ASSERT_EQUAL(String("file.material"), fi3.basename);
    CPPUNIT_ASSERT_EQUAL(String("level1/materials/scripts/"), fi3.path);
    CPPUNIT_ASSERT_EQUAL((size_t)0, fi3.compressedSize);
    CPPUNIT_ASSERT_EQUAL((size_t)0, fi3.uncompressedSize);

    FileInfo& fi4 = vec->at(3);
    CPPUNIT_ASSERT_EQUAL(String("level1/materials/scripts/file2.material"), fi4.filename);
    CPPUNIT_ASSERT_EQUAL(String("file2.material"), fi4.basename);
    CPPUNIT_ASSERT_EQUAL(String("level1/materials/scripts/"), fi4.path);
    CPPUNIT_ASSERT_EQUAL((size_t)0, fi4.compressedSize);
    CPPUNIT_ASSERT_EQUAL((size_t)0, fi4.uncompressedSize);

    FileInfo& fi5 = vec->at(4);
    CPPUNIT_ASSERT_EQUAL(String("level2/materials/scripts/file3.material"), fi5.filename);
    CPPUNIT_ASSERT_EQUAL(String("file3.material"), fi5.basename);
    CPPUNIT_ASSERT_EQUAL(String("level2/materials/scripts/"), fi5.path);
    CPPUNIT_ASSERT_EQUAL((size_t)0, fi5.compressedSize);
    CPPUNIT_ASSERT_EQUAL((size_t)0, fi5.uncompressedSize);

    FileInfo& fi6 = vec->at(5);
    CPPUNIT_ASSERT_EQUAL(String("level2/materials/scripts/file4.material"), fi6.filename);
    CPPUNIT_ASSERT_EQUAL(String("file4.material"), fi6.basename);
    CPPUNIT_ASSERT_EQUAL(String("level2/materials/scripts/"), fi6.path);
    CPPUNIT_ASSERT_EQUAL((size_t)0, fi6.compressedSize);
    CPPUNIT_ASSERT_EQUAL((size_t)0, fi6.uncompressedSize);
}
//--------------------------------------------------------------------------
void FileSystemArchiveTests::testFindNonRecursive()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    FileSystemArchive arch(mTestPath, "FileSystem", true);
    arch.load();
    StringVectorPtr vec = arch.find("*.txt", false);

    CPPUNIT_ASSERT_EQUAL((size_t)2, vec->size());
    CPPUNIT_ASSERT_EQUAL(String("rootfile.txt"), vec->at(0));
    CPPUNIT_ASSERT_EQUAL(String("rootfile2.txt"), vec->at(1));
}
//--------------------------------------------------------------------------
void FileSystemArchiveTests::testFindRecursive()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    FileSystemArchive arch(mTestPath, "FileSystem", true);
    arch.load();
    StringVectorPtr vec = arch.find("*.material", true);

    CPPUNIT_ASSERT_EQUAL((size_t)4, vec->size());
    CPPUNIT_ASSERT_EQUAL(String("level1/materials/scripts/file.material"), vec->at(0));
    CPPUNIT_ASSERT_EQUAL(String("level1/materials/scripts/file2.material"), vec->at(1));
    CPPUNIT_ASSERT_EQUAL(String("level2/materials/scripts/file3.material"), vec->at(2));
    CPPUNIT_ASSERT_EQUAL(String("level2/materials/scripts/file4.material"), vec->at(3));
}
//--------------------------------------------------------------------------
void FileSystemArchiveTests::testFindFileInfoNonRecursive()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    FileSystemArchive arch(mTestPath, "FileSystem", true);
    arch.load();
    FileInfoListPtr vec = arch.findFileInfo("*.txt", false);

    // Only execute size checks, if the values have been set for the current platform
    if(mFileSizeRoot1 >0 && mFileSizeRoot2 > 0) 
    {
        CPPUNIT_ASSERT_EQUAL((size_t)2, vec->size());
        FileInfo& fi1 = vec->at(0);
        CPPUNIT_ASSERT_EQUAL(String("rootfile.txt"), fi1.filename);
        CPPUNIT_ASSERT_EQUAL(String("rootfile.txt"), fi1.basename);
        CPPUNIT_ASSERT_EQUAL(BLANKSTRING, fi1.path);
        CPPUNIT_ASSERT_EQUAL((size_t)mFileSizeRoot1, fi1.compressedSize);
        CPPUNIT_ASSERT_EQUAL((size_t)mFileSizeRoot1, fi1.uncompressedSize);

        FileInfo& fi2 = vec->at(1);
        CPPUNIT_ASSERT_EQUAL(String("rootfile2.txt"), fi2.filename);
        CPPUNIT_ASSERT_EQUAL(String("rootfile2.txt"), fi2.basename);
        CPPUNIT_ASSERT_EQUAL(BLANKSTRING, fi2.path);
        CPPUNIT_ASSERT_EQUAL((size_t)mFileSizeRoot2, fi2.compressedSize);
        CPPUNIT_ASSERT_EQUAL((size_t)mFileSizeRoot2, fi2.uncompressedSize);
    }
}
//--------------------------------------------------------------------------
void FileSystemArchiveTests::testFindFileInfoRecursive()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    FileSystemArchive arch(mTestPath, "FileSystem", true);
    arch.load();
    FileInfoListPtr vec = arch.findFileInfo("*.material", true);

    CPPUNIT_ASSERT_EQUAL((size_t)4, vec->size());

    FileInfo& fi3 = vec->at(0);
    CPPUNIT_ASSERT_EQUAL(String("level1/materials/scripts/file.material"), fi3.filename);
    CPPUNIT_ASSERT_EQUAL(String("file.material"), fi3.basename);
    CPPUNIT_ASSERT_EQUAL(String("level1/materials/scripts/"), fi3.path);
    CPPUNIT_ASSERT_EQUAL((size_t)0, fi3.compressedSize);
    CPPUNIT_ASSERT_EQUAL((size_t)0, fi3.uncompressedSize);

    FileInfo& fi4 = vec->at(1);
    CPPUNIT_ASSERT_EQUAL(String("level1/materials/scripts/file2.material"), fi4.filename);
    CPPUNIT_ASSERT_EQUAL(String("file2.material"), fi4.basename);
    CPPUNIT_ASSERT_EQUAL(String("level1/materials/scripts/"), fi4.path);
    CPPUNIT_ASSERT_EQUAL((size_t)0, fi4.compressedSize);
    CPPUNIT_ASSERT_EQUAL((size_t)0, fi4.uncompressedSize);

    FileInfo& fi5 = vec->at(2);
    CPPUNIT_ASSERT_EQUAL(String("level2/materials/scripts/file3.material"), fi5.filename);
    CPPUNIT_ASSERT_EQUAL(String("file3.material"), fi5.basename);
    CPPUNIT_ASSERT_EQUAL(String("level2/materials/scripts/"), fi5.path);
    CPPUNIT_ASSERT_EQUAL((size_t)0, fi5.compressedSize);
    CPPUNIT_ASSERT_EQUAL((size_t)0, fi5.uncompressedSize);

    FileInfo& fi6 = vec->at(3);
    CPPUNIT_ASSERT_EQUAL(String("level2/materials/scripts/file4.material"), fi6.filename);
    CPPUNIT_ASSERT_EQUAL(String("file4.material"), fi6.basename);
    CPPUNIT_ASSERT_EQUAL(String("level2/materials/scripts/"), fi6.path);
    CPPUNIT_ASSERT_EQUAL((size_t)0, fi6.compressedSize);
    CPPUNIT_ASSERT_EQUAL((size_t)0, fi6.uncompressedSize);
}
//--------------------------------------------------------------------------
void FileSystemArchiveTests::testFileRead()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    FileSystemArchive arch(mTestPath, "FileSystem", true);
    arch.load();

    DataStreamPtr stream = arch.open("rootfile.txt");
    CPPUNIT_ASSERT_EQUAL(String("this is line 1 in file 1"), stream->getLine());
    CPPUNIT_ASSERT_EQUAL(String("this is line 2 in file 1"), stream->getLine());
    CPPUNIT_ASSERT_EQUAL(String("this is line 3 in file 1"), stream->getLine());
    CPPUNIT_ASSERT_EQUAL(String("this is line 4 in file 1"), stream->getLine());
    CPPUNIT_ASSERT_EQUAL(String("this is line 5 in file 1"), stream->getLine());
    CPPUNIT_ASSERT_EQUAL(BLANKSTRING, stream->getLine()); // blank at end of file
    CPPUNIT_ASSERT(stream->eof());
}
//--------------------------------------------------------------------------
void FileSystemArchiveTests::testReadInterleave()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    // Test overlapping reads from same archive
    FileSystemArchive arch(mTestPath, "FileSystem", true);
    arch.load();

    // File 1
    DataStreamPtr stream1 = arch.open("rootfile.txt");
    CPPUNIT_ASSERT_EQUAL(String("this is line 1 in file 1"), stream1->getLine());
    CPPUNIT_ASSERT_EQUAL(String("this is line 2 in file 1"), stream1->getLine());

    // File 2
    DataStreamPtr stream2 = arch.open("rootfile2.txt");
    CPPUNIT_ASSERT_EQUAL(String("this is line 1 in file 2"), stream2->getLine());
    CPPUNIT_ASSERT_EQUAL(String("this is line 2 in file 2"), stream2->getLine());
    CPPUNIT_ASSERT_EQUAL(String("this is line 3 in file 2"), stream2->getLine());
    
    // File 1
    CPPUNIT_ASSERT_EQUAL(String("this is line 3 in file 1"), stream1->getLine());
    CPPUNIT_ASSERT_EQUAL(String("this is line 4 in file 1"), stream1->getLine());
    CPPUNIT_ASSERT_EQUAL(String("this is line 5 in file 1"), stream1->getLine());
    CPPUNIT_ASSERT_EQUAL(BLANKSTRING, stream1->getLine()); // blank at end of file
    CPPUNIT_ASSERT(stream1->eof());

    // File 2
    CPPUNIT_ASSERT_EQUAL(String("this is line 4 in file 2"), stream2->getLine());
    CPPUNIT_ASSERT_EQUAL(String("this is line 5 in file 2"), stream2->getLine());
    CPPUNIT_ASSERT_EQUAL(String("this is line 6 in file 2"), stream2->getLine());
    CPPUNIT_ASSERT_EQUAL(BLANKSTRING, stream2->getLine()); // blank at end of file
    CPPUNIT_ASSERT(stream2->eof());
}
//--------------------------------------------------------------------------
void FileSystemArchiveTests::testCreateAndRemoveFile()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    FileSystemArchive arch("./", "FileSystem", false);
    arch.load();

    CPPUNIT_ASSERT(!arch.isReadOnly());

    String fileName = "a_test_file.txt";
    DataStreamPtr stream = arch.create(fileName);

    String testString = "Some text here";
    size_t written = stream->write((void*)testString.c_str(), testString.size());
    CPPUNIT_ASSERT_EQUAL(testString.size(), written);

    stream->close();

    arch.remove(fileName);

    CPPUNIT_ASSERT(!arch.exists(fileName));
}
//--------------------------------------------------------------------------
