/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#include "RenderSystemCapabilitiesTests.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreRenderSystemCapabilitiesManager.h"
#include "OgreStringConverter.h"
#include "OgreRenderSystemCapabilitiesSerializer.h"

#include <fstream>
#include <algorithm>


// Regsiter the suite
CPPUNIT_TEST_SUITE_REGISTRATION( RenderSystemCapabilitiesTests );

void RenderSystemCapabilitiesTests::setUp()
{
    using namespace Ogre;

    // we need to be able to create FileSystem archives to load .rendercaps
    mFileSystemArchiveFactory = new FileSystemArchiveFactory();

    mArchiveManager = new ArchiveManager();
    ArchiveManager::getSingleton().addArchiveFactory( mFileSystemArchiveFactory );

    mRenderSystemCapabilitiesManager = new RenderSystemCapabilitiesManager();
    // actual parsing happens here. test methods confirm parse results only
    mRenderSystemCapabilitiesManager->parseCapabilitiesFromArchive("../../../Media/CustomCapabilities", "FileSystem", true);
}

void RenderSystemCapabilitiesTests::tearDown()
{
    delete mRenderSystemCapabilitiesManager;
    delete mArchiveManager;
    delete mFileSystemArchiveFactory;

}

void RenderSystemCapabilitiesTests::testIsShaderProfileSupported(void)
{
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

void RenderSystemCapabilitiesTests::testHasCapability()
{
    using namespace Ogre;

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

void RenderSystemCapabilitiesTests::testSerializeBlank()
{
    using namespace Ogre;

    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities("TestCaps Blank");

    // if we have a non-NULL it's good enough
    CPPUNIT_ASSERT(rsc != 0);
}

void RenderSystemCapabilitiesTests::testSerializeEnumCapability()
{
    using namespace Ogre;

    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities("TestCaps enum Capabilities");
    // confirm that RSC was loaded
    CPPUNIT_ASSERT(rsc != 0);

    // confirm that the contents are rhe same as in .rendercaps file
    CPPUNIT_ASSERT(rsc->hasCapability(RSC_AUTOMIPMAP));
    CPPUNIT_ASSERT(rsc->hasCapability(RSC_FBO_ARB));
}


void RenderSystemCapabilitiesTests::testSerializeStringCapability()
{
    using namespace Ogre;

    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities("TestCaps set String");
    // confirm that RSC was loaded
    CPPUNIT_ASSERT(rsc != 0);

    CPPUNIT_ASSERT(rsc->isShaderProfileSupported("vs99"));
}

void RenderSystemCapabilitiesTests::testSerializeBoolCapability()
{
    using namespace Ogre;

    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rscTrue = rscManager->loadParsedCapabilities("TestCaps set bool (true)");
    RenderSystemCapabilities* rscFalse = rscManager->loadParsedCapabilities("TestCaps set bool (false)");
    // confirm that RSC was loaded
    CPPUNIT_ASSERT(rscTrue != 0);
    CPPUNIT_ASSERT(rscFalse != 0);

    CPPUNIT_ASSERT(rscTrue->getVertexTextureUnitsShared() == true);
    CPPUNIT_ASSERT(rscFalse->getVertexTextureUnitsShared() == false);
}

void RenderSystemCapabilitiesTests::testSerializeIntCapability()
{
    using namespace Ogre;

    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities("TestCaps set int");
    // confirm that RSC was loaded
    CPPUNIT_ASSERT(rsc != 0);

    // TODO: why no get?
    CPPUNIT_ASSERT(rsc->getNumMultiRenderTargets() == 99);
}

void RenderSystemCapabilitiesTests::testSerializeRealCapability()
{
    using namespace Ogre;

    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities("TestCaps set Real");
    // confirm that RSC was loaded
    CPPUNIT_ASSERT(rsc != 0);

    CPPUNIT_ASSERT(rsc->getMaxPointSize() == 99.5);
}

void RenderSystemCapabilitiesTests::testSerializeShaderCapability()
{
    using namespace Ogre;

    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities("TestCaps addShaderProfile");
    // confirm that RSC was loaded
    CPPUNIT_ASSERT(rsc != 0);

    CPPUNIT_ASSERT(rsc->isShaderProfileSupported("vp1"));
    CPPUNIT_ASSERT(rsc->isShaderProfileSupported("vs_1_1"));
    CPPUNIT_ASSERT(rsc->isShaderProfileSupported("ps_99"));
}

void RenderSystemCapabilitiesTests::testWriteSimpleCapabilities()
{
    using namespace Ogre;
	using namespace std;

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
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "automipmap true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "max_point_size 10.5") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "max_vertex_program_version vs999") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "shader_profile sp999") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "vertex_texture_units_shared true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "num_world_matrices 777") != lines.end());
}

void RenderSystemCapabilitiesTests::testWriteAllFalseCapabilities()
{
    using namespace Ogre;
    using namespace std;

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
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "automipmap false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "blending false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "anisotropy false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "dot3 false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "cubemapping false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "hwstencil false") != lines.end());

    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "vbo false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "vertex_program false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "fragment_program false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "scissor_test false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "two_sided_stencil false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "stencil_wrap false") != lines.end());

    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "hwocclusion false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "user_clip_planes false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "vertex_format_ubyte4 false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "infinite_far_plane false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "hwrender_to_texture false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "texture_float false") != lines.end());

    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "non_power_of_2_textures false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "texture_3d false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "point_sprites false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "point_extended_parameters false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "vertex_texture_fetch false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "mipmap_lod_bias false") != lines.end());

    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "texture_compression false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "texture_compression_dxt false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "texture_compression_vtc false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "fbo false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "fbo_arb false") != lines.end());

    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "fbo_ati false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "pbuffer false") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "perstageconstant false") != lines.end());

    // bool caps
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "vertex_texture_units_shared false") != lines.end());

}

void RenderSystemCapabilitiesTests::testWriteAllTrueCapabilities()
{
    using namespace Ogre;
    using namespace std;

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
    caps.setCapability(RSC_FBO);
    caps.setCapability(RSC_FBO_ARB);

    caps.setCapability(RSC_FBO_ATI);
    caps.setCapability(RSC_PBUFFER);
    caps.setCapability(RSC_PERSTAGECONSTANT);


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
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "automipmap true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "blending true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "anisotropy true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "dot3 true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "cubemapping true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "hwstencil true") != lines.end());

    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "vbo true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "vertex_program true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "fragment_program true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "scissor_test true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "two_sided_stencil true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "stencil_wrap true") != lines.end());

    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "hwocclusion true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "user_clip_planes true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "vertex_format_ubyte4 true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "infinite_far_plane true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "hwrender_to_texture true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "texture_float true") != lines.end());

    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "non_power_of_2_textures true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "texture_3d true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "point_sprites true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "point_extended_parameters true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "vertex_texture_fetch true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "mipmap_lod_bias true") != lines.end());

    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "texture_compression true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "texture_compression_dxt true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "texture_compression_vtc true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "fbo true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "fbo_arb true") != lines.end());

    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "fbo_ati true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "pbuffer true") != lines.end());
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "perstageconstant true") != lines.end());

    // bool caps
    CPPUNIT_ASSERT(find(lines.begin(), lines.end(), "vertex_texture_units_shared true") != lines.end());

}

void RenderSystemCapabilitiesTests::testWriteAndReadComplexCapabilities()
{
    using namespace Ogre;
    using namespace std;

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
    caps.setCapability(RSC_PERSTAGECONSTANT);

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

    // create a reference, so that were're working with two refs
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
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_FBO), caps2.hasCapability(RSC_FBO));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_FBO_ARB), caps2.hasCapability(RSC_FBO_ARB));

    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_FBO_ATI), caps2.hasCapability(RSC_FBO_ATI));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_PBUFFER), caps2.hasCapability(RSC_PBUFFER));
    CPPUNIT_ASSERT_EQUAL(caps.hasCapability(RSC_PERSTAGECONSTANT), caps2.hasCapability(RSC_PERSTAGECONSTANT));

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


