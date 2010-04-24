@echo off
if "%1" == "" goto paramErr

set tag=%1
set foldername=ogre_src_%tag%

rem You can set OGRE_RELEASE_CLONE_SOURCE to a local repo if you want to speed things up
if "%OGRE_RELEASE_CLONE_SOURCE%" == "" SET OGRE_RELEASE_CLONE_SOURCE=http://bitbucket.org/sinbad/ogre

rem Clean down existing snapshot area
echo Cleaning up %foldername%...
rmdir /S/Q %foldername%
rem Clone ogre from local copy (you should make sure this includes the tag)
echo Cloning...
hg clone -r %tag% %OGRE_RELEASE_CLONE_SOURCE% %foldername%
rem Build configure
pushd %foldername%
rem delete repo, we only want working copy
echo Remove repository...
rmdir /S/Q .hg
rem Gen docs
cd Docs\src
doxygen html.cfg
rem remove unnecessary files
cd ..\api\html
del /Q/F *.hhk *.hhc *.map *.md5 *.dot *.hhp *.plist ..\*.tmp
popd

rem Make self-extracting zip
del /Q/F %foldername%.exe
rem create self-extracting 7zip archive
7z a -r -y -sfx7z.sfx %foldername%.exe %foldername%

goto end

:paramErr
echo You must supply a tag name

:end