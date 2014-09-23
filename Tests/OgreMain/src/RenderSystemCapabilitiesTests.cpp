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

#include "RenderSystemCapabilitiesTests.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreRenderSystemCapabilitiesManager.h"
#include "OgreStringConverter.h"
#include "OgreRenderSystemCapabilitiesSerializer.h"
#include "OgreArchiveManager.h"

#include "UnitTestSuite.h"

#include <fstream>
#include <algorithm>

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
#include "macUtils.h"
#endif

// Register the test suite
CPPUNIT_TEST_SUITE_REGISTRATION(RenderSystemCapabilitiesTests);

//--------------------------------------------------------------------------
void RenderSystemCapabilitiesTests::setUp()
{
    UnitTestSuite::getSingletonPtr()->startTestSetup(__FUNCTION__);
    
    using namespace Ogre;

    // We need to be able to create FileSystem archives to load .rendercaps
    mFileSystemArchiveFactory = OGRE_NEW FileSystemArchiveFactory();

    mArchiveManager = OGRE_NEW ArchiveManager();
    ArchiveManager::getSingleton().addArchiveFactory(mFileSystemArchiveFactory);

    mRenderSystemCapabilitiesManager = OGRE_NEW RenderSystemCapabilitiesManager();

    // Actual parsing happens here. The following test methods confirm parse results only.
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
    mRenderSystemCapabilitiesManager->parseCapabilitiesFromArchive(macBundlePath() + "/Contents/Resources/Media/CustomCapabilities", "FileSystem", true);
#else
    mRenderSystemCapabilitiesManager->parseCapabilitiesFromArchive("../../Tests/Media/CustomCapabilities", "FileSystem", true);
#endif
}
//--------------------------------------------------------------------------
void RenderSystemCapabilitiesTests::tearDown()
{
    OGRE_DELETE mRenderSystemCapabilitiesManager;
    OGRE_DELETE mArchiveManager;
    OGRE_DELETE mFileSystemArchiveFactory;
}
//--------------------------------------------------------------------------
void RenderSystemCapabilitiesTests::testIsShaderProfileSupported(void)
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    // create a new RSC
    Ogre::RenderSystemCapabilities rsc;

    // check that no shader profile is supported
    CPPUNIT_ASSERT(!rsc.isShaderProfileSupported("vs_1"));
    CPPUNIT_ASSERT(!rsc.isShaderProfileSupported("ps_1_1"));
    CPPUNIT_ASSERT(!rsc.isShaderProfileSupported("fp1"));

    rsc.addShaderProfile("vs_1");
    rsc.addShaderProfile("fp1");

    // check that the added shader profiles are supported
    CPPUNIT_ASSERT(rsc.isShaderProfileSupported("vs_1"));
    CPPUNIT_ASSERT(rsc.isShaderProfileSupported("fp1"));


    // check that non added profile is not supported
    CPPUNIT_ASSERT(!rsc.isShaderProfileSupported("ps_1_1"));


    // check that empty string is not supported
    CPPUNIT_ASSERT(!rsc.isShaderProfileSupported(""));
}
//--------------------------------------------------------------------------
void RenderSystemCapabilitiesTests::testHasCapability()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    RenderSystemCapabilities rsc;

    // check that no caps (from 2 categories) are supported
    CPPUNIT_ASSERT(!rsc.hasCapability(RSC_AUTOMIPMAP));
    CPPUNIT_ASSERT(!rsc.hasCapability(RSC_BLENDING));
    CPPUNIT_ASSERT(!rsc.hasCapability(RSC_FRAGMENT_PROGRAM));
    CPPUNIT_ASSERT(!rsc.hasCapability(RSC_TWO_SIDED_STENCIL));
    CPPUNIT_ASSERT(!rsc.hasCapability(RSC_MIPMAP_LOD_BIAS));
    CPPUNIT_ASSERT(!rsc.hasCapability(RSC_TEXTURE_COMPRESSION));
    CPPUNIT_ASSERT(!rsc.hasCapability(RSC_TEXTURE_COMPRESSION_VTC));
    CPPUNIT_ASSERT(!rsc.hasCapability(RSC_FBO_ATI));
    CPPUNIT_ASSERT(!rsc.hasCapability(RSC_PBUFFER));

    // add support for few caps from each category
    rsc.setCapability(RSC_AUTOMIPMAP);
    rsc.setCapability(RSC_FRAGMENT_PROGRAM);
    rsc.setCapability(RSC_TEXTURE_COMPRESSION);
    rsc.setCapability(RSC_FBO_ATI);

    // check that the newly set caps are supported
    CPPUNIT_ASSERT(rsc.hasCapability(RSC_AUTOMIPMAP));
    CPPUNIT_ASSERT(rsc.hasCapability(RSC_FRAGMENT_PROGRAM));
    CPPUNIT_ASSERT(rsc.hasCapability(RSC_TEXTURE_COMPRESSION));
    CPPUNIT_ASSERT(rsc.hasCapability(RSC_FBO_ATI));

    // check that the non-set caps are NOT supported
    CPPUNIT_ASSERT(!rsc.hasCapability(RSC_BLENDING));
    CPPUNIT_ASSERT(!rsc.hasCapability(RSC_TWO_SIDED_STENCIL));
    CPPUNIT_ASSERT(!rsc.hasCapability(RSC_MIPMAP_LOD_BIAS));
    CPPUNIT_ASSERT(!rsc.hasCapability(RSC_TEXTURE_COMPRESSION_VTC));
    CPPUNIT_ASSERT(!rsc.hasCapability(RSC_PBUFFER));
}
//--------------------------------------------------------------------------
void RenderSystemCapabilitiesTests::testSerializeBlank()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities("TestCaps Blank");

    // if we have a non-NULL it's good enough
    CPPUNIT_ASSERT(rsc != 0);
}
//--------------------------------------------------------------------------
void RenderSystemCapabilitiesTests::testSerializeEnumCapability()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities("TestCaps enum Capabilities");

    // confirm that RSC was loaded
    CPPUNIT_ASSERT(rsc != 0);

    // confirm that the contents are the same as in .rendercaps file
    CPPUNIT_ASSERT(rsc->hasCapability(RSC_AUTOMIPMAP));
    CPPUNIT_ASSERT(rsc->hasCapability(RSC_FBO_ARB));
}
//--------------------------------------------------------------------------
void RenderSystemCapabilitiesTests::testSerializeStringCapability()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities("TestCaps set String");

    // confirm that RSC was loaded
    CPPUNIT_ASSERT(rsc != 0);

    CPPUNIT_ASSERT(rsc->isShaderProfileSupported("vs99"));
}
//--------------------------------------------------------------------------
void RenderSystemCapabilitiesTests::testSerializeBoolCapability()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rscTrue = rscManager->loadParsedCapabilities("TestCaps set bool (true)");
    RenderSystemCapabilities* rscFalse = rscManager->loadParsedCapabilities("TestCaps set bool (false)");

    // confirm that RSC was loaded
    CPPUNIT_ASSERT(rscTrue != 0);
    CPPUNIT_ASSERT(rscFalse != 0);

    CPPUNIT_ASSERT(rscTrue->getVertexTextureUnitsShared() == true);
    CPPUNIT_ASSERT(rscFalse->getVertexTextureUnitsShared() == false);
}
//--------------------------------------------------------------------------
void RenderSystemCapabilitiesTests::testSerializeIntCapability()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities("TestCaps set int");

    // confirm that RSC was loaded
    CPPUNIT_ASSERT(rsc != 0);

    // TODO: why no get?
    CPPUNIT_ASSERT(rsc->getNumMultiRenderTargets() == 99);
}
//--------------------------------------------------------------------------
void RenderSystemCapabilitiesTests::testSerializeRealCapability()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities("TestCaps set Real");

    // confirm that RSC was loaded
    CPPUNIT_ASSERT(rsc != 0);

    CPPUNIT_ASSERT(rsc->getMaxPointSize() == 99.5);
}
//--------------------------------------------------------------------------
void RenderSystemCapabilitiesTests::testSerializeShaderCapability()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities("TestCaps addShaderProfile");

    // confirm that RSC was loaded
    CPPUNIT_ASSERT(rsc != 0);

    CPPUNIT_ASSERT(rsc->isShaderProfileSupported("vp1"));
    CPPUNIT_ASSERT(rsc->isShaderProfileSupported("vs_1_1"));
    CPPUNIT_ASSERT(rsc->isShaderProfileSupported("ps_99"));
}
//--------------------------------------------------------------------------
void RenderSystemCapabilitiesTests::testWriteSimpleCapabilities()
{
    using namespace Ogre;
	using namespace std;

    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    String name = "simple caps";
    String filename = "simpleCapsTest.rendercaps";

    // set up caps of every type
    RenderSystemCapabilitiesSerializer serializer;
    RenderSystemCapabilities caps;
    caps.setCapability(RSC_AUTOMIPMAP);
    caps.setMaxPointSize(10.5);
    caps.addShaderProfile("vs999");
    caps.addShaderProfile("sp999");
    caps.setVertexTextureUnitsShared(true);
    caps.setNumWorldMatrices(777);

    // write them to file
    serializer.writeScript(&caps, name, filename);

    // read them back
    ifstream capsfile(filename.c_str());
    char buff[255];

    capsfile.getline(buff, 255);
    CPPUNIT_ASSERT_EQUAL(String("render_system_capabilities \"") + name + "\"", String(buff));

    capsfile.getline(buff, 255);
    CPPUNIT_ASSERT_EQUAL(String("{"), String(buff));

    // scan every line and find the set capabilities it them
	std::vector <String> lines;
    while(capsfile.good())
    {
        capsfile.getline(buff, 255);
        lines.push_back(String(buff));
    }

    // check that the file is closed nicely
    String closeBracket = *(lines.end() - 2);
    CPPUNIT_ASSERT_EQUAL(String("}"), closeBracket);
    CPPUNIT_ASSERT_EQUAL(String(""), lines.back());

    // check that all the set caps are there
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tautomipmap true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tmax_point_size 10.5") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tshader_profile sp999") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tvertex_texture_units_shared true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tnum_world_matrices 777") != lines.end());
}
//--------------------------------------------------------------------------
void RenderSystemCapabilitiesTests::testWriteAllFalseCapabilities()
{
    using namespace Ogre;
    using namespace std;

    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    String name = "all false caps";
    String filename = "allFalseCapsTest.rendercaps";

    // set up caps of every type
    RenderSystemCapabilitiesSerializer serializer;
    RenderSystemCapabilities caps;

    // all caps are false by default
    caps.setVertexTextureUnitsShared(false);

    // write them to file
    serializer.writeScript(&caps, name, filename);

    // read them back
    ifstream capsfile(filename.c_str());
    char buff[255];

    capsfile.getline(buff, 255);
    CPPUNIT_ASSERT_EQUAL(String("render_system_capabilities \"") + name + "\"", String(buff));

    capsfile.getline(buff, 255);
    CPPUNIT_ASSERT_EQUAL(String("{"), String(buff));

    // scan every line and find the set capabilities it them
    std::vector <String> lines;
    while(capsfile.good())
    {
        capsfile.getline(buff, 255);
        lines.push_back(String(buff));
    }

      // check that the file is closed nicely
    String closeBracket = *(lines.end() - 2);
    CPPUNIT_ASSERT_EQUAL(String("}"), closeBracket);
    CPPUNIT_ASSERT_EQUAL(String(""), lines.back());

    // confirm every caps
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tautomipmap false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tblending false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tanisotropy false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tdot3 false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tcubemapping false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\thwstencil false") != lines.end());

    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tvbo false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tvertex_program false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tfragment_program false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tscissor_test false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\ttwo_sided_stencil false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tstencil_wrap false") != lines.end());

    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\thwocclusion false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tuser_clip_planes false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tvertex_format_ubyte4 false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tinfinite_far_plane false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\thwrender_to_texture false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\ttexture_float false") != lines.end());

    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tnon_power_of_2_textures false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\ttexture_3d false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tpoint_sprites false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tpoint_extended_parameters false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tvertex_texture_fetch false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tmipmap_lod_bias false") != lines.end());

    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\ttexture_compression false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\ttexture_compression_dxt false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\ttexture_compression_vtc false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\ttexture_compression_pvrtc false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\ttexture_compression_bc4_bc5 false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\ttexture_compression_bc6h_bc7 false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tfbo false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tfbo_arb false") != lines.end());

    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tfbo_ati false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tpbuffer false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tperstageconstant false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tseparate_shader_objects false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tvao false") != lines.end());

    // bool caps
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tvertex_texture_units_shared false") != lines.end());
}
//--------------------------------------------------------------------------
void RenderSystemCapabilitiesTests::testWriteAllTrueCapabilities()
{
    using namespace Ogre;
    using namespace std;

    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    String name = "all false caps";
    String filename = "allFalseCapsTest.rendercaps";

    // set up caps of every type
    RenderSystemCapabilitiesSerializer serializer;
    RenderSystemCapabilities caps;

    // set all caps
    caps.setVertexTextureUnitsShared(true);

    caps.setCapability(RSC_AUTOMIPMAP);
    caps.setCapability(RSC_BLENDING);
    caps.setCapability(RSC_ANISOTROPY);
    caps.setCapability(RSC_DOT3);
    caps.setCapability(RSC_CUBEMAPPING);
    caps.setCapability(RSC_HWSTENCIL);

    caps.setCapability(RSC_VBO);
    caps.setCapability(RSC_VERTEX_PROGRAM);
    caps.setCapability(RSC_FRAGMENT_PROGRAM);
    caps.setCapability(RSC_SCISSOR_TEST);
    caps.setCapability(RSC_TWO_SIDED_STENCIL);
    caps.setCapability(RSC_STENCIL_WRAP);

    caps.setCapability(RSC_HWOCCLUSION);
    caps.setCapability(RSC_USER_CLIP_PLANES);
    caps.setCapability(RSC_VERTEX_FORMAT_UBYTE4);
    caps.setCapability(RSC_INFINITE_FAR_PLANE);
    caps.setCapability(RSC_HWRENDER_TO_TEXTURE);
    caps.setCapability(RSC_TEXTURE_FLOAT);

    caps.setCapability(RSC_NON_POWER_OF_2_TEXTURES);
    caps.setCapability(RSC_TEXTURE_3D);
    caps.setCapability(RSC_POINT_SPRITES);
    caps.setCapability(RSC_POINT_EXTENDED_PARAMETERS);
    caps.setCapability(RSC_VERTEX_TEXTURE_FETCH);
    caps.setCapability(RSC_MIPMAP_LOD_BIAS);

    caps.setCapability(RSC_TEXTURE_COMPRESSION);
    caps.setCapability(RSC_TEXTURE_COMPRESSION_DXT);
    caps.setCapability(RSC_TEXTURE_COMPRESSION_VTC);
    caps.setCapability(RSC_TEXTURE_COMPRESSION_PVRTC);
    caps.setCapability(RSC_TEXTURE_COMPRESSION_BC4_BC5);
    caps.setCapability(RSC_TEXTURE_COMPRESSION_BC6H_BC7);
    caps.setCapability(RSC_FBO);
    caps.setCapability(RSC_FBO_ARB);

    caps.setCapability(RSC_FBO_ATI);
    caps.setCapability(RSC_PBUFFER);
    caps.setCapability(RSC_PERSTAGECONSTANT);
    caps.setCapability(RSC_SEPARATE_SHADER_OBJECTS);
    caps.setCapability(RSC_VAO);

    // write them to file
    serializer.writeScript(&caps, name, filename);

    // read them back
    ifstream capsfile(filename.c_str());
    char buff[255];

    capsfile.getline(buff, 255);
    CPPUNIT_ASSERT_EQUAL(String("render_system_capabilities \"") + name + "\"", String(buff));

    capsfile.getline(buff, 255);
    CPPUNIT_ASSERT_EQUAL(String("{"), String(buff));

    // scan every line and find the set capabilities it them
    std::vector <String> lines;
    while(capsfile.good())
    {
        capsfile.getline(buff, 255);
        lines.push_back(String(buff));
    }

    // check that the file is closed nicely
    String closeBracket = *(lines.end() - 2);
    CPPUNIT_ASSERT_EQUAL(String("}"), closeBracket);
    CPPUNIT_ASSERT_EQUAL(String(""), lines.back());

    // confirm all caps
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tautomipmap true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tblending true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tanisotropy true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tdot3 true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tcubemapping true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\thwstencil true") != lines.end());

    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tvbo true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tvertex_program true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tfragment_program true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tscissor_test true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\ttwo_sided_stencil true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tstencil_wrap true") != lines.end());

    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\thwocclusion true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tuser_clip_planes true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tvertex_format_ubyte4 true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tinfinite_far_plane true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\thwrender_to_texture true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\ttexture_float true") != lines.end());

    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tnon_power_of_2_textures true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\ttexture_3d true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tpoint_sprites true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tpoint_extended_parameters true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tvertex_texture_fetch true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tmipmap_lod_bias true") != lines.end());

    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\ttexture_compression true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\ttexture_compression_dxt true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\ttexture_compression_vtc true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\ttexture_compression_pvrtc true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\ttexture_compression_bc4_bc5 true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\ttexture_compression_bc6h_bc7 true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tfbo true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tfbo_arb true") != lines.end());

    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tfbo_ati true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tpbuffer true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tperstageconstant true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tseparate_shader_objects true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tvao true") != lines.end());

    // bool caps
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "\tvertex_texture_units_shared true") != lines.end());
}
//--------------------------------------------------------------------------
void RenderSystemCapabilitiesTests::testWriteAndReadComplexCapabilities()
{
    using namespace Ogre;
    using namespace std;

    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    String name = "complex caps";
    String filename = "complexCapsTest.rendercaps";

    // set up caps of every type
    RenderSystemCapabilitiesSerializer serializer;
    RenderSystemCapabilities caps;

    // set all caps
    caps.setVertexTextureUnitsShared(true);

    caps.setCapability(RSC_AUTOMIPMAP);
    caps.setCapability(RSC_DOT3);
    caps.setCapability(RSC_CUBEMAPPING);
    caps.setCapability(RSC_HWSTENCIL);
    caps.setCapability(RSC_VBO);
    caps.setCapability(RSC_FRAGMENT_PROGRAM);
    caps.setCapability(RSC_SCISSOR_TEST);
    caps.setCapability(RSC_TWO_SIDED_STENCIL);
    caps.setCapability(RSC_HWOCCLUSION);
    caps.setCapability(RSC_VERTEX_FORMAT_UBYTE4);
    caps.setCapability(RSC_HWRENDER_TO_TEXTURE);
    caps.setCapability(RSC_TEXTURE_FLOAT);
    caps.setCapability(RSC_NON_POWER_OF_2_TEXTURES);
    caps.setCapability(RSC_TEXTURE_3D);
    caps.setCapability(RSC_POINT_EXTENDED_PARAMETERS);
    caps.setCapability(RSC_MIPMAP_LOD_BIAS);
    caps.setCapability(RSC_TEXTURE_COMPRESSION);
    caps.setCapability(RSC_TEXTURE_COMPRESSION_DXT);
    caps.setCapability(RSC_TEXTURE_COMPRESSION_VTC);
    caps.setCapability(RSC_TEXTURE_COMPRESSION_PVRTC);
    caps.setCapability(RSC_TEXTURE_COMPRESSION_BC4_BC5);
    caps.setCapability(RSC_TEXTURE_COMPRESSION_BC6H_BC7);
    caps.setCapability(RSC_PERSTAGECONSTANT);
    caps.setCapability(RSC_SEPARATE_SHADER_OBJECTS);
    caps.setCapability(RSC_VAO);

    caps.setNumWorldMatrices(11);
    caps.setNumTextureUnits(22);
    caps.setStencilBufferBitDepth(20001);
    caps.setNumVertexBlendMatrices(33);
    caps.setNumMultiRenderTargets(23);

    caps.addShaderProfile("99foo100");

    // try out stranger names
    caps.addShaderProfile("..f(_)specialsymbolextravaganza!@#$%^&*_but_no_spaces");

    caps.setVertexProgramConstantFloatCount(1111);
    caps.setVertexProgramConstantIntCount(2222);
    caps.setVertexProgramConstantBoolCount(3333);

    caps.setFragmentProgramConstantFloatCount(4444);
    caps.setFragmentProgramConstantIntCount(5555);
    caps.setFragmentProgramConstantBoolCount(64000);

    caps.setMaxPointSize(123.75);
    caps.setNonPOW2TexturesLimited(true);
    caps.setVertexTextureUnitsShared(true);

	DriverVersion driverversion;
	driverversion.major = 11;
	driverversion.minor = 13;
	driverversion.release = 17;
	driverversion.build = 0;

	caps.setDriverVersion(driverversion);
    caps.setDeviceName("Dummy Device");
    caps.setRenderSystemName("Dummy RenderSystem");

    // write them to file
    serializer.writeScript(&caps, name, filename);

    FileStreamDataStream* fdatastream = new FileStreamDataStream(filename,
            OGRE_NEW_T(ifstream, MEMCATEGORY_GENERAL)(filename.c_str()));

    DataStreamPtr dataStreamPtr(fdatastream);

    // parsing does not return a raw RSC, but adds it to the Manager
    serializer.parseScript(dataStreamPtr);

    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities(name);
    // confirm that RSC was loaded
    CPPUNIT_ASSERT(rsc != 0);

    // create a reference, so that were are working with two refs
    RenderSystemCapabilities& caps2 = *rsc;

    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_AUTOMIPMAP), caps2.hasCapability(RSC_AUTOMIPMAP));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_BLENDING), caps2.hasCapability(RSC_BLENDING));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_ANISOTROPY), caps2.hasCapability(RSC_ANISOTROPY));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_DOT3), caps2.hasCapability(RSC_DOT3));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_CUBEMAPPING), caps2.hasCapability(RSC_CUBEMAPPING));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_HWSTENCIL), caps2.hasCapability(RSC_HWSTENCIL));

    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_VBO), caps2.hasCapability(RSC_VBO));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_VERTEX_PROGRAM), caps2.hasCapability(RSC_VERTEX_PROGRAM));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_FRAGMENT_PROGRAM), caps2.hasCapability(RSC_FRAGMENT_PROGRAM));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_SCISSOR_TEST), caps2.hasCapability(RSC_SCISSOR_TEST));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_TWO_SIDED_STENCIL), caps2.hasCapability(RSC_TWO_SIDED_STENCIL));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_STENCIL_WRAP), caps2.hasCapability(RSC_STENCIL_WRAP));

    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_HWOCCLUSION), caps2.hasCapability(RSC_HWOCCLUSION));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_USER_CLIP_PLANES), caps2.hasCapability(RSC_USER_CLIP_PLANES));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_VERTEX_FORMAT_UBYTE4), caps2.hasCapability(RSC_VERTEX_FORMAT_UBYTE4));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_INFINITE_FAR_PLANE), caps2.hasCapability(RSC_INFINITE_FAR_PLANE));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_HWRENDER_TO_TEXTURE), caps2.hasCapability(RSC_HWRENDER_TO_TEXTURE));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_TEXTURE_FLOAT), caps2.hasCapability(RSC_TEXTURE_FLOAT));

    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_NON_POWER_OF_2_TEXTURES), caps2.hasCapability(RSC_NON_POWER_OF_2_TEXTURES));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_TEXTURE_3D), caps2.hasCapability(RSC_TEXTURE_3D));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_POINT_SPRITES), caps2.hasCapability(RSC_POINT_SPRITES));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_POINT_EXTENDED_PARAMETERS), caps2.hasCapability(RSC_POINT_EXTENDED_PARAMETERS));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_VERTEX_TEXTURE_FETCH), caps2.hasCapability(RSC_VERTEX_TEXTURE_FETCH));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_MIPMAP_LOD_BIAS), caps2.hasCapability(RSC_MIPMAP_LOD_BIAS));

    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_TEXTURE_COMPRESSION), caps2.hasCapability(RSC_TEXTURE_COMPRESSION));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_TEXTURE_COMPRESSION_DXT), caps2.hasCapability(RSC_TEXTURE_COMPRESSION_DXT));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_TEXTURE_COMPRESSION_VTC), caps2.hasCapability(RSC_TEXTURE_COMPRESSION_VTC));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_TEXTURE_COMPRESSION_PVRTC), caps2.hasCapability(RSC_TEXTURE_COMPRESSION_PVRTC));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_TEXTURE_COMPRESSION_BC4_BC5), caps2.hasCapability(RSC_TEXTURE_COMPRESSION_BC4_BC5));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_TEXTURE_COMPRESSION_BC6H_BC7), caps2.hasCapability(RSC_TEXTURE_COMPRESSION_BC6H_BC7));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_FBO), caps2.hasCapability(RSC_FBO));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_FBO_ARB), caps2.hasCapability(RSC_FBO_ARB));

    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_FBO_ATI), caps2.hasCapability(RSC_FBO_ATI));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_PBUFFER), caps2.hasCapability(RSC_PBUFFER));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_PERSTAGECONSTANT), caps2.hasCapability(RSC_PERSTAGECONSTANT));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_SEPARATE_SHADER_OBJECTS), caps2.hasCapability(RSC_SEPARATE_SHADER_OBJECTS));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_VAO), caps2.hasCapability(RSC_VAO));

    CPPUNIT_ASSERT_EQUAL(caps.getNumWorldMatrices(), caps2.getNumWorldMatrices());
    CPPUNIT_ASSERT_EQUAL(caps.getNumTextureUnits(), caps2.getNumTextureUnits());
    CPPUNIT_ASSERT_EQUAL(caps.getStencilBufferBitDepth(), caps2.getStencilBufferBitDepth());
    CPPUNIT_ASSERT_EQUAL(caps.getNumVertexBlendMatrices(), caps2.getNumVertexBlendMatrices());
    CPPUNIT_ASSERT_EQUAL(caps.getNumMultiRenderTargets(), caps2.getNumMultiRenderTargets());

    CPPUNIT_ASSERT_EQUAL(caps.getVertexProgramConstantFloatCount(), caps2.getVertexProgramConstantFloatCount());
    CPPUNIT_ASSERT_EQUAL(caps.getVertexProgramConstantIntCount(), caps2.getVertexProgramConstantIntCount());
    CPPUNIT_ASSERT_EQUAL(caps.getVertexProgramConstantBoolCount(), caps2.getVertexProgramConstantBoolCount());

    CPPUNIT_ASSERT_EQUAL(caps.getFragmentProgramConstantFloatCount(), caps2.getFragmentProgramConstantFloatCount());
    CPPUNIT_ASSERT_EQUAL(caps.getFragmentProgramConstantIntCount(), caps2.getFragmentProgramConstantIntCount());
    CPPUNIT_ASSERT_EQUAL(caps.getFragmentProgramConstantBoolCount(), caps2.getFragmentProgramConstantBoolCount());

    CPPUNIT_ASSERT_EQUAL(caps.getMaxPointSize(), caps2.getMaxPointSize());
    CPPUNIT_ASSERT_EQUAL(caps.getNonPOW2TexturesLimited(), caps2.getNonPOW2TexturesLimited());
    CPPUNIT_ASSERT_EQUAL(caps.getVertexTextureUnitsShared(), caps2.getVertexTextureUnitsShared());
	
	// test versions
	CPPUNIT_ASSERT_EQUAL(caps.getDriverVersion().major, caps2.getDriverVersion().major);
	CPPUNIT_ASSERT_EQUAL(caps.getDriverVersion().minor, caps2.getDriverVersion().minor);
	CPPUNIT_ASSERT_EQUAL(caps.getDriverVersion().release, caps2.getDriverVersion().release);
	CPPUNIT_ASSERT_EQUAL(0, caps2.getDriverVersion().build);

    dataStreamPtr.setNull();
}
//--------------------------------------------------------------------------

