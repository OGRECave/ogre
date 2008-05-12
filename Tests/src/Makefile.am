INCLUDES = -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Tests/OgreMain/include $(CPPUNIT_CFLAGS) -I../include

TESTS = TestSuite
check_PROGRAMS  = TestSuite

TestSuite_SOURCES = main.cpp \
                    Suite.cpp \
                    ../OgreMain/src/EdgeBuilderTests.cpp \
                    ../OgreMain/src/FileSystemArchiveTests.cpp \
                    ../OgreMain/src/StringTests.cpp \
                    ../OgreMain/src/ZipArchiveTests.cpp \
                    ../OgreMain/src/BitwiseTests.cpp \
                    ../OgreMain/src/PixelFormatTests.cpp \
                    ../OgreMain/src/RadixSort.cpp \
										../OgreMain/src/RenderSystemCapabilitiesTests.cpp
										../OgreMain/src/UseCustomCapabilitiesTest.cpp

TestSuite_LDFLAGS = -L$(top_builddir)/OgreMain/src $(CPPUNIT_LIBS)
TestSuite_LDADD = -lOgreMain
