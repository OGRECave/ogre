#! /bin/bash

cd ./XMLConverter
xcodebuild -configuration Debug

cd ../MeshUpgrader
xcodebuild -configuration Debug

cd ../MaterialUpgrader
xcodebuild -configuration Debug


