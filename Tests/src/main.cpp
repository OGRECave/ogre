#include <gtest/gtest.h>

#include "OgreLogManager.h"

int main(int argc, char *argv[])
{
    Ogre::LogManager* logMgr = new Ogre::LogManager();
    logMgr->createLog("OgreTest.log", true, false);
    logMgr->setLogDetail(Ogre::LL_BOREME);

    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
