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

#include "OgreMaterialManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreGpuProgramManager.h"
#include "MaterialScriptCompilerTests.h"
#include "OgreStringConverter.h"

#define ARRAYSIZE(array) (sizeof(array)/sizeof(array[0]))

// Regsiter the suite
CPPUNIT_TEST_SUITE_REGISTRATION( MaterialScriptCompilerTests );

void MaterialScriptCompilerTests::setUp()
{
}
void MaterialScriptCompilerTests::tearDown()
{
}

void MaterialScriptCompilerTests::testPositionToNextSymbol()
{
  struct test1result{
    const char character;
    const int line;
  };

  const String TestStr1 = "   \n\r  //c  \n\r// test\n\r  \t  c   - \n\r ,  e";
  const test1result test1results[] = {
    {'c', 4},
    {'-', 4},
    {',', 5},
    {'e', 5}
  };

  // first test: see if positionToNextSymbol can find a valid Symbol
  mSource = &TestStr1;
  mCharPos = 0;
  size_t resultID = 0;
  mCurrentLine = 1;
  mEndOfSource = mSource->length();
  while (positionToNextLexeme()) {
      CPPUNIT_ASSERT_MESSAGE( "test " + StringConverter::toString(resultID) + "  character found: " + (*mSource)[mCharPos] +
          "  Line:%d  : " + StringConverter::toString(mCurrentLine)
          , ((*mSource)[mCharPos] == test1results[resultID].character) && (mCurrentLine==test1results[resultID].line) );
    resultID++;
    mCharPos++;
  }

}

void MaterialScriptCompilerTests::testIsFloatValue(void)
{
  struct testfloatresult{
    const String teststr;
    const float fvalue;
    const size_t charsize;
    testfloatresult(const String& _teststr, const float _fvalue, const size_t _charsize)
        : teststr(_teststr)
        , fvalue(_fvalue)
        , charsize(_charsize)
        {};
  };

  testfloatresult testfloatresults[] = {
    testfloatresult("1 test", 1.0f, 1),
    testfloatresult("2.3f test", 2.3f, 3),
    testfloatresult("-0.5 test", -0.5f, 4),
    testfloatresult(" 23.6 test", 23.6f, 5),
    testfloatresult("  -0.021 test", -0.021f, 8),
    testfloatresult("12 test", 12.0f, 2),
    testfloatresult("3test", 3.0f, 1)
  };

  float fvalue = 0;
  size_t charsize = 0;
  mCharPos = 0;
  size_t testsize = ARRAYSIZE(testfloatresults);
  for(size_t resultID=0; resultID<testsize; resultID++)
  {
    mSource = &testfloatresults[resultID].teststr;
    isFloatValue(fvalue, charsize);
    CPPUNIT_ASSERT_MESSAGE( "test " + StringConverter::toString(resultID) + " value returned: " + StringConverter::toString(fvalue)
        , fvalue == testfloatresults[resultID].fvalue);
    CPPUNIT_ASSERT_MESSAGE( "test " + StringConverter::toString(resultID) + " char size returned: " + StringConverter::toString(charsize)
        , charsize == testfloatresults[resultID].charsize);
  }

}

void MaterialScriptCompilerTests::testIsLexemeMatch(void)
{
  const String TestStr = "material test";
  const String TestSymbols = "material";

  mSource = &TestStr;
  mCharPos = 0;
  CPPUNIT_ASSERT(isLexemeMatch(TestSymbols, false));
  CPPUNIT_ASSERT(isLexemeMatch(TestSymbols, true));
  mCharPos = 1;
  CPPUNIT_ASSERT(!isLexemeMatch(TestSymbols, false));
  CPPUNIT_ASSERT(!isLexemeMatch(TestSymbols, true));

}

void MaterialScriptCompilerTests::testCompileMaterialScript()
{
    Math* mth = new Math();
    if (!ResourceGroupManager::getSingletonPtr())
        new ResourceGroupManager();

    if (!MaterialManager::getSingletonPtr())
    {
        new MaterialManager();
        MaterialManager::getSingleton().initialise();
    }

    if (!HighLevelGpuProgramManager::getSingletonPtr())
        new HighLevelGpuProgramManager();

    //if (!GpuProgramManager::getSingletonPtr())
    //    new GpuProgramManager();


    const String simpleScript =
        "vertex_program boo cg { \n"
        "  source program.cg \n"
        "  entry_point main \n"
        "  profiles vs_1_1 arbvp1 \n"
        " } \n"
        "material test2 { \n"
        " receive_shadows on lod_distances 100 203.6 700 \n"
        " set_texture_alias foo-bar MyTexture.png \n"
        " transparency_casts_shadows on \n"
        " technique { scheme hdr lod_index 1 \n"
        " pass pass-1 { \n"
        "   ambient 0 1 0.5\n"
        "   ambient 0 1 0.5 1 \n"
        "   ambient vertexcolour \n"
        "   diffuse 0.5 1.0 0 0.5 \n"
        "   specular vertexcolour 40 \n"
        "   specular 1 1 1 18 \n"
        "   specular 0 1 0 1 128 \n"
        "   scene_blend alpha_blend \n"
        "   scene_blend one src_alpha \n"
        "   depth_check on \n"
        "   depth_write off \n"
        "   depth_func less_equal \n"
        "   depth_bias 5 \n"
        "   alpha_rejection not_equal 127 \n"
        "   cull_hardware clockwise \n"
        "   cull_software front \n"
        "   lighting on \n"
        "   shading gouraud \n"
        "   polygon_mode wireframe \n"
        "   fog_override false \n"
        "   fog_override true exp 1 1 1 0.002 100 10000 \n"
        "   fog_override true \n"
        "   colour_write off \n"
        "   max_lights 7 \n"
        "   iteration once \n"
        "   iteration once_per_light point \n"
        "   iteration 5 \n"
        "   iteration 5 per_light point \n"
        "   point_size 3 \n"
        "   point_sprites on \n"
        "   point_size_attenuation on \n"
        "   point_size_min 1.00 \n"
        "   point_size_max 10 \n"
        "   texture_unit first {texture MyAlphaTexture.png alpha tex_coord_set 0 } \n"
        "   texture_unit first { \n"
        "       cubic_texture cubemap_fr.jpg \t cubemap_bk.jpg cubemap_lf.jpg cubemap_rt.jpg cubemap_up.jpg cubemap_dn.jpg separateUV \n"
        "   } \n"
        "   texture_unit \"second one\" { \n"
        "       texture_alias foo-bar \n"
        "       anim_texture MyAlphaTexture2.png 4 6.5 \n"
        "       anim_texture MyAlphaTexture2.png MyAlphaTexture6.png    MyAlphaTexture9.png 6.5 \n"
        "       anim_texture MyAlphaTexture2.png MyAlphaTexture6.png \n"
        "           MyAlphaTexture9.png MyAlphaTexture11.png MyAlphaTexture12.png 6.5 \n"
        "       tex_address_mode wrap \n"
        "       tex_address_mode wrap mirror \n"
        "       tex_address_mode wrap mirror border \n"
        "       tex_border_colour 1.0 0.5 0.7 1.0 \n"
        "       filtering trilinear \n"
        "       filtering none \n"
        "       filtering linear point none \n"
        "       max_anisotropy 5 \n"
        "       colour_op replace \n"
        "       colour_op add \n"
        "       colour_op_ex add_signed src_manual src_texture 1 1 1 1\n"
        "       colour_op_multipass_fallback one one_minus_dest_alpha \n"
        "       alpha_op_ex add_signed src_manual src_current 0.5 \n"
        "       env_map cubic_reflection \n"
        "       scroll 5 2 \n"
        "       scroll_anim  2 4 \n"
        "       rotate 2.3 \n"
        "       rotate_anim 0.5 \n"
        "       scale 1 1.4 \n"
        "       wave_xform scale_x sine 1.0 0.2 0.0 5.0 \n"
        "       transform 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 \n"

        "       } \n"
        "   texture_unit \"second one\" { \n"
        "       anim_texture MyAlphaTexture2.png MyAlphaTexture3.png 6.5} \n"
        " \n"
        " } } } \n"
        "material clone 1 : test2 {} \n"
        // GPU Programs
        "vertex_program miny cg { \n"
        "  source program.cg \n"
        "  entry_point main \n"
        "  profiles vs_1_1 arbvp1 \n"
        " } \n"
        "vertex_program tiny hlsl { \n"
        "  source program.hlsl \n"
        "  entry_point main \n"
        "  target vs_1_1 \n"
        "  default_params { \n"
        "    param_named google float4 0 1 2 3 \n"
        "    param_indexed 1 int 0 \n"
        "    param_indexed_auto 0 blah 0 \n"
        "    param_named_auto chicken mustard \n"
        "  } \n"
        " } \n"
        "fragment_program better glsl { \n"
        "   source program.glsl \n"
        "   attach specular.glsl \n"
        " } \n"
        ;
    CPPUNIT_ASSERT(compile(simpleScript, "MaterialScriptTest"));

    delete mth;
}

