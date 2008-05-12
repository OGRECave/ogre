#! /bin/bash

cd ./XMLConverter
xcodebuild -configuration Release

cd ../MeshUpgrader
xcodebuild -configuration Release

cd ../MaterialUpgrader
xcodebuild -configuration Release


