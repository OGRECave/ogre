#!/bin/bash
tag=$1
if [[ "$tag" == "" ]]; then 
  echo No tag specified
  exit -1
fi

foldername=ogre_src_$tag

# You can set OGRE_RELEASE_CLONE_SOURCE to a local repo if you want to speed things up
if [[ "$OGRE_RELEASE_CLONE_SOURCE" == "" ]]; then
  OGRE_RELEASE_CLONE_SOURCE=http://bitbucket.org/sinbad/ogre
fi

hg clone -r $tag $OGRE_RELEASE_CLONE_SOURCE $foldername
# Build configure
pushd $foldername
# delete repo, we only want working copy
rm -rf .hg
# Gen docs
cd Docs
bash ./src/makedocs.sh
# remove unnecessary files
cd ../api/html
rm -f *.hhk *.hhc *.map *.md5 *.dot *.hhp *.plist *.xml ../*.tmp
popd
# tarball for Linux
rm -f $foldername.tar.bz2
tar -cvhjf $foldername.tar.bz2 $foldername

