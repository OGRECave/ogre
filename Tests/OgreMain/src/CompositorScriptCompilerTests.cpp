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
#include "OgreCompositorManager.h"
#include "CompositorScriptCompilerTests.h"
#include "OgreCompositorScriptCompiler.h"
#include "OgreStringConverter.h"

#define ARRAYSIZE(array) (sizeof(array)/sizeof(array[0]))

// Regsiter the suite
CPPUNIT_TEST_SUITE_REGISTRATION( CompositorScriptCompilerTests );

void CompositorScriptCompilerTests::setUp()
{
}
void CompositorScriptCompilerTests::tearDown()
{
}

void CompositorScriptCompilerTests::testPositionToNextSymbol()
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

void CompositorScriptCompilerTests::testIsFloatValue(void)
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

void CompositorScriptCompilerTests::testIsLexemeMatch(void)
{
  const String TestStr = "Compositor test";
  const String TestSymbols = "compositor";

  mSource = &TestStr;
  mCharPos = 0;
  CPPUNIT_ASSERT(isLexemeMatch(TestSymbols, false));
  //CPPUNIT_ASSERT(isLexemeMatch(TestSymbols, true));
  mCharPos = 1;
  CPPUNIT_ASSERT(!isLexemeMatch(TestSymbols, false));
  CPPUNIT_ASSERT(!isLexemeMatch(TestSymbols, true));

}

void CompositorScriptCompilerTests::testCompile()
{
    if (!ResourceGroupManager::getSingletonPtr())
        new ResourceGroupManager();
    if (!MaterialManager::getSingletonPtr())
    {
        new MaterialManager();
        MaterialManager::getSingleton().initialise();
    }
    if (!CompositorManager::getSingletonPtr())
        new CompositorManager();

    //compositorMgr->initialise();
    const String simpleScript = "compositor \"First Test\" { technique { target_output { } } }";
    CPPUNIT_ASSERT(compile(simpleScript, "Test Compositor"));

    const String BW_Script =
        "/// Black and white effect \n"
        "compositor B&W \n"
        "{ \n"
        "    technique \n"
        "    { \n"
        "        // Temporary textures \n"
        "        texture scene target_width target_height PF_A8R8G8B8 \n"
        " \n"
        "        target scene \n"
        "        { \n"
        "            // Render output from previous compositor (or original scene) \n"
        "            input previous \n"
        "        }"
        "        target_output \n"
        "        { \n"
        "            // Start with clear output \n"
        "            input none \n"
        "            // Draw a fullscreen quad with the blur \n"
        "            pass render_quad \n"
        "            { \n"
        "                // Renders a fullscreen quad with a material \n"
        "                material PostFilters/BlackAndWhite \n"
        "                input 0 scene \n"
        "            } \n"
        "        } \n"
        "    } \n"
        "} \n";

    CPPUNIT_ASSERT(compile(BW_Script, "Test Black & White Script"));

    const String Bloom_Script =
    "/// Manuel's bloom \n"
    "/// Needs a scene-sized rtt, but does only one render of the scene \n"
    "compositor Bloom \n"
    "{ \n"
    "    technique \n"
    "    { \n"
    "        // Temporary textures \n"
    "        texture scene target_width target_height PF_A8R8G8B8 \n"
    "        texture rt0 128 128 PF_A8R8G8B8 \n"
    "        texture rt1 128 128 PF_A8R8G8B8 \n"
    "     \n"
    "        target scene \n"
    "        { \n"
    "            // Render output from previous compositor (or original scene) \n"
    "            input previous \n"
    "        } \n"
    "        target rt0 \n"
    "        { \n"
    "            // Start with clear texture \n"
    "            input none \n"
    "            // Vertical blur pass \n"
    "            pass render_quad \n"
    "            { \n"
    "                // Renders a fullscreen quad with a material \n"
    "                material PostFilters/Blur0 \n"
    "                input 0 scene \n"
    "            } \n"
    "        } \n"
    "        target rt1 \n"
    "        { \n"
    "            // Start with clear texture \n"
    "            input none \n"
    "            // Horizontal blur pass \n"
    "            pass render_quad \n"
    "            { \n"
    "                // Renders a fullscreen quad with a material \n"
    "                material PostFilters/Blur1 \n"
    "                input 0 rt0 \n"
    "            } \n"
    "        } \n"
    "        target_output \n"
    "        { \n"
    "            // Start with clear output \n"
    "            input none \n"
    "            // Draw a fullscreen quad \n"
    "            pass render_quad \n"
    "            { \n"
    "                // Renders a fullscreen quad with a material \n"
    "                material PostFilters/BloomBlend \n"
    "                input 0 scene \n"
    "                input 1 rt1 \n"
    "            } \n"
    "        } \n"
    "    } \n"
    "} \n";
    CPPUNIT_ASSERT(compile(Bloom_Script, "Test Bloom Script"));
}

