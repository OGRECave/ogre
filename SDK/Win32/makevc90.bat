bash prepsdkbuild.sh
call buildsdk Ogre_vc9.sln
if errorlevel goto end
bash copysamples.sh VC9
pushd ..\..\Docs\src
doxygen html.cfg
cd ..\api\html
hhc index.hhp
popd
makensis ogresdk_vc90.nsi

:end
pause

