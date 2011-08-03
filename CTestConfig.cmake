## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.
## # The following are required to uses Dart and the Cdash dashboard
##   ENABLE_TESTING()
##   INCLUDE(CTest)
set(CTEST_PROJECT_NAME "OGRE3D")
set(CTEST_NIGHTLY_START_TIME "00:00:00 EST")

set(CTEST_DROP_METHOD "http")
# this is temporary, my personal hosting:
set(CTEST_DROP_SITE "rileyadams.net")
set(CTEST_DROP_LOCATION "/CDash/submit.php?project=OGRE3D")
set(CTEST_DROP_SITE_CDASH TRUE)
