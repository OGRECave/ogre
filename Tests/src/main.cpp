#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/TestRunner.h>

#include "OgreString.h"
#include "OgreLogManager.h"

#include "UnitTestSuite.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
int main(int argc, char *argv[])
#endif
{
    OGRE_NEW UnitTestSuite();
    UnitTestSuite::getSingletonPtr()->setUpSuite();

    // Create the event manager and test controller
    CPPUNIT_NS::TestResult controller;

    // Add a listener that collects test result
    CPPUNIT_NS::TestResultCollector result;
    controller.addListener(&result);

    // Add a listener that print dots as test run.
    CPPUNIT_NS::BriefTestProgressListener progress;
    controller.addListener(&progress);

    // Add the top suite to the test runner
    CPPUNIT_NS::TestRunner runner;
    runner.addTest(CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest());
    runner.run(controller);

    // Print test results to a separate file
    std::ofstream ofile("OgreTestResults.xml");
    
    CPPUNIT_NS::XmlOutputter xmlOut(&result, ofile);
    xmlOut.write();

    // Prepare result paragraph.
    char separator[82];
    memset(separator, '+', 80);
    separator[80] = '\n';
    separator[81] = 0;
    Ogre::StringStream str;
    str << "\n" << separator << separator;
    CPPUNIT_NS::CompilerOutputter txtOut(&result, str);
    txtOut.write();
    str << separator << separator << "\n";

    // Print condensed result.
    Ogre::LogManager::getSingletonPtr()->logMessage("----------------------------------------------------------------------------------------------------");
    Ogre::LogManager::getSingletonPtr()->logMessage("##> Condensed Ogre3D Unit Test results (complete CppUnit log saved to \"OgreTestResults.xml\"):");
    Ogre::LogManager::getSingletonPtr()->logMessage("----------------------------------------------------------------------------------------------------");
    Ogre::LogManager::getSingletonPtr()->logMessage(str.str());

#if OGRE_COMPILER == OGRE_COMPILER_MSVC
    if (IsDebuggerPresent())
    {
        // Write report to MSVC IDE console for convenience.
        // If you click on the assert location then the IDE will jump there!        
        OutputDebugStringA(str.str().c_str());
    }
#endif

    UnitTestSuite::getSingletonPtr()->tearDownSuite();
    delete UnitTestSuite::getSingletonPtr();

    return 0;
}
