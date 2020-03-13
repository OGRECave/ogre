### Simple to follow instruction set for setup first project in self-build ogre1.12.5-project for beginners. (Linux only!)

### Build and install ogre1-12-5:

### Terminal-instructions:

sudo apt-get install -y libgles2-mesa-dev libxt-dev libxaw7-dev  
sudo apt-get install -y nvidia-cg-toolkit libsdl2-dev doxygen  

cd replaceWIthCustomPathToYourWorkspace
git clone --recursive --branch v1.12.5 https://github.com/OGRECave/ogre.git  
mkdir build_me  
cmake-gui  
  * set absolute path to ogre directory and the build_me directory
  * click on configure, then windows pops up “CMakeSetup“: only click on finish and wait until it‘s finished
  * click again on configure and wait until it‘s finished
  * click on generate and wait until it‘s finished

cd build_me  
cmake --build . --config release  
make OgreDoc  
sudo make install  


### Setup 1. Project:
 
mkdir ogreFirstProject  
cd ogreFirstProject  
copy the file ogre/Samples/Tutorials/CMakeLists.txt in this directory  
copy the file ogre/Samples/Tutorials/Bootstrap.cpp in this directory  
mkdir build  
cd build  
cmake ../  
make  
./0_Bootstrap  
  
Done! Your first ogre1.12.5 project without any pain :)  

### Known Issues

Issues after executing ./0_Bootstrap?  
Look here:  
https://forums.ogre3d.org/viewtopic.php?f=2&t=95682  
