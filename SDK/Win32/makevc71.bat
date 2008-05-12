bash prepsdkbuild.sh
call buildsdk_vc71
if errorlevel goto end
bash copysamples.sh VC7
pushd ..\..\Docs\src
doxygen html.cfg
cd ..\api\html
hhc index.hhp
popd
makensis ogresdk_vc71.nsi
:end
pause
