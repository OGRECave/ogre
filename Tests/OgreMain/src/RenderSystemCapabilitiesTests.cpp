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
#include "OgreLogManager.h"
#include "OgreConfigFile.h"
#include "OgreFileSystemLayer.h"

#include <fstream>
#include <algorithm>

// Register the test suite

//--------------------------------------------------------------------------
void RenderSystemCapabilitiesTests::SetUp()
{    
    using namespace Ogre;

    // We need to be able to create FileSystem archives to load .rendercaps
    mFileSystemArchiveFactory = OGRE_NEW FileSystemArchiveFactory();

    mArchiveManager = OGRE_NEW ArchiveManager();
    ArchiveManager::getSingleton().addArchiveFactory(mFileSystemArchiveFactory);

    mRenderSystemCapabilitiesManager = OGRE_NEW RenderSystemCapabilitiesManager();

    Ogre::ConfigFile cf;
    cf.load(Ogre::FileSystemLayer(OGRE_VERSION_NAME).getConfigFilePath("resources.cfg"));
    Ogre::String testPath = cf.getSettings("Tests").begin()->second+"/CustomCapabilities";

    // Actual parsing happens here. The following test methods confirm parse results only.
    mRenderSystemCapabilitiesManager->parseCapabilitiesFromArchive(testPath, "FileSystem", true);
}
//--------------------------------------------------------------------------
void RenderSystemCapabilitiesTests::TearDown()
{
    OGRE_DELETE mRenderSystemCapabilitiesManager;
    OGRE_DELETE mArchiveManager;
    OGRE_DELETE mFileSystemArchiveFactory;
}
//--------------------------------------------------------------------------
TEST_F(RenderSystemCapabilitiesTests,IsShaderProfileSupported)
{
    // create a new RSC
    Ogre::RenderSystemCapabilities rsc;

    // check that no shader profile is supported
    EXPECT_TRUE(!rsc.isShaderProfileSupported("vs_1"));
    EXPECT_TRUE(!rsc.isShaderProfileSupported("ps_1_1"));
    EXPECT_TRUE(!rsc.isShaderProfileSupported("fp1"));

    rsc.addShaderProfile("vs_1");
    rsc.addShaderProfile("fp1");

    // check that the added shader profiles are supported
    EXPECT_TRUE(rsc.isShaderProfileSupported("vs_1"));
    EXPECT_TRUE(rsc.isShaderProfileSupported("fp1"));


    // check that non added profile is not supported
    EXPECT_TRUE(!rsc.isShaderProfileSupported("ps_1_1"));


    // check that empty string is not supported
    EXPECT_TRUE(!rsc.isShaderProfileSupported(""));
}
//--------------------------------------------------------------------------
TEST_F(RenderSystemCapabilitiesTests,HasCapability)
{
    RenderSystemCapabilities rsc;

    // check that no caps (from 2 categories) are supported
    EXPECT_TRUE(!rsc.hasCapability(RSC_TWO_SIDED_STENCIL));
    EXPECT_TRUE(!rsc.hasCapability(RSC_MIPMAP_LOD_BIAS));
    EXPECT_TRUE(!rsc.hasCapability(RSC_TEXTURE_COMPRESSION));
    EXPECT_TRUE(!rsc.hasCapability(RSC_TEXTURE_COMPRESSION_VTC));
    EXPECT_TRUE(!rsc.hasCapability(RSC_PBUFFER));

    // add support for few caps from each category
    rsc.setCapability(RSC_TEXTURE_COMPRESSION);

    // check that the newly set caps are supported
    EXPECT_TRUE(rsc.hasCapability(RSC_TEXTURE_COMPRESSION));

    // check that the non-set caps are NOT supported
    EXPECT_TRUE(!rsc.hasCapability(RSC_TWO_SIDED_STENCIL));
    EXPECT_TRUE(!rsc.hasCapability(RSC_MIPMAP_LOD_BIAS));
    EXPECT_TRUE(!rsc.hasCapability(RSC_TEXTURE_COMPRESSION_VTC));
    EXPECT_TRUE(!rsc.hasCapability(RSC_PBUFFER));
}
//--------------------------------------------------------------------------
TEST_F(RenderSystemCapabilitiesTests,SerializeBlank)
{
    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities("TestCaps Blank");

    // if we have a non-NULL it's good enough
    EXPECT_TRUE(rsc != 0);
}
//--------------------------------------------------------------------------
TEST_F(RenderSystemCapabilitiesTests,SerializeEnumCapability)
{
    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities("TestCaps enum Capabilities");

    // confirm that RSC was loaded
    EXPECT_TRUE(rsc != 0);

    // confirm that the contents are the same as in .rendercaps file
    EXPECT_TRUE(rsc->hasCapability(RSC_AUTOMIPMAP_COMPRESSED));
}
//--------------------------------------------------------------------------
TEST_F(RenderSystemCapabilitiesTests,SerializeStringCapability)
{
    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities("TestCaps set String");

    // confirm that RSC was loaded
    EXPECT_TRUE(rsc != 0);

    EXPECT_TRUE(rsc->isShaderProfileSupported("vs99"));
}
//--------------------------------------------------------------------------
TEST_F(RenderSystemCapabilitiesTests,SerializeBoolCapability)
{
    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rscTrue = rscManager->loadParsedCapabilities("TestCaps set bool (true)");
    RenderSystemCapabilities* rscFalse = rscManager->loadParsedCapabilities("TestCaps set bool (false)");

    // confirm that RSC was loaded
    EXPECT_TRUE(rscTrue != 0);
    EXPECT_TRUE(rscFalse != 0);

    EXPECT_TRUE(rscTrue->getVertexTextureUnitsShared() == true);
    EXPECT_TRUE(rscFalse->getVertexTextureUnitsShared() == false);
}
//--------------------------------------------------------------------------
TEST_F(RenderSystemCapabilitiesTests,SerializeIntCapability)
{
    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities("TestCaps set int");

    // confirm that RSC was loaded
    EXPECT_TRUE(rsc != 0);

    // TODO: why no get?
    EXPECT_TRUE(rsc->getNumMultiRenderTargets() == 99);
}
//--------------------------------------------------------------------------
TEST_F(RenderSystemCapabilitiesTests,SerializeRealCapability)
{
    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities("TestCaps set Real");

    // confirm that RSC was loaded
    EXPECT_TRUE(rsc != 0);

    EXPECT_TRUE(rsc->getMaxPointSize() == 99.5);
}
//--------------------------------------------------------------------------
TEST_F(RenderSystemCapabilitiesTests,SerializeShaderCapability)
{
    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities("TestCaps addShaderProfile");

    // confirm that RSC was loaded
    EXPECT_TRUE(rsc != 0);

    EXPECT_TRUE(rsc->isShaderProfileSupported("vp1"));
    EXPECT_TRUE(rsc->isShaderProfileSupported("vs_1_1"));
    EXPECT_TRUE(rsc->isShaderProfileSupported("ps_99"));
}
//--------------------------------------------------------------------------
TEST_F(RenderSystemCapabilitiesTests,WriteSimpleCapabilities)
{
    using namespace Ogre;
    using namespace std;
    String name = "simple caps";
    String filename = "simpleCapsTest.rendercaps";

    // set up caps of every type
    RenderSystemCapabilitiesSerializer serializer;
    RenderSystemCapabilities caps;
    caps.setMaxPointSize(10.5);
    caps.addShaderProfile("vs999");
    caps.addShaderProfile("sp999");
    caps.setVertexTextureUnitsShared(true);

    // write them to file
    serializer.writeScript(&caps, name, filename);

    // read them back
    ifstream capsfile(filename.c_str());
    char buff[255];

    capsfile.getline(buff, 255);
    EXPECT_EQ(String("render_system_capabilities \"") + name + "\"", String(buff));

    capsfile.getline(buff, 255);
    EXPECT_EQ(String("{"), String(buff));

    // scan every line and find the set capabilities it them
    std::vector <String> lines;
    while(capsfile.good())
    {
        capsfile.getline(buff, 255);
        lines.push_back(String(buff));
    }

    // check that the file is closed nicely
    String closeBracket = *(lines.end() - 2);
    EXPECT_EQ(String("}"), closeBracket);
    EXPECT_EQ(String(""), lines.back());

    // check that all the set caps are there
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tmax_point_size 10.5") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tshader_profile sp999") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tvertex_texture_units_shared true") != lines.end());
}
//--------------------------------------------------------------------------
TEST_F(RenderSystemCapabilitiesTests,WriteAllFalseCapabilities)
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
    EXPECT_EQ(String("render_system_capabilities \"") + name + "\"", String(buff));

    capsfile.getline(buff, 255);
    EXPECT_EQ(String("{"), String(buff));

    // scan every line and find the set capabilities it them
    std::vector <String> lines;
    while(capsfile.good())
    {
        capsfile.getline(buff, 255);
        lines.push_back(String(buff));
    }

      // check that the file is closed nicely
    String closeBracket = *(lines.end() - 2);
    EXPECT_EQ(String("}"), closeBracket);
    EXPECT_EQ(String(""), lines.back());

    // confirm every caps
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tautomipmap_compressed false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tanisotropy false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tdot3 false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\thwstencil false") != lines.end());

    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tvertex_program false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tfragment_program false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tscissor_test false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\ttwo_sided_stencil false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tstencil_wrap false") != lines.end());

    EXPECT_TRUE(find(lines.begin(), lines.end(), "\thwocclusion false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tuser_clip_planes false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tvertex_format_ubyte4 false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tinfinite_far_plane false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\thwrender_to_texture false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\ttexture_float false") != lines.end());

    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tnon_power_of_2_textures false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\ttexture_3d false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tpoint_sprites false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tpoint_extended_parameters false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tvertex_texture_fetch false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tmipmap_lod_bias false") != lines.end());

    EXPECT_TRUE(find(lines.begin(), lines.end(), "\ttexture_compression false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\ttexture_compression_dxt false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\ttexture_compression_vtc false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\ttexture_compression_pvrtc false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\ttexture_compression_bc4_bc5 false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\ttexture_compression_bc6h_bc7 false") != lines.end());

    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tpbuffer false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tperstageconstant false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tseparate_shader_objects false") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tvao false") != lines.end());

    // bool caps
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tvertex_texture_units_shared false") != lines.end());
}
//--------------------------------------------------------------------------
TEST_F(RenderSystemCapabilitiesTests,WriteAllTrueCapabilities)
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

    caps.setCapability(RSC_AUTOMIPMAP_COMPRESSED);
    caps.setCapability(RSC_ANISOTROPY);
    caps.setCapability(RSC_HWSTENCIL);

    caps.setCapability(RSC_TWO_SIDED_STENCIL);
    caps.setCapability(RSC_STENCIL_WRAP);

    caps.setCapability(RSC_HWOCCLUSION);
    caps.setCapability(RSC_USER_CLIP_PLANES);
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
    EXPECT_EQ(String("render_system_capabilities \"") + name + "\"", String(buff));

    capsfile.getline(buff, 255);
    EXPECT_EQ(String("{"), String(buff));

    // scan every line and find the set capabilities it them
    std::vector <String> lines;
    while(capsfile.good())
    {
        capsfile.getline(buff, 255);
        lines.push_back(String(buff));
    }

    // check that the file is closed nicely
    String closeBracket = *(lines.end() - 2);
    EXPECT_EQ(String("}"), closeBracket);
    EXPECT_EQ(String(""), lines.back());

    // confirm all caps
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tautomipmap_compressed true") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tanisotropy true") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\thwstencil true") != lines.end());

    EXPECT_TRUE(find(lines.begin(), lines.end(), "\ttwo_sided_stencil true") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tstencil_wrap true") != lines.end());

    EXPECT_TRUE(find(lines.begin(), lines.end(), "\thwocclusion true") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tuser_clip_planes true") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\thwrender_to_texture true") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\ttexture_float true") != lines.end());

    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tnon_power_of_2_textures true") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\ttexture_3d true") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tpoint_sprites true") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tpoint_extended_parameters true") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tvertex_texture_fetch true") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tmipmap_lod_bias true") != lines.end());

    EXPECT_TRUE(find(lines.begin(), lines.end(), "\ttexture_compression true") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\ttexture_compression_dxt true") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\ttexture_compression_vtc true") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\ttexture_compression_pvrtc true") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\ttexture_compression_bc4_bc5 true") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\ttexture_compression_bc6h_bc7 true") != lines.end());

    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tpbuffer true") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tperstageconstant true") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tseparate_shader_objects true") != lines.end());
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tvao true") != lines.end());

    // bool caps
    EXPECT_TRUE(find(lines.begin(), lines.end(), "\tvertex_texture_units_shared true") != lines.end());
}
//--------------------------------------------------------------------------
TEST_F(RenderSystemCapabilitiesTests,WriteAndReadComplexCapabilities)
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

    caps.setCapability(RSC_HWSTENCIL);
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

    caps.setNumTextureUnits(22);
    caps.setStencilBufferBitDepth(20001);
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
    EXPECT_TRUE(rsc != 0);

    // create a reference, so that were are working with two refs
    RenderSystemCapabilities& caps2 = *rsc;

    EXPECT_EQ(caps.hasCapability(RSC_ANISOTROPY), caps2.hasCapability(RSC_ANISOTROPY));
    EXPECT_EQ(caps.hasCapability(RSC_HWSTENCIL), caps2.hasCapability(RSC_HWSTENCIL));

    EXPECT_EQ(caps.hasCapability(RSC_TWO_SIDED_STENCIL), caps2.hasCapability(RSC_TWO_SIDED_STENCIL));
    EXPECT_EQ(caps.hasCapability(RSC_STENCIL_WRAP), caps2.hasCapability(RSC_STENCIL_WRAP));

    EXPECT_EQ(caps.hasCapability(RSC_HWOCCLUSION), caps2.hasCapability(RSC_HWOCCLUSION));
    EXPECT_EQ(caps.hasCapability(RSC_USER_CLIP_PLANES), caps2.hasCapability(RSC_USER_CLIP_PLANES));
    EXPECT_EQ(caps.hasCapability(RSC_VERTEX_FORMAT_UBYTE4), caps2.hasCapability(RSC_VERTEX_FORMAT_UBYTE4));
    EXPECT_EQ(caps.hasCapability(RSC_HWRENDER_TO_TEXTURE), caps2.hasCapability(RSC_HWRENDER_TO_TEXTURE));
    EXPECT_EQ(caps.hasCapability(RSC_TEXTURE_FLOAT), caps2.hasCapability(RSC_TEXTURE_FLOAT));

    EXPECT_EQ(caps.hasCapability(RSC_NON_POWER_OF_2_TEXTURES), caps2.hasCapability(RSC_NON_POWER_OF_2_TEXTURES));
    EXPECT_EQ(caps.hasCapability(RSC_TEXTURE_3D), caps2.hasCapability(RSC_TEXTURE_3D));
    EXPECT_EQ(caps.hasCapability(RSC_POINT_SPRITES), caps2.hasCapability(RSC_POINT_SPRITES));
    EXPECT_EQ(caps.hasCapability(RSC_POINT_EXTENDED_PARAMETERS), caps2.hasCapability(RSC_POINT_EXTENDED_PARAMETERS));
    EXPECT_EQ(caps.hasCapability(RSC_VERTEX_TEXTURE_FETCH), caps2.hasCapability(RSC_VERTEX_TEXTURE_FETCH));
    EXPECT_EQ(caps.hasCapability(RSC_MIPMAP_LOD_BIAS), caps2.hasCapability(RSC_MIPMAP_LOD_BIAS));

    EXPECT_EQ(caps.hasCapability(RSC_TEXTURE_COMPRESSION), caps2.hasCapability(RSC_TEXTURE_COMPRESSION));
    EXPECT_EQ(caps.hasCapability(RSC_TEXTURE_COMPRESSION_DXT), caps2.hasCapability(RSC_TEXTURE_COMPRESSION_DXT));
    EXPECT_EQ(caps.hasCapability(RSC_TEXTURE_COMPRESSION_VTC), caps2.hasCapability(RSC_TEXTURE_COMPRESSION_VTC));
    EXPECT_EQ(caps.hasCapability(RSC_TEXTURE_COMPRESSION_PVRTC), caps2.hasCapability(RSC_TEXTURE_COMPRESSION_PVRTC));
    EXPECT_EQ(caps.hasCapability(RSC_TEXTURE_COMPRESSION_BC4_BC5), caps2.hasCapability(RSC_TEXTURE_COMPRESSION_BC4_BC5));
    EXPECT_EQ(caps.hasCapability(RSC_TEXTURE_COMPRESSION_BC6H_BC7), caps2.hasCapability(RSC_TEXTURE_COMPRESSION_BC6H_BC7));

    EXPECT_EQ(caps.hasCapability(RSC_PBUFFER), caps2.hasCapability(RSC_PBUFFER));
    EXPECT_EQ(caps.hasCapability(RSC_PERSTAGECONSTANT), caps2.hasCapability(RSC_PERSTAGECONSTANT));
    EXPECT_EQ(caps.hasCapability(RSC_SEPARATE_SHADER_OBJECTS), caps2.hasCapability(RSC_SEPARATE_SHADER_OBJECTS));
    EXPECT_EQ(caps.hasCapability(RSC_VAO), caps2.hasCapability(RSC_VAO));

    EXPECT_EQ(caps.getNumTextureUnits(), caps2.getNumTextureUnits());
    EXPECT_EQ(caps.getStencilBufferBitDepth(), caps2.getStencilBufferBitDepth());
    EXPECT_EQ(caps.getNumMultiRenderTargets(), caps2.getNumMultiRenderTargets());

    EXPECT_EQ(caps.getVertexProgramConstantFloatCount(), caps2.getVertexProgramConstantFloatCount());
    EXPECT_EQ(caps.getVertexProgramConstantIntCount(), caps2.getVertexProgramConstantIntCount());
    EXPECT_EQ(caps.getVertexProgramConstantBoolCount(), caps2.getVertexProgramConstantBoolCount());

    EXPECT_EQ(caps.getFragmentProgramConstantFloatCount(), caps2.getFragmentProgramConstantFloatCount());
    EXPECT_EQ(caps.getFragmentProgramConstantIntCount(), caps2.getFragmentProgramConstantIntCount());
    EXPECT_EQ(caps.getFragmentProgramConstantBoolCount(), caps2.getFragmentProgramConstantBoolCount());

    EXPECT_EQ(caps.getMaxPointSize(), caps2.getMaxPointSize());
    EXPECT_EQ(caps.getNonPOW2TexturesLimited(), caps2.getNonPOW2TexturesLimited());
    EXPECT_EQ(caps.getVertexTextureUnitsShared(), caps2.getVertexTextureUnitsShared());
    
    // test versions
    EXPECT_EQ(caps.getDriverVersion().major, caps2.getDriverVersion().major);
    EXPECT_EQ(caps.getDriverVersion().minor, caps2.getDriverVersion().minor);
    EXPECT_EQ(caps.getDriverVersion().release, caps2.getDriverVersion().release);
    EXPECT_EQ(0, caps2.getDriverVersion().build);

    dataStreamPtr.reset();
}
//--------------------------------------------------------------------------

