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
#include "StringInterfaceTests.h"
#include "OgreStringInterface.h"
#include "OgrePass.h"

#include "UnitTestSuite.h"

using namespace Ogre;

ParamDictionary& passTypeInfo() {
    static AutoParamDictionary<Pass> pass;

    if(not pass.getParameters().empty()) {
        return pass;
    }

    pass.addParameter<ColourValue, &Pass::getAmbient, &Pass::setAmbient>("Ambient")
        /* this requires C++11 or a typedef to int for CullingMode like in VertexColourTracking
         .addParameter<int, reinterpret_cast<int (Pass::*)() const>(&Pass::getCullingMode),
                reinterpret_cast<void (Pass::*)(int)>(&Pass::setCullingMode)>("CullingMode") */
        .addParameter<ColourValue, &Pass::getDiffuse, &Pass::setDiffuse>("Diffuse")
        .addParameter<Real, &Ogre::Pass::getShininess, &Ogre::Pass::setShininess>("Shininess")
        .addParameter<bool, &Ogre::Pass::getDepthWriteEnabled, &Ogre::Pass::setDepthWriteEnabled>("DepthWriteEnabled")
        .addParameter<TrackVertexColourType, &Pass::getVertexColourTracking, &Pass::setVertexColourTracking>("VertexColourTracking")
        .addParameter<ColourValue, &Pass::getSelfIllumination, &Pass::setSelfIllumination>("SelfIllumination");

    return pass;
}

// Register the test suite
CPPUNIT_TEST_SUITE_REGISTRATION(StringInterfaceTests);

//--------------------------------------------------------------------------
void StringInterfaceTests::setUp()
{
    UnitTestSuite::getSingletonPtr()->startTestSetup(__FUNCTION__);
}
//--------------------------------------------------------------------------
void StringInterfaceTests::tearDown()
{
}
//--------------------------------------------------------------------------
void StringInterfaceTests::testReflectionBasic()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    Pass pass(NULL, 0);

    String in = "1 0 1 0";
    passTypeInfo().getParamCommand("Ambient")->doSet(&pass, in);
    String out = passTypeInfo().getParamCommand("Ambient")->doGet(&pass);

    CPPUNIT_ASSERT_EQUAL(in, out);

    in = "2";
    passTypeInfo().getParamCommand("VertexColourTracking")->doSet(&pass, in);
    out =  passTypeInfo().getParamCommand("VertexColourTracking")->doGet(&pass);
    CPPUNIT_ASSERT_EQUAL(in, out);
}
//--------------------------------------------------------------------------
