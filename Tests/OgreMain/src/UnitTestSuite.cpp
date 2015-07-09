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

#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreMaterialManager.h"
//#include "OgreCompositorManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreGpuProgramManager.h"
#include "OgreLodStrategyManager.h"

#include "UnitTestSuite.h"

template<> UnitTestSuite* Ogre::Singleton<UnitTestSuite>::msSingleton = 0;

//--------------------------------------------------------------------------
void UnitTestSuite::setUpSuite()
{
    Ogre::LogManager* logMgr = new Ogre::LogManager();
    logMgr->createLog("OgreTest.log", true, true);
    logMgr->setLogDetail(Ogre::LL_BOREME);
    Ogre::LogManager::getSingletonPtr()->logMessage("----------------------------------------------------------------------------------------------------");
    logMgr->logMessage("##> Ogre3D Unit Test Suite successfully set up.");
    Ogre::LogManager::getSingletonPtr()->logMessage("----------------------------------------------------------------------------------------------------");
}
//--------------------------------------------------------------------------
void UnitTestSuite::tearDownSuite()
{
    // Shutdown and release managers that might have been created
    if(Ogre::HighLevelGpuProgramManager::getSingletonPtr())
        delete Ogre::HighLevelGpuProgramManager::getSingletonPtr();
    if(Ogre::GpuProgramManager::getSingletonPtr())
        delete Ogre::GpuProgramManager::getSingletonPtr();
//    if(Ogre::CompositorManager::getSingletonPtr())
//        delete Ogre::CompositorManager::getSingletonPtr();
    if(Ogre::MaterialManager::getSingletonPtr())
        delete Ogre::MaterialManager::getSingletonPtr();
    if(Ogre::ResourceGroupManager::getSingletonPtr())
        delete Ogre::ResourceGroupManager::getSingletonPtr();

    if(Ogre::ResourceGroupManager::getSingletonPtr())
        delete Ogre::ResourceGroupManager::getSingletonPtr();
    if(Ogre::LodStrategyManager::getSingletonPtr())
        delete Ogre::LodStrategyManager::getSingletonPtr();

    Ogre::LogManager::getSingletonPtr()->logMessage("----------------------------------------------------------------------------------------------------");
    Ogre::LogManager::getSingletonPtr()->logMessage("##> Ogre3D Unit Test Suite successfully shut down.");
    Ogre::LogManager::getSingletonPtr()->logMessage("----------------------------------------------------------------------------------------------------");

    if(Ogre::LogManager::getSingletonPtr())
        delete Ogre::LogManager::getSingletonPtr();
}
//--------------------------------------------------------------------------
void UnitTestSuite::startTestSetup(const std::string testName)
{
    Ogre::LogManager::getSingletonPtr()->logMessage("----------------------------------------------------------------------------------------------------");
    Ogre::LogManager::getSingletonPtr()->logMessage("||--> Starting Setup of Unit Test [" + testName + "]:");
}
//--------------------------------------------------------------------------
void UnitTestSuite::startTestMethod(const std::string testName)
{
    Ogre::LogManager::getSingletonPtr()->logMessage("||--> Starting Unit Test [" + testName + "]:");
}
//--------------------------------------------------------------------------

