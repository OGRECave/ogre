#!/bin/bash

sed -f ../SDK/iPhone/edit_linker_paths.sed OGRE.xcodeproj/project.pbxproj > tmp.pbxproj
mv tmp.pbxproj OGRE.xcodeproj/project.pbxproj
