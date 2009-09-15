/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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

#include "UseCustomCapabilitiesTests.h"
#include "OgreRenderSystem.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreRenderSystemCapabilitiesSerializer.h"
#include "OgreRenderSystemCapabilitiesManager.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreLog.h"



// Regsiter the suite
CPPUNIT_TEST_SUITE_REGISTRATION( UseCustomCapabilitiesTests );

void UseCustomCapabilitiesTests::setUp()
{
    using namespace Ogre;

	// set up silent logging to not polute output
	if(LogManager::getSingletonPtr())
		delete Ogre::LogManager::getSingletonPtr();
	
	// write cleanup to log
	if(LogManager::getSingletonPtr() == 0)
	{
		LogManager* logManager = new LogManager();
		logManager->createLog("testCustomCapabilitiesSetUp.log", true, false);
	}

	
	if(Ogre::HighLevelGpuProgramManager::getSingletonPtr())
		delete Ogre::HighLevelGpuProgramManager::getSingletonPtr();
	if(Ogre::GpuProgramManager::getSingletonPtr())
		delete Ogre::GpuProgramManager::getSingletonPtr();
	if(Ogre::CompositorManager::getSingletonPtr())
		delete Ogre::CompositorManager::getSingletonPtr();
	if(Ogre::MaterialManager::getSingletonPtr())
		delete Ogre::MaterialManager::getSingletonPtr();
	if(Ogre::ResourceGroupManager::getSingletonPtr())
		delete Ogre::ResourceGroupManager::getSingletonPtr();

	// set up silent logging to not polute output
	if(LogManager::getSingletonPtr())
		delete Ogre::LogManager::getSingletonPtr();
}

void UseCustomCapabilitiesTests::tearDown()
{
	using namespace Ogre;
	// set up silent logging to not polute output
	if(LogManager::getSingletonPtr())
		delete Ogre::LogManager::getSingletonPtr();

}

void checkCaps(const Ogre::RenderSystemCapabilities* caps)
{
    using namespace Ogre;

    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_AUTOMIPMAP), true);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_BLENDING), true);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_ANISOTROPY), true);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_DOT3), true);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_CUBEMAPPING), true);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_HWSTENCIL), true);

    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_VBO), true);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_VERTEX_PROGRAM), true);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_FRAGMENT_PROGRAM), true);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_SCISSOR_TEST), true);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_TWO_SIDED_STENCIL), true);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_STENCIL_WRAP), true);

    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_HWOCCLUSION), true);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_USER_CLIP_PLANES), true);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_VERTEX_FORMAT_UBYTE4), true);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_INFINITE_FAR_PLANE), true);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_HWRENDER_TO_TEXTURE), true);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_TEXTURE_FLOAT), true);

    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_NON_POWER_OF_2_TEXTURES), false);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_TEXTURE_3D), true);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_POINT_SPRITES), true);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_POINT_EXTENDED_PARAMETERS), true);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_VERTEX_TEXTURE_FETCH), false);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_MIPMAP_LOD_BIAS), true);

    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_TEXTURE_COMPRESSION), true);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_TEXTURE_COMPRESSION_DXT), true);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_TEXTURE_COMPRESSION_VTC), false);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_FBO), true);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_FBO_ARB), false);

    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_FBO_ATI), false);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_PBUFFER), false);
    CPPUNIT_ASSERT_EQUAL(caps->hasCapability(RSC_PERSTAGECONSTANT), false);

    CPPUNIT_ASSERT(caps->isShaderProfileSupported("arbfp1"));
    CPPUNIT_ASSERT(caps->isShaderProfileSupported("arbvp1"));
    CPPUNIT_ASSERT(caps->isShaderProfileSupported("glsl"));
    CPPUNIT_ASSERT(caps->isShaderProfileSupported("ps_1_1"));
    CPPUNIT_ASSERT(caps->isShaderProfileSupported("ps_1_2"));
    CPPUNIT_ASSERT(caps->isShaderProfileSupported("ps_1_3"));
    CPPUNIT_ASSERT(caps->isShaderProfileSupported("ps_1_4"));

    CPPUNIT_ASSERT_EQUAL(caps->getMaxPointSize(), (Real)1024);
    CPPUNIT_ASSERT_EQUAL(caps->getNonPOW2TexturesLimited(), false);
    CPPUNIT_ASSERT_EQUAL(caps->getVertexTextureUnitsShared(), true);
    CPPUNIT_ASSERT_EQUAL(caps->getNumWorldMatrices(), (Ogre::ushort)0);
    CPPUNIT_ASSERT_EQUAL(caps->getNumTextureUnits(), (Ogre::ushort)16);
    CPPUNIT_ASSERT_EQUAL(caps->getStencilBufferBitDepth(), (Ogre::ushort)8);
    CPPUNIT_ASSERT_EQUAL(caps->getNumVertexBlendMatrices(), (Ogre::ushort)0);
    CPPUNIT_ASSERT_EQUAL(caps->getNumMultiRenderTargets(), (Ogre::ushort)4);

    CPPUNIT_ASSERT_EQUAL(caps->getVertexProgramConstantFloatCount(), (Ogre::ushort)256);
    CPPUNIT_ASSERT_EQUAL(caps->getVertexProgramConstantIntCount(), (Ogre::ushort)0);
    CPPUNIT_ASSERT_EQUAL(caps->getVertexProgramConstantBoolCount(), (Ogre::ushort)0);

    CPPUNIT_ASSERT_EQUAL(caps->getFragmentProgramConstantFloatCount(), (Ogre::ushort)64);
    CPPUNIT_ASSERT_EQUAL(caps->getFragmentProgramConstantIntCount(), (Ogre::ushort)0);
    CPPUNIT_ASSERT_EQUAL(caps->getFragmentProgramConstantBoolCount(), (Ogre::ushort)0);

    CPPUNIT_ASSERT_EQUAL(caps->getNumVertexTextureUnits(), (Ogre::ushort)0);
    CPPUNIT_ASSERT(caps->isShaderProfileSupported("arbvp1"));
    CPPUNIT_ASSERT(caps->isShaderProfileSupported("arbfp1"));
}

void setUpGLRenderSystemOptions(Ogre::RenderSystem* rs)
{
	using namespace Ogre;
	ConfigOptionMap options = rs->getConfigOptions();
	// set default options
	// this should work on every semi-normal system
	rs->setConfigOption(String("Colour Depth"), String("32"));
	rs->setConfigOption(String("FSAA"), String("0"));
	rs->setConfigOption(String("Full Screen"), String("No"));
	rs->setConfigOption(String("VSync"), String("No"));
	rs->setConfigOption(String("Video Mode"), String("800 x 600"));
	
	// use the best RTT
	ConfigOption optionRTT = options["RTT Preferred Mode"];
	
	if(find(optionRTT.possibleValues.begin(), optionRTT.possibleValues.end(), "FBO") != optionRTT.possibleValues.end())
	{
		rs->setConfigOption(String("RTT Preferred Mode"), String("FBO"));
	}
	else if(find(optionRTT.possibleValues.begin(), optionRTT.possibleValues.end(), "PBuffer") != optionRTT.possibleValues.end())
	{
		rs->setConfigOption(String("RTT Preferred Mode"), String("PBuffer"));
	}
	else
		rs->setConfigOption(String("RTT Preferred Mode"), String("Copy"));
}

void UseCustomCapabilitiesTests::testCustomCapabilitiesGL()
{
    using namespace Ogre;

	// set up silent logging to not polute output
	if(LogManager::getSingletonPtr())
		delete Ogre::LogManager::getSingletonPtr();
	
	if(LogManager::getSingletonPtr() == 0)
	{
		LogManager* logManager = new LogManager();
		logManager->createLog("testCustomCapabilitiesGL.log", true, false);
	}

	Root* root = new Root("plugins.cfg");
	RenderSystem* rs = root->getRenderSystemByName("OpenGL Rendering Subsystem");
	if(rs == 0)
	{
		CPPUNIT_ASSERT_ASSERTION_PASS("This test is irrelevant because GL RenderSystem is not available");
	}
	else
	{
		try {
			setUpGLRenderSystemOptions(rs);
			root->setRenderSystem(rs);
			root->initialise(true, "OGRE testCustomCapabilitiesGL Window",
											"../../../Media/CustomCapabilities/customCapabilitiesTest.cfg");

			const RenderSystemCapabilities* caps = rs->getCapabilities();

			checkCaps(caps);
		}
		// clean up root, in case of error, and let cppunit to handle the exception
		catch(...)
		{
		}
	}
    delete root;
}

void setUpD3D9RenderSystemOptions(Ogre::RenderSystem* rs)
{
	using namespace Ogre;
	ConfigOptionMap options = rs->getConfigOptions();
	// set default options
	// this should work on every semi-normal system
	rs->setConfigOption(String("Anti aliasing"), String("None"));
	rs->setConfigOption(String("Full Screen"), String("No"));
	rs->setConfigOption(String("VSync"), String("No"));
	rs->setConfigOption(String("Video Mode"), String("800 x 600 @ 32-bit colour"));
	
	// pick first available device
	ConfigOption optionDevice = options["Rendering Device"];

	rs->setConfigOption(optionDevice.name, optionDevice.currentValue);
}

void UseCustomCapabilitiesTests::testCustomCapabilitiesD3D9()
{
	// set up silent logging to not polute output
	if(LogManager::getSingletonPtr())
		delete Ogre::LogManager::getSingletonPtr();
	
	if(LogManager::getSingletonPtr() == 0)
	{
		LogManager* logManager = new LogManager();
		logManager->createLog("testCustomCapabilitiesD3D9.log", true, false);
	}

    Root* root = new Root("plugins.cfg");
	RenderSystem* rs = root->getRenderSystemByName("Direct3D9 Rendering Subsystem");
	if(rs == 0)
	{
		CPPUNIT_ASSERT_ASSERTION_PASS("This test is irrelevant because D3D9 RenderSystem is not available");
	}
	else
	{	
		try {
			setUpD3D9RenderSystemOptions(rs);
			root->setRenderSystem(rs);
			root->initialise(true, "OGRE testCustomCapabilitiesD3D9 Window",
											"../../../Media/CustomCapabilities/customCapabilitiesTest.cfg");
		
			const RenderSystemCapabilities* caps = rs->getCapabilities();

			checkCaps(caps);
		}
		// clean up root, in case of error, and let cppunit to handle the exception
		catch(...)
		{
		}
	}

    delete root;
}


