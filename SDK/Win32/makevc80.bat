bash prepsdkbuild.sh
call buildsdk Ogre_vc8.sln
if errorlevel goto end
bash copysamples.sh VC8
pushd ..\..\Docs\src
doxygen html.cfg
cd ..\api\html
hhc index.hhp
popd
makensis ogresdk_vc80.nsi

:end
pause

